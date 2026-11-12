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

#include <linux/version.h>
#include <linux/sysfs.h>
#include <linux/netdevice.h>
#include <linux/i2c.h>
#include <linux/workqueue.h>
#include <linux/phy.h>
#include <linux/phylink.h>
#include <linux/mdio/mdio-i2c.h>

#include "edge_ac.h"
#include "edge_link.h"
#include "edge_util.h"

#define _SFP_ETH_10G_COMPL		0x3
#define _ETH_10G_10000BASE_SR		BIT(4) /* bit 4 set -> 10GBASE-SR */

#define _SFP_ETH_COMPL			0x6
#define _ETH_CMPL_1000BASET		BIT(3) /* bit 3 set -> 1000BASE-T */

#define _SFP_EXT_COMPL			0x24
#define _EXT_CMPL_10000BASET_SR		0x1c /* 10GBASE-T Short Reach */
#define _EXT_CMPL_5000BASET		0x1d /* 5GBASE-T */
#define _EXT_CMPL_25000BASET		0x1e /* 2.5GBASE-T */

#define _ADPT_GMII_ID			0xB0
#define _ADPT_RGMII_ID			0xB1
#define _ADPT_RMII_ID			0xB5
#define _ADPT_TRIPLE_ID			0xB9
#define _ADPT_DMA_ID			0xBC
#define _ADPT_NONE_ID			0xBB
#define _ADPT_PCS_PMA_ID		0xBD

#define _ADPT_REG_ID			0x0
#define _ADPT_REG_RXDLY			0x20
#define _ADPT_REG_TXDLY			0x22

#define _ADPT_LINK_STS			0x2
#define _ADPT_PCS_CTL			0x10

#define _PCS_CTL_ANEG			(0x111)
#define _LINK_STATUS_HW_CTL		(1U)

#define _PCS_PMA_REG_CFG_TX		0x1000c
#define _PCS_PMA_SPD_SEL_10G		0x2	/* bits 7:6 */
#define _PCS_PMA_SPD_SEL_1G		0x1	/* bits 7:6 */

#define _PCS_PMA_REG_CFG_VEC		0x11000
#define _PCS_PMA_CFG_VEC_1G_AN_ENA	BIT(4)

#define _PCS_PMA_REG_RESET		0x10000
#define _PCS_PMA_RESET_ALL		BIT(0)

#define _SFP_CHECK_PERIOD		(2 * HZ)
#define _T_PHY_RESET_MS			50U
/* SFP modules appear to always have their PHY configured for bus address
 * 0x56 (which with mdio-i2c, translates to a PHY address of 22).
 */
#define _SFP_PHY_ADDR			22

static __ETHTOOL_DECLARE_LINK_MODE_MASK(_SPEED_CAPS_MASK);
static __ETHTOOL_DECLARE_LINK_MODE_MASK(_SUPPORTED_DUAL_ADAPTER);
static __ETHTOOL_DECLARE_LINK_MODE_MASK(_SUPPORTED_PCS_PMA_ADAPTER);

static const unsigned long *_edgx_link_adjust_triple(struct edgx_link *lnk);
static const unsigned long *_edgx_link_adjust_pcs_pma(struct edgx_link *lnk);

enum _delay_dir {
	_DLY_TX = 0,
	_DLY_RX,
	_NDLYDIRS, /* must be last */
};

enum _delay_min_max {
	_DLY_MIN = 0,
	_DLY_MAX,
	_NDLYMX, /* must be last */
};

enum _adpt_mode {
	_ADPT_MODE_1000BASE_X = 0,
	_ADPT_MODE_SGMII,
	_ADPT_MODE_100BASE_FX,
	_ADPT_MODE_UNKNOWN
};

struct _adpt_desc {
	const char	*name;
	u16              id;
	phy_interface_t  itf;

	/* returns the capabilities of current mode */
	const unsigned long *(*adjust)(struct edgx_link *link);

};

static const struct _adpt_desc _adpt_desc[] = {
	{ .name = "GMII/MII", .id = _ADPT_GMII_ID,
	  .itf = PHY_INTERFACE_MODE_GMII, },
	{ .name = "RGMII", .id = _ADPT_RGMII_ID,
	  .itf = PHY_INTERFACE_MODE_RGMII_ID, },
	{ .name = "RMII", .id = _ADPT_RMII_ID,
	  .itf = PHY_INTERFACE_MODE_RMII, },
	{ .name = "Triple Mode", .id = _ADPT_TRIPLE_ID,
	  .itf = PHY_INTERFACE_MODE_SGMII,
	  .adjust = _edgx_link_adjust_triple },
	{ .name = "PCS/PMA 10G/1G", .id = _ADPT_PCS_PMA_ID,
	  /* SGMII used in 1G mode */
	  .itf = PHY_INTERFACE_MODE_SGMII,
	  .adjust = _edgx_link_adjust_pcs_pma },
};

struct edgx_adpt {
	edgx_io_t               *iobase;
	const struct _adpt_desc *desc;
};

enum _sfp_state {
	SFP_INIT = 0,
	SFP_CONFIGURED,
	SFP_NOT_PRESENT,
	SFP_UNKNOWN
};

struct edgx_link {
	struct edgx_pt    *parent;
	struct phy_device *phydev;
	int                speed;
	__ETHTOOL_DECLARE_LINK_MODE_MASK(supported);
	struct edgx_adpt   adpt;

	struct {
		struct i2c_client  *eeprom;
		struct mii_bus	   *mii_bus;
		struct i2c_adapter *i2c_bus;
		enum _sfp_state	    state;
		edgx_speed_mask_t   spd_mask;
	} sfp;

	/* Note: To be super-safe and super-consistent here, we'd need a
	 *       double-buffer here, but that would be overkill.
	 *       Effects are negligible.
	 */
	unsigned int phy_dly[_NDLYDIRS][EDGX_SPD_CNT][_NDLYMX];
	unsigned int adpt_dly[_NDLYDIRS][EDGX_SPD_CNT];
	u32 dly_flags[_NDLYDIRS];
	edgx_speed_mask_t  spd_mask;

	struct delayed_work	 work_lnk;
	/* Synchronize access to sfp */
	struct mutex		lock;
};

#define EDGX_DLY_BIT(spd_idx, limit)    BIT((spd_idx) * _NDLYMX + (limit))

#define EDGX_DLY_ID_BITS(spd_idx)       (EDGX_DLY_BIT((spd_idx), _DLY_MIN) |   \
					 EDGX_DLY_BIT((spd_idx), _DLY_MAX))

static const struct i2c_board_info _sfp_eeprom = {
	I2C_BOARD_INFO("EEPROM", 0xA0 >> 1),
};

static const unsigned long *_edgx_link_adjust_triple(struct edgx_link *lnk)
{
	/* Link status (affects LEDs) HW controlled */
	edgx_set16(lnk->adpt.iobase, _ADPT_LINK_STS, 1, 1, _LINK_STATUS_HW_CTL);
	/* Adapter to auto negotiation; enabled bit 0,4,8 = 1 */
	edgx_wr16(lnk->adpt.iobase, _ADPT_PCS_CTL, _PCS_CTL_ANEG);

	return _SUPPORTED_DUAL_ADAPTER;
}

static const unsigned long *_edgx_link_adjust_pcs_pma(struct edgx_link *lnk)
{
	u32 spd_sel = 0;

	if (lnk->sfp.spd_mask & BIT(EDGX_SPD_10000))
		spd_sel = _PCS_PMA_SPD_SEL_10G;
	else
		spd_sel = _PCS_PMA_SPD_SEL_1G;
	edgx_set32(lnk->adpt.iobase, _PCS_PMA_REG_CFG_TX, 7, 6, spd_sel);
	if (spd_sel == _PCS_PMA_SPD_SEL_1G)
		edgx_wr32(lnk->adpt.iobase, _PCS_PMA_REG_CFG_VEC,
			  _PCS_PMA_CFG_VEC_1G_AN_ENA);
	edgx_wr32(lnk->adpt.iobase, _PCS_PMA_REG_RESET, _PCS_PMA_RESET_ALL);
	return _SUPPORTED_PCS_PMA_ADAPTER;
}

int edgx_link_ioctl(struct net_device *netdev, struct ifreq *ifr, int cmd)
{
	struct edgx_link *lnk = edgx_net2link(netdev);

	if (lnk->phydev)
		return phy_mii_ioctl(lnk->phydev, ifr, cmd);

	return -ENODEV;
}

int edgx_link_get_ksettings(struct net_device *netdev,
			    struct ethtool_link_ksettings *cmd)
{
	struct edgx_link *lnk = edgx_net2link(netdev);

	if (lnk->phydev)
		return phy_ethtool_get_link_ksettings(netdev, cmd);

	linkmode_copy(cmd->link_modes.supported, lnk->supported);
	linkmode_zero(cmd->link_modes.advertising);
	linkmode_zero(cmd->link_modes.lp_advertising);
	cmd->base.speed            = lnk->speed;
	cmd->base.duplex           = DUPLEX_FULL;
	cmd->base.port             = PORT_MII;
	cmd->base.phy_address      = -1;
	cmd->base.autoneg          = AUTONEG_DISABLE;
	cmd->base.eth_tp_mdix_ctrl = 0;

	return 0;
}

int edgx_link_set_ksettings(struct net_device *netdev,
			    const struct ethtool_link_ksettings *cmd)
{
	struct edgx_link *lnk = edgx_net2link(netdev);

	if (lnk->phydev)
		return phy_ethtool_set_link_ksettings(netdev, cmd);

	/* No support if running on internal port (everything fixed) */
	return -EPERM;
}

int edgx_link_nway_reset(struct net_device *netdev)
{
	struct edgx_link *lnk = edgx_net2link(netdev);

	if (lnk->phydev)
		return phy_start_aneg(netdev->phydev);
	return -ENODEV;
}

static const enum ethtool_link_mode_bit_indices edgx_eth_link_modes[] = {
	[EDGX_SPD_10]    = ETHTOOL_LINK_MODE_10baseT_Full_BIT,
	[EDGX_SPD_100]   = ETHTOOL_LINK_MODE_100baseT_Full_BIT,
	[EDGX_SPD_1000]  = ETHTOOL_LINK_MODE_1000baseT_Full_BIT,
	[EDGX_SPD_2500]  = ETHTOOL_LINK_MODE_2500baseT_Full_BIT,
	[EDGX_SPD_5000]  = ETHTOOL_LINK_MODE_5000baseT_Full_BIT,
	[EDGX_SPD_10000] = ETHTOOL_LINK_MODE_10000baseT_Full_BIT,
};

int edgx_link_get_speed(struct edgx_link *lnk)
{
	// TODO
	if (!lnk) /* shouldn't happen */
		return SPEED_UNKNOWN;

	if (!lnk->phydev) /* no PHY associated, everything fixed. */
		return lnk->speed;

	/* ... if just one speed is advertised, return it ... */
	if (!lnk->phydev->link && lnk->phydev->autoneg) {
		int adv_spd_idx = -1;

		for (int spd_idx = 0; spd_idx < EDGX_SPD_CNT; spd_idx++) {
			if (linkmode_test_bit(edgx_eth_link_modes[spd_idx],
					      lnk->phydev->advertising)) {
				if (adv_spd_idx >= 0)
					return SPEED_UNKNOWN;
				adv_spd_idx = spd_idx;
			}
		}

		return edgx_br_speed(adv_spd_idx);
	}

	/* In all other cases (i.e., link up, or link down but autoneg also
	 * off), return PHY speed.
	 */
	return lnk->phydev->speed;
}

int edgx_link_get_state(struct edgx_link *lnk)
{
	/*
	 * no lnk : shouldn't happen -> DOWN
	 * no phydev : UP (for internal port)
	 *
	 * pyhdev (MDIO and copper SFP): use lnk->phydev->link
	 */
	if (!lnk)
		return 0;
	if (!lnk->phydev)
		return 0; /* TODO: Check what this means for fiber */
	return lnk->phydev->link;
}

void edgx_link_start(struct edgx_link *lnk)
{
	if (lnk->phydev)
		phy_start(lnk->phydev);
}

void edgx_link_stop(struct edgx_link *lnk)
{
	if (lnk->phydev)
		phy_stop(lnk->phydev);
}

static int _edgx_link_has_adpt(struct edgx_link *lnk)
{
	return (lnk && lnk->adpt.desc) ? 1 : 0;
}

static inline ktime_t _edgx_link_get_offline_delay(struct edgx_link *lnk,
						   enum _delay_dir dir,
						   enum _delay_min_max min_max,
						   int speed)
{
	int spd_idx = edgx_br_spd_idx(speed);

	if (spd_idx < 0)
		/* speed, and thus delay unknown - set to 0. */
		return ktime_set(0, 0);

	return ktime_set(0, lnk->phy_dly[dir][spd_idx][min_max] +
			 lnk->adpt_dly[dir][spd_idx]);
}

static inline ktime_t _edgx_link_get_delay(struct edgx_link *lnk,
					   enum _delay_dir dir,
					   enum _delay_min_max min_max)
{
	return _edgx_link_get_offline_delay(lnk, dir, min_max,
					    edgx_link_get_speed(lnk));
}

ktime_t edgx_link_get_tx_delay_min(struct edgx_link *lnk)
{
	return _edgx_link_get_delay(lnk, _DLY_TX, _DLY_MIN);
}

ktime_t edgx_link_get_tx_delay_max(struct edgx_link *lnk)
{
	return _edgx_link_get_delay(lnk, _DLY_TX, _DLY_MAX);
}

ktime_t edgx_link_get_rx_delay_min(struct edgx_link *lnk)
{
	return _edgx_link_get_delay(lnk, _DLY_RX, _DLY_MIN);
}

ktime_t edgx_link_get_rx_delay_max(struct edgx_link *lnk)
{
	return _edgx_link_get_delay(lnk, _DLY_RX, _DLY_MAX);
}

ktime_t edgx_link_get_offline_tx_delay_min(struct edgx_link *lnk, int speed)
{
	return  _edgx_link_get_offline_delay(lnk, _DLY_TX, _DLY_MIN, speed);
}

ktime_t edgx_link_get_offline_tx_delay_max(struct edgx_link *lnk, int speed)
{
	return  _edgx_link_get_offline_delay(lnk, _DLY_TX, _DLY_MAX, speed);
}

ktime_t edgx_link_get_offline_rx_delay_min(struct edgx_link *lnk, int speed)
{
	return  _edgx_link_get_offline_delay(lnk, _DLY_RX, _DLY_MIN, speed);
}

ktime_t edgx_link_get_offline_rx_delay_max(struct edgx_link *lnk, int speed)
{
	return  _edgx_link_get_offline_delay(lnk, _DLY_RX, _DLY_MAX, speed);
}

static inline bool edgx_link_delays_entered_spd(struct edgx_link *lnk,
						enum _delay_dir dir,
						int spd_idx)
{
	return !(lnk->spd_mask & BIT(spd_idx)) ||
	       (lnk->dly_flags[dir] & EDGX_DLY_ID_BITS(spd_idx));
}

static bool edgx_link_delays_entered(struct edgx_link *lnk,
				     enum _delay_dir dir)
{
	for (int spd_idx = 0; spd_idx < EDGX_SPD_CNT; spd_idx++)
		if (!edgx_link_delays_entered_spd(lnk, dir, spd_idx))
			return false;
	return true;
}

edgx_speed_mask_t edgx_link_get_speed_mask(struct edgx_link *lnk)
{
	return lnk->spd_mask;
}

int edgx_link_get_rx_delays_hi(struct edgx_link *lnk,
			       ktime_t *drx_min, ktime_t *drx_max)
{
	int speed = edgx_br_speed(fls(lnk->spd_mask) - 1);

	if (speed == SPEED_UNKNOWN)
		return -EINVAL;

	if (!edgx_link_delays_entered(lnk, _DLY_RX))
		return -EINVAL;

	*drx_min = _edgx_link_get_offline_delay(lnk, _DLY_RX, _DLY_MIN, speed);
	*drx_max = _edgx_link_get_offline_delay(lnk, _DLY_RX, _DLY_MAX, speed);
	return 0;
}

int edgx_link_get_speed_tx_delays(struct edgx_link *lnk, int speed,
				  ktime_t *dtx_min, ktime_t *dtx_max)
{
	int spd_idx;

	if (!edgx_link_delays_entered(lnk, _DLY_TX))
		return -EINVAL;

	spd_idx = edgx_br_spd_idx(speed);
	if (!(lnk->spd_mask & BIT(spd_idx)))
		return -EINVAL;

	*dtx_min = _edgx_link_get_offline_delay(lnk, _DLY_TX, _DLY_MIN, speed);
	*dtx_max = _edgx_link_get_offline_delay(lnk, _DLY_TX, _DLY_MAX, speed);
	return 0;
}

int edgx_link_get_speed_rx_delays(struct edgx_link *lnk, int speed,
				  ktime_t *drx_min, ktime_t *drx_max)
{
	int spd_idx;

	if (!edgx_link_delays_entered(lnk, _DLY_RX))
		return -EINVAL;

	spd_idx = edgx_br_spd_idx(speed);
	if (!(lnk->spd_mask & BIT(spd_idx)))
		return -EINVAL;

	*drx_min = _edgx_link_get_offline_delay(lnk, _DLY_RX, _DLY_MIN, speed);
	*drx_max = _edgx_link_get_offline_delay(lnk, _DLY_RX, _DLY_MAX, speed);
	return 0;
}

void edgx_link_update_speed(struct edgx_link *lnk, int speed)
{
	lnk->speed = speed;
}

void edgx_link_update_delays(struct edgx_link *lnk, int speed)
{
	int spd_idx = edgx_br_spd_idx(speed);

	if (_edgx_link_has_adpt(lnk) & (spd_idx >= 0)) {
		/* Update delays from adapter (if present) */
		lnk->adpt_dly[_DLY_RX][spd_idx] = edgx_rd16(lnk->adpt.iobase,
							    _ADPT_REG_RXDLY);
		lnk->adpt_dly[_DLY_TX][spd_idx] = edgx_rd16(lnk->adpt.iobase,
							    _ADPT_REG_TXDLY);
	}
}

int edgx_link_is_external(struct edgx_link *lnk)
{
	/* Double-negation to turn pointer into bool */
	return ((!!lnk->phydev) || (!!lnk->sfp.i2c_bus));
}

static inline const unsigned long *_edgx_link_adpt_adjust(struct edgx_link *lnk)
{
	if (lnk->adpt.desc && lnk->adpt.desc->adjust)
		return lnk->adpt.desc->adjust(lnk);

	/* None-adapter trivially does everything that PHY and port can do */
	return _SPEED_CAPS_MASK;
}

static inline phy_interface_t _edgx_link_adpt_itf(struct edgx_link *lnk)
{
	if (lnk->adpt.desc) /* we have an adapter */
		return lnk->adpt.desc->itf;

	/* TODO: Select interface for multi-gigabit link modes */
	if (phylink_test(lnk->supported, 1000baseT_Full)) {
		/* No adapter, but link supports gigabit -> GMII */
		return PHY_INTERFACE_MODE_GMII;
	}

	/* No adapter and no gigabit -> chip native MII */
	return PHY_INTERFACE_MODE_MII;
}

static void _edgx_link_disconnect(struct edgx_link *lnk)
{
	if (lnk->phydev) {
		if (lnk->phydev->state != PHY_HALTED)
			phy_stop(lnk->phydev);
		phy_disconnect(lnk->phydev);
		phy_device_remove(lnk->phydev);
		phy_device_free(lnk->phydev);
		lnk->phydev = NULL;
	}
	if (lnk->sfp.mii_bus) {
		mdiobus_unregister(lnk->sfp.mii_bus);
		mdiobus_free(lnk->sfp.mii_bus);
		lnk->sfp.mii_bus = NULL;
	}
	if (lnk->sfp.eeprom) {
		i2c_unregister_device(lnk->sfp.eeprom);
		lnk->sfp.eeprom = NULL;
	}
}

static void _edgx_link_phy_support(struct edgx_link *lnk)
{
	__ETHTOOL_DECLARE_LINK_MODE_MASK(spdcaps);
	__ETHTOOL_DECLARE_LINK_MODE_MASK(applied);

	linkmode_and(spdcaps, lnk->supported, _edgx_link_adpt_adjust(lnk));
	linkmode_and(spdcaps, spdcaps, lnk->phydev->supported);

	linkmode_andnot(applied, lnk->phydev->supported, _SPEED_CAPS_MASK);
	linkmode_or(applied, applied, spdcaps);

	linkmode_copy(lnk->phydev->supported, applied);
	linkmode_copy(lnk->phydev->advertising, applied);
	lnk->phydev->speed  = SPEED_UNKNOWN;
	lnk->phydev->duplex = DUPLEX_UNKNOWN;
}

static int _edgx_link_mdio_alloc(struct edgx_link *lnk)
{
	struct mii_bus *mii;

	mii = mdio_i2c_alloc(&lnk->sfp.i2c_bus->dev, lnk->sfp.i2c_bus,
			     MDIO_I2C_NONE);
	if (IS_ERR(mii)) {
		edgx_pt_err(lnk->parent, "MII bus allocation fail\n");
		return -ENODEV;
	}
	mii->name = "edgx_mdio_i2c";
	mii->phy_mask = ~0;
	lnk->sfp.mii_bus = mii;
	if (mdiobus_register(lnk->sfp.mii_bus)) {
		edgx_pt_err(lnk->parent, "MDIO bus register fail\n");
		mdiobus_free(lnk->sfp.mii_bus);
		return -ENODEV;
	}

	return 0;
}

static int _edgx_link_configure_copper(struct edgx_link *lnk)
{
	int ret = 0;
	struct edgx_pt    *pt      = lnk->parent;
	struct net_device *netdev  = edgx_pt_get_netdev(pt);

	if (_edgx_link_mdio_alloc(lnk))
		return -ENODEV;

	lnk->phydev = mdiobus_scan_c22(lnk->sfp.mii_bus, _SFP_PHY_ADDR);
	if (IS_ERR(lnk->phydev)) {
		if (lnk->phydev == ERR_PTR(-ENODEV))
			edgx_pt_err(lnk->parent, "SFP PHY device not found\n");
		else
			edgx_pt_err(lnk->parent, "SFP PHY device error\n");
		lnk->phydev = NULL;
		goto out;
	}
	/* sleep after phy reset */
	msleep(_T_PHY_RESET_MS);

	ret = phy_connect_direct(netdev, lnk->phydev, edgx_pt_link_change,
				 _edgx_link_adpt_itf(lnk));
	if (ret) {
		edgx_pt_err(lnk->parent, "SFP PHY device not connected\n");
		phy_device_remove(lnk->phydev);
		phy_device_free(lnk->phydev);
		lnk->phydev = NULL;
		goto out;
	}

	_edgx_link_phy_support(lnk);

	netdev->phydev = lnk->phydev;

	if (netif_running(netdev))
		edgx_link_start(lnk);

	return 0;

out:
	_edgx_link_disconnect(lnk);
	return -ENODEV;
}

static void _edgx_link_sfp_work(struct work_struct *work_arg)
{
	struct edgx_link *lnk = container_of(work_arg, struct edgx_link,
					     work_lnk.work);
	struct i2c_client *eeprom = NULL;
	int ret = 0;
	s32 eth_compl = -ENODEV;
	s32 eth10g_compl = -ENODEV;
	s32 ext_compl = -ENODEV;
	enum sfp_kind { sfp_unknown, sfp_use_phy, sfp_skip_phy };
	enum sfp_kind sfp_kind = sfp_unknown;

	mutex_lock(&lnk->lock);
	if (!lnk->sfp.eeprom) {
		eeprom = i2c_new_client_device(lnk->sfp.i2c_bus, &_sfp_eeprom);
		if (IS_ERR(eeprom))
			goto out;
		lnk->sfp.eeprom = eeprom;
	}

	eth_compl = i2c_smbus_read_byte_data(lnk->sfp.eeprom, _SFP_ETH_COMPL);
	if (eth_compl >= 0)
		eth10g_compl = i2c_smbus_read_byte_data(lnk->sfp.eeprom,
							_SFP_ETH_10G_COMPL);
	if (eth10g_compl >= 0)
		ext_compl = i2c_smbus_read_byte_data(lnk->sfp.eeprom,
						     _SFP_EXT_COMPL);
	if (eth_compl < 0 || eth10g_compl < 0 || ext_compl < 0) {
		if (lnk->sfp.state != SFP_NOT_PRESENT) {
			lnk->sfp.state = SFP_NOT_PRESENT;
			edgx_pt_info(lnk->parent, "SFP plugged out...\n");
			_edgx_link_disconnect(lnk);
			edgx_pt_link_change(edgx_pt_get_netdev(lnk->parent));
		}
		goto out;
	}

	if ((eth_compl & _ETH_CMPL_1000BASET) ||
	    (ext_compl == _EXT_CMPL_5000BASET) ||
	    (ext_compl == _EXT_CMPL_25000BASET)) {
		sfp_kind = sfp_use_phy;
		lnk->sfp.spd_mask = 0;
	} else if ((eth10g_compl & _ETH_10G_10000BASE_SR) ||
		   (ext_compl == _EXT_CMPL_10000BASET_SR)) {
		sfp_kind = sfp_skip_phy;
		lnk->sfp.spd_mask = BIT(EDGX_SPD_10000);
	}

	if (lnk->sfp.state != SFP_CONFIGURED && sfp_kind == sfp_use_phy) {
		ret = _edgx_link_configure_copper(lnk);
		if (ret == 0) {
			lnk->sfp.state = SFP_CONFIGURED;
			edgx_pt_info(lnk->parent,
				     "SFP with managed PHY plugged in...\n");
		}
	}

	if (lnk->sfp.state != SFP_CONFIGURED && sfp_kind == sfp_skip_phy) {
		edgx_pt_info(lnk->parent, "SFP plugged in...\n");
		_edgx_link_adpt_adjust(lnk);
		lnk->sfp.state = SFP_CONFIGURED;
	}

	if (lnk->sfp.state != SFP_UNKNOWN && sfp_kind == sfp_unknown) {
		lnk->sfp.state = SFP_UNKNOWN;
		edgx_pt_info(lnk->parent, "SFP plugged in (unknown)...\n");
		_edgx_link_disconnect(lnk);
		edgx_pt_link_change(edgx_pt_get_netdev(lnk->parent));
	}

out:
	mutex_unlock(&lnk->lock);

	if (ret)
		edgx_pt_warn(lnk->parent, "SFP detection went wrong\n");

	schedule_delayed_work(&lnk->work_lnk, _SFP_CHECK_PERIOD);
}

int edgx_link_set_mdiobus(struct edgx_link *lnk, const char *mdiobus_id)
{
	struct edgx_pt    *pt      = lnk->parent;
	struct net_device *netdev  = edgx_pt_get_netdev(pt);

	_edgx_link_disconnect(lnk);
	/* Call port directly on link change, so that notification is
	 * distributed from there. Will also call back to edgx_link_update
	 */

	lnk->phydev = phy_connect(netdev, mdiobus_id, edgx_pt_link_change,
				  _edgx_link_adpt_itf(lnk));
	if (IS_ERR(lnk->phydev)) {
		edgx_pt_err(pt, "Cannot connect PHY '%s': %ld\n", mdiobus_id,
			    PTR_ERR(lnk->phydev));
		lnk->phydev = NULL;
		return -ENODEV;
	}

	_edgx_link_phy_support(lnk);
	edgx_pt_info(pt, "Connected PHY %s at '%s' to port\n",
		     lnk->phydev->drv->name, mdiobus_id);

	netdev->phydev = lnk->phydev;

	if (netif_running(netdev))
		edgx_link_start(lnk);

	return 0;
}

static ssize_t mdiobus_show(struct device *dev,
			    struct device_attribute *attr,
			    char *buf)
{
	struct edgx_pt    *pt  = edgx_dev2pt(dev);
	struct edgx_link  *lnk = edgx_pt_get_link(pt);

	if (lnk->phydev)
		return scnprintf(buf, PAGE_SIZE, PHY_ID_FMT,
				 lnk->phydev->mdio.bus->id,
				 lnk->phydev->mdio.addr);
	return scnprintf(buf, PAGE_SIZE, "(none)");
}

static ssize_t mdiobus_store(struct device *dev,
			     struct device_attribute *attr,
			     const char *buf,
			     size_t count)
{
	int ret;
	struct edgx_pt    *pt      = edgx_dev2pt(dev);
	struct edgx_link  *lnk     = edgx_pt_get_link(pt);

	ret = edgx_link_set_mdiobus(lnk, buf);

	return ret ? ret : count;
}

static ssize_t i2cbus_store(struct device *dev,
			    struct device_attribute *attr,
			    const char *buf,
			    size_t count)
{
	struct edgx_pt     *pt  = edgx_dev2pt(dev);
	struct edgx_link   *lnk = edgx_pt_get_link(pt);
	struct device      *i2cdev;

	mutex_lock(&lnk->lock);
	_edgx_link_disconnect(lnk);

	if (lnk->sfp.i2c_bus) {
		cancel_delayed_work_sync(&lnk->work_lnk);
		put_device(&lnk->sfp.i2c_bus->dev);
		lnk->sfp.i2c_bus = NULL;
	}

	/* Needed to be able to unload the module */
	if (strcmp(buf, "detach") == 0) {
		mutex_unlock(&lnk->lock);
		return count;
	}

	/* We use an external i2c adapter, so it's the provider's duty to
	 * ensure, that the adapter is setup correctly to handle SFPs according
	 * to SFF-8472 (Rev. 11.0), e.g., 100 kHz max. speed.
	 */
	i2cdev = bus_find_device_by_name(&i2c_bus_type, NULL, buf);
	if (!i2cdev) {
		mutex_unlock(&lnk->lock);
		edgx_pt_err(pt, "Cannot find I2C adapter '%s'\n", buf);
		return -ENODEV;
	}

	lnk->sfp.i2c_bus = to_i2c_adapter(i2cdev);
	lnk->sfp.state = SFP_INIT;
	lnk->sfp.eeprom = i2c_new_client_device(lnk->sfp.i2c_bus, &_sfp_eeprom);
	if (IS_ERR(lnk->sfp.eeprom)) {
		lnk->sfp.eeprom = NULL;
		edgx_pt_warn(pt, "Cannot open I2C client for EEPROM\n");
	}
	mutex_unlock(&lnk->lock);

	schedule_delayed_work(&lnk->work_lnk, _SFP_CHECK_PERIOD);

	return count;
}

void edgx_link_set_delays(struct edgx_link *lnk, int speed,
			  unsigned int dtxmin, unsigned int dtxmax,
			  unsigned int drxmin, unsigned int drxmax)
{
	int spd_idx = edgx_br_spd_idx(speed);

	if (spd_idx < 0)
		return;

	lnk->phy_dly[_DLY_TX][spd_idx][_DLY_MIN] = dtxmin;
	lnk->phy_dly[_DLY_TX][spd_idx][_DLY_MAX] = dtxmax;
	lnk->phy_dly[_DLY_RX][spd_idx][_DLY_MIN] = drxmin;
	lnk->phy_dly[_DLY_RX][spd_idx][_DLY_MAX] = drxmax;
	lnk->dly_flags[_DLY_TX] |= EDGX_DLY_ID_BITS(spd_idx);
	lnk->dly_flags[_DLY_RX] |= EDGX_DLY_ID_BITS(spd_idx);
}

static ssize_t _delay_store(struct device *dev, const char *buf, size_t count,
			    enum edgx_speed spd, enum _delay_dir dir,
			    enum _delay_min_max limit)
{
	struct edgx_pt    *pt  = edgx_dev2pt(dev);
	struct edgx_link  *lnk = edgx_pt_get_link(pt);
	int ret;

	if (lnk->dly_flags[dir] & EDGX_DLY_BIT(spd, limit))
		return -EINVAL;

	ret = kstrtouint(buf, 10, &lnk->phy_dly[dir][spd][limit]);

	if (!ret)
		lnk->dly_flags[dir] |= EDGX_DLY_BIT(spd, limit);

	return (ret) ? ret : count;
}

static ssize_t delay10tx_min_store(struct device *dev,
				   struct device_attribute *attr,
				   const char *buf, size_t count)
{
	return _delay_store(dev, buf, count, EDGX_SPD_10, _DLY_TX, _DLY_MIN);
}

static ssize_t delay10tx_max_store(struct device *dev,
				   struct device_attribute *attr,
				   const char *buf, size_t count)
{
	return _delay_store(dev, buf, count, EDGX_SPD_10, _DLY_TX, _DLY_MAX);
}

static ssize_t delay10rx_min_store(struct device *dev,
				   struct device_attribute *attr,
				   const char *buf, size_t count)
{
	return _delay_store(dev, buf, count, EDGX_SPD_10, _DLY_RX, _DLY_MIN);
}

static ssize_t delay10rx_max_store(struct device *dev,
				   struct device_attribute *attr,
				   const char *buf, size_t count)
{
	return _delay_store(dev, buf, count, EDGX_SPD_10, _DLY_RX, _DLY_MAX);
}

static ssize_t delay100tx_min_store(struct device *dev,
				    struct device_attribute *attr,
				    const char *buf, size_t count)
{
	return _delay_store(dev, buf, count, EDGX_SPD_100, _DLY_TX, _DLY_MIN);
}

static ssize_t delay100tx_max_store(struct device *dev,
				    struct device_attribute *attr,
				    const char *buf, size_t count)
{
	return _delay_store(dev, buf, count, EDGX_SPD_100, _DLY_TX, _DLY_MAX);
}

static ssize_t delay100rx_min_store(struct device *dev,
				    struct device_attribute *attr,
				    const char *buf, size_t count)
{
	return _delay_store(dev, buf, count, EDGX_SPD_100, _DLY_RX, _DLY_MIN);
}

static ssize_t delay100rx_max_store(struct device *dev,
				    struct device_attribute *attr,
				    const char *buf, size_t count)
{
	return _delay_store(dev, buf, count, EDGX_SPD_100, _DLY_RX, _DLY_MAX);
}

static ssize_t delay1000tx_min_store(struct device *dev,
				     struct device_attribute *attr,
				     const char *buf, size_t count)
{
	return _delay_store(dev, buf, count, EDGX_SPD_1000, _DLY_TX, _DLY_MIN);
}

static ssize_t delay1000tx_max_store(struct device *dev,
				     struct device_attribute *attr,
				     const char *buf, size_t count)
{
	return _delay_store(dev, buf, count, EDGX_SPD_1000, _DLY_TX, _DLY_MAX);
}

static ssize_t delay1000rx_min_store(struct device *dev,
				     struct device_attribute *attr,
				     const char *buf, size_t count)
{
	return _delay_store(dev, buf, count, EDGX_SPD_1000, _DLY_RX, _DLY_MIN);
}

static ssize_t delay1000rx_max_store(struct device *dev,
				     struct device_attribute *attr,
				     const char *buf, size_t count)
{
	return _delay_store(dev, buf, count, EDGX_SPD_1000, _DLY_RX, _DLY_MAX);
}

static ssize_t delay2500tx_min_store(struct device *dev,
				     struct device_attribute *attr,
				     const char *buf, size_t count)
{
	return _delay_store(dev, buf, count, EDGX_SPD_2500, _DLY_TX, _DLY_MIN);
}

static ssize_t delay2500tx_max_store(struct device *dev,
				     struct device_attribute *attr,
				     const char *buf, size_t count)
{
	return _delay_store(dev, buf, count, EDGX_SPD_2500, _DLY_TX, _DLY_MAX);
}

static ssize_t delay2500rx_min_store(struct device *dev,
				     struct device_attribute *attr,
				     const char *buf, size_t count)
{
	return _delay_store(dev, buf, count, EDGX_SPD_2500, _DLY_RX, _DLY_MIN);
}

static ssize_t delay2500rx_max_store(struct device *dev,
				     struct device_attribute *attr,
				     const char *buf, size_t count)
{
	return _delay_store(dev, buf, count, EDGX_SPD_2500, _DLY_RX, _DLY_MAX);
}

static ssize_t delay5000tx_min_store(struct device *dev,
				     struct device_attribute *attr,
				     const char *buf, size_t count)
{
	return _delay_store(dev, buf, count, EDGX_SPD_5000, _DLY_TX, _DLY_MIN);
}

static ssize_t delay5000tx_max_store(struct device *dev,
				     struct device_attribute *attr,
				     const char *buf, size_t count)
{
	return _delay_store(dev, buf, count, EDGX_SPD_5000, _DLY_TX, _DLY_MAX);
}

static ssize_t delay5000rx_min_store(struct device *dev,
				     struct device_attribute *attr,
				     const char *buf, size_t count)
{
	return _delay_store(dev, buf, count, EDGX_SPD_5000, _DLY_RX, _DLY_MIN);
}

static ssize_t delay5000rx_max_store(struct device *dev,
				     struct device_attribute *attr,
				     const char *buf, size_t count)
{
	return _delay_store(dev, buf, count, EDGX_SPD_5000, _DLY_RX, _DLY_MAX);
}

static ssize_t delay10000tx_min_store(struct device *dev,
				      struct device_attribute *attr,
				      const char *buf, size_t count)
{
	return _delay_store(dev, buf, count, EDGX_SPD_10000, _DLY_TX, _DLY_MIN);
}

static ssize_t delay10000tx_max_store(struct device *dev,
				      struct device_attribute *attr,
				      const char *buf, size_t count)
{
	return _delay_store(dev, buf, count, EDGX_SPD_10000, _DLY_TX, _DLY_MAX);
}

static ssize_t delay10000rx_min_store(struct device *dev,
				      struct device_attribute *attr,
				      const char *buf, size_t count)
{
	return _delay_store(dev, buf, count, EDGX_SPD_10000, _DLY_RX, _DLY_MIN);
}

static ssize_t delay10000rx_max_store(struct device *dev,
				      struct device_attribute *attr,
				      const char *buf, size_t count)
{
	return _delay_store(dev, buf, count, EDGX_SPD_10000, _DLY_RX, _DLY_MAX);
}

DEVICE_ATTR_RW(mdiobus);
DEVICE_ATTR_WO(i2cbus);
DEVICE_ATTR_WO(delay10tx_min);
DEVICE_ATTR_WO(delay10tx_max);
DEVICE_ATTR_WO(delay10rx_min);
DEVICE_ATTR_WO(delay10rx_max);
DEVICE_ATTR_WO(delay100tx_min);
DEVICE_ATTR_WO(delay100tx_max);
DEVICE_ATTR_WO(delay100rx_min);
DEVICE_ATTR_WO(delay100rx_max);
DEVICE_ATTR_WO(delay1000tx_min);
DEVICE_ATTR_WO(delay1000tx_max);
DEVICE_ATTR_WO(delay1000rx_min);
DEVICE_ATTR_WO(delay1000rx_max);
DEVICE_ATTR_WO(delay2500tx_min);
DEVICE_ATTR_WO(delay2500tx_max);
DEVICE_ATTR_WO(delay2500rx_min);
DEVICE_ATTR_WO(delay2500rx_max);
DEVICE_ATTR_WO(delay5000tx_min);
DEVICE_ATTR_WO(delay5000tx_max);
DEVICE_ATTR_WO(delay5000rx_min);
DEVICE_ATTR_WO(delay5000rx_max);
DEVICE_ATTR_WO(delay10000tx_min);
DEVICE_ATTR_WO(delay10000tx_max);
DEVICE_ATTR_WO(delay10000rx_min);
DEVICE_ATTR_WO(delay10000rx_max);

static struct attribute *phy_attrs[] = {
	&dev_attr_mdiobus.attr,
	&dev_attr_i2cbus.attr,
	&dev_attr_delay10tx_min.attr,
	&dev_attr_delay10tx_max.attr,
	&dev_attr_delay10rx_min.attr,
	&dev_attr_delay10rx_max.attr,
	&dev_attr_delay100tx_min.attr,
	&dev_attr_delay100tx_max.attr,
	&dev_attr_delay100rx_min.attr,
	&dev_attr_delay100rx_max.attr,
	&dev_attr_delay1000tx_min.attr,
	&dev_attr_delay1000tx_max.attr,
	&dev_attr_delay1000rx_min.attr,
	&dev_attr_delay1000rx_max.attr,
	&dev_attr_delay2500tx_min.attr,
	&dev_attr_delay2500tx_max.attr,
	&dev_attr_delay2500rx_min.attr,
	&dev_attr_delay2500rx_max.attr,
	&dev_attr_delay5000tx_min.attr,
	&dev_attr_delay5000tx_max.attr,
	&dev_attr_delay5000rx_min.attr,
	&dev_attr_delay5000rx_max.attr,
	&dev_attr_delay10000tx_min.attr,
	&dev_attr_delay10000tx_max.attr,
	&dev_attr_delay10000rx_min.attr,
	&dev_attr_delay10000rx_max.attr,
	NULL,
};

static struct attribute_group phy_group = {
	.name  = "phy",
	.attrs = phy_attrs,
};

static void edgx_link_set_speeds(struct edgx_link *lnk,
				 edgx_speed_mask_t spd_mask,
				 bool internal)
{
	int highest_supported = -1;

	lnk->spd_mask = spd_mask;

	linkmode_zero(lnk->supported);

	lnk->speed = SPEED_UNKNOWN;
	if (spd_mask & BIT(EDGX_SPD_10)) {
		lnk->speed = SPEED_10;
		if (!internal)
			phylink_set(lnk->supported, 10baseT_Full);
		highest_supported = ETHTOOL_LINK_MODE_10baseT_Full_BIT;
	}
	if (spd_mask & BIT(EDGX_SPD_100)) {
		lnk->speed = SPEED_100;
		if (!internal)
			phylink_set(lnk->supported, 100baseT_Full);
		highest_supported = ETHTOOL_LINK_MODE_100baseT_Full_BIT;
	}
	if (spd_mask & BIT(EDGX_SPD_1000)) {
		lnk->speed = SPEED_1000;
		if (!internal)
			phylink_set(lnk->supported, 1000baseT_Full);
		highest_supported = ETHTOOL_LINK_MODE_1000baseT_Full_BIT;
	}
	if (spd_mask & BIT(EDGX_SPD_2500)) {
		lnk->speed = SPEED_2500;
		if (!internal)
			phylink_set(lnk->supported, 2500baseT_Full);
		highest_supported = ETHTOOL_LINK_MODE_2500baseT_Full_BIT;
	}
	if (spd_mask & BIT(EDGX_SPD_5000)) {
		lnk->speed = SPEED_5000;
		if (!internal)
			phylink_set(lnk->supported, 5000baseT_Full);
		highest_supported = ETHTOOL_LINK_MODE_5000baseT_Full_BIT;
	}
	if (spd_mask & BIT(EDGX_SPD_10000)) {
		lnk->speed = SPEED_10000;
		if (!internal)
			phylink_set(lnk->supported, 10000baseT_Full);
		highest_supported = ETHTOOL_LINK_MODE_10000baseT_Full_BIT;
	}
	if (internal && highest_supported >= 0)
		linkmode_set_bit(highest_supported, lnk->supported);
}

static void edgx_link_init_once(void)
{
	/* This function must be called at least once before using any of
	 * these variables which are meant to be constant, but cannot
	 * be initialized as such directly.
	 */

	if (phylink_test(_SPEED_CAPS_MASK, 10baseT_Full))
		return;

	phylink_set(_SPEED_CAPS_MASK, 10baseT_Full);
	phylink_set(_SPEED_CAPS_MASK, 10baseT_Half);
	phylink_set(_SPEED_CAPS_MASK, 100baseT_Full);
	phylink_set(_SPEED_CAPS_MASK, 100baseT_Half);
	phylink_set(_SPEED_CAPS_MASK, 1000baseT_Full);
	phylink_set(_SPEED_CAPS_MASK, 1000baseT_Half);
	phylink_set(_SPEED_CAPS_MASK, 2500baseT_Full);
	phylink_set(_SPEED_CAPS_MASK, 5000baseT_Full);
	phylink_set(_SPEED_CAPS_MASK, 10000baseT_Full);
	phylink_set(_SPEED_CAPS_MASK, Pause);
	phylink_set(_SPEED_CAPS_MASK, Asym_Pause);

	phylink_set(_SUPPORTED_DUAL_ADAPTER, MII);
	phylink_set(_SUPPORTED_DUAL_ADAPTER, TP);
	phylink_set(_SUPPORTED_DUAL_ADAPTER, FIBRE);
	phylink_set(_SUPPORTED_DUAL_ADAPTER, 1000baseT_Full);
	phylink_set(_SUPPORTED_DUAL_ADAPTER, 100baseT_Full);
	phylink_set(_SUPPORTED_DUAL_ADAPTER, 10baseT_Full);
	phylink_set(_SUPPORTED_DUAL_ADAPTER, Autoneg);

	phylink_set(_SUPPORTED_PCS_PMA_ADAPTER, MII);
	phylink_set(_SUPPORTED_PCS_PMA_ADAPTER, TP);
	phylink_set(_SUPPORTED_PCS_PMA_ADAPTER, FIBRE);
	phylink_set(_SUPPORTED_PCS_PMA_ADAPTER, 10000baseT_Full);
	phylink_set(_SUPPORTED_PCS_PMA_ADAPTER, 1000baseT_Full);
	phylink_set(_SUPPORTED_PCS_PMA_ADAPTER, Autoneg);
}

int edgx_link_init(struct edgx_pt *pt, struct edgx_link **lnk,
		   edgx_speed_mask_t spd_mask,
		   bool internal)
{
	const struct edgx_ifreq ifreq[] = {
		{ .id = AC_ADPT_ID,     .v_maj = 1 },
		{ .id = AC_EXT_ADPT_ID, .v_maj = 1 },
		{ .id = AC_EOT_ID }
	};
	struct edgx_ifdesc ptifd;
	const struct edgx_ifdesc *adptif;
	int i;
	u16 reg;

	edgx_link_init_once();

	*lnk = kzalloc(sizeof(**lnk), GFP_KERNEL);
	if (!(*lnk))
		return -ENOMEM;

	(*lnk)->parent = pt;

	edgx_link_set_speeds(*lnk, spd_mask, internal);

	if (PT_IS_EP_ID(edgx_pt_get_id(pt))) {
		goto end_adpt;
	}

	adptif = edgx_ac_get_ptifver(ifreq, edgx_pt_get_id(pt), &ptifd, NULL);
	if (!adptif) {
		edgx_pt_info(pt, "   Adding mii/gmii (native) ...\n");
		goto end_adpt;
	}

	reg = edgx_get16(adptif->iobase, _ADPT_REG_ID, 7, 0);
	for (i = 0; i < ARRAY_SIZE(_adpt_desc); i++)
		if (_adpt_desc[i].id == reg) {
			(*lnk)->adpt.iobase = adptif->iobase;
			(*lnk)->adpt.desc = &_adpt_desc[i];
			edgx_pt_info(pt, "   Adding %s interface adapter ...\n",
				     (*lnk)->adpt.desc->name);
			goto end_adpt;
		}

	if (reg != _ADPT_DMA_ID && reg != _ADPT_NONE_ID)
		edgx_pt_err(pt, "   Unknown Adapter ID 0x%X\n", reg);

end_adpt:
	edgx_pt_add_sysfs(pt, &phy_group);
	mutex_init(&(*lnk)->lock);

	INIT_DELAYED_WORK(&((*lnk)->work_lnk), &_edgx_link_sfp_work);

	return 0;
}

void edgx_link_shutdown(struct edgx_link *lnk)
{
	cancel_delayed_work_sync(&lnk->work_lnk);
	edgx_pt_rem_sysfs(lnk->parent, &phy_group);
	_edgx_link_disconnect(lnk);
	kfree(lnk);
}
