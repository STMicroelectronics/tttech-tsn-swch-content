/* SPDX-License-Identifier: GPL-2.0 */
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

#ifndef _EDGE_LINK_H
#define _EDGE_LINK_H

#include "edge_port.h"

struct edgx_link;

int edgx_link_ioctl(struct net_device *netdev, struct ifreq *ifr, int cmd);

int edgx_link_nway_reset(struct net_device *netdev);

int edgx_link_get_ksettings(struct net_device *netdev,
			    struct ethtool_link_ksettings *cmd);
int edgx_link_set_ksettings(struct net_device *netdev,
			    const struct ethtool_link_ksettings *cmd);

void edgx_link_start(struct edgx_link *lnk);
void edgx_link_stop(struct edgx_link *lnk);
int edgx_link_get_speed(struct edgx_link *lnk);
int edgx_link_get_state(struct edgx_link *lnk);
ktime_t edgx_link_get_tx_delay_min(struct edgx_link *lnk);
ktime_t edgx_link_get_tx_delay_max(struct edgx_link *lnk);
ktime_t edgx_link_get_rx_delay_min(struct edgx_link *lnk);
ktime_t edgx_link_get_rx_delay_max(struct edgx_link *lnk);
ktime_t edgx_link_get_offline_tx_delay_min(struct edgx_link *lnk, int speed);
ktime_t edgx_link_get_offline_tx_delay_max(struct edgx_link *lnk, int speed);
ktime_t edgx_link_get_offline_rx_delay_min(struct edgx_link *lnk, int speed);
ktime_t edgx_link_get_offline_rx_delay_max(struct edgx_link *lnk, int speed);
void edgx_link_update_speed(struct edgx_link *lnk, int speed);
void edgx_link_update_delays(struct edgx_link *lnk, int speed);
int edgx_link_is_external(struct edgx_link *lnk);
int edgx_link_set_mdiobus(struct edgx_link *lnk, const char *mdiobus_id);
void edgx_link_set_delays(struct edgx_link *lnk, int speed,
			  unsigned int dtxmin, unsigned int dtxmax,
			  unsigned int drxmin, unsigned int drxmax);
edgx_speed_mask_t edgx_link_get_speed_mask(struct edgx_link *lnk);
int edgx_link_get_rx_delays_hi(struct edgx_link *lnk,
			       ktime_t *drx_min, ktime_t *drx_max);
int edgx_link_get_speed_tx_delays(struct edgx_link *lnk, int speed,
				  ktime_t *dtx_min, ktime_t *dtx_max);
int edgx_link_get_speed_rx_delays(struct edgx_link *lnk, int speed,
				  ktime_t *dtx_min, ktime_t *dtx_max);
int edgx_link_init(struct edgx_pt *pt, struct edgx_link **lnk,
		   edgx_speed_mask_t spd_mask,
		   bool internal);
void edgx_link_shutdown(struct edgx_link *lnk);

#endif /* _EDGE_LINK_H */
