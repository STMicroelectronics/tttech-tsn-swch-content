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

#ifndef _EDGE_BR_FDB_H
#define _EDGE_BR_FDB_H

#include <net/switchdev.h>
#include "edge_bridge.h"

struct edgx_brfdb;

//xxx to be removed
struct edgx_sid_params {
	u16 str_hdl;
	u16 vid;
	u8  addr[ETH_ALEN];
	u8 id_type; //1->NULL 0->SRC
	u8 col;
	u16 row;
};

enum sid_ident_type {
	SID_UNKNOWN  = 0,
	SID_NULL = 1,
	SID_SOURCE = 2,
	SID_ACTIVE = 3,
	SID_IPSTR = 4,
	SID_IDENT_MAX
};

/* Struct used for handling stream parameters and red-black tree
 * for Stream Identification
 */
struct edgx_sid {
	struct rb_node rbnode;
	enum sid_ident_type id_type;
	u8  addr[ETH_ALEN];
	u16 vid;
	u16 str_hdl; //key
	u16 port_mask;
	bool valid_fdb;
};

typedef int edgx_brfdb_cb_t(struct sk_buff *skb, struct netlink_callback *cb,
			    struct net_device *dev,
			    const unsigned char *addr, u16 vid, int *idx);

int edgx_brfdb_dump(struct edgx_brfdb *brfdb, struct sk_buff *skb,
		    struct netlink_callback *ncb, struct net_device *dev,
		    edgx_brfdb_cb_t *cb, ptid_t ptid, int *idx);

int  edgx_brfdb_init(struct edgx_br *br, edgx_io_t *iobase,
		     struct edgx_brfdb **brfdb);
void edgx_brfdb_shutdown(struct edgx_brfdb *brfdb);
int  edgx_brfdb_init_dmac_sync(struct edgx_brfdb *brfdb);
void edgx_brfdb_shutdown_dmac_sync(struct edgx_brfdb *brfdb);

int edgx_brfdb_add(struct edgx_brfdb *brfdb,
		   struct switchdev_notifier_fdb_info *info,
		   ptid_t ptid);

int edgx_brfdb_sid_add(struct edgx_brfdb *brfdb, struct edgx_sid *sid);

int edgx_brfdb_del(struct edgx_brfdb *brfdb,
		   struct switchdev_notifier_fdb_info *info,
		   ptid_t ptid);

int edgx_brfdb_sid_del(struct edgx_brfdb *brfdb, u8 *addr, u16 vid);

unsigned int edgx_brfdb_sz(struct edgx_brfdb *brfdb);

unsigned int edgx_brfdb_nsmac(struct edgx_brfdb *brfdb);

void edgx_brfdb_flush(struct edgx_brfdb *brfdb, u8 fid);

void edgx_brfdb_sid_get_smac(struct edgx_brfdb *brfdb, u8 col, u16 row,
			     u8 *addr, u16 *vid);
u16 edgx_brfdb_sid_get_max_str(struct edgx_brfdb *brfdb);

void edgx_brfdb_update_vid2fid(struct edgx_brfdb *brfdb, vid_t vid,
			       fid_t fid_new, fid_t fid_old);

#endif /* _EDGE_BR_FDB_H */
