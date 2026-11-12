// SPDX-License-Identifier: GPL-2.0
/*
 * TTTech EDGE/DE-IP Linux driver
 * Copyright(c) 2018 TTTech Computertechnik AG.
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
 * support@4me.tttech-dependablenetworks.com
 * TTTech Computertechnik AG, Schoenbrunnerstrasse 7, 1040 Vienna, Austria
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/pci.h>
#include <linux/interrupt.h>
#include <linux/version.h>
#include <linux/phy.h>

#include "edge_bridge.h"
#include "edge_port.h"
#include "edge_link.h"
#include "edge_mdio.h"

#define DRV_NAME		"edgx-pcie"
#define VENDOR_ID		(0x1c7e)
#define DEVICE_ID		(0xb100)
#define SUBVENDOR_ID		(0x1c7e)
#define SUBDEVICE_ID		(0x0001)
#define EDGX_PCI_BR_BAR		(0U)
#define EDGX_PCI_BR_OFFS	(0x0)
#define EDGX_PCI_BR_SIZE	(0x1200000)
#define EDGX_PCI_MDIO_BAR	(0U)
#define EDGX_PCI_MDIO_OFFS	(0x2000000)
#define EDGX_PCI_MDIO_SIZE	(0x800000)
#define EDGX_PCI_MDIO_MAP_SIZE	(0x1000)
#define EDGX_PCI_MDIO_COUNT	(2)

struct edgx_pci_drv {
	struct edgx_br *br;
	struct edgx_mdio *mdio[EDGX_PCI_MDIO_COUNT];
	struct edgx_br_irq irq;
};

static const struct pci_device_id edgx_pci_ids[] = {
	{PCI_DEVICE_SUB(VENDOR_ID, DEVICE_ID, SUBVENDOR_ID, SUBDEVICE_ID)},
	{}
};

static int bridge_id;
static int mdio_id;

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

	ret = pci_alloc_irq_vectors(pdev, EDGX_IRQ_CNT, EDGX_IRQ_CNT,
				    PCI_IRQ_MSI | PCI_IRQ_MSIX);
	if (ret < 0) {
		dev_err(&pdev->dev, "pci_alloc_irq_vectors failed! ret=%d\n",
			ret);
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
		dev_info(&pdev->dev, "IRQ-vector %d: %d\n", i, irq->irq_vec[i]);
	}

	return 0;
}

static int edgx_pci_probe(struct pci_dev *pdev, const struct pci_device_id *id)
{
	int i = 0;
	int ret;
	void *br_base;
	void *mdio_base[EDGX_PCI_MDIO_COUNT];
	struct edgx_pci_drv *pci_drv;

	ret = pci_enable_device(pdev);
	if (ret) {
		dev_err(&pdev->dev, "Cannot enable PCI device.\n");
		goto probe_out_enable;
	}

	pci_set_master(pdev);
	pr_info("pci_try_set_mwi: %d\n", pci_try_set_mwi(pdev));

	if (!(pci_resource_flags(pdev, 0) & IORESOURCE_MEM)) {
		dev_err(&pdev->dev, "Invalid BAR0 resource flags.\n");
		ret = -ENODEV;
		goto probe_out_res_flag;
	}

	ret = pci_request_regions(pdev, DRV_NAME);
	if (ret) {
		dev_err(&pdev->dev, "Cannot get PCI resource.\n");
		goto probe_out_res_flag;
	}

	br_base = pci_iomap_range(pdev, EDGX_PCI_BR_BAR,
				  EDGX_PCI_BR_OFFS, EDGX_PCI_BR_SIZE);
	if (!br_base) {
		dev_err(&pdev->dev, "Cannot map bridge device memory.\n");
		ret = -ENOMEM;
		goto probe_out_iomap_br;
	}

	pci_drv = kzalloc(sizeof(*pci_drv), GFP_KERNEL);
	if (!pci_drv) {
		ret = -ENOMEM;
		goto probe_out_drv_alloc;
	}

	ret = edgx_pci_get_irq(pdev, &pci_drv->irq);
	if (ret)
		goto probe_out_irq;

	for (i = 0; i < EDGX_PCI_MDIO_COUNT; i++) {
		mdio_base[i] = pci_iomap_range(pdev, EDGX_PCI_MDIO_BAR,
					       EDGX_PCI_MDIO_OFFS
					       + i * EDGX_PCI_MDIO_SIZE,
					       EDGX_PCI_MDIO_MAP_SIZE);
		if (!mdio_base[i]) {
			dev_err(&pdev->dev, "Cannot map MDIO device memory.\n");
			ret = -ENOMEM;
			goto probe_out_iomap_mdio;
		}
		ret = edgx_mdio_probe_one(mdio_id, &pdev->dev, mdio_base[i],
					  &pci_drv->mdio[i]);
		if (ret) {
			pci_iounmap(pdev, mdio_base[i]);
			ret = -ENOMEM;
			goto probe_out_iomap_mdio;
		}
		mdio_id++;
	}

	ret = edgx_br_probe_one(bridge_id, &pdev->dev, br_base, &pci_drv->irq,
				&pci_drv->br, edgx_pci_driver.name);
	if (ret)
		goto probe_out_bridge;
	bridge_id++;

	dev_set_drvdata(&pdev->dev, pci_drv);

	return ret;

probe_out_bridge:
probe_out_iomap_mdio:
	while (i-- > 0) {
		edgx_mdio_shutdown(pci_drv->mdio[i]);
		pci_iounmap(pdev, mdio_base[i]);
		mdio_id--;
	}
	pci_free_irq_vectors(pdev);
probe_out_irq:
	kfree(pci_drv);
probe_out_drv_alloc:
	pci_iounmap(pdev, br_base);
probe_out_iomap_br:
	pci_release_regions(pdev);
probe_out_res_flag:
	pci_disable_device(pdev);
probe_out_enable:
	return ret;
}

static void edgx_pci_remove(struct pci_dev *pdev)
{
	struct edgx_pci_drv *pci_drv = dev_get_drvdata(&pdev->dev);
	void *br_base;
	void *mdio_base;

	if (!pci_drv)
		return;

	br_base = edgx_br_get_base(pci_drv->br);

	edgx_br_shutdown(pci_drv->br);
	pci_free_irq_vectors(pdev);
	for (int i = 0; i < EDGX_PCI_MDIO_COUNT; i++) {
		mdio_base = edgx_mdio_get_base(pci_drv->mdio[i]);
		edgx_mdio_shutdown(pci_drv->mdio[i]);
		pci_iounmap(pdev, mdio_base);
	}
	kfree(pci_drv);
	pci_iounmap(pdev, br_base);
	pci_release_regions(pdev);
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
MODULE_AUTHOR("TTTech Computertechnik AG <support@4me.tttech-dependablenetworks.com>");
MODULE_DESCRIPTION("EDGE PCIe Host Interface Driver");
MODULE_DEVICE_TABLE(pci, edgx_pci_ids);
MODULE_VERSION(EDGX_SW_CORE_VERSION);
