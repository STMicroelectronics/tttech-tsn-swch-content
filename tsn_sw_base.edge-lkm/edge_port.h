/* SPDX-License-Identifier: GPL-2.0 */
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

#ifndef _EDGE_PORT_H
#define _EDGE_PORT_H

#include <linux/ethtool.h>
#include <net/switchdev.h>

#include "edge_bridge.h"
#include "edge_com.h"

struct edgx_pt;

typedef void (*edgx_pt_link_cb)(struct edgx_pt *pt, int link, int speed);

int edgx_probe_brports(struct edgx_br *br,
		       enum edgx_lnk_mode lnk_mode,
		       struct edgx_pt **ppt);
int edgx_init_epport(struct edgx_br *br,
		     enum edgx_lnk_mode lnk_mode,
		     struct edgx_pt **ppt);

void edgx_shutdown_brports(struct edgx_pt **ppt);
void edgx_shutdown_epport(struct edgx_pt *pt);

struct edgx_link  *edgx_net2link(struct net_device *netdev);

ptid_t             edgx_pt_get_id(struct edgx_pt *pt);
struct edgx_time  *edgx_pt_get_time(struct edgx_pt *pt);
const char        *edgx_pt_get_name(struct edgx_pt *pt);
struct edgx_br    *edgx_pt_get_br(struct edgx_pt *pt);
struct edgx_link  *edgx_pt_get_link(struct edgx_pt *pt);
struct edgx_fqtss *edgx_pt_get_fqtss(struct edgx_pt *pt);
struct edgx_st    *edgx_pt_get_st(struct edgx_pt *pt);
struct net_device *edgx_pt_get_netdev(struct edgx_pt *pt);
struct edgx_preempt *edgx_pt_get_preempt(struct edgx_pt *pt);

int                edgx_pt_get_speed(struct edgx_pt *pt);
u32                edgx_pt_get_framesize(struct edgx_pt *pt, int idx);
void               edgx_pt_set_framesize(struct edgx_pt *pt, int idx, u32 val);

ktime_t            edgx_pt_get_g2omax(struct edgx_pt *pt);
ktime_t            edgx_pt_get_g2omin(struct edgx_pt *pt);
ktime_t            edgx_pt_get_i2gmax(struct edgx_pt *pt);
ktime_t            edgx_pt_get_i2gmin(struct edgx_pt *pt);
ktime_t            edgx_pt_get_i2sgmax(struct edgx_pt *pt);
ktime_t            edgx_pt_get_i2sgmin(struct edgx_pt *pt);
int edgx_pt_get_g2o_lo(struct edgx_pt *pt, ktime_t *g2o_min,
		       ktime_t *g2o_max);
int edgx_pt_get_g2o_hi(struct edgx_pt *pt, ktime_t *g2o_min,
		       ktime_t *g2o_max);
int edgx_pt_get_i2sg_hi(struct edgx_pt *pt, ktime_t *i2sg_min,
			ktime_t *i2sg_max);
void edgx_pt_set_sg_delay(struct edgx_pt *pt, ktime_t sg_dly);
void edgx_pt_set_delay_adjust(struct edgx_pt *pt,
			      int idtx10min, int idtx10max,
			      int idtx100min, int idtx100max,
			      int idtx1000min, int idtx1000max,
			      int idrx10min, int idrx10max,
			      int idrx100min, int idrx100max,
			      int idrx1000min, int idrx1000max);

void edgx_pt_link_change(struct net_device *netdev);

void edgx_pt_set_pvid(struct edgx_pt *pt, u16 vid);
void edgx_pt_clear_pvid(struct edgx_pt *pt);
u16 edgx_pt_get_pvid(struct edgx_pt *pt);

void edgx_pt_set_fid_fwd_state(struct edgx_pt *pt, fid_t fid, u8 ptstate);

void edgx_pt_rcv(struct edgx_pt *pt, struct sk_buff *skb, ptflags_t flags);

void edgx_pt_add_sysfs(struct edgx_pt *pt, struct attribute_group *grp);
void edgx_pt_rem_sysfs(struct edgx_pt *pt, struct attribute_group *grp);

struct edgx_pt *edgx_dev2pt(struct device *dev);
ptid_t          edgx_dev2ptid(struct device *dev);

int edgx_pt_call_switchdev_notifiers(struct edgx_pt *pt, unsigned long type,
				     struct switchdev_notifier_info *info);

#define edgx_pt_err(_pt, _fmt, ...) pr_err("%s: " _fmt,               \
					edgx_pt_get_name(_pt), ##__VA_ARGS__)

#define edgx_pt_warn(_pt, _fmt, ...) pr_warn("%s: " _fmt,          \
					edgx_pt_get_name(_pt), ##__VA_ARGS__)

#define edgx_pt_info(_pt, _fmt, ...) pr_info("%s: " _fmt,             \
					edgx_pt_get_name(_pt), ##__VA_ARGS__)

void edgx_pt_stats(struct net_device *netdev,
		   struct ethtool_stats *cmd, u64 *pt);
void edgx_pt_strings(struct net_device *netdev, u32 stringset, u8 *pt_num);
int edgx_pt_sset_count(struct net_device *netdev, int stringset_id);

#endif /* _EDGE_PORT_H */
