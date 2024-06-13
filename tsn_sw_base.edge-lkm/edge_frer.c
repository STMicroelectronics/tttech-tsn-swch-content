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

#include "edge_frer.h"
#include "edge_util.h"
#include "edge_br_fdb.h"
#include "edge_port.h"
#include "edge_br_sid.h"
#include "edge_ac.h"

#define STREAM_CMD    0x0400
#define STREAM_TABLE0 0x0410
#define STREAM_TABLE1 0x0412
#define STREAM_TABLE2 0x0414
#define STREAM_TABLE3 0x0416

#define FRER_MAX_NO_STREAMS 1024U /*max FRER_ENTRIES = max FRER_STREAMS*/
#define FRER_TBL0 0
#define FRER_TBL1 1
#define FRER_TBL2 2 //Sequence Generation
#define FRER_TBL3 3 //Sequence Recovery
#define FRER_DEL 1U
#define FRER_U16_INVALID 0xFFFF
#define FRER_BOOL_INVALID 0XFF
#define FRER_WRITE_BUSY(ctrl)     ((ctrl) & BIT(15))
#define FRER_RESET_BUSY(ctrl)     ((ctrl) & BIT(13))

#define _SEQ_REC_CMD   0x0300
#define _SEQ_REC_TBL   0x0318
#define _SEQ_GEN_CMD 0x0380  /* Reset sequence number */
#define _STRTBL_MASK 0xFFF /* bits 11-0 - used as pointer to row of tbl*/

#define _SEQREC_MAX_HISTL	0xF
#define _SEQREC_MIN_HISTL	0x2
#define _SEQREC_NO_SEQ		0x1
#define _SEQREC_IND_REC		0x2
#define _SEQREC_LAT_ERR		0x3
#define _SEQREC_SHIFT_ALG	5U
#define _SEQREC_SHIFT_INDREC	6U
#define _SEQREC_SHIFT_TNS	7U
#define _SEQREC_SHIFT_BRATE	12U
#define _SEQREC_SHIFT_ARATE	14U
#define _SEQREC_INVSEQ		65536U //RecovSeqSpace
#define _SEQREC_RCVRST_MAXRES	16U

#define _CNT_SEQREC_TOUT_L 0x200
#define _CNT_SEQREC_TOUT_H 0x202
#define _CNT_VECT_JUMP_L 0x0204
#define _CNT_VECT_JUMP_H 0x0206
#define _CNT_SEQREC_OOO_L 0x0208
#define _CNT_SEQREC_OOO_H 0x020A
#define _CNT_SEQREC_OOO2_L 0x0308
#define _CNT_SEQREC_OOO2_H 0x030A
#define _CNT_VECT_ROG_L 0x030C
#define _CNT_VECT_ROG_H 0x030E
#define _CNT_SEQREC_TAGLESS_L 0x0310
#define _CNT_SEQREC_TAGLESS_H 0x0312
#define _CNT_SEQREC_SB_L 0x0314
#define _CNT_SEQREC_SB_H 0x0316
#define _CNT_CAPT0 0x0
#define _CNT_CAPT1 0x2
#define _CNT_CAPT2 0x4
#define _CNT_CAPT3 0x6

#define AGING_BASE_TIME_LO 0x0022
#define AGING_BASE_TIME_HI 0x0024
#define FRER_SCNT_OFFS 0xF000

enum frer_dir {
	DIR_IN_FAC = 0,
	DIR_OUT_FAC,
	DIR_UNKNOWN,
};

enum _seqenc_type {
	TYPE_UNKNOWN = 0,
	TYPE_R_TAG = 1, //Only R-tag used
	TYPE_HSR_TAG = 2,
	TYPE_PRP_TRAILER = 3,
};

enum _seqenc_active {
	ENC_PASSIVE = 0,
	ENC_ACTIVE = 1,
	ENC_UNKNOWN = 2,//used when entry is added but active not set yet
	ENC_UNDEFINED = 3,//used at init, entry doesn't exist

};

enum _seqrec_alg {
	ALG_VECT = 0,
	ALG_MATCH = 1,
	ALG_UNKNOWN = 2,
};

struct _se_strlist {
	struct list_head entry;
	u16 strhdl;
};

struct _se_ports {
	struct list_head se_list;
	struct _se_strlist *l;
	u8 active;
};

struct _se_streams_rbt {
	struct rb_node rbnode;
	u16 strhdl; //key
	u16 port_mask;
};

struct strlist {
	struct list_head entry;
	u16 strhdl;
};

struct _seqgen_rbt {
	struct rb_node rbnode;
	struct list_head sg_list;
	struct strlist *l; /*multiple str handles per 1 seqgenfn possible*/
	u8 dir;
	u16 fnidx; /* RBT key */
};

struct _seqrec_rbt {
	struct rb_node rbnode;
	struct list_head sr_list; //list of stream handles
	struct strlist *l; /*list of strhdls for 1 seqrec fn*/
	enum _seqrec_alg alg;
	u16 fnidx; //key
	u16 rcv_rst;
	u16 ptmask;
	u8 dir;
	u8 hist_len;//4 bits assigned by the IP
	u8 take_no_seq;
	u8 ind_rec;
	u8 lat_err;
	bool seqrec_cfg_applied;
};

/*sq - sequence generation, se - sequence encode, sr - sequence recovery*/
struct edgx_frer {
	struct edgx_br *parent;
	edgx_io_t *iobase;
	struct rb_root sg_root;
	struct rb_root se_root;
	struct rb_root sr_root;
	struct _se_ports se_ports[EDGX_BR_MAX_PORTS];
	u16 seqrec_max;
	u16 streams_max;
	u16 seqgencnt;
	u16 seqreccnt;
	u16 seqrec_rst[_SEQREC_RCVRST_MAXRES];//FLEXDE-3139
	struct mutex lock; /*Protects FRER data; access to STREAM_TABLE0/1/2/3*/
};

//FLEXDE-3139
static u32 calc_rcv_rst(struct edgx_frer **f)
{
	u8 i, j;
	u32 aging_bt, denom;
	u64 num;
	unsigned int tclk = edgx_br_get_cycle_ns((*f)->parent);

	aging_bt = edgx_rd16((*f)->iobase, AGING_BASE_TIME_LO) |
		   (((u32)edgx_rd16((*f)->iobase, AGING_BASE_TIME_HI)) << 16);

	//rem = do_div(num, sec_denom);
	for (i = 0; i < 4; i++) {
		for (j = 0; j < 4; j++) {
			//i = aging rate scale, j = aging basic rate
			num = tclk * aging_bt * 124;
			//multiply with 1000000 to convert ns to ms
			denom = 1000000 * (1 << i) * (j + 5);
			do_div(num, denom);
			(*f)->seqrec_rst[(i * 4) + j] = num;
		}
	}

	return 0;
}

static u8 first_bigger(struct edgx_frer *f, u16 val)
{
	s8 i = 15;

	//The array is sorted, first member is the biggest one
	while (f->seqrec_rst[(u8)i] < val && i >= 0)
		i--;
	if (i < 0)
		i = 0;
	return (u8)i;
}

/*per entity = function counter*/
static u32 edgx_frer_ip_perfn_cnt(edgx_io_t *base_addr, u16 fnidx, int cnt)
{
	u16 capture = fnidx;
	edgx_io_t *cnt_base = base_addr + FRER_SCNT_OFFS;

	edgx_wr16(cnt_base, _CNT_CAPT1, capture);

	do {
		capture = edgx_rd16(cnt_base, _CNT_CAPT0);
	} while (capture & BIT(0));

	switch (cnt) {
	case _CNT_SEQREC_TOUT_L:
	case _CNT_SEQREC_TOUT_H:
		return (edgx_rd16(cnt_base, _CNT_SEQREC_TOUT_L) |
		       (((u32)edgx_rd16(cnt_base, _CNT_SEQREC_TOUT_H)) << 16));
	case _CNT_VECT_JUMP_L:
	case _CNT_VECT_JUMP_H:
		return (edgx_rd16(cnt_base, _CNT_VECT_JUMP_L) |
		       (((u32)edgx_rd16(cnt_base, _CNT_VECT_JUMP_H)) << 16));
	case _CNT_SEQREC_OOO_L:
	case _CNT_SEQREC_OOO_H:
		return (edgx_rd16(cnt_base, _CNT_SEQREC_OOO_L) |
		       (((u32)edgx_rd16(cnt_base, _CNT_SEQREC_OOO_H)) << 16));
	}

	return 0;
}

/*per port per stream frer counters*/
static u32 edgx_frer_ip_ppps_cnt(edgx_io_t *base_addr, u16 str_hdl, u8 port,
				 int cnt)
{
	u16 capture = port | (str_hdl << 4);
	edgx_io_t *cnt_base = base_addr + FRER_SCNT_OFFS;

	edgx_wr16(cnt_base, _CNT_CAPT3, capture);

	do {
		capture = edgx_rd16(cnt_base, _CNT_CAPT0);
	} while (capture & BIT(5));

	switch (cnt) {
	case _CNT_SEQREC_OOO2_L:
	case _CNT_SEQREC_OOO2_H:
		return (edgx_rd16(cnt_base, _CNT_SEQREC_OOO2_L) |
		       (((u32)edgx_rd16(cnt_base, _CNT_SEQREC_OOO2_H)) << 16));
	case _CNT_VECT_ROG_L:
	case _CNT_VECT_ROG_H:
		return (edgx_rd16(cnt_base, _CNT_VECT_ROG_L) |
		       (((u32)edgx_rd16(cnt_base, _CNT_VECT_ROG_H)) << 16));
	case _CNT_SEQREC_TAGLESS_L:
	case _CNT_SEQREC_TAGLESS_H:
		return (edgx_rd16(cnt_base, _CNT_SEQREC_TAGLESS_L) |
		       (((u32)edgx_rd16(cnt_base,
					_CNT_SEQREC_TAGLESS_H)) << 16));
	case _CNT_SEQREC_SB_L:
	case _CNT_SEQREC_SB_H:
		return (edgx_rd16(cnt_base, _CNT_SEQREC_SB_L) |
		       (((u32)edgx_rd16(cnt_base, _CNT_SEQREC_SB_H)) << 16));
	}
	return 0;
}

static u16 edgx_frer_ip_rd_strtbl(struct edgx_frer *f, u16 strhdl, int tbl_num)
{
	u16 val;

	edgx_wr16(f->iobase, STREAM_CMD, 0x8000 | strhdl);
	do {
		val = edgx_rd16(f->iobase, STREAM_CMD);
	} while (FRER_WRITE_BUSY(val));

	switch (tbl_num) {
	case FRER_TBL0:
		return edgx_rd16(f->iobase, STREAM_TABLE0);
	case FRER_TBL1:
		return edgx_rd16(f->iobase, STREAM_TABLE1);
	case FRER_TBL2:
		return edgx_rd16(f->iobase, STREAM_TABLE2);
	case FRER_TBL3:
		return edgx_rd16(f->iobase, STREAM_TABLE3);
	}

	return 0;
}

static void edgx_frer_ip_wr_strtbl(struct edgx_frer *f, u16 strhdl, u16 val,
				   int tbl_num, u8 del)
{
	/*
	 * Read before writing in order to refresh values STREAM_TABLE0-3
	 * Third parameter is not important, it can be any table
	 */
	edgx_frer_ip_rd_strtbl(f, strhdl, FRER_TBL0);

	switch (tbl_num) {
	case FRER_TBL0:
		/*r-tag modify port vector*/
		edgx_wr16(f->iobase, STREAM_TABLE0, val);
		break;
	case FRER_TBL1:
		/*r-tag output port vector*/
		edgx_wr16(f->iobase, STREAM_TABLE1, val);
		break;
	case FRER_TBL2:
		/*val = sequence generation table row*/
		if (!del)
			edgx_wr16(f->iobase, STREAM_TABLE2, val | BIT(15));
		else
			edgx_wr16(f->iobase, STREAM_TABLE2, val & (~BIT(15)));
		break;
	case FRER_TBL3:
		/*val = sequence recovery table row*/
		if (!del)
			edgx_wr16(f->iobase, STREAM_TABLE3, val | BIT(15));
		else
			edgx_wr16(f->iobase, STREAM_TABLE3, val & (~BIT(15)));

		break;
	}

	/* write to row = strhdl of the Stream Table */
	edgx_wr16(f->iobase, STREAM_CMD, 0xC000 | strhdl);
	do {
		val = edgx_rd16(f->iobase, STREAM_CMD);
	} while (FRER_WRITE_BUSY(val));
}

static void edgx_frer_ip_wr_seqrectbl(struct edgx_frer *f, u16 fnidx, u16 val)
{
	/*All parameters are updated, write directly to register*/
	edgx_wr16(f->iobase, _SEQ_REC_TBL, val);

	/* write to row = strhdl of the SeqRecTable */
	edgx_wr16(f->iobase, _SEQ_REC_CMD, 0xC000 | fnidx);
	do {
		val = edgx_rd16(f->iobase, _SEQ_REC_CMD);
	} while (FRER_WRITE_BUSY(val));
}

static int edgx_frer_check_fnidx(u16 fnidx, struct edgx_frer *f)
{
	if (fnidx >= f->seqrec_max) {
		edgx_br_err(f->parent,
			    "SeqGen fn index out of boundaries, max = %d\n",
			    f->seqrec_max - 1);
		return -EINVAL;
	}
	return 0;
}

static int edgx_frer_check_strhdl(u16 strhdl, struct edgx_frer *f)
{
	if (strhdl >= f->streams_max) {
		edgx_br_err(f->parent, "Stream handle out of boundaries\n");
		return -EINVAL;
	}
	return 0;
}

int edgx_frer_check_port(u16 ptid, struct edgx_frer *f)
{
	if (ptid > edgx_br_get_num_ports(f->parent)) {
		edgx_br_err(f->parent, "SeqEnc: Port number invalid\n");
		return -EINVAL;
	} else {
		return 0;
	}
}

/*check if stream handle has sequence or recovery generation function enabled*/
static int edgx_frer_seqfn_enabled(struct edgx_frer *f, u16 strhdl, int tbl)
{
	u16 reg = edgx_frer_ip_rd_strtbl(f, strhdl, tbl);

	return (reg & BIT(15)) ? 1 : 0;
}

static void _seqgen_ip_reset(struct edgx_frer *f, u16 fnidx)
{
	u16 val;

	edgx_wr16(f->iobase, _SEQ_GEN_CMD, fnidx | BIT(13));
	do {
		val = edgx_rd16(f->iobase, _SEQ_GEN_CMD);
	} while (FRER_RESET_BUSY(val));
}

static int _seqgen_check_direction(enum frer_dir dir, struct edgx_frer *f)
{
	if (dir != (u8)DIR_IN_FAC) {
		edgx_br_err(f->parent, "Only in-facing(0) direction allowed\n");
		return -EINVAL;
	}
	return 0;
}

static inline int _seqgen_list_add(u16 strhdl, struct _seqgen_rbt *gen)
{
	gen->l = kzalloc(sizeof(*gen->l), GFP_KERNEL);
	if (!gen->l)
		return -ENOMEM;
	gen->l->strhdl = strhdl;
	list_add_tail(&gen->l->entry, &gen->sg_list);

	return 0;
}

static int seqgen_list_entry_exists(u16 strhdl, struct _seqgen_rbt *gen)
{
	struct list_head *pos;
	struct strlist *l;

	list_for_each(pos, &gen->sg_list) {
		l = list_entry(pos, struct strlist, entry);
		if (l->strhdl == strhdl)
			return 1;
	}

	return 0;
}

static struct _seqgen_rbt *_seqgen_rbt_search(struct rb_root *root, u16 key)
{
	struct rb_node *node = root->rb_node;

	while (node) {
		struct _seqgen_rbt *sgen = rb_entry(node, struct _seqgen_rbt,
						    rbnode);
		if (key < sgen->fnidx)
			node = node->rb_left;
		else if (key > sgen->fnidx)
			node = node->rb_right;
		else
			return sgen;
	}

	return NULL;
}

/*
 * Function used for removing entry from the list within RBT. In case this is
 * the last entry in the RBT, entire rbt node is removed
 */
static int _seqgen_list_del_entry(struct edgx_frer *f, u16 str)
{
	struct strlist *l;
	struct list_head *pos;
	struct list_head *tmp;
	struct _seqgen_rbt *n;
	u16 fn;

	/*
	 * xxx Erroneous use case: SeqGen function added, direction not set,
	 * streams added to the list; delete will not work since nothing is
	 * written to the IP until direction is set. One way to solve this
	 * is to go through entire RBT and through every list in every node and
	 * erase stream handle. Worst case scenario:1024 nodes in the RBT
	 * up to 1024 streams added   Other way is to pass fnidx as parameter
	 * in the sysfs call. Third: leave as is :)
	 */
	if (!edgx_frer_seqfn_enabled(f, str, FRER_TBL2))
		return -EINVAL;

	fn = edgx_frer_ip_rd_strtbl(f, str, FRER_TBL2) & _STRTBL_MASK;

	n = _seqgen_rbt_search(&f->sg_root, fn);

	if (!n) {
		edgx_br_err(f->parent, "Seqgen function not found in SW dtb\n");
		return -EINVAL;
	}

	list_for_each_safe(pos, tmp, &n->sg_list)	{
		l = list_entry(pos, struct strlist, entry);
		if (l->strhdl == str) {
			list_del(pos);
			kfree(l);
			edgx_frer_ip_wr_strtbl(f, str, fn, FRER_TBL2, FRER_DEL);
			return 0;
		}
	}

	/*couldn't find match*/
	return -EINVAL;
}

/* Remove entire RBT entry */
static int _seqgen_rbt_del_entry(struct edgx_frer *f, u16 key)
{
	struct strlist *l;
	struct list_head *tmp;
	struct list_head *pos;
	struct _seqgen_rbt *n = _seqgen_rbt_search(&f->sg_root, key);

	if (!n)
		return -EINVAL;

	/*Delete all stream handles from the list assigned to rbtree*/
	if (!list_empty(&n->sg_list)) {
		list_for_each_safe(pos, tmp, &n->sg_list) {
			l = list_entry(pos, struct strlist, entry);
			list_del(pos);
			kfree(l);
			edgx_frer_ip_wr_strtbl(f, l->strhdl, 0, FRER_TBL2,
					       FRER_DEL);
		}
	}

	/*Delete entry from rb-tree*/
	rb_erase(&n->rbnode, &f->sg_root);
	RB_CLEAR_NODE(&n->rbnode);
	kfree(n);
	f->seqgencnt--;

	return 0;
}

/* Remove single stream from the list without checking RBT or registers */
static int _seqgen_list_rm_single(u16 strhdl, struct _seqgen_rbt *n)
{
	struct list_head *pos;
	struct list_head *tmp;
	struct strlist *l;

	list_for_each_safe(pos, tmp, &n->sg_list) {
		l = list_entry(pos, struct strlist, entry);
		if (strhdl == l->strhdl) {
			list_del(pos);
			kfree(l);
			return 0;
		}
	}

	return -ENOENT;
}

static int _seqgen_rbt_insert_node(struct _seqgen_rbt *new,
				   struct edgx_frer *f)
{
	struct _seqgen_rbt *sgen;
	struct rb_node **pp, *p;

	pp = &f->sg_root.rb_node;
	p = NULL;

	while (*pp) {
		p = *pp;
		sgen = rb_entry(p, struct _seqgen_rbt, rbnode);
		if (new->fnidx < sgen->fnidx)
			pp = &(*pp)->rb_left;
		else if (new->fnidx > sgen->fnidx)
			pp = &(*pp)->rb_right;
		else
			return -EINVAL;
	}

	rb_link_node(&new->rbnode, p, pp);
	rb_insert_color(&new->rbnode, &f->sg_root);

	return 0;
}

static int edgx_frer_seqgen_apply(struct _seqgen_rbt *n, struct edgx_frer *f)
{
	struct list_head *pos;
	struct strlist *l;
	u16 reg;

	if (n->dir == (u8)DIR_UNKNOWN || list_empty(&n->sg_list))
		return 0;

	list_for_each(pos, &n->sg_list) {
		l = list_entry(pos, struct strlist, entry);
		/* entry has Seq-Gen-Fn enabled already*/
		if (edgx_frer_seqfn_enabled(f, l->strhdl, FRER_TBL2)) {
			reg = edgx_frer_ip_rd_strtbl(f, l->strhdl,
						     FRER_TBL2);
			if ((reg & 0xFFF) != n->fnidx) {
				edgx_br_err(f->parent,
					    "Stream handle %d assigned to seqGenFn, %d\n",
					    l->strhdl, reg & 0xFFF);
				return -EBUSY;
			}

		} else {
		/*SeqGenFn data not written to the table yet*/
			edgx_frer_ip_wr_strtbl(f, l->strhdl, n->fnidx,
					       FRER_TBL2, 0);
			_seqgen_ip_reset(f, n->fnidx);
		}
	}

	return 0;
}

static int edgx_frer_seqgen_addstr(struct edgx_frer *f, u16 str, u16 fnidx)
{
	struct _seqgen_rbt *seqgen;

	if (edgx_frer_check_strhdl(str, f))
		return -EINVAL;

	seqgen = _seqgen_rbt_search(&f->sg_root, fnidx);
	if (!seqgen) {
		edgx_br_err(f->parent, "AddStr:SeqGenfn doesn't exist\n");
		return -ENOENT;
	}

	if (!seqgen_list_entry_exists(str, seqgen))
		if (_seqgen_list_add(str, seqgen))
			return -ENOMEM;

	if (edgx_frer_seqgen_apply(seqgen, f)) {
		_seqgen_list_rm_single(str, seqgen);
		return -EBUSY;
	}

	return 0;
}

static int edgx_frer_seqgen_adddir(struct edgx_frer *f, u8 dir, u16 fnidx)
{
	struct _seqgen_rbt *seqgen;

	if (_seqgen_check_direction(dir, f))
		return -EINVAL;

	seqgen = _seqgen_rbt_search(&f->sg_root, fnidx);
	if (!seqgen) {
		edgx_br_err(f->parent, "AddDir:SeqGenFn doesn't exist\n");
		return -ENOENT;
	}
	seqgen->dir = dir;

	if (edgx_frer_seqgen_apply(seqgen, f)) {
		seqgen->dir = (u8)DIR_UNKNOWN;
		return -EBUSY;
	}

	return 0;
}

static int edgx_frer_seqgen_create(struct edgx_frer *f, u16 fnidx)
{
	struct _seqgen_rbt *rbt_gen;

	if (edgx_frer_check_fnidx(fnidx, f))
		return -EINVAL;

	rbt_gen = _seqgen_rbt_search(&f->sg_root, fnidx);
	if (rbt_gen) {
		edgx_br_err(f->parent, "SeqGen entry %d already exists\n",
			    fnidx);
		return -EBUSY;
	}
	rbt_gen = kzalloc(sizeof(*rbt_gen), GFP_KERNEL);
	if (!rbt_gen)
		return -ENOMEM;

	rbt_gen->dir = (u8)DIR_UNKNOWN;
	rbt_gen->fnidx = fnidx;
	f->seqgencnt++;
	INIT_LIST_HEAD(&rbt_gen->sg_list);
	if (_seqgen_rbt_insert_node(rbt_gen, f))
		return -EINVAL;

	return 0;
}

int _seqenc_check_dir(u8 dir, struct edgx_frer *f)
{
	if (dir != (u8)DIR_OUT_FAC) {
		edgx_br_err(f->parent,
			    "SeqEnc:Only out-facing(1) direction supported\n");
		return -EINVAL;
	} else {
		return 0;
	}
}

int _seqenc_check_type(u8 type, struct edgx_frer *f)
{
	if (type != (u8)TYPE_R_TAG) {
		edgx_br_err(f->parent, "Supporting only R-tag encapsulation\n");
		return -EINVAL;
	} else {
		return 0;
	}
}

int _seqenc_check_active(u16 active, struct edgx_frer *f)
{
	if ((active != ENC_ACTIVE) && (active != ENC_PASSIVE)) {
		edgx_br_err(f->parent, "SeqEnc: invalid activation value\n");
		return -EINVAL;
	} else {
		return 0;
	}
}

static int _seqenc_list_entry_exists(u16 strhdl, struct edgx_frer *f, u16 idx)
{
	struct list_head *pos;
	struct _se_strlist *l;

	list_for_each(pos, &f->se_ports[idx].se_list) {
		l = list_entry(pos, struct _se_strlist, entry);
		if (l->strhdl == strhdl)
			return -EEXIST;
	}

	return 0;
}

static inline int _seqenc_list_add(u16 strhdl, struct edgx_frer *f, u16 idx)
{
	f->se_ports[idx].l = kzalloc(sizeof(*f->se_ports[idx].l), GFP_KERNEL);

	if (!f->se_ports[idx].l)
		return -ENOMEM;
	f->se_ports[idx].l->strhdl = strhdl;
	list_add_tail(&f->se_ports[idx].l->entry, &f->se_ports[idx].se_list);
	return 0;
}

static struct _se_streams_rbt *_seqenc_rbt_search(struct rb_root *root, u16 key)
{
	struct rb_node *node = root->rb_node;

	while (node) {
		struct _se_streams_rbt *senc = rb_entry(node,
							struct _se_streams_rbt,
							rbnode);
		if (key < senc->strhdl)
			node = node->rb_left;
		else if (key > senc->strhdl)
			node = node->rb_right;
		else
			return senc;
	}

	return NULL;
}

static int _seqenc_rbt_insert_node(struct _se_streams_rbt *new,
				   struct edgx_frer *f)
{
	struct _se_streams_rbt *senc;
	struct rb_node **pp, *p;

	pp = &f->se_root.rb_node;
	p = NULL;

	while (*pp) {
		p = *pp;
		senc = rb_entry(p, struct _se_streams_rbt, rbnode);
		if (new->strhdl < senc->strhdl)
			pp = &(*pp)->rb_left;
		else if (new->strhdl > senc->strhdl)
			pp = &(*pp)->rb_right;
		else
			return -EINVAL;
	}

	rb_link_node(&new->rbnode, p, pp);
	rb_insert_color(&new->rbnode, &f->se_root);

	return 0;
}

static void _seqenc_apply_active(struct edgx_frer *f, u16 pt, u16 str)
{
	u16 reg;

	//ACTIVE: write 1 to STREAM_TABLE0 and 1 for the chosen port and stream
	reg = edgx_frer_ip_rd_strtbl(f, str, FRER_TBL0);
	reg |= BIT(pt);
	edgx_frer_ip_wr_strtbl(f, str, reg, FRER_TBL0, 0);

	reg = edgx_frer_ip_rd_strtbl(f, str, FRER_TBL1);
	reg |= BIT(pt);
	edgx_frer_ip_wr_strtbl(f, str, reg, FRER_TBL1, 0);
}

static void _seqenc_apply_passive(struct edgx_frer *f, u16 pt, u16 str)
{
	u16 reg;

	//PASSIVE: set STR_TABLE0 to 1 and STR_TABLE1 to 1 for chosen pt-str
	reg = edgx_frer_ip_rd_strtbl(f, str, FRER_TBL0);
	reg |= BIT(pt);
	edgx_frer_ip_wr_strtbl(f, str, reg, FRER_TBL0, 0);

	reg = edgx_frer_ip_rd_strtbl(f, str, FRER_TBL1);
	reg &= (~BIT(pt));
	edgx_frer_ip_wr_strtbl(f, str, reg, FRER_TBL1, 0);
}

static int edgx_frer_seqenc_apply(struct edgx_frer *f, u16 pt)
{
	struct list_head *pos;
	struct strlist *l;

	/*
	 * Configuration is written to the board only if list of streams for
	 * the port is not empty and type of encoding is chosen
	 */
	if (f->se_ports[pt].active > ENC_ACTIVE ||
	    list_empty(&f->se_ports[pt].se_list))
		return -ENOENT;
	/*
	 * Possible improvement: save pointer to the lastly written (applied)
	 * value from list and continue from there
	 */
	list_for_each(pos, &f->se_ports[pt].se_list) {
		l = list_entry(pos, struct strlist, entry);
		if (f->se_ports[pt].active)
			_seqenc_apply_active(f, pt, l->strhdl);
		else
			_seqenc_apply_passive(f, pt, l->strhdl);
	}

	return 0;
}

static void edgx_frer_seqenc_applydel(struct edgx_frer *f, u16 port,
				      u16 strhdl, u16 portmask)
{
	u16 reg;

	if (portmask == 0) {
		/*Stream doesn't have any seq-enc settings, reset IP settings*/
		edgx_frer_ip_wr_strtbl(f, strhdl, 0, FRER_TBL0, 0);
		edgx_frer_ip_wr_strtbl(f, strhdl, 0, FRER_TBL1, 0);
	} else {
		/*Only reset table1 for the corresponding port*/
		reg = edgx_frer_ip_rd_strtbl(f, strhdl, FRER_TBL0);
		reg &= ~BIT(port);
		edgx_frer_ip_wr_strtbl(f, strhdl, reg, FRER_TBL0, 0);

		reg = edgx_frer_ip_rd_strtbl(f, strhdl, FRER_TBL1);
		reg &= ~BIT(port);
		edgx_frer_ip_wr_strtbl(f, strhdl, reg, FRER_TBL1, 0);
	}
}

static int edgx_frer_seqenc_create_entry(u16 port, u8 dir, struct edgx_frer *f)
{
	if (_seqenc_check_dir(dir, f) ||
	    edgx_frer_check_port(port, f))
		return -EINVAL;

	/*
	 * ENC_UNKNOWN denotes that entry was added, but parameters are still
	 * not completely set (stream handles to be added, act/passive to be set
	 * Driver doesn't save data about position since only out-fac is allowed
	 */
	f->se_ports[port].active = ENC_UNKNOWN;
	return 0;
}

static int edgx_frer_seqenc_add_str(u16 str, u16 port,
				    struct edgx_frer *f)
{
	struct _se_streams_rbt *s;
	/* If entry doesn't exist */
	if (f->se_ports[port].active == ENC_UNDEFINED) {
		edgx_br_err(f->parent, "AddStr: SeqEnc entry doesn't exist\n");
		return -ENOENT;
	}
	if (edgx_frer_check_strhdl(str, f))
		return -EINVAL;

	/* If the string is already added to the list nothing to be done */
	if (!_seqenc_list_entry_exists(str, f, port)) {
		if (_seqenc_list_add(str, f, port))
			return -ENOMEM;
	}

	s = _seqenc_rbt_search(&f->se_root, str);

	if (s) {
		//if stream already exists in rbtree, just update port mask
		s->port_mask |= BIT(port);
	} else {
		s = kzalloc(sizeof(*s), GFP_KERNEL);
		if (!s)
			return -ENOMEM;
		s->port_mask = BIT(port);
		s->strhdl = str;
		if (_seqenc_rbt_insert_node(s, f)) {
			kfree(s);
			return -EINVAL;
		}
	}

	edgx_frer_seqenc_apply(f, port);

	return 0;
}

static int edgx_frer_seqenc_set_active(u8 active, u16 port, struct edgx_frer *f)
{
	/* If entry doesn't exist */
	if (f->se_ports[port].active == ENC_UNDEFINED) {
		edgx_br_err(f->parent,
			    "SetActive: SeqEnc entry doesn't exist\n");
		return -ENOENT;
	} else if (_seqenc_check_active(active, f)) {
		edgx_br_err(f->parent,
			    "SetActive value incorrect, set to either Passive(0), Active(1)\n");
		return -EINVAL;
	}
	f->se_ports[port].active = active ? 1 : 0;
	edgx_frer_seqenc_apply(f, port);

	return 0;
}

static int _seqenc_del_str_from_rbt(u16 strhdl, struct edgx_frer *f)
{
	struct _se_streams_rbt *se_rbt;

	se_rbt = _seqenc_rbt_search(&f->se_root, strhdl);

	if (se_rbt) {
		rb_erase(&se_rbt->rbnode, &f->se_root);
		RB_CLEAR_NODE(&se_rbt->rbnode);
		kfree(se_rbt);
	} else {
		return -ENOENT;
	}

	return 0;
}

static int edgx_frer_seqenc_delportfn(u16 port, struct edgx_frer *f)
{
	struct list_head *tmp;
	struct list_head *pos;
	struct _se_strlist *l_se;
	struct _se_streams_rbt *se_rbt;

	if (f->se_ports[port].active == ENC_UNDEFINED)
		return -ENOENT;

	if (!list_empty(&f->se_ports[port].se_list)) {
		list_for_each_safe(pos, tmp, &f->se_ports[port].se_list) {
			l_se = list_entry(pos, struct _se_strlist, entry);
			se_rbt = _seqenc_rbt_search(&f->se_root, l_se->strhdl);
			if (se_rbt) {
				se_rbt->port_mask &= ~BIT(port);
				if (!se_rbt->port_mask)
					_seqenc_del_str_from_rbt(l_se->strhdl,
								 f);
			} else {
				edgx_br_err(f->parent,
					    "Undefined behavior: Stream handle %d might not be added properly\n",
					    l_se->strhdl);
				return -EINVAL;
			}
			list_del(pos);
			edgx_frer_seqenc_applydel(f, port, l_se->strhdl,
						  se_rbt->port_mask);
		}
	}

	f->se_ports[port].active = ENC_UNDEFINED;

	return 0;
}

static int edgx_frer_seqenc_delstr(u16 str, u16 port, struct edgx_frer *f)
{
	struct list_head *tmp;
	struct list_head *pos;
	struct _se_strlist *l_se;
	struct _se_streams_rbt *se_rbt;
	u8 entry_exists = 0;
	u16 portmask = 0xff;

	if (!list_empty(&f->se_ports[port].se_list)) {
		list_for_each_safe(pos, tmp, &f->se_ports[port].se_list) {
			l_se = list_entry(pos, struct _se_strlist, entry);
			if (str == l_se->strhdl) {
				se_rbt = _seqenc_rbt_search(&f->se_root, str);
				if (se_rbt) {
					se_rbt->port_mask &= ~BIT(port);
					portmask = se_rbt->port_mask;
					if (!se_rbt->port_mask) {
						_seqenc_del_str_from_rbt(str,
									 f);
					}
				}
				list_del(pos);
				entry_exists = 1;
				break;
			}
		}
	}
	if (list_empty(&f->se_ports[port].se_list))
		f->se_ports[port].active = ENC_UNDEFINED;

	if (!entry_exists)
		return -ENOENT;

	edgx_frer_seqenc_applydel(f, port, str, portmask);

	return 0;
}

static int edgx_frer_seqenc_str2port(u16 str, struct edgx_frer *f, u16 *mask)
{
	struct _se_streams_rbt *rbt;

	rbt = _seqenc_rbt_search(&f->se_root, str);

	if (rbt)
		*mask = rbt->port_mask;
	else
		return -ENOENT;
	return 0;
}

int _seqrec_check_dir(u8 dir, struct edgx_frer *f)
{
	if (dir != (u8)DIR_IN_FAC) {
		edgx_br_err(f->parent,
			    "SeqRec:Only in-facing(0) direction supported\n");
	} else {
		return 0;
	}
	return -EINVAL;
}

int _seqrec_check_bool(u8 val, struct edgx_frer *f)
{
	if (val > 0x1) {
		edgx_br_err(f->parent,
			    "SeqRec:Variable can be set to either True(1) or False(0)\n");
	} else {
		return 0;
	}
	return -EINVAL;
}

int _seqrec_check_rcv_rst(u16 rcvrst, struct edgx_frer *f)
{
	if (rcvrst > f->seqrec_rst[0]) {
		edgx_br_err(f->parent,
			    "SeqRec:Maximum value of Recovery reset is %d\n",
			    f->seqrec_rst[0]);
	} else {
		return 0;
	}
	return -EINVAL;
}

int _seqrec_check_histl(u8 histl, struct edgx_frer *f)
{
	if (histl > _SEQREC_MAX_HISTL || histl < _SEQREC_MIN_HISTL) {
		edgx_br_err(f->parent,
			    "SeqRec:History length in range [%d, %d]\n",
			    _SEQREC_MIN_HISTL, _SEQREC_MAX_HISTL);
	} else {
		return 0;
	}
	return -EINVAL;
}

int _seqrec_check_alg(u8 alg, struct edgx_frer *f)
{
	if (alg > ALG_MATCH) {
		edgx_br_err(f->parent,
			    "SeqRec:Algorithm can be either Vector(0) or Match(1) alg = %d\n",
			    alg);
	} else {
		return 0;
	}

	return -EINVAL;
}

void _seqrec_rbt_delnode(struct edgx_frer *f, struct _seqrec_rbt *seqrec)
{
	rb_erase(&seqrec->rbnode, &f->sr_root);
	RB_CLEAR_NODE(&seqrec->rbnode);
	kfree(seqrec);
	f->seqreccnt--;
}

static struct _seqrec_rbt *_seqrec_rbt_search(struct rb_root *root, u16 key)
{
	struct rb_node *node = root->rb_node;

	while (node) {
		struct _seqrec_rbt *srec = rb_entry(node, struct _seqrec_rbt,
						     rbnode);
		if (key < srec->fnidx)
			node = node->rb_left;
		else if (key > srec->fnidx)
			node = node->rb_right;
		else
			return srec;
	}

	return NULL;
}

static int _seqrec_rbt_insert_node(struct _seqrec_rbt *new,
				   struct edgx_frer *f)
{
	struct _seqrec_rbt *srec;
	struct rb_node **pp, *p;

	pp = &f->sr_root.rb_node;
	p = NULL;

	while (*pp) {
		p = *pp;
		srec = rb_entry(p, struct _seqrec_rbt, rbnode);
		if (new->fnidx < srec->fnidx)
			pp = &(*pp)->rb_left;
		else if (new->fnidx > srec->fnidx)
			pp = &(*pp)->rb_right;
		else
			return -EINVAL;
	}

	rb_link_node(&new->rbnode, p, pp);
	rb_insert_color(&new->rbnode, &f->sr_root);

	return 0;
}

static int _seqrec_list_entry_exists(u16 strhdl, struct _seqrec_rbt *gen)
{
	struct list_head *pos;
	struct strlist *l;

	list_for_each(pos, &gen->sr_list) {
		l = list_entry(pos, struct strlist, entry);
		if (l->strhdl == strhdl)
			return 1;
	}

	return 0;
}

static inline int _seqrec_list_add(u16 strhdl, struct _seqrec_rbt *gen)
{
	gen->l = kzalloc(sizeof(*gen->l), GFP_KERNEL);
	if (!gen->l)
		return -ENOMEM;
	gen->l->strhdl = strhdl;
	list_add_tail(&gen->l->entry, &gen->sr_list);

	return 0;
}

static inline int _seqrec_list_del(u16 strhdl, struct _seqrec_rbt *rbt)
{
	struct list_head *pos;
	struct list_head *tmp;
	struct strlist *l;

	list_for_each_safe(pos, tmp, &rbt->sr_list) {
		l = list_entry(pos, struct strlist, entry);
		if (strhdl == l->strhdl) {
			list_del(pos);
			kfree(l);
			return 0;
		}
	}

	return -ENOENT;
}

static int _seqrec_fn_exists(struct edgx_frer *f, u16 fnidx,
			     struct _seqrec_rbt **seqrec)
{
	*seqrec = _seqrec_rbt_search(&f->sr_root, fnidx);
	if (!*seqrec) {
		edgx_br_err(f->parent, "SeqRecfn doesn't exist\n");
		return 0;
	}

	return -EEXIST;
}

static int _seqrec_all_set(struct _seqrec_rbt *sr)
{
	//History length not checked - it is at least set to default value = 2
	if (sr->alg != ALG_UNKNOWN &&
	    sr->rcv_rst != FRER_U16_INVALID &&
	    sr->ptmask != 0 && //used for checking, writing to ip has no effect
	    sr->ind_rec != FRER_BOOL_INVALID &&
	    sr->lat_err != FRER_BOOL_INVALID &&
	    sr->take_no_seq != FRER_BOOL_INVALID &&
	    sr->dir != DIR_UNKNOWN &&
	    !list_empty(&sr->sr_list))
		return 1;

	return 0;
}

static u16 _seqrec_calc_cfg_val(struct _seqrec_rbt *sr, u8 aging_rate)
{
	return (sr->hist_len & 0x1F) |
	       ((sr->alg & 0x1)		<< _SEQREC_SHIFT_ALG) |
	       ((sr->ind_rec & 0x1)	<< _SEQREC_SHIFT_INDREC) |
	       ((sr->take_no_seq & 0x1)	<< _SEQREC_SHIFT_TNS) |
	       ((aging_rate & 0xf)	<< _SEQREC_SHIFT_BRATE);
	//rate = basic rate | (rate scale << 2), last 4 bits
}

int _seqrec_write_tbl_err(struct edgx_frer *f, u16 str, u16 fnidx)
{
	u16 reg;

	if (edgx_frer_seqfn_enabled(f, str, FRER_TBL3)) {
		reg = edgx_frer_ip_rd_strtbl(f, str, FRER_TBL3);
		if ((reg & 0xFFF) != fnidx) {
			edgx_br_err(f->parent,
				    "Stream handle %d assigned to seqRecFn %d\n",
				    str, (reg & 0xFFF));
			return -EBUSY;
		}
	}

	return 0;
}

/*
 * Set configuration of the SeqRec function and assign Streams to it. When
 * deleting, SeqRec function is disabled for certain streams - it can happen
 * that it is configured but disabled for all streams. Configuration can be
 * overwritten at any time all parameters for SeqRecFn are set
 * FRER_U16_INVALID is used when there is no change in stream config (no stream
 * added), so only parameter setting is updated (_SEQ_REC_TBL)
 */
static void edgx_frer_seqrec_apply(struct edgx_frer *f, struct _seqrec_rbt *sr,
				   u16 str, u8 del)
{
	u16 val;
	u8 aging_rate = 0;
	struct list_head *pos;
	struct strlist *l;

	if (_seqrec_all_set(sr) || del) {
		/* All parameters set, write config to IP(_SEQ_REC_TBL) */
		aging_rate = first_bigger(f, sr->rcv_rst);//FLEXDE-3139
		if (!sr->seqrec_cfg_applied) {
			val = _seqrec_calc_cfg_val(sr, aging_rate);
			edgx_frer_ip_wr_seqrectbl(f, sr->fnidx, val);
			/*
			 * First time when all parameters are set, all streams
			 * from stream list are written to the IP
			 */
			list_for_each(pos, &sr->sr_list) {
				l = list_entry(pos, struct strlist, entry);
				if (!_seqrec_write_tbl_err(f, str, sr->fnidx))
					edgx_frer_ip_wr_strtbl(f, l->strhdl,
							       sr->fnidx,
							       FRER_TBL3, del);
			}

			sr->seqrec_cfg_applied = 1;

		} else if (str != FRER_U16_INVALID) {
			/*Only update one stream handle*/
			if (!_seqrec_write_tbl_err(f, str, sr->fnidx))
				edgx_frer_ip_wr_strtbl(f, str, sr->fnidx,
						       FRER_TBL3, del);
		} else {
			/*Only config update*/
			val = _seqrec_calc_cfg_val(sr, aging_rate);
			edgx_frer_ip_wr_seqrectbl(f, sr->fnidx, val);
		}
	}
}

static int edgx_frer_seqrec_addport(u16 ptid, u16 fnidx, struct edgx_frer *f)
{
	struct _seqrec_rbt *seqrec;

	if (edgx_frer_check_port(ptid, f))
		return -EINVAL;

	if (_seqrec_fn_exists(f, fnidx, &seqrec)) {
		seqrec->ptmask |= BIT(ptid);
		edgx_frer_seqrec_apply(f, seqrec, FRER_U16_INVALID, 0);
	} else {
		return -ENOENT;
	}
	return 0;
}

static int edgx_frer_seqrec_delport(u16 ptid, u16 fnidx, struct edgx_frer *f)
{
	struct _seqrec_rbt *sr;

	if (edgx_frer_check_port(ptid, f))
		return -EINVAL;

	if (_seqrec_fn_exists(f, fnidx, &sr)) {
		sr->ptmask  &= ~BIT(ptid);
	} else {
		return -ENOENT;
	}
	return 0;
}

static int edgx_frer_seqrec_delstr(struct edgx_frer *f, u16 str, u16 fn)
{
	struct _seqrec_rbt *sr;

	if (edgx_frer_check_strhdl(str, f))
		return -EINVAL;

	if (_seqrec_fn_exists(f, fn, &sr) && _seqrec_list_entry_exists(str, sr))
		if (!_seqrec_list_del(str, sr)) {
			/*Also delete data for this str the IP*/
			edgx_frer_seqrec_apply(f, sr, str, FRER_DEL);
			return 0;
		}
	return -ENOENT;
}

static int edgx_frer_seqrec_delfn(u16 fnidx, struct edgx_frer *f)
{
	struct _seqrec_rbt *sr;
	struct strlist *l;
	struct list_head *tmp;
	struct list_head *pos;

	if (_seqrec_fn_exists(f, fnidx, &sr)) {
		/*Delete all stream handles from the list assigned to rbtree*/
		if (!list_empty(&sr->sr_list)) {
			list_for_each_safe(pos, tmp, &sr->sr_list) {
				l = list_entry(pos, struct strlist, entry);
				/*Delete from the IP*/
				edgx_frer_seqrec_apply(f, sr, l->strhdl,
						       FRER_DEL);
				list_del(pos);
				kfree(l);
			}
		}

		/*Delete entry from rb-tree*/
		_seqrec_rbt_delnode(f, sr);
		return 0;
	}
	return -ENOENT;
}

static int edgx_frer_seqrec_set_rbt_bool(struct edgx_frer *f, u8 val, u16 fnidx,
					 int field)
{
	struct _seqrec_rbt *seqrec;

	/*val can only be 0 or 1*/
	if (_seqrec_check_bool(val, f))
		return -EINVAL;

	if (_seqrec_fn_exists(f, fnidx, &seqrec)) {
		switch (field) {
		case _SEQREC_NO_SEQ:
			seqrec->take_no_seq = val;
			break;
		case _SEQREC_IND_REC:
			/*Can be set to True only if lat_err = 0*/
			if (!val || (val && !seqrec->lat_err))
				seqrec->ind_rec = val;
			else {
				edgx_br_warn(f->parent,
					     "Individual Recovery can be set to True(1) only if Latent Error is False(0)\n");
				return -EINVAL;
			}
			break;
		case _SEQREC_LAT_ERR:
			if (!val || (seqrec->ind_rec == !val))
				seqrec->lat_err = val;
			else {
				edgx_br_warn(f->parent,
					     "Latent Error can be set to True(1) only if Individual Recovery is False(0)\n");
				return -EINVAL;
			}
			break;
		default:
			return -EINVAL;
		}
		edgx_frer_seqrec_apply(f, seqrec, FRER_U16_INVALID, 0);
		return 0;
	} else {
		return -ENOENT;
	}
	return -EINVAL;
}

static int edgx_frer_seqrec_set_alg(struct edgx_frer *f, u32 alg, u16 fnidx)
{
	struct _seqrec_rbt *seqrec;

	if (_seqrec_check_alg(alg, f))
		return -EINVAL;

	if (_seqrec_fn_exists(f, fnidx, &seqrec))
		seqrec->alg = (enum _seqrec_alg)alg;
	else
		return -ENOENT;

	edgx_frer_seqrec_apply(f, seqrec, FRER_U16_INVALID, 0);
	return 0;
}

static int edgx_frer_seqrec_set_rrst(struct edgx_frer *f, u16 rrst, u16 fnidx)
{
	struct _seqrec_rbt *seqrec;

	if (_seqrec_check_rcv_rst(rrst, f))
		return -EINVAL;

	if (_seqrec_fn_exists(f, fnidx, &seqrec))
		seqrec->rcv_rst = rrst;
	else
		return -ENOENT;

	edgx_frer_seqrec_apply(f, seqrec, FRER_U16_INVALID, 0);
	return 0;
}

static int edgx_frer_seqrec_set_histl(struct edgx_frer *f, u8 hl, u16 fnidx)
{
	struct _seqrec_rbt *seqrec;

	if (_seqrec_check_histl(hl, f))
		return -EINVAL;

	if (_seqrec_fn_exists(f, fnidx, &seqrec))
		seqrec->hist_len = hl;
	else
		return -ENOENT;

	edgx_frer_seqrec_apply(f, seqrec, FRER_U16_INVALID, 0);
	return 0;
}

static int edgx_frer_seqrec_addstr(struct edgx_frer *f, u16 str, u16 fnidx)
{
	struct _seqrec_rbt *seqrec;

	if (edgx_frer_check_strhdl(str, f))
		return -EINVAL;

	seqrec = _seqrec_rbt_search(&f->sr_root, fnidx);
	if (!seqrec) {
		edgx_br_err(f->parent, "AddStr:SeqRecfn doesn't exist\n");
		return -ENOENT;
	}

	if (!_seqrec_list_entry_exists(str, seqrec))
		if (_seqrec_list_add(str, seqrec))
			return -ENOMEM;

	edgx_frer_seqrec_apply(f, seqrec, str, 0);

	return 0;
}

static int edgx_frer_seqrec_adddir(struct edgx_frer *f, u8 dir, u16 fnidx)
{
	struct _seqrec_rbt *seqrec;

	if (_seqrec_check_dir(dir, f))
		return -EINVAL;

	seqrec = _seqrec_rbt_search(&f->sr_root, fnidx);
	if (!seqrec) {
		edgx_br_err(f->parent, "AddDir:SeqRecfn doesn't exist\n");
		return -ENOENT;
	}
	seqrec->dir = dir;
	edgx_frer_seqrec_apply(f, seqrec, FRER_U16_INVALID, 0);

	return 0;
}

static int edgx_frer_seqrec_create(struct edgx_frer *f, u16 fnidx)
{
	struct _seqrec_rbt *rbt_gen;

	if (edgx_frer_check_fnidx(fnidx, f))
		return -EINVAL;

	rbt_gen = _seqrec_rbt_search(&f->sr_root, fnidx);
	if (rbt_gen) {
		edgx_br_err(f->parent,
			    "SeqRec create: entry %d already exists\n",
			    fnidx);
		return -EBUSY;
	}
	rbt_gen = kzalloc(sizeof(*rbt_gen), GFP_KERNEL);
	if (!rbt_gen)
		return -ENOMEM;

	rbt_gen->fnidx = fnidx;
	rbt_gen->dir = (u8)DIR_UNKNOWN;
	rbt_gen->alg = ALG_UNKNOWN;
	rbt_gen->hist_len = _SEQREC_MIN_HISTL; //default value and min value
	rbt_gen->rcv_rst = FRER_U16_INVALID;
	rbt_gen->ptmask = 0;
	rbt_gen->ind_rec = FRER_BOOL_INVALID;
	rbt_gen->lat_err = FRER_BOOL_INVALID;
	rbt_gen->take_no_seq = FRER_BOOL_INVALID;
	rbt_gen->seqrec_cfg_applied = 0;
	INIT_LIST_HEAD(&rbt_gen->sr_list);
	if (_seqrec_rbt_insert_node(rbt_gen, f))
		return -EINVAL;

	f->seqreccnt++;

	return 0;
}

static bool is_seqrec_strhdl_used(struct edgx_frer *f, u16 strhdl, ptid_t ptid)
{
	struct rb_node *nd;
	struct _seqrec_rbt *rec;

	for (nd = rb_first(&f->sr_root); nd; nd = rb_next(nd)) {
		rec = rb_entry(nd, struct _seqrec_rbt, rbnode);
		if (rec->seqrec_cfg_applied &&
		    (rec->ptmask & BIT(ptid)) &&
		     _seqrec_list_entry_exists(strhdl, rec))
			return true;
	}
	edgx_br_err(f->parent, "Invalid stream handle or FN_IDX not in use!\n");
	return false;
}

/****************************** Sysfs functions ******************************/

static ssize_t seqgen_del_str_store(struct device *dev,
				    struct device_attribute *attr,
				    const char *buf, size_t count)
{
	struct edgx_br *br = edgx_dev2br(dev);
	struct edgx_frer *f = edgx_br_get_frer(br);
	u16 strhdl;
	int ret;

	if (kstrtou16(buf, 0, &strhdl))
		return -EINVAL;

	mutex_lock(&f->lock);
	ret = _seqgen_list_del_entry(f, strhdl);
	mutex_unlock(&f->lock);

	return ret ? ret : count;
}

static ssize_t seqgen_del_fn_store(struct device *dev,
				   struct device_attribute *attr,
				   const char *buf, size_t count)
{
	struct edgx_br *br = edgx_dev2br(dev);
	struct edgx_frer *f = edgx_br_get_frer(br);
	u16 key;
	int ret;

	if (kstrtou16(buf, 0, &key))
		return -EINVAL;

	mutex_lock(&f->lock);
	ret = _seqgen_rbt_del_entry(f, key);
	mutex_unlock(&f->lock);

	return ret ? ret : count;
}

static ssize_t seqgen_create_store(struct device *dev,
				   struct device_attribute *attr,
				   const char *buf,
				   size_t count)
{
	struct edgx_br *br = edgx_dev2br(dev);
	struct edgx_frer *f = edgx_br_get_frer(br);
	int ret;
	u16 fnidx;

	if (kstrtou16(buf, 0, &fnidx))
		return -EINVAL;

	mutex_lock(&f->lock);
	ret = edgx_frer_seqgen_create(f, fnidx);
	mutex_unlock(&f->lock);

	return ret ? ret : count;
}

static ssize_t seqgen_add_dir_write(struct file *filp, struct kobject *kobj,
				    struct bin_attribute *bin_attr,
				    char *buf, loff_t ofs, size_t count)
{
	struct edgx_br *br = edgx_dev2br(kobj_to_dev(kobj));
	struct edgx_frer *f = edgx_br_get_frer(br);
	loff_t idx = 0;
	u8 dir;
	int ret;

	if (edgx_sysfs_tbl_params(ofs, count, sizeof(u8), &idx) ||
	    idx >= f->seqrec_max)
		return -EINVAL;

	dir = *(u8 *)buf;
	mutex_lock(&f->lock);
	ret = edgx_frer_seqgen_adddir(f, dir, idx);
	mutex_unlock(&f->lock);

	return ret ? ret : count;
}

static ssize_t seqgen_add_str_write(struct file *filp, struct kobject *kobj,
				    struct bin_attribute *bin_attr,
				    char *buf, loff_t ofs, size_t count)
{
	struct edgx_br *br = edgx_dev2br(kobj_to_dev(kobj));
	struct edgx_frer *f = edgx_br_get_frer(br);
	loff_t idx = 0;
	u16 str;
	int ret;

	if (edgx_sysfs_tbl_params(ofs, count, sizeof(u16), &idx) ||
	    idx >= f->seqrec_max)
		return -EINVAL;

	str = *(u16 *)buf;

	mutex_lock(&f->lock);
	ret = edgx_frer_seqgen_addstr(f, str, idx);
	mutex_unlock(&f->lock);

	return ret ? ret : count;
}

static ssize_t seqgen_list_all_fn_read(struct file *filp, struct kobject *kobj,
				       struct bin_attribute *bin_attr,
				       char *buf, loff_t ofs, size_t count)
{
	struct edgx_br *br = edgx_dev2br(kobj_to_dev(kobj));
	struct edgx_frer *f = edgx_br_get_frer(br);
	u16 *fn_list = (u16 *)buf;
	loff_t idx = 0;
	size_t nelems = 0;
	size_t elem = 0;
	size_t i = 0;
	struct rb_node *nd;
	struct _seqgen_rbt *n;

	if (edgx_sysfs_list_params(ofs, count, sizeof(*fn_list),
				   &idx, &nelems) ||
	    idx + nelems > f->seqrec_max)
		return -EINVAL;

	mutex_lock(&f->lock);
	for (nd = rb_first(&f->sg_root); nd; nd = rb_next(nd)) {
		i++;
		if (i > idx + nelems)
			break;
		if (i <= idx)
			continue;
		n = rb_entry(nd, struct _seqgen_rbt, rbnode);
		fn_list[elem++] = n->fnidx;
	}
	mutex_unlock(&f->lock);

	return elem * sizeof(*fn_list);
}

static ssize_t seqgen_cnt_show(struct device *dev,
			       struct device_attribute *attr,
			       char *buf)
{
	struct edgx_frer *f = edgx_br_get_frer(edgx_dev2br(dev));
	int ret = 0;

	mutex_lock(&f->lock);
	ret = scnprintf(buf, PAGE_SIZE, "%u\n", f->seqgencnt);
	mutex_unlock(&f->lock);

	return ret;
}

static ssize_t seqgen_sid2fn_read(struct file *filp, struct kobject *kobj,
				  struct bin_attribute *bin_attr,
				  char *buf, loff_t ofs, size_t count)
{
	u16 val;
	loff_t idx = 0;
	int ret = -ENOENT;
	struct edgx_br *br = edgx_dev2br(kobj_to_dev(kobj));
	struct edgx_frer *f = edgx_br_get_frer(br);
	struct rb_node *n;
	struct strlist *l;
	struct list_head *pos;
	struct _seqgen_rbt *sg;

	if (edgx_sysfs_tbl_params(ofs, count, sizeof(u16), &idx) ||
	    idx >= f->streams_max)
		return -EINVAL;

	mutex_lock(&f->lock);
	val = edgx_frer_ip_rd_strtbl(f, (u16)idx, FRER_TBL2);
	/*SeqGen is enabled in the IP, no need to go through the RBT*/
	if (val & BIT(15)) {
		((u16 *)buf)[0] = val & 0xFFF;
		ret = 0;
	} else {
		for (n = rb_first(&f->sg_root); n; n = rb_next(n)) {
			sg = rb_entry(n, struct _seqgen_rbt, rbnode);
			list_for_each(pos, &sg->sg_list) {
				l = list_entry(pos, struct strlist, entry);
				if (l->strhdl == (u16)idx) {
					((u16 *)buf)[0] = sg->fnidx;
					ret = 0;
					break;
				}
			}
		}
	}
	mutex_unlock(&f->lock);

	return ret ? ret : count;
}

static ssize_t seqgen_fn2dir_read(struct file *filp, struct kobject *kobj,
				  struct bin_attribute *bin_attr,
				  char *buf, loff_t ofs, size_t count)
{
	loff_t idx = 0;
	struct edgx_br *br = edgx_dev2br(kobj_to_dev(kobj));
	struct edgx_frer *f = edgx_br_get_frer(br);

	if (edgx_sysfs_tbl_params(ofs, count, sizeof(u8), &idx) ||
	    idx >= f->seqrec_max)
		return -EINVAL;

	mutex_lock(&f->lock);
	/*No need to go through the tree since direction is always in facing*/
	((u8 *)buf)[0] = (u8)DIR_IN_FAC;

	mutex_unlock(&f->lock);

	return count;
}

static ssize_t seqgen_fn2sid_read(struct file *filp, struct kobject *kobj,
				  struct bin_attribute *bin_attr,
				  char *buf, loff_t ofs, size_t count)
{
	struct edgx_br *br = edgx_dev2br(kobj_to_dev(kobj));
	struct edgx_frer *f = edgx_br_get_frer(br);
	u16 *list = (u16 *)buf;
	struct _seqgen_rbt *n;
	struct list_head *pos;
	loff_t idx = 0;
	size_t nelems = 0;
	size_t elem = 0;
	size_t fnidx = 0;
	size_t stridx = 0;
	size_t i = 0;
	struct strlist *l;
	int ret = 0;

	if (f->streams_max == 0)
		return -ENOENT;

	ret = edgx_sysfs_list_params(ofs, count, sizeof(*list), &idx, &nelems);
	if (ret)
		return ret;
	fnidx = (size_t)idx / f->streams_max;
	stridx = (size_t)idx % f->streams_max;
	if (fnidx >= f->seqrec_max || stridx + nelems > f->streams_max)
		return -EINVAL;

	mutex_lock(&f->lock);
	/* idx is the rbt key - fnidx */
	n = _seqgen_rbt_search(&f->sg_root, (u16)fnidx);
	if (n) {
		list_for_each(pos, &n->sg_list) {
			i++;
			if (i > stridx + nelems)
				break;
			if (i <= stridx)
				continue;
			l = list_entry(pos, struct strlist, entry);
			list[elem++] = l->strhdl;
		}
		while (elem < nelems)
			list[elem++] = FRER_U16_INVALID;
	} else {
		edgx_br_err(f->parent,
			    "ShowFn2Sid:SeqGenFn %lld doesn't exist\n",
			    idx);
		ret = -ENOENT;
	}
	mutex_unlock(&f->lock);

	return ret ? ret : count;
}

static ssize_t seqenc_create_store(struct device *dev,
				   struct device_attribute *attr,
				   const char *buf, size_t count)
{
	struct edgx_pt *pt = edgx_dev2pt(dev);
	struct edgx_frer *f = edgx_br_get_frer(edgx_pt_get_br(pt));
	int ret;
	u8 dir;

	if (kstrtou8(buf, 0, &dir))
		return -EINVAL;

	mutex_lock(&f->lock);
	ret = edgx_frer_seqenc_create_entry((u16)edgx_pt_get_id(pt), dir, f);
	mutex_unlock(&f->lock);

	return ret ? ret : count;
}

static ssize_t seqenc_create_show(struct device *dev,
				  struct device_attribute *attr,
				  char *buf)
{
	struct edgx_pt *pt = edgx_dev2pt(dev);
	struct edgx_frer *f = edgx_br_get_frer(edgx_pt_get_br(pt));
	int ret;

	/* direction is not stored in data, as only DIR_OUT_FAC is possible,
	 * but it has to be checked if a seqenc entry was created for the port.
	 * For the check the active attribute is used. If active attribute
	 * equal ENC_UNDEFINED, no seqenc entry exists.
	 */
	mutex_lock(&f->lock);
	if (f->se_ports[edgx_pt_get_id(pt)].active == ENC_UNDEFINED)
		ret = scnprintf(buf, PAGE_SIZE, "%d\n", DIR_UNKNOWN);
	else
		ret = scnprintf(buf, PAGE_SIZE, "%d\n", DIR_OUT_FAC);
	mutex_unlock(&f->lock);
	return ret;
}

static ssize_t seqenc_active_store(struct device *dev,
				   struct device_attribute *attr,
				   const char *buf, size_t count)
{
	struct edgx_pt *pt = edgx_dev2pt(dev);
	struct edgx_frer *f = edgx_br_get_frer(edgx_pt_get_br(pt));
	int ret;
	u8 active;

	if (kstrtou8(buf, 0, &active))
		return -EINVAL;

	mutex_lock(&f->lock);
	ret = edgx_frer_seqenc_set_active(active, (u16)edgx_pt_get_id(pt), f);
	mutex_unlock(&f->lock);

	return ret ? ret : count;
}

static ssize_t seqenc_active_show(struct device *dev,
				  struct device_attribute *attr,
				  char *buf)
{
	struct edgx_pt *pt = edgx_dev2pt(dev);
	struct edgx_frer *f = edgx_br_get_frer(edgx_pt_get_br(pt));
	int ret;

	mutex_lock(&f->lock);
	ret = scnprintf(buf, PAGE_SIZE, "%d\n",
			f->se_ports[edgx_pt_get_id(pt)].active);
	mutex_unlock(&f->lock);

	return ret;
}

static ssize_t seqenc_type_store(struct device *dev,
				 struct device_attribute *attr,
				 const char *buf,
				 size_t count)
{
	struct edgx_pt *pt = edgx_dev2pt(dev);
	struct edgx_frer *f = edgx_br_get_frer(edgx_pt_get_br(pt));
	int ret = 0;
	u8 type;

	if (kstrtou8(buf, 0, &type))
		return -EINVAL;

	/* As only type TYPE_R_TAG(1) is supported, the value isn't stored.
	 * But checks have to be done.
	 */
	if (f->se_ports[(u16)edgx_pt_get_id(pt)].active == ENC_UNDEFINED) {
		edgx_br_err(f->parent,
			    "SetType: SeqEnc entry doesn't exist\n");
		return -ENOENT;
	} else if (_seqenc_check_type(type, f)) {
		edgx_br_err(f->parent,
			    "SetType value incorrect, only TYPE_R_TAG(1) supported.\n");
		return -EINVAL;
	}

	return ret ? ret : count;
}

static ssize_t seqenc_type_show(struct device *dev,
				struct device_attribute *attr,
				char *buf)
{
	struct edgx_pt *pt = edgx_dev2pt(dev);
	struct edgx_frer *f = edgx_br_get_frer(edgx_pt_get_br(pt));
	int ret;

	if (f->se_ports[(u16)edgx_pt_get_id(pt)].active == ENC_UNDEFINED) {
		ret = scnprintf(buf, PAGE_SIZE, "%d\n", TYPE_UNKNOWN);
	} else {
		/* As only R-tag encapsulation is supported, type isn't stored.
		 * The constant value can be returned here.
		 */
		ret = scnprintf(buf, PAGE_SIZE, "%d\n", TYPE_R_TAG);
	}

	return ret;
}

static ssize_t get_numofports_show(struct device *dev,
				   struct device_attribute *attr,
				   char *buf)
{
	struct edgx_br *br = edgx_dev2br(dev);
	struct edgx_frer *f = edgx_br_get_frer(br);
	int ret;

	mutex_lock(&f->lock);
	ret = scnprintf(buf, PAGE_SIZE, "%d\n",
			edgx_br_get_num_ports(f->parent));
	mutex_unlock(&f->lock);

	return ret;
}

static ssize_t seqenc_add_str_store(struct device *dev,
				    struct device_attribute *attr,
				    const char *buf, size_t count)
{
	struct edgx_pt *pt = edgx_dev2pt(dev);
	struct edgx_frer *f = edgx_br_get_frer(edgx_pt_get_br(pt));
	u16 str;
	int ret;

	if (kstrtou16(buf, 0, &str))
		return -EINVAL;

	mutex_lock(&f->lock);
	ret = edgx_frer_seqenc_add_str(str, (u16)edgx_pt_get_id(pt), f);
	mutex_unlock(&f->lock);

	return ret ? ret : count;
}

static ssize_t seqenc_delportfn_store(struct device *dev,
				      struct device_attribute *attr,
				      const char *buf, size_t count)
{
	struct edgx_pt *pt = edgx_dev2pt(dev);
	struct edgx_frer *f = edgx_br_get_frer(edgx_pt_get_br(pt));
	int ret;

	mutex_lock(&f->lock);
	ret = edgx_frer_seqenc_delportfn((u16)edgx_pt_get_id(pt), f);
	mutex_unlock(&f->lock);

	return ret ? ret : count;
}

static ssize_t seqenc_delstr_store(struct device *dev,
				   struct device_attribute *attr,
				   const char *buf, size_t count)
{
	struct edgx_pt *pt = edgx_dev2pt(dev);
	struct edgx_frer *f = edgx_br_get_frer(edgx_pt_get_br(pt));
	int ret;
	u16 str;

	if (kstrtou16(buf, 0, &str))
		return -EINVAL;

	mutex_lock(&f->lock);
	ret = edgx_frer_seqenc_delstr(str, (u16)edgx_pt_get_id(pt), f);
	mutex_unlock(&f->lock);

	return ret ? ret : count;
}

static ssize_t seqenc_str2port_read(struct file *filp, struct kobject *kobj,
				    struct bin_attribute *bin_attr,
				    char *buf, loff_t ofs, size_t count)
{
	struct edgx_br *br = edgx_dev2br(kobj_to_dev(kobj));
	struct edgx_frer *f = edgx_br_get_frer(br);
	loff_t idx = 0;
	int ret;
	u16 mask;

	if (edgx_sysfs_tbl_params(ofs, count, sizeof(u16), &idx) ||
	    idx >= f->streams_max)
		return -EINVAL;

	mutex_lock(&f->lock);
	ret = edgx_frer_seqenc_str2port((u16)idx, f, &mask);

	if (!ret)
		((u16 *)buf)[0] =  mask;

	mutex_unlock(&f->lock);

	return ret ? ret : count;
}

static ssize_t seqenc_port2str_read(struct file *filp, struct kobject *kobj,
				    struct bin_attribute *bin_attr,
				    char *buf, loff_t ofs, size_t count)
{
	struct edgx_pt *pt = edgx_dev2pt(kobj_to_dev(kobj));
	struct edgx_frer *f = edgx_br_get_frer(edgx_pt_get_br(pt));
	ptid_t ptid = edgx_pt_get_id(pt);
	u16 *str = (u16 *)buf;
	loff_t idx = 0;
	size_t nelems = 0;
	size_t elem = 0;
	size_t i = 0;
	struct list_head *pos;
	struct _se_strlist *l;

	if (edgx_sysfs_list_params(ofs, count, sizeof(*str), &idx, &nelems) ||
	    idx + nelems > f->streams_max)
		return -EINVAL;

	mutex_lock(&f->lock);
	list_for_each(pos, &f->se_ports[ptid].se_list) {
		i++;
		if (i > idx + nelems)
			break;
		if (i <= idx)
			continue;
		l = list_entry(pos, struct _se_strlist, entry);
		str[elem++] = l->strhdl;
	}
	while (elem < nelems)
		str[elem++] = FRER_U16_INVALID;

	mutex_unlock(&f->lock);

	return elem * sizeof(u16);
}

static ssize_t seqrec_create_store(struct device *dev,
				   struct device_attribute *attr,
				   const char *buf, size_t count)
{
	struct edgx_br *br = edgx_dev2br(dev);
	struct edgx_frer *f = edgx_br_get_frer(br);
	u16 key;
	int ret;

	if (kstrtou16(buf, 0, &key))
		return -EINVAL;

	mutex_lock(&f->lock);
	ret = edgx_frer_seqrec_create(f, key);
	mutex_unlock(&f->lock);

	return ret ? ret : count;
}

static ssize_t seqrec_dir_write(struct file *filp, struct kobject *kobj,
				struct bin_attribute *bin_attr,
				char *buf, loff_t ofs, size_t count)
{
	struct edgx_br *br = edgx_dev2br(kobj_to_dev(kobj));
	struct edgx_frer *f = edgx_br_get_frer(br);
	loff_t idx = 0;
	u8 dir;
	int ret;

	//idx is seqrec function index
	if (edgx_sysfs_tbl_params(ofs, count, sizeof(u8), &idx) ||
	    idx >= f->seqrec_max)
		return -EINVAL;

	dir = *(u8 *)buf;

	mutex_lock(&f->lock);
	ret = edgx_frer_seqrec_adddir(f, dir, (u16)idx);
	mutex_unlock(&f->lock);

	return ret ? ret : count;
}

static ssize_t seqrec_add_str_write(struct file *filp, struct kobject *kobj,
				    struct bin_attribute *bin_attr,
				    char *buf, loff_t ofs, size_t count)
{
	struct edgx_br *br = edgx_dev2br(kobj_to_dev(kobj));
	struct edgx_frer *f = edgx_br_get_frer(br);
	loff_t idx = 0;
	u16 str;
	int ret;

	if (edgx_sysfs_tbl_params(ofs, count, sizeof(u16), &idx) ||
	    idx >= f->seqrec_max)
		return -EINVAL;

	str = *(u16 *)buf;
	mutex_lock(&f->lock);
	ret = edgx_frer_seqrec_addstr(f, str, (u16)idx);
	mutex_unlock(&f->lock);

	return ret ? ret : count;
}

static ssize_t seqrec_del_str_write(struct file *filp, struct kobject *kobj,
				    struct bin_attribute *bin_attr,
				    char *buf, loff_t ofs, size_t count)
{
	struct edgx_br *br = edgx_dev2br(kobj_to_dev(kobj));
	struct edgx_frer *f = edgx_br_get_frer(br);
	loff_t idx = 0;
	u16 str;
	int ret;

	if (edgx_sysfs_tbl_params(ofs, count, sizeof(u16), &idx) ||
	    idx >= f->seqrec_max)
		return -EINVAL;

	str = *(u16 *)buf;

	mutex_lock(&f->lock);
	ret = edgx_frer_seqrec_delstr(f, str, (u16)idx);
	mutex_unlock(&f->lock);

	return ret ? ret : count;
}

static ssize_t seqrec_alg_write(struct file *filp, struct kobject *kobj,
				struct bin_attribute *bin_attr,
				char *buf, loff_t ofs, size_t count)
{
	struct edgx_br *br = edgx_dev2br(kobj_to_dev(kobj));
	struct edgx_frer *f = edgx_br_get_frer(br);
	loff_t idx = 0;
	u32 alg;
	int ret;

	//idx is seqrec function index
	if (edgx_sysfs_tbl_params(ofs, count, sizeof(u32), &idx) ||
	    idx >= f->seqrec_max)
		return -EINVAL;

	alg = *(u8 *)buf;

	mutex_lock(&f->lock);
	ret = edgx_frer_seqrec_set_alg(f, alg, (u16)idx);
	mutex_unlock(&f->lock);

	return ret ? ret : count;
}

static ssize_t seqrec_histlen_write(struct file *filp, struct kobject *kobj,
				    struct bin_attribute *bin_attr,
				    char *buf, loff_t ofs, size_t count)
{
	struct edgx_br *br = edgx_dev2br(kobj_to_dev(kobj));
	struct edgx_frer *f = edgx_br_get_frer(br);
	loff_t idx = 0;
	u8 hl;
	int ret;

	//idx is seqrec function index
	if (edgx_sysfs_tbl_params(ofs, count, sizeof(u8), &idx) ||
	    idx >= f->seqrec_max)
		return -EINVAL;

	hl = *(u8 *)buf;

	mutex_lock(&f->lock);
	ret = edgx_frer_seqrec_set_histl(f, hl, (u16)idx);
	mutex_unlock(&f->lock);

	return ret ? ret : count;
}

static ssize_t seqrec_rst_timeout_write(struct file *filp, struct kobject *kobj,
					struct bin_attribute *bin_attr,
					char *buf, loff_t ofs, size_t count)
{
	struct edgx_br *br = edgx_dev2br(kobj_to_dev(kobj));
	struct edgx_frer *f = edgx_br_get_frer(br);
	loff_t idx = 0;
	u16 rrst;
	int ret;

	//idx is seqrec function index
	if (edgx_sysfs_tbl_params(ofs, count, sizeof(u16), &idx) ||
	    idx >= f->seqrec_max)
		return -EINVAL;

	rrst = *(u16 *)buf;

	mutex_lock(&f->lock);
	ret = edgx_frer_seqrec_set_rrst(f, rrst, (u16)idx);
	mutex_unlock(&f->lock);

	return ret ? ret : count;
}

static int _seqrec_sysfs_set_bool(loff_t ofs, size_t count, char *buf,
				  int field, struct edgx_frer *f)
{
	loff_t idx = 0;
	u8 val;
	int ret;

	if (edgx_sysfs_tbl_params(ofs, count, sizeof(u8), &idx))
		return -EINVAL;

	val = *(u8 *)buf;

	mutex_lock(&f->lock);
	ret = edgx_frer_seqrec_set_rbt_bool(f, val, (u16)idx, field);
	mutex_unlock(&f->lock);

	return ret ? ret : count;
}

static ssize_t seqrec_take_no_seq_write(struct file *filp, struct kobject *kobj,
					struct bin_attribute *bin_attr,
					char *buf, loff_t ofs, size_t count)
{
	struct edgx_br *br = edgx_dev2br(kobj_to_dev(kobj));
	struct edgx_frer *f = edgx_br_get_frer(br);

	return _seqrec_sysfs_set_bool(ofs, count, buf, _SEQREC_NO_SEQ, f);
}

static ssize_t seqrec_ind_recv_write(struct file *filp, struct kobject *kobj,
				     struct bin_attribute *bin_attr,
				     char *buf, loff_t ofs, size_t count)
{
	struct edgx_br *br = edgx_dev2br(kobj_to_dev(kobj));
	struct edgx_frer *f = edgx_br_get_frer(br);

	return _seqrec_sysfs_set_bool(ofs, count, buf, _SEQREC_IND_REC, f);
}

static ssize_t seqrec_latent_err_write(struct file *filp, struct kobject *kobj,
				       struct bin_attribute *bin_attr,
				       char *buf, loff_t ofs, size_t count)
{
	struct edgx_br *br = edgx_dev2br(kobj_to_dev(kobj));
	struct edgx_frer *f = edgx_br_get_frer(br);

	/* NOTE: FLEXDE-3148: Standard deviation:
	 * Latent error detection is not supported, accept only 0 (false).
	 */
	if (*(u8 *)buf != 0) {
		edgx_br_err(f->parent, "Latent error not supported\n");
		return -EPERM;
	}

	return _seqrec_sysfs_set_bool(ofs, count, buf, _SEQREC_LAT_ERR, f);
}

static ssize_t seqrec_add_pt_store(struct device *dev,
				   struct device_attribute *attr,
				   const char *buf, size_t count)
{
	struct edgx_pt *pt = edgx_dev2pt(dev);
	struct edgx_frer *f = edgx_br_get_frer(edgx_pt_get_br(pt));
	int ret = 0;
	u16 fnidx;

	if (kstrtou16(buf, 0, &fnidx))
		return -EINVAL;

	mutex_lock(&f->lock);
	ret = edgx_frer_seqrec_addport((u16)edgx_pt_get_id(pt), fnidx, f);
	mutex_unlock(&f->lock);

	return ret ? ret : count;
}

static ssize_t seqrec_del_pt_store(struct device *dev,
				   struct device_attribute *attr,
				   const char *buf, size_t count)
{
	struct edgx_pt *pt = edgx_dev2pt(dev);
	struct edgx_frer *f = edgx_br_get_frer(edgx_pt_get_br(pt));
	int ret = 0;
	u16 fnidx;

	if (kstrtou16(buf, 0, &fnidx))
		return -EINVAL;

	mutex_lock(&f->lock);
	ret = edgx_frer_seqrec_delport((u16)edgx_pt_get_id(pt), fnidx, f);
	mutex_unlock(&f->lock);

	return ret ? ret : count;
}

static ssize_t seqrec_del_fn_store(struct device *dev,
				   struct device_attribute *attr,
				   const char *buf, size_t count)
{
	struct edgx_br *br = edgx_dev2br(dev);
	struct edgx_frer *f = edgx_br_get_frer(br);
	u16 key;
	int ret;

	if (kstrtou16(buf, 0, &key))
		return -EINVAL;

	mutex_lock(&f->lock);
	ret = edgx_frer_seqrec_delfn(key, f);
	mutex_unlock(&f->lock);

	return ret ? ret : count;
}

static ssize_t seqrec_fn2sid_read(struct file *filp, struct kobject *kobj,
				  struct bin_attribute *bin_attr,
				  char *buf, loff_t ofs, size_t count)
{
	struct edgx_br *br = edgx_dev2br(kobj_to_dev(kobj));
	struct edgx_frer *f = edgx_br_get_frer(br);
	u16 *list = (u16 *)buf;
	struct _seqrec_rbt *n;
	struct list_head *pos;
	loff_t idx = 0;
	size_t nelems = 0;
	size_t elem = 0;
	size_t fnidx = 0;
	size_t stridx = 0;
	size_t i = 0;
	struct strlist *l;
	int ret = 0;

	if (f->streams_max == 0)
		return -ENOENT;

	ret = edgx_sysfs_list_params(ofs, count, sizeof(*list), &idx, &nelems);
	if (ret)
		return ret;
	fnidx = (size_t)idx / f->streams_max;
	stridx = (size_t)idx % f->streams_max;
	if (fnidx >= f->seqrec_max || stridx + nelems > f->streams_max)
		return -EINVAL;

	mutex_lock(&f->lock);
	/* idx is the rbt key - fnidx */
	n = _seqrec_rbt_search(&f->sr_root, (u16)fnidx);
	if (n) {
		list_for_each(pos, &n->sr_list) {
			i++;
			if (i > stridx + nelems)
				break;
			if (i <= stridx)
				continue;
			l = list_entry(pos, struct strlist, entry);
			list[elem++] = l->strhdl;
		}
		while (elem < nelems)
			list[elem++] = FRER_U16_INVALID;
	} else {
		edgx_br_err(f->parent, "SeqrecFn %lld doesn't exist\n", idx);
		ret = -ENOENT;
	}
	mutex_unlock(&f->lock);
	return ret ? ret : count;
}

static ssize_t seqrec_dir_read(struct file *filp, struct kobject *kobj,
			       struct bin_attribute *bin_attr,
			       char *buf, loff_t ofs, size_t count)
{
	struct edgx_br *br = edgx_dev2br(kobj_to_dev(kobj));
	struct edgx_frer *f = edgx_br_get_frer(br);
	struct _seqrec_rbt *n;
	loff_t idx = 0;
	int ret = 0;

	if (edgx_sysfs_tbl_params(ofs, count, sizeof(u8), &idx) ||
	    idx >= f->seqrec_max)
		return -EINVAL;

	mutex_lock(&f->lock);
	n = _seqrec_rbt_search(&f->sr_root, (u16)idx);
	if (n)
		((u8 *)buf)[0] = n->dir;
	else
		ret = -ENOENT;
	mutex_unlock(&f->lock);

	return ret ? ret : count;
}

static ssize_t seqrec_alg_read(struct file *filp, struct kobject *kobj,
			       struct bin_attribute *bin_attr,
			       char *buf, loff_t ofs, size_t count)
{
	struct edgx_br *br = edgx_dev2br(kobj_to_dev(kobj));
	struct edgx_frer *f = edgx_br_get_frer(br);
	struct _seqrec_rbt *n;
	loff_t idx = 0;
	int ret = 0;

	if (edgx_sysfs_tbl_params(ofs, count, sizeof(u32), &idx) ||
	    idx >= f->seqrec_max)
		return -EINVAL;

	mutex_lock(&f->lock);
	n = _seqrec_rbt_search(&f->sr_root, (u16)idx);
	if (n)
		((u32 *)buf)[0] = n->alg;
	else
		ret = -ENOENT;
	mutex_unlock(&f->lock);

	return ret ? ret : count;
}

static ssize_t seqrec_histlen_read(struct file *filp, struct kobject *kobj,
				   struct bin_attribute *bin_attr,
				   char *buf, loff_t ofs, size_t count)
{
	struct edgx_br *br = edgx_dev2br(kobj_to_dev(kobj));
	struct edgx_frer *f = edgx_br_get_frer(br);
	struct _seqrec_rbt *n;
	loff_t idx = 0;
	int ret = 0;

	if (edgx_sysfs_tbl_params(ofs, count, sizeof(u8), &idx) ||
	    idx >= f->seqrec_max)
		return -EINVAL;

	mutex_lock(&f->lock);
	n = _seqrec_rbt_search(&f->sr_root, (u16)idx);
	if (n)
		((u8 *)buf)[0] = n->hist_len;
	else
		ret = -ENOENT;
	mutex_unlock(&f->lock);

	return ret ? ret : count;
}

static ssize_t seqrec_rst_timeout_read(struct file *filp, struct kobject *kobj,
				       struct bin_attribute *bin_attr,
				       char *buf, loff_t ofs, size_t count)
{
	struct edgx_br *br = edgx_dev2br(kobj_to_dev(kobj));
	struct edgx_frer *f = edgx_br_get_frer(br);
	struct _seqrec_rbt *n;
	loff_t idx = 0;
	int ret = 0;

	if (edgx_sysfs_tbl_params(ofs, count, sizeof(u16), &idx) ||
	    idx >= f->seqrec_max)
		return -EINVAL;

	mutex_lock(&f->lock);
	n = _seqrec_rbt_search(&f->sr_root, (u16)idx);
	if (n)
		((u16 *)buf)[0] = n->rcv_rst;
	else
		ret = -ENOENT;
	mutex_unlock(&f->lock);

	return ret ? ret : count;
}

static ssize_t seqrec_take_no_seq_read(struct file *filp, struct kobject *kobj,
				       struct bin_attribute *bin_attr,
				       char *buf, loff_t ofs, size_t count)
{
	struct edgx_br *br = edgx_dev2br(kobj_to_dev(kobj));
	struct edgx_frer *f = edgx_br_get_frer(br);
	struct _seqrec_rbt *n;
	loff_t idx = 0;
	int ret = 0;

	if (edgx_sysfs_tbl_params(ofs, count, sizeof(u8), &idx) ||
	    idx >= f->seqrec_max)
		return -EINVAL;

	mutex_lock(&f->lock);
	n = _seqrec_rbt_search(&f->sr_root, (u16)idx);
	if (n)
		((u8 *)buf)[0] = n->take_no_seq;
	else
		ret = -ENOENT;
	mutex_unlock(&f->lock);

	return ret ? ret : count;
}

static ssize_t seqrec_ind_recv_read(struct file *filp, struct kobject *kobj,
				    struct bin_attribute *bin_attr,
				    char *buf, loff_t ofs, size_t count)
{
	struct edgx_br *br = edgx_dev2br(kobj_to_dev(kobj));
	struct edgx_frer *f = edgx_br_get_frer(br);
	struct _seqrec_rbt *n;
	loff_t idx = 0;
	int ret = 0;

	if (edgx_sysfs_tbl_params(ofs, count, sizeof(u8), &idx) ||
	    idx >= f->seqrec_max)
		return -EINVAL;

	mutex_lock(&f->lock);
	n = _seqrec_rbt_search(&f->sr_root, (u16)idx);
	if (n)
		((u8 *)buf)[0] = n->ind_rec;
	else
		ret = -ENOENT;
	mutex_unlock(&f->lock);

	return ret ? ret : count;
}

static ssize_t seqrec_latent_err_read(struct file *filp, struct kobject *kobj,
				      struct bin_attribute *bin_attr,
				      char *buf, loff_t ofs, size_t count)
{
	struct edgx_br *br = edgx_dev2br(kobj_to_dev(kobj));
	struct edgx_frer *f = edgx_br_get_frer(br);
	struct _seqrec_rbt *n;
	loff_t idx = 0;
	int ret = 0;

	if (edgx_sysfs_tbl_params(ofs, count, sizeof(u8), &idx) ||
	    idx >= f->seqrec_max)
		return -EINVAL;

	mutex_lock(&f->lock);
	n = _seqrec_rbt_search(&f->sr_root, (u16)idx);
	if (n)
		((u8 *)buf)[0] = n->lat_err;
	else
		ret = -ENOENT;
	mutex_unlock(&f->lock);

	return ret ? ret : count;
}

static ssize_t seqrec_pt_mask_read(struct file *filp, struct kobject *kobj,
				   struct bin_attribute *bin_attr,
				   char *buf, loff_t ofs, size_t count)
{
	struct edgx_br *br = edgx_dev2br(kobj_to_dev(kobj));
	struct edgx_frer *f = edgx_br_get_frer(br);
	struct _seqrec_rbt *n;
	loff_t idx = 0;
	int ret = 0;

	if (edgx_sysfs_tbl_params(ofs, count, sizeof(u16), &idx) ||
	    idx >= f->seqrec_max)
		return -EINVAL;

	mutex_lock(&f->lock);
	n = _seqrec_rbt_search(&f->sr_root, (u16)idx);
	if (n)
	// TODO: Add proper conversion from ptid_t (ptmask) to numbering
	// in phys_port_name used by tsntool.
		((u16 *)buf)[0] = n->ptmask << 1;

	else
		ret = -ENOENT;
	mutex_unlock(&f->lock);

	return ret ? ret : count;
}

static ssize_t seqrec_cnt_show(struct device *dev,
			       struct device_attribute *attr, char *buf)
{
	struct edgx_frer *f = edgx_br_get_frer(edgx_dev2br(dev));
	int ret = 0;

	mutex_lock(&f->lock);
	ret = scnprintf(buf, PAGE_SIZE, "%u\n", f->seqreccnt);
	mutex_unlock(&f->lock);

	return ret;
}

static ssize_t seqrec_invseq_show(struct device *dev,
				  struct device_attribute *attr,
				  char *buf)
{
	return scnprintf(buf, PAGE_SIZE, "%u\n", _SEQREC_INVSEQ);
}

static ssize_t seqrec_list_all_fn_read(struct file *filp, struct kobject *kobj,
				       struct bin_attribute *bin_attr,
				       char *buf, loff_t ofs, size_t count)
{
	struct edgx_br *br = edgx_dev2br(kobj_to_dev(kobj));
	struct edgx_frer *f = edgx_br_get_frer(br);
	u16 *fn_list = (u16 *)buf;
	loff_t idx = 0;
	size_t nelems = 0;
	size_t elem = 0;
	size_t i = 0;
	struct rb_node *nd;
	struct _seqrec_rbt *n;

	if (edgx_sysfs_list_params(ofs, count, sizeof(*fn_list),
				   &idx, &nelems) ||
	    idx + nelems > f->seqrec_max)
		return -EINVAL;

	mutex_lock(&f->lock);

	for (nd = rb_first(&f->sr_root); nd; nd = rb_next(nd)) {
		i++;
		if (i > idx + nelems)
			break;
		if (i <= idx)
			continue;
		n = rb_entry(nd, struct _seqrec_rbt, rbnode);
		fn_list[elem++] = n->fnidx;
	}
	mutex_unlock(&f->lock);

	return elem * sizeof(*fn_list);
}

static ssize_t cnt_sr_out_of_order_read(struct file *filp, struct kobject *kobj,
					struct bin_attribute *bin_attr,
					char *buf, loff_t ofs, size_t count)
{
	loff_t idx = 0;
	struct edgx_pt *pt = edgx_dev2pt(kobj_to_dev(kobj));
	struct edgx_frer *f = edgx_br_get_frer(edgx_pt_get_br(pt));
	ptid_t ptid = edgx_pt_get_id(pt);
	int ret = 0;

	if (edgx_sysfs_tbl_params(ofs, count, sizeof(u32), &idx) ||
	    idx >= f->streams_max)
		return -EINVAL;

	mutex_lock(&f->lock);
	if (!is_seqrec_strhdl_used(f, (u16)idx, ptid)) {
		mutex_unlock(&f->lock);
		return -EINVAL;
	}
	((u32 *)buf)[0] = edgx_frer_ip_ppps_cnt(f->iobase, (u16)idx, ptid,
						_CNT_SEQREC_OOO2_L);
	mutex_unlock(&f->lock);

	return ret ? ret : count;
}

static ssize_t cnt_tagless_read(struct file *filp, struct kobject *kobj,
				struct bin_attribute *bin_attr,
				char *buf, loff_t ofs, size_t count)
{
	loff_t idx = 0;
	struct edgx_pt *pt = edgx_dev2pt(kobj_to_dev(kobj));
	struct edgx_frer *f = edgx_br_get_frer(edgx_pt_get_br(pt));
	ptid_t ptid = edgx_pt_get_id(pt);
	int ret = 0;

	if (edgx_sysfs_tbl_params(ofs, count, sizeof(u32), &idx) ||
	    idx >= f->streams_max)
		return -EINVAL;

	mutex_lock(&f->lock);
	if (!is_seqrec_strhdl_used(f, (u16)idx, ptid)) {
		mutex_unlock(&f->lock);
		return -EINVAL;
	}
	((u32 *)buf)[0] = edgx_frer_ip_ppps_cnt(f->iobase, (u16)idx, ptid,
						_CNT_SEQREC_TAGLESS_L);
	mutex_unlock(&f->lock);

	return ret ? ret : count;
}

static ssize_t cnt_sr_rogue_read(struct file *filp, struct kobject *kobj,
				 struct bin_attribute *bin_attr,
				 char *buf, loff_t ofs, size_t count)
{
	loff_t idx = 0;
	struct edgx_pt *pt = edgx_dev2pt(kobj_to_dev(kobj));
	struct edgx_frer *f = edgx_br_get_frer(edgx_pt_get_br(pt));
	ptid_t ptid = edgx_pt_get_id(pt);
	int ret = 0;

	if (edgx_sysfs_tbl_params(ofs, count, sizeof(u32), &idx) ||
	    idx >= f->streams_max)
		return -EINVAL;

	mutex_lock(&f->lock);
	if (!is_seqrec_strhdl_used(f, (u16)idx, ptid)) {
		mutex_unlock(&f->lock);
		return -EINVAL;
	}
	((u32 *)buf)[0] = edgx_frer_ip_ppps_cnt(f->iobase, (u16)idx, ptid,
						_CNT_VECT_ROG_L);
	mutex_unlock(&f->lock);

	return ret ? ret : count;
}

static ssize_t cnt_sr_discarded_read(struct file *filp, struct kobject *kobj,
				     struct bin_attribute *bin_attr,
				     char *buf, loff_t ofs, size_t count)
{
	loff_t idx = 0;
	struct edgx_pt *pt = edgx_dev2pt(kobj_to_dev(kobj));
	struct edgx_frer *f = edgx_br_get_frer(edgx_pt_get_br(pt));
	ptid_t ptid = edgx_pt_get_id(pt);
	int ret = 0;

	if (edgx_sysfs_tbl_params(ofs, count, sizeof(u32), &idx) ||
	    idx >= f->streams_max)
		return -EINVAL;

	mutex_lock(&f->lock);
	if (!is_seqrec_strhdl_used(f, (u16)idx, ptid)) {
		mutex_unlock(&f->lock);
		return -EINVAL;
	}
	((u32 *)buf)[0] = edgx_frer_ip_ppps_cnt(f->iobase, (u16)idx, ptid,
						_CNT_SEQREC_SB_L);
	mutex_unlock(&f->lock);

	return ret ? ret : count;
}

static ssize_t cnt_sr_passed_read(struct file *filp, struct kobject *kobj,
				  struct bin_attribute *bin_attr,
				  char *buf, loff_t ofs, size_t count)
{
	loff_t idx = 0;
	struct edgx_pt *pt = edgx_dev2pt(kobj_to_dev(kobj));
	struct edgx_frer *f = edgx_br_get_frer(edgx_pt_get_br(pt));
	struct edgx_sid_br *sid = edgx_br_get_sid(edgx_pt_get_br(pt));
	ptid_t ptid = edgx_pt_get_id(pt);
	int ret = 0;

	if (edgx_sysfs_tbl_params(ofs, count, sizeof(u32), &idx) ||
	    idx >= f->streams_max)
		return -EINVAL;

	mutex_lock(&f->lock);
	if (!is_seqrec_strhdl_used(f, (u16)idx, ptid)) {
		mutex_unlock(&f->lock);
		return -EINVAL;
	}
	((u32 *)buf)[0] = edgx_sid_get_cnt(sid, (u16)idx, ptid, CNT_OUT);

	mutex_unlock(&f->lock);

	return ret ? ret : count;
}

static ssize_t cnt_sr_lost_read(struct file *filp, struct kobject *kobj,
				struct bin_attribute *bin_attr,
				char *buf, loff_t ofs, size_t count)
{
	loff_t idx = 0;
	struct edgx_br *br = edgx_dev2br(kobj_to_dev(kobj));
	struct edgx_frer *f = edgx_br_get_frer(br);
	int ret = 0;

	if (edgx_sysfs_tbl_params(ofs, count, sizeof(u32), &idx) ||
	    idx >= f->seqrec_max)
		return -EINVAL;

	mutex_lock(&f->lock);
	if (_seqrec_rbt_search(&f->sr_root, (u16)idx)) {
		u32 cnt1, cnt2;

		cnt1 = edgx_frer_ip_perfn_cnt(f->iobase, (u16)idx,
					      _CNT_VECT_JUMP_L);
		cnt2 = edgx_frer_ip_perfn_cnt(f->iobase, (u16)idx,
					      _CNT_SEQREC_OOO_L);
		//Vector_jumpaheads - Vector_outoforder
		((u32 *)buf)[0] = cnt1 - cnt2;
	} else {
		ret = -ENOENT;
	}
	mutex_unlock(&f->lock);

	return ret ? ret : count;
}

static ssize_t cnt_sr_resets_read(struct file *filp, struct kobject *kobj,
				  struct bin_attribute *bin_attr,
				  char *buf, loff_t ofs, size_t count)
{
	loff_t idx = 0;
	struct edgx_br *br = edgx_dev2br(kobj_to_dev(kobj));
	struct edgx_frer *f = edgx_br_get_frer(br);
	int ret = 0;

	if (edgx_sysfs_tbl_params(ofs, count, sizeof(u32), &idx) ||
	    idx >= f->seqrec_max)
		return -EINVAL;

	mutex_lock(&f->lock);
	if (_seqrec_rbt_search(&f->sr_root, (u16)idx)) {
		//+1 because there is a reset at the beginning
		((u32 *)buf)[0] = edgx_frer_ip_perfn_cnt(f->iobase, (u16)idx,
				_CNT_SEQREC_TOUT_L) + 1;
	} else {
		ret = -ENOENT;
	}
	mutex_unlock(&f->lock);

	return ret ? ret : count;
}

/****************************** Sysfs Port group ******************************/
/*Sequence Encode-Decode sysfs group*/
EDGX_DEV_ATTR_RW(seqenc_create, "tsnFrerSeqEncCreate");
EDGX_DEV_ATTR_RW(seqenc_active, "tsnFrerSeqEncActive");
EDGX_DEV_ATTR_RW(seqenc_type, "tsnFrerSeqEncType");
EDGX_DEV_ATTR_WO(seqenc_delstr, "tsnFrerSeqEncDelStr");
EDGX_DEV_ATTR_WO(seqenc_delportfn, "tsnFrerSeqEncDelPortFn");
EDGX_DEV_ATTR_WO(seqrec_add_pt, "tsnFrerSeqRecAddPort");
EDGX_DEV_ATTR_WO(seqrec_del_pt, "tsnFrerSeqRecDelPort");

EDGX_DEV_ATTR_WO(seqenc_add_str, "tsnFrerSeqEncAddStr");
EDGX_BIN_ATTR_RO(seqenc_port2str, "tsnFrerSeqEncPort2Str",
		 FRER_MAX_NO_STREAMS * sizeof(u16));
EDGX_BIN_ATTR_RO(cnt_tagless, "frerCpsSeqRcvyTaglessPackets",
		 FRER_MAX_NO_STREAMS * sizeof(u32));
EDGX_BIN_ATTR_RO(cnt_sr_out_of_order, "frerCpsSeqRcvyOutOfOrderPackets",
		 FRER_MAX_NO_STREAMS * sizeof(uint32_t));
EDGX_BIN_ATTR_RO(cnt_sr_rogue, "frerCpsSeqRcvyRoguePackets",
		 FRER_MAX_NO_STREAMS * sizeof(uint32_t));
EDGX_BIN_ATTR_RO(cnt_sr_discarded, "frerCpsSeqRcvyDiscardedPackets",
		 FRER_MAX_NO_STREAMS * sizeof(uint32_t));
EDGX_BIN_ATTR_RO(cnt_sr_passed, "frerCpsSeqRcvyPassedPackets",
		 FRER_MAX_NO_STREAMS * sizeof(uint32_t));

static struct attribute *ieee1cb_frer_pt_attr[] = {
	&dev_attr_seqenc_create.attr,
	&dev_attr_seqenc_active.attr,
	&dev_attr_seqenc_type.attr,
	&dev_attr_seqenc_delstr.attr,
	&dev_attr_seqenc_delportfn.attr,
	&dev_attr_seqrec_add_pt.attr,
	&dev_attr_seqrec_del_pt.attr,
	&dev_attr_seqenc_add_str.attr,
	NULL
};

static struct bin_attribute *ieee1cb_frer_pt_binattr[] = {
	&bin_attr_seqenc_port2str,
	&bin_attr_cnt_tagless,
	&bin_attr_cnt_sr_out_of_order,
	&bin_attr_cnt_sr_rogue,
	&bin_attr_cnt_sr_discarded,
	&bin_attr_cnt_sr_passed,
	NULL
};

static struct attribute_group frer_pt_group = {
	.name  = "ieee8021FRER",
	.attrs = ieee1cb_frer_pt_attr,
	.bin_attrs = ieee1cb_frer_pt_binattr,
};

/***************************** Sysfs Bridge group *****************************/
EDGX_DEV_ATTR_RO(get_numofports, "tsnFrerNumPorts");
/* Sequence Encode-Decode */
EDGX_BIN_ATTR_RO(seqenc_str2port, "tsnFrerSeqEncStr2Port",
		 FRER_MAX_NO_STREAMS * sizeof(u16));
/* Sequence Generation */
EDGX_DEV_ATTR_WO(seqgen_create, "tsnFrerSeqGenCreate");
EDGX_DEV_ATTR_WO(seqgen_del_fn, "tsnFrerSeqGenDelFn");
EDGX_DEV_ATTR_WO(seqgen_del_str, "tsnFrerSeqGenDelStrhdl");
EDGX_DEV_ATTR_RO(seqgen_cnt, "tsnfrerSeqGenCnt");
EDGX_DEV_ATTR_WO(seqrec_del_fn, "tsnfrerSeqRecDelFn");
EDGX_DEV_ATTR_RO(seqrec_cnt, "tsnfrerSeqRecCnt");
EDGX_DEV_ATTR_RO(seqrec_invseq, "frerSeqRcvyInvalidSequenceValue");
EDGX_DEV_ATTR_WO(seqrec_create, "tsnFrerSeqRecCreate");

EDGX_BIN_ATTR_WO(seqgen_add_dir, "tsnFrerSeqGenAddDir",
		 FRER_MAX_NO_STREAMS * sizeof(u8));
EDGX_BIN_ATTR_WO(seqgen_add_str, "tsnFrerSeqGenAddStream",
		 FRER_MAX_NO_STREAMS * sizeof(u16));
EDGX_BIN_ATTR_RO(seqgen_list_all_fn, "tsnFrerSeqGenList",
		 FRER_MAX_NO_STREAMS * sizeof(u16));
EDGX_BIN_ATTR_RO(seqgen_sid2fn, "tsnFrerSeqGenSid2Func",
		 FRER_MAX_NO_STREAMS * sizeof(u16));
EDGX_BIN_ATTR_RO(seqgen_fn2dir, "tsnFrerSeqGenFunc2Dir",
		 FRER_MAX_NO_STREAMS * sizeof(u8));
EDGX_BIN_ATTR_RO(seqgen_fn2sid, "tsnFrerSeqGenFunc2Sid",
		 FRER_MAX_NO_STREAMS * FRER_MAX_NO_STREAMS * sizeof(u16));
EDGX_BIN_ATTR_RW(seqrec_dir, "tsnFrerSeqRecDir",
		 FRER_MAX_NO_STREAMS * sizeof(u8));
EDGX_BIN_ATTR_WO(seqrec_add_str, "tsnFrerSeqRecAddStr",
		 FRER_MAX_NO_STREAMS * sizeof(u16));
EDGX_BIN_ATTR_RW(seqrec_alg, "tsnFrerSeqRecAlg",
		 FRER_MAX_NO_STREAMS * sizeof(u32));
EDGX_BIN_ATTR_RW(seqrec_histlen, "tsnFrerSeqRecHistLen",
		 FRER_MAX_NO_STREAMS * sizeof(u8));
EDGX_BIN_ATTR_RW(seqrec_rst_timeout, "tsnFrerSeqRecRstTime",
		 FRER_MAX_NO_STREAMS * sizeof(u16));
EDGX_BIN_ATTR_RW(seqrec_take_no_seq, "tsnFrerSeqRecTakeNoSeq",
		 FRER_MAX_NO_STREAMS * sizeof(u8));
EDGX_BIN_ATTR_RW(seqrec_ind_recv, "tsnFrerSeqRecIndRecv",
		 FRER_MAX_NO_STREAMS *
		 sizeof(u8));
EDGX_BIN_ATTR_RW(seqrec_latent_err, "tsnFrerSeqRecLatentErr",
		 FRER_MAX_NO_STREAMS * sizeof(u8));
EDGX_BIN_ATTR_RO(seqrec_pt_mask, "tsnFrerSeqRecPtMask",
		 FRER_MAX_NO_STREAMS * sizeof(u16));
EDGX_BIN_ATTR_WO(seqrec_del_str, "tsnFrerSeqRecDelStr",
		 FRER_MAX_NO_STREAMS * sizeof(u16));
EDGX_BIN_ATTR_RO(seqrec_fn2sid, "tsnFrerSeqRecFn2Sid",
		 FRER_MAX_NO_STREAMS * FRER_MAX_NO_STREAMS * sizeof(u16));
EDGX_BIN_ATTR_RO(seqrec_list_all_fn, "tsnFrerSeqRecList",
		 FRER_MAX_NO_STREAMS * sizeof(u16));
EDGX_BIN_ATTR_RO(cnt_sr_lost, "frerCpsSeqRcvyLostPackets",
		 FRER_MAX_NO_STREAMS * sizeof(uint32_t));
EDGX_BIN_ATTR_RO(cnt_sr_resets, "frerCpsSeqRcvyResets",
		 FRER_MAX_NO_STREAMS * sizeof(uint32_t));

static struct attribute *ieee1cb_frer_br_attr[] = {
	&dev_attr_seqgen_del_str.attr,
	&dev_attr_seqgen_del_fn.attr,
	&dev_attr_seqgen_create.attr,
	&dev_attr_seqgen_cnt.attr,
	&dev_attr_get_numofports.attr,
	&dev_attr_seqrec_create.attr,
	&dev_attr_seqrec_del_fn.attr,
	&dev_attr_seqrec_cnt.attr,
	&dev_attr_seqrec_invseq.attr,
	NULL
};

static struct bin_attribute *ieee1cb_frer_br_binattr[] = {
	&bin_attr_seqgen_add_dir,
	&bin_attr_seqgen_add_str,
	&bin_attr_seqgen_list_all_fn,
	&bin_attr_seqgen_sid2fn,
	&bin_attr_seqgen_fn2sid,
	&bin_attr_seqgen_fn2dir,
	&bin_attr_seqenc_str2port,
	&bin_attr_seqrec_dir,
	&bin_attr_seqrec_add_str,
	&bin_attr_seqrec_alg,
	&bin_attr_seqrec_histlen,
	&bin_attr_seqrec_rst_timeout,
	&bin_attr_seqrec_take_no_seq,
	&bin_attr_seqrec_ind_recv,
	&bin_attr_seqrec_latent_err,
	&bin_attr_seqrec_del_str,
	&bin_attr_seqrec_fn2sid,
	&bin_attr_seqrec_pt_mask,
	&bin_attr_seqrec_list_all_fn,
	&bin_attr_cnt_sr_lost,
	&bin_attr_cnt_sr_resets,
	NULL
};

static struct attribute_group frer_bridge_group = {
	.name  = "ieee8021FRER",
	.attrs = ieee1cb_frer_br_attr,
	.bin_attrs = ieee1cb_frer_br_binattr,
};

int edgx_probe_frer(struct edgx_br *br, struct edgx_frer **f)
{
	struct edgx_frer *frer;
	int i;
	int ret;
	const struct edgx_ifdesc *ifd;
	struct edgx_ifdesc        pifd;
	ptid_t                    ptid;
	const struct edgx_ifreq   ifreq = { .id = AC_FRER_ID, .v_maj = 1 };

	ifd = edgx_ac_get_if(&ifreq);
	if (!ifd)
		return -ENODEV;

	frer = kzalloc(sizeof(*frer), GFP_KERNEL);
	if (!frer)
		return -ENOMEM;

	frer->parent = br;
	frer->iobase = ifd->iobase;
	frer->sg_root = RB_ROOT;
	frer->se_root = RB_ROOT;
	frer->sr_root = RB_ROOT;
	frer->seqgencnt = 0;
	frer->seqreccnt = 0;
	frer->streams_max = edgx_brfdb_sid_get_max_str(edgx_br_get_fdb(br));
	frer->seqrec_max = edgx_br_get_generic_pow2(br, BR_GX_FRER_ENTRIES);
	for (i = 0; i < EDGX_BR_MAX_PORTS; i++) {
		frer->se_ports[i].active = ENC_UNDEFINED;
		INIT_LIST_HEAD(&frer->se_ports[i].se_list);
	}
	mutex_init(&frer->lock);
	calc_rcv_rst(&frer);

	ret = edgx_br_sysfs_add(br, &frer_bridge_group);
	if (ret)
		goto out_probe_frer;

	edgx_ac_for_each_ifpt(ptid, ifd, &pifd) {
		struct edgx_pt *pt = edgx_br_get_brpt(br, ptid);

		if (pt)
			edgx_pt_add_sysfs(pt, &frer_pt_group);
	}

	edgx_br_info(br, "Setup FRER ... done\n");
	*f = frer;
	return 0;

out_probe_frer:
	kfree(frer);
	return ret;
}

void edgx_shutdown_frer(struct edgx_frer *f)
{
	struct strlist *l_sg;
	struct _se_strlist *l_se;
	struct list_head *tmp;
	struct list_head *pos;
	struct rb_node *next;
	struct _seqgen_rbt *n_sg;
	struct _se_streams_rbt *n_se;
	int i;
	const struct edgx_ifdesc *ifd;
	struct edgx_ifdesc        pifd;
	ptid_t                    ptid;
	const struct edgx_ifreq   ifreq = { .id = AC_FRER_ID, .v_maj = 1 };

	if (!f)
		return;

	/* delete list from se_ports */
	for (i = 0; i < EDGX_BR_MAX_PORTS; i++) {
		if (!list_empty(&f->se_ports[i].se_list)) {
			list_for_each_safe(pos, tmp, &f->se_ports[i].se_list) {
				l_se = list_entry(pos, struct _se_strlist,
						  entry);
				list_del(pos);
				kfree(l_se);
			}
		}
	}
	/* delete seqenc streams RBT */
	next = rb_first(&f->se_root);
	while (next) {
		n_se = rb_entry(next, struct _se_streams_rbt, rbnode);
		next = rb_next(&n_se->rbnode);
		rb_erase(&n_se->rbnode, &f->se_root);
		RB_CLEAR_NODE(&n_se->rbnode);
		kfree(n_se);
	}
	/* delete seqgen rbt but first list from it */
	next = rb_first(&f->sg_root);
	while (next) {
		n_sg = rb_entry(next, struct _seqgen_rbt, rbnode);
		next = rb_next(&n_sg->rbnode);
		if (!list_empty(&n_sg->sg_list)) {
			list_for_each_safe(pos, tmp, &n_sg->sg_list) {
				l_sg = list_entry(pos, struct strlist,
						  entry);
				list_del(pos);
				kfree(l_sg);
			}
		}
		rb_erase(&n_sg->rbnode, &f->sg_root);
		RB_CLEAR_NODE(&n_sg->rbnode);
		kfree(n_sg);
	}

	ifd = edgx_ac_get_if(&ifreq);
	if (!ifd)
		return;

	edgx_ac_for_each_ifpt(ptid, ifd, &pifd) {
		struct edgx_pt *pt = edgx_br_get_brpt(f->parent, ptid);

		if (pt)
			edgx_pt_rem_sysfs(pt, &frer_pt_group);
	}

	kfree(f);
}
