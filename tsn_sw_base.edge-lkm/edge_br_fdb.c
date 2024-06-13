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

#include "linux/bitops.h"
#include "linux/etherdevice.h"
#include "linux/workqueue.h"
#include "linux/bitfield.h"
#include "linux/rbtree.h"

#include "edge_br_fdb.h"
#include "edge_br_vlan.h"
#include "edge_port.h"
#include "edge_util.h"

struct smac_entry {
	u16 general;
	u8  mac[ETH_ALEN];
	u16 fwd_mask;
	u16 policed_pts;
	u16 policer;
	u16 vlan;
	u16 stream_no;    /* Future, for Stream Identification (802.1CB) */
};

struct dmac_entry {
	struct rb_node rbnode;
	fid_t          change_detector : 1;  /* New/refresh vs. aged */
	fid_t          fid             : 12; /* Big enough for max. supported */
	u8             mac[ETH_ALEN];
	ptid_t         ptid;
	ptid_t         ptid_new;
};

struct edgx_dmac {
	struct edgx_br      *parent;
	edgx_io_t           *iobase;
	struct mutex         lock;
	struct delayed_work  work_sync;           /* DMAC to bridge FDB sync */
	struct rb_root       root;                /* Current DMAC entries */
	fid_t                detector_state : 1;  /* Match change_detector */
};

struct edgx_brfdb {
	struct edgx_br      *parent;
	edgx_io_t           *iobase;
	u16                  nrows;
	unsigned int         sents;  /* Number of static entries */
	struct mutex         lock;   /* SMAC table lock */
	struct edgx_dmac     dmac;   /* Dynamic MAC address table state */
};

#define SMAC_CTL_POS(row, col)  ((((col) & 0x3) << 12) | ((row) & 0xFFF))
#define SMAC_CTL_RD(row, col)   (BIT(15) | SMAC_CTL_POS(row, col))
#define SMAC_CTL_WR(row, col)   (BIT(15) | BIT(14) | SMAC_CTL_POS(row, col))
#define SMAC_CTL_BUSY(ctrl)     ((ctrl) & BIT(15))

#define SMAC_NCOLS              (4)   /* Number of SMAC columns */
#define NCOLS_VLAN              (2)   /* First two columns for specific VLANs*/
#define COL_VLAN                (1)   /* VLAN-aware column */
#define COL_WILDCARD            (0)   /* VLAN-wildcard column */
#define VID_WILDCARD            (0)   /* switchdev wildcard ("for all VIDs") */

/* If defined, DMAC address table synchronization to Linux bridge via switchdev
 * FDB notifications is enabled automatically
 */
/* #define DMAC_SYNC_AUTO_ENABLE */
#define DMAC_SYNC_DELAY_MS      60000 /* Wait time until next sync */

#define DMAC_TABLE0             0x200
#define DMAC_TABLE0_PORT        GENMASK(3, 0)
#define DMAC_TABLE0_TRANSFER    BIT(15)

#define DMAC_TABLE1             0x202
#define DMAC_TABLE2             0x204
#define DMAC_TABLE3             0x206

#define DMAC_TABLE4             0x208
#define DMAC_TABLE4_FID         GENMASK(5, 0)

#define DMAC_INV_FID            U16_MAX

/* CRC40 lookup table for polynomial 0x0104c11db7. */
static const u64 crc40tbl[] = {
	0x000000000ull, 0x104c11db7ull, 0x209823b6eull, 0x30d4326d9ull,
	0x4130476dcull, 0x517c56b6bull, 0x61a864db2ull, 0x71e475005ull,
	0x82608edb8ull, 0x922c9f00full, 0xa2f8ad6d6ull, 0xb2b4bcb61ull,
	0xc350c9b64ull, 0xd31cd86d3ull, 0xe3c8ea00aull, 0xf384fbdbdull,
};

/* "Broadcast" MAC, terminating dynamic FDB traversal */
static const u8 mac_bcast[ETH_ALEN] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

/* We do a 50:50 split on the columns.
 * Columns 0 and 1 are used for FDB-entries applicable to specific VLANs.
 * Columns 2 and  are used for FDB-entries applicable to all VLANs.
 */
static const u8 column[SMAC_NCOLS] = { COL_VLAN,     COL_VLAN,
				       COL_WILDCARD, COL_WILDCARD };

static inline bool edgx_brfdb_is_fdb_used(struct smac_entry *e)
{
	return ((e->general & BIT(15)) != 0);
}

static inline bool edgx_brfdb_is_sid_used(struct smac_entry *e)
{
	return ((e->general & BIT(6)) != 0);
}

static inline bool edgx_brfdb_is_smac_match(struct smac_entry *e,
					    const unsigned char *mac, u16 vid)
{
	return ((!ether_addr_cmp(mac, e->mac)) && (vid == e->vlan));
}

static inline bool edgx_brfdb_is_sid_fit(struct smac_entry *e,
					 const unsigned char *mac, u16 vid)
{
	if (!edgx_brfdb_is_sid_used(e))
		return false;
	return edgx_brfdb_is_smac_match(e, mac, vid);
}

static inline bool edgx_brfdb_is_fdb_fit(struct smac_entry *e,
					 const unsigned char *mac, u16 vid)
{
	if (!edgx_brfdb_is_fdb_used(e))
		return false;
	return edgx_brfdb_is_smac_match(e, mac, vid);
}

static inline bool edgx_brfdb_col_is_vid(u16 col, u16 vid)
{
	if (vid == VID_WILDCARD)
		return (column[col] == COL_WILDCARD);
	return (column[col] == COL_VLAN);
}

static void edgx_brfdb_calc_smac_rows(struct edgx_brfdb *brfdb,
				      const unsigned char *mac, u16 vid,
				      u16 *row)
{
	unsigned int i;
	u64 hash;
	u64 extra;
	u64 crc = ~((u64)0);

	for (i = 0; i < ETH_ALEN; i++) {
		crc = (crc << 4) ^
		      crc40tbl[(((u8)(crc >> 36) & 0xfu) ^ (mac[i] >> 4))];
		crc = (crc << 4) ^
		      crc40tbl[(((u8)(crc >> 36) & 0xfu) ^ (mac[i] & 0xfu))];
	}
	crc &= 0xffffffffffull;
	/* Split the 12-bit column hashes of CRC40 to a 64-bit value.
	 * Use 16 bits (not 12) for each column hash.
	 */
	extra = ((mac[3] ^ mac[4]) & 0x3) << 10;
	hash =   (extra | ((crc >>  0) & 0x3ff)) <<  0;
	hash |=  (extra | ((crc >> 10) & 0x3ff)) << 16;
	hash |=  (extra | ((crc >> 20) & 0x3ff)) << 32;
	hash |=  (extra | ((crc >> 30) & 0x3ff)) << 48;
	if (vid != VID_WILDCARD)
		/* XOR each column hash with VLAN ID. */
		hash ^= ((u64)vid << 48) |
			((u64)vid << 32) |
			((u64)vid << 16) |
			((u64)vid << 0);
	/* Extract and trucate rows */
	row[0] = hash & (brfdb->nrows - 1);
	row[1] = (hash >> 16) & (brfdb->nrows - 1);
	row[2] = (hash >> 32) & (brfdb->nrows - 1);
	row[3] = (hash >> 48) & (brfdb->nrows - 1);
}

static void edgx_brfdb_smac_get(struct edgx_brfdb *brfdb, u16 col, u16 row,
				struct smac_entry *e)
{
	u16 ctrl = SMAC_CTL_RD(row, col);

	edgx_wr16(brfdb->iobase, 0x220, ctrl);
	do {
		ctrl = edgx_rd16(brfdb->iobase, 0x220);
	} while (SMAC_CTL_BUSY(ctrl));

	e->general           = edgx_rd16(brfdb->iobase, 0x230);
	*((u16 *)&e->mac[0]) = edgx_rd16(brfdb->iobase, 0x232);
	*((u16 *)&e->mac[2]) = edgx_rd16(brfdb->iobase, 0x234);
	*((u16 *)&e->mac[4]) = edgx_rd16(brfdb->iobase, 0x236);
	e->fwd_mask          = edgx_rd16(brfdb->iobase, 0x238);
	e->policed_pts       = 0; /* unused */
	e->policer           = 0; /* unused */
	e->vlan              = edgx_rd16(brfdb->iobase, 0x23E);
	e->stream_no	     = edgx_rd16(brfdb->iobase, 0x240);
}

static void edgx_brfdb_smac_set(struct edgx_brfdb *brfdb, u16 col, u16 row,
				struct smac_entry *e, bool sid)
{
	u16 ctrl = SMAC_CTL_WR(row, col);

	edgx_wr16(brfdb->iobase, 0x230, e->general);
	edgx_wr16(brfdb->iobase, 0x232, *((u16 *)&e->mac[0]));
	edgx_wr16(brfdb->iobase, 0x234, *((u16 *)&e->mac[2]));
	edgx_wr16(brfdb->iobase, 0x236, *((u16 *)&e->mac[4]));
	edgx_wr16(brfdb->iobase, 0x23A, 0);
	edgx_wr16(brfdb->iobase, 0x23C, 0);
	edgx_wr16(brfdb->iobase, 0x23E, e->vlan);
	edgx_wr16(brfdb->iobase, 0x238, e->fwd_mask);

	if (sid)
		edgx_wr16(brfdb->iobase, 0x240, e->stream_no);

	edgx_wr16(brfdb->iobase, 0x220, ctrl);
	do {
		ctrl = edgx_rd16(brfdb->iobase, 0x220);
	} while (SMAC_CTL_BUSY(ctrl));
}

static inline void edgx_brfdb_set_general(struct smac_entry *e)
{
	/* If no ports are left in fwd-mask, set entry unused again */
	if (e->fwd_mask)
		e->general |= BIT(15);
	else
		e->general &= ~BIT(15);

	if (e->vlan == VID_WILDCARD)
		e->general &= ~BIT(12); /* clear VLAN matching */
	else
		e->general |= BIT(12);  /* set VLAN matching */
}

u16 edgx_brfdb_sid_get_max_str(struct edgx_brfdb *brfdb)
{
	u16 frer_streams =  edgx_br_get_generic(brfdb->parent, BR_GX_NSTREAMS);
	u32 fdb_streams = edgx_brfdb_sz(brfdb) / 2;
	/* Maximum supported streams is the minimum between number of supported
	 * streams according to register FRER_STREAMS value and number of
	 * entries in the first two columns in FDB (only first two columns
	 * support VLAN tagged frames)
	 */
	if (fdb_streams < (1 << frer_streams))
		return (u16)fdb_streams;
	else
		return 1 << frer_streams;
}

void edgx_brfdb_sid_get_smac(struct edgx_brfdb *brfdb, u8 col, u16 row,
			     u8 *addr, u16 *vid)
{
	struct smac_entry e;

	mutex_lock(&brfdb->lock);
	edgx_brfdb_smac_get(brfdb, col, row, &e);
	mutex_unlock(&brfdb->lock);

	ether_addr_copy(addr, e.mac);
	*vid = e.vlan;
}

int edgx_brfdb_sid_add(struct edgx_brfdb *brfdb, struct edgx_sid *sid)
{
	u16 row[SMAC_NCOLS];
	struct smac_entry e[SMAC_NCOLS];
	unsigned int i;
	int ret = 0;

	edgx_brfdb_calc_smac_rows(brfdb, sid->addr, sid->vid, row);
	mutex_lock(&brfdb->lock);

	/* Get all entries on hash positions only columns 1 2 (VLAN aware)*/
	for (i = 0; i < NCOLS_VLAN; i++)
		edgx_brfdb_smac_get(brfdb, i, row[i], &e[i]);

	for (i = 0; i < NCOLS_VLAN; i++)
		if (edgx_brfdb_is_fdb_fit(&e[i], sid->addr, sid->vid)) {
			if ((e[i].stream_no == sid->str_hdl) &&
					(edgx_brfdb_is_sid_used(&e[i]))){
				/* Same entry in fdb, nothing to be done
				 * especially for SID = 0 it is important that it is checked
				 * if flag for "sid in use" is set*/
				goto out_done;
			}
		}
	/* Try to use existing entry with unused sid or try to create new entry ... */
	for (i = 0; i < NCOLS_VLAN; i++) {
		if (!edgx_brfdb_is_sid_used(&e[i])) {
			/* we use this entry */
			ether_addr_copy(e[i].mac, sid->addr);
			e[i].vlan = sid->vid;
			e[i].general |= BIT(6) | BIT(12);
			e[i].general |= (sid->id_type) ? BIT(7) : 0;
			e[i].stream_no = sid->str_hdl;
			edgx_brfdb_smac_set(brfdb, i, row[i], &e[i], 1);
			brfdb->sents++;
			sid->valid_fdb = 1;

			goto out_done;
		}
	}
	/* No re-usable entry and no possible entry left, set return code */
	ret = -ENOMEM;

out_done:
	mutex_unlock(&brfdb->lock);
	return ret;

	return 0;
}

int edgx_brfdb_add(struct edgx_brfdb *brfdb,
		   struct switchdev_notifier_fdb_info *info,
		   ptid_t ptid)
{
	u16 row[SMAC_NCOLS];
	struct smac_entry entry[SMAC_NCOLS];
	unsigned int i;
	int r = 0;

	edgx_brfdb_calc_smac_rows(brfdb, info->addr, info->vid, row);

	/* RTNL lock is not sufficient, since SMAC table will also be used
	 *for Stream lookup in the future
	 */
	mutex_lock(&brfdb->lock);

	/* Get all entries on hash positions */
	for (i = 0; i < SMAC_NCOLS; i++)
		edgx_brfdb_smac_get(brfdb, i, row[i], &entry[i]);

	/* Try to add port to existing entry ... */
	for (i = 0; i < SMAC_NCOLS; i++)
		if (edgx_brfdb_is_fdb_fit(&entry[i], info->addr, info->vid)) {
			entry[i].fwd_mask |= BIT(ptid);
			edgx_brfdb_set_general(&entry[i]);
			/*As we use an existing entry we have to check if SID is used,
			 * and in this case we have to write the stream no to the register
			 * (parameter "1" in call ) edgx_brfdb_smac_set
			 */
			if (edgx_brfdb_is_sid_used(&entry[i]))
				edgx_brfdb_smac_set(brfdb, i, row[i], &entry[i], 1);
			else
				edgx_brfdb_smac_set(brfdb, i, row[i], &entry[i], 0);
			goto out_done;
		}

	/* Try to create new entry ... */
	for (i = 0; i < SMAC_NCOLS; i++)
		if (!edgx_brfdb_is_fdb_used(&entry[i]) &&
		    edgx_brfdb_col_is_vid(i, info->vid)) {
			/* we use this entry */
			ether_addr_copy(entry[i].mac, info->addr);
			entry[i].fwd_mask    = BIT(ptid);
			entry[i].policed_pts = 0;
			entry[i].policer     = 0;
			entry[i].vlan        = info->vid;
			edgx_brfdb_set_general(&entry[i]);
			/* As we write a new entry here we can be sure that sid isn't used. */
			edgx_brfdb_smac_set(brfdb, i, row[i], &entry[i], 0);
			brfdb->sents++;
			goto out_done;
		}

	/* No re-usable entry and no possible entry left, set return code */
	r = -ENOMEM;

out_done:
	mutex_unlock(&brfdb->lock);
	return r;
}

int edgx_brfdb_sid_del(struct edgx_brfdb *brfdb, u8 *addr, u16 vid)
{
	struct smac_entry entry[NCOLS_VLAN];
	u16 row[SMAC_NCOLS];
	u8 i;

	edgx_brfdb_calc_smac_rows(brfdb, addr, vid, row);

	mutex_lock(&brfdb->lock);
	/* Get all entries on hash positions */
	for (i = 0; i < NCOLS_VLAN; i++)
		edgx_brfdb_smac_get(brfdb, i, row[i], &entry[i]);

	for (i = 0; i < NCOLS_VLAN; i++)
		if (edgx_brfdb_is_sid_fit(&entry[i], addr, vid)) {
			entry[i].general &=  ~BIT(6);
			edgx_brfdb_smac_set(brfdb, i, row[i], &entry[i], 1);
			break;
		}

	mutex_unlock(&brfdb->lock);

	return 0;
}

int edgx_brfdb_del(struct edgx_brfdb *brfdb,
		   struct switchdev_notifier_fdb_info *info,
		   ptid_t ptid)
{
	u16 row[SMAC_NCOLS];
	struct smac_entry entry[SMAC_NCOLS];
	unsigned int i;

	edgx_brfdb_calc_smac_rows(brfdb, info->addr, info->vid, row);

	/* RTNL lock is not sufficient, since SMAC table will also be used
	 *for Stream lookup in the future
	 */
	mutex_lock(&brfdb->lock);

	/* Get all entries on hash positions */
	for (i = 0; i < SMAC_NCOLS; i++)
		edgx_brfdb_smac_get(brfdb, i, row[i], &entry[i]);

	for (i = 0; i < SMAC_NCOLS; i++)
		if (edgx_brfdb_is_fdb_fit(&entry[i], info->addr, info->vid)) {
			entry[i].fwd_mask &= ~BIT(ptid);
			edgx_brfdb_set_general(&entry[i]);
			/*As we use an existing entry we have to check if SID is used,
			 * and in this case we have to write the stream no to the register
			 * (parameter "1" in call ) edgx_brfdb_smac_set
			 */
			if (edgx_brfdb_is_sid_used(&entry[i]))
				edgx_brfdb_smac_set(brfdb, i, row[i], &entry[i], 1);
			else
				edgx_brfdb_smac_set(brfdb, i, row[i], &entry[i], 0);
			if (!entry[i].fwd_mask)
				brfdb->sents--;
			break;
		}

	mutex_unlock(&brfdb->lock);
	return 0;
}

int edgx_brfdb_dump(struct edgx_brfdb *brfdb, struct sk_buff *skb,
		    struct netlink_callback *ncb, struct net_device *dev,
		    edgx_brfdb_cb_t *cb, ptid_t ptid, int *idx)
{
	unsigned int row, col;
	struct smac_entry entry;
	u16 pt_mask = BIT(ptid);
	int stored_r = 0;

	/* For all rows ... */
	for (row = 0; row < brfdb->nrows; row++)
		/* ... and all columns. */
		for (col = 0; col < SMAC_NCOLS; col++) {
			mutex_lock(&brfdb->lock);
			edgx_brfdb_smac_get(brfdb, col, row, &entry);

			/* No need to match fdb->vid too; bridge does the
			 * filtering itself and always sets fdb->vid = 0
			 */
			if (entry.fwd_mask & pt_mask &&
			    edgx_brfdb_is_fdb_used(&entry)) {
				int r = 0;

				r = cb(skb, ncb, dev,
				       entry.mac, entry.vlan, idx);
				if (r)
					stored_r = r;
			}
			mutex_unlock(&brfdb->lock);
		}
	return stored_r;
}

unsigned int edgx_brfdb_sz(struct edgx_brfdb *brfdb)
{
	return (brfdb) ? brfdb->nrows * SMAC_NCOLS : 0;
}

unsigned int edgx_brfdb_nsmac(struct edgx_brfdb *brfdb)
{
	return (brfdb) ? brfdb->sents : 0;
}

static inline void edgx_brfdb_rb_erase(struct rb_node *node,
				       struct rb_root *root)
{
	rb_erase(node, root);
	RB_CLEAR_NODE(node);
}

static struct dmac_entry *edgx_dmac_create(const u8 *mac, fid_t fid,
					   ptid_t ptid)
{
	struct dmac_entry *entry;

	entry = kzalloc(sizeof(*entry), GFP_KERNEL);
	if (!entry)
		return NULL;
	ether_addr_copy(entry->mac, mac);
	entry->fid = fid;
	entry->ptid = ptid;
	entry->ptid_new = ptid;
	return entry;
}

static inline int edgx_dmac_cmp(fid_t lfid, const u8 *lmac,
				fid_t rfid, const u8 *rmac)
{
	if (lfid < rfid)
		return -1;
	if (lfid > rfid)
		return 1;
	return ether_addr_cmp(lmac, rmac);
}

static int edgx_dmac_insert(struct edgx_dmac *dmac, struct dmac_entry *new)
{
	struct rb_node **pp = &dmac->root.rb_node;
	struct rb_node *p = NULL;
	struct dmac_entry *entry = NULL;
	int cmp;

	while (*pp) {
		p = *pp;
		entry = rb_entry(p, struct dmac_entry, rbnode);
		cmp = edgx_dmac_cmp(new->fid, new->mac, entry->fid, entry->mac);
		if (cmp < 0)
			pp = &(*pp)->rb_left;
		else if (cmp > 0)
			pp = &(*pp)->rb_right;
		else
			return -EINVAL;
	}

	rb_link_node(&new->rbnode, p, pp);
	rb_insert_color(&new->rbnode, &dmac->root);
	return 0;
}

static struct dmac_entry *edgx_dmac_search(struct rb_root *root,
					   fid_t fid, const u8 *mac)
{
	struct rb_node *node = root->rb_node;

	while (node) {
		struct dmac_entry *entry =
				rb_entry(node, struct dmac_entry, rbnode);
		int cmp = edgx_dmac_cmp(fid, mac, entry->fid, entry->mac);

		if (cmp < 0)
			node = node->rb_left;
		else if (cmp > 0)
			node = node->rb_right;
		else
			return entry;
	}
	return NULL;
}

static void edgx_dmac_erase(struct rb_root *root, struct dmac_entry *entry)
{
	edgx_brfdb_rb_erase(&entry->rbnode, root);
	kfree(entry);
}

static int edgx_dmac_read_next(struct edgx_dmac *dmac)
{
	struct dmac_entry *entry = NULL;
	u8 mac[ETH_ALEN];
	u16 *addr = (u16 *)mac;
	int ret = 0;
	fid_t fid;
	ptid_t ptid;
	u16 reg;

	edgx_wr16(dmac->iobase, DMAC_TABLE0, DMAC_TABLE0_TRANSFER);
	usleep_range(300, 400);
	do {
		reg = edgx_rd16(dmac->iobase, DMAC_TABLE0);
	} while (reg & DMAC_TABLE0_TRANSFER);

	addr[0] = edgx_rd16(dmac->iobase, DMAC_TABLE1);
	addr[1] = edgx_rd16(dmac->iobase, DMAC_TABLE2);
	addr[2] = edgx_rd16(dmac->iobase, DMAC_TABLE3);
	if (ether_addr_cmp(addr, mac_bcast) == 0)
		return -ENOENT;

	fid = edgx_rd16(dmac->iobase, DMAC_TABLE4);
	ptid = FIELD_GET(DMAC_TABLE0_PORT, reg);
	entry = edgx_dmac_search(&dmac->root, fid, mac);
	if (!entry) {
		entry = edgx_dmac_create(mac, fid, ptid);
		if (!entry)
			return -ENOMEM;
		ret = edgx_dmac_insert(dmac, entry);
		if (ret) {
			WARN_ON(ret);
			goto out_insert;
		}
	}
	entry->change_detector = dmac->detector_state;
	return 0;

out_insert:
	kfree(entry);
	return ret;
}

struct dmac_notify {
	struct edgx_br_vlan *brvlan;    /* For asking VLANs in FID */
	fid_t fid;                      /* Current FID */
	fid_iter fid_iter;              /* Cached FID iterator */
	vlan_iter vlan_iter;            /* Iterator to get each VLAN in FID */
	struct switchdev_notifier_fdb_info fdb_info;
};

static void edgx_dmac_init_notify(struct dmac_notify *notify,
				  struct edgx_dmac *dmac)
{
	memset(notify, 0, sizeof(*notify));
	notify->brvlan = edgx_br_get_vlan(dmac->parent);
	notify->fid = DMAC_INV_FID;
}

static void edgx_dmac_notify_fid(struct edgx_dmac *dmac,
				 struct dmac_notify *notify,
				 unsigned long val,
				 ptid_t ptid)
{
	struct edgx_pt *pt = edgx_br_get_brpt(dmac->parent, ptid);
	/* Keep original VLAN iterator pointing to the first VLAN of FID */
	vlan_iter vlan_iter = notify->vlan_iter;
	vid_t vid;

	if (!pt)
		return;

	/* Send separate notification for each VLAN in current FID */
	while (vlan_iter) {
		vid = edgx_br_vlan_get_fid_vlan(notify->fid_iter, &vlan_iter);
		notify->fdb_info.vid = vid;
		call_switchdev_notifiers(val, edgx_pt_get_netdev(pt),
					 &notify->fdb_info.info, NULL);
	}
}

static bool edgx_dmac_update_fid(struct dmac_notify *notify, fid_t fid)
{
	/* Entries are in FID order, reuse FID iterator when possible */
	if (notify->fid != fid) {
		notify->fid = fid;
		notify->fid_iter = edgx_br_vlan_iter_fid_vlans(
					notify->brvlan,
					notify->fid,
					&notify->vlan_iter);
	}
	return notify->fid_iter && notify->vlan_iter;
}

static void edgx_dmac_call_notifiers(struct edgx_dmac *dmac)
{
	struct dmac_notify notify;
	struct rb_node *node = rb_first(&dmac->root);
	struct dmac_entry *entry = NULL;
	bool aged;

	edgx_dmac_init_notify(&notify, dmac);
	rtnl_lock();

	while (node) {
		entry = rb_entry(node, struct dmac_entry, rbnode);
		node = rb_next(node);

		if (!edgx_dmac_update_fid(&notify, entry->fid))
			continue;

		notify.fdb_info.addr = &entry->mac[0];
		aged = entry->change_detector != dmac->detector_state;
		if (aged || entry->ptid_new != entry->ptid) {
			edgx_dmac_notify_fid(dmac, &notify,
					     SWITCHDEV_FDB_DEL_TO_BRIDGE,
					     entry->ptid);
		}
		if (aged) {
			edgx_dmac_erase(&dmac->root, entry);
		} else {
			entry->ptid = entry->ptid_new;
			edgx_dmac_notify_fid(dmac, &notify,
					     SWITCHDEV_FDB_ADD_TO_BRIDGE,
					     entry->ptid);
		}
	}

	rtnl_unlock();
}

static void edgx_dmac_sync_work(struct work_struct *work)
{
	struct edgx_dmac *dmac = container_of(work, struct edgx_dmac,
					      work_sync.work);
	int ret;

	/* Update new and still existing */

	mutex_lock(&dmac->lock);

	dmac->detector_state++;
	do {
		ret = edgx_dmac_read_next(dmac);
	} while (ret != -ENOENT);

	mutex_unlock(&dmac->lock);

	/* Prevent VID to FID mapping changes during notifications */
	edgx_br_vlan_lock(edgx_br_get_vlan(dmac->parent));
	mutex_lock(&dmac->lock);

	/* Send notifications and remove aged entries */
	edgx_dmac_call_notifiers(dmac);

	edgx_br_vlan_unlock(edgx_br_get_vlan(dmac->parent));
	mutex_unlock(&dmac->lock);

	cancel_delayed_work(&dmac->work_sync);
	schedule_delayed_work(&dmac->work_sync,
			      msecs_to_jiffies(DMAC_SYNC_DELAY_MS));
}

void edgx_brfdb_update_vid2fid(struct edgx_brfdb *brfdb, vid_t vid,
			       fid_t fid_new, fid_t fid_old)
{
	struct edgx_dmac *dmac = &brfdb->dmac;
	struct switchdev_notifier_fdb_info fdb_info = { .vid = vid };
	struct rb_node *node = rb_first(&dmac->root);
	struct dmac_entry *entry = NULL;
	struct edgx_pt *pt = NULL;

	/* When VLAN FID changes, send forgotten notifications with that VID
	 * immediately for entries in old FID to avoid dangling entries
	 */
	mutex_lock(&dmac->lock);
	rtnl_lock();

	while (node) {
		entry = rb_entry(node, struct dmac_entry, rbnode);
		node = rb_next(node);

		if (entry->fid != fid_old)
			continue;

		pt = edgx_br_get_brpt(dmac->parent, entry->ptid);
		if (!pt)
			continue;

		fdb_info.addr = &entry->mac[0];
		call_switchdev_notifiers(SWITCHDEV_FDB_DEL_TO_BRIDGE,
					 edgx_pt_get_netdev(pt),
					 &fdb_info.info, NULL);
	}

	rtnl_unlock();
	mutex_unlock(&dmac->lock);
}

static inline struct edgx_dmac *edgx_dev2dmac(struct device *dev)
{
	struct edgx_br *br = edgx_dev2br(dev);
	struct edgx_brfdb *brfdb = edgx_br_get_fdb(br);

	return brfdb ? &brfdb->dmac : NULL;
}

static void edgx_dmac_sync_enable(struct edgx_dmac *dmac)
{
	cancel_delayed_work(&dmac->work_sync);
	/* Trigger synchronization immediately */
	schedule_delayed_work(&dmac->work_sync, 0);
}

enum dmac_sync_disable_mode {
	DMAC_SEND_NOTIFICATIONS,
	DMAC_SKIP_NOTIFICATIONS,
};

static void edgx_dmac_sync_disable(struct edgx_dmac *dmac,
				   enum dmac_sync_disable_mode mode)
{
	struct dmac_notify notify;
	struct rb_node *node = rb_first(&dmac->root);
	struct dmac_entry *entry = NULL;

	edgx_dmac_init_notify(&notify, dmac);

	cancel_delayed_work_sync(&dmac->work_sync);

	edgx_br_vlan_lock(edgx_br_get_vlan(dmac->parent));
	mutex_lock(&dmac->lock);

	while (node) {
		entry = rb_entry(node, struct dmac_entry, rbnode);
		node = rb_next(node);

		if (mode == DMAC_SEND_NOTIFICATIONS &&
		    edgx_dmac_update_fid(&notify, entry->fid)) {
			notify.fdb_info.addr = &entry->mac[0];
			edgx_dmac_notify_fid(dmac, &notify,
					     SWITCHDEV_FDB_DEL_TO_BRIDGE,
					     entry->ptid);
		}

		edgx_dmac_erase(&dmac->root, entry);
	}

	mutex_unlock(&dmac->lock);
	edgx_br_vlan_unlock(edgx_br_get_vlan(dmac->parent));
}

static ssize_t dmac_sync_enable_store(struct device *dev,
				      struct device_attribute *attr,
				      const char *buf, size_t count)
{
	struct edgx_dmac *dmac = edgx_dev2dmac(dev);
	bool en;

	if (kstrtobool(buf, &en))
		return -EINVAL;

	if (en)
		edgx_dmac_sync_enable(dmac);
	else
		edgx_dmac_sync_disable(dmac, DMAC_SEND_NOTIFICATIONS);

	return count;
}

EDGX_DEV_ATTR_WO(dmac_sync_enable, "sync_enable");

static struct attribute *edgx_dmac_attrs[] = {
	&dev_attr_dmac_sync_enable.attr,
	NULL,
};

static struct attribute_group dmac_attr_group = {
	.name = "edgex-dmac",
	.attrs = edgx_dmac_attrs,
};

static int edgx_dmac_init(struct edgx_br *br, edgx_io_t *iobase,
			   struct edgx_dmac *dmac)
{
	dmac->parent = br;
	dmac->iobase = iobase;
	mutex_init(&dmac->lock);
	dmac->root = RB_ROOT;
	INIT_DELAYED_WORK(&dmac->work_sync, edgx_dmac_sync_work);

	return edgx_br_sysfs_add(br, &dmac_attr_group);
}

int edgx_brfdb_init(struct edgx_br *br, edgx_io_t *iobase,
		    struct edgx_brfdb **brfdb)
{
	struct edgx_brfdb *fdb;

	if (!br || !brfdb)
		return -EINVAL;
	fdb = kzalloc(sizeof(*fdb), GFP_KERNEL);
	if (!fdb)
		return -ENOMEM;

	fdb->parent = br;
	fdb->iobase = iobase;
	fdb->nrows  = edgx_br_get_generic_pow2z(br, BR_GX_SMAC_ROWS);
	fdb->sents  = 0;
	if (!fdb->nrows)
		goto out_rows;
	mutex_init(&fdb->lock);
	*brfdb = fdb;

	/* Initialize per-column VLAN-XOR setting */
	edgx_set16(iobase, 0x222, 1, 0, column[0]);
	edgx_set16(iobase, 0x222, 3, 2, column[1]);
	edgx_set16(iobase, 0x222, 5, 4, column[2]);
	edgx_set16(iobase, 0x222, 7, 6, column[3]);

	edgx_br_info(br, "Instantiating FDB (%u rows) ... done\n", fdb->nrows);
	return 0;

out_rows:
	kfree(fdb);
	return -ENODEV;
}

void edgx_brfdb_shutdown(struct edgx_brfdb *brfdb)
{
	if (brfdb) {
		kfree(brfdb);
	}
}

int edgx_brfdb_init_dmac_sync(struct edgx_brfdb *brfdb)
{
	int ret = edgx_dmac_init(brfdb->parent, brfdb->iobase, &brfdb->dmac);

	if (ret)
		return ret;

#ifdef DMAC_SYNC_AUTO_ENABLE
	schedule_delayed_work(&brfdb->dmac.work_sync,
			      msecs_to_jiffies(DMAC_SYNC_DELAY_MS));
#endif
	return 0;
}

void edgx_brfdb_shutdown_dmac_sync(struct edgx_brfdb *brfdb)
{
	edgx_dmac_sync_disable(&brfdb->dmac, DMAC_SKIP_NOTIFICATIONS);
}

void edgx_brfdb_flush(struct edgx_brfdb *brfdb, u8 fid)
{
	(void)brfdb;
	(void)fid;
}
