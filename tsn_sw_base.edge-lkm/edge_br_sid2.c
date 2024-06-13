// SPDX-License-Identifier: GPL-2.0
/*
 * TTTech EDGE/DE-IP Linux driver
 * Copyright(c) 2021 TTTech Industrial Automation AG.
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

#include <linux/rbtree.h>
#include <linux/etherdevice.h>

#include "edge_defines.h"
#include "edge_bridge.h"
#include "edge_util.h"
#include "edge_port.h"
#include "edge_br_fdb.h"
#include "edge_ac.h"
#include "edge_br_sid.h"

#define sid_dbg edgx_dbg

#define SID_CNT_CAPT0		(0x0)
#define SID_CNT_CAPT2		(0x4)
#define SID_CNT_FRAMES_IN_L	(0x0300)
#define SID_CNT_FRAMES_IN_H	(0x0302)
#define SID_CNT_FRAMES_OUT_L	(0x0304)
#define SID_CNT_FRAMES_OUT_H	(0x0306)
#define SID_MAX_NO_STREAMS	(1024U)
#define SID_INVALID_STRHDL	((u16)(-1))
#define SID_MAX_ORD		((u16)(-1))
#define SID_MAX_ORD_CNT		(SID_MAX_ORD + 1)
#define SID_TAB_INVALID		((u16)(-1))
#define SID_MAX_NPORTS		(12U)
#define VID_WILDCARD		(0U)
#define SID_CTL_POS(row, col)	((((col) & 0x3) << 12) | ((row) & 0xFFF))
#define SID_CTL_RD(row, col)	(BIT(15) | SID_CTL_POS(row, col))
#define SID_CTL_WR(row, col)	(BIT(15) | BIT(14) | SID_CTL_POS(row, col))
#define SID_CTL_BUSY(ctrl)	((ctrl) & BIT(15))

enum sid_tag {
	SID_TAGGED = 1,
	SID_PRIORITY = 2,
	SID_ALL = 3,
};

#define sid_tag_str(tag) ((tag == SID_TAGGED) ? "tagged" : \
			  ((tag == SID_PRIORITY) ? " priority" : "all"))

enum sid_pos {
	IN_FAC_OUT = 0,
	IN_FAC_IN,
	OUT_FAC_OUT,
	OUT_FAC_IN,
};

struct sid_tab_entry {
	bool used;
	bool match_vlan;
	u8  mac[ETH_ALEN];
	u16 vid;
	u16 str_hdl[SID_MAX_NPORTS];
};

/* CRC40 lookup table for polynomial 0x0104c11db7. */
static const u64 crc40tbl[] = {
	0x000000000ull, 0x104c11db7ull, 0x209823b6eull, 0x30d4326d9ull,
	0x4130476dcull, 0x517c56b6bull, 0x61a864db2ull, 0x71e475005ull,
	0x82608edb8ull, 0x922c9f00full, 0xa2f8ad6d6ull, 0xb2b4bcb61ull,
	0xc350c9b64ull, 0xd31cd86d3ull, 0xe3c8ea00aull, 0xf384fbdbdull,
};

struct port_pos {
	u16 port;
	enum sid_pos pos;
} __packed;

struct ident_params {
	u8  addr[ETH_ALEN];
	enum sid_tag tag;
	u16 vid;
	enum sid_ident_type id_type;
	u16 str_hdl;
}  __packed;

struct port_list {
	u16 in_fac_out;
	u16 out_fac_out;
	u16 in_fac_in;
	u16 out_fac_in;
} __packed;

struct sid_rb_entry {
	struct rb_node node;
	u64 key;
};

/* tmvt_o RBT entry. RBT key: ordinary number */
struct sid_tmvt_o_entry {
	struct sid_rb_entry rb_entry;
	u16 str_hdl;
	u32 port_mask;
};

#define rbe_to_tmvt_o_entry(rbe) container_of(rbe, struct sid_tmvt_o_entry, \
					      rb_entry)

struct sid_ht_entry;

/* tmvt RBT entry. RBT key: tag & vid & mac */
struct sid_tmvt_entry {
	struct sid_rb_entry rb_entry;
	u8  addr[ETH_ALEN];
	enum sid_tag tag;
	u16 vid;
	struct rb_root rb_tmvt_o_root;
	struct sid_ht_entry *ht_entry[2];
};

#define rbe_to_tmvt_entry(rbe) container_of(rbe, struct sid_tmvt_entry, \
					    rb_entry)

/* o RBT entry. RBT key: ordinary number */
struct sid_o_entry {
	struct sid_rb_entry rb_entry;
	u16 ord;
	struct sid_tmvt_entry * tmvt_entry;
	struct sid_tmvt_o_entry * tmvt_o_entry;
};

#define rbe_to_o_entry(rbe) container_of(rbe, struct sid_o_entry, rb_entry)

/* Hash-table SW "map" RBT entry. RBT key: hash-table row & column */
struct sid_ht_entry {
	struct sid_rb_entry rb_entry;
	struct sid_tmvt_entry * tmvt_entry;
	bool used;
};

#define rbe_to_ht_entry(rbe) container_of(rbe, struct sid_ht_entry, rb_entry)

struct edgx_sid_br {
	struct edgx_br *parent;
	edgx_io_t *tab_iobase;
	edgx_io_t *counter_iobase;
	struct mutex lock; /*Protects edgx_sid data*/
	u16 max_streams;
	u32 total_cnt;
	struct rb_root rb_o_root;
	struct rb_root rb_tmvt_root;
	struct rb_root rb_ht_root;
	u16 tab_row_cnt;
	int port_cnt;
	u16 sup_port_mask;
};

static inline bool edgx_sid_get_match_vlan(enum sid_tag tag, u16 vid)
{
	return ((tag == SID_TAGGED) && (vid != 0)) ? true : false;
}

static void edgx_sid_calc_tab_rows(struct edgx_sid_br *sid_br,
				   const unsigned char *mac,
				   enum sid_tag tag, u16 vid,
				   bool use_src_match, u16 *row)
{
	unsigned int i;
	u64 hash;
	u64 extra;
	u64 crc = ~((u64)0);
	u16 tagged = (tag == SID_TAGGED) ? BIT(5) : 0U;
	u16 srcdst = (use_src_match) ? BIT(6) : 0U;

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

	/* XOR the first two columns hash with VLAN ID. */
	hash ^= ((u64)vid << 16) |
		((u64)vid << 0);

	hash ^= ((u64)tagged << 48) |
		((u64)tagged << 32) |
		((u64)tagged << 16) |
		((u64)tagged << 0);

	hash ^= ((u64)srcdst << 48) |
		((u64)srcdst << 32) |
		((u64)srcdst << 16) |
		((u64)srcdst << 0);

	/* Extract and truncate rows. First two columns are reserved
	 * for HW VID match.
	 * Untagged and priority tagged frames can go only to columns 2 and 3.
	 * Tagged frames can go to all 4 columns if no wildcard
	 * is used (vid != 0).
	 * Tagged frames can go only to rows 2 and 3 if wildcard
	 * is used (vid == 0).*/
	row[0] = edgx_sid_get_match_vlan(tag, vid) ?
		 (hash & (sid_br->tab_row_cnt - 1)) : SID_TAB_INVALID;
	row[1] = edgx_sid_get_match_vlan(tag, vid) ?
		 ((hash >> 16) & (sid_br->tab_row_cnt - 1)) : SID_TAB_INVALID;
	row[2] = (hash >> 32) & (sid_br->tab_row_cnt - 1);
	row[3] = (hash >> 48) & (sid_br->tab_row_cnt - 1);

	sid_dbg("rows: %u(%u), %u(%u), %u, %u\n",
		row[0], (u16)(hash & (sid_br->tab_row_cnt - 1)),
		row[1], (u16)((hash >> 16) & (sid_br->tab_row_cnt - 1)),
		row[2], row[3]);
}

static void edgx_sid_tab_get(struct edgx_sid_br *sid_br, u16 col, u16 row,
			     struct sid_tab_entry *e)
{
	u16 ctrl = SID_CTL_RD(row, col);
	u16 general;
	int i;

	edgx_wr16(sid_br->tab_iobase, 0x280, ctrl);
	do {
		ctrl = edgx_rd16(sid_br->tab_iobase, 0x280);
	} while (SID_CTL_BUSY(ctrl));

	general = edgx_rd16(sid_br->tab_iobase, 0x290);
	e->used = (general & BIT(0)) ? true : false;
	e->match_vlan = (general & BIT(1)) ? true : false;
	*((u16 *)&e->mac[0]) = edgx_rd16(sid_br->tab_iobase, 0x292);
	*((u16 *)&e->mac[2]) = edgx_rd16(sid_br->tab_iobase, 0x294);
	*((u16 *)&e->mac[4]) = edgx_rd16(sid_br->tab_iobase, 0x296);
	e->vid = edgx_rd16(sid_br->tab_iobase, 0x298);
	for (i = 0; i < sid_br->port_cnt; i++)
		e->str_hdl[i] = edgx_rd16(sid_br->tab_iobase, 0x29a + (i * 2));
}

static inline bool edgx_sid_tab_match(struct sid_tab_entry *e,
				      const unsigned char *mac,
				      enum sid_tag tag, u16 vid)
{
	return (e->match_vlan == edgx_sid_get_match_vlan(tag, vid)) &&
		(vid == e->vid) && !ether_addr_cmp(mac, e->mac);
}

static void edgx_sid_tab_set(struct edgx_sid_br *sid_br, u16 col, u16 row,
			     struct sid_tab_entry *e)
{
	u16 ctrl = SID_CTL_WR(row, col);
	u16 general = 0U;
	int i;

	if (e->used)
		general = BIT(0);
	if (e->match_vlan)
		general |= BIT(1);
	edgx_wr16(sid_br->tab_iobase, 0x290, general);
	edgx_wr16(sid_br->tab_iobase, 0x292, *((u16 *)&e->mac[0]));
	edgx_wr16(sid_br->tab_iobase, 0x294, *((u16 *)&e->mac[2]));
	edgx_wr16(sid_br->tab_iobase, 0x296, *((u16 *)&e->mac[4]));
	edgx_wr16(sid_br->tab_iobase, 0x298, e->vid);
	for (i = 0; i < sid_br->port_cnt; i++)
		edgx_wr16(sid_br->tab_iobase, 0x29a + (i * 2), e->str_hdl[i]);

	edgx_wr16(sid_br->tab_iobase, 0x280, ctrl);
	do {
		ctrl = edgx_rd16(sid_br->tab_iobase, 0x280);
	} while (SID_CTL_BUSY(ctrl));
}

static inline u64 params_to_tmvt_key(struct ident_params *params)
{
	return ((u64)params->addr[0] << 56) | ((u64)params->addr[1] << 48) |
	       ((u64)params->addr[2] << 40) | ((u64)params->addr[3] << 32) |
	       ((u64)params->addr[4] << 24) | ((u64)params->addr[5] << 16) |
	       ((u64)(params->tag & 0x3) << 14) |
	       ((u64)(params->vid & 0x3fff));
}

static struct sid_rb_entry *sid_rb_search(struct rb_root *root, u64 key)
{
	struct rb_node *node = root->rb_node;
	struct sid_rb_entry *entry;

	while (node) {
		entry = rb_entry(node, struct sid_rb_entry, node);
		if (key < entry->key)
			node = node->rb_left;
		else if (key > entry->key)
			node = node->rb_right;
		else
			return entry;
	}
	return NULL;
}

static int sid_rb_insert(struct rb_root *root, struct sid_rb_entry *entry)
{
	struct rb_node **new = &(root->rb_node);
	struct rb_node *parent = NULL;
	struct sid_rb_entry *this;

	while (*new) {
		parent = *new;
		this = rb_entry(*new, struct sid_rb_entry, node);
		if (entry->key < this->key)
			new = &((*new)->rb_left);
		else if (entry->key > this->key)
			new = &((*new)->rb_right);
		else
			return -EEXIST;
	}

	rb_link_node(&entry->node, parent, new);
	rb_insert_color(&entry->node, root);

	return 0;
}

static void sid_del_entry(struct rb_root *root, struct sid_rb_entry *entry)
{
	rb_erase(&entry->node, root);
	RB_CLEAR_NODE(&entry->node);
}

static struct sid_rb_entry* sid_rb_first(struct rb_root *root)
{
	struct rb_node *first;
	struct sid_rb_entry *entry = NULL;

	first = rb_first(root);
	if (first)
		entry = rb_entry(first, struct sid_rb_entry, node);

	return entry;
}

static struct sid_rb_entry* sid_rb_next(struct sid_rb_entry* cur)
{
	struct rb_node *next;
	struct sid_rb_entry *entry = NULL;

	next = rb_next(&cur->node);
	if (next)
		entry = rb_entry(next, struct sid_rb_entry, node);

	return entry;
}

static struct sid_rb_entry* sid_rb_last(struct rb_root *root)
{
	struct rb_node *last;
	struct sid_rb_entry *entry = NULL;

	last = rb_last(root);
	if (last)
		entry = rb_entry(last, struct sid_rb_entry, node);

	return entry;
}

static struct sid_rb_entry* sid_rb_prev(struct sid_rb_entry* cur)
{
	struct rb_node *prev;
	struct sid_rb_entry *entry = NULL;

	prev = rb_prev(&cur->node);
	if (prev)
		entry = rb_entry(prev, struct sid_rb_entry, node);

	return entry;
}

static void sid_dump(struct edgx_sid_br *sid)
{
	struct sid_rb_entry *rbe;
	struct sid_o_entry *oe;
	struct sid_rb_entry *rbe_to;
	struct sid_tmvt_o_entry *toe;
	struct sid_tab_entry e;
	int col, row;

	sid_dbg("SID DUMP:\n");
	for (rbe = sid_rb_first(&sid->rb_o_root); rbe; rbe = sid_rb_next(rbe))
	{
		oe = rbe_to_o_entry(rbe);
		sid_dbg("ORD: %u, StrHdl: %u, port_mask: 0x%x\n",
			oe->ord, oe->tmvt_o_entry->str_hdl,
			oe->tmvt_o_entry->port_mask);
		sid_dbg("\t MAC: %pM, Tag: %s, VId: %u\n",
			oe->tmvt_entry->addr, sid_tag_str(oe->tmvt_entry->tag),
			oe->tmvt_entry->vid);

		sid_dbg("\t\tLinked ORDs:\n");
		for (rbe_to = sid_rb_first(&oe->tmvt_entry->rb_tmvt_o_root);
		     rbe_to; rbe_to = sid_rb_next(rbe_to))
		{
			toe = rbe_to_tmvt_o_entry(rbe_to);
			sid_dbg("\t\t%llu\n", toe->rb_entry.key);
		}
	}

	sid_dbg("\nSID HW TABLE:\n");
	for (row = 0; row < sid->tab_row_cnt; row++) {
		for (col = 0; col < 4; col++) {
			memset(&e, 0, sizeof(e));
			edgx_sid_tab_get(sid, col, row, &e);
			if (e.used) {
				sid_dbg("%d:%d\tmatch_vlan: %d Vid: %u  MAC: %pM\n",
					row, col, e.match_vlan, e.vid, e.mac);
				sid_dbg("\t\t\tSTR_HDLs: %u %u %u %u %u %u %u %u %u %u %u %u\n",
					e.str_hdl[0], e.str_hdl[1],
					e.str_hdl[2], e.str_hdl[3],
					e.str_hdl[4], e.str_hdl[5],
					e.str_hdl[6], e.str_hdl[7],
					e.str_hdl[8], e.str_hdl[8],
					e.str_hdl[10], e.str_hdl[11]);
			}
		}
	}
}

inline static u64 sid_ht_get_key(u16 row, u8 col)
{
	return ((u64)row << 8) | col;
}

inline static u16 sid_ht_get_row(struct sid_ht_entry *hte)
{
	return (u16)(hte->rb_entry.key >> 8);
}

inline static u8 sid_ht_get_col(struct sid_ht_entry *hte)
{
	return (u8)(hte->rb_entry.key & 0xff);
}

static int sid_ht_create(struct edgx_sid_br *sid_br,
			 struct sid_tmvt_entry *tmvte,
			 enum sid_tag sub_tag,
			 struct sid_ht_entry **new_hte)
{
	u16 rows[4];
	u8 i;
	struct sid_tmvt_entry *ce[4] = {NULL};
	struct sid_ht_entry *hte;
	struct sid_rb_entry *rbe;
	u64 ht_key;
	int ret;

	edgx_sid_calc_tab_rows(sid_br, tmvte->addr, sub_tag, tmvte->vid,
			       false, rows);

	for (i = 0; i < 4; i++) {
		if (rows[i] == SID_TAB_INVALID)
			continue;
		ht_key = sid_ht_get_key(rows[i], i);
		rbe = sid_rb_search(&sid_br->rb_ht_root, ht_key);
		if(rbe) {
			hte = rbe_to_ht_entry(rbe);
			ce[i] = hte->tmvt_entry;
		} else {
			break;
		}
	}

	if (i == 4) {
		edgx_br_err(sid_br->parent,"Unable to add SID entry due to a hash conflict!\n");
		for (i = 0; i < 4; i++)
			if (ce[i])
				edgx_br_err(sid_br->parent,
					    "Hash conflict with entry: MAC: %pM Tag: %s, Vid: %u\n",
					    ce[i]->addr,
					    sid_tag_str(ce[i]->tag),
					    ce[i]->vid);
		return -ENOSPC;
	}

	hte = kzalloc(sizeof(*hte), GFP_KERNEL);
	if (!hte)
		return -ENOMEM;

	hte->rb_entry.key = ht_key;
	hte->tmvt_entry = tmvte;
	hte->used = false;
	ret = sid_rb_insert(&sid_br->rb_ht_root, &hte->rb_entry);
	if (ret) {
		kfree(hte);
		return ret;
	}

	edgx_dbg("Inserted to hash table map at row: %d, col: %d\n",
		 sid_ht_get_row(hte), sid_ht_get_col(hte));

	*new_hte = hte;
	return 0;
}

static void sid_ht_deactivate(struct edgx_sid_br *sid_br,
			      struct sid_ht_entry *hte)
{
	struct sid_tab_entry e = { .used = false, .str_hdl = {0} };

	if (!hte->used)
		return;

	edgx_sid_tab_set(sid_br, sid_ht_get_col(hte), sid_ht_get_row(hte), &e);
	hte->used = false;
	edgx_dbg("deactivated row: %d, col: %d\n",
		 sid_ht_get_row(hte), sid_ht_get_col(hte));
}

static void sid_ht_activate(struct edgx_sid_br *sid_br,
			    struct sid_ht_entry *hte,
			    enum sid_tag tag, struct sid_tab_entry *e)
{
	e->used = true;
	e->vid = hte->tmvt_entry->vid;
	ether_addr_copy(e->mac, hte->tmvt_entry->addr);
	e->match_vlan = edgx_sid_get_match_vlan(tag, e->vid);

	edgx_sid_tab_set(sid_br, sid_ht_get_col(hte), sid_ht_get_row(hte), e);
	hte->used = true;
	edgx_dbg("activated row: %d, col: %d\n",
		 sid_ht_get_row(hte), sid_ht_get_col(hte));
}

static void sid_ht_release(struct edgx_sid_br *sid_br,
			   struct sid_ht_entry *hte)
{
	if (hte->used)
		sid_ht_deactivate(sid_br, hte);
	sid_del_entry(&sid_br->rb_ht_root, &hte->rb_entry);
	kfree(hte);
}

static int sid_tmvt_create(struct edgx_sid_br *sid_br,
			   u64 tmvt_key,
			   enum sid_tag tag,
			   u16 vid,
			   const u8 *addr,
			   struct sid_tmvt_entry **new_tmvte)
{
	int ret;
	struct sid_tmvt_entry *tmvte;

	tmvte = kzalloc(sizeof(*tmvte), GFP_KERNEL);
	if (!tmvte) {
		return -ENOMEM;
	}

	tmvte->rb_entry.key = tmvt_key;
	tmvte->rb_tmvt_o_root = RB_ROOT;
	tmvte->tag = tag;
	tmvte->vid= vid;
	ether_addr_copy(tmvte->addr, addr);

	/* To support Tag = "all", 2 ht entries are required. */
	if (tmvte->tag == SID_ALL) {
		ret = sid_ht_create(sid_br, tmvte, SID_TAGGED,
				    &tmvte->ht_entry[0]);
		if (ret)
			goto out_ctmvt_free_tmvte;

		ret = sid_ht_create(sid_br, tmvte, SID_PRIORITY,
				    &tmvte->ht_entry[1]);
		if (ret)
			goto out_ctmvt_rel_ht;
	} else {
		ret = sid_ht_create(sid_br, tmvte, tmvte->tag,
				    &tmvte->ht_entry[0]);
		if (ret)
			goto out_ctmvt_free_tmvte;
	}
	ret = sid_rb_insert(&sid_br->rb_tmvt_root, &tmvte->rb_entry);
	if (ret)
		goto out_ctmvt_rel_ht;

	*new_tmvte = tmvte;
	return 0;

out_ctmvt_rel_ht:
	if (tmvte->ht_entry[1])
		sid_ht_release(sid_br, tmvte->ht_entry[1]);
	sid_ht_release(sid_br, tmvte->ht_entry[0]);
out_ctmvt_free_tmvte:
	kfree(tmvte);
	return ret;
}

static void sid_tmvt_release(struct edgx_sid_br *sid_br,
			     struct sid_tmvt_entry *tmvte)
{
	if (tmvte->ht_entry[1])
		sid_ht_release(sid_br, tmvte->ht_entry[1]);
	if (tmvte->ht_entry[0])
		sid_ht_release(sid_br, tmvte->ht_entry[0]);

	sid_del_entry(&sid_br->rb_tmvt_root, &tmvte->rb_entry);
	kfree(tmvte);
}

static inline bool sid_tmvt_is_empty(struct sid_tmvt_entry *tmvte)
{
	return !sid_rb_first(&tmvte->rb_tmvt_o_root);
}

static void sid_gen_port_str_hdls(struct edgx_sid_br *sid_br,
				  struct sid_tmvt_entry *tmvte)
{
	struct sid_tab_entry e = { .str_hdl = {0} };
	struct sid_rb_entry *rbe;
	struct sid_tmvt_o_entry *toe;
	struct sid_ht_entry **hte = tmvte->ht_entry;
	bool str_hdl_found = false;
	int i;

	for (rbe = sid_rb_last(&tmvte->rb_tmvt_o_root);
	     rbe; rbe = sid_rb_prev(rbe))
	{
		toe = rbe_to_tmvt_o_entry(rbe);
		for (i = 0; toe->port_mask &&
			    (toe->str_hdl != SID_INVALID_STRHDL) &&
			    (i < sid_br->port_cnt); i++) {
			if (toe->port_mask & BIT(i + 1)) {
				e.str_hdl[i] = toe->str_hdl;
				str_hdl_found = true;
			}
		}
	}

	if (str_hdl_found) {
		if (tmvte->tag == SID_ALL) {
			sid_ht_activate(sid_br, hte[0], SID_TAGGED, &e);
			sid_ht_activate(sid_br, hte[1], SID_PRIORITY, &e);
		} else {
			sid_ht_activate(sid_br, hte[0], tmvte->tag, &e);
		}
	} else {
		sid_ht_deactivate(sid_br, hte[0]);
		if (tmvte->tag == SID_ALL)
			sid_ht_deactivate(sid_br, hte[1]);
	}
}

static int sid_o_create(struct edgx_sid_br *sid_br, u16 ord,
			struct sid_tmvt_entry *tmvte)
{
	struct sid_o_entry *oe;
	struct sid_tmvt_o_entry *tmvtoe;
	int ret = -ENOMEM;

	oe = kzalloc(sizeof(*oe), GFP_KERNEL);
	if (!oe)
		goto out_o_create_ret;

	tmvtoe = kzalloc(sizeof(*tmvtoe), GFP_KERNEL);
	if (!tmvtoe) {
		ret = -ENOMEM;
		goto out_o_create_free_oe;
	}

	oe->rb_entry.key = ord;
	oe->ord = ord;
	oe->tmvt_entry = tmvte;
	oe->tmvt_o_entry = tmvtoe;

	tmvtoe->rb_entry.key = ord;
	tmvtoe->str_hdl = SID_INVALID_STRHDL;

	ret = sid_rb_insert(&sid_br->rb_o_root, &oe->rb_entry);
	if (ret)
		goto out_o_create_rel_tmvtoe;

	ret = sid_rb_insert(&tmvte->rb_tmvt_o_root, &tmvtoe->rb_entry);
	if (ret)
		goto out_o_create_del_oe;

	sid_br->total_cnt++;
	return 0;

out_o_create_del_oe:
	sid_del_entry(&sid_br->rb_o_root, &oe->rb_entry);
out_o_create_rel_tmvtoe:
	kfree(tmvtoe);
out_o_create_free_oe:
	kfree(oe);
out_o_create_ret:
	return ret;
}

static void sid_o_release(struct edgx_sid_br *sid_br, struct sid_o_entry *oe)
{
	sid_del_entry(&oe->tmvt_entry->rb_tmvt_o_root,
		      &oe->tmvt_o_entry->rb_entry);
	kfree(oe->tmvt_o_entry);

	sid_del_entry(&sid_br->rb_o_root, &oe->rb_entry);
	kfree(oe);
	sid_br->total_cnt--;
}

static ssize_t sid_cnt_show(struct device *dev,
			    struct device_attribute *attr,
			    char *buf)
{
	struct edgx_sid_br *sid_br = edgx_br_get_sid(edgx_dev2br(dev));
	int ret = 0;

	sid_dbg("\n");
	mutex_lock(&sid_br->lock);
	ret = scnprintf(buf, PAGE_SIZE, "%u\n", sid_br->total_cnt);
	mutex_unlock(&sid_br->lock);

	return ret;
}

static ssize_t sid_tab_len_show(struct device *dev,
				struct device_attribute *attr,
				char *buf)
{
	struct edgx_sid_br *sid_br = edgx_br_get_sid(edgx_dev2br(dev));
	int ret = 0;

	sid_dbg("\n");
	mutex_lock(&sid_br->lock);
	ret = scnprintf(buf, PAGE_SIZE, "%u\n", sid_br->tab_row_cnt * 4U);
	mutex_unlock(&sid_br->lock);

	return ret;
}

static ssize_t sid_max_sup_show(struct device *dev,
				struct device_attribute *attr,
				char *buf)
{
	struct edgx_sid_br *sid_br = edgx_br_get_sid(edgx_dev2br(dev));
	int ret = 0;

	sid_dbg("\n");
	mutex_lock(&sid_br->lock);
	/* Max. supported stream handle value */
	ret =  scnprintf(buf, PAGE_SIZE, "%u\n",
			 sid_br->max_streams ? sid_br->max_streams - 1 : 0);
	mutex_unlock(&sid_br->lock);

	return ret;
}

static ssize_t sid_sup_ports_show(struct device *dev,
				  struct device_attribute *attr,
				  char *buf)
{
	struct edgx_sid_br *sid_br = edgx_br_get_sid(edgx_dev2br(dev));
	int ret = 0;

	sid_dbg("\n");
	mutex_lock(&sid_br->lock);
	ret =  scnprintf(buf, PAGE_SIZE, "%u\n", sid_br->sup_port_mask);
	mutex_unlock(&sid_br->lock);

	return ret;
}

static ssize_t sid_list_entries_read(struct file *filp, struct kobject *kobj,
				     struct bin_attribute *bin_attr,
				     char *buf, loff_t ofs, size_t count)
{
	struct edgx_br *br = edgx_dev2br(kobj_to_dev(kobj));
	struct edgx_sid_br *sid_br = edgx_br_get_sid(br);
	struct sid_rb_entry *rbe;
	struct sid_o_entry *oe;
	loff_t idx;
	size_t nelems;
	u16 *ord_list = (u16*)buf;
	int i, ret;

	sid_dbg("\n");
	mutex_lock(&sid_br->lock);

	if (!sid_br->total_cnt) {
		mutex_unlock(&sid_br->lock);
		return -ENOENT;
	}
	ret = edgx_sysfs_list_params(ofs, count, sizeof(u16), &idx, &nelems);
	if (ret || ((idx + nelems) > sid_br->total_cnt)) {
		mutex_unlock(&sid_br->lock);
		return -EINVAL;
	}
	for (i = 0, rbe = sid_rb_first(&sid_br->rb_o_root);
	     rbe && (i < idx); rbe = sid_rb_next(rbe), i++);

	for (i = 0; rbe && (i < nelems); rbe = sid_rb_next(rbe), i++) {
		oe = rbe_to_o_entry(rbe);
		ord_list[i] = oe->ord;
	}
	ret = i * sizeof(u16);

	mutex_unlock(&sid_br->lock);
	return ret;
}

static ssize_t sid_sup_ident_types_read(struct file *filp, struct kobject *kobj,
					struct bin_attribute *bin_attr,
					char *buf, loff_t ofs, size_t count)
{
	struct edgx_br *br = edgx_dev2br(kobj_to_dev(kobj));
	struct edgx_sid_br *sid_br = edgx_br_get_sid(br);
	loff_t idx;
	u8 *supported = (u8*)buf;

	sid_dbg("\n");

	if (edgx_sysfs_tbl_params(ofs, count, sizeof(u8), &idx) ||
	    idx >= SID_IDENT_MAX || idx == SID_UNKNOWN)
		return -EINVAL;

	mutex_lock(&sid_br->lock);

	if (idx == SID_NULL)
		*supported = 1U;
	else
		*supported = 0;

	mutex_unlock(&sid_br->lock);
	return count;
}

static ssize_t sid_ident_params_read(struct file *filp, struct kobject *kobj,
				     struct bin_attribute *bin_attr,
				     char *buf, loff_t ofs, size_t count)
{
	struct edgx_br *br = edgx_dev2br(kobj_to_dev(kobj));
	struct edgx_sid_br *sid_br = edgx_br_get_sid(br);
	struct sid_rb_entry *rbe;
	struct sid_o_entry *oe;
	struct ident_params *params = (struct ident_params*)buf;
	loff_t	idx;

	sid_dbg("\n");
	if (edgx_sysfs_tbl_params(ofs, count, sizeof(struct ident_params),
				  &idx) || idx > SID_MAX_ORD)
		return -EINVAL;

	mutex_lock(&sid_br->lock);

	rbe = sid_rb_search(&sid_br->rb_o_root, idx);
	if (!rbe) {
		edgx_br_err(br, "ORD %u does not exist\n", (u16)idx);
		mutex_unlock(&sid_br->lock);
		return -ENOENT;
	}
	oe = rbe_to_o_entry(rbe);
	ether_addr_copy(params->addr, oe->tmvt_entry->addr);
	params->vid = oe->tmvt_entry->vid;
	params->tag =  oe->tmvt_entry->tag;
	params->str_hdl = oe->tmvt_o_entry->str_hdl;
	params->id_type = SID_NULL;

	mutex_unlock(&sid_br->lock);

	return count;
}

static ssize_t sid_ident_params_write(struct file *filp, struct kobject *kobj,
				     struct bin_attribute *bin_attr,
				     char *buf, loff_t ofs, size_t count)
{
	struct edgx_br *br = edgx_dev2br(kobj_to_dev(kobj));
	struct edgx_sid_br *sid_br = edgx_br_get_sid(br);
	loff_t	idx;
	int ret;
	struct sid_rb_entry *rbe;
	struct sid_tmvt_entry *tmvt_entry;
	struct sid_tmvt_entry *new_tmvt_entry = NULL;
	u64 tmvt_key;
	struct ident_params *params = (struct ident_params*)buf;

	sid_dbg("\n");
	if (edgx_sysfs_tbl_params(ofs, count, sizeof(struct ident_params),
				  &idx) || idx > SID_MAX_ORD)
		return -EINVAL;

	if (!(params->tag == SID_TAGGED) &&
	    !((params->tag == SID_PRIORITY) && (params->vid == 0)) &&
	    !((params->tag == SID_ALL) && (params->vid == 0))) {
		edgx_br_err(br, "Create: unsupported VLAN tag and VID combination (supported: TAG = tagged; TAG = priority or all & VID = 0)\n");
		return -EINVAL;
	}

	if (params->id_type != SID_NULL) {
		edgx_br_err(br, "Create: Invalid stream identification type (supported type: NULL)\n");
		return -EINVAL;
	}
	mutex_lock(&sid_br->lock);

	rbe = sid_rb_search(&sid_br->rb_o_root, idx);
	if (rbe) {
		edgx_br_err(br, "Create: ORD %u already exists\n", (u16)idx);
		ret = -EEXIST;
		goto out_params_unlock;
	}

	tmvt_key = params_to_tmvt_key(params);
	rbe = sid_rb_search(&sid_br->rb_tmvt_root, tmvt_key);
	if(rbe) {
		tmvt_entry = rbe_to_tmvt_entry(rbe);
	}
	else {
		ret = sid_tmvt_create(sid_br, tmvt_key, params->tag,
				      params->vid, params->addr,
				      &new_tmvt_entry);
		if (ret)
			goto out_params_unlock;

		tmvt_entry = new_tmvt_entry;
	}

	ret = sid_o_create(sid_br, idx, tmvt_entry);
	if (ret)
		goto out_params_rel_tmvt;

	mutex_unlock(&sid_br->lock);

	sid_dump(sid_br);

	return count;

out_params_rel_tmvt:
	if (new_tmvt_entry)
		sid_tmvt_release(sid_br, new_tmvt_entry);
out_params_unlock:
	mutex_unlock(&sid_br->lock);
	return ret;
}

static ssize_t sid_delete_write(struct file *filp, struct kobject *kobj,
				struct bin_attribute *bin_attr,
				char *buf, loff_t ofs, size_t count)
{
	struct edgx_br *br = edgx_dev2br(kobj_to_dev(kobj));
	struct edgx_sid_br *sid_br = edgx_br_get_sid(br);
	struct sid_rb_entry *rbe;
	struct sid_o_entry *oe;
	struct sid_tmvt_entry *tmvte;
	loff_t	idx;

	sid_dbg("\n");
	if (edgx_sysfs_tbl_params(ofs, count, sizeof(u16),
				  &idx) || idx > SID_MAX_ORD)
		return -EINVAL;

	mutex_lock(&sid_br->lock);

	rbe = sid_rb_search(&sid_br->rb_o_root, (u16)idx);
	if (!rbe) {
		edgx_br_err(br, "Delete: ORD %u does not exist\n", (u16)idx);
		mutex_unlock(&sid_br->lock);
		return -ENOENT;
	}
	oe = rbe_to_o_entry(rbe);
	tmvte = oe->tmvt_entry;

	sid_o_release(sid_br, oe);
	sid_gen_port_str_hdls(sid_br, tmvte);
	if (sid_tmvt_is_empty(tmvte))
		sid_tmvt_release(sid_br, tmvte);

	sid_dump(sid_br);
	mutex_unlock(&sid_br->lock);
	return count;
}

static ssize_t sid_set_strhdl_write(struct file *filp, struct kobject *kobj,
				    struct bin_attribute *bin_attr,
				    char *buf, loff_t ofs, size_t count)
{
	struct edgx_br *br = edgx_dev2br(kobj_to_dev(kobj));
	struct edgx_sid_br *sid_br = edgx_br_get_sid(br);
	struct sid_rb_entry *rbe;
	struct sid_o_entry *oe;
	loff_t	idx;

	sid_dbg("\n");
	if (edgx_sysfs_tbl_params(ofs, count, sizeof(u16),
				  &idx) || idx > SID_MAX_ORD)
		return -EINVAL;

	mutex_lock(&sid_br->lock);

	rbe = sid_rb_search(&sid_br->rb_o_root, (u16)idx);
	if (!rbe) {
		edgx_br_err(br, "Set stream handle: ORD %u does not exist\n",
			    (u16)idx);
		mutex_unlock(&sid_br->lock);
		return -ENOENT;
	}
	oe = rbe_to_o_entry(rbe);

	if (*(u16*)buf >= sid_br->max_streams) {
		edgx_br_err(br, "Stream handle out of range\n");
		mutex_unlock(&sid_br->lock);
		return -EINVAL;
	}
	oe->tmvt_o_entry->str_hdl = *(u16*)buf;

	sid_gen_port_str_hdls(sid_br, oe->tmvt_entry);

	sid_dump(sid_br);
	mutex_unlock(&sid_br->lock);

	return count;
}

static ssize_t sid_port_pos_read(struct file *filp, struct kobject *kobj,
				 struct bin_attribute *bin_attr,
				 char *buf, loff_t ofs, size_t count)
{
	struct edgx_br *br = edgx_dev2br(kobj_to_dev(kobj));
	struct edgx_sid_br *sid_br = edgx_br_get_sid(br);
	struct port_list *plist = (struct port_list *)buf;
	struct sid_rb_entry *rbe;
	struct sid_o_entry *oe;
	loff_t	idx;

	sid_dbg("\n");
	if (edgx_sysfs_tbl_params(ofs, count, sizeof(struct port_list),
				  &idx) || idx > SID_MAX_ORD)
		return -EINVAL;

	mutex_lock(&sid_br->lock);

	rbe = sid_rb_search(&sid_br->rb_o_root, (u16)idx);
	if (!rbe) {
		edgx_br_err(br, "Port & position read: ORD %u does not exist\n",
			    (u16)idx);
		mutex_unlock(&sid_br->lock);
		return -ENOENT;
	}
	oe = rbe_to_o_entry(rbe);
	memset(plist, 0, sizeof(*plist));
	plist->out_fac_in = oe->tmvt_o_entry->port_mask;

	mutex_unlock(&sid_br->lock);

	return count;
}

static int sid_port_pos_set(struct edgx_sid_br *sid_br, u16 ord,
			    const struct port_pos *port_pos, bool clear,
			    const char *msg)
{
	struct sid_rb_entry *rbe;
	struct sid_o_entry *oe;

	if (port_pos->port > sid_br->port_cnt) {
		edgx_br_err(sid_br->parent, "%s failed: Port doesn't exist\n",
			    msg);
		return -EINVAL;
	}
	if (port_pos->pos != OUT_FAC_IN) {
		edgx_br_err(sid_br->parent, "%s failed: Only Out-Fac-In position supported\n",
			     msg);
		return -EINVAL;
	}
	rbe = sid_rb_search(&sid_br->rb_o_root, ord);
	if (!rbe) {
		edgx_br_err(sid_br->parent, "%s failed: ORD %u does not exist\n",
			    msg, ord);
		return -ENOENT;
	}
	oe = rbe_to_o_entry(rbe);

	if (clear)
		oe->tmvt_o_entry->port_mask &= ~BIT(port_pos->port);
	else
		oe->tmvt_o_entry->port_mask |= BIT(port_pos->port);

	sid_gen_port_str_hdls(sid_br, oe->tmvt_entry);

	sid_dump(sid_br);
	return 0;
}

static ssize_t sid_port_pos_write(struct file *filp, struct kobject *kobj,
				  struct bin_attribute *bin_attr,
				  char *buf, loff_t ofs, size_t count)
{
	struct edgx_br *br = edgx_dev2br(kobj_to_dev(kobj));
	struct edgx_sid_br *sid_br = edgx_br_get_sid(br);
	loff_t	idx;
	int ret;

	sid_dbg("\n");
	if (edgx_sysfs_tbl_params(ofs, count, sizeof(struct port_pos),
				  &idx) || idx > SID_MAX_ORD) {
		edgx_br_err(sid_br->parent, "Add port failed: Wrong index %u\n",
			    (unsigned int)idx);
		return -EINVAL;
	}

	mutex_lock(&sid_br->lock);
	ret = sid_port_pos_set(sid_br, (u16)idx, (struct port_pos*)buf,
			       false, "Add port");
	mutex_unlock(&sid_br->lock);
	return ret ? ret : count;
}

static ssize_t sid_port_pos_del_write(struct file *filp, struct kobject *kobj,
				      struct bin_attribute *bin_attr,
				      char *buf, loff_t ofs, size_t count)
{
	struct edgx_br *br = edgx_dev2br(kobj_to_dev(kobj));
	struct edgx_sid_br *sid_br = edgx_br_get_sid(br);
	loff_t	idx;
	int ret;

	sid_dbg("\n");
	if (edgx_sysfs_tbl_params(ofs, count, sizeof(struct port_pos),
				  &idx) || idx > SID_MAX_ORD)
		return -EINVAL;

	mutex_lock(&sid_br->lock);
	ret = sid_port_pos_set(sid_br,  (u16)idx, (struct port_pos*)buf,
			       true, "Delete port");
	mutex_unlock(&sid_br->lock);
	return ret ? ret : count;
}

static u32 _edgx_sid_get_cnt(struct edgx_sid_br *sid, u16 str_hdl, u8 port,
			     enum cnt_type type)
{
	u16 capture = port | (str_hdl << 4);
	u32 val = 0;

	edgx_wr16(sid->counter_iobase, SID_CNT_CAPT2, capture);

	do {
		capture = edgx_rd16(sid->counter_iobase, SID_CNT_CAPT0);
	} while (capture & BIT(4));

	if (type == CNT_IN)
		val = (edgx_rd16(sid->counter_iobase, SID_CNT_FRAMES_IN_L) |
		      (((u32)edgx_rd16(sid->counter_iobase,
				       SID_CNT_FRAMES_IN_H)) << 16));
	else
		val = (edgx_rd16(sid->counter_iobase, SID_CNT_FRAMES_OUT_L) |
		      (((u32)edgx_rd16(sid->counter_iobase,
				       SID_CNT_FRAMES_OUT_H)) << 16));

	return val;
}

u32 edgx_sid_get_cnt(struct edgx_sid_br *sid, u16 str_hdl, u8 port,
		     enum cnt_type type)
{
	u32 ret;

	mutex_lock(&sid->lock);
	ret = _edgx_sid_get_cnt(sid, str_hdl, port, type);
	mutex_unlock(&sid->lock);

	return ret;
}

static ssize_t sid_cps_in_read(struct file *filp, struct kobject *kobj,
			       struct bin_attribute *bin_attr,
			       char *buf, loff_t ofs, size_t count)
{
	struct edgx_pt *pt = edgx_dev2pt(kobj_to_dev(kobj));
	struct edgx_sid_br *sid_br = edgx_br_get_sid(edgx_pt_get_br(pt));
	loff_t	idx;

	if (edgx_sysfs_tbl_params(ofs, count, sizeof(u32),
				  &idx) || idx >= sid_br->max_streams)
		return -EINVAL;

	mutex_lock(&sid_br->lock);

	*(u32*)buf = _edgx_sid_get_cnt(sid_br, idx, edgx_pt_get_id(pt), CNT_IN);

	mutex_unlock(&sid_br->lock);
	return count;
}

static ssize_t sid_cps_out_read(struct file *filp, struct kobject *kobj,
				struct bin_attribute *bin_attr,
				char *buf, loff_t ofs, size_t count)
{
	struct edgx_pt *pt = edgx_dev2pt(kobj_to_dev(kobj));
	struct edgx_sid_br *sid_br = edgx_br_get_sid(edgx_pt_get_br(pt));
	loff_t	idx;

	if (edgx_sysfs_tbl_params(ofs, count, sizeof(u32),
				  &idx) || idx >= sid_br->max_streams)
		return -EINVAL;

	mutex_lock(&sid_br->lock);

	/* Standard deviation: It is not clear from the standard
	 * when should the IP increment this counter.
	 * Ignoring for now to avoid confusion.
	 */
	/*	*(u32*)buf = _edgx_sid_get_cnt(sid_br, idx, edgx_pt_get_id(pt),
					       CNT_OUT);
	*/
	*(u32*)buf = 0U;

	mutex_unlock(&sid_br->lock);
	return count;
}

EDGX_DEV_ATTR_RO(sid_cnt, "tsnSidCnt");
EDGX_DEV_ATTR_RO(sid_tab_len, "tsnSidTableLength");
EDGX_DEV_ATTR_RO(sid_max_sup, "tsnSidMaxSupported");
EDGX_DEV_ATTR_RO(sid_sup_ports, "tsnSidSupportedPorts");

EDGX_BIN_ATTR_RW(sid_ident_params, "tsnSidIdentParams",
		 SID_MAX_ORD_CNT * sizeof(struct ident_params));
EDGX_BIN_ATTR_WO(sid_delete, "tsnSidDelete", SID_MAX_ORD_CNT * sizeof(u16));
EDGX_BIN_ATTR_RW(sid_port_pos, "tsnSidPortPos",
		 SID_MAX_ORD_CNT * sizeof(struct port_list));
EDGX_BIN_ATTR_WO(sid_port_pos_del, "tsnSidPortPosDelete",
		 SID_MAX_ORD_CNT * sizeof(struct port_pos));
EDGX_BIN_ATTR_WO(sid_set_strhdl, "tsnSidSetStrHdl",
		 SID_MAX_ORD_CNT * sizeof(u16));
EDGX_BIN_ATTR_RO(sid_cps_in, "tsnCpsSidInputPackets",
		 SID_MAX_NO_STREAMS * sizeof(u32));
EDGX_BIN_ATTR_RO(sid_cps_out, "tsnCpsSidOutputPackets",
		 SID_MAX_NO_STREAMS * sizeof(u32));
EDGX_BIN_ATTR_RO(sid_list_entries, "tsnListEntries",
		 SID_MAX_ORD_CNT * sizeof(u16));
EDGX_BIN_ATTR_RO(sid_sup_ident_types, "tsnSupportedIdentTypes",
		 SID_IDENT_MAX * sizeof(u8));

static struct attribute *ieee1cb_sid_attr[] = {
	&dev_attr_sid_cnt.attr,
	&dev_attr_sid_tab_len.attr,
	&dev_attr_sid_max_sup.attr,
	&dev_attr_sid_sup_ports.attr,
	NULL,
};

static struct bin_attribute *ieee1cb_sid_binattr[] = {
	&bin_attr_sid_ident_params,
	&bin_attr_sid_delete,
	&bin_attr_sid_port_pos,
	&bin_attr_sid_port_pos_del,
	&bin_attr_sid_set_strhdl,
	&bin_attr_sid_list_entries,
	&bin_attr_sid_sup_ident_types,
	NULL
};

static struct attribute_group sid_group = {
	.name  = "ieee8021SID",
	.attrs = ieee1cb_sid_attr,
	.bin_attrs = ieee1cb_sid_binattr,
};

static struct bin_attribute *ieee1cb_sid_pt_bin_attr[] = {
	&bin_attr_sid_cps_in,
	&bin_attr_sid_cps_out,
	NULL
};

static struct attribute_group sid_pt_group = {
	.name  = "ieee8021SID",
	.bin_attrs = ieee1cb_sid_pt_bin_attr,
};

int edgx_probe_sid(struct edgx_br *br, edgx_io_t *br_iobase,
		   struct edgx_sid_br **br_sid)
{
	int ret;
	struct edgx_sid_br *sid;
	const struct edgx_ifdesc *ifd;
	struct edgx_ifdesc        pifd;
	ptid_t                    ptid;
	const struct edgx_ifreq   ifreq = { .id = AC_SID_ID, .v_maj = 2 };

	if (!br)
		return -EINVAL;

	ifd = edgx_ac_get_if(&ifreq);
	if (!ifd)
		return -ENODEV;

	sid = kzalloc(sizeof(*sid), GFP_KERNEL);
	if (!(sid)) {
		edgx_br_err(br, "Cannot allocate Stream Identification\n");
		return -ENOMEM;
	}

	ret = edgx_br_sysfs_add(br, &sid_group);
	if (ret)
		goto out_sid_sysfs_add;

	sid->parent = br;
	sid->tab_iobase = br_iobase;
	sid->counter_iobase = ifd->iobase;
	mutex_init(&sid->lock);
	sid->rb_o_root = RB_ROOT;
	sid->rb_tmvt_root = RB_ROOT;
	sid->rb_ht_root = RB_ROOT;
	sid->port_cnt = min(edgx_br_get_num_ports(br), (u16)SID_MAX_NPORTS);
	sid->max_streams = max(edgx_br_get_generic_pow2(br, BR_GX_NSTREAMS),
			       edgx_br_get_generic_pow2(br,
							BR_GX_PSFP_NSTREAMS));
	sid->tab_row_cnt = edgx_br_get_generic_pow2(br, BR_GX_SID_TAB_NROWS);
	sid->sup_port_mask = (u16)ifd->ptmap << 1; /* TODO: Check port numbering! */

	/* XOR with VID in first two columns */
	edgx_wr16(sid->tab_iobase, 0x282, 0x5);

	edgx_ac_for_each_ifpt(ptid, ifd, &pifd) {
		struct edgx_pt *pt = edgx_br_get_brpt(br, ptid);

		if (pt)
			edgx_pt_add_sysfs(pt, &sid_pt_group);
	}

	edgx_br_info(br, "Setup Stream Identification ... done\n");
	*br_sid = sid;
	return 0;

out_sid_sysfs_add:
	kfree(sid);
	return ret;
}

static void sid_release_all(struct edgx_sid_br *sid_br)
{
	struct sid_rb_entry *rbe;
	struct sid_o_entry *oe;
	struct sid_tmvt_entry *tmvte;

	rbe = sid_rb_first(&sid_br->rb_o_root);
	while (rbe) {
		oe = rbe_to_o_entry(rbe);
		tmvte = oe->tmvt_entry;

		sid_o_release(sid_br, oe);
		sid_gen_port_str_hdls(sid_br, tmvte);
		if (sid_tmvt_is_empty(tmvte))
			sid_tmvt_release(sid_br, tmvte);

		rbe = sid_rb_first(&sid_br->rb_o_root);
	}
}

void edgx_shutdown_sid(struct edgx_sid_br *sid)
{
	const struct edgx_ifdesc *ifd;
	struct edgx_ifdesc        pifd;
	ptid_t                    ptid;
	const struct edgx_ifreq   ifreq = { .id = AC_SID_ID, .v_maj = 2 };

	if (!sid)
		return;

	sid_release_all(sid);

	ifd = edgx_ac_get_if(&ifreq);
	if (!ifd)
		return;

	edgx_ac_for_each_ifpt(ptid, ifd, &pifd) {
		struct edgx_pt *pt = edgx_br_get_brpt(sid->parent, ptid);

		if (pt)
			edgx_pt_rem_sysfs(pt, &sid_pt_group);
	}

	kfree(sid);
}
