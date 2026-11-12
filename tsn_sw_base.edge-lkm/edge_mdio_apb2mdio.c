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

#include <linux/kernel.h>
#include <linux/mdio.h>
#include <linux/phy.h>
#include <linux/of_mdio.h>
#include <linux/bitfield.h>

#include "edge_mdio.h"

#define _MDIO_CTRL                      0x0
#define _MDIO_CTRL_WAIT                 0x4
#define _MDIO_CTRL_DATA                 GENMASK(15, 0)
#define _MDIO_CTRL_REG                  GENMASK(20, 16)
#define _MDIO_CTRL_DEV                  GENMASK(25, 21)
#define _MDIO_CTRL_IGNORED              BIT(28)
#define _MDIO_CTRL_READY                BIT(29)
#define _MDIO_CTRL_READ_ERR             BIT(30)
#define _MDIO_CTRL_WRITE                BIT(31)
#define _MDIO_CTRL_READ                 0

#define _MDIO_CFG                       0x400
#define _MDIO_CFG_CLK_DIV               GENMASK(15, 0)
#define _MDIO_CFG_CLK_PERIOD            GENMASK(23, 16)
#define _MDIO_CFG_LONG_TURNAROUND       BIT(25)
#define _MDIO_CFG_DISABLE_PREAMBLE      BIT(31)

#define _MDIO_MDC_FREQ_KHZ              100
/* ~ 65 cycles, 100 kHz clock -> 650 us, 400 kHz clock -> 163 us */
#define _MDIO_WAIT_MIN_US               ((65 * 1000) / _MDIO_MDC_FREQ_KHZ)
#define _MDIO_WAIT_MAX_US               (_MDIO_WAIT_MIN_US + 100)

struct edgx_mdio {
	struct list_head  entry;
	struct mii_bus   *mii;
	void             *base;
	struct mutex      lock; /* MDIO read/write access lock */
	unsigned int      id;
};

static int edgx_mdio_wait(struct edgx_mdio *mdio, u32 *ctrl_ret)
{
	unsigned int timeout = 1000;
	int ret = 0;
	u32 ctrl;

	usleep_range(_MDIO_WAIT_MIN_US, _MDIO_WAIT_MAX_US);

	ctrl = ioread32(mdio->base + _MDIO_CTRL);
	while (!(ctrl & _MDIO_CTRL_READY)) {
		cpu_relax();
		ctrl = ioread32(mdio->base + _MDIO_CTRL);
		if (timeout-- == 0) {
			ret = -EIO;
			break;
		}
	}

	if (ctrl_ret)
		*ctrl_ret = ctrl;

	return ret;
}

static int edgx_mdio_init(struct edgx_mdio *mdio)
{
	/*
	 * mdc-period-ns = mdio-clk-period-ns x clk-div x 2
	 * clk-div       = mdc-period-ns / (mdio-clk-period-ns x 2)
	 *               = 1000000 / (mdc-clk-freq-khz x mdio-clk-period-ns x 2)
	 */
	u32 cfg = ioread32(mdio->base + _MDIO_CFG);
	u32 clk_period_ns = FIELD_GET(_MDIO_CFG_CLK_PERIOD, cfg);
	u32 mdio_div;
	int ret;

	if (clk_period_ns == 0)
		return -EIO;

	mdio_div = 1000000 / (2 * clk_period_ns * _MDIO_MDC_FREQ_KHZ);
	cfg = FIELD_PREP(_MDIO_CFG_CLK_DIV, mdio_div);

	iowrite32(cfg, mdio->base + _MDIO_CFG);
	ret = edgx_mdio_wait(mdio, NULL);
	return ret;
}

static int edgx_mdio_read(struct mii_bus *bus, int addr, int regnum)
{
	struct edgx_mdio *mdio = bus->priv;
	u32 ctrl = 0;
	int ret;

	mutex_lock(&mdio->lock);
	iowrite32(FIELD_PREP(_MDIO_CTRL_DEV, addr) |
		  FIELD_PREP(_MDIO_CTRL_REG, regnum) |
		  _MDIO_CTRL_READ,
		  mdio->base + _MDIO_CTRL);
	ret = edgx_mdio_wait(mdio, &ctrl);
	mutex_unlock(&mdio->lock);

	if (ret)
		return ret;

	if (ctrl & _MDIO_CTRL_READ_ERR)
		return -EIO;

	return FIELD_GET(_MDIO_CTRL_DATA, ctrl);
}

static int edgx_mdio_write(struct mii_bus *bus, int addr, int regnum, u16 val)
{
	struct edgx_mdio *mdio = bus->priv;
	int ret;

	mutex_lock(&mdio->lock);
	iowrite32(FIELD_PREP(_MDIO_CTRL_DEV, addr) |
		  FIELD_PREP(_MDIO_CTRL_REG, regnum) |
		  FIELD_PREP(_MDIO_CTRL_DATA, val) |
		  _MDIO_CTRL_WRITE,
		  mdio->base + _MDIO_CTRL);
	ret = edgx_mdio_wait(mdio, NULL);
	mutex_unlock(&mdio->lock);

	return ret;
}

int edgx_mdio_probe_one(unsigned int mdio_id, struct device *dev, void *base,
			struct edgx_mdio **mdio_ret)
{
	int ret = -ENOMEM;
	struct edgx_mdio *mdio = kzalloc(sizeof(*mdio), GFP_KERNEL);
	struct device_node *np = dev->of_node;

	if (!mdio)
		return -ENOMEM;

	mdio->mii = mdiobus_alloc();
	if (!mdio->mii)
		goto out_bus;

	mdio->id   = mdio_id;
	mdio->base = base;
	mutex_init(&mdio->lock);

	mdio->mii->phy_mask = 0;
	mdio->mii->read     = &edgx_mdio_read;
	mdio->mii->write    = &edgx_mdio_write;
	mdio->mii->parent   = dev;
	mdio->mii->priv     = mdio;
	mdio->mii->name     = "edgx_mdio";
	snprintf(mdio->mii->id, MII_BUS_ID_SIZE, "%s-%x",
		 mdio->mii->name, mdio_id + 1);

	ret = edgx_mdio_init(mdio);
	if (ret) {
		dev_err(dev, "Failed to initialize MDIO\n");
		goto out_mdio_init;
	}

	if (np)
		/*
		 * try devicetree based phy registration to provide possible
		 * devicetree configuration data to the phys
		 */
		ret = of_mdiobus_register(mdio->mii, np);
	else
		ret = mdiobus_register(mdio->mii);

	if (ret)
		goto out_reg;


	*mdio_ret = mdio;
	return 0;

out_mdio_init:
out_reg:
	mdiobus_free(mdio->mii);
out_bus:
	kfree(mdio);
	return ret;
}

void edgx_mdio_shutdown(struct edgx_mdio *mdio)
{
	if (!mdio)
		return;

	mdiobus_unregister(mdio->mii);
	mdiobus_free(mdio->mii);
	kfree(mdio);
}

void *edgx_mdio_get_base(struct edgx_mdio *mdio)
{
	if (!mdio)
		return NULL;

	return mdio->base;
}

const char *edgx_mdio_get_id(struct edgx_mdio *mdio)
{
	return mdio->mii->id;
}
