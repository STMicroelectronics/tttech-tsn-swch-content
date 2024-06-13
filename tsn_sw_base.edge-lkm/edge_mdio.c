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

#include <linux/kernel.h>
#include <linux/mdio.h>
#include <linux/phy.h>
#include <linux/of_mdio.h>

#include "edge_mdio.h"

#define _MDIO_ADDR               0x84
#define _MDIO_ADDR_DEV_SHIFT     0
#define _MDIO_ADDR_DEV_MASK      0x1F
#define _MDIO_ADDR_PORT_SHIFT    8
#define _MDIO_ADDR_PORT_MASK     0x1F
#define _MDIO_ADDR_REG_SHIFT     16
#define _MDIO_ADDR_REG_MASK      0xFFFF

struct edgx_mdio {
	struct list_head  entry;
	struct mii_bus   *mii;
	void             *base;
	struct mutex      lock; /* MDIO read/write access lock */
	unsigned int      id;
};

static int edgx_mdio_read(struct mii_bus *bus, int addr, int regnum)
{
	int ret;
	struct edgx_mdio *mdio = bus->priv;

	mutex_lock(&mdio->lock);
	/* Set PHY address. */
	iowrite32((addr & _MDIO_ADDR_DEV_MASK) <<
		  _MDIO_ADDR_DEV_SHIFT,
		  mdio->base + _MDIO_ADDR);
	/* Read value, but NOT from documented MDIO_ACCESS register. */
	ret = ioread16(mdio->base + (regnum << 2));
	mutex_unlock(&mdio->lock);

	return ret;
}

int edgx_mdio_write(struct mii_bus *bus, int addr, int regnum, u16 val)
{
	struct edgx_mdio *mdio = bus->priv;

	mutex_lock(&mdio->lock);
	/* Set PHY address. */
	iowrite32((addr & _MDIO_ADDR_DEV_MASK) <<
		  _MDIO_ADDR_DEV_SHIFT,
		  mdio->base + _MDIO_ADDR);
	/* Write value, but NOT to documented MDIO_ACCESS register. */
	iowrite16(val, mdio->base + (regnum << 2));
	mutex_unlock(&mdio->lock);

	return 0;
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
