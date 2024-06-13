// SPDX-License-Identifier: GPL-2.0
/*
 * TTTech EDGE/DE-IP Linux driver
 * Copyright(c) 2018 TTTech Industrial Automation AG.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, see <http://www.gnu.org/licenses/>.
 *
 * The full GNU General Public License is included in this distribution in
 * the file called "COPYING".
 *
 * Contact Information:
 * support@tttech-industrial.com
 * TTTech Industrial Automation AG, Schoenbrunnerstrasse 7, 1040 Vienna, Austria
 */

#include <linux/version.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of_device.h>
#include <linux/slab.h>
#include <linux/interrupt.h>

#include "edge.h"
#include "edge_bridge.h"
#include "edge_time.h"
#include "edge_mdio.h"
#include "altera_pio.h"

#define PFX "EDGX-PLATFORM: "

static const struct of_device_id flx_pio_dt_ids[] = {
	{ .compatible = "flx,pio" },
	{ /* sentinel */ },
};

static const struct of_device_id edgx_sw_dt_ids[] = {
	{ .compatible = "ttt,deip-sw",
	},
	{ .compatible = "ttt,es",
	},
	{ /* sentinel */ },
};

static const struct of_device_id edgx_mdio_dt_ids[] = {
	{ .compatible = "flx,eth-mdio",
	},
	{ /* sentinel */ },
};

static int bridge_id;
static int mdio_id;
static int pio_id;

static int flx_pio_pfm_probe(struct platform_device *pdev);
static int flx_pio_pfm_remove(struct platform_device *pdev);
static int edgx_sw_pfm_probe(struct platform_device *pdev);
static int edgx_sw_pfm_remove(struct platform_device *pdev);
static int edgx_mdio_pfm_probe(struct platform_device *pdev);
static int edgx_mdio_pfm_remove(struct platform_device *pdev);

static struct platform_driver flx_pio_pfm_driver = {
	.driver = {
		.name = "flx-pio",
		.of_match_table = flx_pio_dt_ids,
	},
	.probe = &flx_pio_pfm_probe,
	.remove = &flx_pio_pfm_remove,
};

static struct platform_driver edgx_sw_pfm_driver = {
	.driver = {
		.name = "edgx-pfm",
		.of_match_table = edgx_sw_dt_ids,
	},
	.probe  = edgx_sw_pfm_probe,
	.remove = edgx_sw_pfm_remove,
};

struct edgx_sw_pfm_drv {
	struct edgx_br *br;
	struct edgx_br_irq irq;
};

static struct platform_driver edgx_mdio_pfm_driver = {
	.driver = {
		.name = "edgx-mdio",
		.of_match_table = edgx_mdio_dt_ids,
	},
	.probe  = edgx_mdio_pfm_probe,
	.remove = edgx_mdio_pfm_remove,
};

static int edgx_get_mem_res(struct platform_device *pdev, void **base)
{
	struct resource *mem;

	mem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!mem) {
		dev_err(&pdev->dev, "No I/O memory defined on dev '%s'\n",
			dev_name(&pdev->dev));
		return -ENODEV;
	}

	*base = ioremap(mem->start, resource_size(mem));
	if (!*base) {
		dev_err(&pdev->dev,
			"I/O remapping failed for dev '%s' 0x%llx/0x%llx\n",
			dev_name(&pdev->dev),
			(long long unsigned int)mem->start,
			(long long unsigned int)resource_size(mem));
		return -ENODEV;
	}

	dev_info(&pdev->dev, "MEM-resource '%s' 0x%llx/0x%llx -> 0x%px\n",
		 mem->name, (long long unsigned int)mem->start,
		 (long long unsigned int)resource_size(mem), *base);
	return 0;
}

static int flx_pio_pfm_probe(struct platform_device *pdev)
{
	int ret;
	void *base;
	struct flx_pio_dev_priv *pio;

	ret = edgx_get_mem_res(pdev, &base);
	if (ret)
		return ret;

	ret = flx_pio_probe_one(pio_id, &pdev->dev, base, &pio);
	if (ret) {
		iounmap(base);
		return ret;
	}
	dev_set_drvdata(&pdev->dev, pio);
	pio_id++;
	return 0;
}

static int flx_pio_pfm_remove(struct platform_device *pdev)
{
	void *base;
	struct flx_pio_dev_priv *pio;

	pio = dev_get_drvdata(&pdev->dev);
	if (!pio)
		return -ENODEV;

	base = flx_pio_get_base(pio);
	if (!base)
		return -ENODEV;

	flx_pio_shutdown(pio);
	iounmap(base);
	dev_info(&pdev->dev, "Removed device '%s'\n",
		 dev_name(&pdev->dev));

	return 0;
}

static int edgx_sw_pfm_get_irq(struct platform_device *pdev,
			       struct edgx_br_irq *sw_irq)
{
	int ret;
	int i;
	int irq;
	u32 type;

	ret = platform_irq_count(pdev);
	if ((ret != 1) && (ret != EDGX_IRQ_CNT)) {
		dev_err(&pdev->dev, "Invalid IRQ count = %d!\n", ret);
		return -ENODEV;
	}

	sw_irq->shared = (ret == 1);

	for (i = 0; i < ret; i++) {
		irq = platform_get_irq(pdev, i);
		if (irq < 0) {
			dev_err(&pdev->dev, "Get interrupt %d failed!\n", i);
			return ret;
		}
		sw_irq->irq_vec[i] = irq;
		type = irq_get_trigger_type(irq);

		if (i == 0) {
			if (type & IRQ_TYPE_LEVEL_MASK)
				sw_irq->trig = EDGX_IRQ_LEVEL_TRIG;
			else
				sw_irq->trig = EDGX_IRQ_EDGE_TRIG;
		}
		dev_info(&pdev->dev, "Interrupt %d: IRQ=%d type=0x%x\n",
			 i, irq, type);
	}
	return 0;
}

static int edgx_sw_pfm_probe(struct platform_device *pdev)
{
	int ret;
	void *base;
	struct edgx_sw_pfm_drv *sw_pfm_drv;

	ret = edgx_get_mem_res(pdev, &base);
	if (ret)
		return ret;

	sw_pfm_drv = kzalloc(sizeof(*sw_pfm_drv), GFP_KERNEL);
	if (!sw_pfm_drv) {
		ret = -ENOMEM;
		goto sw_pfm_probe_out_alloc;
	}

	ret = edgx_sw_pfm_get_irq(pdev, &sw_pfm_drv->irq);
	if (ret)
		goto sw_pfm_probe_out_irq;

	ret = edgx_br_probe_one(bridge_id, &pdev->dev, base, &sw_pfm_drv->irq,
				&sw_pfm_drv->br, edgx_sw_pfm_driver.driver.name);
	if (ret)
		goto sw_pfm_probe_out_br_probe;

	dev_set_drvdata(&pdev->dev, sw_pfm_drv);
	bridge_id++;
	return 0;

sw_pfm_probe_out_br_probe:
sw_pfm_probe_out_irq:
	kfree(sw_pfm_drv);
sw_pfm_probe_out_alloc:
	iounmap(base);
	return ret;
}

static int edgx_sw_pfm_remove(struct platform_device *pdev)
{
	void *base;
	struct edgx_sw_pfm_drv *sw_pfm_drv;

	sw_pfm_drv = dev_get_drvdata(&pdev->dev);
	if (!sw_pfm_drv)
		return -ENODEV;

	base = edgx_br_get_base(sw_pfm_drv->br);
	if (!base)
		return -ENODEV;

	edgx_br_shutdown(sw_pfm_drv->br);
	iounmap(base);
	dev_info(&pdev->dev, "Removed device '%s'\n",
		 dev_name(&pdev->dev));

	return 0;
}

static int edgx_mdio_pfm_probe(struct platform_device *pdev)
{
	int ret;
	void *base;
	struct edgx_mdio *mdio;

	ret = edgx_get_mem_res(pdev, &base);
	if (ret)
		return ret;

	ret = edgx_mdio_probe_one(mdio_id, &pdev->dev, base, &mdio);
	if (ret) {
		iounmap(base);
		return ret;
	}
	dev_set_drvdata(&pdev->dev, mdio);
	mdio_id++;
	return 0;
}

static int edgx_mdio_pfm_remove(struct platform_device *pdev)
{
	void *base;
	struct edgx_mdio *mdio;

	mdio = dev_get_drvdata(&pdev->dev);
	if (!mdio)
		return -ENODEV;

	base = edgx_mdio_get_base(mdio);
	if (!base)
		return -ENODEV;

	edgx_mdio_shutdown(mdio);
	iounmap(base);
	dev_info(&pdev->dev, "Removed device '%s'\n",
		 dev_name(&pdev->dev));

	return 0;
}

static int __init edgx_pfm_init(void)
{
	int ret;

	pr_info(PFX "Registering pio platform device driver\n");
	ret = platform_driver_register(&flx_pio_pfm_driver);
	if (ret)
		return ret;

	pr_info(PFX "Registering mdio platform device driver\n");
	ret = platform_driver_register(&edgx_mdio_pfm_driver);
	if (ret)
		goto out_pfm_init_mdio;

	pr_info(PFX "Registering switch platform device driver\n");
	ret = platform_driver_register(&edgx_sw_pfm_driver);
	if (ret)
		goto out_pfm_init_sw;

	return ret;

out_pfm_init_sw:
	platform_driver_unregister(&edgx_mdio_pfm_driver);
out_pfm_init_mdio:
	platform_driver_unregister(&flx_pio_pfm_driver);
	return ret;
}

static void __exit edgx_pfm_exit(void)
{
	pr_info(PFX "Unregistering sw platform device driver\n");
	platform_driver_unregister(&edgx_sw_pfm_driver);
	pr_info(PFX "Unregistering mdio platform device driver\n");
	platform_driver_unregister(&edgx_mdio_pfm_driver);
	pr_info(PFX "Unregistering pio platform device driver\n");
	platform_driver_unregister(&flx_pio_pfm_driver);
}

struct edgx_br *edgx_get_edgx_br(struct platform_device *pdev)
{
	struct edgx_sw_pfm_drv *sw_pfm_drv = dev_get_drvdata(&pdev->dev);

	return sw_pfm_drv->br;
}
EXPORT_SYMBOL_GPL(edgx_get_edgx_br);

ktime_t edgx_ktime_get_worker_ptp(struct platform_device *pdev)
{
	struct timespec64 ts = { 0 };
	struct edgx_br     *br = edgx_get_edgx_br(pdev);
	struct edgx_time *time = edgx_br_get_br_time(br);
	if (!time)
		goto out;

	(void)edgx_tm_get_wrk_time(time, &ts);
out:
	return timespec64_to_ktime(ts);
}
EXPORT_SYMBOL_GPL(edgx_ktime_get_worker_ptp);

module_init(edgx_pfm_init);
module_exit(edgx_pfm_exit);

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("TTTech Industrial Automation AG <support@tttech-industrial.com>");
MODULE_DESCRIPTION("EDGE Platform Host Interface Driver");
MODULE_VERSION(EDGX_SW_CORE_VERSION);
