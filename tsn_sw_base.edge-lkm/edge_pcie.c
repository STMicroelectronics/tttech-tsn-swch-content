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

#include "edge_bridge.h"
#include "edge_port.h"
#include "edge_link.h"
#include "edge_mdio.h"
#include "altera_pio.h"

#define PFX			"EDGX-PCIe: "
#define DRV_NAME		"edgx-pcie"
#define VENDOR_ID		(0x1c7e)
#define DEVICE_ID		(0xa101)
#define SUBVENDOR_ID		(0x1059)
#define SUBDEVICE_ID		(0x0000)
#define EDGX_PCI_BR_BAR		(0U)
#define EDGX_PCI_BR_OFFS	(0x6000000)
#define EDGX_PCI_BR_SIZE	(0x1ffffff)
#define EDGX_PCI_MDIO_BAR	(1U)
#define EDGX_PCI_MDIO_OFFS	(0x0)
#define EDGX_PCI_MDIO_SIZE	(0x3ff)
#define EDGX_PCI_PIO_BAR	(2U)
#define EDGX_PCI_PIO_OFFS	(0xf0f00)
#define EDGX_PCI_PIO_SIZE	(0x1f)

struct edgx_pci_drv {
	struct edgx_br *br;
	struct edgx_mdio *mdio;
	struct flx_pio_dev_priv *pio;
	struct edgx_br_irq irq;
};

static const struct pci_device_id edgx_pci_ids[] = {
	{PCI_DEVICE_SUB(VENDOR_ID, DEVICE_ID, SUBVENDOR_ID, SUBDEVICE_ID)},
	{}
};

static int bridge_id;
static int mdio_id;
static int pio_id;

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
		dev_err(&pdev->dev, "pci_alloc_irq_vectors failed! ret=%d\n", ret);
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

static int edgx_pci_set_delays(struct edgx_pci_drv *pci_drv, struct device *dev,
			       ptid_t pt_id, int mdio_id, bool set_mdio,
			       unsigned int dtx10min, unsigned int dtx10max,
			       unsigned int dtx100min, unsigned int dtx100max,
			       unsigned int dtx1000min, unsigned int dtx1000max,
			       unsigned int drx10min, unsigned int drx10max,
			       unsigned int drx100min, unsigned int drx100max,
			       unsigned int drx1000min, unsigned int drx1000max)
{
	const char *mdio_bus_id;
	struct edgx_link *lnk;
	struct edgx_pt *pt;
	char mdio_bus_id_pt[MII_BUS_ID_SIZE + 4];

	pt =  edgx_br_get_brpt(pci_drv->br, pt_id);
	if (!pt) {
		dev_err(dev, "Cannot get port %d.\n", pt_id);
		return -EINVAL;
	}

	pr_info("%s: pt_id: %d, pt-name: %s\n", __func__, pt_id, edgx_pt_get_name(pt));

	lnk = edgx_pt_get_link(pt);
	if (!pt) {
		dev_err(dev, "Cannot get port %d link.\n", pt_id);
		return -EINVAL;
	}

	if (set_mdio) {
		mdio_bus_id = edgx_mdio_get_id(pci_drv->mdio);
		snprintf(mdio_bus_id_pt, MII_BUS_ID_SIZE, "%s:%02d",
			 mdio_bus_id, mdio_id);

		if (edgx_link_set_mdiobus(lnk, mdio_bus_id_pt)) {
			dev_err(dev, "Cannot set port %d link.\n", pt_id);
			return -EINVAL;
		}
	}

	edgx_link_set_delays(lnk, dtx10min, dtx10max, dtx100min, dtx100max,
			     dtx1000min, dtx1000max, drx10min, drx10max,
			     drx100min, drx100max, drx1000min, drx1000max);
	return 0;
}

static int edgx_pci_probe(struct pci_dev *pdev, const struct pci_device_id *id)
{
	int ret, i;
	void *br_base;
	void *mdio_base;
	void *pio_base;
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

	if (!(pci_resource_flags(pdev, 1) & IORESOURCE_MEM)) {
		dev_err(&pdev->dev, "Invalid BAR1 resource flags.\n");
		ret = -ENODEV;
		goto probe_out_res_flag;
	}

	if (!(pci_resource_flags(pdev, 2) & IORESOURCE_MEM)) {
		dev_err(&pdev->dev, "Invalid BAR2 resource flags.\n");
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

	mdio_base = pci_iomap_range(pdev, EDGX_PCI_MDIO_BAR,
				    EDGX_PCI_MDIO_OFFS, EDGX_PCI_MDIO_SIZE);
	if (!mdio_base) {
		dev_err(&pdev->dev, "Cannot map MDIO device memory.\n");
		ret = -ENOMEM;
		goto probe_out_iomap_mdio;
	}

	pio_base = pci_iomap_range(pdev, EDGX_PCI_PIO_BAR,
				   EDGX_PCI_PIO_OFFS, EDGX_PCI_PIO_SIZE);
	if (!pio_base) {
		dev_err(&pdev->dev, "Cannot map PIO device memory.\n");
		ret = -ENOMEM;
		goto probe_out_iomap_pio;
	}

	pci_drv = kzalloc(sizeof(*pci_drv), GFP_KERNEL);
	if (!pci_drv) {
		ret = -ENOMEM;
		goto probe_out_drv_alloc;
	}

	ret = edgx_pci_get_irq(pdev, &pci_drv->irq);
	if (ret)
		goto probe_out_irq;

	ret = flx_pio_probe_one(pio_id, &pdev->dev, pio_base,
				&pci_drv->pio);
	if (ret)
		goto probe_out_pio;
	pio_id++;

	ret = edgx_mdio_probe_one(mdio_id, &pdev->dev, mdio_base,
				  &pci_drv->mdio);
	if (ret)
		goto probe_out_mdio;
	mdio_id++;

	ret = edgx_br_probe_one(bridge_id, &pdev->dev, br_base, &pci_drv->irq,
				&pci_drv->br, edgx_pci_driver.name);
	if (ret)
		goto probe_out_bridge;
	bridge_id++;

	dev_set_drvdata(&pdev->dev, pci_drv);

	ret = edgx_pci_set_delays(pci_drv, &pdev->dev, 0, 0, false,
				  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
	 if (ret)
		 goto probe_out_dly;
	 edgx_pt_set_delay_adjust(edgx_br_get_brpt(pci_drv->br, 0),
				  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);

	/* Min delays for Marvell 88E1510P - standard latency mode */
	for (i = 0; i < 3; i++) {
		 ret = edgx_pci_set_delays(pci_drv, &pdev->dev, i + 1, i, true,
				     4032, 4832,
				     412, 492,
				     109, 135,
				     1083, 1183,
				     220, 220,
				     203, 211);
		 if (ret)
			 goto probe_out_dly;
		 edgx_pt_set_delay_adjust(edgx_br_get_brpt(pci_drv->br, i + 1),
					  0, 0,
					  -600, 0,
					  -250, 0,
					  0, 0,
					  -100, 0,
					  -50, 0);
	}

	return ret;

probe_out_dly:
	edgx_br_shutdown(pci_drv->br);
	bridge_id--;
probe_out_bridge:
	edgx_mdio_shutdown(pci_drv->mdio);
	mdio_id--;
probe_out_mdio:
	flx_pio_shutdown(pci_drv->pio);
	pio_id--;
probe_out_pio:
	pci_free_irq_vectors(pdev);
probe_out_irq:
	kfree(pci_drv);
probe_out_drv_alloc:
	pci_iounmap(pdev, pio_base);
probe_out_iomap_pio:
	pci_iounmap(pdev, mdio_base);
probe_out_iomap_mdio:
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
	void *pio_base;

	if (!pci_drv)
		return;

	br_base = edgx_br_get_base(pci_drv->br);
	mdio_base = edgx_mdio_get_base(pci_drv->mdio);
	pio_base = flx_pio_get_base(pci_drv->pio);

	edgx_br_shutdown(pci_drv->br);
	edgx_mdio_shutdown(pci_drv->mdio);
	flx_pio_shutdown(pci_drv->pio);
	pci_free_irq_vectors(pdev);
	kfree(pci_drv);
	pci_iounmap(pdev, pio_base);
	pci_iounmap(pdev, mdio_base);
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
MODULE_AUTHOR("TTTech Industrial Automation AG <support@tttech-industrial.com>");
MODULE_DESCRIPTION("EDGE PCIe Host Interface Driver");
MODULE_DEVICE_TABLE(pci, edgx_pci_ids);
MODULE_VERSION(EDGX_SW_CORE_VERSION);
