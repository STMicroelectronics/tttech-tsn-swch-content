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

#include "edge_ac.h"
#include "edge_port.h"
#include "edge_st.h"
#include "edge_stat.h"
#include "edge_time.h"
#include "edge_bridge.h"
#include "edge_fqtss.h"
#include "edge_util.h"
#include "edge_link.h"

/* #define EDGX_SCHED_DBG */

#if defined(EDGX_SCHED_DBG)
#define sched_dbg	pr_info
#else
#define sched_dbg(args...)
#endif

/* FSC HW ITF address definitions */

#define EDGX_SCHED_PT_RES_LEN		(0x1000)
#define EDGX_SCHED_PT_RES_OFFS		(0x10000)
#define EDGX_SCHED_OF_GEN_REGS		(0x4000)
#define EDGX_SCHED_OF_GATE_CNT		((EDGX_SCHED_OF_GEN_REGS) + 0x002)
#define EDGX_SCHED_GATE_CNT_MASK	(0x7f)
#define EDGX_SCHED_OF_ROW_CNT		((EDGX_SCHED_OF_GEN_REGS) + 0x004)
#define EDGX_SCHED_ROW_CNT_MASK		(0x0f)
#define EDGX_SCHED_OF_CLK_FRQ		((EDGX_SCHED_OF_GEN_REGS) + 0x006)
#define EDGX_SCHED_CLK_FRQ_MASK		(0xff)
#define EDGX_SCHED_TAB_0		(0x800)
#define EDGX_SCHED_TAB_1		(0x900)
#define EDGX_SCHED_TBL_OFS	       ((EDGX_SCHED_TAB_1) - (EDGX_SCHED_TAB_0))
#define EDGX_SCHED_TBL_GEN		((EDGX_SCHED_TAB_0) + 0x0)
#define EDGX_SCHED_CAN_USE_MASK		(0x0001)
#define EDGX_SCHED_IN_USE_MASK		(0x0002)
#define EDGX_SCHED_START_TIME_NSEC	((EDGX_SCHED_TAB_0) + 0x014)
#define EDGX_SCHED_START_TIME_SEC	((EDGX_SCHED_TAB_0) + 0x018)
#define EDGX_SCHED_CYC_TIME_NSEC	((EDGX_SCHED_TAB_0) + 0x024)
#define EDGX_SCHED_CYC_TIME_SUBNS	((EDGX_SCHED_TAB_0) + 0x020)
#define EDGX_SCHED_CYC_TS_SEC		((EDGX_SCHED_TAB_0) + 0x038)
#define EDGX_SCHED_CYC_TS_NS_H		((EDGX_SCHED_TAB_0) + 0x036)
#define EDGX_SCHED_CYC_TS_NS_L		((EDGX_SCHED_TAB_0) + 0x034)
#define EDGX_SCHED_CYCLE_CNT		((EDGX_SCHED_TAB_0) + 0x040)
#define EDGX_SCHED_LAST_CYC		((EDGX_SCHED_TAB_0) + 0x044)
#define EDGX_SCHED_START_TIME_SEC_MASK	(0x00ff)
#define EDGX_SCHED_STOP_LAST		(0x0100)
#define EDGX_SCHED_ROW_AC_CMD0		(0x1000)
#define EDGX_SCHED_ROW_AC_CMD1		(0x1002)
#define EDGX_SCHED_ROW_AC_CMD1_MASK	(0x3ff)
#define EDGX_SCHED_ROW_DATA_OUT0	(0x1010)
#define EDGX_SCHED_ROW_DATA_CYCLES	(0x1018)
#define EDGX_SCHED_CMD_SCHED_MASK	(0x000f)
#define EDGX_SCHED_CMD_TAB_MASK		(0x0001)
#define EDGX_SCHED_CMD_TAB_MASK_SHIFT	(8)
#define EDGX_SCHED_AC_WRITE		BIT(14)
#define EDGX_SCHED_AC_TRANSFER		BIT(15)
#define EDGX_SCHED_AC_ERR		BIT(13)
#define EDGX_SCHED_EME_DIS_CTRL		(0x020)
#define EDGX_SCHED_EME_DIS_ON		(0x0001)
#define EDGX_SCHED_EME_DIS_OFF		(0x0000)
#define EDGX_SCHED_EME_STAT_DEF		(0xffff)
#define EDGX_SCHED_EME_DIS_CTRL_R_MASK	(0x0002)
#define EDGX_SCHED_EME_DIS_STAT0	(0x030)
#define EDGX_SCHED_EME_DIS_STAT1	(0x032)
#define EDGX_SCHED_EME_DIS_STAT2	(0x034)
#define EDGX_SCHED_EME_DIS_STAT3	(0x036)
#define EDGX_SCHED_SCH_GEN		(0x000)
#define EDGX_SCHED_DC_SPD		(0x002)
#define EDGX_SCHED_HW_DC_SPD_200MHZ	(0x5 << 5)
#define EDGX_SCHED_HW_DC_SPD_125MHZ	(0x8 << 5)
#define EDGX_SCHED_HW_DC_SPD_100MHZ	(0xa << 5)
#define EDGX_SCHED_MAX_DELAY		(64U)
#define EDGX_SCHED_MIN_ADVANCE		(32U)
#define EDGX_SCHED_TIME_ADJ_DC		(1540U)
#define EDGX_SCHED_INTERVAL_MAX		(0xffff)
#define EDGX_SCHED_INTERVAL_MIN		(8U)
#define EDGX_SCHED_HW_DC_1000FULL	(1540 << 4)
#define EDGX_SCHED_HW_DC_100FULL	((1540 << 4) | 0x001)
#define EDGX_SCHED_HW_DC_10FULL		((1540 << 4) | 0x002)
#define EDGX_SCHED_UPDATE_TS_MASK	(0x8000)
#define EDGX_SCHED_HW_MAX_CT_NS		(999999999U)
#define EDGX_SCHED_INT_MASK		(0x1100)
#define EDGX_SCHED_INT_STAT		(0x1102)
#define EDGX_SCHED_INT_MSKVAL		(0x01)
#define EDGX_SCHED_DEF_MAX_SDU		(1506U)

/** Number of HW scheduling tables */
#define EDGX_SCHED_HW_TAB_CNT		(2U)

/* Maximum possible number of schedule table rows */
#define EDGX_SCHED_HW_MAX_ROWS		(1024U)

#define EDGX_SCHED_DEF_GATE_STATES	(0xff)
#define EDGX_SCHED_OP_SET_GSTATES	(0x00)
#define EDGX_SCHED_OP_HOLD_MAC		(0x01)
#define EDGX_SCHED_OP_REL_MAC		(0x02)

/** Scheduler state machine states */
enum edgx_sched_state {
	EDGX_SCHED_ST_IDLE = 0,	/* Idle state, no pending schedule */
	EDGX_SCHED_ST_PENDING,	/* New schedule submitted and pending in HW */
	EDGX_SCHED_ST_PENDING_DELAYED/* New schedule pending in SW
				      * and waiting to be submitted to HW
				      */
};

/** Schedule table entry */
struct edgx_sched_tab_entry {
	u32 time_interval;
	u8 operation_name;
	u8 gate_states;
	u16 padding;
} __packed;

/** Schedule table */
struct edgx_sched_table {
	struct edgx_sched_tab_entry *entries;	    /* Table rows */
	u32			     entry_cnt;     /* Number of entries */
	u32			     list_len;	    /* Number of used entries */
	struct timespec64	     base_time;	    /* Base time */
	u32			     cycle_time_num;/* Numerator */
	u32			     cycle_time_denom;  /* Denominator */
	u8			     gate_states;       /* Initial gate states*/
};

/** Qbv Scheduler */
struct edgx_st {
	struct edgx_pt		*parent;
	struct edgx_st_com      *com;
	struct edgx_stat_hdl	*hstat;

	edgx_io_t		*iobase_pt;  /* Port specific HW base address */
	int			 sched_idx;  /* FSC scheduler number */

	struct edgx_sched_table	 admin_tab;  /* The admin schedule table */
	/* The operational schedule table mirrors of the HW schedule tables */
	struct edgx_sched_table	 op_tabs[EDGX_SCHED_HW_TAB_CNT];

	bool			 gate_enabled;	   /* Gate enabled */
	bool			 config_change;	   /* Config change requested */
	struct timespec64	 conf_change_time; /* Config change time */
	struct timespec64	 start_time;	   /* Schedule start time */
	enum edgx_sched_state	 state;		   /* Scheduler state */
	struct delayed_work	 hnd_delayed;	   /* Pending delayed work */
	u64			 conf_change_err;
	int			 link_mode;
	u16			 down_cnt_speed;  /* Down-counter speed value */
	/* Sum of intervals in the admin entries */
	u32			 interval_sum;
	struct mutex		 lock;		   /* Protect edgx_sched */
	struct edgx_st_tr_rate	 tr_rate[EDGX_ST_MAX_QUEUES];
};

enum _stat_st_idx {
	_STAT_TX_OVERRUN = 0,
	_STAT_MAX,
};

static const struct edgx_statinfo _st_statinfo = {
	.feat_id = EDGX_STAT_FEAT_ST,
	/* Worst case, there can be one TX_OVERFLOW in every interval of the
	 * gate control list. Since the lower limit of the interval as of
	 * the IP implementation is 64 clock (= 250ns@255MHz, where 255 MHz
	 * is the highest frequency possible on FSC), i.e., the 32bit delta-
	 * counter wraps after
	 *   2^32 * 250 nanoseconds (approx 18 minutes).
	 * So we'll run the capture counter update every 15 minutes to have
	 * a 'nice' number.
	 *
	 * Note that this is the real worst case, where it is assumed that
	 * every interval produces a TX_OVERRUN, which is extremely unlikely.
	 */
	.rate_ms = 900000, /* = 15 minutes */
	.base    = 0x2C0,
	.nwords  = _STAT_MAX,
};

/** Qbv scheduler common part */
struct edgx_st_com {
	struct edgx_br		 *parent;
	const struct edgx_ifdesc *ifd_com;
	edgx_io_t		 *iobase;	  /* Common HW base address */
	struct edgx_st		 *sched_list[EDGX_BR_MAX_PORTS];
	u16			  ns_per_clk;	  /* ns per clock tick */
	u32			  max_entry_cnt; /* Maximum ctrl entry count */
	u32			  max_cyc_time_ns;/*Maximum cycle time*/
	struct edgx_br_irq	 *irq;
	struct work_struct	  work_isr;
	struct workqueue_struct	 *wq_isr;
	u8			  nr_queues;
	u8			  gate_st_msk;
};

/** Lock of the common shared registers across all schedulers */
static struct mutex com_itf_lock;

/** Get the number of rows per table from FSC HW ITF. */
static u16 edgx_sched_get_hw_row_cnt(edgx_io_t *base)
{
	u16	val;

	val = edgx_rd16(base, EDGX_SCHED_OF_ROW_CNT);
	val = val & EDGX_SCHED_ROW_CNT_MASK;

	return (1 << val);
}

static int edgx_sched_hw_init(edgx_io_t *base_pt, u16 ns_per_clk,
			      u8 gate_states)
{
	u16 dc_speed;

	mutex_init(&com_itf_lock);
	edgx_wr16(base_pt, EDGX_SCHED_EME_DIS_STAT0, (u16)gate_states);
	edgx_wr16(base_pt, EDGX_SCHED_EME_DIS_STAT1, 0);
	edgx_wr16(base_pt, EDGX_SCHED_EME_DIS_STAT2, 0);
	edgx_wr16(base_pt, EDGX_SCHED_EME_DIS_STAT3, 0);
	edgx_wr16(base_pt, EDGX_SCHED_EME_DIS_CTRL, EDGX_SCHED_EME_DIS_ON);

	if (ns_per_clk == 5U)
		dc_speed = EDGX_SCHED_HW_DC_SPD_200MHZ;
	else if (ns_per_clk == 8U)
		dc_speed = EDGX_SCHED_HW_DC_SPD_125MHZ;
	else if (ns_per_clk == 10U)
		dc_speed = EDGX_SCHED_HW_DC_SPD_100MHZ;
	else
		return -EINVAL;

	edgx_wr16(base_pt, EDGX_SCHED_DC_SPD, dc_speed);

	return 0;
}

/** Get the HW clock frequency */
static u16 edgx_sched_get_hw_clk_frq(edgx_io_t *base)
{
	u16 val;

	val = edgx_rd16(base, EDGX_SCHED_OF_CLK_FRQ);
	val &= EDGX_SCHED_CLK_FRQ_MASK;

	return val;
}

/** Check if a new configuration is pending */
static bool edgx_sched_hw_is_pending(edgx_io_t *base_pt)
{
	u16 pending_0 = edgx_rd16(base_pt, EDGX_SCHED_TBL_GEN) &
			EDGX_SCHED_CAN_USE_MASK;
	u16 pending_1 = edgx_rd16(base_pt,
				  EDGX_SCHED_TBL_GEN + EDGX_SCHED_TBL_OFS) &
			EDGX_SCHED_CAN_USE_MASK;

	return (pending_0 || pending_1);
}

/** Cancel a pending configuration */
static void edgx_sched_hw_cancel_pending(edgx_io_t *base_pt)
{
	u16 tbl_gen = edgx_rd16(base_pt, EDGX_SCHED_TBL_GEN);

	edgx_wr16(base_pt, EDGX_SCHED_TBL_GEN,
		  tbl_gen & ~EDGX_SCHED_CAN_USE_MASK);

	tbl_gen = edgx_rd16(base_pt, EDGX_SCHED_TBL_GEN + EDGX_SCHED_TBL_OFS);
	edgx_wr16(base_pt, EDGX_SCHED_TBL_GEN + EDGX_SCHED_TBL_OFS,
		  tbl_gen & ~EDGX_SCHED_CAN_USE_MASK);
}

/** Get the first free table index */
static int edgx_sched_hw_get_free_tab(edgx_io_t *base_pt)
{
	u16 in_use = edgx_rd16(base_pt, EDGX_SCHED_TBL_GEN) &
			       EDGX_SCHED_IN_USE_MASK;
	if (!in_use)
		return 0;

	in_use = edgx_rd16(base_pt, EDGX_SCHED_TBL_GEN + EDGX_SCHED_TBL_OFS) &
			   EDGX_SCHED_IN_USE_MASK;
	if (!in_use)
		return 1;

	return -EINVAL;
}

/** Get the first used table index */
static int edgx_sched_hw_get_used_tab(edgx_io_t *base_pt)
{
	u16 in_use = edgx_rd16(base_pt, EDGX_SCHED_TBL_GEN) &
			       EDGX_SCHED_IN_USE_MASK;
	if (in_use)
		return 0;

	in_use = edgx_rd16(base_pt, EDGX_SCHED_TBL_GEN + EDGX_SCHED_TBL_OFS) &
			   EDGX_SCHED_IN_USE_MASK;
	if (in_use)
		return 1;

	return -EINVAL;
}

/** Submit a new configuration */
static void edgx_sched_hw_set_pending(edgx_io_t *base_pt,
				      int tab_idx,
				      const struct timespec64 *base_time,
				      u32 cycle_nsec,
				      u32 cycle_subnsec,
				      u16 down_cnt_speed)
{
	u16 tbl_gen;

	sched_dbg("SET PENDING: Cycle time: nsec=%u, subnsec=%u\n",
		  cycle_nsec, cycle_subnsec);
	sched_dbg("SET PENDING: Start time: sec=%u, nsec=%u\n",
		  (u16)base_time->tv_sec & EDGX_SCHED_START_TIME_SEC_MASK,
		  (u32)base_time->tv_nsec);

	edgx_wr32(base_pt, EDGX_SCHED_CYC_TIME_NSEC +
			   (tab_idx * EDGX_SCHED_TBL_OFS),
		  cycle_nsec);
	edgx_wr32(base_pt, EDGX_SCHED_CYC_TIME_SUBNS +
			   (tab_idx * EDGX_SCHED_TBL_OFS),
		  cycle_subnsec);

	edgx_wr16(base_pt,
		  EDGX_SCHED_START_TIME_SEC + (tab_idx * EDGX_SCHED_TBL_OFS),
		 (u16)base_time->tv_sec & EDGX_SCHED_START_TIME_SEC_MASK);
	edgx_wr32(base_pt,
		  EDGX_SCHED_START_TIME_NSEC + (tab_idx * EDGX_SCHED_TBL_OFS),
		 (u32)base_time->tv_nsec);
	edgx_wr16(base_pt,
		  EDGX_SCHED_CYCLE_CNT + (tab_idx * EDGX_SCHED_TBL_OFS), 0);

	edgx_wr16(base_pt, EDGX_SCHED_SCH_GEN, down_cnt_speed);

	tbl_gen = edgx_rd16(base_pt,
			    EDGX_SCHED_TBL_GEN +
			    (tab_idx * EDGX_SCHED_TBL_OFS));
	edgx_wr16(base_pt, EDGX_SCHED_TBL_GEN + (tab_idx * EDGX_SCHED_TBL_OFS),
		  (tbl_gen | EDGX_SCHED_CAN_USE_MASK) & ~EDGX_SCHED_STOP_LAST);
}

static void edgx_sched_hw_get_cc_time(edgx_io_t *base_pt, int tab_idx,
				      struct timespec64 *time)
{
	u16 start_sec;
	u32 start_nsec;

	start_sec = edgx_rd16(base_pt,
			      EDGX_SCHED_START_TIME_SEC +
			      (tab_idx * EDGX_SCHED_TBL_OFS));
	start_nsec = edgx_rd32(base_pt,
			       EDGX_SCHED_START_TIME_NSEC +
			       (tab_idx * EDGX_SCHED_TBL_OFS));

	time->tv_sec = (time64_t)start_sec;
	time->tv_nsec = (long)start_nsec;
}

static int edgx_sched_hw_transfer_wait(edgx_io_t *base)
{
	u16 cmd;
	u16 timeout = 500;

	do {
		cmd = edgx_rd16(base, EDGX_SCHED_ROW_AC_CMD0);
		cpu_relax();

		if ((timeout-- == 0U) || (cmd & EDGX_SCHED_AC_ERR))
			return -EBUSY;

	} while (cmd & EDGX_SCHED_AC_TRANSFER);

	return 0;
}

static int edgx_sched_hw_write_entry(edgx_io_t *base, int sched_idx,
				     int hw_tab_idx, int row_idx,
				     u16 gate_states, u16 time_interval_clk)
{
	int ret;
	u16 cmd = 0;

	mutex_lock(&com_itf_lock);
	edgx_wr16(base, EDGX_SCHED_ROW_AC_CMD1,
		  row_idx & EDGX_SCHED_ROW_AC_CMD1_MASK);
	edgx_wr16(base, EDGX_SCHED_ROW_DATA_OUT0, gate_states);
	edgx_wr16(base, EDGX_SCHED_ROW_DATA_CYCLES, time_interval_clk);

	cmd = (sched_idx & EDGX_SCHED_CMD_SCHED_MASK) |
	      ((hw_tab_idx & EDGX_SCHED_CMD_TAB_MASK)
	       << EDGX_SCHED_CMD_TAB_MASK_SHIFT) |
	      EDGX_SCHED_AC_WRITE | EDGX_SCHED_AC_TRANSFER;
	edgx_wr16(base, EDGX_SCHED_ROW_AC_CMD0, cmd);
	ret = edgx_sched_hw_transfer_wait(base);

	mutex_unlock(&com_itf_lock);
	return ret;
}

static int edgx_sched_hw_read_entry(edgx_io_t *base, int sched_idx,
				    int hw_tab_idx, int row_idx,
				    u16 *gate_states, u16 *interval_clk)
{
	int ret;
	u16 cmd = 0;

	mutex_lock(&com_itf_lock);
	edgx_wr16(base, EDGX_SCHED_ROW_AC_CMD1,
		  row_idx & EDGX_SCHED_ROW_AC_CMD1_MASK);

	cmd = (sched_idx & EDGX_SCHED_CMD_SCHED_MASK) |
	      ((hw_tab_idx & EDGX_SCHED_CMD_TAB_MASK)
	       << EDGX_SCHED_CMD_TAB_MASK_SHIFT) |
	      EDGX_SCHED_AC_TRANSFER;
	edgx_wr16(base, EDGX_SCHED_ROW_AC_CMD0, cmd);

	ret = edgx_sched_hw_transfer_wait(base);
	if (!ret) {
		*gate_states = edgx_rd16(base, EDGX_SCHED_ROW_DATA_OUT0);
		*interval_clk = edgx_rd16(base, EDGX_SCHED_ROW_DATA_CYCLES);
	}
	mutex_unlock(&com_itf_lock);
	return ret;
}

static void edgx_sched_hw_disable(edgx_io_t *base_pt, u8 gate_states)
{
	edgx_wr16(base_pt, EDGX_SCHED_EME_DIS_STAT0, (u16)gate_states);
	edgx_wr16(base_pt, EDGX_SCHED_EME_DIS_CTRL, EDGX_SCHED_EME_DIS_ON);
}

static void edgx_sched_hw_enable(edgx_io_t *base_pt, u8 gate_states)
{
	edgx_wr16(base_pt, EDGX_SCHED_EME_DIS_STAT0, (u16)gate_states);
	edgx_wr16(base_pt, EDGX_SCHED_EME_DIS_CTRL, EDGX_SCHED_EME_DIS_OFF);
}

#if defined(EDGX_SCHED_DBG)
static void edgx_sched_hw_dump(edgx_io_t *base, edgx_io_t *base_pt,
			       int sched_idx, int tab_idx)
{
	u16 start_sec;
	u32 start_nsec;
	u16 tbl_gen;
	int i, ret;
	u16 states;
	u16 clk;
	u32 cyc_nsec, cyc_subnsec;
	u16 eme_dis;

	eme_dis = edgx_rd16(base_pt, EDGX_SCHED_EME_DIS_CTRL);

	start_sec = edgx_rd16(base_pt,
			      EDGX_SCHED_START_TIME_SEC +
			      (tab_idx * EDGX_SCHED_TBL_OFS));
	start_nsec = edgx_rd32(base_pt,
			       EDGX_SCHED_START_TIME_NSEC +
			       (tab_idx * EDGX_SCHED_TBL_OFS));
	tbl_gen = edgx_rd16(base_pt,
			    EDGX_SCHED_TBL_GEN +
			    (tab_idx * EDGX_SCHED_TBL_OFS));

	cyc_nsec = edgx_rd32(base_pt, EDGX_SCHED_CYC_TIME_NSEC +
			     (tab_idx * EDGX_SCHED_TBL_OFS));
	cyc_subnsec = edgx_rd32(base_pt, EDGX_SCHED_CYC_TIME_SUBNS +
				(tab_idx * EDGX_SCHED_TBL_OFS));

	sched_dbg("SCHED: tbl_gen=0x%x, start_sec=%u, start_nsec=%u\n",
		  tbl_gen, start_sec, start_nsec);
	sched_dbg("SCHED: cyc_nsec=%u, cyc_subnsec=%u\n",
		  cyc_nsec, cyc_subnsec);
	sched_dbg("SCHED: EMEDIS=0x%x\n", eme_dis);

	for (i = 0; i < 30; i++) {
		ret = edgx_sched_hw_read_entry(base, sched_idx,
					       tab_idx, i, &states, &clk);
		sched_dbg("SCHED: TAB0, row%d ret=%d state=0x%x, clk=%u\n",
			  i, ret, states, clk);
	}
}
#endif

static void edgx_sched_stm_cct_handler(struct edgx_st *sched);

static inline struct edgx_st *edgx_dev2sched(struct device *dev)
{
	return edgx_pt_get_st(edgx_dev2pt(dev));
}

static inline u16 edgx_sched_get_hold(struct edgx_st *sched)
{
	return BIT(sched->com->nr_queues);
}

static void edgx_sched_isr_work(struct work_struct *work)
{
	struct edgx_st_com *sc = container_of(work, struct edgx_st_com,
					      work_isr);
	int i;
	u16 intstat = edgx_rd16(sc->iobase, EDGX_SCHED_INT_STAT);

	edgx_wr16(sc->iobase, EDGX_SCHED_INT_STAT, ~intstat);
	sched_dbg("SCHED ISR work.\n");
	for (i = 0; i < EDGX_BR_MAX_PORTS; i++) {
		if (sc->sched_list[i])
			edgx_sched_stm_cct_handler(sc->sched_list[i]);
	}
	if (sc->irq->trig == EDGX_IRQ_LEVEL_TRIG)
		edgx_wr16(sc->iobase, EDGX_SCHED_INT_MASK,
			  EDGX_SCHED_INT_MSKVAL);
}

static irqreturn_t edgx_sched_isr(int irq, void *device)
{
	struct edgx_st_com *sc = (struct edgx_st_com *)device;
	u16 mask;
	u16 stat;

	if (sc->irq->shared) {
		mask = edgx_rd16(sc->iobase, EDGX_SCHED_INT_MASK);
		if (!mask)
			return IRQ_NONE;

		stat = edgx_rd16(sc->iobase, EDGX_SCHED_INT_STAT);
		if (!(mask & stat))
			return IRQ_NONE;
	}

	if (sc->irq->trig == EDGX_IRQ_LEVEL_TRIG)
		edgx_wr16(sc->iobase, EDGX_SCHED_INT_MASK, 0);
	queue_work(sc->wq_isr, &sc->work_isr);

	return IRQ_HANDLED;
}

void edgx_st_com_set_params(struct edgx_st_com *sched_com,
			    u32 freq_err_abs, u32 sched_granul)
{
}

/** Initialize common the scheduler part. */
int edgx_st_com_probe(struct edgx_br *br, struct edgx_br_irq *irq,
		      const char *drv_name,
		      struct edgx_st_com **psc, bool *max_2_speeds)
{
	const struct edgx_ifreq ifreq = { .id = AC_SCHED_ID, .v_maj = 1 };
	const struct edgx_ifdesc *ifd_com = edgx_ac_get_if(&ifreq);
	u16 clk_frq;
	int ret = 0;

	if (!br || !psc || !max_2_speeds)
		return -EINVAL;

	if (!ifd_com || !ifd_com->ptmap)
		return -ENODEV;

	*psc = kzalloc(sizeof(**psc), GFP_KERNEL);
	if (!(*psc)) {
		edgx_br_err(br, "Cannot allocate Common Scheduled Traffic\n");
		return -ENOMEM;
	}

	*max_2_speeds = false;
	(*psc)->parent = br;
	(*psc)->ifd_com = ifd_com;
	(*psc)->iobase = (*psc)->ifd_com->iobase;
	(*psc)->max_entry_cnt = edgx_sched_get_hw_row_cnt((*psc)->iobase);
	(*psc)->nr_queues = edgx_br_get_generic(br, BR_GX_QUEUES);
	(*psc)->gate_st_msk = (u8)(BIT((*psc)->nr_queues) - 1);
	/* The last row is reserved for 0 interval entries */
	(*psc)->max_entry_cnt -= 1U;
	clk_frq = edgx_sched_get_hw_clk_frq((*psc)->iobase);
	(*psc)->ns_per_clk = 1000U / clk_frq;
	(*psc)->max_cyc_time_ns = min(EDGX_SCHED_HW_MAX_CT_NS,
				      (*psc)->max_entry_cnt *
				      (u32)((*psc)->ns_per_clk) *
				      EDGX_SCHED_INTERVAL_MAX);

	(*psc)->irq = irq;
	INIT_WORK(&(*psc)->work_isr, &edgx_sched_isr_work);
	(*psc)->wq_isr = alloc_workqueue(drv_name,
					 WQ_HIGHPRI | WQ_MEM_RECLAIM, 0);
	if (!(*psc)->wq_isr) {
		pr_err("%s(): alloc_workqueue failed!\n", __func__);
		return -ENOMEM;
	}

	if (irq->shared)
		ret = request_irq(irq->irq_vec[0], &edgx_sched_isr, IRQF_SHARED,
				  drv_name, *psc);
	else
		ret = request_irq(irq->irq_vec[EDGX_IRQ_NR_ST_SCHED_TAB],
				  &edgx_sched_isr, IRQF_SHARED, drv_name, *psc);

	if (ret) {
		pr_err("%s(): request_irq failed! ret=%d, irq=%d\n",
		       __func__, ret, irq->shared ?
		       irq->irq_vec[0] : irq->irq_vec[EDGX_IRQ_NR_ST_SCHED_TAB]);
		destroy_workqueue((*psc)->wq_isr);
		return ret;
	}

	edgx_wr16((*psc)->iobase, EDGX_SCHED_INT_STAT, 0);
	edgx_wr16((*psc)->iobase, EDGX_SCHED_INT_MASK, EDGX_SCHED_INT_MSKVAL);

	sched_dbg("FSC cfg: max_entry_cnt=%d, ns_per_clk=%d\n",
		  (*psc)->max_entry_cnt, (*psc)->ns_per_clk);
	return ret;
}

void edgx_st_com_shutdown(struct edgx_st_com *sched_com)
{
	if (sched_com) {
		if (sched_com->irq->shared)
			free_irq(sched_com->irq->irq_vec[0], sched_com);
		else
			free_irq(sched_com->irq->irq_vec[EDGX_IRQ_NR_ST_SCHED_TAB],
				 sched_com);

		cancel_work_sync(&sched_com->work_isr);
		edgx_wr16(sched_com->iobase, EDGX_SCHED_INT_STAT, 0);
		edgx_wr16(sched_com->iobase, EDGX_SCHED_INT_MASK, 0);
		destroy_workqueue(sched_com->wq_isr);
		kfree(sched_com);
	}
}

#if defined(EDGX_SCHED_DBG)
static void edgx_sched_dump(struct edgx_st *sched)
{
	int i;

	sched_dbg("Port Id: %d\n", edgx_pt_get_id(sched->parent));
	sched_dbg("sched_idx: %d\n", sched->sched_idx);
	sched_dbg("iobase_com: 0x%p; iobase_pt: 0x%p\n",
		  sched->com->iobase, sched->iobase_pt);
	sched_dbg("max_entry_cnt: %u\n", sched->com->max_entry_cnt);
	sched_dbg("gate_enabled: %d\n", sched->gate_enabled);
	sched_dbg("config_change: %d\n", sched->config_change);
	sched_dbg("conf_change_time: sec=%lli, nsec=%li\n",
		  sched->conf_change_time.tv_sec,
		  sched->conf_change_time.tv_nsec);
	sched_dbg("start_time: sec=%lli, nsec=%li\n",
		  sched->start_time.tv_sec, sched->start_time.tv_nsec);
	sched_dbg("state: %d\n", sched->state);
	sched_dbg("ns_per_clk: %d\n", sched->com->ns_per_clk);
	sched_dbg("conf_change_err: %llu\n", sched->conf_change_err);
	sched_dbg("link_mode: %d\n", sched->link_mode);
	sched_dbg("interval_sum: %d\n", sched->interval_sum);
	sched_dbg("max_cyc_time_ns: %d\n", sched->com->max_cyc_time_ns);

	for (i = 0; i < sched->com->nr_queues; i++) {
		sched_dbg("tr_rate[%d]: %llu, %llu\n", i,
			  sched->tr_rate[i].num,
			  sched->tr_rate[i].denom);
	}
}

static void edgx_sched_dump_tab(struct edgx_sched_table *tab, const char *label)
{
	int i;

	sched_dbg("TABLE %s\n", label);
	sched_dbg("entry_cnt: %d\n", tab->entry_cnt);
	sched_dbg("list_len: %d\n", tab->list_len);
	sched_dbg("gate_states: 0x%x\n", tab->gate_states);
	sched_dbg("cycle_time_num: %d\n", tab->cycle_time_num);
	sched_dbg("cycle_time_denom: %d\n", tab->cycle_time_denom);
	sched_dbg("base_time: sec=%lli, nsec=%li\n", tab->base_time.tv_sec,
		  tab->base_time.tv_nsec);

	if (!tab->entries)
		return;

	sched_dbg("Entries:\n");
	for (i = 0; i < tab->entry_cnt; i++) {
		sched_dbg("op=0x%x\tstate=0x%x\tint=%d\n",
			  tab->entries[i].operation_name,
			  tab->entries[i].gate_states,
			  tab->entries[i].time_interval);
	}
}
#endif

static void edgx_sched_rational_to_nsec(u32 sec_num,
					u32 sec_denom,
					u64 *nsec,
					u32 *subnsec)
{
	u32 cycle_sec;
	u64 num = sec_num;
	u64 rem = do_div(num, sec_denom);

	cycle_sec = num;
	num = (u64)rem * NSEC_PER_SEC;
	rem = do_div(num, sec_denom);
	*nsec = num + (cycle_sec * NSEC_PER_SEC);

	num = (u64)rem << 32;
	rem = do_div(num, sec_denom);
	*subnsec = num;
}

static void edgx_sched_nsec_to_rational(u64 nsec,
					u32 subnsec,
					u32 *sec_num,
					u32 *sec_denom)
{
	//TODO Add a real conversion algorithm.
	*sec_num = nsec;
	*sec_denom = NSEC_PER_SEC;
}

static void edgx_st_calc_trans_rate(struct edgx_st *sched)
{
	int queue, i;
	u64 sum, idle_time, cyc_time_nsec, sum_total = 0;
	struct edgx_sched_table	*tab = &sched->admin_tab;
	u32 num, denom, cyc_time_subnsec;

	edgx_sched_rational_to_nsec(sched->admin_tab.cycle_time_num,
				    sched->admin_tab.cycle_time_denom,
				    &cyc_time_nsec,
				    &cyc_time_subnsec);

	for (queue = 0; queue < sched->com->nr_queues; queue++) {
		sum = 0;
		sum_total = 0;
		for (i = 0; i < tab->list_len; i++) {
			sum_total += tab->entries[i].time_interval;

			if (tab->entries[i].gate_states & BIT(queue))
				sum += tab->entries[i].time_interval;
		}

		idle_time = cyc_time_nsec - sum_total;

		/* The gate remains in the last state -> add idle time */
		if (tab->entries[tab->list_len - 1].gate_states & BIT(queue))
			sum += idle_time;

		if (sum) {
			edgx_sched_nsec_to_rational(sum, 0, &num, &denom);
			sched->tr_rate[queue].num = (u64)denom *
						    (u64)tab->cycle_time_num;
			sched->tr_rate[queue].denom = (u64)num *
						      (u64)
						      tab->cycle_time_denom;
		} else {
			sched->tr_rate[queue].num = 0;
			sched->tr_rate[queue].denom = 1;
		}
	}
}

static void edgx_sched_notify(struct edgx_st *sched)
{
	struct edgx_fqtss *fqtss = edgx_pt_get_fqtss(sched->parent);

	sched_dbg("Notify\n");
	edgx_fqtss_st_change(fqtss, sched);
}

/* NOTE: Deviation from the standard:
 * The control interval is limited to 16-bit cycles.
 * Larger intervals are truncated.
 */
static ssize_t edgx_sched_adjust_admin_tab(struct edgx_st *sched)
{
	int i;
	u32 cycles;
	struct edgx_sched_table	*tab = &sched->admin_tab;

	sched->interval_sum = 0;

	for (i = 0; i < tab->entry_cnt; i++) {
		tab->entries[i].gate_states &= sched->com->gate_st_msk;
		cycles = tab->entries[i].time_interval / sched->com->ns_per_clk;
		if ((cycles < EDGX_SCHED_INTERVAL_MIN) ||
		    (cycles > EDGX_SCHED_INTERVAL_MAX)) {
			sched->conf_change_err++;
			edgx_pt_err(sched->parent,
				    "Control List Interval out of range!\n");
			return -EINVAL;
		}
		sched->interval_sum += tab->entries[i].time_interval;
	}
	return 0;
}

static int edgx_sched_check_cc_params(struct edgx_st *sched)
{
	if ((!sched->admin_tab.entries) ||
	    (sched->admin_tab.entry_cnt == 0) ||
	    (sched->admin_tab.cycle_time_num == 0) ||
	    (sched->admin_tab.list_len == 0) ||
	    (sched->admin_tab.entry_cnt < sched->admin_tab.list_len))
		return -EFAULT;

	sched->link_mode = edgx_pt_get_speed(sched->parent);

	if (sched->link_mode == SPEED_UNKNOWN) {
		edgx_pt_warn(sched->parent, "Link speed unknown!\n");
		return -EIO;
	}
	return 0;
}

/** Adjust time by FSC down-counter delay for given link speed */
static void edgx_sched_adjust_time(struct edgx_st *sched,
				   const struct timespec64 *in_time,
				   struct timespec64 *out_time,
				   bool increase)
{
	u32 adj = EDGX_SCHED_TIME_ADJ_DC;
	struct edgx_link *lnk = edgx_pt_get_link(sched->parent);
	ktime_t link_dly = edgx_link_get_tx_delay_min(lnk);
	ktime_t g2o_min = edgx_pt_get_g2omin(sched->parent);
	s64 diff;

	switch (sched->link_mode) {
	case SPEED_1000:
		sched->down_cnt_speed = EDGX_SCHED_HW_DC_1000FULL;
		break;
	case SPEED_100:
		adj *= 10U;
		sched->down_cnt_speed = EDGX_SCHED_HW_DC_100FULL;
		break;
	case SPEED_10:
		adj *= 100U;
		sched->down_cnt_speed = EDGX_SCHED_HW_DC_10FULL;
		break;
	default:
		sched->down_cnt_speed = EDGX_SCHED_HW_DC_1000FULL;
		edgx_pt_err(sched->parent, "Unknown link speed during down counter delay calculation, assuming 1GBps");
		break;
	}

	adj *= sched->com->ns_per_clk;
	if (sched->com->ns_per_clk == 10U)
		adj = (adj * 4U) / 5U;	/* Divide by 1.25 */
	else if (sched->com->ns_per_clk == 5U)
		adj = (adj * 8U) / 5U;  /* 200Mhz */

	adj += 10U * sched->com->ns_per_clk;	/* Additional fixed delay */
	diff = adj + ktime_to_ns(g2o_min) + ktime_to_ns(link_dly);

	if (increase)
		set_normalized_timespec64(out_time, in_time->tv_sec,
					  in_time->tv_nsec + diff);
	else
		set_normalized_timespec64(out_time, in_time->tv_sec,
					  in_time->tv_nsec - diff);
}

/*  Calculate the number of cycles between two points in time.
 *
 *  Algorithm taken from deipce_fsc_hw.c:
 * Basically N = count = delta / (numerator/denominator) [round up].
 * That could overflow with 64 bit arithmetic if nanoseconds were used
 * directly. Calculate N separately for seconds and nanoseconds,
 * taking remainders into account, and round up the result. So
 *
 * N = count = (delta * denominator) / numerator =
 * (delta_sec * denominator) / numerator +
 * (delta_nsec * denominator) / (numerator * NSEC_PER_SEC)
 *
 * With integer division returning quotient as whole number and remainder,
 * this can be written as
 *
 * count = count_sec + count_sec_rem / numerator +
 * (count_nsec + count_nsec_rem / numerator) / NSEC_PER_SEC
 * = count_sec +
 * (NSEC_PER_SEC * count_sec_rem + count_nsec * numerator + count_nsec_rem) /
 * (NSEC_PER_SEC * numerator)
 *
 * That could still overflow, but only when start_time is very much
 * (in the order of 2^32 seconds, i.e. about 136 years) behind min_time.
 */
static u64 edgx_sched_diff_to_cycles(const struct timespec64 *begin_time,
				     const struct timespec64 *end_time,
				     u32 cycle_time_num,
				     u32 cycle_time_denom,
				     bool round_up,
				     u64 diff_sec_mask)
{
	struct timespec64 delta = timespec64_sub(*end_time, *begin_time);

	u64 count;
	u64 count_sec;
	u32 count_sec_rem;
	u64 count_nsec;
	u32 count_nsec_rem;
	u64 count_res;

	delta.tv_sec &= diff_sec_mask;

	count_sec = delta.tv_sec * cycle_time_denom;
	count_sec_rem = do_div(count_sec, cycle_time_num);

	count_nsec = (u64)delta.tv_nsec * cycle_time_denom;
	count_nsec_rem = do_div(count_nsec, cycle_time_num);

	count_res = (u64)count_sec_rem * NSEC_PER_SEC +
			count_nsec * cycle_time_num + count_nsec_rem;
	if (round_up)
		count_res += (u64)cycle_time_num * NSEC_PER_SEC - 1;
	do_div(count_res, cycle_time_num);
	do_div(count_res, NSEC_PER_SEC);

	count = count_sec + count_res;

	return count;
}

static void edgx_sched_calc_future_time(const struct timespec64 *in_time,
					const struct timespec64 *cur_time,
					struct timespec64 *out_time,
					u32 cycle_time_num,
					u32 cycle_time_denom)
{
	u64 count;
	u64 num;
	u32 rem;
	struct timespec64 advance;

	count = edgx_sched_diff_to_cycles(in_time, cur_time, cycle_time_num,
					  cycle_time_denom, true, ~((u64)0));

	/* Add calculated number of cycle times to out_time */
	num = count * cycle_time_num;
	rem = do_div(num, cycle_time_denom);
	advance.tv_sec = num;

	num = (u64)rem * NSEC_PER_SEC;
	do_div(num, cycle_time_denom);
	advance.tv_nsec = num;

	sched_dbg("EDGX_SCHED: %s() start0 %lli.%09li cycle_time %u/%u\n",
		  __func__, in_time->tv_sec, in_time->tv_nsec,
		  cycle_time_num, cycle_time_denom);
	sched_dbg("EDGX_SCHED: %s() count %llu advance %lli.%09li\n",
		  __func__, count, advance.tv_sec,
		  advance.tv_nsec);

	*out_time = timespec64_add(*in_time, advance);
}

static void edgx_sched_calc_cc_time(struct edgx_st *sched,
				    time64_t *delay_sec,
				    bool *time_in_past)
{
	struct timespec64 cur_time;
	time64_t diff;
	struct edgx_time *time = edgx_pt_get_time(sched->parent);

	*time_in_past = false;
	*delay_sec = 0;
	edgx_tm_get_wrk_time(time, &cur_time);

	/* If base time in the past */
	if ((sched->admin_tab.base_time.tv_sec < cur_time.tv_sec) ||
	    ((sched->admin_tab.base_time.tv_sec == cur_time.tv_sec) &&
	    (sched->admin_tab.base_time.tv_nsec <= cur_time.tv_nsec))) {
		edgx_sched_calc_future_time(&sched->admin_tab.base_time,
					    &cur_time,
					    &sched->start_time,
					    sched->admin_tab.cycle_time_num,
					    sched->admin_tab.cycle_time_denom);
		*time_in_past = true;
	} else {
		sched->start_time = sched->admin_tab.base_time;
	}

	sched->conf_change_time = sched->start_time;
	edgx_sched_adjust_time(sched, &sched->start_time,
			       &sched->start_time, false);

	diff = sched->start_time.tv_sec - cur_time.tv_sec;

	if (diff > EDGX_SCHED_MAX_DELAY) {
		if ((diff - EDGX_SCHED_MIN_ADVANCE) < EDGX_SCHED_MAX_DELAY)
			*delay_sec = diff - EDGX_SCHED_MIN_ADVANCE;
		else
			*delay_sec = EDGX_SCHED_MAX_DELAY;
	}
}

static int edgx_sched_config_change(struct edgx_st *sched)
{
	int tb, i, ret = 0;
	struct edgx_sched_table *tab;
	struct edgx_sched_tab_entry *entries;
	u64 ct_nsec;
	u32 ct_subnsec;
	u16 hold = 0;

	tb = edgx_sched_hw_get_free_tab(sched->iobase_pt);
	tab = &sched->op_tabs[tb];
	memcpy(tab, &sched->admin_tab, sizeof(*tab));
	entries = tab->entries;

	for (i = 0; !ret && (i < tab->list_len); i++) {
		if (entries[i].operation_name == EDGX_SCHED_OP_HOLD_MAC)
			hold = edgx_sched_get_hold(sched);
		else if (entries[i].operation_name == EDGX_SCHED_OP_REL_MAC)
			hold = 0;

		ret = edgx_sched_hw_write_entry(sched->com->iobase,
						sched->sched_idx, tb, i,
						(u16)entries[i].gate_states
						| hold,
						entries[i].time_interval /
						sched->com->ns_per_clk);
	}

	/* Add a 0 interval entry at the end. FSC will stop on this entry */
	if (!ret)
		ret = edgx_sched_hw_write_entry(sched->com->iobase,
						sched->sched_idx, tb, i,
						entries[i - 1].gate_states,
						0);

	for (i = 0; hold && !ret && (i < tab->list_len) &&
	     entries[i].operation_name != EDGX_SCHED_OP_HOLD_MAC &&
	     entries[i].operation_name != EDGX_SCHED_OP_REL_MAC; i++) {
		ret = edgx_sched_hw_write_entry(sched->com->iobase,
						sched->sched_idx, tb, i,
						(u16)entries[i].gate_states
						| hold,
						entries[i].time_interval /
						sched->com->ns_per_clk);
	}

	if (!ret) {
		edgx_sched_rational_to_nsec(sched->op_tabs[tb].cycle_time_num,
					    sched->op_tabs[tb].cycle_time_denom,
					    &ct_nsec,
					    &ct_subnsec);
		edgx_sched_hw_set_pending(sched->iobase_pt, tb,
					  &sched->start_time,
					  (u32)ct_nsec,
					  ct_subnsec,
					  sched->down_cnt_speed);
	}
	tab->entries = NULL;
	return ret;
}

/* Check if HW adjusted the start time. Adjust CCT accordingly.*/
static void edgx_sched_fix_cc_time(struct edgx_st *sched)
{
	struct timespec64 hw_start_time;
	struct timespec64 start_time_reduced;
	struct timespec64 delta;

	int tab_idx = edgx_sched_hw_get_used_tab(sched->iobase_pt);

	if (tab_idx < 0)
		return;

	edgx_sched_hw_get_cc_time(sched->iobase_pt, tab_idx, &hw_start_time);
	/* hw_cct has a HW limited range. Start time range must be reduced. */
	start_time_reduced = sched->start_time;
	start_time_reduced.tv_sec &= EDGX_SCHED_START_TIME_SEC_MASK;

	delta = timespec64_sub(hw_start_time, start_time_reduced);

	if ((delta.tv_sec > 0) || (delta.tv_nsec > 0)) {
		sched->conf_change_err++;
		sched->start_time = timespec64_add(sched->start_time, delta);
		edgx_sched_adjust_time(sched, &sched->start_time,
				       &sched->conf_change_time, true);
		edgx_pt_warn(sched->parent, "Start time adjusted by HW!\n");
	}
}

/******************************************************************************
 * State Machine Request Handlers
 *****************************************************************************/
static void edgx_sched_stm_cct_handler(struct edgx_st *sched)
{
	mutex_lock(&sched->lock);
	if ((sched->state == EDGX_SCHED_ST_PENDING) &&
	    (!edgx_sched_hw_is_pending(sched->iobase_pt))) {
		edgx_sched_fix_cc_time(sched);
		sched->state = EDGX_SCHED_ST_IDLE;
		edgx_sched_notify(sched);
	}
	mutex_unlock(&sched->lock);
}

/* NOTE: Deviation from the standard:
 * The Qbv List Config state machine allows to change administrative
 * parameters during a pending configuration.
 * The driver ensures this behavior only up to the point in time, when
 * the administrative values are copied to the pending operational
 * table (EDGX_SCHED_ST_PENDING_DELAYED).
 * Change of administrative values after that point in time
 * (EDGX_SCHED_ST_PENDING) have no effect on the already submitted
 * pending config change.
 */
static int edgx_sched_stm_cfg_change(struct edgx_st *sched)
{
	int ret = 0;
	bool time_in_past;
	unsigned long jif;
	time64_t delay_sec;

	if (!sched->config_change)
		return ret;

	if (sched->state == EDGX_SCHED_ST_PENDING)
		edgx_sched_hw_cancel_pending(sched->iobase_pt);

	if (sched->state == EDGX_SCHED_ST_PENDING_DELAYED)
		cancel_delayed_work_sync(&sched->hnd_delayed);

	sched->state = EDGX_SCHED_ST_IDLE;

	if (!sched->gate_enabled)
		return ret;

	ret = edgx_sched_check_cc_params(sched);
	if (ret) {
		sched->config_change = false;
		return ret;
	}

	edgx_sched_calc_cc_time(sched, &delay_sec, &time_in_past);
	if (time_in_past) {
		sched->conf_change_err++;
		edgx_pt_warn(sched->parent, "AdminBaseTime in the past!\n");
	}

	if (delay_sec) {
		sched->state = EDGX_SCHED_ST_PENDING_DELAYED;
		jif = msecs_to_jiffies(delay_sec * MSEC_PER_SEC);
		schedule_delayed_work(&sched->hnd_delayed, jif);
		sched_dbg("SCHED: Delayed work started\n delay_sec=%llu\n",
			  delay_sec);
	} else {
		ret = edgx_sched_config_change(sched);
		if (!ret) {
			sched->state = EDGX_SCHED_ST_PENDING;
			edgx_st_calc_trans_rate(sched);
		}
	}
	sched->config_change = false;
	return ret;
}

static int edgx_sched_stm_gate_enable(struct edgx_st *sched)
{
	int ret = 0;
	int tab_idx;

	if (sched->state == EDGX_SCHED_ST_IDLE) {
		edgx_sched_hw_enable(sched->iobase_pt,
				     sched->admin_tab.gate_states);
		/* Notify only if there is a running schedule */
		tab_idx = edgx_sched_hw_get_used_tab(sched->iobase_pt);
		if (tab_idx >= 0)
			edgx_sched_notify(sched);

		if (sched->config_change)
			ret = edgx_sched_stm_cfg_change(sched);
	}
	return ret;
}

/* NOTE: Deviation from the standard:
 * Gate enable/disable is simulated by the MUX. The scheduler itself
 * is not disabled. After re-enabling the MUX, the schedule continues
 * at an arbitrary point in the control list.
 */
static void edgx_sched_stm_gate_disable(struct edgx_st *sched)
{
	if (sched->state == EDGX_SCHED_ST_PENDING)
		edgx_sched_hw_cancel_pending(sched->iobase_pt);
	else if (sched->state == EDGX_SCHED_ST_PENDING_DELAYED)
		cancel_delayed_work_sync(&sched->hnd_delayed);

	edgx_sched_hw_disable(sched->iobase_pt, sched->admin_tab.gate_states);
	sched->state = EDGX_SCHED_ST_IDLE;
	edgx_sched_notify(sched);
}

static void edgx_sched_stm_pending_delayed(struct work_struct *work)
{
	struct edgx_st *sched =
			container_of(work, struct edgx_st, hnd_delayed.work);

	sched_dbg("Pending_delayed work.\n");
	mutex_lock(&sched->lock);
	sched->state = EDGX_SCHED_ST_IDLE;
	edgx_sched_stm_cfg_change(sched);
	mutex_unlock(&sched->lock);
}

/** Initialize the scheduler. */
static int edgx_sched_init(struct edgx_st *sched, struct edgx_pt *pt,
			   struct edgx_st_com *sched_com,
			   edgx_io_t *iobase_pt)
{
	int i;

	sched->parent = pt;
	sched->com = sched_com;
	sched->iobase_pt = iobase_pt;
	sched->sched_idx = edgx_pt_get_id(pt);

	mutex_init(&sched->lock);
	sched->link_mode = SPEED_UNKNOWN;
	sched->down_cnt_speed = EDGX_SCHED_HW_DC_1000FULL;
	sched->admin_tab.gate_states = EDGX_SCHED_DEF_GATE_STATES &
				       sched_com->gate_st_msk;
	sched->admin_tab.cycle_time_denom = 1;

	for (i = 0; i < sched->com->nr_queues; i++) {
		sched->tr_rate[i].num = 1;
		sched->tr_rate[i].denom = 1;
		edgx_pt_set_framesize(sched->parent, i, EDGX_SCHED_DEF_MAX_SDU);
	}
	INIT_DELAYED_WORK(&sched->hnd_delayed, edgx_sched_stm_pending_delayed);

	return edgx_sched_hw_init(iobase_pt, sched->com->ns_per_clk,
				  EDGX_SCHED_DEF_GATE_STATES &
				  sched_com->gate_st_msk);
}

/** Get FSC HW port specific interface. */
static const
struct edgx_ifdesc *edgx_sched_if2ptif(const struct edgx_ifdesc *ifdesc,
				       ptid_t pt,
				       struct edgx_ifdesc *ptifdesc)
{
	if (!ifdesc || !ptifdesc || (!(ifdesc->ptmap & BIT(pt))))
		return NULL;

	ptifdesc->id      = ifdesc->id;
	ptifdesc->ver     = ifdesc->ver;
	ptifdesc->len     = EDGX_SCHED_PT_RES_LEN;
	ptifdesc->iobase  = ifdesc->iobase + EDGX_SCHED_PT_RES_OFFS +
			    (pt * EDGX_SCHED_PT_RES_LEN);
	ptifdesc->ptmap   = BIT(pt);

	return ptifdesc;
}

static ssize_t edgx_sched_strtotimespec64(const char *buf,
					  size_t count,
					  struct timespec64 *time)
{
	char str[64], str_nsec[10];
	char *token, *str_p = str;
	s64 sec;
	long nsec = 0;
	ssize_t max_size = min_t(ssize_t, 63U, count);

	strncpy(str, buf, max_size);
	str[max_size] = '\0';
	token = strsep(&str_p, ".");
	if (!token)
		return -EINVAL;

	if (kstrtos64(token, 10, &sec))
		return -EINVAL;

	token = strsep(&str_p, "\n\r\t ");
	if (token) {
		if (strlen(token) > 9)
			return -EINVAL;
		/* Add trailing zeros to nsec part */
		scnprintf(str_nsec, 10, "%s%.*d", token,
			  (int)(9 - strlen(token)), 0);
		if (kstrtol(str_nsec, 10, &nsec))
			return -EINVAL;
	}
	time->tv_sec = sec;
	time->tv_nsec = nsec;
	return count;
}

/******************************************************************************
 * User Interface
 *****************************************************************************/

int edgx_st_get_trans_rate_locked(struct edgx_st *sched,
				  unsigned int queue_idx,
				  struct edgx_st_tr_rate *tr_rate)
{
	if (!sched->gate_enabled) {
		tr_rate->num = 1;
		tr_rate->denom = 1;
		return 0;
	}
	tr_rate->num = sched->tr_rate[queue_idx].num;
	tr_rate->denom = sched->tr_rate[queue_idx].denom;
	return 0;
}

int edgx_st_get_trans_rate(struct edgx_st *sched,
			   unsigned int queue_idx,
			   struct edgx_st_tr_rate *tr_rate)
{
	if (!sched || !tr_rate)
		return -EINVAL;

	if (queue_idx >= sched->com->nr_queues)
		return -EINVAL;

	mutex_lock(&sched->lock);
	edgx_st_get_trans_rate_locked(sched, queue_idx, tr_rate);
	mutex_unlock(&sched->lock);
	return 0;
}

static ssize_t current_time_show(struct device *dev,
				 struct device_attribute *attr,
				 char *buf)
{
	struct timespec64 ts;
	struct edgx_time *time = edgx_pt_get_time(edgx_dev2pt(dev));

	edgx_tm_get_wrk_time(time, &ts);
	return scnprintf(buf, PAGE_SIZE, "%lli.%09li\n",
			 (long long)ts.tv_sec, ts.tv_nsec);
}

static ssize_t gate_enabled_show(struct device *dev,
				 struct device_attribute *attr,
				 char *buf)
{
	ssize_t ret;
	struct edgx_st *sched = edgx_dev2sched(dev);

	mutex_lock(&sched->lock);
	ret = scnprintf(buf, PAGE_SIZE, "%u\n", sched->gate_enabled);
	mutex_unlock(&sched->lock);

	return ret;
}

static ssize_t gate_enabled_store(struct device *dev,
				  struct device_attribute *attr,
				  const char *buf,
				  size_t count)
{
	int ret = 0;
	struct edgx_st *sched = edgx_dev2sched(dev);

	mutex_lock(&sched->lock);

	if (kstrtobool(buf, &sched->gate_enabled))
		ret = -EINVAL;
	else if (sched->gate_enabled)
		ret = edgx_sched_stm_gate_enable(sched);
	else
		edgx_sched_stm_gate_disable(sched);

	mutex_unlock(&sched->lock);

	return ret ? ret : count;
}

static ssize_t admin_gate_states_show(struct device *dev,
				      struct device_attribute *attr,
				      char *buf)
{
	ssize_t ret;
	struct edgx_st *sched = edgx_dev2sched(dev);

	mutex_lock(&sched->lock);
	ret = scnprintf(buf, PAGE_SIZE, "0x%X\n", sched->admin_tab.gate_states);
	mutex_unlock(&sched->lock);

	return ret;
}

static ssize_t admin_gate_states_store(struct device *dev,
				       struct device_attribute *attr,
				       const char *buf,
				       size_t count)
{
	ssize_t ret = -EINVAL;
	struct edgx_st *sched = edgx_dev2sched(dev);

	mutex_lock(&sched->lock);
	if (!kstrtou8(buf, 0, &sched->admin_tab.gate_states))
		ret = count;
	sched->admin_tab.gate_states &= sched->com->gate_st_msk;
	mutex_unlock(&sched->lock);

	return ret;
}

static ssize_t max_sdu_tab_read(struct file *filp, struct kobject *kobj,
				struct bin_attribute *bin_attr,
				char *buf, loff_t ofs, size_t count)
{
	struct device *dev = kobj_to_dev(kobj);
	struct edgx_st *sched = edgx_dev2sched(dev);
	loff_t idx = 0;

	if (edgx_sysfs_tbl_params(ofs, count, sizeof(u32), &idx) ||
	    idx >= sched->com->nr_queues)
		return -EINVAL;

	/* NOTE: FLEXDE-2329: Standard Deviation:
	 * FRAMESIZEx registers does not account for VLAN- and R-TAG
	 */
	*((u32 *)buf) = edgx_pt_get_framesize(sched->parent, idx);

#if defined(EDGX_SCHED_DBG)
	mutex_lock(&sched->lock);
	edgx_sched_dump(sched);
	edgx_sched_dump_tab(&sched->admin_tab, "Admin");
	edgx_sched_dump_tab(&sched->op_tabs[0], "Oper 0");
	edgx_sched_hw_dump(sched->com->iobase, sched->iobase_pt,
			   sched->sched_idx, 0);
	edgx_sched_dump_tab(&sched->op_tabs[1], "Oper 1");
	edgx_sched_hw_dump(sched->com->iobase, sched->iobase_pt,
			   sched->sched_idx, 1);
	mutex_unlock(&sched->lock);
#endif
	return count;
}

static ssize_t max_sdu_tab_write(struct file *filp, struct kobject *kobj,
				 struct bin_attribute *bin_attr,
				 char *buf, loff_t ofs, size_t count)
{
	struct device *dev = kobj_to_dev(kobj);
	struct edgx_st *sched = edgx_dev2sched(dev);
	loff_t idx = 0;
	u32 val = *((u32 *)buf);

	if (edgx_sysfs_tbl_params(ofs, count, sizeof(u32), &idx) ||
	    idx >= sched->com->nr_queues)
		return -EINVAL;

	if (val > EDGX_SCHED_DEF_MAX_SDU)
		return -ERANGE;

	/* NOTE: FLEXDE-2329: Standard Deviation:
	 * FRAMESIZEx registers does not account for VLAN- and R-TAG
	 */
	edgx_pt_set_framesize(sched->parent, idx, val);
	return count;
}

static ssize_t overrun_sdu_tab_read(struct file *filp, struct kobject *kobj,
				    struct bin_attribute *bin_attr,
				    char *buf, loff_t ofs, size_t count)
{
	struct device *dev = kobj_to_dev(kobj);
	struct edgx_st *sched = edgx_dev2sched(dev);
	loff_t idx = 0;

	if (edgx_sysfs_tbl_params(ofs, count, sizeof(statw_t), &idx) ||
	    idx >= sched->com->nr_queues)
		return -EINVAL;

	/* NOTE: FLEXDE-451: Standard Deviation:
	 * The IP provides only one counters per port,
	 * the standard defines one per queue, so 8 per port.
	 */
	*((statw_t *)buf) = edgx_stat_upget(sched->hstat, 0);
	return count;
}

static ssize_t admin_ctrl_list_len_show(struct device *dev,
					struct device_attribute *attr,
					char *buf)
{
	ssize_t ret;
	struct edgx_st *sched = edgx_dev2sched(dev);

	mutex_lock(&sched->lock);
	ret = scnprintf(buf, PAGE_SIZE, "%u\n", sched->admin_tab.list_len);
	mutex_unlock(&sched->lock);

	return ret;
}

static ssize_t admin_ctrl_list_len_store(struct device *dev,
					 struct device_attribute *attr,
					 const char *buf,
					 size_t count)
{
	u32 list_len;
	struct edgx_st *sched = edgx_dev2sched(dev);

	if (kstrtou32(buf, 10, &list_len))
		return -EINVAL;

	if (list_len > sched->com->max_entry_cnt)
		return -ERANGE;

	mutex_lock(&sched->lock);
	sched->admin_tab.list_len = list_len;
	mutex_unlock(&sched->lock);

	return count;
}

static ssize_t oper_ctrl_list_len_show(struct device *dev,
				       struct device_attribute *attr,
				       char *buf)
{
	int row_cnt;
	struct edgx_st *sched = edgx_dev2sched(dev);
	int op_tab_idx;

	mutex_lock(&sched->lock);
	op_tab_idx = edgx_sched_hw_get_used_tab(sched->iobase_pt);

	if (op_tab_idx >= 0) {
		row_cnt = sched->op_tabs[op_tab_idx].list_len;
	} else {
		// no schedule in operation! return 0 as list length
		row_cnt = 0;
	}
	mutex_unlock(&sched->lock);

	return scnprintf(buf, PAGE_SIZE, "%d\n", row_cnt);
}

static ssize_t admin_ctrl_list_read(struct file *filp, struct kobject *kobj,
				    struct bin_attribute *bin_attr,
				    char *buf, loff_t ofs, size_t count)
{
	struct device *dev = kobj_to_dev(kobj);
	struct edgx_st *sched = edgx_dev2sched(dev);
	struct edgx_sched_tab_entry *ent = (struct edgx_sched_tab_entry *)buf;
	struct edgx_sched_tab_entry undef_entry = {0, 0xff, 0, 0};
	loff_t idx, nmax;
	size_t nelems;
	int i, ret = 0;

	mutex_lock(&sched->lock);

	if (!sched->admin_tab.entry_cnt)
		ret = -ENOENT;

	if (!ret)
		ret = edgx_sysfs_list_params(ofs, count,
					     sizeof(*sched->admin_tab.entries),
					     &idx, &nelems);
	if (!ret) {
		nmax = min((u32)(idx + nelems), sched->admin_tab.entry_cnt);
		for (i = 0; idx < nmax; i++, idx++)
			ent[i] = sched->admin_tab.entries[idx];

		for (; i < nelems; i++)
			ent[i] = undef_entry;
		ret = i * sizeof(struct edgx_sched_tab_entry);
	}
	mutex_unlock(&sched->lock);

	return ret;
}

static ssize_t edgx_sched_set_cl_entry(struct edgx_st *sched,
				       loff_t ofs, size_t count,
				       struct edgx_sched_tab_entry *entry)
{
	struct edgx_sched_tab_entry *new_entries;
	loff_t idx;
	size_t nelems;
	int i;
	ssize_t ret;

	if (edgx_sysfs_list_params(ofs, count,
				   sizeof(*sched->admin_tab.entries),
				   &idx, &nelems))
		return -EINVAL;

	if ((idx + nelems) > sched->com->max_entry_cnt)
		return -EFBIG;

	/* Clear table on entry[0] access */
	if (idx == 0) {
		sched->admin_tab.entry_cnt = 0;
		kfree(sched->admin_tab.entries);
		sched->admin_tab.entries = 0;
	}

	/* Enforce consecutive write */
	if (idx != sched->admin_tab.entry_cnt)
		return -EINVAL;

	new_entries = krealloc(sched->admin_tab.entries,
			       (idx + nelems) *
			       sizeof(struct edgx_sched_tab_entry),
			       GFP_KERNEL);
	if (!new_entries)
		return -ENOMEM;

	sched->admin_tab.entries = new_entries;
	sched->admin_tab.entry_cnt = idx + nelems;
	for (i = 0; i < nelems; i++, idx++)
		sched->admin_tab.entries[idx] = entry[i];

	ret = edgx_sched_adjust_admin_tab(sched);

	return ret ? ret : (nelems * sizeof(struct edgx_sched_tab_entry));
}

static ssize_t admin_ctrl_list_write(struct file *filp, struct kobject *kobj,
				     struct bin_attribute *bin_attr,
				     char *buf, loff_t ofs, size_t count)
{
	struct device *dev = kobj_to_dev(kobj);
	struct edgx_st *sched = edgx_dev2sched(dev);
	ssize_t ret;

	mutex_lock(&sched->lock);
	ret = edgx_sched_set_cl_entry(sched, ofs, count,
				      (struct edgx_sched_tab_entry *)buf);
	mutex_unlock(&sched->lock);
	return ret;
}

static ssize_t edgx_sched_get_op_entry(struct edgx_st *sched,
				       loff_t ofs, size_t count,
				       struct edgx_sched_tab_entry *entry)
{
	u16 int_clk;
	int tab_idx = edgx_sched_hw_get_used_tab(sched->iobase_pt);
	struct edgx_sched_tab_entry undef_entry = {0, 0xff, 0, 0};
	loff_t idx, nmax;
	size_t nelems;
	int i;
	u16 hw_gate_states, new_hold, hold = 0;

	if ((tab_idx < 0) || (!sched->op_tabs[tab_idx].list_len))
		return -ENOENT;

	if (edgx_sysfs_list_params(ofs, count,
				   sizeof(struct edgx_sched_tab_entry),
				   &idx, &nelems))
		return -EINVAL;

	nmax = min((u32)(idx + nelems), sched->op_tabs[tab_idx].list_len);
	for (i = 0; idx < nmax; i++, idx++) {
		if (edgx_sched_hw_read_entry(sched->com->iobase,
					     sched->sched_idx, tab_idx, idx,
					     &hw_gate_states,
					     &int_clk))
			return -EBUSY;
		entry[i].time_interval = (u32)int_clk * sched->com->ns_per_clk;
		entry[i].gate_states = hw_gate_states & sched->com->gate_st_msk;
		new_hold = hw_gate_states & edgx_sched_get_hold(sched);
		if (!hold && new_hold)
			entry[i].operation_name = EDGX_SCHED_OP_HOLD_MAC;
		else if (hold && !new_hold)
			entry[i].operation_name = EDGX_SCHED_OP_REL_MAC;
		else
			entry[i].operation_name = EDGX_SCHED_OP_SET_GSTATES;
		hold = new_hold;
	}
	for (; i < nelems; i++)
		entry[i] = undef_entry;

	return i * sizeof(struct edgx_sched_tab_entry);
}

static ssize_t oper_ctrl_list_read(struct file *filp, struct kobject *kobj,
				   struct bin_attribute *bin_attr,
				   char *buf, loff_t ofs, size_t count)
{
	struct device *dev = kobj_to_dev(kobj);
	struct edgx_st *sched = edgx_dev2sched(dev);
	ssize_t ret;

	mutex_lock(&sched->lock);
	ret = edgx_sched_get_op_entry(sched, ofs, count,
				      (struct edgx_sched_tab_entry *)buf);
	mutex_unlock(&sched->lock);

	return ret;
}

static ssize_t admin_cycle_time_show(struct device *dev,
				     struct device_attribute *attr,
				     char *buf)
{
	ssize_t ret;
	struct edgx_st *sched = edgx_dev2sched(dev);

	mutex_lock(&sched->lock);
	ret = scnprintf(buf, PAGE_SIZE, "%u/%u\n",
			sched->admin_tab.cycle_time_num,
			sched->admin_tab.cycle_time_denom);
	mutex_unlock(&sched->lock);

	return ret;
}

static ssize_t admin_cycle_time_store(struct device *dev,
				      struct device_attribute *attr,
				      const char *buf,
				      size_t count)
{
	int ret;
	u32 num, denom;
	u64 ct_nsec;
	u32 ct_subnsec;
	struct edgx_st *sched = edgx_dev2sched(dev);

	mutex_lock(&sched->lock);
	ret = sscanf(buf, "%u/%u", &num, &denom);

	if (ret == 2U) {
		if (denom) {
			edgx_sched_rational_to_nsec(num, denom,
						    &ct_nsec, &ct_subnsec);

			if (ct_nsec <= sched->com->max_cyc_time_ns) {
				sched->admin_tab.cycle_time_num = num;
				sched->admin_tab.cycle_time_denom = denom;
				ret = count;
			} else {
				sched->conf_change_err++;
				ret = -ERANGE;
			}
		} else {
			ret = -EINVAL;
		}
	}

	mutex_unlock(&sched->lock);
	return (ssize_t)ret;
}

static ssize_t oper_cycle_time_show(struct device *dev,
				    struct device_attribute *attr,
				    char *buf)
{
	ssize_t ret = -ENOENT;
	struct edgx_st *sched = edgx_dev2sched(dev);
	int tab_idx;

	mutex_lock(&sched->lock);
	tab_idx = edgx_sched_hw_get_used_tab(sched->iobase_pt);

	if (tab_idx >= 0)
		ret = scnprintf(buf, PAGE_SIZE, "%u/%u\n",
				sched->op_tabs[tab_idx].cycle_time_num,
				sched->op_tabs[tab_idx].cycle_time_denom);

	mutex_unlock(&sched->lock);
	return ret;
}

static ssize_t admin_cycle_time_ext_show(struct device *dev,
					 struct device_attribute *attr,
					 char *buf)
{
	/* NOTE: FLEXDE-4962: Standard Deviation:
	 * Not supported, report as zero.
	 */
	return scnprintf(buf, PAGE_SIZE, "%d\n", 0);
}

static ssize_t admin_cycle_time_ext_store(struct device *dev,
					  struct device_attribute *attr,
					  const char *buf,
					  size_t count)
{
	struct edgx_st *st = edgx_dev2sched(dev);
	unsigned int ext_ns;
	int ret;

	/* NOTE: FLEXDE-4962: Standard Deviation:
	 * Not supported, log warning if not zero.
	 */
	ret = kstrtouint(buf, 10, &ext_ns);
	if (ret == 0 && ext_ns != 0)
		edgx_pt_warn(st->parent,
			     "CycleTimeExtension not supported, using zero!\n");
	return ret ? ret : count;
}

static ssize_t oper_cycle_time_ext_show(struct device *dev,
					struct device_attribute *attr,
					char *buf)
{
	/* NOTE: FLEXDE-4962: Standard Deviation:
	 * Not supported, report as zero.
	 */
	return scnprintf(buf, PAGE_SIZE, "%d\n", 0);
}

static ssize_t admin_base_time_show(struct device *dev,
				    struct device_attribute *attr,
				    char *buf)
{
	ssize_t ret;
	struct edgx_st *sched = edgx_dev2sched(dev);

	mutex_lock(&sched->lock);
	ret = scnprintf(buf, PAGE_SIZE, "%lli.%09li\n",
			(long long)sched->admin_tab.base_time.tv_sec,
			sched->admin_tab.base_time.tv_nsec);

	mutex_unlock(&sched->lock);
	return ret;
}

static ssize_t admin_base_time_store(struct device *dev,
				     struct device_attribute *attr,
				     const char *buf,
				     size_t count)
{
	ssize_t ret;
	struct edgx_st *sched = edgx_dev2sched(dev);

	mutex_lock(&sched->lock);
	ret = edgx_sched_strtotimespec64(buf, count,
					 &sched->admin_tab.base_time);

	mutex_unlock(&sched->lock);
	return ret;
}

static ssize_t oper_base_time_show(struct device *dev,
				   struct device_attribute *attr,
				   char *buf)
{
	ssize_t ret = -ENOENT;
	struct edgx_st *sched = edgx_dev2sched(dev);
	int tab_idx;
	struct edgx_sched_table *op;

	mutex_lock(&sched->lock);
	tab_idx = edgx_sched_hw_get_used_tab(sched->iobase_pt);

	if (tab_idx >= 0) {
		op = &sched->op_tabs[tab_idx];
		ret = scnprintf(buf, PAGE_SIZE, "%lli.%09li\n",
				(long long)op->base_time.tv_sec,
				op->base_time.tv_nsec);
	}
	mutex_unlock(&sched->lock);
	return ret;
}

static ssize_t config_change_show(struct device *dev,
				  struct device_attribute *attr,
				  char *buf)
{
	ssize_t ret;
	struct edgx_st *sched = edgx_dev2sched(dev);

	mutex_lock(&sched->lock);
	ret = scnprintf(buf, PAGE_SIZE, "%u\n", sched->config_change);
	mutex_unlock(&sched->lock);

	return ret;
}

static ssize_t config_change_store(struct device *dev,
				   struct device_attribute *attr,
				   const char *buf,
				   size_t count)
{
	int ret = -EINVAL;
	struct edgx_st *sched = edgx_dev2sched(dev);

	mutex_lock(&sched->lock);
	if (!kstrtobool(buf, &sched->config_change))
		ret = edgx_sched_stm_cfg_change(sched);

	mutex_unlock(&sched->lock);
	return ret ? ret : count;
}

static ssize_t config_change_time_show(struct device *dev,
				       struct device_attribute *attr,
				       char *buf)
{
	ssize_t ret;
	struct edgx_st *sched = edgx_dev2sched(dev);

	mutex_lock(&sched->lock);
	ret = scnprintf(buf, PAGE_SIZE, "%lli.%09li\n",
			(long long)sched->conf_change_time.tv_sec,
			sched->conf_change_time.tv_nsec);
	mutex_unlock(&sched->lock);
	return ret;
}

static ssize_t tick_granul_show(struct device *dev,
				struct device_attribute *attr,
				char *buf)
{
	ssize_t ret;
	struct edgx_st *sched = edgx_dev2sched(dev);

	mutex_lock(&sched->lock);

	sched->link_mode = edgx_pt_get_speed(sched->parent);

	/* Report in 0.1ns units: */
	switch (sched->link_mode) {
	case SPEED_10:
		ret = scnprintf(buf, PAGE_SIZE, "10000000\n");
		break;
	case SPEED_100:
		ret = scnprintf(buf, PAGE_SIZE, "1000000\n");
		break;
	case SPEED_1000:
		ret = scnprintf(buf, PAGE_SIZE, "100000\n");
		break;
	default:
		ret = scnprintf(buf, PAGE_SIZE, "UNKNOWN\n");
	}

	mutex_unlock(&sched->lock);
	return ret;
}

static ssize_t config_pending_show(struct device *dev,
				   struct device_attribute *attr,
				   char *buf)
{
	ssize_t ret;
	struct edgx_st *sched = edgx_dev2sched(dev);

	mutex_lock(&sched->lock);
	ret = scnprintf(buf, PAGE_SIZE, "%u\n", sched->state);
	mutex_unlock(&sched->lock);
	return ret;
}

static ssize_t config_change_error_show(struct device *dev,
					struct device_attribute *attr,
					char *buf)
{
	ssize_t ret;
	struct edgx_st *sched = edgx_dev2sched(dev);

	mutex_lock(&sched->lock);
	ret = scnprintf(buf, PAGE_SIZE, "%llu\n", sched->conf_change_err);
	mutex_unlock(&sched->lock);

	return ret;
}

static ssize_t supported_list_max_show(struct device *dev,
				       struct device_attribute *attr,
				       char *buf)
{
	ssize_t ret;
	struct edgx_st *sched = edgx_dev2sched(dev);

	mutex_lock(&sched->lock);
	ret = scnprintf(buf, PAGE_SIZE, "%u\n", sched->com->max_entry_cnt);
	mutex_unlock(&sched->lock);
	return ret;
}

static ssize_t supported_cyc_max_show(struct device *dev,
				      struct device_attribute *attr,
				      char *buf)
{
	ssize_t ret;
	struct edgx_st *sched = edgx_dev2sched(dev);
	u32 sec_num, sec_denom;

	mutex_lock(&sched->lock);
	edgx_sched_nsec_to_rational(sched->com->max_cyc_time_ns, 0,
				    &sec_num, &sec_denom);
	ret = scnprintf(buf, PAGE_SIZE, "%u/%u\n", sec_num, sec_denom);
	mutex_unlock(&sched->lock);

	return ret;
}

static ssize_t supported_int_max_show(struct device *dev,
				      struct device_attribute *attr,
				      char *buf)
{
	ssize_t ret;
	struct edgx_st *sched = edgx_dev2sched(dev);
	u32 max_int = (u32)sched->com->ns_per_clk * EDGX_SCHED_INTERVAL_MAX;

	mutex_lock(&sched->lock);
	ret = scnprintf(buf, PAGE_SIZE, "%u\n", max_int);
	mutex_unlock(&sched->lock);

	return ret;
}

EDGX_DEV_ATTR_RW(gate_enabled,         "GateEnabled");
EDGX_DEV_ATTR_RW(admin_gate_states,    "AdminGateStates");
EDGX_DEV_ATTR_RW(admin_ctrl_list_len,  "AdminControlListLength");
EDGX_DEV_ATTR_RO(oper_ctrl_list_len,   "OperControlListLength");
EDGX_DEV_ATTR_RW(admin_cycle_time,     "AdminCycleTime");
EDGX_DEV_ATTR_RO(oper_cycle_time,      "OperCycleTime");
EDGX_DEV_ATTR_RW(admin_cycle_time_ext, "AdminCycleTimeExtension");
EDGX_DEV_ATTR_RO(oper_cycle_time_ext,  "OperCycleTimeExtension");
EDGX_DEV_ATTR_RW(admin_base_time,      "AdminBaseTime");
EDGX_DEV_ATTR_RO(oper_base_time,       "OperBaseTime");
EDGX_DEV_ATTR_RW(config_change,        "ConfigChange");
EDGX_DEV_ATTR_RO(config_change_time,   "ConfigChangeTime");
EDGX_DEV_ATTR_RO(tick_granul,          "TickGranularity");
EDGX_DEV_ATTR_RO(current_time,         "CurrentTime");
EDGX_DEV_ATTR_RO(config_pending,       "ConfigPending");
EDGX_DEV_ATTR_RO(config_change_error,  "ConfigChangeError");
EDGX_DEV_ATTR_RO(supported_list_max,   "SupportedListMax");
EDGX_DEV_ATTR_RO(supported_cyc_max,    "SupportedCycleMax");
EDGX_DEV_ATTR_RO(supported_int_max,    "SupportedIntervalMax");
EDGX_BIN_ATTR_RW(admin_ctrl_list,      "AdminControlList",
		 EDGX_SCHED_HW_MAX_ROWS * sizeof(struct edgx_sched_tab_entry));
EDGX_BIN_ATTR_RO(oper_ctrl_list,       "OperControlList",
		 EDGX_SCHED_HW_MAX_ROWS * sizeof(struct edgx_sched_tab_entry));
EDGX_BIN_ATTR_RW(max_sdu_tab,          "queueMaxSDUTable",
		 EDGX_ST_MAX_QUEUES * sizeof(u32));
EDGX_BIN_ATTR_RO(overrun_sdu_tab,      "transmissionOverrunTable",
		 EDGX_ST_MAX_QUEUES * sizeof(statw_t));

static struct attribute_group ieee8021_st_group = {
	.name = "ieee8021ST",
	.attrs = (struct attribute*[]){
		&dev_attr_gate_enabled.attr,
		&dev_attr_admin_gate_states.attr,
		&dev_attr_admin_ctrl_list_len.attr,
		&dev_attr_oper_ctrl_list_len.attr,
		&dev_attr_admin_cycle_time.attr,
		&dev_attr_oper_cycle_time.attr,
		&dev_attr_admin_cycle_time_ext.attr,
		&dev_attr_oper_cycle_time_ext.attr,
		&dev_attr_admin_base_time.attr,
		&dev_attr_oper_base_time.attr,
		&dev_attr_config_change.attr,
		&dev_attr_config_change_time.attr,
		&dev_attr_tick_granul.attr,
		&dev_attr_current_time.attr,
		&dev_attr_config_pending.attr,
		&dev_attr_config_change_error.attr,
		&dev_attr_supported_list_max.attr,
		&dev_attr_supported_cyc_max.attr,
		&dev_attr_supported_int_max.attr,
		NULL
	},
	.bin_attrs = (struct bin_attribute*[]){
		&bin_attr_admin_ctrl_list,
		&bin_attr_oper_ctrl_list,
		&bin_attr_max_sdu_tab,
		&bin_attr_overrun_sdu_tab,
		NULL
	}
};

/* NOTE: The current FSC HW ITF is not completely separated on port basis.
 * There is a common part, a generic part and a port specific part.
 * Therefore edgx_ac_get_ptif() cannot be used.
 */

/* NOTE: Bug in AC: On IP 4.7 the AC block returns FSC resource size 0x10000.
 * The real resource size however is 0x15000 for 4 (+1) ports.
 */

/* NOTE: Missing from the Manual: On IP 4.7 the port number matches
 * the scheduler number. On older IPs, the first available scheduler
 * matches the first available port, that supports scheduling.
 */

int edgx_probe_st(struct edgx_pt *pt, struct edgx_st_com *sched_com,
		  struct edgx_st **psched)
{
	const struct edgx_ifdesc *ifd_pt;
	struct edgx_ifdesc        ptifd;
	ptid_t                    ptid = edgx_pt_get_id(pt);
	struct edgx_stat         *sm = edgx_br_get_pt_stat(edgx_pt_get_br(pt),
							   ptid);
	struct edgx_time *time = edgx_pt_get_time(pt);

	if (!pt || !sched_com || !psched || !time)
		return -EINVAL;

	/* Get the port specific part of the HW ITF */
	ifd_pt = edgx_sched_if2ptif(sched_com->ifd_com, ptid, &ptifd);
	if (!ifd_pt)
		return -ENODEV;

	*psched = kzalloc(sizeof(**psched), GFP_KERNEL);
	if (!(*psched)) {
		edgx_pt_err(pt, "Cannot allocate Scheduled Traffic\n");
		return -ENOMEM;
	}

	if (edgx_sched_init(*psched, pt, sched_com, ifd_pt->iobase))
		goto out_sched_init;

	(*psched)->hstat = edgx_stat_alloc_hdl(sm, ptid, &_st_statinfo);

	edgx_pt_add_sysfs(pt, &ieee8021_st_group);
	sched_com->sched_list[ptid] = *psched;

	return 0;

out_sched_init:
	kfree(*psched);
	return -ENODEV;
}

void edgx_shutdown_st(struct edgx_st *sched)
{
	if (sched) {
		edgx_pt_rem_sysfs(sched->parent, &ieee8021_st_group);
		edgx_stat_free_hdl(sched->hstat);
		edgx_sched_hw_disable(sched->iobase_pt,
				      sched->admin_tab.gate_states);
		kfree(sched->admin_tab.entries);
		kfree(sched);
	}
}

/* dummy functions, the "proper" twins are in fscr.c */
int edgx_sched_calc_delays(struct edgx_st *st)
{
	(void)st;
	return 0;
}

u64 edgx_st_get_part_indep_tx_dly_hi_min(struct edgx_st *st)
{
	(void)st;
	return 0;
}

u64 edgx_st_get_part_indep_tx_dly_hi_max(struct edgx_st *st)
{
	(void)st;
	return 0;
}

u64 edgx_st_get_part_indep_tx_dly_lo_min(struct edgx_st *st)
{
	(void)st;
	return 0;
}

u64 edgx_st_get_part_indep_tx_dly_lo_max(struct edgx_st *st)
{
	(void)st;
	return 0;
}

u64 edgx_st_get_freq_err_per_cyc(struct edgx_st *st)
{
	(void)st;
	return 0;
}
