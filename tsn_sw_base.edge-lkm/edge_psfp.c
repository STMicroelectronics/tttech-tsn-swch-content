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

#include <linux/rbtree.h>
#include <linux/bitfield.h>
#include <linux/delay.h>

#include "edge_psfp.h"
#include "edge_util.h"
#include "edge_port.h"
#include "edge_link.h"
#include "edge_sched.h"
#include "edge_ac.h"
#include "edge_defines.h"

#define FILTER_CMD			0x500
#define FILTER_ROW			GENMASK(9, 0)
#define FILTER_PCP			GENMASK(13, 11)
#define FILTER_READ			0
#define FILTER_WRITE			BIT(14)
#define FILTER_TRANSFER			BIT(15)

#define FILTER_TBL_0			0x510
#define FILTER_MAX_FRAME_SZ		GENMASK(10, 0)
#define FILTER_BLK_OVERSZ		BIT(14)
#define FILTER_BLK_OVERSZ_ENA		BIT(15)

#define FILTER_TBL_1			0x512
#define FILTER_POLICER			GENMASK(11, 0)
#define FILTER_POLICER_NPRI		0
#define FILTER_POLICER_LPRI		BIT(14)
#define FILTER_POLICER_ENA		BIT(15)

#define FILTER_TBL_2			0x514
#define FILTER_GATE			GENMASK(3, 0)
#define FILTER_GATE_ENA			BIT(7)

#define CNT_CAPT0			0xF000
#define CNT_CAPT5			0xF00A
#define CNT_CAPT5_PRIO			GENMASK(2, 0)
#define CNT_CAPT5_STREAM		GENMASK(15, 4)
#define CNT_MATCHING			0xF400
#define CNT_P_FRAMESIZE			0xF404
#define CNT_P_SGATE			0xF408
#define CNT_P_DROP			0xF40C
#define CNT_CAPT4			0xF008
#define CNT_CAPT4_PORT			GENMASK(3, 0)
#define CNT_CAPT4_STREAM		GENMASK(15, 4)
#define CNT_HIT_A			0xF318
#define CNT_HIT_B			0xF31C

#define HIT_SGATE			0xF104
#define HIT_SGATE_MASK			GENMASK(3, 0)
#define HIT_TH_L			0xF100
#define HIT_TH_L_MIN			1U
#define HIT_TH_L_MAX			16382U
#define HIT_TH_H			0xF102
#define HIT_TH_H_MIN			2U
#define HIT_TH_H_MAX			16383U
#define HIT_TH_MASK 			GENMASK(13, 0)
#define HIT_TH_GRAN_NS			320U

/* Maximum possible number of schedule table rows */
#define EDGX_SCHED_HW_MAX_ROWS		(1024U)

#define PSFP_GT_SCHED_GRAN_NS		320

#define PSFP_FRAME_SZ_OVERHEAD		12U
#define PSFP_MAX_SDU			1506U
#define PSFP_MAX_FRAME_SZ		(PSFP_MAX_SDU + PSFP_FRAME_SZ_OVERHEAD)

#define PSFP_MAX_NO_STREAMS		1024U
#define PSFP_MAX_NO_FILTERS		(PSFP_MAX_NO_STREAMS * EDGX_BR_NUM_PCP)
#define PSFP_MAX_NO_GATES		16
#define PSFP_MAX_FMTR			0xFFFU

#define PSFP_WC_STR_HDL			-1
#define PSFP_WC_PCP			-1
#define PSFP_FMTR_NONE			-1

#define PSFP_GT_GATE_MASK		BIT(0)
#define PSFP_GT_IPV_ENA_MASK		BIT(1)
#define PSFP_GT_IPV_MASK		GENMASK(4, 2)

#define SGATE_CMD			0x550
#define SGATE_ROW			GENMASK(3, 0)
#define SGATE_READ			0
#define SGATE_WRITE			BIT(14)
#define SGATE_TRANSFER			BIT(15)

#define SGATE_TBL_0			0x560
#define SGATE_CLOSED_INV_RX_ENA		BIT(1)
#define SGATE_CLOSED_INV_RX		BIT(0)

#define PSFP_GT_DEF_GATE_STATES		PSFP_GT_GATE_MASK
#define PSFP_GT_OP_SET_GSTATES_IPV	0x00
#define PSFP_GT_IPV_NULL		-1

#ifdef EDGX_PSFP_DBG
#define DEBUG
#define edgx_psfp_dbg(_br, _fmt, ...) \
	pr_debug("bridge-%d: " _fmt, edgx_br_get_id(_br), ## __VA_ARGS__)
#define edgx_psfp_pt_dbg(_pt, _fmt, ...) \
	pr_debug("%s: " _fmt, edgx_pt_get_name(_pt), ## __VA_ARGS__)
#else
#define edgx_psfp_dbg(_br, _fmt, ...)		do { } while (0)
#define edgx_psfp_pt_dbg(_br, _fmt, ...)	do { } while (0)
#endif

struct psfp_flt_sysfs_fsl {
	u32 max_sdu;
	s32 fmtr;
} __packed;

struct psfp_flt_data {
	u32 gate;
	struct psfp_flt_sysfs_fsl fsl;
	bool blk_oversz_ena;
	bool blk_oversz;
} __packed;

struct psfp_flt_sysfs_entry {
	s32 str_hdl;
	s32 pcp;
	struct psfp_flt_data data;
} __packed;

static const struct psfp_flt_data psfp_flt_data_default = {
	.gate =	U32_MAX,
	.fsl = {
		.max_sdu = PSFP_MAX_SDU,
		.fmtr    = PSFP_FMTR_NONE,
	},
	.blk_oversz_ena = false,
	.blk_oversz = false,
};

enum psfp_cnt_type {
	/* provided by HW */
	PSFP_CNT_MATCHING_FRAMES,
	PSFP_CNT_PASSING_FRAMES,
	PSFP_CNT_PASSING_SDU,
	PSFP_CNT_RED_FRAMES,

	/* number of counters provided by HW */
	PSFP_CNT_HW_CNT,

	/* rest are calculated by driver */
	PSFP_CNT_NOT_PASSING_FRAMES = PSFP_CNT_HW_CNT,
	PSFP_CNT_NOT_PASSING_SDU,
};

struct psfp_flt_cnt {
	u32 raw[PSFP_CNT_HW_CNT];
};

/* Track filters in RBT */
struct psfp_flt_str_entry;

/* Track filters of streams with (handle,PCP) in RBT */
struct psfp_flt_sub_entry {
	struct rb_node rbnode;
	struct psfp_flt_str_entry *str_entry;
};

struct psfp_flt_entry {
	struct rb_node rbnode;
	u32 filter;
	struct psfp_flt_sub_entry sub_entry;
	struct psfp_flt_data data;
	struct psfp_flt_cnt cnt_ref;
};

static inline struct psfp_flt_entry *to_flt_entry(
		const struct psfp_flt_sub_entry *sub)
{
	return container_of(sub, struct psfp_flt_entry, sub_entry);
}

/* Track streams with (handle,PCP) in RBT */
struct psfp_flt_str_entry {
	struct rb_node rbnode;
	struct rb_root sub_root;	/* entries: psfp_flt_sub_entry */
	s32 str_hdl;
	s32 pcp;
};

#define PSFP_FLT_SUB_IS_VALID(sube) \
	((sube) && !RB_EMPTY_NODE(&(sube)->rbnode))

struct psfp_limits {
	u32 max_str_cnt;
	u32 max_flt_cnt;
	u32 max_gate_cnt;
	u32 max_fmtr_cnt;
	u32 res_fmtr_cnt;
	u32 max_gcl_len;
};

struct psfp_flt {
	struct mutex lock;
	edgx_io_t *iobase;
	struct rb_root flt_root;	/* entries: psfp_flt_entry */
	struct rb_root str_root;	/* entries: psfp_flt_str_entry */
	const struct psfp_limits *limits;
	const struct psfp_flt_sub_entry *(*cur_sub)[EDGX_BR_NUM_PCP];
};

enum psfp_hit_th_type {
	PSFP_HIT_TH_LOW,
	PSFP_HIT_TH_HIGH,
};

enum psfp_hit_cnt_type {
	PSFP_CNT_HIT_A,
	PSFP_CNT_HIT_B,
};

struct psfp_hit {
	struct edgx_br *parent;
	struct mutex lock;
	edgx_io_t *iobase;
	unsigned int th_low_ns;
	unsigned int th_high_ns;
};

/* NOTE: layout and size same as in edgx_sched_tab_entry except for ipv */
struct psfp_gt_tab_entry {
	u32 time_interval;
	u8 operation_name;
	u8 gate_state;
	s8 ipv;
	u8 padding;
} __packed;

struct psfp_gt_sgate_entry {
	bool closed_inv_rx_ena;
	bool closed_inv_rx;
};

#define PSFP_SGATE_INV_RX_ENA	BIT(0)
#define PSFP_SGATE_INV_RX	BIT(1)

struct psfp_gt_dly_pt {
	struct edgx_pt *parent;
	int speed;
	ktime_t lnk_rx_min, lnk_rx_max;
	ktime_t i2sg_min, i2sg_max;
	u64 w2sg_min, w2sg_max, w2sg;
	s64 sgate_delay;
};

struct psfp_gt_dly_sched {
	u64 clk;
	u64 s2g_min, s2g_max, s2g, s2sg;
	s64 retard;
};

struct psfp_gt {
	struct edgx_sched_event sched_evt;
	struct psfp_gt_dly_sched dly_sched;
	struct psfp_gt_dly_pt dly_pt[EDGX_BR_MAX_PORTS];
	struct psfp_gt_dly_pt dly_pt_ref;
};

struct psfp_gates {
	struct edgx_br *parent;
	edgx_io_t *iobase;
	const struct psfp_limits *limits;
	struct edgx_sched_com *sched_com;
	struct psfp_gt gt[];
};

struct edgx_psfp {
	struct edgx_br *parent;
	struct psfp_limits limits;
	struct psfp_flt flt;
	struct psfp_hit *hit_cnt;
	struct psfp_gates gates;	/* must be last */
};

/* PSFP Stream Filters */

static inline void _psfp_rb_erase(struct rb_node *node, struct rb_root *root)
{
	rb_erase(node, root);
	RB_CLEAR_NODE(node);
}

static inline struct edgx_br *edgx_psfp_flt_parent(const struct psfp_flt *flt)
{
	return container_of(flt, struct edgx_psfp, flt)->parent;
}

static struct psfp_flt_entry *edgx_psfp_flt_create(u32 filter)
{
	struct psfp_flt_entry *flte;

	flte = kzalloc(sizeof(*flte), GFP_KERNEL);
	if (!flte)
		return NULL;
	flte->filter = filter;
	flte->data = psfp_flt_data_default;
	return flte;
}

static int edgx_psfp_flt_insert(struct psfp_flt *flt,
				struct psfp_flt_entry *new)
{
	struct rb_node **pp = &flt->flt_root.rb_node;
	struct rb_node *p = NULL;
	struct psfp_flt_entry *flte = NULL;

	while (*pp) {
		p = *pp;
		flte = rb_entry(p, struct psfp_flt_entry, rbnode);
		if (new->filter < flte->filter)
			pp = &(*pp)->rb_left;
		else if (new->filter > flte->filter)
			pp = &(*pp)->rb_right;
		else
			return -EINVAL;
	}

	rb_link_node(&new->rbnode, p, pp);
	rb_insert_color(&new->rbnode, &flt->flt_root);

	return 0;
}

static struct psfp_flt_entry *edgx_psfp_flt_search(struct psfp_flt *flt,
						   u32 filter)
{
	struct rb_node *node = flt->flt_root.rb_node;

	while (node) {
		struct psfp_flt_entry *flte =
			rb_entry(node, struct psfp_flt_entry, rbnode);

		if (filter < flte->filter)
			node = node->rb_left;
		else if (filter > flte->filter)
			node = node->rb_right;
		else
			return flte;
	}
	return NULL;
}

static struct psfp_flt_str_entry *edgx_psfp_flt_str_create(s32 str_hdl,
							   s32 pcp)
{
	struct psfp_flt_str_entry *stre;

	stre = kzalloc(sizeof(*stre), GFP_KERNEL);
	if (!stre)
		return NULL;
	stre->sub_root = RB_ROOT;
	stre->str_hdl = str_hdl;
	stre->pcp = pcp;
	return stre;
}

static int edgx_psfp_flt_str_insert(struct psfp_flt *flt,
				    struct psfp_flt_str_entry *new)
{
	struct rb_node **pp = &flt->str_root.rb_node;
	struct rb_node *p = NULL;
	struct psfp_flt_str_entry *stre = NULL;

	while (*pp) {
		p = *pp;
		stre = rb_entry(p, struct psfp_flt_str_entry, rbnode);
		if (new->str_hdl < stre->str_hdl)
			pp = &(*pp)->rb_left;
		else if (new->str_hdl > stre->str_hdl)
			pp = &(*pp)->rb_right;
		else if (new->pcp < stre->pcp)
			pp = &(*pp)->rb_left;
		else if (new->pcp > stre->pcp)
			pp = &(*pp)->rb_right;
		else
			return -EINVAL;
	}

	rb_link_node(&new->rbnode, p, pp);
	rb_insert_color(&new->rbnode, &flt->str_root);

	return 0;
}

static struct psfp_flt_str_entry *edgx_psfp_flt_str_search(
		struct psfp_flt *flt, s32 str_hdl, s32 pcp)
{
	struct rb_node *node = flt->str_root.rb_node;

	while (node) {
		struct psfp_flt_str_entry *stre =
			rb_entry(node, struct psfp_flt_str_entry, rbnode);

		if (str_hdl < stre->str_hdl)
			node = node->rb_left;
		else if (str_hdl > stre->str_hdl)
			node = node->rb_right;
		else if (pcp < stre->pcp)
			node = node->rb_left;
		else if (pcp > stre->pcp)
			node = node->rb_right;
		else
			return stre;
	}
	return NULL;
}

static const struct psfp_flt_sub_entry *edgx_psfp_flt_first_sub(
		struct psfp_flt *flt, s32 str_hdl, s32 pcp)
{
	struct psfp_flt_str_entry *stre = edgx_psfp_flt_str_search(flt,
								   str_hdl,
								   pcp);
	struct psfp_flt_sub_entry *sube = NULL;

	if (stre) {
		struct rb_node *first = rb_first(&stre->sub_root);

		sube = rb_entry(first, struct psfp_flt_sub_entry, rbnode);
	}
	return sube;
}

static const struct psfp_flt_sub_entry *edgx_psfp_flt_str_find_eff_sub(
		struct psfp_flt *flt,
		s32 str_hdl, s32 pcp,
		const struct psfp_flt_sub_entry **cand,
		size_t cand_cnt)
{
	const struct psfp_flt_sub_entry *eff = NULL;

	edgx_psfp_dbg(edgx_psfp_flt_parent(flt),
		      "PSFP: Find effective sub for %d/%d from %zu cands\n",
		      str_hdl, pcp, cand_cnt);
	while (cand_cnt-- > 0) {
		edgx_psfp_dbg(edgx_psfp_flt_parent(flt),
			      "PSFP: ^ check cand %zu %s %d/%d filter %u\n",
			      cand_cnt,
			      *cand ?
			      (!RB_EMPTY_NODE(&(*cand)->rbnode) ? "valid" :
			      "deleted") :
			      "none",
			      *cand ? (*cand)->str_entry->str_hdl : 0,
			      *cand ? (*cand)->str_entry->pcp : 0,
			      *cand ? to_flt_entry(*cand)->filter : 0);
		if (*cand && (!eff || ((to_flt_entry(eff)->filter >
					to_flt_entry(*cand)->filter))))
			eff = *cand;
		cand++;
	}
	edgx_psfp_dbg(edgx_psfp_flt_parent(flt),
		      "PSFP: ^ result %s %d/%d filter %u\n",
		      eff ? "found" : "none",
		      eff ? eff->str_entry->str_hdl : 0,
		      eff ? eff->str_entry->pcp : 0,
		      eff ? to_flt_entry(eff)->filter : 0);
	return eff;
}

static int edgx_psfp_flt_sub_insert(struct psfp_flt_str_entry *stre,
				    struct psfp_flt_sub_entry *new)
{
	struct rb_node **pp = &stre->sub_root.rb_node;
	struct rb_node *p = NULL;
	struct psfp_flt_sub_entry *sube = NULL;
	u32 filter_new = to_flt_entry(new)->filter;
	u32 filter;

	while (*pp) {
		p = *pp;
		sube = rb_entry(p, struct psfp_flt_sub_entry, rbnode);
		filter = to_flt_entry(sube)->filter;
		if (filter_new < filter)
			pp = &(*pp)->rb_left;
		else if (filter_new > filter)
			pp = &(*pp)->rb_right;
		else
			return -EINVAL;
	}

	rb_link_node(&new->rbnode, p, pp);
	rb_insert_color(&new->rbnode, &stre->sub_root);

	return 0;
}

static inline bool edgx_psfp_flt_str_is_wc(
		const struct psfp_flt_str_entry *stre)
{
	return stre->str_hdl == PSFP_WC_STR_HDL ||
	       stre->pcp == PSFP_WC_PCP;
}

static int edgx_psfp_flt_check(const struct psfp_flt *flt,
			       const struct psfp_flt_sysfs_entry *flt_def)
{
	if (!(flt_def->str_hdl == PSFP_WC_STR_HDL ||
	      (flt_def->str_hdl >= 0 &&
	       flt_def->str_hdl < flt->limits->max_str_cnt))) {
		edgx_br_warn(edgx_psfp_flt_parent(flt),
			     "PSFP: Invalid stream handle %d, "
			     "max. supported < %u\n",
			     flt_def->str_hdl,
			     flt->limits->max_str_cnt);
		return -EINVAL;
	}
	if (!(flt_def->pcp == PSFP_WC_PCP ||
	      (flt_def->pcp >= 0 &&
	       flt_def->pcp < EDGX_BR_NUM_PCP))) {
		edgx_br_warn(edgx_psfp_flt_parent(flt),
			     "PSFP: Invalid PCP %d\n", flt_def->pcp);
		return -EINVAL;
	}
	if (!(flt_def->data.gate < flt->limits->max_gate_cnt)) {
		edgx_br_warn(edgx_psfp_flt_parent(flt),
			     "PSFP: Invalid gate instance %d, "
			     "max. supported < %u\n",
			     flt_def->data.gate,
			     flt->limits->max_gate_cnt);
		return -EINVAL;
	}
	if (!(flt_def->data.fsl.fmtr == PSFP_FMTR_NONE ||
	      (flt_def->data.fsl.fmtr >= 0 &&
	       flt_def->data.fsl.fmtr < flt->limits->max_fmtr_cnt))) {
		edgx_br_warn(edgx_psfp_flt_parent(flt),
			     "PSFP: Invalid flow meter instance %d, "
			     "max. supported < %u\n",
			     flt_def->data.fsl.fmtr,
			     flt->limits->max_fmtr_cnt);
		return -EINVAL;
	}

	return 0;
}

static inline void _edgx_psfp_flt_wait_cmd(struct psfp_flt *flt)
{
	u16 cmd;

	usleep_range(300, 400);
	do {
		cmd = edgx_get16(flt->iobase, FILTER_CMD, 15, 15);
	} while (cmd);
}

static void _edgx_psfp_flt_write_hw(struct psfp_flt *flt,
				    s32 str_hdl, s32 pcp,
				    const struct psfp_flt_data *fltd)
{
	u16 cmd    = FIELD_PREP(FILTER_ROW, str_hdl) |
		     FIELD_PREP(FILTER_PCP, pcp) |
		     FILTER_WRITE |
		     FILTER_TRANSFER;
	u16 table0 = FIELD_PREP(FILTER_MAX_FRAME_SZ, PSFP_MAX_FRAME_SZ) |
		     FIELD_PREP(FILTER_BLK_OVERSZ, false) |
		     FIELD_PREP(FILTER_BLK_OVERSZ_ENA, false);
	u16 table1 = FIELD_PREP(FILTER_POLICER, 0) |
		     FILTER_POLICER_NPRI |
		     FIELD_PREP(FILTER_POLICER_ENA, false);
	u16 table2 = FIELD_PREP(FILTER_GATE, 0) |
		     FIELD_PREP(FILTER_GATE_ENA, false);

	WARN_ON(str_hdl < 0);
	WARN_ON(pcp < 0);
	if (fltd) {
		u32 max_frame_sz = fltd->fsl.max_sdu + PSFP_FRAME_SZ_OVERHEAD;
		u32 fmtr = 0;

		if (fltd->fsl.fmtr != PSFP_FMTR_NONE)
			fmtr = fltd->fsl.fmtr + flt->limits->res_fmtr_cnt;

		table0 = FIELD_PREP(FILTER_MAX_FRAME_SZ,
				    min(max_frame_sz, PSFP_MAX_FRAME_SZ)) |
			 FIELD_PREP(FILTER_BLK_OVERSZ,
				    !!fltd->blk_oversz) |
			 FIELD_PREP(FILTER_BLK_OVERSZ_ENA,
				    !!fltd->blk_oversz_ena);
		table1 = FIELD_PREP(FILTER_POLICER, fmtr) |
			 FILTER_POLICER_NPRI |
			 FIELD_PREP(FILTER_POLICER_ENA,
				    fltd->fsl.fmtr != PSFP_FMTR_NONE);
		table2 = FIELD_PREP(FILTER_GATE, fltd->gate) |
			 FILTER_GATE_ENA;
	}
	edgx_psfp_dbg(edgx_psfp_flt_parent(flt),
		      "PSFP: Write filter %d/%d cmd 0x%04x "
		      "table0 0x%04x table1 0x%04x table2 0x%04x\n",
		      str_hdl, pcp, cmd, table0, table1, table2);
	edgx_wr16(flt->iobase, FILTER_TBL_0, table0);
	edgx_wr16(flt->iobase, FILTER_TBL_1, table1);
	edgx_wr16(flt->iobase, FILTER_TBL_2, table2);
	edgx_wr16(flt->iobase, FILTER_CMD, cmd);
	_edgx_psfp_flt_wait_cmd(flt);
}

static void _edgx_psfp_flt_read_hw(struct psfp_flt *flt,
				   s32 str_hdl, s32 pcp,
				   struct psfp_flt_data *fltd)
{
	u16 cmd    = FIELD_PREP(FILTER_ROW, str_hdl) |
		     FIELD_PREP(FILTER_PCP, pcp) |
		     FILTER_READ |
		     FILTER_TRANSFER;
	u16 table0 = 0;
	u16 table1 = 0;
	u16 table2 = 0;
	u32 max_frame_sz;

	WARN_ON(str_hdl < 0);
	WARN_ON(pcp < 0);

	edgx_wr16(flt->iobase, FILTER_CMD, cmd);
	_edgx_psfp_flt_wait_cmd(flt);
	table0 = edgx_rd16(flt->iobase, FILTER_TBL_0);
	table1 = edgx_rd16(flt->iobase, FILTER_TBL_1);
	table2 = edgx_rd16(flt->iobase, FILTER_TBL_2);
	if (FIELD_GET(FILTER_GATE_ENA, table2))
		fltd->gate = FIELD_GET(FILTER_GATE, table2);
	else
		fltd->gate = 0;
	max_frame_sz = FIELD_GET(FILTER_MAX_FRAME_SZ, table0);
	fltd->fsl.max_sdu = max_frame_sz - PSFP_FRAME_SZ_OVERHEAD;
	if (FIELD_GET(FILTER_POLICER_ENA, table1))
		fltd->fsl.fmtr = FIELD_GET(FILTER_POLICER, table1)
				 - flt->limits->res_fmtr_cnt;
	else
		fltd->fsl.fmtr = 0;
	fltd->blk_oversz_ena = FIELD_GET(FILTER_BLK_OVERSZ_ENA, table0);
	fltd->blk_oversz = FIELD_GET(FILTER_BLK_OVERSZ, table0);
	edgx_psfp_dbg(edgx_psfp_flt_parent(flt),
		      "PSFP: Read filter %d/%d cmd 0x%04x "
		      "table0 0x%04x table1 0x%04x table2 0x%04x\n",
		      str_hdl, pcp, cmd, table0, table1, table2);
}

static void edgx_psfp_flt_str_get_cnt_hw(struct psfp_flt *flt,
					 u16 str_hdl, u8 pcp,
					 struct psfp_flt_cnt *cnt)
{
	u16 capture = FIELD_PREP(CNT_CAPT5_PRIO, pcp) |
		      FIELD_PREP(CNT_CAPT5_STREAM, str_hdl);

	edgx_wr16(flt->iobase, CNT_CAPT5, capture);
	do {
		capture = edgx_get16(flt->iobase, CNT_CAPT0, 8, 8);
	} while (capture);

	cnt->raw[PSFP_CNT_MATCHING_FRAMES] = edgx_rd32(flt->iobase,
						       CNT_MATCHING);
	cnt->raw[PSFP_CNT_PASSING_FRAMES]  = edgx_rd32(flt->iobase,
						       CNT_P_SGATE);
	cnt->raw[PSFP_CNT_PASSING_SDU]     = edgx_rd32(flt->iobase,
						       CNT_P_FRAMESIZE);
	cnt->raw[PSFP_CNT_RED_FRAMES]      = edgx_rd32(flt->iobase,
						       CNT_P_DROP);
	edgx_psfp_dbg(edgx_psfp_flt_parent(flt),
		      "PSFP: hw cnt for %d/%d values %u %u %u %u\n",
		      str_hdl, pcp,
		      cnt->raw[PSFP_CNT_MATCHING_FRAMES],
		      cnt->raw[PSFP_CNT_PASSING_FRAMES],
		      cnt->raw[PSFP_CNT_PASSING_SDU],
		      cnt->raw[PSFP_CNT_RED_FRAMES]);
}

static void edgx_psfp_flt_update_cnt_ref(struct psfp_flt_entry *flte,
					 const struct psfp_flt_cnt *cur)
{
	unsigned int i;

	for (i = 0; i < ARRAY_SIZE(flte->cnt_ref.raw); i++)
		flte->cnt_ref.raw[i] = cur->raw[i] - flte->cnt_ref.raw[i];
}

static u32 edgx_psfp_flt_get_user_cnt(const struct psfp_flt_cnt *cur,
				      const struct psfp_flt_cnt *ref,
				      enum psfp_cnt_type cnt_type)
{
	static const struct psfp_flt_cnt cnt_zero = { .raw = { 0 } };
	u32 cnt;

	if (!ref)
		ref = &cnt_zero;

	switch (cnt_type) {
	case PSFP_CNT_MATCHING_FRAMES:
		cnt = cur->raw[cnt_type] - ref->raw[cnt_type];
		break;
	case PSFP_CNT_PASSING_FRAMES:
		cnt = cur->raw[cnt_type] - ref->raw[cnt_type];
		break;
	case PSFP_CNT_PASSING_SDU:
		cnt = cur->raw[cnt_type] - ref->raw[cnt_type];
		break;
	case PSFP_CNT_RED_FRAMES:
		cnt = cur->raw[cnt_type] - ref->raw[cnt_type];
		break;
	case PSFP_CNT_NOT_PASSING_FRAMES:
		cnt = cur->raw[PSFP_CNT_MATCHING_FRAMES] -
		      ref->raw[PSFP_CNT_MATCHING_FRAMES] -
		      cur->raw[PSFP_CNT_PASSING_FRAMES] +
		      ref->raw[PSFP_CNT_PASSING_FRAMES];
		break;
	case PSFP_CNT_NOT_PASSING_SDU:
		cnt = cur->raw[PSFP_CNT_MATCHING_FRAMES] -
		      ref->raw[PSFP_CNT_MATCHING_FRAMES] -
		      cur->raw[PSFP_CNT_PASSING_SDU] +
		      ref->raw[PSFP_CNT_PASSING_SDU];
		break;
	}

	return cnt;
}

/* Index to struct psfp_flt_iter cand array */
enum psfp_flt_cand {
	PSFP_FLT_STR_PCP,
	PSFP_FLT_WC_STR,
	PSFP_FLT_WC_PCP,
	PSFP_FLT_WC,
	PSFP_FLT_CAND_CNT,	/* must be last */
};

struct psfp_flt_iter {
	struct psfp_flt *flt;
	s32 str_hdl;
	s32 pcp;
	const struct psfp_flt_sub_entry *trigger;
	const struct psfp_flt_sub_entry *cand[PSFP_FLT_CAND_CNT];
	int (*func)(struct psfp_flt_iter *iter,
		    const struct psfp_flt_sub_entry *sube_eff,
		    s32 str_hdl,
		    s32 pcp);
};

static inline const struct psfp_flt_sub_entry *edgx_psfp_flt_get_cand(
		struct psfp_flt_iter *iter,
		s32 str_hdl,
		s32 pcp)
{
	if (PSFP_FLT_SUB_IS_VALID(iter->trigger))
		return iter->trigger;
	return edgx_psfp_flt_first_sub(iter->flt, str_hdl, pcp);
}

static int _edgx_psfp_flt_for_each_one(struct psfp_flt_iter *iter,
				       s32 str_hdl, s32 pcp)
{
	struct psfp_flt *flt = iter->flt;
	const struct psfp_flt_sub_entry *sube = iter->trigger;
	const struct psfp_flt_sub_entry *sube_eff =
			edgx_psfp_flt_str_find_eff_sub(flt, str_hdl, pcp,
						       iter->cand,
						       ARRAY_SIZE(iter->cand));

	edgx_psfp_dbg(edgx_psfp_flt_parent(flt),
		      "PSFP: filter %d/%d %s for %d/%d\n",
		      sube->str_entry->str_hdl,
		      sube->str_entry->pcp,
		      sube_eff ?
		      (sube_eff == sube ? "active" : "inactive") :
		      "none",
		      str_hdl, pcp);
	return iter->func(iter, sube_eff, str_hdl, pcp);
}

static int edgx_psfp_flt_for_each_one(struct psfp_flt_iter *iter)
{
	struct psfp_flt *flt = iter->flt;
	const struct psfp_flt_sub_entry **cand = iter->cand;

	*cand++ = edgx_psfp_flt_get_cand(iter, iter->str_hdl, iter->pcp);
	*cand++ = edgx_psfp_flt_first_sub(flt, iter->str_hdl, PSFP_WC_PCP);
	*cand++ = edgx_psfp_flt_first_sub(flt, PSFP_WC_STR_HDL, iter->pcp);
	*cand++ = edgx_psfp_flt_first_sub(flt, PSFP_WC_STR_HDL, PSFP_WC_PCP);

	return _edgx_psfp_flt_for_each_one(iter, iter->str_hdl, iter->pcp);
}

static int edgx_psfp_flt_for_each_wc(struct psfp_flt_iter *iter)
{
	struct psfp_flt *flt = iter->flt;
	const struct psfp_flt_sub_entry *wc_str[EDGX_BR_NUM_PCP];
	const struct psfp_flt_sub_entry **cand = iter->cand;
	u32 str_hdl;
	unsigned pcp;
	int ret;

	cand[PSFP_FLT_WC] = edgx_psfp_flt_get_cand(iter, PSFP_WC_STR_HDL,
						   PSFP_WC_PCP);

	for (pcp = 0; pcp < EDGX_BR_NUM_PCP; pcp++)
		wc_str[pcp] = edgx_psfp_flt_first_sub(flt, PSFP_WC_STR_HDL,
						      pcp);

	for (str_hdl = 0; str_hdl < flt->limits->max_str_cnt; str_hdl++) {
		cand[PSFP_FLT_WC_PCP] =
				edgx_psfp_flt_first_sub(flt, str_hdl,
							PSFP_WC_PCP);
		for (pcp = 0; pcp < EDGX_BR_NUM_PCP; pcp++) {
			cand[PSFP_FLT_WC_STR] = wc_str[pcp];
			cand[PSFP_FLT_STR_PCP] =
					edgx_psfp_flt_first_sub(flt, str_hdl,
								pcp);
			ret = _edgx_psfp_flt_for_each_one(iter, str_hdl, pcp);
			if (ret)
				break;
		}
	}
	return ret;
}

static int edgx_psfp_flt_for_each_wc_pcp(struct psfp_flt_iter *iter)
{
	struct psfp_flt *flt = iter->flt;
	const struct psfp_flt_sub_entry **cand = iter->cand;
	unsigned pcp;
	int ret;

	cand[PSFP_FLT_WC_PCP] = edgx_psfp_flt_get_cand(iter, iter->str_hdl,
						       PSFP_WC_PCP);
	cand[PSFP_FLT_WC] = edgx_psfp_flt_first_sub(flt, PSFP_WC_STR_HDL,
						    PSFP_WC_PCP);

	for (pcp = 0; pcp < EDGX_BR_NUM_PCP; pcp++) {
		cand[PSFP_FLT_STR_PCP] = edgx_psfp_flt_first_sub(flt,
								 iter->str_hdl,
								 pcp);
		cand[PSFP_FLT_WC_STR] =
				edgx_psfp_flt_first_sub(flt, PSFP_WC_STR_HDL,
							pcp);
		ret = _edgx_psfp_flt_for_each_one(iter, iter->str_hdl, pcp);
		if (ret)
			break;
	}
	return ret;
}

static int edgx_psfp_flt_for_each_wc_str_hdl(struct psfp_flt_iter *iter)
{
	struct psfp_flt *flt = iter->flt;
	const struct psfp_flt_sub_entry **cand = iter->cand;
	u32 str_hdl;
	int ret;

	cand[PSFP_FLT_WC_STR] = edgx_psfp_flt_get_cand(iter, PSFP_WC_STR_HDL,
						       iter->pcp);
	cand[PSFP_FLT_WC] = edgx_psfp_flt_first_sub(flt, PSFP_WC_STR_HDL,
						    PSFP_WC_PCP);

	for (str_hdl = 0; str_hdl < flt->limits->max_str_cnt; str_hdl++) {
		cand[PSFP_FLT_STR_PCP] =
				edgx_psfp_flt_first_sub(flt, str_hdl,
							iter->pcp);
		cand[PSFP_FLT_WC_PCP] =
				edgx_psfp_flt_first_sub(flt, str_hdl,
							PSFP_WC_PCP);
		ret = _edgx_psfp_flt_for_each_one(iter, str_hdl, iter->pcp);
		if (ret)
			break;
	}
	return ret;
}

static int edgx_psfp_flt_for_each(
		struct psfp_flt *flt,
		const struct psfp_flt_sub_entry *trigger,
		int (*func)(struct psfp_flt_iter *iter,
			    const struct psfp_flt_sub_entry *sube_eff,
			    s32 str_hdl, s32 pcp),
		struct psfp_flt_iter *iter)
{
	int ret;

	iter->func = func;
	iter->flt = flt;
	iter->trigger = trigger;
	if (iter->str_hdl == PSFP_WC_STR_HDL && iter->pcp == PSFP_WC_PCP)
		ret = edgx_psfp_flt_for_each_wc(iter);
	else if (iter->str_hdl != PSFP_WC_STR_HDL && iter->pcp == PSFP_WC_PCP)
		ret = edgx_psfp_flt_for_each_wc_pcp(iter);
	else if (iter->str_hdl == PSFP_WC_STR_HDL && iter->pcp != PSFP_WC_PCP)
		ret = edgx_psfp_flt_for_each_wc_str_hdl(iter);
	else
		ret = edgx_psfp_flt_for_each_one(iter);
	return ret;
}

static void edgx_psfp_flt_upd_cnt(struct psfp_flt_iter *iter,
				  s32 str_hdl,
				  s32 pcp,
				  const struct psfp_flt_sub_entry *eff)
{
	struct psfp_flt *flt = iter->flt;
	const struct psfp_flt_sub_entry *cur = flt->cur_sub[str_hdl][pcp];
	struct psfp_flt_cnt cnt_cur;
	bool update_eff;
	bool update_cur;

	if (eff == cur)
		return;

	/* NOTE: FLEXDE-5096: Standard Deviation:
	 * Wildcard filter counters are not supported.
	 * Avoid also reading counters unnecessarily.
	 */
	update_eff = eff && !edgx_psfp_flt_str_is_wc(eff->str_entry);
	update_cur = cur && iter->trigger == eff &&
		     !edgx_psfp_flt_str_is_wc(cur->str_entry);
	if (!update_eff && !update_cur)
		return;

	edgx_psfp_dbg(edgx_psfp_flt_parent(iter->flt),
		      "PSFP: update cnt ref eff filter %d cur filter %d\n",
		      to_flt_entry(eff)->filter,
		      update_cur ? to_flt_entry(cur)->filter : -1);

	edgx_psfp_flt_str_get_cnt_hw(flt, str_hdl, pcp, &cnt_cur);
	if (update_eff)
		edgx_psfp_flt_update_cnt_ref(to_flt_entry(eff), &cnt_cur);
	if (update_cur)
		edgx_psfp_flt_update_cnt_ref(to_flt_entry(cur), &cnt_cur);
}

static int edgx_psfp_flt_upd_hw_iter(struct psfp_flt_iter *iter,
				     const struct psfp_flt_sub_entry *eff,
				     s32 str_hdl,
				     s32 pcp)
{
	if (eff)
		_edgx_psfp_flt_write_hw(iter->flt, str_hdl, pcp,
					&to_flt_entry(eff)->data);
	else
		_edgx_psfp_flt_write_hw(iter->flt, str_hdl, pcp, NULL);
	if (iter->trigger)
		edgx_psfp_flt_upd_cnt(iter, str_hdl, pcp, eff);
	iter->flt->cur_sub[str_hdl][pcp] = eff;
	return 0;
}

static void edgx_psfp_flt_upd_hw(struct psfp_flt *flt,
				 const struct psfp_flt_sub_entry *sube)
{
	struct psfp_flt_iter iter = {
		.str_hdl = sube ? sube->str_entry->str_hdl : PSFP_WC_STR_HDL,
		.pcp = sube ? sube->str_entry->pcp : PSFP_WC_PCP,
	};

	edgx_psfp_flt_for_each(flt, sube, edgx_psfp_flt_upd_hw_iter, &iter);
}

static int edgx_psfp_flt_get_cnt(struct psfp_flt *flt,
				 u32 filter,
				 enum psfp_cnt_type cnt_type,
				 u32 *cnt)
{
	struct psfp_flt_cnt cnt_hw;
	struct psfp_flt_entry *flte;
	struct psfp_flt_sub_entry *sube;
	struct psfp_flt_str_entry *stre;

	flte = edgx_psfp_flt_search(flt, filter);
	if (!flte)
		return -ENOENT;

	sube = &flte->sub_entry;
	stre = sube->str_entry;

	/* NOTE: FLEXDE-5096: Standard Deviation:
	 * Wildcard filter counters are not supported.
	 * Return zero.
	 */
	if (edgx_psfp_flt_str_is_wc(stre)) {
		*cnt = 0;
		return 0;
	}

	if (flt->cur_sub[stre->str_hdl][stre->pcp] != sube) {
		*cnt = edgx_psfp_flt_get_user_cnt(&flte->cnt_ref, NULL,
						  cnt_type);
		return 0;
	}

	edgx_psfp_flt_str_get_cnt_hw(flt, stre->str_hdl, stre->pcp, &cnt_hw);
	*cnt = edgx_psfp_flt_get_user_cnt(&cnt_hw, &flte->cnt_ref, cnt_type);
	return 0;
}

static int edgx_psfp_flt_init(struct psfp_flt *flt, edgx_io_t *iobase,
			      const struct psfp_limits *limits)
{
	edgx_psfp_dbg(edgx_psfp_flt_parent(flt), "PSFP: start init\n");
	flt->cur_sub = kcalloc(limits->max_str_cnt, sizeof(*flt->cur_sub),
			       GFP_KERNEL);
	if (!flt->cur_sub)
		return -ENOMEM;
	mutex_init(&flt->lock);
	flt->iobase = iobase;
	flt->limits = limits;
	flt->flt_root = RB_ROOT;
	flt->str_root = RB_ROOT;
	edgx_psfp_flt_upd_hw(flt, NULL);
	edgx_psfp_dbg(edgx_psfp_flt_parent(flt), "PSFP: init ready\n");
	return 0;
}

static void edgx_psfp_flt_shutdown(struct psfp_flt *flt)
{
	struct psfp_flt_entry *flte;
	struct psfp_flt_str_entry *stre;
	struct psfp_flt_sub_entry *sube;
	struct rb_node *next, *next_sub;

	next = rb_first(&flt->str_root);
	while (next) {
		stre = rb_entry(next, struct psfp_flt_str_entry, rbnode);
		next = rb_next(next);
		_psfp_rb_erase(&stre->rbnode, &flt->str_root);
		next_sub = rb_first(&stre->sub_root);
		while (next_sub) {
			sube = rb_entry(next_sub, struct psfp_flt_sub_entry,
					rbnode);
			next_sub = rb_next(next_sub);
			_psfp_rb_erase(&sube->rbnode, &stre->sub_root);
		}
		kfree(stre);
	}

	next = rb_first(&flt->flt_root);
	while (next) {
		flte = rb_entry(next, struct psfp_flt_entry, rbnode);
		next = rb_next(next);
		_psfp_rb_erase(&flte->rbnode, &flt->flt_root);
		kfree(flte);
	}

	kfree(flt->cur_sub);
}

/* PSFP Hit Counters */

static int edgx_psfp_hit_set_gate_hw(struct psfp_hit *hit, unsigned int gate)
{
	mutex_lock(&hit->lock);
	edgx_wr16(hit->iobase, HIT_SGATE, gate);
	mutex_unlock(&hit->lock);
	return 0;
}

static unsigned int edgx_psfp_hit_get_gate_hw(struct psfp_hit *hit)
{
	unsigned int gate;

	mutex_lock(&hit->lock);
	gate = edgx_rd16(hit->iobase, HIT_SGATE) & HIT_SGATE_MASK;
	mutex_unlock(&hit->lock);
	return gate;
}

static void edgx_psfp_hit_get_th_hw(struct psfp_hit *hit,
				    unsigned int *low_ns, unsigned int *high_ns)
{
	u16 low  = edgx_rd16(hit->iobase, HIT_TH_L) & HIT_TH_MASK;
	u16 high = edgx_rd16(hit->iobase, HIT_TH_H) & HIT_TH_MASK;

	*low_ns = low * HIT_TH_GRAN_NS;
	*high_ns = high * HIT_TH_GRAN_NS;
}

static int edgx_psfp_hit_check(struct psfp_hit *hit,
			       unsigned int low_ns, unsigned int high_ns,
			       u16 *low, u16 *high)
{
	if (low_ns < HIT_TH_L_MIN * HIT_TH_GRAN_NS ||
	    low_ns > HIT_TH_L_MAX * HIT_TH_GRAN_NS) {
		edgx_br_warn(hit->parent,
			     "PSFP: Hit counter low threshold %u ns "
			     "is not a within %u - %u ns\n",
			     low_ns,
			     HIT_TH_L_MIN * HIT_TH_GRAN_NS,
			     HIT_TH_L_MAX * HIT_TH_GRAN_NS);
		return -ERANGE;
	}

	*low = low_ns / HIT_TH_GRAN_NS;
	if (*low * HIT_TH_GRAN_NS != low_ns) {
		edgx_br_warn(hit->parent,
			     "PSFP: Hit counter low threshold %u ns "
			     "is not a multiple of %u ns\n",
			     low_ns, HIT_TH_GRAN_NS);
		return -EINVAL;
	}

	if (high_ns < HIT_TH_H_MIN * HIT_TH_GRAN_NS ||
	    high_ns > HIT_TH_H_MAX * HIT_TH_GRAN_NS) {
		edgx_br_warn(hit->parent,
			     "PSFP: Hit counter high threshold %u ns "
			     "is not a within %u - %u ns\n",
			     high_ns,
			     HIT_TH_H_MIN * HIT_TH_GRAN_NS,
			     HIT_TH_H_MAX * HIT_TH_GRAN_NS);
		return -ERANGE;
	}

	*high = high_ns / HIT_TH_GRAN_NS;
	if (*high * HIT_TH_GRAN_NS != high_ns) {
		edgx_br_warn(hit->parent,
			     "PSFP: Hit counter high threshold %u ns "
			     "is not a multiple of %u ns\n",
			     high_ns, HIT_TH_GRAN_NS);
		return -EINVAL;
	}

	if (*low >= *high) {
		edgx_br_warn(hit->parent,
			     "PSFP: Hit counter low threshold %u ns "
			     "is not smaller than high threshold %u ns\n",
			     low_ns, high_ns);
		return -EINVAL;
	}

	return 0;
}

static int edgx_psfp_hit_set_th_hw(struct psfp_hit *hit,
				   unsigned int low_ns, unsigned int high_ns)
{
	u16 low, high;
	int ret = edgx_psfp_hit_check(hit, low_ns, high_ns, &low, &high);

	if (ret)
		return ret;

	/* Ensure HIT_L < HIT_H always on HW */
	if (low_ns >= hit->th_high_ns) {
		/* Must set HIT_H first */
		hit->th_high_ns = high_ns;
		edgx_psfp_dbg(hit->parent,
			      "PSFP: Set HIT_H to %u (%u ns -> %u ns)\n",
			      high, hit->th_high_ns, high * HIT_TH_GRAN_NS);
		edgx_wr16(hit->iobase, HIT_TH_H, high);
	}
	if (low_ns != hit->th_low_ns) {
		hit->th_low_ns = low_ns;
		edgx_psfp_dbg(hit->parent,
			      "PSFP: Set HIT_L to %u (%u ns -> %u ns)\n",
			      low, hit->th_low_ns, low * HIT_TH_GRAN_NS);
		edgx_wr16(hit->iobase, HIT_TH_L, low);
	}
	if (high_ns != hit->th_high_ns) {
		hit->th_high_ns = high_ns;
		edgx_psfp_dbg(hit->parent,
			      "PSFP: Set HIT_H to %u (%u ns -> %u ns)\n",
			      high, hit->th_high_ns, high * HIT_TH_GRAN_NS);
		edgx_wr16(hit->iobase, HIT_TH_H, high);
	}
	return 0;
}

static u32 edgx_psfp_hit_get_cnt_hw(struct psfp_hit *hit,
				    u16 port, u16 str_hdl,
				    enum psfp_hit_cnt_type cnt_type)
{
	u16 capture = FIELD_PREP(CNT_CAPT4_PORT, port) |
		      FIELD_PREP(CNT_CAPT4_STREAM, str_hdl);
	u32 cnt = 0;

	edgx_wr16(hit->iobase, CNT_CAPT4, capture);
	do {
		capture = edgx_get16(hit->iobase, CNT_CAPT0, 6, 6);
	} while (capture);

	switch (cnt_type) {
	case PSFP_CNT_HIT_A:
		cnt = edgx_rd32(hit->iobase, CNT_HIT_A);
		break;
	case PSFP_CNT_HIT_B:
		cnt = edgx_rd32(hit->iobase, CNT_HIT_B);
		break;
	}
	return cnt;
}

static int edgx_psfp_hit_init(struct psfp_hit **phit,
			      struct edgx_br *parent,
			      edgx_io_t *iobase)
{
	int ret;

	*phit = kzalloc(sizeof(*phit), GFP_KERNEL);
	if (!*phit)
		return -ENOMEM;

	(*phit)->parent = parent;
	(*phit)->iobase = iobase;
	edgx_psfp_hit_get_th_hw(*phit,
				&(*phit)->th_low_ns,
				&(*phit)->th_high_ns);
	ret = edgx_psfp_hit_set_th_hw(*phit,
				      HIT_TH_L_MIN * HIT_TH_GRAN_NS,
				      HIT_TH_H_MIN * HIT_TH_GRAN_NS);
	if (ret) {
		kfree(*phit);
		*phit = NULL;
	}
	return ret;
}

static void edgx_psfp_hit_shutdown(struct psfp_hit *hit)
{
	if (hit)
		kfree(hit);
}

/* PSFP Stream Gates */

static inline void edgx_psfp_gt_wait_cmd(struct psfp_gates *gates)
{
	u16 cmd;

	usleep_range(300, 400);
	do {
		cmd = edgx_get16(gates->iobase, SGATE_CMD, 15, 15);
	} while (cmd);
}

static void edgx_psfp_gt_read_sgate_hw(struct psfp_gates *gates,
				       u16 gate,
				       struct psfp_gt_sgate_entry *entry)
{
	u16 cmd    = FIELD_PREP(SGATE_ROW, gate) |
		     SGATE_READ |
		     SGATE_TRANSFER;
	u16 table0 = 0;

	edgx_wr16(gates->iobase, SGATE_CMD, cmd);
	edgx_psfp_gt_wait_cmd(gates);
	table0 = edgx_rd16(gates->iobase, SGATE_TBL_0);
	edgx_psfp_dbg(gates->parent,
		      "PSFP: Read sgate %d cmd 0x%04x table0 0x%04x\n",
		      gate, cmd, table0);
	entry->closed_inv_rx_ena = table0 & SGATE_CLOSED_INV_RX_ENA;
	entry->closed_inv_rx	 = table0 & SGATE_CLOSED_INV_RX;
}

static void edgx_psfp_gt_write_sgate_hw(struct psfp_gates *gates,
					u16 gate,
					const struct psfp_gt_sgate_entry *entry)
{
	u16 cmd    = FIELD_PREP(SGATE_ROW, gate) |
		     SGATE_WRITE |
		     SGATE_TRANSFER;
	u16 table0 = FIELD_PREP(SGATE_CLOSED_INV_RX_ENA,
				entry->closed_inv_rx_ena) |
		     FIELD_PREP(SGATE_CLOSED_INV_RX,
				entry->closed_inv_rx);

	edgx_psfp_dbg(gates->parent,
		      "PSFP: Write sgate %d cmd 0x%04x table0 0x%04x\n",
		      gate, cmd, table0);
	edgx_wr16(gates->iobase, SGATE_TBL_0, table0);
	edgx_wr16(gates->iobase, SGATE_CMD, cmd);
	edgx_psfp_gt_wait_cmd(gates);
}

static inline struct edgx_sched *edgx_psfp_gt_get_sched(
		struct psfp_gates *gates, size_t idx)
{
	return gates->gt[idx].sched_evt.sched;
}

static inline void edgx_psfp_gt_get_sgate(struct psfp_gates *gates,
					  u16 idx,
					  struct psfp_gt_sgate_entry *entry)
{
	struct edgx_sched *sched = edgx_psfp_gt_get_sched(gates, idx);

	edgx_sched_lock(sched);
	edgx_psfp_gt_read_sgate_hw(gates, idx, entry);
	edgx_sched_unlock(sched);
}

static inline void edgx_psfp_gt_set_sgate(struct psfp_gates *gates,
					  u16 idx,
					  struct psfp_gt_sgate_entry *entry,
					  unsigned int mask)
{
	struct edgx_sched *sched = edgx_psfp_gt_get_sched(gates, idx);
	struct psfp_gt_sgate_entry new_entry;

	edgx_sched_lock(sched);
	edgx_psfp_gt_read_sgate_hw(gates, idx, &new_entry);
	if (mask & PSFP_SGATE_INV_RX_ENA)
		new_entry.closed_inv_rx_ena = entry->closed_inv_rx_ena;
	if (mask & PSFP_SGATE_INV_RX)
		new_entry.closed_inv_rx = entry->closed_inv_rx;
	edgx_psfp_gt_write_sgate_hw(gates, idx, &new_entry);
	edgx_sched_unlock(sched);
}

static inline struct psfp_gt *edgx_sched_event2gt(struct edgx_sched_event *evt)
{
	return container_of(evt, struct psfp_gt, sched_evt);
}

static inline struct psfp_gates *edgx_psfp_gt_get_gates(struct psfp_gt *gt)
{
	return container_of(gt, struct psfp_gates,
			    gt[edgx_sched_get_idx(gt->sched_evt.sched)]);
}

static inline u8 edgx_psfp_gt_make_states(bool gate_state,
					  bool ipv_enable,
					  u8 ipv)
{
	return FIELD_PREP(PSFP_GT_GATE_MASK,	gate_state) |
	       FIELD_PREP(PSFP_GT_IPV_ENA_MASK,	ipv_enable) |
	       FIELD_PREP(PSFP_GT_IPV_MASK,	ipv);
}

static void edgx_psfp_gt_dump_dly_pt(const struct psfp_gt *gt,
				     const struct psfp_gt_dly_pt *dly_pt)
{
	struct edgx_pt *pt = dly_pt->parent;
	struct edgx_br *br = edgx_pt_get_br(pt);
	unsigned int sched_idx = edgx_sched_get_idx(gt->sched_evt.sched);

	edgx_psfp_dbg(br, "PSFP: gate %u delays port %s:\n",
		      sched_idx, edgx_pt_get_name(pt));
	edgx_psfp_dbg(br, "PSFP:     link rx %lld .. %lld ns speed %d %s\n",
		      ktime_to_ns(dly_pt->lnk_rx_min),
		      ktime_to_ns(dly_pt->lnk_rx_max),
		      dly_pt->speed,
		      dly_pt->speed == SPEED_UNKNOWN ?
		      "(unknown)" : "");
	edgx_psfp_dbg(br, "PSFP:     i2sg %lld .. %lld ns\n",
		      ktime_to_ns(dly_pt->i2sg_min),
		      ktime_to_ns(dly_pt->i2sg_max));
	edgx_psfp_dbg(br, "PSFP:     w2sg %lld .. %lld ns -> %lld ns\n",
		      dly_pt->w2sg_min, dly_pt->w2sg_max, dly_pt->w2sg);
	edgx_psfp_dbg(br, "PSFP:     sgate_delay %lld ns\n",
		      dly_pt->sgate_delay);
}

static void edgx_psfp_gt_dump_dly_sched(struct psfp_gt *gt)
{
	unsigned int sched_idx = edgx_sched_get_idx(gt->sched_evt.sched);
	const struct psfp_gt_dly_sched *dly_sch = &gt->dly_sched;
	struct psfp_gates *gates = edgx_psfp_gt_get_gates(gt);
	struct edgx_br *br = gates->parent;

	edgx_psfp_dbg(br, "PSFP: gate %u delays:\n", sched_idx);
	edgx_psfp_dbg(br, "PSFP:     clk %llu\n", dly_sch->clk);
	edgx_psfp_dbg(br, "PSFP:     s2g %lld .. %lld ns -> %lld ns\n",
		      dly_sch->s2g_min, dly_sch->s2g_max, dly_sch->s2g);
	edgx_psfp_dbg(br, "PSFP:     s2sg %lld ns\n", dly_sch->s2sg);
	if (dly_sch->retard < S64_MAX)
		edgx_psfp_dbg(br, "PSFP:     retard %lld ns\n",
			      dly_sch->retard);
}

static s64 edgx_psfp_gt_calc_retard(struct psfp_gt_dly_sched *dly_sch,
				    struct psfp_gt_dly_pt *dly_pt,
				    struct psfp_gt_dly_pt *dly_pt_ref)
{
	s64 retard;

	dly_pt->w2sg_min = ktime_to_ns(ktime_add(dly_pt->lnk_rx_min,
						 dly_pt->i2sg_min));
	dly_pt->w2sg_max = ktime_to_ns(ktime_add(dly_pt->lnk_rx_max,
						 dly_pt->i2sg_max));
	dly_pt->w2sg = (dly_pt->w2sg_min + dly_pt->w2sg_max + 1) / 2;
	if (dly_pt_ref)
		dly_pt->sgate_delay = (s64)dly_pt->w2sg - (s64)dly_sch->s2g
				      - ((s64)dly_pt_ref->w2sg -
					 (s64)dly_sch->s2g -
					 dly_pt_ref->sgate_delay);
	else
		dly_pt->sgate_delay = 0;
	retard = (s64)dly_pt->w2sg - (s64)dly_sch->s2sg -
		 (s64)dly_pt->sgate_delay;
	return retard;
}

static int edgx_psfp_gt_calc_delays(struct psfp_gt *gt)
{
	struct edgx_sched *sched = gt->sched_evt.sched;
	struct psfp_gates *gates = edgx_psfp_gt_get_gates(gt);
	unsigned int sched_idx = edgx_sched_get_idx(gt->sched_evt.sched);
	struct psfp_gt_dly_sched *dly_sch = &gt->dly_sched;
	struct psfp_gt_dly_pt *dly_pt_ref = &gt->dly_pt_ref;
	struct psfp_gt_dly_pt *dly_pt;
	struct edgx_pt *pt;
	struct edgx_link *lnk;
	ptid_t ptid;
	u64 s2sg_sched;
	s64 retard;
	int speed;
	int ret;

	edgx_psfp_dbg(gates->parent, "PSFP: gate: calc delays for sched %u\n",
		      sched_idx);

	dly_pt_ref->speed = SPEED_UNKNOWN;
	dly_sch->retard = S64_MAX;
	dly_sch->clk = edgx_br_get_cycle_ns(edgx_sched_get_br(sched));
	if (!dly_sch->clk)
		return -EINVAL;

	/* Scheduler index affects stream to stream gate delay and retard,
	 * but not sgate_delay (gets cancelled)
	 */
	s2sg_sched = sched_idx * dly_sch->clk;

	ret = edgx_sched_get_s2g(sched, dly_sch->clk,
				 &dly_sch->s2g_min, &dly_sch->s2g_max);
	if (ret)
		return ret;
	dly_sch->s2g = (dly_sch->s2g_min + dly_sch->s2g_max + 1)/2;
	dly_sch->s2sg = dly_sch->s2g + s2sg_sched;

	edgx_psfp_dbg(gates->parent,
		      "PSFP: Find reference and determine retard\n");
	edgx_psfp_gt_dump_dly_sched(gt);
	for (ptid = 0; ptid < EDGX_BR_MAX_PORTS; ptid++) {
		dly_pt = &gt->dly_pt[ptid];
		pt = dly_pt->parent;
		if (!pt)
			continue;

		edgx_psfp_dbg(gates->parent, "PSFP: port %u\n", ptid);
		dly_pt->speed = SPEED_UNKNOWN;
		lnk = edgx_pt_get_link(pt);
		speed = edgx_link_get_speed(lnk);
		if (speed == SPEED_UNKNOWN)
			continue;

		ret = edgx_link_get_rx_delays_hi(lnk, &dly_pt->lnk_rx_min,
						 &dly_pt->lnk_rx_max);
		if (ret)
			continue;
		ret = edgx_pt_get_i2sg_hi(pt, &dly_pt->i2sg_min,
					  &dly_pt->i2sg_max);
		if (ret)
			continue;

		dly_pt->speed = speed;
		retard = edgx_psfp_gt_calc_retard(dly_sch, dly_pt, NULL);
		edgx_psfp_gt_dump_dly_pt(gt, dly_pt);
		edgx_psfp_dbg(gates->parent, "PSFP:     retard %lld ns\n",
			      retard);
		if (retard < dly_sch->retard) {
			*dly_pt_ref = *dly_pt;
			dly_sch->retard = retard;
			edgx_psfp_dbg(gates->parent, "PSFP:     ^ new ref\n");
		}
	}

	if (dly_sch->retard == S64_MAX) {
		edgx_br_warn(gates->parent,
			     "PSFP: Failed to determine schedule retard\n");
		return -EINVAL;
	}
	edgx_psfp_dbg(gates->parent, "PSFP: Gate %u retard %lld ns\n",
		      sched_idx, dly_sch->retard);

	edgx_psfp_dbg(gates->parent, "PSFP: Calc stream delays for ports\n");
	for (ptid = 0; ptid < EDGX_BR_MAX_PORTS; ptid++) {
		dly_pt = &gt->dly_pt[ptid];
		pt = dly_pt->parent;
		if (!pt)
			continue;

		edgx_psfp_dbg(gates->parent, "PSFP: port %u\n", ptid);
		if (dly_pt->speed == SPEED_UNKNOWN)
			continue;

		lnk = edgx_pt_get_link(pt);
		dly_pt->lnk_rx_min = edgx_link_get_rx_delay_min(lnk);
		dly_pt->lnk_rx_max = edgx_link_get_rx_delay_max(lnk);
		dly_pt->i2sg_min = edgx_pt_get_i2sgmin(pt);
		dly_pt->i2sg_max = edgx_pt_get_i2sgmax(pt);
		retard = edgx_psfp_gt_calc_retard(dly_sch, dly_pt, dly_pt_ref);
		edgx_psfp_gt_dump_dly_pt(gt, dly_pt);
		edgx_psfp_dbg(gates->parent, "PSFP:     retard %lld ns\n",
			      retard);
		/* sgate_delay is positive but check */
		if (dly_pt->sgate_delay >= 0)
			edgx_pt_set_sg_delay(pt,
					     ktime_set(0, dly_pt->sgate_delay));
		else
			edgx_pt_set_sg_delay(pt, ktime_set(0, 0));
		if (retard != dly_sch->retard)
			edgx_pt_warn(pt, "PSFP: gate: retard does not match\n");
	}

	edgx_psfp_dbg(gates->parent, "PSFP: gate: done delays for sched %u\n",
		      sched_idx);
	return 0;
}

static int edgx_psfp_gt_get_sched_offset(struct edgx_sched_event *sched_evt,
					 s64 *offset_ns)
{
	struct psfp_gt *gt = edgx_sched_event2gt(sched_evt);
	int ret;

	ret = edgx_psfp_gt_calc_delays(gt);
	if (ret)
		return ret;

	*offset_ns = gt->dly_sched.retard;
	return 0;
}

static int edgx_psfp_gt_prepare_config(
		struct edgx_sched_event *sched_evt,
		const struct edgx_sched_tab_entry *entries,
		size_t count)
{
	struct edgx_sched *sched = sched_evt->sched;
	size_t i;
	u32 time_from_cyc_start = 0;
	int tab_idx = edgx_sched_get_free_tab(sched);
	int ret = 0;

	if (tab_idx < 0)
		return tab_idx;

	for (i = 0; !ret && (i < count); i++) {
		time_from_cyc_start += entries[i].time_interval;


		ret = edgx_sched_write_entry(sched,
					     tab_idx, i,
					     entries[i].gate_states,
					     time_from_cyc_start);
	}

	/* Add the last interval entry again. FSC will stop on this entry */
	if (!ret)
		ret = edgx_sched_write_entry(sched,
					     tab_idx, i,
					     entries[i - 1].gate_states,
					     time_from_cyc_start);

	return ret;
}

static void edgx_psfp_gt_config_changed(struct edgx_sched_event *sched_evt)
{
	/* No-op */
}

static struct edgx_sched_ops edgx_psfp_gt_ops = {
	.get_sched_offset	= edgx_psfp_gt_get_sched_offset,
	.prepare_config		= edgx_psfp_gt_prepare_config,
	.config_changed		= edgx_psfp_gt_config_changed,
};

static int edgx_psfp_gt_init(struct edgx_br *br, struct edgx_br_irq *irq,
			     const char *drv_name,
			     const struct edgx_ifdesc *ifd_gating,
			     const struct edgx_ifdesc *ifd_sgate,
			     struct psfp_limits *limits,
			     struct psfp_gates *gates)
{
	u16 init_gate_states = edgx_psfp_gt_make_states(true, false, 0);
	const struct psfp_gt_sgate_entry init_sgate = {
		.closed_inv_rx_ena = false,
		.closed_inv_rx = false,
	};
	struct psfp_gt		*gt;
	struct edgx_sched	*sched;
	struct edgx_ifdesc	 ptifd;
	ptid_t			 ptid;
	unsigned int i;
	int ret;

	gates->parent = br;
	gates->iobase = ifd_sgate->iobase;
	gates->limits = limits;

	ret = edgx_sched_com_probe(br, irq, EDGX_IRQ_NR_PSFP_SCHED_TAB,
				   drv_name, ifd_gating,
				   &gates->sched_com,
				   &edgx_psfp_gt_ops,
				   init_gate_states);
	if (ret)
		goto out_sched_com;
	/* Use always 320 ns scheduler granularity with Qci */
	edgx_sched_com_set_params(gates->sched_com, 0, PSFP_GT_SCHED_GRAN_NS);

	for (i = 0; i < gates->limits->max_gate_cnt; i++) {
		gt = &gates->gt[i];
		ret = edgx_probe_sched(gates->sched_com, i, &gt->sched_evt);
		if (ret)
			goto out_sched;
		edgx_ac_for_each_ifpt(ptid, ifd_gating, &ptifd) {
			struct edgx_pt *pt = edgx_br_get_brpt(br, ptid);

			if (pt)
				gt->dly_pt[ptid].parent = pt;
		}

		edgx_psfp_gt_write_sgate_hw(gates, i, &init_sgate);
	}

	sched = edgx_psfp_gt_get_sched(gates, 0);
	limits->max_gcl_len = edgx_sched_get_limits(sched)->max_entry_cnt;

	return ret;

out_sched:
	do {
		edgx_shutdown_sched(gates->gt[i].sched_evt.sched);
	} while (--i > 0);
out_sched_com:
	return ret;
}

static void edgx_psfp_gt_shutdown(struct psfp_gates *gates)
{
	struct edgx_sched *sched;
	unsigned int i;

	i = gates->limits->max_gate_cnt;
	while (i-- > 0) {
		sched = edgx_psfp_gt_get_sched(gates, i);
		if (!sched)
			continue;
		edgx_shutdown_sched(sched);
	}

	edgx_sched_com_shutdown(gates->sched_com);
}

/* User Interface: PSFP Stream Filter Table */

static inline struct edgx_psfp *edgx_dev2psfp(struct device *dev)
{
	return edgx_br_get_psfp(edgx_dev2br(dev));
}

static inline struct psfp_flt *edgx_dev2psfp_flt(struct device *dev)
{
	return &edgx_dev2psfp(dev)->flt;
}

static ssize_t flt_tbl_read(struct file *filp, struct kobject *kobj,
			    struct bin_attribute *bin_attr,
			    char *buf, loff_t ofs, size_t count)
{
	struct edgx_br *br = edgx_dev2br(kobj_to_dev(kobj));
	struct psfp_flt *flt = edgx_dev2psfp_flt(kobj_to_dev(kobj));
	struct psfp_flt_sysfs_entry *flt_def = NULL;
	struct psfp_flt_entry *flte = NULL;
	struct psfp_flt_str_entry *stre = NULL;
	struct psfp_flt_sub_entry *sube = NULL;
	struct psfp_flt_data fltd;
	loff_t idx = 0;
	int ret = -ENOENT;

	if (edgx_sysfs_tbl_params(ofs, count, sizeof(*flt_def), &idx) ||
	    idx >= PSFP_MAX_NO_FILTERS)
		return -EINVAL;

	edgx_psfp_dbg(br, "PSFP: read flt %lld\n", idx);
	mutex_lock(&flt->lock);

	flte = edgx_psfp_flt_search(flt, idx);
	if (!flte)
		goto out;
	sube = &flte->sub_entry;
	stre = sube->str_entry;

	flt_def = (struct psfp_flt_sysfs_entry *)buf;
	flt_def->str_hdl = stre->str_hdl;
	flt_def->pcp = stre->pcp;
	flt_def->data = flte->data;
	/* TODO: Handle wildcard also */
	if (!edgx_psfp_flt_str_is_wc(stre)) {
		_edgx_psfp_flt_read_hw(flt, stre->str_hdl, stre->pcp, &fltd);
		flt_def->data.blk_oversz = fltd.blk_oversz;
	} else {
		flt_def->data.blk_oversz = flte->data.blk_oversz;
	}
	ret = 0;

out:
	mutex_unlock(&flt->lock);

	return ret ? ret : count;
}

static ssize_t _edgx_psfp_flt_add(struct psfp_flt *flt, u32 filter,
				  const struct psfp_flt_sysfs_entry *flt_def)
{
	struct psfp_flt_entry *flte = NULL;
	struct psfp_flt_str_entry *stre = NULL;
	struct psfp_flt_str_entry *stre_new = NULL;
	struct psfp_flt_sub_entry *sube = NULL;
	int ret = -ENOMEM;

	flte = edgx_psfp_flt_create(filter);
	if (!flte)
		goto out;
	flte->data = flt_def->data;

	stre = edgx_psfp_flt_str_search(flt, flt_def->str_hdl, flt_def->pcp);
	edgx_psfp_dbg(edgx_psfp_flt_parent(flt),
		      "PSFP: ^ %s\n", stre ? "str already exists" : "new str");
	if (!stre) {
		stre_new = edgx_psfp_flt_str_create(flt_def->str_hdl,
						    flt_def->pcp);
		if (!stre_new)
			goto out_str_create;
		stre = stre_new;
	}

	sube = &flte->sub_entry;
	sube->str_entry = stre;

	edgx_psfp_dbg(edgx_psfp_flt_parent(flt), "PSFP: ^ insert sub\n");
	ret = edgx_psfp_flt_sub_insert(stre, sube);
	if (ret) {
		edgx_psfp_dbg(edgx_psfp_flt_parent(flt),
			      "PSFP: ^ insert sub failed\n");
		WARN_ON(ret);
		goto out_sub_insert;
	}
	if (stre_new) {
		edgx_psfp_dbg(edgx_psfp_flt_parent(flt),
			      "PSFP: ^ insert str\n");
		ret = edgx_psfp_flt_str_insert(flt, stre);
		if (ret) {
			edgx_psfp_dbg(edgx_psfp_flt_parent(flt),
				      "PSFP: ^ insert str failed\n");
			WARN_ON(ret);
			goto out_str_insert;
		}
	}
	edgx_psfp_dbg(edgx_psfp_flt_parent(flt), "PSFP: ^ insert flt\n");
	ret = edgx_psfp_flt_insert(flt, flte);
	if (ret) {
		edgx_psfp_dbg(edgx_psfp_flt_parent(flt),
			      "PSFP: ^ insert flt failed\n");
		WARN_ON(ret);
		goto out_flt_insert;
	}

	edgx_psfp_dbg(edgx_psfp_flt_parent(flt),
		      "PSFP: Add filter %u strhdl %d pcp %d gate %u "
		      "max_sdu %u fmtr %d\n",
		      flte->filter, stre->str_hdl, stre->pcp, flte->data.gate,
		      flte->data.fsl.max_sdu, flte->data.fsl.fmtr);

	if (rb_first(&stre->sub_root) == &sube->rbnode)
		edgx_psfp_flt_upd_hw(flt, sube);

	return 0;

out_flt_insert:
	if (stre_new)
		_psfp_rb_erase(&stre->rbnode, &flt->str_root);
out_str_insert:
	_psfp_rb_erase(&sube->rbnode, &stre->sub_root);
out_sub_insert:
	if (stre_new)
		kfree(stre_new);
out_str_create:
	kfree(flte);
out:
	return ret;
}

static ssize_t flt_tbl_write(struct file *filp, struct kobject *kobj,
			     struct bin_attribute *bin_attr,
			     char *buf, loff_t ofs, size_t count)
{
	struct edgx_br *br = edgx_dev2br(kobj_to_dev(kobj));
	struct psfp_flt *flt = edgx_dev2psfp_flt(kobj_to_dev(kobj));
	struct psfp_flt_sysfs_entry *flt_def = NULL;
	struct psfp_flt_entry *flte = NULL;
	loff_t idx = 0;
	int ret = -EEXIST;

	if (edgx_sysfs_tbl_params(ofs, count, sizeof(*flt_def), &idx) ||
	    idx >= PSFP_MAX_NO_FILTERS)
		return -EINVAL;

	flt_def = (struct psfp_flt_sysfs_entry *)buf;
	edgx_psfp_dbg(br,
		      "PSFP: Write flt %lld str_hdl %d (0x%x) pcp %d (0x%x) "
		      "gate %u (0x%x) max_sdu %u (0x%x) fmtr %d (0x%x)\n",
		      idx,
		      flt_def->str_hdl, flt_def->str_hdl,
		      flt_def->pcp, flt_def->pcp,
		      flt_def->data.gate, flt_def->data.gate,
		      flt_def->data.fsl.max_sdu, flt_def->data.fsl.max_sdu,
		      flt_def->data.fsl.fmtr, flt_def->data.fsl.fmtr);

	if (edgx_psfp_flt_check(flt, flt_def))
		return -EINVAL;

	edgx_psfp_dbg(br, "PSFP: ^ sanity check passed, adding\n");
	mutex_lock(&flt->lock);

	flte = edgx_psfp_flt_search(flt, idx);
	edgx_psfp_dbg(br, "PSFP: ^ %s\n", flte ? "already exists" : "new");
	if (flte)
		goto out;
	ret = _edgx_psfp_flt_add(flt, idx, flt_def);

out:
	mutex_unlock(&flt->lock);
	return ret ? ret : count;
}

static ssize_t flt_del_store(struct device *dev,
			     struct device_attribute *attr,
			     const char *buf, size_t count)
{
	struct edgx_br *br = edgx_dev2br(dev);
	struct psfp_flt *flt = edgx_dev2psfp_flt(dev);
	struct psfp_flt_entry *flte = NULL;
	struct psfp_flt_str_entry *stre = NULL;
	struct psfp_flt_sub_entry *sube = NULL;
	u32 filter = 0;
	bool was_first = false;
	int ret = -ENOENT;

	if (kstrtou32(buf, 10, &filter))
		return -EINVAL;

	edgx_psfp_dbg(br, "PSFP: Delete filter %u\n", filter);
	mutex_lock(&flt->lock);
	flte = edgx_psfp_flt_search(flt, filter);
	if (!flte)
		goto out;
	sube = &flte->sub_entry;
	WARN_ON(!sube);
	stre = sube->str_entry;
	WARN_ON(!stre);
	was_first = rb_first(&stre->sub_root) == &sube->rbnode;
	edgx_psfp_dbg(br, "PSFP: ^ erase %s sub\n",
		      was_first ? "active" : "inactive");
	_psfp_rb_erase(&sube->rbnode, &stre->sub_root);
	if (RB_EMPTY_ROOT(&stre->sub_root)) {
		edgx_psfp_dbg(br, "PSFP: ^ last sub, del str\n");
		_psfp_rb_erase(&stre->rbnode, &flt->str_root);
		if (was_first)
			edgx_psfp_flt_upd_hw(flt, sube);
		kfree(stre);
	} else {
		edgx_psfp_dbg(br, "PSFP: ^ not last sub\n");
		if (was_first)
			edgx_psfp_flt_upd_hw(flt, sube);
	}
	_psfp_rb_erase(&flte->rbnode, &flt->flt_root);
	kfree(flte);

	ret = 0;

out:
	mutex_unlock(&flt->lock);

	return ret ? ret : count;
}

static ssize_t flt_str_cnt_read(struct kobject *kobj,
				char *buf, loff_t ofs, size_t count,
				enum psfp_cnt_type cnt_type)
{
	struct edgx_br *br = edgx_dev2br(kobj_to_dev(kobj));
	struct psfp_flt *flt = edgx_dev2psfp_flt(kobj_to_dev(kobj));
	u32 *cnt = (u32 *)buf;
	loff_t idx = 0;
	int ret = -ENOENT;

	if (edgx_sysfs_tbl_params(ofs, count, sizeof(*cnt), &idx) ||
	    idx >= PSFP_MAX_NO_FILTERS)
		return -EINVAL;

	edgx_psfp_dbg(br, "PSFP: read filter %lld counter %d\n",
		      idx, cnt_type);

	if (idx >= flt->limits->max_flt_cnt)
		return -EINVAL;

	mutex_lock(&flt->lock);
	ret = edgx_psfp_flt_get_cnt(flt, idx, cnt_type, cnt);
	mutex_unlock(&flt->lock);

	return ret ? ret : count;
}

static ssize_t flt_cnt_matching_frames_read(struct file *filp,
					    struct kobject *kobj,
					    struct bin_attribute *bin_attr,
					    char *buf, loff_t ofs, size_t count)
{
	return flt_str_cnt_read(kobj, buf, ofs, count,
				PSFP_CNT_MATCHING_FRAMES);
}

static ssize_t flt_cnt_passing_frames_read(struct file *filp,
					   struct kobject *kobj,
					   struct bin_attribute *bin_attr,
					   char *buf, loff_t ofs, size_t count)
{
	return flt_str_cnt_read(kobj, buf, ofs, count,
				PSFP_CNT_PASSING_FRAMES);
}

static ssize_t flt_cnt_not_passing_frames_read(struct file *filp,
					       struct kobject *kobj,
					       struct bin_attribute *bin_attr,
					       char *buf, loff_t ofs, size_t count)
{
	return flt_str_cnt_read(kobj, buf, ofs, count,
				PSFP_CNT_NOT_PASSING_FRAMES);
}

static ssize_t flt_cnt_passing_sdu_read(struct file *filp,
					struct kobject *kobj,
					struct bin_attribute *bin_attr,
					char *buf, loff_t ofs, size_t count)
{
	return flt_str_cnt_read(kobj, buf, ofs, count,
				PSFP_CNT_PASSING_SDU);
}

static ssize_t flt_cnt_not_passing_sdu_read(struct file *filp,
					    struct kobject *kobj,
					    struct bin_attribute *bin_attr,
					    char *buf, loff_t ofs, size_t count)
{
	return flt_str_cnt_read(kobj, buf, ofs, count,
				PSFP_CNT_NOT_PASSING_SDU);
}

static ssize_t flt_cnt_red_frames_read(struct file *filp,
				       struct kobject *kobj,
				       struct bin_attribute *bin_attr,
				       char *buf, loff_t ofs, size_t count)
{
	return flt_str_cnt_read(kobj, buf, ofs, count,
				PSFP_CNT_RED_FRAMES);
}

/* User Interface: PSFP Hit Counters */

static inline struct psfp_hit *edgx_dev2psfp_hit(struct device *dev)
{
	return edgx_dev2psfp(dev)->hit_cnt;
}

static ssize_t gt_hit_gate_store(struct device *dev,
				 struct device_attribute *attr,
				 const char *buf, size_t count)
{
	struct edgx_psfp *psfp = edgx_dev2psfp(dev);
	unsigned int gate;
	int ret;

	if (!psfp->hit_cnt)
		return -EOPNOTSUPP;

	if (kstrtouint(buf, 0, &gate))
		return -EINVAL;

	if (gate >= psfp->limits.max_gate_cnt)
		return -EINVAL;

	ret = edgx_psfp_hit_set_gate_hw(psfp->hit_cnt, gate);
	return ret ? ret : count;
}

static ssize_t gt_hit_gate_show(struct device *dev,
				struct device_attribute *attr,
				char *buf)
{
	struct psfp_hit *hit = edgx_dev2psfp_hit(dev);
	unsigned int gate;

	if (!hit)
		return -EOPNOTSUPP;

	gate = edgx_psfp_hit_get_gate_hw(hit);
	return scnprintf(buf, PAGE_SIZE, "%u\n", gate);
}

static ssize_t gt_hit_th_store(struct device *dev,
			       struct device_attribute *attr,
			       const char *buf, size_t count,
			       enum psfp_hit_th_type th_type)
{
	struct psfp_hit *hit = edgx_dev2psfp_hit(dev);
	unsigned int th_ns;
	int ret = -EINVAL;

	if (!hit)
		return -EOPNOTSUPP;
	if (kstrtouint(buf, 0, &th_ns))
		return -EINVAL;

	mutex_lock(&hit->lock);
	if (th_type == PSFP_HIT_TH_LOW)
		ret = edgx_psfp_hit_set_th_hw(hit, th_ns, hit->th_high_ns);
	else
		ret = edgx_psfp_hit_set_th_hw(hit, hit->th_low_ns, th_ns);
	mutex_unlock(&hit->lock);

	return ret ? ret : count;
}

static ssize_t gt_hit_th_show(struct device *dev,
			      struct device_attribute *attr,
			      char *buf, enum psfp_hit_th_type th_type)
{
	struct psfp_hit *hit = edgx_dev2psfp_hit(dev);
	unsigned int th_ns;
	int ret = 0;

	if (!hit)
		return -EOPNOTSUPP;

	mutex_lock(&hit->lock);
	if (th_type == PSFP_HIT_TH_LOW)
		th_ns = hit->th_low_ns;
	else
		th_ns = hit->th_high_ns;
	ret = scnprintf(buf, PAGE_SIZE, "%u\n", th_ns);
	mutex_unlock(&hit->lock);

	return ret;
}

static ssize_t gt_hit_th_low_store(struct device *dev,
				   struct device_attribute *attr,
				   const char *buf, size_t count)
{
	return gt_hit_th_store(dev, attr, buf, count, PSFP_HIT_TH_LOW);
}

static ssize_t gt_hit_th_low_show(struct device *dev,
				  struct device_attribute *attr,
				  char *buf)
{
	return gt_hit_th_show(dev, attr, buf, PSFP_HIT_TH_LOW);
}

static ssize_t gt_hit_th_high_store(struct device *dev,
				    struct device_attribute *attr,
				    const char *buf, size_t count)
{
	return gt_hit_th_store(dev, attr, buf, count, PSFP_HIT_TH_HIGH);
}

static ssize_t gt_hit_th_high_show(struct device *dev,
				   struct device_attribute *attr,
				   char *buf)
{
	return gt_hit_th_show(dev, attr, buf, PSFP_HIT_TH_HIGH);
}

static ssize_t gt_hit_cnt_read(struct kobject *kobj,
			       char *buf, loff_t ofs, size_t count,
			       enum psfp_hit_cnt_type cnt_type)
{
	struct edgx_pt *pt = edgx_dev2pt(kobj_to_dev(kobj));
	struct edgx_psfp *psfp = edgx_br_get_psfp(edgx_pt_get_br(pt));
	struct psfp_hit *hit = psfp->hit_cnt;
	u32 *cnt = (u32 *)buf;
	loff_t idx = 0;

	if (!psfp->hit_cnt)
		return -EOPNOTSUPP;

	if (edgx_sysfs_tbl_params(ofs, count, sizeof(*cnt), &idx) ||
	    idx >= PSFP_MAX_NO_STREAMS)
		return -EINVAL;

	edgx_psfp_pt_dbg(pt, "PSFP: read stream %lld hit counter %d\n",
			 idx, cnt_type);

	if (idx >= psfp->limits.max_str_cnt)
		return -EINVAL;

	mutex_lock(&hit->lock);
	*cnt = edgx_psfp_hit_get_cnt_hw(hit, edgx_pt_get_id(pt), idx,
					cnt_type);
	mutex_unlock(&hit->lock);

	return count;
}

static ssize_t gt_hit_low_cnt_read(struct file *filp,
				   struct kobject *kobj,
				   struct bin_attribute *bin_attr,
				   char *buf, loff_t ofs, size_t count)
{
	return gt_hit_cnt_read(kobj, buf, ofs, count, PSFP_CNT_HIT_A);
}

static ssize_t gt_hit_high_cnt_read(struct file *filp,
				    struct kobject *kobj,
				    struct bin_attribute *bin_attr,
				    char *buf, loff_t ofs, size_t count)
{
	return gt_hit_cnt_read(kobj, buf, ofs, count, PSFP_CNT_HIT_B);
}

/* User Interface: PSFP Stream Gate Table */

static inline struct psfp_gates *edgx_dev2psfp_gates(struct device *dev)
{
	return &edgx_dev2psfp(dev)->gates;
}

static inline struct psfp_gates *edgx_dev2psfp_gate(struct device *dev,
						    loff_t ofs, size_t count,
						    size_t entry_size,
						    size_t *gate)
{
	struct psfp_gates *gates = edgx_dev2psfp_gates(dev);
	loff_t idx;
	int ret;

	ret = edgx_sysfs_tbl_params(ofs, count, entry_size, &idx);
	if (ret || idx >= gates->limits->max_gate_cnt)
		return NULL;
	*gate = idx;
	return gates;
}

static inline struct edgx_sched *edgx_dev2psfp_sched(struct device *dev,
						     loff_t ofs, size_t count,
						     size_t entry_size)
{
	size_t gate;
	struct psfp_gates *gates = edgx_dev2psfp_gate(dev, ofs, count,
						      entry_size, &gate);

	if (!gates)
		return NULL;
	return edgx_psfp_gt_get_sched(gates, gate);
}

static struct edgx_sched *edgx_dev2psfp_gcl_sched(struct device *dev,
						  loff_t ofs, size_t count,
						  size_t *first,
						  size_t *nelems)
{
	struct psfp_gates *gates = edgx_dev2psfp_gates(dev);
	loff_t idx;
	size_t sched_idx;
	size_t entries_per_sched;
	size_t entry_size = sizeof(struct psfp_gt_tab_entry);
	int ret;

	ret = edgx_sysfs_list_params(ofs, count, entry_size, &idx, nelems);
	if (ret)
		return NULL;
	entries_per_sched = gates->limits->max_gcl_len;
	sched_idx = (size_t)idx / entries_per_sched;
	if (sched_idx >= gates->limits->max_gate_cnt)
		return NULL;
	*first = (size_t)idx % entries_per_sched;

	return edgx_psfp_gt_get_sched(gates, sched_idx);
}

static ssize_t gt_gate_enabled_read(struct file *filp,
				    struct kobject *kobj,
				    struct bin_attribute *bin_attr,
				    char *buf, loff_t ofs, size_t count)
{
	struct edgx_sched *sched = edgx_dev2psfp_sched(kobj_to_dev(kobj),
						       ofs, count, 1);
	u8 *gate_enabled = (u8 *)buf;

	if (!sched)
		return -EINVAL;

	*gate_enabled = edgx_sched_get_gate_enabled(sched);
	return count;
}

static ssize_t gt_gate_enabled_write(struct file *filp,
				     struct kobject *kobj,
				     struct bin_attribute *bin_attr,
				     char *buf, loff_t ofs, size_t count)
{
	struct edgx_sched *sched = edgx_dev2psfp_sched(kobj_to_dev(kobj),
						       ofs, count, 1);
	int ret;

	if (!sched)
		return -EINVAL;

	ret = edgx_sched_set_gate_enabled(sched, *buf);
	return ret ? ret : count;
}

static ssize_t gt_admin_gate_states_read(struct file *filp,
					 struct kobject *kobj,
					 struct bin_attribute *bin_attr,
					 char *buf, loff_t ofs, size_t count)
{
	struct edgx_sched *sched = edgx_dev2psfp_sched(kobj_to_dev(kobj),
						       ofs, count, 1);
	u8 *gate_states = (u8 *)buf;

	if (!sched)
		return -EINVAL;

	*gate_states = edgx_sched_get_admin_gate_states(sched,
							PSFP_GT_GATE_MASK);
	return count;
}

static ssize_t gt_admin_gate_states_write(struct file *filp,
					  struct kobject *kobj,
					  struct bin_attribute *bin_attr,
					  char *buf, loff_t ofs, size_t count)
{
	struct edgx_sched *sched = edgx_dev2psfp_sched(kobj_to_dev(kobj),
						       ofs, count, 1);
	u8 gate_states;
	int ret;

	if (!sched)
		return -EINVAL;

	gate_states = *buf ? PSFP_GT_GATE_MASK : 0;
	ret = edgx_sched_set_admin_gate_states(sched, gate_states,
					       PSFP_GT_GATE_MASK);
	return ret ? ret : count;
}

static ssize_t gt_admin_ipv_read(struct file *filp,
				 struct kobject *kobj,
				 struct bin_attribute *bin_attr,
				 char *buf, loff_t ofs, size_t count)
{
	s8 *ipv = (s8 *)buf;
	struct edgx_sched *sched = edgx_dev2psfp_sched(kobj_to_dev(kobj),
						       ofs, count,
						       sizeof(*ipv));
	u8 gate_states;

	if (!sched)
		return -EINVAL;

	gate_states = edgx_sched_get_admin_gate_states(sched,
						       PSFP_GT_IPV_MASK |
						       PSFP_GT_IPV_ENA_MASK);
	if (gate_states & PSFP_GT_IPV_ENA_MASK)
		*ipv = FIELD_GET(PSFP_GT_IPV_MASK, gate_states);
	else
		*ipv = PSFP_GT_IPV_NULL;
	return count;
}

static ssize_t gt_admin_ipv_write(struct file *filp,
				  struct kobject *kobj,
				  struct bin_attribute *bin_attr,
				  char *buf, loff_t ofs, size_t count)
{
	s8 *ipv = (s8 *)buf;
	struct edgx_sched *sched = edgx_dev2psfp_sched(kobj_to_dev(kobj),
						       ofs, count,
						       sizeof(*ipv));
	u8 gate_states = 0;
	int ret;

	if (!sched)
		return -EINVAL;

	if (*ipv != PSFP_GT_IPV_NULL)
		gate_states = PSFP_GT_IPV_ENA_MASK |
			      FIELD_PREP(PSFP_GT_IPV_MASK, *ipv);
	ret = edgx_sched_set_admin_gate_states(sched, gate_states,
					       PSFP_GT_IPV_MASK |
					       PSFP_GT_IPV_ENA_MASK);
	return ret ? ret : count;
}

static ssize_t gt_closed_inv_rx_ena_read(struct file *filp,
					 struct kobject *kobj,
					 struct bin_attribute *bin_attr,
					 char *buf, loff_t ofs, size_t count)
{
	size_t idx;
	struct psfp_gt_sgate_entry entry;
	struct psfp_gates *gates = edgx_dev2psfp_gate(kobj_to_dev(kobj),
						      ofs, count, 1, &idx);

	if (!gates)
		return -EINVAL;

	edgx_psfp_gt_get_sgate(gates, idx, &entry);
	*buf = entry.closed_inv_rx_ena;
	return count;
}

static ssize_t gt_closed_inv_rx_ena_write(struct file *filp,
					  struct kobject *kobj,
					  struct bin_attribute *bin_attr,
					  char *buf, loff_t ofs, size_t count)
{
	size_t idx;
	struct psfp_gates *gates = edgx_dev2psfp_gate(kobj_to_dev(kobj),
						      ofs, count, 1, &idx);
	struct psfp_gt_sgate_entry entry = {
		.closed_inv_rx_ena = *buf,
	};

	if (!gates)
		return -EINVAL;

	edgx_psfp_gt_set_sgate(gates, idx, &entry, PSFP_SGATE_INV_RX_ENA);
	return count;
}

static ssize_t gt_closed_inv_rx_read(struct file *filp,
				     struct kobject *kobj,
				     struct bin_attribute *bin_attr,
				     char *buf, loff_t ofs, size_t count)
{
	size_t idx;
	struct psfp_gt_sgate_entry entry;
	struct psfp_gates *gates = edgx_dev2psfp_gate(kobj_to_dev(kobj),
						      ofs, count, 1, &idx);

	if (!gates)
		return -EINVAL;

	edgx_psfp_gt_get_sgate(gates, idx, &entry);
	*buf = entry.closed_inv_rx;
	return count;
}

static ssize_t gt_closed_inv_rx_write(struct file *filp,
				      struct kobject *kobj,
				      struct bin_attribute *bin_attr,
				      char *buf, loff_t ofs, size_t count)
{
	size_t idx;
	struct psfp_gates *gates = edgx_dev2psfp_gate(kobj_to_dev(kobj),
						      ofs, count, 1, &idx);
	struct psfp_gt_sgate_entry entry = {
		.closed_inv_rx = *buf,
	};

	if (!gates)
		return -EINVAL;

	edgx_psfp_gt_set_sgate(gates, idx, &entry, PSFP_SGATE_INV_RX);
	return count;
}

static ssize_t gt_admin_ctrl_list_len_read(struct file *filp,
					   struct kobject *kobj,
					   struct bin_attribute *bin_attr,
					   char *buf, loff_t ofs, size_t count)
{
	u32 *list_len = (u32 *)buf;
	struct edgx_sched *sched = edgx_dev2psfp_sched(kobj_to_dev(kobj),
						       ofs, count,
						       sizeof(*list_len));
	int ret;

	if (!sched)
		return -EINVAL;

	ret = edgx_sched_get_ctrl_list_len(sched, EDGX_SCHED_ADMIN, list_len);
	return ret ? ret : count;
}

static ssize_t gt_admin_ctrl_list_len_write(struct file *filp,
					    struct kobject *kobj,
					    struct bin_attribute *bin_attr,
					    char *buf, loff_t ofs, size_t count)
{
	const u32 *list_len = (u32 *)buf;
	struct edgx_sched *sched = edgx_dev2psfp_sched(kobj_to_dev(kobj),
						       ofs, count,
						       sizeof(*list_len));
	int ret;

	if (!sched)
		return -EINVAL;

	ret = edgx_sched_set_ctrl_list_len(sched, *list_len);
	return ret ? ret : count;
}

static ssize_t gt_oper_ctrl_list_len_read(struct file *filp,
					  struct kobject *kobj,
					  struct bin_attribute *bin_attr,
					  char *buf, loff_t ofs, size_t count)
{
	u32 *list_len = (u32 *)buf;
	struct edgx_sched *sched = edgx_dev2psfp_sched(kobj_to_dev(kobj),
						       ofs, count,
						       sizeof(*list_len));
	int ret;

	if (!sched)
		return -EINVAL;

	ret = edgx_sched_get_ctrl_list_len(sched, EDGX_SCHED_OPER, list_len);
	return ret ? ret : count;
}

static void edgx_psfp_gt_import_gcl(struct edgx_sched_tab_entry *sched_entries,
				    size_t count)
{
	struct psfp_gt_tab_entry *gt_entries =
			(struct psfp_gt_tab_entry *)sched_entries;
	size_t i;

	for (i = 0; i < count; i++) {
		sched_entries[i].gate_states =
			edgx_psfp_gt_make_states(gt_entries[i].gate_state,
						 gt_entries[i].ipv >= 0,
						 gt_entries[i].ipv);
	}
}

static void edgx_psfp_gt_export_gcl(struct edgx_sched_tab_entry *sched_entries,
				    size_t count)
{
	struct psfp_gt_tab_entry *gt_entries =
			(struct psfp_gt_tab_entry *)sched_entries;
	u8 gate_states;
	size_t i;

	for (i = 0; i < count; i++) {
		gate_states = sched_entries[i].gate_states;
		if (FIELD_GET(PSFP_GT_IPV_ENA_MASK, gate_states))
			gt_entries[i].ipv = FIELD_GET(PSFP_GT_IPV_MASK,
						      gate_states);
		else
			gt_entries[i].ipv = PSFP_GT_IPV_NULL;
		gt_entries[i].gate_state  = FIELD_GET(PSFP_GT_GATE_MASK,
						      gate_states);
	}
}

static ssize_t gt_admin_ctrl_list_read(struct file *filp,
				       struct kobject *kobj,
				       struct bin_attribute *bin_attr,
				       char *buf, loff_t ofs, size_t count)
{
	struct edgx_sched_tab_entry *entry = (struct edgx_sched_tab_entry *)buf;
	size_t first;
	size_t nelems;
	struct edgx_sched *sched = edgx_dev2psfp_gcl_sched(kobj_to_dev(kobj),
							   ofs, count,
							   &first, &nelems);
	int ret;

	if (!sched)
		return -EINVAL;

	ret = edgx_sched_get_admin_ctrl_list(sched, first, entry, nelems);
	if (ret == 0)
		edgx_psfp_gt_export_gcl(entry, nelems);
	return ret ? ret : count;
}

static ssize_t gt_admin_ctrl_list_write(struct file *filp,
					struct kobject *kobj,
					struct bin_attribute *bin_attr,
					char *buf, loff_t ofs, size_t count)
{
	struct edgx_sched_tab_entry *entry = (struct edgx_sched_tab_entry *)buf;
	size_t first;
	size_t nelems;
	struct edgx_sched *sched = edgx_dev2psfp_gcl_sched(kobj_to_dev(kobj),
							   ofs, count,
							   &first, &nelems);
	int ret;

	if (!sched)
		return -EINVAL;

	edgx_psfp_gt_import_gcl(entry, nelems);
	ret = edgx_sched_set_admin_ctrl_list(sched, first, entry, nelems);
	return ret ? ret : count;
}

static int edgx_psfp_gt_get_oper_ctrl_list(struct edgx_sched *sched,
					   size_t first,
					   struct edgx_sched_tab_entry *entry,
					   size_t count)
{
	u32 time_ns, prev_time_ns = 0;
	u16 gate_states;
	size_t i;
	int ret = 0;
	int tab_idx = edgx_sched_get_used_tab(sched);

	if (tab_idx < 0)
		return tab_idx;

	if (first) {
		ret = edgx_sched_read_entry(sched, tab_idx, first - 1,
					    &gate_states, &prev_time_ns);
		if (ret)
			return ret;
	}

	for (i = 0; !ret && i < count; i++) {
		/* TODO: be explicit on list_len? */
		ret = edgx_sched_read_entry(sched, tab_idx, first + i,
					    &gate_states, &time_ns);
		if (ret == -ENOENT)
			continue;

		entry[i].time_interval = time_ns - prev_time_ns;
		prev_time_ns = time_ns;

		entry[i].gate_states = gate_states;
		entry[i].operation_name = PSFP_GT_OP_SET_GSTATES_IPV;
	}
	for (; i < count; i++)
		entry[i] = edgx_sched_undef_entry;

	return 0;
}

static ssize_t gt_oper_ctrl_list_read(struct file *filp, struct kobject *kobj,
				      struct bin_attribute *bin_attr,
				      char *buf, loff_t ofs, size_t count)
{
	struct edgx_sched_tab_entry *entry = (struct edgx_sched_tab_entry *)buf;
	size_t first;
	size_t nelems;
	struct edgx_sched *sched = edgx_dev2psfp_gcl_sched(kobj_to_dev(kobj),
							   ofs, count,
							   &first, &nelems);
	int ret;

	if (!sched)
		return -EINVAL;

	edgx_sched_lock(sched);
	ret = edgx_psfp_gt_get_oper_ctrl_list(sched, first, entry, nelems);
	if (ret == 0)
		edgx_psfp_gt_export_gcl(entry, nelems);
	edgx_sched_unlock(sched);
	return ret ? ret : count;
}

static ssize_t gt_admin_cycle_time_read(struct file *filp,
					struct kobject *kobj,
					struct bin_attribute *bin_attr,
					char *buf, loff_t ofs, size_t count)
{
	struct edgx_sched_rational *cycle_time =
			(struct edgx_sched_rational *)buf;
	struct edgx_sched *sched = edgx_dev2psfp_sched(kobj_to_dev(kobj),
						       ofs, count,
						       sizeof(*cycle_time));
	int ret;

	if (!sched)
		return -EINVAL;

	ret = edgx_sched_get_cycle_time(sched, EDGX_SCHED_ADMIN, cycle_time);
	return ret ? ret : count;
}

static ssize_t gt_admin_cycle_time_write(struct file *filp,
					 struct kobject *kobj,
					 struct bin_attribute *bin_attr,
					 char *buf, loff_t ofs, size_t count)
{
	const struct edgx_sched_rational *cycle_time =
			(struct edgx_sched_rational *)buf;
	struct edgx_sched *sched = edgx_dev2psfp_sched(kobj_to_dev(kobj),
						       ofs, count,
						       sizeof(*cycle_time));
	int ret;

	if (!sched)
		return -EINVAL;

	ret = edgx_sched_set_cycle_time(sched, cycle_time);
	return ret ? ret : count;
}

static ssize_t gt_oper_cycle_time_read(struct file *filp,
				       struct kobject *kobj,
				       struct bin_attribute *bin_attr,
				       char *buf, loff_t ofs, size_t count)
{
	struct edgx_sched_rational *cycle_time =
			(struct edgx_sched_rational *)buf;
	struct edgx_sched *sched = edgx_dev2psfp_sched(kobj_to_dev(kobj),
						       ofs, count,
						       sizeof(*cycle_time));
	int ret;

	if (!sched)
		return -EINVAL;

	ret = edgx_sched_get_cycle_time(sched, EDGX_SCHED_OPER, cycle_time);
	return ret ? ret : count;
}

static ssize_t gt_admin_cycle_time_ext_read(struct file *filp,
					    struct kobject *kobj,
					    struct bin_attribute *bin_attr,
					    char *buf, loff_t ofs, size_t count)
{
	u32 *cycle_time_ext = (u32 *)buf;
	struct edgx_sched *sched = edgx_dev2psfp_sched(kobj_to_dev(kobj),
						       ofs, count,
						       sizeof(*cycle_time_ext));

	if (!sched)
		return -EINVAL;

	/* NOTE: FLEXDE-5153: Standard Deviation:
	 * Not supported, report as zero.
	 */
	*cycle_time_ext = 0;
	return count;
}

static ssize_t gt_admin_cycle_time_ext_write(struct file *filp,
					     struct kobject *kobj,
					     struct bin_attribute *bin_attr,
					     char *buf, loff_t ofs,
					     size_t count)
{
	u32 *cycle_time_ext = (u32 *)buf;
	size_t gate;
	struct psfp_gates *gates = edgx_dev2psfp_gate(kobj_to_dev(kobj),
						      ofs, count,
						      sizeof(*cycle_time_ext),
						      &gate);

	if (!gates)
		return -EINVAL;

	/* NOTE: FLEXDE-5153: Standard Deviation:
	 * Not supported, log warning if not zero.
	 */
	if (*cycle_time_ext != 0)
		edgx_br_warn(gates->parent,
			     "PSFP CycleTimeExtension not supported, using zero!\n");
	return count;
}

static ssize_t gt_oper_cycle_time_ext_read(struct file *filp,
					   struct kobject *kobj,
					   struct bin_attribute *bin_attr,
					   char *buf, loff_t ofs, size_t count)
{
	u32 *cycle_time_ext = (u32 *)buf;
	struct edgx_sched *sched = edgx_dev2psfp_sched(kobj_to_dev(kobj),
						       ofs, count,
						       sizeof(*cycle_time_ext));

	if (!sched)
		return -EINVAL;

	/* NOTE: FLEXDE-5153: Standard Deviation:
	 * Not supported, report as zero.
	 */
	*cycle_time_ext = 0;
	return count;
}

static ssize_t gt_admin_base_time_read(struct file *filp,
				       struct kobject *kobj,
				       struct bin_attribute *bin_attr,
				       char *buf, loff_t ofs, size_t count)
{
	struct timespec64 *base_time = (struct timespec64 *)buf;
	struct edgx_sched *sched = edgx_dev2psfp_sched(kobj_to_dev(kobj),
						       ofs, count,
						       sizeof(*base_time));
	int ret;

	if (!sched)
		return -EINVAL;

	ret = edgx_sched_get_base_time(sched, EDGX_SCHED_ADMIN, base_time);
	return ret ? ret : count;
}

static ssize_t gt_admin_base_time_write(struct file *filp,
					struct kobject *kobj,
					struct bin_attribute *bin_attr,
					char *buf, loff_t ofs, size_t count)
{
	const struct timespec64 *base_time = (struct timespec64 *)buf;
	struct edgx_sched *sched = edgx_dev2psfp_sched(kobj_to_dev(kobj),
						       ofs, count,
						       sizeof(*base_time));
	int ret;

	if (!sched)
		return -EINVAL;

	ret = edgx_sched_set_base_time(sched, base_time);
	return ret ? ret : count;
}

static ssize_t gt_oper_base_time_read(struct file *filp,
				      struct kobject *kobj,
				      struct bin_attribute *bin_attr,
				      char *buf, loff_t ofs, size_t count)
{
	struct timespec64 *base_time = (struct timespec64 *)buf;
	struct edgx_sched *sched = edgx_dev2psfp_sched(kobj_to_dev(kobj),
						       ofs, count,
						       sizeof(*base_time));
	int ret;

	if (!sched)
		return -EINVAL;

	ret = edgx_sched_get_base_time(sched, EDGX_SCHED_OPER, base_time);
	return ret ? ret : count;
}

static ssize_t gt_config_change_read(struct file *filp,
				     struct kobject *kobj,
				     struct bin_attribute *bin_attr,
				     char *buf, loff_t ofs, size_t count)
{
	bool config_change;
	struct edgx_sched *sched = edgx_dev2psfp_sched(kobj_to_dev(kobj),
						       ofs, count, 1);
	int ret;

	if (!sched)
		return -EINVAL;

	ret = edgx_sched_get_config_change(sched, &config_change);
	if (ret == 0)
		*buf = config_change;
	return ret ? ret : count;
}

static ssize_t gt_config_change_write(struct file *filp,
				      struct kobject *kobj,
				      struct bin_attribute *bin_attr,
				      char *buf, loff_t ofs, size_t count)
{
	struct edgx_sched *sched = edgx_dev2psfp_sched(kobj_to_dev(kobj),
						       ofs, count, 1);
	int ret;

	if (!sched)
		return -EINVAL;

	ret = edgx_sched_set_config_change(sched, *buf);
	return ret ? ret : count;
}

static ssize_t gt_config_change_time_read(struct file *filp,
					  struct kobject *kobj,
					  struct bin_attribute *bin_attr,
					  char *buf, loff_t ofs, size_t count)
{
	struct timespec64 *cc_time = (struct timespec64 *)buf;
	struct edgx_sched *sched = edgx_dev2psfp_sched(kobj_to_dev(kobj),
						       ofs, count,
						       sizeof(*cc_time));
	int ret;

	if (!sched)
		return -EINVAL;

	ret = edgx_sched_get_config_change_time(sched, cc_time);
	return ret ? ret : count;
}

static ssize_t gt_config_pending_read(struct file *filp,
				      struct kobject *kobj,
				      struct bin_attribute *bin_attr,
				      char *buf, loff_t ofs, size_t count)
{
	bool config_pending;
	struct edgx_sched *sched = edgx_dev2psfp_sched(kobj_to_dev(kobj),
						       ofs, count, 1);
	int ret;

	if (!sched)
		return -EINVAL;

	ret = edgx_sched_get_config_pending(sched, &config_pending);
	if (ret == 0)
		*buf = config_pending;
	return ret ? ret : count;
}

static ssize_t gt_config_change_error_read(struct file *filp,
					   struct kobject *kobj,
					   struct bin_attribute *bin_attr,
					   char *buf, loff_t ofs, size_t count)
{
	u64 *cc_error = (u64 *)buf;
	struct edgx_sched *sched = edgx_dev2psfp_sched(kobj_to_dev(kobj),
						       ofs, count,
						       sizeof(*cc_error));
	int ret;

	if (!sched)
		return -EINVAL;

	ret = edgx_sched_get_config_change_error(sched, cc_error);
	return ret ? ret : count;
}

static ssize_t gt_tick_granul_read(struct file *filp,
				   struct kobject *kobj,
				   struct bin_attribute *bin_attr,
				   char *buf, loff_t ofs, size_t count)
{
	u32 *tick_granul = (u32 *)buf;
	struct edgx_sched *sched = edgx_dev2psfp_sched(kobj_to_dev(kobj),
						       ofs, count,
						       sizeof(*tick_granul));
	const struct edgx_sched_limits *limits;

	if (!sched)
		return -EINVAL;

	limits = edgx_sched_get_limits(sched);
	*tick_granul = limits->hw_granularity_ns * 10;
	return count;
}

static ssize_t gt_current_time_read(struct file *filp,
				    struct kobject *kobj,
				    struct bin_attribute *bin_attr,
				    char *buf, loff_t ofs, size_t count)
{
	struct timespec64 *ts = (struct timespec64 *)buf;
	struct edgx_sched *sched = edgx_dev2psfp_sched(kobj_to_dev(kobj),
						       ofs, count,
						       sizeof(*ts));
	int ret;

	if (!sched)
		return -EINVAL;

	ret = edgx_sched_get_current_time(sched, ts);
	return ret ? ret : count;
}

/* User Interface: PSFP Parameter Table */

static ssize_t max_flts_show(struct device *dev,
			     struct device_attribute *attr,
			     char *buf)
{
	struct edgx_psfp *psfp = edgx_dev2psfp(dev);

	return scnprintf(buf, PAGE_SIZE, "%u\n", psfp->limits.max_flt_cnt);
}

static ssize_t max_gates_show(struct device *dev,
			      struct device_attribute *attr,
			      char *buf)
{
	struct edgx_psfp *psfp = edgx_dev2psfp(dev);

	return scnprintf(buf, PAGE_SIZE, "%u\n", psfp->limits.max_gate_cnt);
}

static ssize_t max_fmtrs_show(struct device *dev,
			      struct device_attribute *attr,
			      char *buf)
{
	struct edgx_psfp *psfp = edgx_dev2psfp(dev);

	return scnprintf(buf, PAGE_SIZE, "%u\n", psfp->limits.max_fmtr_cnt);
}

static ssize_t max_gcl_len_show(struct device *dev,
				struct device_attribute *attr,
				char *buf)
{
	struct edgx_psfp *psfp = edgx_dev2psfp(dev);

	return scnprintf(buf, PAGE_SIZE, "%u\n", psfp->limits.max_gcl_len);
}

static ssize_t max_cyc_len_show(struct device *dev,
				struct device_attribute *attr,
				char *buf)
{
	struct psfp_gates *gates = edgx_dev2psfp_gates(dev);
	struct edgx_sched *sched = edgx_psfp_gt_get_sched(gates, 0);
	const struct edgx_sched_limits *limits = edgx_sched_get_limits(sched);
	struct edgx_sched_rational cycle_time;
	int ret;

	edgx_sched_nsec_to_rational(limits->max_cyc_ms * NSEC_PER_MSEC, 0,
				    &cycle_time);
	ret = scnprintf(buf, PAGE_SIZE, "%u/%u\n",
			cycle_time.num, cycle_time.denom);
	return ret;
}

static ssize_t max_int_len_show(struct device *dev,
				struct device_attribute *attr,
				char *buf)
{
	struct psfp_gates *gates = edgx_dev2psfp_gates(dev);
	struct edgx_sched *sched = edgx_psfp_gt_get_sched(gates, 0);
	const struct edgx_sched_limits *limits = edgx_sched_get_limits(sched);
	u32 max_int_ns = limits->hw_granularity_ns * limits->max_int_len;
	int ret;

	ret = scnprintf(buf, PAGE_SIZE, "%u\n", max_int_ns);
	return ret;
}

/* User Interface: sysfs */

EDGX_DEV_ATTR_RO(max_flts,	"tsnPsfpMaxStreamFilterInstances");
EDGX_DEV_ATTR_RO(max_gates,	"tsnPsfpMaxStreamGateInstances");
EDGX_DEV_ATTR_RO(max_fmtrs,	"tsnPsfpMaxFlowMeterInstances");
EDGX_DEV_ATTR_RO(max_gcl_len,	"tsnPsfpSupportedListMax");
EDGX_DEV_ATTR_RO(max_cyc_len,	"tsnPsfpSupportedCycleMax");
EDGX_DEV_ATTR_RO(max_int_len,	"tsnPsfpSupportedIntervalMax");
EDGX_DEV_ATTR_WO(flt_del,	"tsnPsfpStreamFilterDelete");

EDGX_BIN_ATTR_RW(flt_tbl, "tsnPsfpStreamFilterTable",
		 PSFP_MAX_NO_FILTERS * sizeof(struct psfp_flt_sysfs_entry));
EDGX_BIN_ATTR_RO(flt_cnt_matching_frames, "tsnPsfpMatchingFramesCount",
		 PSFP_MAX_NO_FILTERS * sizeof(u32));
EDGX_BIN_ATTR_RO(flt_cnt_passing_frames, "tsnPsfpPassingFramesCount",
		 PSFP_MAX_NO_FILTERS * sizeof(u32));
EDGX_BIN_ATTR_RO(flt_cnt_not_passing_frames, "tsnPsfpNotPassingFramesCount",
		 PSFP_MAX_NO_FILTERS * sizeof(u32));
EDGX_BIN_ATTR_RO(flt_cnt_passing_sdu, "tsnPsfpPassingSDUCount",
		 PSFP_MAX_NO_FILTERS * sizeof(u32));
EDGX_BIN_ATTR_RO(flt_cnt_not_passing_sdu, "tsnPsfpNotPassingSDUCount",
		 PSFP_MAX_NO_FILTERS * sizeof(u32));
EDGX_BIN_ATTR_RO(flt_cnt_red_frames, "tsnPsfpREDFramesCount",
		 PSFP_MAX_NO_FILTERS * sizeof(u32));

EDGX_DEV_ATTR_RW(gt_hit_gate, "_ExpectedGateThresholdGateSelect");
EDGX_DEV_ATTR_RW(gt_hit_th_low, "_ExpectedGateThresholdLow");
EDGX_DEV_ATTR_RW(gt_hit_th_high, "_ExpectedGateThresholdHigh");

EDGX_BIN_ATTR_RO(gt_hit_low_cnt, "_ExpectedGateThresholdLowViolationCount",
		 PSFP_MAX_NO_STREAMS * sizeof(u32));
EDGX_BIN_ATTR_RO(gt_hit_high_cnt, "_ExpectedGateThresholdHighViolationCount",
		 PSFP_MAX_NO_STREAMS * sizeof(u32));

EDGX_BIN_ATTR_RW(gt_gate_enabled, "PSFPGateEnabled",
		 PSFP_MAX_NO_GATES * 1);
EDGX_BIN_ATTR_RW(gt_admin_gate_states, "PSFPAdminGateStates",
		 PSFP_MAX_NO_GATES * 1);
EDGX_BIN_ATTR_RW(gt_admin_ipv, "PSFPAdminIPV",
		 PSFP_MAX_NO_GATES * sizeof(s32));
EDGX_BIN_ATTR_RW(gt_closed_inv_rx_ena, "PSFPGateClosedDueToInvalidRxEnable",
		 PSFP_MAX_NO_GATES * 1);
EDGX_BIN_ATTR_RW(gt_closed_inv_rx, "PSFPGateClosedDueToInvalidRx",
		 PSFP_MAX_NO_GATES * 1);
EDGX_BIN_ATTR_RW(gt_admin_ctrl_list_len, "PSFPAdminControlListLength",
		 PSFP_MAX_NO_GATES * sizeof(u32));
EDGX_BIN_ATTR_RO(gt_oper_ctrl_list_len, "PSFPOperControlListLength",
		 PSFP_MAX_NO_GATES * sizeof(u32));
EDGX_BIN_ATTR_RW(gt_admin_ctrl_list, "PSFPAdminControlList",
		 PSFP_MAX_NO_GATES * EDGX_SCHED_HW_MAX_ROWS *
		 sizeof(struct edgx_sched_tab_entry));
EDGX_BIN_ATTR_RO(gt_oper_ctrl_list, "PSFPOperControlList",
		 PSFP_MAX_NO_GATES * EDGX_SCHED_HW_MAX_ROWS *
		 sizeof(struct edgx_sched_tab_entry));
EDGX_BIN_ATTR_RW(gt_admin_cycle_time, "PSFPAdminCycleTime",
		 PSFP_MAX_NO_GATES * sizeof(struct edgx_sched_rational));
EDGX_BIN_ATTR_RO(gt_oper_cycle_time, "PSFPOperCycleTime",
		 PSFP_MAX_NO_GATES * sizeof(struct edgx_sched_rational));
EDGX_BIN_ATTR_RW(gt_admin_cycle_time_ext, "PSFPAdminCycleTimeExtension",
		 PSFP_MAX_NO_GATES * sizeof(u32));
EDGX_BIN_ATTR_RO(gt_oper_cycle_time_ext, "PSFPOperCycleTimeExtension",
		 PSFP_MAX_NO_GATES * sizeof(u32));
EDGX_BIN_ATTR_RW(gt_admin_base_time, "PSFPAdminBaseTime",
		 PSFP_MAX_NO_GATES * sizeof(struct timespec64));
EDGX_BIN_ATTR_RO(gt_oper_base_time, "PSFPOperBaseTime",
		 PSFP_MAX_NO_GATES * sizeof(struct timespec64));
EDGX_BIN_ATTR_RW(gt_config_change, "PSFPConfigChange",
		 PSFP_MAX_NO_GATES * 1);
EDGX_BIN_ATTR_RO(gt_config_change_time, "PSFPConfigChangeTime",
		 PSFP_MAX_NO_GATES * sizeof(struct timespec64));
EDGX_BIN_ATTR_RO(gt_config_pending, "PSFPConfigPending",
		 PSFP_MAX_NO_GATES * 1);
EDGX_BIN_ATTR_RO(gt_config_change_error, "PSFPConfigChangeError",
		 PSFP_MAX_NO_GATES * sizeof(u64));
EDGX_BIN_ATTR_RO(gt_tick_granul, "PSFPTickGranularity",
		 PSFP_MAX_NO_GATES * sizeof(u32));
EDGX_BIN_ATTR_RO(gt_current_time, "PSFPCurrentTime",
		 PSFP_MAX_NO_GATES * sizeof(struct timespec64));

static struct attribute *ieee8021_psfp_attrs[] = {
	&dev_attr_max_flts.attr,
	&dev_attr_max_gates.attr,
	&dev_attr_max_fmtrs.attr,
	&dev_attr_max_gcl_len.attr,
	&dev_attr_max_cyc_len.attr,
	&dev_attr_max_int_len.attr,
	&dev_attr_flt_del.attr,
	&dev_attr_gt_hit_gate.attr,
	&dev_attr_gt_hit_th_low.attr,
	&dev_attr_gt_hit_th_high.attr,
	NULL,
};

static struct bin_attribute *ieee8021_psfp_binattrs[] = {
	&bin_attr_flt_tbl,
	&bin_attr_flt_cnt_matching_frames,
	&bin_attr_flt_cnt_passing_frames,
	&bin_attr_flt_cnt_not_passing_frames,
	&bin_attr_flt_cnt_passing_sdu,
	&bin_attr_flt_cnt_not_passing_sdu,
	&bin_attr_flt_cnt_red_frames,
	&bin_attr_gt_gate_enabled,
	&bin_attr_gt_admin_gate_states,
	&bin_attr_gt_admin_ipv,
	&bin_attr_gt_closed_inv_rx_ena,
	&bin_attr_gt_closed_inv_rx,
	&bin_attr_gt_admin_ctrl_list_len,
	&bin_attr_gt_oper_ctrl_list_len,
	&bin_attr_gt_admin_ctrl_list,
	&bin_attr_gt_oper_ctrl_list,
	&bin_attr_gt_admin_cycle_time,
	&bin_attr_gt_oper_cycle_time,
	&bin_attr_gt_admin_cycle_time_ext,
	&bin_attr_gt_oper_cycle_time_ext,
	&bin_attr_gt_admin_base_time,
	&bin_attr_gt_oper_base_time,
	&bin_attr_gt_config_change,
	&bin_attr_gt_config_change_time,
	&bin_attr_gt_config_pending,
	&bin_attr_gt_config_change_error,
	&bin_attr_gt_tick_granul,
	&bin_attr_gt_current_time,
	NULL,
};

static struct attribute_group ieee8021_psfp_group = {
	.name = "ieee8021PSFP",
	.attrs = ieee8021_psfp_attrs,
	.bin_attrs = ieee8021_psfp_binattrs,
};

static struct bin_attribute *ieee8021_psfp_pt_binattrs[] = {
	&bin_attr_gt_hit_low_cnt,
	&bin_attr_gt_hit_high_cnt,
	NULL,
};

static struct attribute_group ieee8021_psfp_pt_group = {
	.name = "ieee8021PSFP",
	.bin_attrs = ieee8021_psfp_pt_binattrs,
};

/* PSFP functionality [de]initialization */

int edgx_probe_psfp(struct edgx_br *br, struct edgx_br_irq *irq,
		    const char *drv_name, struct edgx_psfp **ppsfp)
{
	const struct edgx_ifdesc *ifd_gating;
	const struct edgx_ifreq   ifreq_gating = {
		.id    = AC_PSFP_GATING_ID,
		.v_maj = 2
	};
	const struct edgx_ifdesc *ifd_flt;
	const struct edgx_ifreq   ifreq_flt    = {
		.id    = AC_PSFP_ID,
		.v_maj = 1
	};
	struct edgx_ifdesc	 ptifd;
	ptid_t			 ptid;
	struct edgx_psfp	*psfp;
	struct psfp_limits	*limits;
	u32			 res_fmtr_cnt;
	u32			 max_gate_cnt;
	int ret = -ENODEV;

	if (!br || !ppsfp)
		return -EINVAL;

	ifd_gating = edgx_ac_get_if(&ifreq_gating);
	if (!ifd_gating || ifd_gating->ptmap == 0)
		return -ENODEV;

	max_gate_cnt = edgx_br_get_generic(br, BR_GX_STREAM_NGATES);

	psfp = kzalloc(sizeof(**ppsfp) + max_gate_cnt * sizeof(struct psfp_gt),
		       GFP_KERNEL);
	if (!psfp)
		return -ENOMEM;

	limits = &psfp->limits;
	res_fmtr_cnt = edgx_br_get_num_ports(br);
	psfp->parent = br;
	limits->max_str_cnt  = edgx_br_get_generic_pow2(br,
							BR_GX_PSFP_NSTREAMS);
	limits->max_gate_cnt = max_gate_cnt;
	limits->max_fmtr_cnt = edgx_br_get_generic_pow2(br, BR_GX_POLICERS);
	limits->max_fmtr_cnt -= res_fmtr_cnt;
	/* TODO: No flow meter (policing) support yet */
	limits->max_fmtr_cnt = 0;
	limits->max_flt_cnt  = limits->max_str_cnt * EDGX_BR_NUM_PCP;

	ifd_flt = edgx_ac_get_if(&ifreq_flt);
	if (!ifd_flt)
		goto out_flt;

	ret = edgx_psfp_flt_init(&psfp->flt, ifd_flt->iobase, limits);
	if (ret)
		goto out_flt;
	if (edgx_br_get_generic(br, BR_GX_COUNTERS) & BIT(3))
		if ((ret = edgx_psfp_hit_init(&psfp->hit_cnt,
					      br,
					      ifd_flt->iobase)) != 0)
			goto out_hit;
	ret = edgx_psfp_gt_init(br, irq, drv_name, ifd_gating, ifd_flt,
				limits, &psfp->gates);
	if (ret)
		goto out_gates;
	if ((ret = edgx_br_sysfs_add(br, &ieee8021_psfp_group)) != 0)
		goto out_sysfs;

	edgx_ac_for_each_ifpt(ptid, ifd_gating, &ptifd) {
		struct edgx_pt *pt = edgx_br_get_brpt(br, ptid);

		if (pt)
			edgx_pt_add_sysfs(pt, &ieee8021_psfp_pt_group);
	}

	edgx_br_info(br, "Setup Per Stream Filtering and Policing ... done");
	*ppsfp = psfp;
	return 0;

out_sysfs:
out_gates:
out_hit:
out_flt:
	edgx_br_err(br, "Setup Per Stream Filtering and Policing ... failed");
	edgx_shutdown_psfp(*ppsfp);
	return ret;
}

void edgx_shutdown_psfp(struct edgx_psfp *psfp)
{
	const struct edgx_ifreq ifreq = {
		.id    = AC_PSFP_GATING_ID,
		.v_maj = 2
	};
	const struct edgx_ifdesc *ifd;
	struct edgx_ifdesc        ptifd;
	ptid_t                    ptid;

	if (!psfp)
		return;

	ifd = edgx_ac_get_if(&ifreq);
	if (!ifd)
		return;

	edgx_ac_for_each_ifpt(ptid, ifd, &ptifd) {
		struct edgx_pt *pt = edgx_br_get_brpt(psfp->parent, ptid);

		if (pt)
			edgx_pt_rem_sysfs(pt, &ieee8021_psfp_pt_group);
	}

	edgx_psfp_gt_shutdown(&psfp->gates);
	edgx_psfp_hit_shutdown(psfp->hit_cnt);
	edgx_psfp_flt_shutdown(&psfp->flt);
	kfree(psfp);
}
