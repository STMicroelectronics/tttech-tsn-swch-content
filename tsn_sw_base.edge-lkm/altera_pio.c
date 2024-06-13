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

#define DRV_NAME                "flx_pio"

/// PIO data register
#define FLX_PIO_DATA_REG        0x0

/// PIO direction register
#define FLX_PIO_DIR_REG         0x4

/// PIO interrupt mask register
#define FLX_PIO_IRQ_MASK_REG    0x8

/// PIO edge capture register
#define FLX_PIO_EDGE_CAPTURE_REG        0xc

/// PIO value set register to set individual output bits
#define FLX_PIO_SET_REG         0x10

/// PIO value set register to clear individual output bits
#define FLX_PIO_CLR_REG         0x14

#include <linux/device.h>
#include <linux/gpio/driver.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/of.h>
#include "altera_pio.h"

/**
 * PIO device context
 */
struct flx_pio_dev_priv {
	struct device *dev;
	struct list_head list;      ///< linked list
	unsigned int width;         ///< Number of I/O on device
	struct gpio_chip gpio_chip; ///< GPIO chip for PIO
	u32 dir;                    ///< Current PIO direction bits

	/* Serialize dir changes */
	spinlock_t lock;            ///< Serialize dir changes
	void __iomem *ioaddr;       ///< Memory mapped register I/O
};

/**
 * GPIO chip interface: set GPIO direction as input
 */
static int flx_pio_gpio_direction_input(struct gpio_chip *chip,
					unsigned int offset)
{
	struct flx_pio_dev_priv *dp = container_of(chip,
						   struct flx_pio_dev_priv,
						   gpio_chip);

	spin_lock_bh(&dp->lock);

	dp->dir &= ~(1u << offset);
	iowrite32(dp->dir, dp->ioaddr + FLX_PIO_DIR_REG);

	spin_unlock_bh(&dp->lock);

	dev_info(dp->dev, "DIR %u input  0x%08x DIR 0x%08x DATA %08x\n",
		 offset, dp->dir,
		 ioread32(dp->ioaddr + FLX_PIO_DIR_REG),
		 ioread32(dp->ioaddr + FLX_PIO_DATA_REG));

	return 0;
}

/**
 * GPIO chip interface: set GPIO direction as output
 */
static int flx_pio_gpio_direction_output(struct gpio_chip *chip,
					 unsigned int offset, int value)
{
	struct flx_pio_dev_priv *dp = container_of(chip,
						   struct flx_pio_dev_priv,
						   gpio_chip);

	spin_lock_bh(&dp->lock);

	if (value)
		iowrite32(1u << offset, dp->ioaddr + FLX_PIO_SET_REG);
	else
		iowrite32(1u << offset, dp->ioaddr + FLX_PIO_CLR_REG);

	dp->dir |= 1u << offset;
	iowrite32(dp->dir, dp->ioaddr + FLX_PIO_DIR_REG);

	spin_unlock_bh(&dp->lock);

	dev_info(dp->dev, "DIR %u output 0x%08x DIR 0x%08x DATA %08x\n",
		 offset, dp->dir,
		 ioread32(dp->ioaddr + FLX_PIO_DIR_REG),
		 ioread32(dp->ioaddr + FLX_PIO_DATA_REG));

	return 0;
}

/**
 * GPIO chip interface: Get GPIO value
 */
static int flx_pio_gpio_get(struct gpio_chip *chip, unsigned int offset)
{
	struct flx_pio_dev_priv *dp = container_of(chip,
						   struct flx_pio_dev_priv,
						   gpio_chip);
	u32 tmp;

	tmp = ioread32(dp->ioaddr + FLX_PIO_DATA_REG);

	dev_info(dp->dev, "GET %u value %u DIR 0x%08x DATA %08x\n",
		 offset, (tmp & (1u << offset)),
		 ioread32(dp->ioaddr + FLX_PIO_DIR_REG),
		 ioread32(dp->ioaddr + FLX_PIO_DATA_REG));

	return (tmp & (1u << offset));
}

/**
 * GPIO chip interface: set GPIO value
 */
static void flx_pio_gpio_set(struct gpio_chip *chip,
			     unsigned int offset, int value)
{
	struct flx_pio_dev_priv *dp = container_of(chip,
						   struct flx_pio_dev_priv,
						   gpio_chip);

	if (value)
		iowrite32(1u << offset, dp->ioaddr + FLX_PIO_SET_REG);
	else
		iowrite32(1u << offset, dp->ioaddr + FLX_PIO_CLR_REG);

	dev_info(dp->dev, "SET %u to %u DIR 0x%08x DATA %08x\n",
		 offset, value,
		 ioread32(dp->ioaddr + FLX_PIO_DIR_REG),
		 ioread32(dp->ioaddr + FLX_PIO_DATA_REG));
}

/**
 * Turn PIO device to GPIOs.
 * @param pdev Platform_device providing PIO
 */
int flx_pio_probe_one(unsigned int pio_id, struct device *dev, void *base,
		      struct flx_pio_dev_priv **pio_ret)
{
	int ret = -ENXIO;
	struct flx_pio_dev_priv *dp = NULL;
	struct gpio_chip *chip = NULL;
	u32 value = 32;

	dev_info(dev, "Init PIO dev %i\n", pio_id);

	dp = kmalloc(sizeof(*dp), GFP_KERNEL);
	if (!dp) {
		ret = -ENOMEM;
		goto err_alloc_priv;
	}
	dp->dev = dev;
	dp->ioaddr = NULL;
	INIT_LIST_HEAD(&dp->list);
	spin_lock_init(&dp->lock);

#ifdef CONFIG_OF
	ret = of_property_read_u32(dev->of_node, "width", &value);
#else
	value = 32;
	ret = 0;
#endif
	if (ret) {
		dev_info(dev, "Missing width\n");
		ret = -EINVAL;
		goto err_width;
	} else {
		dp->width = value;
	}

	dp->ioaddr = base;

	// Direction register is not correct if PIO is fixed to output.
#ifdef CONFIG_OF
	ret = of_property_read_u32(dev->of_node, "direction", &value);
	if (ret == 0)
		dp->dir = value;
#else
	ret = -ENOENT;
#endif
	if (ret)
		dp->dir = ioread32(dp->ioaddr + FLX_PIO_DIR_REG);

	chip = &dp->gpio_chip;

	*chip = (struct gpio_chip) {
		.label = DRV_NAME,
		.parent = dev,
		.direction_input = &flx_pio_gpio_direction_input,
		.direction_output = &flx_pio_gpio_direction_output,
		.get = &flx_pio_gpio_get,
		.set = &flx_pio_gpio_set,
		// Allocate GPIO numbers dynamically
		.base = -1,
		.ngpio = dp->width,
		.can_sleep = 0,
	};

	dev_info(dev, "Adding gpio chip\n");

	ret = gpiochip_add(chip);
	if (ret) {
		dev_err(dev, "Failed to add gpio chip\n");
		goto err_gpiochip_add;
	}

	dev_info(dev, "Added gpio base %i\n", chip->base);
	dev_info(dev, "Added PIO device GPIOs %u .. %u\n",
		 chip->base, chip->base + chip->ngpio - 1);

	*pio_ret = dp;

	/* FIXME: workaround for PHY reset */
	flx_pio_gpio_direction_output(chip, 0, 0);
	flx_pio_gpio_set(chip, 0, 0);
	msleep(300);
	flx_pio_gpio_set(chip, 0, 1);
	msleep(300);

	return 0;

err_gpiochip_add:
err_width:
	kfree(dp);
err_alloc_priv:
	return ret;
}

/**
 * Cleanup GPIO device for PIO.
 * @param pdev Platform device of PIO.
 */
void flx_pio_shutdown(struct flx_pio_dev_priv *dp)
{
	if (!dp)
		return;

	dev_info(dp->dev, "Removing GPIO chip GPIOs %u .. %u\n",
		 dp->gpio_chip.base,
		 dp->gpio_chip.base + dp->gpio_chip.ngpio - 1);

	gpiochip_remove(&dp->gpio_chip);
	kfree(dp);
}

void *flx_pio_get_base(struct flx_pio_dev_priv *dp)
{
	if (!dp)
		return NULL;

	return dp->ioaddr;
}
