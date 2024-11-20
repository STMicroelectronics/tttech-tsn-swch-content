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
#include <linux/sysfs.h>
#include <linux/netdevice.h>
#include <linux/i2c.h>
#include <linux/workqueue.h>
#include <linux/phy.h>

#include "edge_ac.h"
#include "edge_link.h"
#include "edge_util.h"

#define _SFP_ETH_COMPL 0x6
#define _CMPL_1000BASET BIT(3) /*3rd bit set -> 1000BASE-T*/

#define _ADPT_GMII_ID   0xB0
#define _ADPT_RGMII_ID  0xB1
#define _ADPT_RMII_ID   0xB5
#define _ADPT_TRIPLE_ID 0xB9
#define _ADPT_NONE_ID	0xBC

#define _ADPT_REG_ID    0x0
#define _ADPT_REG_RXDLY 0x20
#define _ADPT_REG_TXDLY 0x22

#define _ADPT_LINK_STS  0x2
#define _ADPT_PCS_CTL	0x10

#define _PCS_CTL_ANEG		(0x111)
#define _LINK_STATUS_HW_CTL	(1U)

#define _SFP_CHECK_PERIOD	(2 * HZ)
#define _T_PHY_RESET_MS		50U
/* SFP modules appear to always have their PHY configured for bus address
 * 0x56 (which with mdio-i2c, translates to a PHY address of 22).
 */
#define _SFP_PHY_ADDR	22

#define _SPEED_CAPS_MASK (SUPPORTED_10baseT_Full   |     \
			  SUPPORTED_10baseT_Half   |     \
			  SUPPORTED_100baseT_Full  |     \
			  SUPPORTED_100baseT_Half  |     \
			  SUPPORTED_1000baseT_Full |     \
			  SUPPORTED_1000baseT_Half |     \
			  SUPPORTED_Pause          |     \
			  SUPPORTED_Asym_Pause           \
			 )

#define _SUPPORTED_DUAL_ADAPTER (SUPPORTED_MII |		\
				SUPPORTED_TP |			\
				SUPPORTED_FIBRE |		\
				SUPPORTED_1000baseT_Full |	\
				SUPPORTED_100baseT_Full |	\
				SUPPORTED_10baseT_Full |	\
				SUPPORTED_Autoneg)

int _edgx_link_adjust_adapter(struct edgx_link *lnk);

enum _delay_id {
	_DLY_10 = 0,
	_DLY_100,
	_DLY_1000,
	_NDLYS, /* must be last */
};

#define EDGX_DLY_MASK_10_100_1000	(0x3f)
#define EDGX_DLY_MASK_100_1000		(0x3c)
#define EDGX_DLY_MASK_10_100		(0xf)
#define EDGX_DLY_MASK_100		(0xc)

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
	u16             id;
	phy_interface_t itf;

	/* returns the capabilities of current mode */
	int (*adjust)(struct edgx_link *link);

} _adpt_desc[] = {
	{ .id = _ADPT_GMII_ID,   .itf = PHY_INTERFACE_MODE_GMII, },
	{ .id = _ADPT_RGMII_ID,  .itf = PHY_INTERFACE_MODE_RGMII_ID, },
	{ .id = _ADPT_RMII_ID,   .itf = PHY_INTERFACE_MODE_RMII, },
	{ .id = _ADPT_TRIPLE_ID, .itf = PHY_INTERFACE_MODE_SGMII,
	  .adjust = _edgx_link_adjust_adapter },
};

struct edgx_adpt {
	edgx_io_t         *iobase;
	struct _adpt_desc *desc;
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
	u32                supported;
	struct edgx_adpt   adpt;

	struct {
		struct i2c_client  *eeprom;
		struct mii_bus	   *mii_bus;
		struct i2c_adapter *i2c_bus;
		enum _sfp_state	    state; //1 present 0 not present
	} sfp;

	/* Note: To be super-safe and super-consistent here, we'd need a
	 *       double-buffer here, but that would be overkill.
	 *       Effects are negligible.
	 */
	unsigned int phy_dly[_NDLYDIRS][_NDLYS][_NDLYMX];
	unsigned int adpt_dly[_NDLYDIRS][_NDLYS];
	u32 dly_flags[_NDLYDIRS];
	enum edgx_lnk_mode lnk_mode;

	struct delayed_work	 work_lnk;
	/* Synchronize access to sfp */
	struct mutex		lock;
};

static struct i2c_board_info _sfp_eeprom = {
	I2C_BOARD_INFO("EEPROM", 0xA0 >> 1),
};

int _edgx_link_adjust_adapter(struct edgx_link *lnk)
{
	/* Link status (affects LEDs) HW controlled */
	edgx_set16(lnk->adpt.iobase, _ADPT_LINK_STS, 1, 1, _LINK_STATUS_HW_CTL);
	/* Adapter to auto negotiation; enabled bit 0,4,8 = 1 */
	edgx_wr16(lnk->adpt.iobase, _ADPT_PCS_CTL, _PCS_CTL_ANEG);

	return _SUPPORTED_DUAL_ADAPTER;
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

	ethtool_convert_legacy_u32_to_link_mode(cmd->link_modes.supported,
						lnk->supported);
	ethtool_convert_legacy_u32_to_link_mode(cmd->link_modes.advertising,
						0);
	ethtool_convert_legacy_u32_to_link_mode(cmd->link_modes.lp_advertising,
						0);
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

int edgx_link_get_speed(struct edgx_link *lnk)
{
	// TODO
	if (!lnk) /* shouldn't happen */
		return SPEED_UNKNOWN;

	if (!lnk->phydev) /* no PHY associated, everything fixed. */
		return lnk->speed;

	/* ... if just one speed is advertised, return it ... */
	if (!lnk->phydev->link && lnk->phydev->autoneg) {
		enum ethtool_link_mode_bit_indices lnk_modes[] = {
			ETHTOOL_LINK_MODE_10baseT_Full_BIT,
			ETHTOOL_LINK_MODE_100baseT_Full_BIT,
			ETHTOOL_LINK_MODE_1000baseT_Full_BIT,
			__ETHTOOL_LINK_MODE_MASK_NBITS
		};
		int spds[] = { SPEED_10, SPEED_100, SPEED_1000};
		int i, speed, cnt;

		for (i = 0, cnt = 0, speed = SPEED_UNKNOWN;
		     lnk_modes[i] != __ETHTOOL_LINK_MODE_MASK_NBITS; i++)
			if (linkmode_test_bit(lnk_modes[i],
					     lnk->phydev->advertising)) {
				speed = spds[i];
				cnt++;
			}

		if (cnt > 1)
			speed = SPEED_UNKNOWN;
		return speed;
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

static inline int _edgx_link_dly_idx(int speed)
{
	switch (speed) {
	case SPEED_10:
		return _DLY_10;
	case SPEED_100:
		return _DLY_100;
	case SPEED_1000:
		return _DLY_1000;
	default:
		return -1;
	}
}

static inline ktime_t _edgx_link_get_offline_delay(struct edgx_link *lnk, int dir,
						   enum _delay_min_max min_max, int speed)
{
	int dly_idx = _edgx_link_dly_idx(speed);

	if (dly_idx < 0)
		/* speed, and thus delay unknown - set to 0. */
		return ktime_set(0, 0);

	return ktime_set(0, lnk->phy_dly[dir][dly_idx][min_max] +
			 lnk->adpt_dly[dir][dly_idx]);
}

static inline ktime_t _edgx_link_get_delay(struct edgx_link *lnk, int dir,
					   enum _delay_min_max min_max)
{
	return _edgx_link_get_offline_delay(lnk, dir, min_max, edgx_link_get_speed(lnk));
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

static bool edgx_link_delays_entered(struct edgx_link  *lnk,
				     enum _delay_dir dir)
{
	bool ret = false;

	switch (lnk->lnk_mode) {
	case EDGX_LNKMOD_10_100_1000:
		ret = (lnk->dly_flags[dir] &
		       EDGX_DLY_MASK_10_100_1000) == EDGX_DLY_MASK_10_100_1000;
		break;
	case EDGX_LNKMOD_100_1000:
		ret = (lnk->dly_flags[dir] & EDGX_DLY_MASK_100_1000) ==
		      EDGX_DLY_MASK_100_1000;
		break;
	case EDGX_LNKMOD_10_100:
		ret = (lnk->dly_flags[dir] & EDGX_DLY_MASK_10_100) ==
		      EDGX_DLY_MASK_10_100;
		break;
	case EDGX_LNKMOD_100:
		ret = (lnk->dly_flags[dir] & EDGX_DLY_MASK_100) ==
		      EDGX_DLY_MASK_100;
		break;
	case EDGX_LNKMOD_INVAL:
	default:
		break;
	}
	return ret;
}

enum edgx_lnk_mode edgx_link_get_lnk_mode(struct edgx_link  *lnk)
{
	return lnk->lnk_mode;
}

int edgx_link_get_tx_delays_hi(struct edgx_link  *lnk,
			       ktime_t *dtx_min, ktime_t *dtx_max)
{
	if (!edgx_link_delays_entered(lnk, _DLY_TX))
		return -EINVAL;

	if (lnk->lnk_mode == EDGX_LNKMOD_100_1000) {
		*dtx_min = ktime_set(0, lnk->phy_dly[_DLY_TX][_DLY_1000][_DLY_MIN] +
				     lnk->adpt_dly[_DLY_TX][_DLY_1000]);
		*dtx_max = ktime_set(0, lnk->phy_dly[_DLY_TX][_DLY_1000][_DLY_MAX] +
				     lnk->adpt_dly[_DLY_TX][_DLY_1000]);
	} else if ((lnk->lnk_mode == EDGX_LNKMOD_10_100) ||
		   (lnk->lnk_mode == EDGX_LNKMOD_100)) {
		*dtx_min = ktime_set(0, lnk->phy_dly[_DLY_TX][_DLY_100][_DLY_MIN] +
				     lnk->adpt_dly[_DLY_TX][_DLY_100]);
		*dtx_max = ktime_set(0, lnk->phy_dly[_DLY_TX][_DLY_100][_DLY_MAX] +
				     lnk->adpt_dly[_DLY_TX][_DLY_100]);
	} else {
		return -EINVAL;
	}
	return 0;
}

int edgx_link_get_tx_delays_lo(struct edgx_link  *lnk,
			       ktime_t *dtx_min, ktime_t *dtx_max)
{
	if (!edgx_link_delays_entered(lnk, _DLY_TX))
		return -EINVAL;

	if ((lnk->lnk_mode == EDGX_LNKMOD_100_1000) ||
	    (lnk->lnk_mode == EDGX_LNKMOD_100)) {
		*dtx_min = ktime_set(0, lnk->phy_dly[_DLY_TX][_DLY_100][_DLY_MIN] +
				     lnk->adpt_dly[_DLY_TX][_DLY_100]);
		*dtx_max = ktime_set(0, lnk->phy_dly[_DLY_TX][_DLY_100][_DLY_MAX] +
				     lnk->adpt_dly[_DLY_TX][_DLY_100]);
	} else if (lnk->lnk_mode == EDGX_LNKMOD_10_100) {
		*dtx_min = ktime_set(0, lnk->phy_dly[_DLY_TX][_DLY_10][_DLY_MIN] +
				     lnk->adpt_dly[_DLY_TX][_DLY_10]);
		*dtx_max = ktime_set(0, lnk->phy_dly[_DLY_TX][_DLY_10][_DLY_MAX] +
				     lnk->adpt_dly[_DLY_TX][_DLY_10]);
	} else {
		return -EINVAL;
	}
	return 0;
}

int edgx_link_get_rx_delays_hi(struct edgx_link  *lnk,
			       ktime_t *drx_min, ktime_t *drx_max)
{
	if (!edgx_link_delays_entered(lnk, _DLY_RX))
		return -EINVAL;

	if (lnk->lnk_mode == EDGX_LNKMOD_100_1000) {
		*drx_min = ktime_set(0, lnk->phy_dly[_DLY_RX][_DLY_1000][_DLY_MIN] +
				     lnk->adpt_dly[_DLY_RX][_DLY_1000]);
		*drx_max = ktime_set(0, lnk->phy_dly[_DLY_RX][_DLY_1000][_DLY_MAX] +
				     lnk->adpt_dly[_DLY_RX][_DLY_1000]);
	} else if ((lnk->lnk_mode == EDGX_LNKMOD_10_100) ||
		   (lnk->lnk_mode == EDGX_LNKMOD_100)) {
		*drx_min = ktime_set(0, lnk->phy_dly[_DLY_RX][_DLY_100][_DLY_MIN] +
				     lnk->adpt_dly[_DLY_RX][_DLY_100]);
		*drx_max = ktime_set(0, lnk->phy_dly[_DLY_RX][_DLY_100][_DLY_MAX] +
				     lnk->adpt_dly[_DLY_RX][_DLY_100]);
	} else {
		return -EINVAL;
	}
	return 0;
}

void edgx_link_update_speed(struct edgx_link *lnk, int speed)
{
	lnk->speed = speed;

}

void edgx_link_update_delays(struct edgx_link *lnk, int speed){
	int idx;
	idx = _edgx_link_dly_idx(speed);

	if (_edgx_link_has_adpt(lnk) & (idx >= 0)) {

		/* Update delays from adapter (if present) */
		lnk->adpt_dly[_DLY_RX][idx] = edgx_rd16(lnk->adpt.iobase,
						   _ADPT_REG_RXDLY);
		lnk->adpt_dly[_DLY_TX][idx] = edgx_rd16(lnk->adpt.iobase,
						   _ADPT_REG_TXDLY);
	}
}

int edgx_link_is_external(struct edgx_link *lnk)
{
	/* Double-negation to turn pointer into bool */
	return ((!!lnk->phydev) || (!!lnk->sfp.i2c_bus));
}

static inline int _edgx_link_adpt_adjust(struct edgx_link *lnk)
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
	if (lnk->supported & SUPPORTED_1000baseT_Full)
		/* No adapter, but link supports gigabit -> GMII */
		return PHY_INTERFACE_MODE_GMII;

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
	u32 spdcaps, supported, applied;

	ethtool_convert_link_mode_to_legacy_u32(&supported,
						lnk->phydev->supported);

	spdcaps = lnk->supported &
		  _edgx_link_adpt_adjust(lnk) &
		  supported;

	applied = (supported & ~_SPEED_CAPS_MASK) | spdcaps;

	ethtool_convert_legacy_u32_to_link_mode(lnk->phydev->supported,
						applied);
	ethtool_convert_legacy_u32_to_link_mode(lnk->phydev->advertising,
						applied);
	lnk->phydev->speed        = SPEED_UNKNOWN;
	lnk->phydev->duplex       = DUPLEX_UNKNOWN;
}

/*
 * I2C bus addresses 0x50 and 0x51 are normally an EEPROM, which is
 * specified to be present in SFP modules.  These correspond with PHY
 * addresses 16 and 17.  Disallow access to these "phy" addresses.
 */
static bool _edgx_link_valid_phy_id(int phy_id)
{
	return phy_id != 0x10 && phy_id != 0x11;
}

static unsigned int _edgx_link_phy_addr(int phy_id)
{
	return phy_id + 0x40;
}

static int _edgx_link_i2c_mii_read(struct mii_bus *bus, int phy_id, int reg)
{
	struct i2c_adapter *i2c = bus->priv;
	struct i2c_msg msgs[2];
	u8 data[2], dev_addr = reg;
	int bus_addr, ret;

	if (!_edgx_link_valid_phy_id(phy_id))
		return 0;

	bus_addr = _edgx_link_phy_addr(phy_id);
	msgs[0].addr = bus_addr;
	msgs[0].flags = 0;
	msgs[0].len = 1;
	msgs[0].buf = &dev_addr;
	msgs[1].addr = bus_addr;
	msgs[1].flags = I2C_M_RD;
	msgs[1].len = sizeof(data);
	msgs[1].buf = data;

	ret = i2c_transfer(i2c, msgs, ARRAY_SIZE(msgs));
	if (ret != ARRAY_SIZE(msgs))
		return 0;

	return data[0] << 8 | data[1];
}

static int _edgx_link_i2c_mii_write(struct mii_bus *bus, int phy_id, int reg,
				    u16 val)
{
	struct i2c_adapter *i2c = bus->priv;
	struct i2c_msg msg;
	int ret;
	u8 data[3];

	if (!_edgx_link_valid_phy_id(phy_id))
		return 0;

	data[0] = reg;
	data[1] = val >> 8;
	data[2] = val;

	msg.addr = _edgx_link_phy_addr(phy_id);
	msg.flags = 0;
	msg.len = 3;
	msg.buf = data;

	ret = i2c_transfer(i2c, &msg, 1);

	return ret < 0 ? ret : 0;
}

int _edgx_link_mdio_alloc(struct edgx_link *lnk)
{
	struct mii_bus *mii;
	struct i2c_adapter *i2c = lnk->sfp.i2c_bus;

	if (!i2c_check_functionality(i2c, I2C_FUNC_I2C))
		return -EINVAL;

	mii = mdiobus_alloc();
	if (!mii)
		return -ENOMEM;

	mii->name = "edgx_mdio_i2c";
	mii->read = _edgx_link_i2c_mii_read;
	mii->write = _edgx_link_i2c_mii_write;
	mii->parent = &lnk->sfp.i2c_bus->dev;
	snprintf(mii->id, MII_BUS_ID_SIZE, "i2c:%s", dev_name(mii->parent));
	mii->priv = i2c;
	mii->phy_mask = ~0;
	if (IS_ERR(mii)) {
		edgx_pt_err(lnk->parent, "MII bus allocation fail\n");
		return -ENODEV;
	}
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

#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 3, 0)
	lnk->phydev  = mdiobus_scan(lnk->sfp.mii_bus, _SFP_PHY_ADDR);
#else
	lnk->phydev  = mdiobus_scan_c22(lnk->sfp.mii_bus, _SFP_PHY_ADDR);
#endif
	if (lnk->phydev  == ERR_PTR(-ENODEV)) {
		edgx_pt_err(lnk->parent, "PHY device not found on mdio bus\n");
		return -ENODEV;
	}
	// sleep after mdiobus_scan (which does phy reset)
	msleep(_T_PHY_RESET_MS);

	ret = phy_connect_direct(netdev, lnk->phydev, edgx_pt_link_change,
				 _edgx_link_adpt_itf(lnk));
	if (ret) {
		edgx_pt_err(lnk->parent, "PHY device not connected\n");
		phy_device_remove(lnk->phydev);
		phy_device_free(lnk->phydev);
		/*if phy_disconnect is called -> failure, so we set it to NULL*/
		lnk->phydev = NULL;
		_edgx_link_disconnect(lnk);
		return -ENODEV;
	}

	_edgx_link_phy_support(lnk);

	netdev->phydev = lnk->phydev;

	if (netif_running(netdev))
		edgx_link_start(lnk);

	return 0;
}

static void _edgx_link_sfp_work(struct work_struct *work_arg)
{
	struct edgx_link *lnk = container_of(work_arg, struct edgx_link,
					     work_lnk.work);
	u8 eth_compl = 0xFF;
	int ret = 0;

	if (!lnk->sfp.eeprom) {
		mutex_lock(&lnk->lock);
		lnk->sfp.eeprom = i2c_new_client_device(lnk->sfp.i2c_bus,
							&_sfp_eeprom);
		mutex_unlock(&lnk->lock);
	}

	eth_compl  = i2c_smbus_read_byte_data(lnk->sfp.eeprom, _SFP_ETH_COMPL);

	/*xxx Cureently we only support 1000base-t sfp (copper)*/
	/* How and will the .state field be used with fiber? Maybe remove.*/

	if ((eth_compl & _CMPL_1000BASET) && lnk->sfp.state != SFP_CONFIGURED) {
		ret = _edgx_link_configure_copper(lnk);
		if (!ret) {
			mutex_lock(&lnk->lock);
			lnk->sfp.state = SFP_CONFIGURED;
			mutex_unlock(&lnk->lock);
			edgx_pt_info(lnk->parent, "SFP %d plugged in...\n",
				     edgx_pt_get_id(lnk->parent));
		}
	}
	if (!(eth_compl & (1u << 3)) && lnk->sfp.state != SFP_NOT_PRESENT) {
		mutex_lock(&lnk->lock);
		lnk->sfp.state = SFP_NOT_PRESENT;
		mutex_unlock(&lnk->lock);
		edgx_pt_info(lnk->parent, "SFP %d plugged out...\n",
			     edgx_pt_get_id(lnk->parent));
		_edgx_link_disconnect(lnk);
		edgx_pt_link_change(edgx_pt_get_netdev(lnk->parent));
	}

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

	_edgx_link_disconnect(lnk);

	/*Workaround in order to be able to unload the module*/
	if (!strcmp(buf, "detach"))
		return count;

	/* We use an external i2c adapter, so it's the provider's duty to
	 * ensure, that the adapter is setup correctly to handle SFPs according
	 * to SFF-8472 (Rev. 11.0), e.g., 100 KHz max. speed.
	 */
	i2cdev = bus_find_device_by_name(&i2c_bus_type, NULL, buf);
	if (!i2cdev) {
		edgx_pt_err(pt, "Cannot find I2C adapter '%s'\n", buf);
		return -ENODEV;
	}

	mutex_lock(&lnk->lock);
	lnk->sfp.i2c_bus = to_i2c_adapter(i2cdev);
	lnk->sfp.state = SFP_INIT;
	lnk->sfp.eeprom = i2c_new_client_device(lnk->sfp.i2c_bus, &_sfp_eeprom);
	mutex_unlock(&lnk->lock);

	if (!lnk->sfp.eeprom) {
		edgx_pt_err(pt, "Cannot open I2C client for EEPROM\n");
		return -ENODEV;
	}

	schedule_delayed_work(&lnk->work_lnk, _SFP_CHECK_PERIOD);

	put_device(i2cdev);

	return count;
}

void edgx_link_set_delays(struct edgx_link  *lnk,
			  unsigned int dtx10min, unsigned int dtx10max,
			  unsigned int dtx100min, unsigned int dtx100max,
			  unsigned int dtx1000min, unsigned int dtx1000max,
			  unsigned int drx10min, unsigned int drx10max,
			  unsigned int drx100min, unsigned int drx100max,
			  unsigned int drx1000min, unsigned int drx1000max)
{
	lnk->phy_dly[_DLY_TX][_DLY_10][_DLY_MIN] = dtx10min;
	lnk->phy_dly[_DLY_TX][_DLY_10][_DLY_MAX] = dtx10max;
	lnk->phy_dly[_DLY_TX][_DLY_100][_DLY_MIN] = dtx100min;
	lnk->phy_dly[_DLY_TX][_DLY_100][_DLY_MAX] = dtx100max;
	lnk->phy_dly[_DLY_TX][_DLY_1000][_DLY_MIN] = dtx1000min;
	lnk->phy_dly[_DLY_TX][_DLY_1000][_DLY_MAX] = dtx1000max;
	lnk->phy_dly[_DLY_RX][_DLY_10][_DLY_MIN] = drx10min;
	lnk->phy_dly[_DLY_RX][_DLY_10][_DLY_MAX] = drx10max;
	lnk->phy_dly[_DLY_RX][_DLY_100][_DLY_MIN] = drx100min;
	lnk->phy_dly[_DLY_RX][_DLY_100][_DLY_MAX] = drx100max;
	lnk->phy_dly[_DLY_RX][_DLY_1000][_DLY_MIN] = drx1000min;
	lnk->phy_dly[_DLY_RX][_DLY_1000][_DLY_MAX] = drx1000max;
	lnk->dly_flags[_DLY_TX] = EDGX_DLY_MASK_10_100_1000;
	lnk->dly_flags[_DLY_RX] = EDGX_DLY_MASK_10_100_1000;
}

static ssize_t _delay_store(struct device *dev, const char *buf, size_t count,
			    enum _delay_id id, enum _delay_dir dir,
			    enum _delay_min_max limit)
{
	struct edgx_pt    *pt  = edgx_dev2pt(dev);
	struct edgx_link  *lnk = edgx_pt_get_link(pt);
	int ret;

	if (lnk->dly_flags[dir] & BIT(_NDLYMX * id + limit))
		return -EINVAL;

	ret = kstrtouint(buf, 10, &lnk->phy_dly[dir][id][limit]);

	if (!ret)
		lnk->dly_flags[dir] |= BIT(_NDLYMX * id + limit);

	return (ret) ? ret : count;
}

static ssize_t delay10tx_min_store(struct device *dev,
				   struct device_attribute *attr,
				   const char *buf, size_t count)
{
	return _delay_store(dev, buf, count, _DLY_10, _DLY_TX, _DLY_MIN);
}

static ssize_t delay10tx_max_store(struct device *dev,
				   struct device_attribute *attr,
				   const char *buf, size_t count)
{
	return _delay_store(dev, buf, count, _DLY_10, _DLY_TX, _DLY_MAX);
}

static ssize_t delay10rx_min_store(struct device *dev,
				   struct device_attribute *attr,
				   const char *buf, size_t count)
{
	return _delay_store(dev, buf, count, _DLY_10, _DLY_RX, _DLY_MIN);
}

static ssize_t delay10rx_max_store(struct device *dev,
				   struct device_attribute *attr,
				   const char *buf, size_t count)
{
	return _delay_store(dev, buf, count, _DLY_10, _DLY_RX, _DLY_MAX);
}

static ssize_t delay100tx_min_store(struct device *dev,
				    struct device_attribute *attr,
				    const char *buf, size_t count)
{
	return _delay_store(dev, buf, count, _DLY_100, _DLY_TX, _DLY_MIN);
}

static ssize_t delay100tx_max_store(struct device *dev,
				    struct device_attribute *attr,
				    const char *buf, size_t count)
{
	return _delay_store(dev, buf, count, _DLY_100, _DLY_TX, _DLY_MAX);
}

static ssize_t delay100rx_min_store(struct device *dev,
				    struct device_attribute *attr,
				    const char *buf, size_t count)
{
	return _delay_store(dev, buf, count, _DLY_100, _DLY_RX, _DLY_MIN);
}

static ssize_t delay100rx_max_store(struct device *dev,
				    struct device_attribute *attr,
				    const char *buf, size_t count)
{
	return _delay_store(dev, buf, count, _DLY_100, _DLY_RX, _DLY_MAX);
}

static ssize_t delay1000tx_min_store(struct device *dev,
				     struct device_attribute *attr,
				     const char *buf, size_t count)
{
	return _delay_store(dev, buf, count, _DLY_1000, _DLY_TX, _DLY_MIN);
}

static ssize_t delay1000tx_max_store(struct device *dev,
				     struct device_attribute *attr,
				     const char *buf, size_t count)
{
	return _delay_store(dev, buf, count, _DLY_1000, _DLY_TX, _DLY_MAX);
}

static ssize_t delay1000rx_min_store(struct device *dev,
				     struct device_attribute *attr,
				     const char *buf, size_t count)
{
	return _delay_store(dev, buf, count, _DLY_1000, _DLY_RX, _DLY_MIN);
}

static ssize_t delay1000rx_max_store(struct device *dev,
				     struct device_attribute *attr,
				     const char *buf, size_t count)
{
	return _delay_store(dev, buf, count, _DLY_1000, _DLY_RX, _DLY_MAX);
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
	NULL,
};

static struct attribute_group phy_group = {
	.name  = "phy",
	.attrs = phy_attrs,
};

static void edgx_link_set_lnk_mode(struct edgx_link *lnk,
				   enum edgx_lnk_mode lnk_mode,
				   bool internal)
{
	lnk->lnk_mode = lnk_mode;

	switch (lnk_mode) {
	case EDGX_LNKMOD_10_100_1000:
			lnk->speed = SPEED_1000;
			lnk->supported = SUPPORTED_10baseT_Full |
					 SUPPORTED_100baseT_Full |
					 SUPPORTED_1000baseT_Full;
			break;
	case EDGX_LNKMOD_100_1000:
			lnk->speed = SPEED_1000;
			lnk->supported = SUPPORTED_1000baseT_Full;
			if (!internal)
				lnk->supported |= SUPPORTED_100baseT_Full;
			break;
	case EDGX_LNKMOD_10_100:
		lnk->speed = SPEED_100;
		lnk->supported = SUPPORTED_100baseT_Full;
		if (!internal)
			lnk->supported |= SUPPORTED_10baseT_Full;
		break;
	case EDGX_LNKMOD_100:
		lnk->speed     = SPEED_100;
		lnk->supported = SUPPORTED_100baseT_Full;
		break;
	case EDGX_LNKMOD_INVAL:
	default:
		lnk->speed = SPEED_UNKNOWN;
		lnk->supported = 0;
		break;
	}
}

int  edgx_link_init(struct edgx_pt *pt, struct edgx_link **lnk,
		    enum edgx_lnk_mode lnk_mode,
		    bool internal)
{
	const struct edgx_ifreq ifreq = { .id = AC_ADPT_ID, .v_maj = 1 };
	struct edgx_ifdesc ptifd;
	const struct edgx_ifdesc *adptif;
	int i;
	u16 reg;

	*lnk = kzalloc(sizeof(**lnk), GFP_KERNEL);
	if (!(*lnk))
		return -ENOMEM;

	(*lnk)->parent = pt;

	edgx_link_set_lnk_mode(*lnk, lnk_mode, internal);

	if (PT_IS_EP_ID(edgx_pt_get_id(pt))) {
		goto end_adpt;
	}

	adptif = edgx_ac_get_ptif(&ifreq, edgx_pt_get_id(pt), &ptifd);
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
				     phy_modes((*lnk)->adpt.desc->itf));
			goto end_adpt;
		}

	if (reg != _ADPT_NONE_ID)
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
