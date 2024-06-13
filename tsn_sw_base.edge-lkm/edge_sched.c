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
#include "edge_sched.h"
#include "edge_time.h"
#include "edge_bridge.h"
#include "edge_fqtss.h"
#include "edge_util.h"

#define EDGX_SCHED_DBG

#if defined(EDGX_SCHED_DBG)
#define sched_dbg edgx_dbg
#else
#define sched_dbg(args, ...) do { } while (0)
#endif

/* FSC HW ITF address definitions */

#define EDGX_SCHED_GENERAL		(0x10)
#define EDGX_SCHED_PT_RES_LEN		(0x1000)
#define EDGX_SCHED_PT_RES_OFFS		(0x10000)
#define EDGX_SCHED_OF_GEN_REGS		(0x4000)
#define EDGX_SCHED_OF_SCHEDULERS	((EDGX_SCHED_OF_GEN_REGS) + 0x0)
#define EDGX_SCHED_OF_GATE_CNT		((EDGX_SCHED_OF_GEN_REGS) + 0x002)
#define EDGX_SCHED_GATE_CNT_MASK	(0x7f)
#define EDGX_SCHED_OF_ROW_CNT		((EDGX_SCHED_OF_GEN_REGS) + 0x004)
#define EDGX_SCHED_ROW_CNT_MASK		(0x0f)
#define EDGX_SCHED_OF_DELAY_CHAIN	((EDGX_SCHED_OF_GEN_REGS) + 0x006)
#define EDGX_SCHED_OF_HOLD		((EDGX_SCHED_OF_GEN_REGS) + 0x008)
#define EDGX_SCHED_OF_S2G_MIN_320	((EDGX_SCHED_OF_GEN_REGS) + 0x200)
#define EDGX_SCHED_OF_S2G_MIN_3200	((EDGX_SCHED_OF_GEN_REGS) + 0x202)
#define EDGX_SCHED_OF_S2G_MAX_320	((EDGX_SCHED_OF_GEN_REGS) + 0x208)
#define EDGX_SCHED_OF_S2G_MAX_3200	((EDGX_SCHED_OF_GEN_REGS) + 0x20a)
#define EDGX_SCHED_TAB_0		(0x800)
#define EDGX_SCHED_TAB_1		(0x900)
#define EDGX_SCHED_TBL_OFS	       ((EDGX_SCHED_TAB_1) - (EDGX_SCHED_TAB_0))
#define EDGX_SCHED_TBL_GEN		((EDGX_SCHED_TAB_0) + 0x0)
#define EDGX_SCHED_CAN_USE_MASK		(0x0001)
#define EDGX_SCHED_IN_USE_MASK		(0x0002)
#define EDGX_SCHED_START_TIME_NSEC_L	((EDGX_SCHED_TAB_0) + 0x014)
#define EDGX_SCHED_START_TIME_NSEC_H	((EDGX_SCHED_TAB_0) + 0x016)
#define EDGX_SCHED_START_TIME_SEC	((EDGX_SCHED_TAB_0) + 0x018)
#define EDGX_SCHED_CYC_TIME_NSEC_L	((EDGX_SCHED_TAB_0) + 0x024)
#define EDGX_SCHED_CYC_TIME_NSEC_H	((EDGX_SCHED_TAB_0) + 0x026)
#define EDGX_SCHED_CYC_TIME_SUBNS	((EDGX_SCHED_TAB_0) + 0x020)
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
#define EDGX_SCHED_DELAY		(0x0)
#define EDGX_SCHED_G_CLOSE_ADV_LO	(0x10)
#define EDGX_SCHED_G_CLOSE_ADV_HI	(0x12)
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
#define EDGX_SCHED_MAX_DELAY		(64U)
#define EDGX_SCHED_MIN_ADVANCE		(32U)
#define EDGX_SCHED_INTERVAL_MAX		(0xffff)
#define EDGX_SCHED_INT_MASK		(0x1100)
#define EDGX_SCHED_INT_STAT		(0x1102)
#define EDGX_SCHED_INT_MSKVAL		(0x01)
#define EDGX_SCHED_DEF_START_TIME_ERR	(256)
#define EDGX_SCHED_MAX_CYC_MS		(16)
#define EDGX_SCHED_MAX_CYC_NS		(EDGX_SCHED_MAX_CYC_MS * NSEC_PER_MSEC)
/** Number of HW scheduling tables */
#define EDGX_SCHED_HW_TAB_CNT		(2U)
/* Maximum possible number of schedule table rows */
#define EDGX_SCHED_HW_MAX_ROWS		(1024U)

/** Scheduler state machine states */
enum edgx_sched_state {
	EDGX_SCHED_ST_IDLE = 0,	/* Idle state, no pending schedule */
	EDGX_SCHED_ST_PENDING,	/* New schedule submitted and pending in HW */
	EDGX_SCHED_ST_PENDING_DELAYED/* New schedule pending in SW
				      * and waiting to be submitted to HW
				      */
};

/** Schedule table */
struct edgx_sched_table {
	struct edgx_sched_tab_entry *entries;	    /* Table rows */
	u32			     entry_cnt;     /* Number of entries */
	u32			     list_len;	    /* Number of used entries */
	struct timespec64	     base_time;	    /* Base time */
	struct edgx_sched_rational   cycle_time;    /* Cycle time */
	u8			     gate_states;   /* Initial gate states*/
};

/** Scheduler */
struct edgx_sched {
	struct mutex		 lock;		   /* Protect edgx_sched */
	struct edgx_sched_com   *com;
	struct edgx_sched_event	*sched_evt;

	edgx_io_t		*iobase;     /* Scheduler HW base address */
	int			 sched_idx;  /* FSC scheduler number */

	struct edgx_sched_table	 admin_tab;  /* The admin schedule table */
	/* The operational schedule table mirrors of the HW schedule tables */
	struct edgx_sched_table	 op_tabs[EDGX_SCHED_HW_TAB_CNT];

	bool			 gate_enabled;	   /* Gate enabled */
	bool			 conf_change;	   /* Config change requested */
	s64			 offset_ns;	   /* Scheduler time offset */
	struct timespec64	 conf_change_time; /* Config change time */
	struct timespec64	 start_time;	   /* Schedule start time */
	enum edgx_sched_state	 state;		   /* Scheduler state */
	struct delayed_work	 hnd_delayed;	   /* Pending delayed work */
	u64			 conf_change_err;
};

/** Scheduler common part */
struct edgx_sched_com {
	struct edgx_br		 *parent;
	const struct edgx_ifdesc *ifd_com;
	struct mutex		  com_itf_lock;
	edgx_io_t		 *iobase;	  /* Common HW base address */
	struct edgx_sched	 *sched_list[EDGX_BR_MAX_PORTS];
	struct edgx_br_irq	 *irq;
	enum edgx_br_irq_nr	  irq_nr;
	struct work_struct	  work_isr;
	struct workqueue_struct	 *wq_isr;
	struct edgx_sched_limits  limits;
	u16			  init_gate_states;
	u8			  delay_chain;
	const struct edgx_sched_ops *ops;
};

const struct edgx_sched_tab_entry edgx_sched_undef_entry = {
	0, 0xff, 0, 0,
};

unsigned int edgx_sched_get_idx(struct edgx_sched *sched)
{
	return sched->sched_idx;
}

static u16 edgx_sched_hw_get_delay_chain(edgx_io_t *base)
{
	u16 val;

	val = edgx_rd16(base, EDGX_SCHED_OF_DELAY_CHAIN);

	return val;
}

/** Get the number of rows per table from FSC HW ITF. */
static u16 edgx_sched_get_hw_row_cnt(edgx_io_t *base)
{
	u16 val;

	val = edgx_rd16(base, EDGX_SCHED_OF_ROW_CNT);
	val = val & EDGX_SCHED_ROW_CNT_MASK;

	return (1 << val);
}

static u16 edgx_sched_get_hw_gate_cnt(edgx_io_t *base)
{
	u16 val;

	val = edgx_rd16(base, EDGX_SCHED_OF_GATE_CNT);
	val = val & EDGX_SCHED_GATE_CNT_MASK;

	return val;
}

static void edgx_sched_set_hw_granularity(edgx_io_t *base,
					  u32 sched_granularity)
{
	edgx_wr16(base, EDGX_SCHED_GENERAL,
		  (sched_granularity == 3200) ? 1 : 0);
}

static int edgx_sched_hw_init(edgx_io_t *base, u8 gate_states)
{
	edgx_wr16(base, EDGX_SCHED_EME_DIS_STAT0, (u16)gate_states);
	edgx_wr16(base, EDGX_SCHED_EME_DIS_STAT1, 0);
	edgx_wr16(base, EDGX_SCHED_EME_DIS_STAT2, 0);
	edgx_wr16(base, EDGX_SCHED_EME_DIS_STAT3, 0);
	edgx_wr16(base, EDGX_SCHED_EME_DIS_CTRL, EDGX_SCHED_EME_DIS_ON);

	return 0;
}

static bool edgx_sched_hw_is_pending(edgx_io_t *base)
{
	u16 pending_0 = edgx_rd16(base, EDGX_SCHED_TBL_GEN) &
			EDGX_SCHED_CAN_USE_MASK;
	u16 pending_1 = edgx_rd16(base,
				  EDGX_SCHED_TBL_GEN + EDGX_SCHED_TBL_OFS) &
			EDGX_SCHED_CAN_USE_MASK;

	return pending_0 || pending_1;
}

static void edgx_sched_hw_cancel_pending(edgx_io_t *base)
{
	u16 tbl_gen = edgx_rd16(base, EDGX_SCHED_TBL_GEN);

	edgx_wr16(base, EDGX_SCHED_TBL_GEN,
		  tbl_gen & ~EDGX_SCHED_CAN_USE_MASK);

	tbl_gen = edgx_rd16(base, EDGX_SCHED_TBL_GEN + EDGX_SCHED_TBL_OFS);
	edgx_wr16(base, EDGX_SCHED_TBL_GEN + EDGX_SCHED_TBL_OFS,
		  tbl_gen & ~EDGX_SCHED_CAN_USE_MASK);
}

/** Get the first free table index */
int edgx_sched_get_free_tab(struct edgx_sched *sched)
{
	edgx_io_t *base = sched->iobase;
	u16 in_use = edgx_rd16(base, EDGX_SCHED_TBL_GEN) &
		     EDGX_SCHED_IN_USE_MASK;

	if (!in_use)
		return 0;

	in_use = edgx_rd16(base, EDGX_SCHED_TBL_GEN + EDGX_SCHED_TBL_OFS) &
		 EDGX_SCHED_IN_USE_MASK;
	if (!in_use)
		return 1;

	return -EINVAL;
}

/** Get the first used table index */
int edgx_sched_get_used_tab(struct edgx_sched *sched)
{
	edgx_io_t *base = sched->iobase;
	u16 in_use = edgx_rd16(base, EDGX_SCHED_TBL_GEN) &
		     EDGX_SCHED_IN_USE_MASK;

	if (in_use)
		return 0;

	in_use = edgx_rd16(base, EDGX_SCHED_TBL_GEN + EDGX_SCHED_TBL_OFS) &
		 EDGX_SCHED_IN_USE_MASK;
	if (in_use)
		return 1;

	return -EINVAL;
}

/** Submit a new configuration */
static void edgx_sched_hw_set_pending(edgx_io_t *base,
				      unsigned int tab_idx,
				      const struct timespec64 *base_time,
				      u32 cycle_nsec)
{
	u16 tbl_gen;

	sched_dbg("SET PENDING: Cycle time: nsec=%u\n",
		  cycle_nsec);
	sched_dbg("SET PENDING: Start time: sec=%u, nsec=%u\n",
		  (u16)base_time->tv_sec & EDGX_SCHED_START_TIME_SEC_MASK,
		  (u32)base_time->tv_nsec);

	edgx_wr16(base, EDGX_SCHED_CYC_TIME_NSEC_L +
		  (tab_idx * EDGX_SCHED_TBL_OFS),
		  (u16)(cycle_nsec & 0xffff));

	edgx_wr16(base, EDGX_SCHED_CYC_TIME_NSEC_H +
		  (tab_idx * EDGX_SCHED_TBL_OFS),
		  (u16)(cycle_nsec >> 16));

	edgx_wr16(base,
		  EDGX_SCHED_START_TIME_SEC + (tab_idx * EDGX_SCHED_TBL_OFS),
		  (u16)base_time->tv_sec & EDGX_SCHED_START_TIME_SEC_MASK);
	edgx_wr16(base,
		  EDGX_SCHED_START_TIME_NSEC_L + (tab_idx * EDGX_SCHED_TBL_OFS),
		  (u16)((u32)base_time->tv_nsec & 0xffff));
	edgx_wr16(base,
		  EDGX_SCHED_START_TIME_NSEC_H + (tab_idx * EDGX_SCHED_TBL_OFS),
		  (u16)((u32)base_time->tv_nsec >> 16));

	tbl_gen = edgx_rd16(base,
			    EDGX_SCHED_TBL_GEN +
			    (tab_idx * EDGX_SCHED_TBL_OFS));
	edgx_wr16(base, EDGX_SCHED_TBL_GEN + (tab_idx * EDGX_SCHED_TBL_OFS),
		  (tbl_gen | EDGX_SCHED_CAN_USE_MASK) & ~EDGX_SCHED_STOP_LAST);
}

static void edgx_sched_hw_get_cc_time(edgx_io_t *base, unsigned int tab_idx,
				      struct timespec64 *time)
{
	u16 start_sec;
	u32 start_nsec;

	start_sec = edgx_rd16(base,
			      EDGX_SCHED_START_TIME_SEC +
			      (tab_idx * EDGX_SCHED_TBL_OFS));
	start_nsec = (u32)edgx_rd16(base,
				    EDGX_SCHED_START_TIME_NSEC_L +
				    (tab_idx * EDGX_SCHED_TBL_OFS));
	start_nsec |= (u32)edgx_rd16(base,
				     EDGX_SCHED_START_TIME_NSEC_H +
				     (tab_idx * EDGX_SCHED_TBL_OFS)) << 16;

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

int edgx_sched_write_entry(struct edgx_sched *sched,
			   unsigned int tab_idx, unsigned int row_idx,
			   u16 gate_states, u32 time_ns)
{
	edgx_io_t *base = sched->com->iobase;
	u16 cmd;
	int ret;

	sched_dbg("write_entry: sched_idx=%d, tab_idx=%u, row_idx=%u, time_ns=%u",
		  sched->sched_idx, tab_idx, row_idx, time_ns);

	mutex_lock(&sched->com->com_itf_lock);
	edgx_wr16(base, EDGX_SCHED_ROW_AC_CMD1,
		  row_idx & EDGX_SCHED_ROW_AC_CMD1_MASK);
	edgx_wr16(base, EDGX_SCHED_ROW_DATA_OUT0, gate_states);
	edgx_wr16(base, EDGX_SCHED_ROW_DATA_CYCLES,
		  (u16)(time_ns / sched->com->limits.hw_granularity_ns));

	cmd = (sched->sched_idx & EDGX_SCHED_CMD_SCHED_MASK) |
	      ((tab_idx & EDGX_SCHED_CMD_TAB_MASK)
	       << EDGX_SCHED_CMD_TAB_MASK_SHIFT) |
	      EDGX_SCHED_AC_WRITE | EDGX_SCHED_AC_TRANSFER;
	edgx_wr16(base, EDGX_SCHED_ROW_AC_CMD0, cmd);
	ret = edgx_sched_hw_transfer_wait(base);
	mutex_unlock(&sched->com->com_itf_lock);
	return ret;
}

int edgx_sched_read_entry(struct edgx_sched *sched,
			  unsigned int tab_idx, unsigned int row_idx,
			  u16 *gate_states, u32 *time_ns)
{
	edgx_io_t *base = sched->com->iobase;
	u16 cmd;
	int ret;

	if (row_idx >= sched->op_tabs[tab_idx].list_len)
		return -ENOENT;

	mutex_lock(&sched->com->com_itf_lock);
	edgx_wr16(base, EDGX_SCHED_ROW_AC_CMD1,
		  row_idx & EDGX_SCHED_ROW_AC_CMD1_MASK);

	cmd = (sched->sched_idx & EDGX_SCHED_CMD_SCHED_MASK) |
	      ((tab_idx & EDGX_SCHED_CMD_TAB_MASK)
	       << EDGX_SCHED_CMD_TAB_MASK_SHIFT) |
	      EDGX_SCHED_AC_TRANSFER;
	edgx_wr16(base, EDGX_SCHED_ROW_AC_CMD0, cmd);

	ret = edgx_sched_hw_transfer_wait(base);
	if (!ret) {
		*gate_states = edgx_rd16(base, EDGX_SCHED_ROW_DATA_OUT0);
		*time_ns = edgx_rd16(base, EDGX_SCHED_ROW_DATA_CYCLES) *
			   sched->com->limits.hw_granularity_ns;
	}
	mutex_unlock(&sched->com->com_itf_lock);
	return ret;
}

static void edgx_sched_hw_disable(edgx_io_t *base, u8 gate_states)
{
	edgx_wr16(base, EDGX_SCHED_EME_DIS_STAT0, (u16)gate_states);
	edgx_wr16(base, EDGX_SCHED_EME_DIS_CTRL, EDGX_SCHED_EME_DIS_ON);
}

static void edgx_sched_hw_enable(edgx_io_t *base, u8 gate_states)
{
	edgx_wr16(base, EDGX_SCHED_EME_DIS_STAT0, (u16)gate_states);
	edgx_wr16(base, EDGX_SCHED_EME_DIS_CTRL, EDGX_SCHED_EME_DIS_OFF);
}

#if defined(EDGX_SCHED_DBG)
static void edgx_sched_hw_dump(struct edgx_sched *sched, unsigned int tab_idx)
{
	edgx_io_t *base = sched->com->iobase;
	edgx_io_t *base_pt = sched->iobase;
	u16 start_sec;
	u32 start_nsec;
	u16 tbl_gen;
	int i, ret;
	u16 states;
	u32 clk;
	u32 cyc_nsec;
	u16 eme_dis;

	sched_dbg("DEV_ID0=0x%x, DEV_ID1=0x%x, INT_ID0=0x%x, INT_ID1=0x%x\n",
		  edgx_rd16(base, 0x0),
		  edgx_rd16(base, 0x2),
		  edgx_rd16(base, 0x4),
		  edgx_rd16(base, 0x6));
	sched_dbg("GENERAL=0x%x\n", edgx_rd16(base, EDGX_SCHED_GENERAL));
	sched_dbg("INT_MASK=0x%x\n", edgx_rd16(base, EDGX_SCHED_INT_MASK));
	sched_dbg("INT_STAT=0x%x\n", edgx_rd16(base, EDGX_SCHED_INT_STAT));
	sched_dbg("SCHEDULERS=0x%x\n",
		  edgx_rd16(base, EDGX_SCHED_OF_SCHEDULERS));
	sched_dbg("OUTPUTS=0x%x\n", edgx_rd16(base, EDGX_SCHED_OF_GATE_CNT));
	sched_dbg("TABLE_ROWS=0x%x\n", edgx_rd16(base, EDGX_SCHED_OF_ROW_CNT));
	sched_dbg("DELAY_CHAIN=0x%x\n",
		  edgx_rd16(base, EDGX_SCHED_OF_DELAY_CHAIN));
	sched_dbg("HOLD=0x%x\n", edgx_rd16(base, EDGX_SCHED_OF_HOLD));
	sched_dbg("OF_S2G_MIN_320=0x%x\n",
		  edgx_rd16(base, EDGX_SCHED_OF_S2G_MIN_320));
	sched_dbg("S2G_MIN_3200=0x%x\n",
		  edgx_rd16(base, EDGX_SCHED_OF_S2G_MIN_3200));
	sched_dbg("OF_S2G_MAX_320=0x%x\n",
		  edgx_rd16(base, EDGX_SCHED_OF_S2G_MAX_320));
	sched_dbg("S2G_MAX_3200=0x%x\n",
		  edgx_rd16(base, EDGX_SCHED_OF_S2G_MAX_3200));

	sched_dbg("DELAY=0x%x\n",
		  edgx_rd16(base_pt, EDGX_SCHED_DELAY));
	sched_dbg("G_CLOSE_ADV_LO=0x%x\n",
		  edgx_rd16(base_pt, EDGX_SCHED_G_CLOSE_ADV_LO));
	sched_dbg("G_CLOSE_ADV_HI=0x%x\n",
		  edgx_rd16(base_pt, EDGX_SCHED_G_CLOSE_ADV_HI));

	eme_dis = edgx_rd16(base_pt, EDGX_SCHED_EME_DIS_CTRL);

	start_sec = edgx_rd16(base_pt,
			      EDGX_SCHED_START_TIME_SEC +
			      (tab_idx * EDGX_SCHED_TBL_OFS));
	start_nsec = edgx_rd16(base_pt,
			       EDGX_SCHED_START_TIME_NSEC_L +
			       (tab_idx * EDGX_SCHED_TBL_OFS));
	start_nsec |= (u32)edgx_rd16(base_pt,
				     EDGX_SCHED_START_TIME_NSEC_H +
				     (tab_idx * EDGX_SCHED_TBL_OFS)) << 16;
	tbl_gen = edgx_rd16(base_pt,
			    EDGX_SCHED_TBL_GEN +
			    (tab_idx * EDGX_SCHED_TBL_OFS));

	cyc_nsec = edgx_rd16(base_pt, EDGX_SCHED_CYC_TIME_NSEC_L +
			     (tab_idx * EDGX_SCHED_TBL_OFS));
	cyc_nsec |= (u32)edgx_rd16(base_pt, EDGX_SCHED_CYC_TIME_NSEC_H +
				   (tab_idx * EDGX_SCHED_TBL_OFS)) << 16;

	sched_dbg("SCHED: tbl_gen=0x%x, start_sec=%u, start_nsec=%u\n",
		  tbl_gen, start_sec, start_nsec);
	sched_dbg("SCHED: cyc_nsec=%u\n",
		  cyc_nsec);
	sched_dbg("SCHED: EMEDIS=0x%x\n", eme_dis);

	for (i = 0; i < 30; i++) {
		ret = edgx_sched_read_entry(sched, tab_idx, i, &states, &clk);
		sched_dbg("SCHED: TAB0, row%d ret=%d state=0x%x, time_int=%u\n",
			  i, ret, states, clk);
	}
}
#endif

static void edgx_sched_stm_cct_handler(struct edgx_sched *sched);

static void edgx_sched_isr_work(struct work_struct *work)
{
	struct edgx_sched_com *sc = container_of(work, struct edgx_sched_com,
						 work_isr);
	u16 intstat = edgx_rd16(sc->iobase, EDGX_SCHED_INT_STAT);
	int i;

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
	struct edgx_sched_com *sc = (struct edgx_sched_com *)device;
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

int edgx_sched_com_probe(struct edgx_br *br, struct edgx_br_irq *irq,
			 enum edgx_br_irq_nr irq_nr,
			 const char *drv_name,
			 const struct edgx_ifdesc *ifd_com,
			 struct edgx_sched_com **psc,
			 const struct edgx_sched_ops *ops,
			 u16 init_gate_states)
{
	int ret = 0;

	if (!br || !psc)
		return -EINVAL;

	if (!ifd_com || !ifd_com->ptmap)
		return -ENODEV;

	*psc = kzalloc(sizeof(**psc), GFP_KERNEL);
	if (!(*psc)) {
		edgx_br_err(br, "Cannot allocate Common Scheduled Traffic\n");
		return -ENOMEM;
	}

	(*psc)->parent = br;
	(*psc)->ifd_com = ifd_com;
	(*psc)->iobase = (*psc)->ifd_com->iobase;
	(*psc)->limits.max_entry_cnt =
		edgx_sched_get_hw_row_cnt((*psc)->iobase) - 1;
	(*psc)->limits.nr_gates = edgx_sched_get_hw_gate_cnt((*psc)->iobase);
	(*psc)->limits.max_cyc_ms = EDGX_SCHED_MAX_CYC_MS;
	(*psc)->limits.max_int_len = EDGX_SCHED_INTERVAL_MAX;
	(*psc)->delay_chain = edgx_sched_hw_get_delay_chain((*psc)->iobase);
	(*psc)->init_gate_states = init_gate_states;
	(*psc)->ops = ops;
	mutex_init(&(*psc)->com_itf_lock);

	(*psc)->irq = irq;
	(*psc)->irq_nr = irq_nr;
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
		ret = request_irq(irq->irq_vec[irq_nr],
				  &edgx_sched_isr, IRQF_SHARED, drv_name, *psc);
	if (ret) {
		pr_err("%s(): request_irq failed! ret=%d, irq=%d\n",
		       __func__, ret, irq->shared ?
		       irq->irq_vec[0] : irq->irq_vec[irq_nr]);
		destroy_workqueue((*psc)->wq_isr);
		return ret;
	}

	edgx_wr16((*psc)->iobase, EDGX_SCHED_INT_STAT, 0);
	edgx_wr16((*psc)->iobase, EDGX_SCHED_INT_MASK, EDGX_SCHED_INT_MSKVAL);

	sched_dbg("FSC cfg: max_entry_cnt=%d, delay_chain =%d\n",
		  (*psc)->limits.max_entry_cnt, (*psc)->delay_chain);
	return ret;
}

void edgx_sched_com_set_params(struct edgx_sched_com *sched_com,
			       u32 freq_err_abs, u32 sched_granul)
{
	sched_com->limits.hw_granularity_ns = sched_granul;
	edgx_sched_set_hw_granularity(sched_com->iobase, sched_granul);
	sched_com->limits.freq_err_abs = freq_err_abs;
}

void edgx_sched_com_shutdown(struct edgx_sched_com *sched_com)
{
	if (sched_com) {
		struct edgx_br_irq *irq = sched_com->irq;

		if (irq->shared)
			free_irq(irq->irq_vec[0], sched_com);
		else
			free_irq(irq->irq_vec[sched_com->irq_nr], sched_com);

		cancel_work_sync(&sched_com->work_isr);
		edgx_wr16(sched_com->iobase, EDGX_SCHED_INT_MASK, 0);
		edgx_wr16(sched_com->iobase, EDGX_SCHED_INT_STAT, 0);
		destroy_workqueue(sched_com->wq_isr);
		kfree(sched_com);
	}
}

#if defined(EDGX_SCHED_DBG)
static void edgx_sched_dump(struct edgx_sched *sched)
{
	sched_dbg("sched_idx: %d\n", sched->sched_idx);
	sched_dbg("iobase_com: 0x%p; iobase_sched: 0x%p\n",
		  sched->com->iobase, sched->iobase);
	sched_dbg("max_entry_cnt: %u\n", sched->com->limits.max_entry_cnt);
	sched_dbg("hw_granularity_ns: %d\n", sched->com->limits.hw_granularity_ns);
	sched_dbg("gate_enabled: %d\n", sched->gate_enabled);
	sched_dbg("config_change: %d\n", sched->conf_change);
	sched_dbg("conf_change_time: sec=%lli, nsec=%li\n",
		  sched->conf_change_time.tv_sec,
		  sched->conf_change_time.tv_nsec);
	sched_dbg("start_time: sec=%lli, nsec=%li\n",
		  sched->start_time.tv_sec, sched->start_time.tv_nsec);
	sched_dbg("state: %d\n", sched->state);
	sched_dbg("conf_change_err: %llu\n", sched->conf_change_err);
}

static void edgx_sched_dump_tab(struct edgx_sched_table *tab, const char *label)
{
	int i;

	sched_dbg("TABLE %s\n", label);
	sched_dbg("entry_cnt: %d\n", tab->entry_cnt);
	sched_dbg("list_len: %d\n", tab->list_len);
	sched_dbg("gate_states: 0x%x\n", tab->gate_states);
	sched_dbg("cycle_time: %u/%u\n",
		  tab->cycle_time.num, tab->cycle_time.denom);
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

void edgx_sched_rational_to_nsec(const struct edgx_sched_rational *rat,
				 u64 *nsec,
				 u32 *subnsec)
{
	u32 cycle_sec;
	u64 num = rat->num;
	u64 rem = do_div(num, rat->denom);

	cycle_sec = num;
	num = (u64)rem * NSEC_PER_SEC;
	rem = do_div(num, rat->denom);
	*nsec = num + (cycle_sec * NSEC_PER_SEC);

	num = (u64)rem << 32;
	rem = do_div(num, rat->denom);
	*subnsec = num;
}

void edgx_sched_nsec_to_rational(u64 nsec, u32 subnsec,
				 struct edgx_sched_rational *rational)
{
	/* TODO: Add a real conversion algorithm */
	rational->num = nsec;
	rational->denom = NSEC_PER_SEC;
}

static void edgx_sched_notify(struct edgx_sched *sched)
{
	sched_dbg("Notify\n");
	if (sched->com->ops->config_changed)
		sched->com->ops->config_changed(sched->sched_evt);
}

static ssize_t edgx_sched_adjust_admin_tab(struct edgx_sched *sched)
{
	struct edgx_sched_table	*tab = &sched->admin_tab;
	u32 hw_time_interval;
	int i;

	for (i = 0; i < tab->entry_cnt; i++) {
		if (tab->entries[i].time_interval %
		    sched->com->limits.hw_granularity_ns) {
			sched->conf_change_err++;
			edgx_br_err(sched->com->parent,
				    "Control List Interval is not a multiple of TickGranularity!\n");
			return -EINVAL;
		}
		hw_time_interval = tab->entries[i].time_interval /
				   sched->com->limits.hw_granularity_ns;
		if (hw_time_interval > EDGX_SCHED_INTERVAL_MAX) {
			sched->conf_change_err++;
			edgx_br_err(sched->com->parent,
				    "Control List Interval out of range!\n");
			return -EINVAL;
		}
	}
	return 0;
}

static int edgx_sched_check_cc_params(struct edgx_sched *sched)
{
	int ret;

	if ((!sched->admin_tab.entries) ||
	    (sched->admin_tab.entry_cnt == 0) ||
	    (sched->admin_tab.cycle_time.num == 0) ||
	    (sched->admin_tab.list_len == 0) ||
	    (sched->admin_tab.entry_cnt < sched->admin_tab.list_len))
		return -EFAULT;

	ret = sched->com->ops->get_sched_offset(sched->sched_evt,
						&sched->offset_ns);
	if (ret)
		return ret;

	return 0;
}

/* Calculate the number of cycles between two points in time.
 *
 * Algorithm taken from deipce_fsc_hw.c:
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
				     const struct edgx_sched_rational *ct,
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

	count_sec = delta.tv_sec * ct->denom;
	count_sec_rem = do_div(count_sec, ct->num);

	count_nsec = (u64)delta.tv_nsec * ct->denom;
	count_nsec_rem = do_div(count_nsec, ct->num);

	count_res = (u64)count_sec_rem * NSEC_PER_SEC +
			count_nsec * ct->num + count_nsec_rem;
	if (round_up)
		count_res += (u64)ct->num * NSEC_PER_SEC - 1;
	do_div(count_res, ct->num);
	do_div(count_res, NSEC_PER_SEC);

	count = count_sec + count_res;

	return count;
}

static void edgx_sched_calc_future_time(const struct timespec64 *in_time,
					const struct timespec64 *cur_time,
					struct timespec64 *out_time,
					const struct edgx_sched_rational *ct)
{
	u64 count;
	u64 num;
	u32 rem;
	struct timespec64 advance;

	count = edgx_sched_diff_to_cycles(in_time, cur_time, ct,
					  true, ~((u64)0));

	/* Add calculated number of cycle times to out_time */
	num = count * ct->num;
	rem = do_div(num, ct->denom);
	advance.tv_sec = num;

	num = (u64)rem * NSEC_PER_SEC;
	do_div(num, ct->denom);
	advance.tv_nsec = num;

	sched_dbg("EDGX_SCHED: %s() start0 %lli.%09li cycle_time %u/%u\n",
		  __func__, in_time->tv_sec, in_time->tv_nsec,
		  ct->num, ct->denom);
	sched_dbg("EDGX_SCHED: %s() cur_time %lli.%09li\n",
		  __func__, cur_time->tv_sec, cur_time->tv_nsec);
	sched_dbg("EDGX_SCHED: %s() count %llu advance %lli.%09li\n",
		  __func__, count, advance.tv_sec, advance.tv_nsec);

	*out_time = timespec64_add(*in_time, advance);

	sched_dbg("EDGX_SCHED: %s() out_time %lli.%09li\n",
		  __func__, out_time->tv_sec, out_time->tv_nsec);
}

static void edgx_sched_calc_cc_time(struct edgx_sched *sched,
				    time64_t *delay_sec,
				    bool *time_in_past)
{
	struct edgx_time *time = edgx_br_get_br_time(sched->com->parent);
	struct timespec64 cur_time;
	time64_t diff;

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
					    &sched->admin_tab.cycle_time);
		*time_in_past = true;
	} else {
		sched->start_time = sched->admin_tab.base_time;
	}

	sched->conf_change_time = sched->start_time;
	set_normalized_timespec64(&sched->start_time,
				  sched->start_time.tv_sec,
				  sched->start_time.tv_nsec + sched->offset_ns);

	diff = sched->start_time.tv_sec - cur_time.tv_sec;

	sched_dbg("EDGX_SCHED: %s() start_time %lli.%09li with offset %+lld\n",
		  __func__,
		  sched->start_time.tv_sec, sched->start_time.tv_nsec,
		  sched->offset_ns);

	if (diff > EDGX_SCHED_MAX_DELAY) {
		if (diff - EDGX_SCHED_MIN_ADVANCE < EDGX_SCHED_MAX_DELAY)
			*delay_sec = diff - EDGX_SCHED_MIN_ADVANCE;
		else
			*delay_sec = EDGX_SCHED_MAX_DELAY;
	}
}

static int edgx_sched_config_change(struct edgx_sched *sched)
{
	int ret = 0;
	struct edgx_sched_table *tab;
	struct edgx_sched_tab_entry *entries;
	u64 ct_nsec;
	u32 ct_subnsec;
	int tab_idx = edgx_sched_get_free_tab(sched);

	if (tab_idx < 0)
		return tab_idx;
	tab = &sched->op_tabs[tab_idx];
	*tab = sched->admin_tab;
	entries = tab->entries;
	tab->entries = NULL;

	ret = sched->com->ops->prepare_config(sched->sched_evt,
					      entries, tab->list_len);
	if (ret)
		return ret;

	edgx_sched_rational_to_nsec(&sched->op_tabs[tab_idx].cycle_time,
				    &ct_nsec,
				    &ct_subnsec);
	edgx_sched_hw_set_pending(sched->iobase, tab_idx,
				  &sched->start_time,
				  (u32)ct_nsec);
	return 0;
}

/* Check if HW adjusted the start time. Adjust CCT accordingly. */
static void edgx_sched_fix_cc_time(struct edgx_sched *sched)
{
	struct timespec64 hw_start_time;
	struct timespec64 start_time_reduced;
	s64 delta;
	int tab_idx = edgx_sched_get_used_tab(sched);

	if (tab_idx < 0)
		return;

	edgx_sched_hw_get_cc_time(sched->iobase, tab_idx, &hw_start_time);
	/* hw_cct has a HW limited range. Start time range must be reduced. */
	start_time_reduced = sched->start_time;
	start_time_reduced.tv_sec &= EDGX_SCHED_START_TIME_SEC_MASK;

	delta = timespec64_to_ns(&hw_start_time) -
		timespec64_to_ns(&start_time_reduced);

	sched_dbg("%s(): hw_start_time = %lli.%09li\n",
		  __func__, hw_start_time.tv_sec, hw_start_time.tv_nsec);
	sched_dbg("%s(): start_time_reduced = %lli.%09li\n",
		  __func__, start_time_reduced.tv_sec,
		  start_time_reduced.tv_nsec);
	sched_dbg("%s(): delta = %lli\n", __func__, delta);

	if (abs(delta) > EDGX_SCHED_DEF_START_TIME_ERR) {
		sched->conf_change_err++;
		timespec64_add_ns(&sched->start_time, delta);
		set_normalized_timespec64(&sched->conf_change_time,
					  sched->start_time.tv_sec,
					  sched->start_time.tv_nsec -
					  sched->offset_ns);
		edgx_br_warn(sched->com->parent,
			     "Start time adjusted by HW!\n");
	}
}

/******************************************************************************
 * State Machine Request Handlers
 *****************************************************************************/
static void edgx_sched_stm_cct_handler(struct edgx_sched *sched)
{
	mutex_lock(&sched->lock);
	if ((sched->state == EDGX_SCHED_ST_PENDING) &&
	    (!edgx_sched_hw_is_pending(sched->iobase))) {
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
static int edgx_sched_stm_cfg_change(struct edgx_sched *sched)
{
	bool time_in_past;
	unsigned long jif;
	time64_t delay_sec;
	int ret = 0;

	if (!sched->conf_change)
		return ret;

	if (sched->state == EDGX_SCHED_ST_PENDING)
		edgx_sched_hw_cancel_pending(sched->iobase);

	if (sched->state == EDGX_SCHED_ST_PENDING_DELAYED)
		cancel_delayed_work_sync(&sched->hnd_delayed);

	sched->state = EDGX_SCHED_ST_IDLE;

	if (!sched->gate_enabled)
		return ret;

	ret = edgx_sched_check_cc_params(sched);
	if (ret) {
		sched->conf_change = false;
		return ret;
	}

	edgx_sched_calc_cc_time(sched, &delay_sec, &time_in_past);
	if (time_in_past) {
		sched->conf_change_err++;
		edgx_br_warn(sched->com->parent,
			     "AdminBaseTime in the past!\n");
	}

	if (delay_sec) {
		sched->state = EDGX_SCHED_ST_PENDING_DELAYED;
		jif = msecs_to_jiffies(delay_sec * MSEC_PER_SEC);
		schedule_delayed_work(&sched->hnd_delayed, jif);
		sched_dbg("SCHED: Delayed work started delay_sec=%llu\n",
			  delay_sec);
	} else {
		ret = edgx_sched_config_change(sched);
		if (!ret) {
			sched->state = EDGX_SCHED_ST_PENDING;
		}
	}
	sched->conf_change = false;
	return ret;
}

static int edgx_sched_stm_gate_enable(struct edgx_sched *sched)
{
	int ret = 0;
	int tab_idx;

	if (sched->state == EDGX_SCHED_ST_IDLE) {
		edgx_sched_hw_enable(sched->iobase,
				     sched->admin_tab.gate_states);
		/* Notify only if there is a running schedule */
		tab_idx = edgx_sched_get_used_tab(sched);
		if (tab_idx >= 0)
			edgx_sched_notify(sched);

		if (sched->conf_change)
			ret = edgx_sched_stm_cfg_change(sched);
	}
	return ret;
}

/* NOTE: Deviation from the standard:
 * Gate enable/disable is simulated by the MUX. The scheduler itself
 * is not disabled. After re-enabling the MUX, the schedule continues
 * at an arbitrary point in the control list.
 */
static void edgx_sched_stm_gate_disable(struct edgx_sched *sched)
{
	if (sched->state == EDGX_SCHED_ST_PENDING)
		edgx_sched_hw_cancel_pending(sched->iobase);
	else if (sched->state == EDGX_SCHED_ST_PENDING_DELAYED)
		cancel_delayed_work_sync(&sched->hnd_delayed);

	edgx_sched_hw_disable(sched->iobase, sched->admin_tab.gate_states);
	sched->state = EDGX_SCHED_ST_IDLE;
	edgx_sched_notify(sched);
}

static void edgx_sched_stm_pending_delayed(struct work_struct *work)
{
	struct edgx_sched *sched =
			container_of(work, struct edgx_sched, hnd_delayed.work);

	sched_dbg("Pending_delayed work.\n");
	mutex_lock(&sched->lock);
	sched->state = EDGX_SCHED_ST_IDLE;
	edgx_sched_stm_cfg_change(sched);
	mutex_unlock(&sched->lock);
}

static int edgx_sched_init(struct edgx_sched *sched,
			   struct edgx_sched_com *sched_com,
			   unsigned int sched_idx,
			   edgx_io_t *iobase)
{
	sched->com = sched_com;
	sched->iobase = iobase;
	sched->sched_idx = sched_idx;

	mutex_init(&sched->lock);
	sched->admin_tab.gate_states = sched_com->init_gate_states;
	sched->admin_tab.cycle_time.denom = 1;

	INIT_DELAYED_WORK(&sched->hnd_delayed, edgx_sched_stm_pending_delayed);

	return edgx_sched_hw_init(iobase, sched->admin_tab.gate_states);
}

/** Get FSC HW scheduler specific interface. */
static const
struct edgx_ifdesc *edgx_sched_get_if(const struct edgx_ifdesc *ifd_com,
				      unsigned int sched_idx,
				      struct edgx_ifdesc *ifd_sched)
{
	if (!ifd_com || !ifd_sched)
		return NULL;

	ifd_sched->id     = ifd_com->id;
	ifd_sched->ver    = ifd_com->ver;
	ifd_sched->len    = EDGX_SCHED_PT_RES_LEN;
	ifd_sched->iobase = ifd_com->iobase + EDGX_SCHED_PT_RES_OFFS +
			    (sched_idx * EDGX_SCHED_PT_RES_LEN);
	ifd_sched->ptmap  = 0;

	return ifd_sched;
}

/******************************************************************************
 * User Interface
 *****************************************************************************/

void edgx_sched_lock(struct edgx_sched *sched)
{
	mutex_lock(&sched->lock);
}

void edgx_sched_unlock(struct edgx_sched *sched)
{
	mutex_unlock(&sched->lock);
}

struct edgx_br *edgx_sched_get_br(struct edgx_sched *sched)
{
	return sched->com->parent;
}

void edgx_sched_set_delay(struct edgx_sched *sched, u8 delay)
{
	edgx_wr16(sched->iobase, EDGX_SCHED_DELAY, delay);
}

void edgx_sched_set_g_close_adv(struct edgx_sched *sched,
				u16 g_cl_adv_lo, u16 g_cl_adv_hi)
{
	edgx_wr16(sched->iobase, EDGX_SCHED_G_CLOSE_ADV_LO, g_cl_adv_lo);
	edgx_wr16(sched->iobase, EDGX_SCHED_G_CLOSE_ADV_HI, g_cl_adv_hi);
}

int edgx_sched_get_s2g(struct edgx_sched *sched, u64 clk,
		       u64 *s2g_min, u64 *s2g_max)
{
	edgx_io_t *base = sched->com->iobase;
	u16 reg_min, reg_max;

	if (sched->com->limits.hw_granularity_ns == 320) {
		reg_min = edgx_rd16(base, EDGX_SCHED_OF_S2G_MIN_320);
		reg_max = edgx_rd16(base, EDGX_SCHED_OF_S2G_MAX_320);
		*s2g_min = ((u64)(reg_min & 0x0f) * clk) + (u64)(reg_min >> 4);
		*s2g_max = ((u64)(reg_max & 0x0f) * clk) + (u64)(reg_max >> 4);
		return 0;
	} else if (sched->com->limits.hw_granularity_ns == 3200) {
		reg_min = edgx_rd16(base, EDGX_SCHED_OF_S2G_MIN_3200);
		reg_max = edgx_rd16(base, EDGX_SCHED_OF_S2G_MAX_3200);
		*s2g_min = ((u64)(reg_min & 0x0f) * clk) + (u64)(reg_min >> 4);
		*s2g_max = ((u64)(reg_max & 0x0f) * clk) + (u64)(reg_max >> 4);
		return 0;
	}
	return -EINVAL;
}

const struct edgx_sched_limits *edgx_sched_get_limits(struct edgx_sched *sched)
{
	return &sched->com->limits;
}

int edgx_sched_get_current_time(struct edgx_sched *sched,
				struct timespec64 *ts)
{
	struct edgx_time *time = edgx_br_get_br_time(sched->com->parent);

	return edgx_tm_get_wrk_time(time, ts);
}

int edgx_sched_set_gate_enabled(struct edgx_sched *sched, bool enabled)
{
	int ret = 0;

	mutex_lock(&sched->lock);
	sched->gate_enabled = enabled;
	if (sched->gate_enabled)
		ret = edgx_sched_stm_gate_enable(sched);
	else
		edgx_sched_stm_gate_disable(sched);
	mutex_unlock(&sched->lock);
	return ret;
}

bool edgx_sched_get_gate_enabled_locked(struct edgx_sched *sched)
{
	return sched->gate_enabled;
}

bool edgx_sched_get_gate_enabled(struct edgx_sched *sched)
{
	bool gate_enabled;

	mutex_lock(&sched->lock);
	gate_enabled = sched->gate_enabled;
	mutex_unlock(&sched->lock);
	return gate_enabled;
}

int edgx_sched_set_admin_gate_states(struct edgx_sched *sched,
				     u8 gate_states,
				     u8 mask)
{
	mutex_lock(&sched->lock);
	sched->admin_tab.gate_states &= ~mask;
	sched->admin_tab.gate_states |= gate_states & mask;
	mutex_unlock(&sched->lock);
	return 0;
}

u8 edgx_sched_get_admin_gate_states(struct edgx_sched *sched, u8 mask)
{
	u8 gate_states;

	mutex_lock(&sched->lock);
	gate_states = sched->admin_tab.gate_states & mask;
	mutex_unlock(&sched->lock);
	return gate_states;
}

int edgx_sched_set_cycle_time(struct edgx_sched *sched,
			      const struct edgx_sched_rational *ct)
{
	int ret = -EINVAL;
	u64 ct_nsec;
	u32 ct_subnsec;

	mutex_lock(&sched->lock);
	if (ct->denom) {
		edgx_sched_rational_to_nsec(ct, &ct_nsec, &ct_subnsec);

		if (ct_nsec <= EDGX_SCHED_MAX_CYC_NS) {
			sched->admin_tab.cycle_time = *ct;
			ret = 0;
		} else {
			sched->conf_change_err++;
			ret = -ERANGE;
		}
	}
	mutex_unlock(&sched->lock);
	return ret;
}

int edgx_sched_get_cycle_time_locked(struct edgx_sched *sched,
				     enum edgx_sched_sel sel,
				     struct edgx_sched_rational *ct)
{
	int ret = -ENOENT;

	if (sel == EDGX_SCHED_ADMIN) {
		*ct = sched->admin_tab.cycle_time;
		ret = 0;
	} else {
		int tab_idx = edgx_sched_get_used_tab(sched);

		if (tab_idx >= 0) {
			*ct = sched->op_tabs[tab_idx].cycle_time;
			ret = 0;
		}
	}
	return ret;
}

int edgx_sched_get_cycle_time(struct edgx_sched *sched,
			      enum edgx_sched_sel sel,
			      struct edgx_sched_rational *ct)
{
	int ret;

	mutex_lock(&sched->lock);
	ret = edgx_sched_get_cycle_time_locked(sched, sel, ct);
	mutex_unlock(&sched->lock);
	return ret;
}

int edgx_sched_set_base_time(struct edgx_sched *sched,
			     const struct timespec64 *base_time)
{
	mutex_lock(&sched->lock);
	sched->admin_tab.base_time = *base_time;
	mutex_unlock(&sched->lock);
	return 0;
}

int edgx_sched_get_base_time(struct edgx_sched *sched,
			     enum edgx_sched_sel sel,
			     struct timespec64 *base_time)
{
	int ret = -ENOENT;

	mutex_lock(&sched->lock);
	if (sel == EDGX_SCHED_ADMIN) {
		*base_time = sched->admin_tab.base_time;
		ret = 0;
	} else {
		int tab_idx = edgx_sched_get_used_tab(sched);

		if (tab_idx >= 0) {
			*base_time = sched->op_tabs[tab_idx].base_time;
			ret = 0;
		}
	}
	mutex_unlock(&sched->lock);
	return ret;
}

int edgx_sched_set_ctrl_list_len(struct edgx_sched *sched,
				 u32 list_len)
{
	if (list_len > sched->com->limits.max_entry_cnt)
		return -ERANGE;

	mutex_lock(&sched->lock);
	sched->admin_tab.list_len = list_len;
	mutex_unlock(&sched->lock);
	return 0;
}

int edgx_sched_get_ctrl_list_len(struct edgx_sched *sched,
				 enum edgx_sched_sel sel,
				 u32 *list_len)
{
	mutex_lock(&sched->lock);
	if (sel == EDGX_SCHED_ADMIN) {
		*list_len = sched->admin_tab.list_len;
	} else {
		int tab_idx = edgx_sched_get_used_tab(sched);

		if (tab_idx >= 0) {
			*list_len = sched->op_tabs[tab_idx].list_len;
		} else {
			// no schedule in operation! return 0 as list length
			*list_len = 0;
		}
	}
	mutex_unlock(&sched->lock);
	return 0;
}

static int edgx_sched_set_entries(struct edgx_sched *sched,
				  size_t first,
				  const struct edgx_sched_tab_entry *entries,
				  size_t count)
{
	struct edgx_sched_tab_entry *new_entries;
	size_t i;
	int ret;

	if (first + count > sched->com->limits.max_entry_cnt)
		return -EFBIG;

	/* Clear table on entry[0] access */
	if (first == 0) {
		sched->admin_tab.entry_cnt = 0;
		kfree(sched->admin_tab.entries);
		sched->admin_tab.entries = NULL;
	}

	/* Enforce consecutive write */
	if (first != sched->admin_tab.entry_cnt)
		return -EINVAL;

	new_entries = krealloc(sched->admin_tab.entries,
			       (first + count) * sizeof(*entries),
			       GFP_KERNEL);
	if (!new_entries)
		return -ENOMEM;

	sched->admin_tab.entries = new_entries;
	sched->admin_tab.entry_cnt = first + count;
	for (i = 0; i < count; i++)
		sched->admin_tab.entries[first + i] = entries[i];

	ret = edgx_sched_adjust_admin_tab(sched);

	return ret;
}

int edgx_sched_set_admin_ctrl_list(struct edgx_sched *sched,
				   size_t first,
				   const struct edgx_sched_tab_entry *entries,
				   size_t count)
{
	int ret;

	mutex_lock(&sched->lock);
	ret = edgx_sched_set_entries(sched, first, entries, count);
	mutex_unlock(&sched->lock);
	return ret;
}

static int edgx_sched_get_entries(struct edgx_sched *sched,
				  size_t first,
				  struct edgx_sched_tab_entry *entries,
				  size_t count)
{
	size_t count_max;
	size_t i;

	if (!sched->admin_tab.entry_cnt)
		return -ENOENT;

	count_max = min(first + count, sched->admin_tab.entry_cnt);
	for (i = 0; i < count_max; i++)
		entries[i] = sched->admin_tab.entries[first + i];

	for (; i < count; i++)
		entries[i] = edgx_sched_undef_entry;
	return 0;
}

int edgx_sched_get_admin_ctrl_list(struct edgx_sched *sched,
				   size_t first,
				   struct edgx_sched_tab_entry *entries,
				   size_t count)
{
	int ret = 0;

	mutex_lock(&sched->lock);
	ret = edgx_sched_get_entries(sched, first, entries, count);
	mutex_unlock(&sched->lock);
	return ret;
}

int edgx_sched_set_config_change(struct edgx_sched *sched, bool config_change)
{
	int ret;

	mutex_lock(&sched->lock);
	sched->conf_change = config_change;
	ret = edgx_sched_stm_cfg_change(sched);
	mutex_unlock(&sched->lock);
	return ret;
}

int edgx_sched_get_config_change(struct edgx_sched *sched, bool *config_change)
{
	mutex_lock(&sched->lock);
	*config_change = sched->conf_change;
	mutex_unlock(&sched->lock);
	return 0;
}

int edgx_sched_get_config_change_time(struct edgx_sched *sched,
				      struct timespec64 *cct)
{
	mutex_lock(&sched->lock);
	*cct = sched->conf_change_time;
	mutex_unlock(&sched->lock);
	return 0;
}

int edgx_sched_get_config_pending(struct edgx_sched *sched, bool *pending)
{
	mutex_lock(&sched->lock);
	*pending = sched->state != EDGX_SCHED_ST_IDLE;
	mutex_unlock(&sched->lock);
	return 0;
}

int edgx_sched_get_config_change_error(struct edgx_sched *sched,
				       u64 *cc_error)
{
	mutex_lock(&sched->lock);
	*cc_error = sched->conf_change_err;
	mutex_unlock(&sched->lock);
	return 0;
}

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

int edgx_probe_sched(struct edgx_sched_com *sched_com,
		     unsigned int sched_idx,
		     struct edgx_sched_event *sched_evt)
{
	const struct edgx_ifdesc *ifd_sched;
	struct edgx_ifdesc        ifd_tmp;
	struct edgx_sched	 *sched;
	struct edgx_time         *time;

	if (!sched_com || !sched_evt)
		return -EINVAL;

	time = edgx_br_get_br_time(sched_com->parent);
	if (!time)
		return -EINVAL;

	/* Get the scheduler specific part of the HW ITF */
	ifd_sched = edgx_sched_get_if(sched_com->ifd_com, sched_idx, &ifd_tmp);
	if (!ifd_sched)
		return -ENODEV;

	sched = kzalloc(sizeof(*sched), GFP_KERNEL);
	if (!sched) {
		edgx_br_err(sched_com->parent, "Cannot allocate scheduler\n");
		return -ENOMEM;
	}

	if (edgx_sched_init(sched, sched_com, sched_idx, ifd_sched->iobase))
		goto out_sched_init;

	sched->sched_evt = sched_evt;
	sched_com->sched_list[sched_idx] = sched;
	sched_evt->sched = sched;

	return 0;

out_sched_init:
	kfree(sched);
	return -ENODEV;
}

void edgx_shutdown_sched(struct edgx_sched *sched)
{
	if (sched) {
		edgx_sched_hw_disable(sched->iobase,
				      sched->admin_tab.gate_states);
		kfree(sched->admin_tab.entries);
		kfree(sched);
	}
}
