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

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/pci.h>
#include <linux/interrupt.h>
#include <linux/version.h>
#include <linux/phy.h>
#include <linux/mfd/core.h>
#include <linux/platform_device.h>

#include "edge_bridge.h"
#include "edge_port.h"
#include "edge_link.h"
#include "edge_mdio.h"
#include "altera_pio.h"

#define PFX			"EDGX-PCIe: "
#define DRV_NAME		"edgx-pcie-c10"
#define VENDOR_ID		(0x1c7e)
#define DEVICE_ID		(0xa102)
#define SUBVENDOR_ID		(0x0000)
#define SUBDEVICE_ID		(0x0000)
#define EDGX_PCI_BR_BAR		(0U)
#define EDGX_PCI_BR_OFFS	(0x2000000)
#define EDGX_PCI_BR_SIZE	(0x1ffffff)
#define EDGX_PCI_I2C_BAR	(1U)
#define EDGX_PCI_I2C_OFFS	(0x0)
#define EDGX_PCI_I2C_SIZE	(0x80)

struct edgx_pci_drv {
	struct edgx_br *br;
	struct edgx_br_irq irq;
};

static const struct pci_device_id edgx_pci_ids[] =
{
	{PCI_DEVICE_SUB(VENDOR_ID, DEVICE_ID, SUBVENDOR_ID, SUBDEVICE_ID)},
	{}
};

static struct resource i2c_res[] = {
	{
		.start = 0x40,
		.end =   0x7f,
		.flags = IORESOURCE_MEM,
	},
	{
		.start = 0,
		.end = 0,
		.flags = IORESOURCE_IRQ,
	},
};

static struct resource i2c_res2[] = {
	{
		.start = 0x0,
		.end =   0x3f,
		.flags = IORESOURCE_MEM,
	},
	{
		.start = 1,
		.end = 1,
		.flags = IORESOURCE_IRQ,
	},
};

static struct mfd_cell i2c_cell[] = {
	{
		.id = 1,
		.name = "altera-i2c",
		.of_compatible = "altr,softip-i2c-v1.0",
		.num_resources = ARRAY_SIZE(i2c_res),
		.resources = i2c_res,
	},
	{
		.id = 2,
		.name = "altera-i2c",
		.of_compatible = "altr,softip-i2c-v1.0",
		.num_resources = ARRAY_SIZE(i2c_res2),
		.resources = i2c_res2,
	}
};

static int bridge_id;

static int edgx_pci_probe(struct pci_dev *pdev, const struct pci_device_id *id);
static void edgx_pci_remove(struct pci_dev *pdev);

static struct pci_driver edgx_pci_driver = {
	.name = DRV_NAME,
	.id_table = edgx_pci_ids,
	.probe = &edgx_pci_probe,
	.remove = &edgx_pci_remove,
};

static int edgx_pci_get_irq(struct pci_dev *pdev, struct edgx_br_irq *irq)
{
	int ret, i;
	u8 *cra_base;

	/* Avalon-MM to PCI Express Interrupt Status Enable Register setting */
	cra_base = pci_iomap_range(pdev, 0, 0, 0x3fff);
	if (!cra_base) {
		dev_err(&pdev->dev, "Cannot map CRA device memory.\n");
		return -ENOMEM;
	}
	*((u16 *)&cra_base[0x50]) = 0x2;
	pci_iounmap(pdev, cra_base);

	ret = pci_alloc_irq_vectors(pdev, EDGX_IRQ_CNT, EDGX_IRQ_CNT,
				    PCI_IRQ_MSI | PCI_IRQ_MSIX);
	if (ret < 0) {
		dev_err(&pdev->dev, "pci_alloc_irq_vectors failed: %d!\n", ret);
		return ret;
	}

	if (ret != EDGX_IRQ_CNT) {
		dev_err(&pdev->dev, "Invalid IRQ count = %d!\n", ret);
		pci_free_irq_vectors(pdev);
		return -ENODEV;
	}

	irq->shared = false;
	irq->trig = EDGX_IRQ_EDGE_TRIG;

	for (i = 0; i < ret; i++) {
		irq->irq_vec[i] = pci_irq_vector(pdev, i);
		if (irq->irq_vec[i] < 0) {
			dev_err(&pdev->dev, "IRQ vector %d failed!\n", i);
			pci_free_irq_vectors(pdev);
			return ret;
		}
		dev_dbg(&pdev->dev, "IRQ-vector %d: %d\n", i, irq->irq_vec[i]);
	}

	return 0;
}

static int edgx_pci_probe(struct pci_dev *pdev, const struct pci_device_id *id)
{
	int ret;
	void *br_base;
	int i2c_irqbase;
	struct edgx_pci_drv *pci_drv;

	ret = pci_enable_device(pdev);
	if (ret) {
		dev_err(&pdev->dev, "Cannot enable PCI device.\n");
		return ret;
	}

	pci_set_master(pdev);

	pci_drv = kzalloc(sizeof(*pci_drv), GFP_KERNEL);
	if (!pci_drv) {
		ret = -ENOMEM;
		goto probe_out_drv_alloc;
	}

	ret = edgx_pci_get_irq(pdev, &pci_drv->irq);
	if (ret)
		goto probe_out_irq;

	i2c_irqbase = pci_drv->irq.irq_vec[7];

	ret =  devm_mfd_add_devices(&pdev->dev, PLATFORM_DEVID_NONE, i2c_cell,
			      	    ARRAY_SIZE(i2c_cell),
				    &pdev->resource[EDGX_PCI_I2C_BAR],
				    i2c_irqbase, NULL);
	if (ret) {
		dev_err(&pdev->dev, "MFD add I2C devices failed: %d\n", ret);
		goto probe_out_mdf_add;
	}

	if (!(pci_resource_flags(pdev, 0) & IORESOURCE_MEM)) {
		dev_err(&pdev->dev, "Invalid BAR0 resource flags.\n");
		ret = -ENODEV;
		goto probe_out_res_flag;
	}

	ret = pci_request_region(pdev, EDGX_PCI_BR_BAR, DRV_NAME);
	if (ret) {
		dev_err(&pdev->dev, "Cannot get PCI resource.\n");
		goto probe_out_res_flag;
	}

	br_base = pci_iomap_range(pdev, EDGX_PCI_BR_BAR,
				  EDGX_PCI_BR_OFFS, EDGX_PCI_BR_SIZE);
	if (!br_base) {
		dev_err(&pdev->dev, "Cannot map bridge device memory.\n");
		ret = -ENOMEM;
		goto probe_out_iomap_range;
	}

	ret = edgx_br_probe_one(bridge_id, &pdev->dev, br_base, &pci_drv->irq,
				&pci_drv->br, edgx_pci_driver.name);
	if (ret)
		goto probe_out_bridge;

	bridge_id++;
	dev_set_drvdata(&pdev->dev, pci_drv);
	return ret;

probe_out_bridge:
	pci_iounmap(pdev, br_base);
probe_out_iomap_range:
	pci_release_regions(pdev);
probe_out_res_flag:
	mfd_remove_devices(&pdev->dev);
probe_out_mdf_add:
	pci_free_irq_vectors(pdev);
probe_out_irq:
	kfree(pci_drv);
probe_out_drv_alloc:
	pci_disable_device(pdev);
	return ret;
}

static void edgx_pci_remove(struct pci_dev *pdev)
{
	struct edgx_pci_drv *pci_drv = dev_get_drvdata(&pdev->dev);
	void *br_base;

	if (!pci_drv)
		return;

	br_base = edgx_br_get_base(pci_drv->br);
	edgx_br_shutdown(pci_drv->br);
	pci_iounmap(pdev, br_base);
	pci_release_region(pdev, EDGX_PCI_BR_BAR);
	mfd_remove_devices(&pdev->dev);
	pci_free_irq_vectors(pdev);
	kfree(pci_drv);
	pci_disable_device(pdev);
}

static int __init edgx_pcie_module_init(void)
{
	return pci_register_driver(&edgx_pci_driver);
}

static void __exit edgx_pcie_module_exit(void)
{
	pci_unregister_driver(&edgx_pci_driver);
}

module_init(edgx_pcie_module_init);
module_exit(edgx_pcie_module_exit);

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("TTTech Industrial Automation AG <support@tttech-industrial.com>");
MODULE_DESCRIPTION("EDGE PCIe Host Interface Driver");
MODULE_DEVICE_TABLE(pci, edgx_pci_ids);
MODULE_VERSION(EDGX_SW_CORE_VERSION);
