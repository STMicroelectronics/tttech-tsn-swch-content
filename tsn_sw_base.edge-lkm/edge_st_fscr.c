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
#include "edge_sched.h"
#include "edge_stat.h"
#include "edge_time.h"
#include "edge_bridge.h"
#include "edge_fqtss.h"
#include "edge_util.h"
#include "edge_link.h"

#define EDGX_SCHED_DBG

#if defined(EDGX_SCHED_DBG)
#define sched_dbg edgx_dbg
#else
#define sched_dbg(args...)
#endif

#define EDGX_SCHED_DEF_MAX_SDU		(1506U)
#define EDGX_SCHED_DEF_START_TIME_ERR	(256)
/* Maximum possible number of schedule table rows */
#define EDGX_SCHED_HW_MAX_ROWS		(1024U)

#define EDGX_SCHED_DEF_GATE_STATES	(0xff)
#define EDGX_SCHED_OP_SET_GSTATES	(0x00)
#define EDGX_SCHED_OP_HOLD_MAC		(0x01)
#define EDGX_SCHED_OP_REL_MAC		(0x02)

/** Delay parameters */
struct edgx_sched_dly {
	bool ready;
	ktime_t phy_tx_hi_min, phy_tx_hi_max, phy_tx_lo_min, phy_tx_lo_max;
	ktime_t g2o_hi_min, g2o_hi_max, g2o_lo_min, g2o_lo_max;
	u64 s2g_min, s2g_max;
	u64 v_s2g_hi, v_s2g_lo;
	u64 g2w_hi_min, g2w_hi_max, g2w_lo_min, g2w_lo_max;
	u64 s2g_hi_min, s2g_hi_max, s2g_lo_min, s2g_lo_max;
	u64 s2w_hi_min, s2w_hi_max, s2w_lo_min, s2w_lo_max;
	u64 clk, starttime_err, delay_reg, time_advance;
	u64 freq_err_per_sec, freq_err_per_nsec, freq_err_per_cyc;
	u64 adv_err_hi, adv_err_lo, max_overrun_hi, max_overrun_lo;
	u64 st_part_indep_tx_dly_hi_min, st_part_indep_tx_dly_hi_max;
	u64 st_part_indep_tx_dly_lo_min, st_part_indep_tx_dly_lo_max;
	u64 max_cyc_ms;
	u64 g_close_adv_lo, g_close_adv_hi;
};

struct edgx_st_com {
	struct edgx_sched_com	*sched_com;
	unsigned int		 nr_queues;
};

/** Qbv Scheduler */
struct edgx_st {
	struct edgx_pt		*parent;
	struct edgx_st_com	*st_com;
	struct edgx_sched_event	 sched_evt;
	struct edgx_stat_hdl	*hstat;

	struct edgx_st_tr_rate	 tr_rate[EDGX_ST_MAX_QUEUES];
	struct edgx_sched_dly	 dly;
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

static inline struct edgx_st *edgx_sched_event2st(struct edgx_sched_event *evt)
{
	return container_of(evt, struct edgx_st, sched_evt);
}

static inline struct edgx_sched *edgx_st2sched(struct edgx_st *st)
{
	return st->sched_evt.sched;
}

static inline struct edgx_st *edgx_dev2st(struct device *dev)
{
	return edgx_pt_get_st(edgx_dev2pt(dev));
}

static inline struct edgx_sched *edgx_dev2sched(struct device *dev)
{
	return edgx_dev2st(dev)->sched_evt.sched;
}

static inline u8 edgx_st_get_gate_mask(struct edgx_st *st)
{
	return BIT(st->st_com->nr_queues) - 1;
}

static inline u16 edgx_st_get_hold(struct edgx_st *st)
{
	return BIT(st->st_com->nr_queues);
}


#if defined(EDGX_SCHED_DBG)
void edgx_sched_dump_dly(struct edgx_st *st)
{
	struct edgx_sched_dly *d = &st->dly;
	struct edgx_sched *sched = edgx_st2sched(st);
	const struct edgx_sched_limits *limits = edgx_sched_get_limits(sched);

	sched_dbg("Dumping delays:\n");
	sched_dbg("phy_tx_hi_min: %lld\n", ktime_to_ns(d->phy_tx_hi_min));
	sched_dbg("phy_tx_hi_max: %lld\n", ktime_to_ns(d->phy_tx_hi_max));
	sched_dbg("phy_tx_lo_min: %lld\n", ktime_to_ns(d->phy_tx_lo_min));
	sched_dbg("phy_tx_lo_max: %lld\n", ktime_to_ns(d->phy_tx_lo_max));

	sched_dbg("g2o_hi_min: %lld\n", ktime_to_ns(d->g2o_hi_min));
	sched_dbg("g2o_hi_max: %lld\n", ktime_to_ns(d->g2o_hi_max));
	sched_dbg("g2o_lo_min: %lld\n", ktime_to_ns(d->g2o_lo_min));
	sched_dbg("g2o_lo_max: %lld\n", ktime_to_ns(d->g2o_lo_max));

	sched_dbg("s2g_min: %lld\n", ktime_to_ns(d->s2g_min));
	sched_dbg("s2g_max: %lld\n", ktime_to_ns(d->s2g_max));
	sched_dbg("v_s2g_hi: %llu\n", d->v_s2g_hi);
	sched_dbg("v_s2g_lo: %llu\n", d->v_s2g_lo);

	sched_dbg("g2w_hi_min: %llu\n", d->g2w_hi_min);
	sched_dbg("g2w_hi_max: %llu\n", d->g2w_hi_max);
	sched_dbg("g2w_lo_min: %llu\n", d->g2w_lo_min);
	sched_dbg("g2w_lo_max: %llu\n", d->g2w_lo_max);

	sched_dbg("s2g_hi_min: %llu\n", d->s2g_hi_min);
	sched_dbg("s2g_hi_max: %llu\n", d->s2g_hi_max);
	sched_dbg("s2g_lo_min: %llu\n", d->s2g_lo_min);
	sched_dbg("s2g_lo_max: %llu\n", d->s2g_lo_max);

	sched_dbg("s2w_hi_min: %llu\n", d->s2w_hi_min);
	sched_dbg("s2w_hi_max: %llu\n", d->s2w_hi_max);
	sched_dbg("s2w_lo_min: %llu\n", d->s2w_lo_min);
	sched_dbg("s2w_lo_max: %llu\n", d->s2w_lo_max);

	sched_dbg("clk: %llu\n", d->clk);
	sched_dbg("starttime_err: %llu\n", d->starttime_err);
	sched_dbg("delay_reg: %llu\n", d->delay_reg);
	sched_dbg("time_advance: %llu\n", d->time_advance);

	sched_dbg("freq_err_abs: %u\n", limits->freq_err_abs);
	sched_dbg("hw_granularity_ns: %u\n", limits->hw_granularity_ns);
	sched_dbg("freq_err_per_sec: %llu\n", d->freq_err_per_sec);
	sched_dbg("freq_err_per_nsec: %llu\n", d->freq_err_per_nsec);
	sched_dbg("freq_err_per_cyc: %llu\n", d->freq_err_per_cyc);

	sched_dbg("adv_err_hi: %llu\n", d->adv_err_hi);
	sched_dbg("adv_err_lo: %llu\n", d->adv_err_lo);
	sched_dbg("max_overrun_hi: %llu\n", d->max_overrun_hi);
	sched_dbg("max_overrun_lo: %llu\n", d->max_overrun_lo);
	sched_dbg("st_part_indep_tx_dly_hi_min: %llu\n",
		  d->st_part_indep_tx_dly_hi_min);
	sched_dbg("st_part_indep_tx_dly_hi_max: %llu\n",
		  d->st_part_indep_tx_dly_hi_max);
	sched_dbg("st_part_indep_tx_dly_lo_min: %llu\n",
		  d->st_part_indep_tx_dly_lo_min);
	sched_dbg("st_part_indep_tx_dly_lo_max: %llu\n",
		  d->st_part_indep_tx_dly_lo_max);
	sched_dbg("max_cyc_ms: %llu\n", d->max_cyc_ms);
	sched_dbg("g_close_adv_lo: %llu\n", d->g_close_adv_lo);
	sched_dbg("g_close_adv_hi: %llu\n", d->g_close_adv_hi);
}
#endif

static void edgx_st_calc_trans_rate(struct edgx_st *st,
				    const struct edgx_sched_tab_entry *entries,
				    size_t count)
{
	struct edgx_sched *sched = edgx_st2sched(st);
	unsigned int queue, i;
	u64 sum, idle_time, cyc_time_nsec, sum_total = 0;
	struct edgx_sched_rational cycle_time, rate;
	u32 cyc_time_subnsec;

	edgx_sched_get_cycle_time_locked(sched, EDGX_SCHED_ADMIN, &cycle_time);
	edgx_sched_rational_to_nsec(&cycle_time,
				    &cyc_time_nsec,
				    &cyc_time_subnsec);

	for (queue = 0; queue < st->st_com->nr_queues; queue++) {
		sum = 0;
		sum_total = 0;
		for (i = 0; i < count; i++) {
			sum_total += entries[i].time_interval;

			if (entries[i].gate_states & BIT(queue))
				sum += entries[i].time_interval;
		}

		idle_time = cyc_time_nsec - sum_total;

		sched_dbg("calc_trans_rate: cyc_time_nsec=%llu, sum_total=%llu, sum\%llu, idle_time\%llu\n",
			  cyc_time_nsec, sum_total, sum, idle_time);
		/* The gate remains in the last state -> add idle time */
		if (entries[count - 1].gate_states & BIT(queue))
			sum += idle_time;

		sched_dbg("calc_trans_rate2: cyc_time_nsec=%llu, sum_total=%llu, sum\%llu, idle_time\%llu\n",
			  cyc_time_nsec, sum_total, sum, idle_time);

		if (sum) {
			edgx_sched_nsec_to_rational(sum, 0, &rate);
			st->tr_rate[queue].num = (u64)rate.denom *
						 (u64)cycle_time.num;
			st->tr_rate[queue].denom = (u64)rate.num *
						   (u64)cycle_time.denom;
		} else {
			st->tr_rate[queue].num = 0;
			st->tr_rate[queue].denom = 1;
		}
	}
}

static void edgx_st_config_changed(struct edgx_sched_event *sched_evt)
{
	struct edgx_st *st = edgx_sched_event2st(sched_evt);
	struct edgx_fqtss *fqtss = edgx_pt_get_fqtss(st->parent);

	sched_dbg("Notify\n");
	edgx_fqtss_st_change(fqtss, st);
}

int edgx_sched_calc_delays(struct edgx_st *st)
{
	struct edgx_sched *sched = edgx_st2sched(st);
	const struct edgx_sched_limits *limits = edgx_sched_get_limits(sched);
	struct edgx_link *lnk = edgx_pt_get_link(st->parent);
	struct edgx_sched_dly *d = &st->dly;
	enum edgx_lnk_mode lnk_mode = edgx_link_get_lnk_mode(lnk);
	u64 max_g_close_adv_lo, max_g_close_adv_hi, rem;
	u64 g_close_adv_div_lo, g_close_adv_div_hi;
	int ret;

	if (d->ready)
		return 0;

	ret = edgx_link_get_tx_delays_hi(lnk, &d->phy_tx_hi_min,
					 &d->phy_tx_hi_max);
	if (ret)
		return ret;

	ret = edgx_link_get_tx_delays_lo(lnk, &d->phy_tx_lo_min,
					 &d->phy_tx_lo_max);
	if (ret)
		return ret;

	ret = edgx_pt_get_g2o_hi(st->parent, &d->g2o_hi_min,
				 &d->g2o_hi_max);
	if (ret)
		return ret;

	ret = edgx_pt_get_g2o_lo(st->parent, &d->g2o_lo_min,
				 &d->g2o_lo_max);
	if (ret)
		return ret;

	d->clk = (u64)edgx_br_get_cycle_ns(edgx_sched_get_br(sched));
	if (!d->clk)
		return -EINVAL;

	ret = edgx_sched_get_s2g(sched, d->clk, &d->s2g_min, &d->s2g_max);
	if (ret)
		return ret;

	d->starttime_err = EDGX_SCHED_DEF_START_TIME_ERR;
	d->max_cyc_ms = limits->max_cyc_ms;

	d->g2w_hi_min = ktime_to_ns(ktime_add(d->phy_tx_hi_min, d->g2o_hi_min));
	d->g2w_hi_max = ktime_to_ns(ktime_add(d->phy_tx_hi_max, d->g2o_hi_max));
	d->g2w_lo_min = ktime_to_ns(ktime_add(d->phy_tx_lo_min, d->g2o_lo_min));
	d->g2w_lo_max = ktime_to_ns(ktime_add(d->phy_tx_lo_max, d->g2o_lo_max));

	d->delay_reg = abs((s64)d->g2w_hi_min - (s64)d->g2w_lo_min);
	do_div(d->delay_reg, limits->hw_granularity_ns);

	d->s2g_hi_min = d->s2g_min +
			(399 * (u64)limits->hw_granularity_ns);
	d->s2g_hi_max = d->s2g_max +
			(399 * (u64)limits->hw_granularity_ns) +
			d->starttime_err;

	d->s2g_lo_min = d->s2g_min +
			(399 * (u64)limits->hw_granularity_ns);
	d->s2g_lo_max = d->s2g_max +
			(399 * (u64)limits->hw_granularity_ns) +
			d->starttime_err;

	d->s2w_hi_min = d->s2g_hi_min + d->g2w_hi_min;
	d->s2w_hi_max = d->s2g_hi_max + d->g2w_hi_max;
	d->s2w_lo_min = d->s2g_lo_min + d->g2w_lo_min;
	d->s2w_lo_max = d->s2g_lo_max + d->g2w_lo_max;

	d->freq_err_per_sec = (u64)limits->freq_err_abs * 1000;
	d->freq_err_per_nsec = d->freq_err_per_sec;
	do_div(d->freq_err_per_nsec, 1000000);

	d->freq_err_per_cyc = d->freq_err_per_sec * d->max_cyc_ms;
	do_div(d->freq_err_per_cyc, 1000);

	d->time_advance = min(d->s2w_hi_min, d->s2w_lo_min) -
			  d->freq_err_per_cyc;

	d->adv_err_hi = d->s2w_hi_min - d->time_advance;
	d->adv_err_lo = d->s2w_lo_min - d->time_advance -
			(u64)limits->hw_granularity_ns * d->delay_reg;
	d->max_overrun_hi = d->s2w_hi_max - d->time_advance +
			    d->freq_err_per_cyc;
	d->max_overrun_lo = d->s2w_lo_max - d->time_advance +
			    d->freq_err_per_cyc -
			    (u64)limits->hw_granularity_ns * d->delay_reg;

	d->v_s2g_hi = d->s2g_hi_max - d->s2g_hi_min;
	d->v_s2g_lo = d->s2g_lo_max - d->s2g_lo_min;

	d->st_part_indep_tx_dly_hi_min = d->adv_err_hi;
	d->st_part_indep_tx_dly_hi_max = d->adv_err_hi + d->v_s2g_hi +
					 d->starttime_err + d->freq_err_per_cyc;
	d->st_part_indep_tx_dly_lo_min = d->adv_err_lo;
	d->st_part_indep_tx_dly_lo_max = d->adv_err_lo + d->v_s2g_lo +
					 d->starttime_err + d->freq_err_per_cyc;

	if (lnk_mode == EDGX_LNKMOD_100_1000) {
		max_g_close_adv_lo = 46;
		g_close_adv_div_lo = 80;
		max_g_close_adv_hi = 255;
		g_close_adv_div_hi = 8;
	} else if (lnk_mode == EDGX_LNKMOD_10_100) {
		max_g_close_adv_lo = 46;
		g_close_adv_div_lo = 800;
		max_g_close_adv_hi = 255;
		g_close_adv_div_hi = 80;
	} else if (lnk_mode == EDGX_LNKMOD_100) {
		max_g_close_adv_lo = 46;
		g_close_adv_div_lo = 80;
		max_g_close_adv_hi = 0;
		g_close_adv_div_hi = 1;
	} else {
		return -EINVAL;
	}

	d->g_close_adv_lo = d->max_overrun_lo;
	rem = do_div(d->g_close_adv_lo, g_close_adv_div_lo);
	if (rem)
		d->g_close_adv_lo++;
	d->g_close_adv_lo = min(d->g_close_adv_lo, max_g_close_adv_lo);

	d->g_close_adv_hi = d->max_overrun_hi;
	rem = do_div(d->g_close_adv_hi, g_close_adv_div_hi);
	if (rem)
		d->g_close_adv_hi++;
	d->g_close_adv_hi = min(d->g_close_adv_hi, max_g_close_adv_hi);

	edgx_sched_set_delay(sched, (u8)d->delay_reg);
	edgx_sched_set_g_close_adv(sched, d->g_close_adv_lo, d->g_close_adv_hi);

	d->ready = true;
	return 0;
}

u64 edgx_st_get_part_indep_tx_dly_hi_min(struct edgx_st *st)
{
	return st->dly.st_part_indep_tx_dly_hi_min;
}

u64 edgx_st_get_part_indep_tx_dly_hi_max(struct edgx_st *st)
{
	return st->dly.st_part_indep_tx_dly_hi_max;
}

u64 edgx_st_get_part_indep_tx_dly_lo_min(struct edgx_st *st)
{
	return st->dly.st_part_indep_tx_dly_lo_min;
}

u64 edgx_st_get_part_indep_tx_dly_lo_max(struct edgx_st *st)
{
	return st->dly.st_part_indep_tx_dly_lo_max;
}

u64 edgx_st_get_freq_err_per_cyc(struct edgx_st *st)
{
	return st->dly.freq_err_per_cyc;
}

static int edgx_st_get_sched_offset(struct edgx_sched_event *sched_evt,
				    s64 *offset_ns)
{
	struct edgx_st *st = edgx_sched_event2st(sched_evt);
	int ret;

	ret = edgx_sched_calc_delays(st);
	if (ret)
		return ret;

	*offset_ns = -st->dly.time_advance;
	return 0;
}

static int edgx_st_prepare_config(struct edgx_sched_event *sched_evt,
				  const struct edgx_sched_tab_entry *entries,
				  size_t count)
{
	struct edgx_st *st = edgx_sched_event2st(sched_evt);
	struct edgx_sched *sched = sched_evt->sched;
	size_t i;
	u16 hold = 0;
	u32 time_from_cyc_start = 0;
	int tab_idx = edgx_sched_get_free_tab(sched);
	int ret = 0;

	if (tab_idx < 0)
		return tab_idx;

	for (i = 0; !ret && (i < count); i++) {
		if (entries[i].operation_name == EDGX_SCHED_OP_HOLD_MAC)
			hold = edgx_st_get_hold(st);
		else if (entries[i].operation_name == EDGX_SCHED_OP_REL_MAC)
			hold = 0;

		time_from_cyc_start += entries[i].time_interval;

		sched_dbg("CC: i=%zd, time_from_cyc_start=%d, entries[i].time_interval=%d\n",
			  i, time_from_cyc_start, entries[i].time_interval);

		ret = edgx_sched_write_entry(sched,
					     tab_idx, i,
					     (u16)entries[i].gate_states | hold,
					     time_from_cyc_start);
	}

	/* Add the last interval entry again. FSC will stop on this entry */
	if (!ret)
		ret = edgx_sched_write_entry(sched,
					     tab_idx, i,
					     entries[i - 1].gate_states,
					     time_from_cyc_start);

	time_from_cyc_start = 0;
	for (i = 0; hold && !ret && (i < count - 1) &&
	     entries[i].operation_name != EDGX_SCHED_OP_HOLD_MAC &&
	     entries[i].operation_name != EDGX_SCHED_OP_REL_MAC; i++) {

		time_from_cyc_start += entries[i].time_interval;
		ret = edgx_sched_write_entry(sched,
					     tab_idx, i,
					     (u16)entries[i].gate_states | hold,
					     time_from_cyc_start);
	}

	if (!ret)
		edgx_st_calc_trans_rate(st, entries, count);

	return ret;
}

static void edgx_st_init(struct edgx_st *st, struct edgx_pt *pt)
{
	unsigned int i;

	st->parent = pt;

	for (i = 0; i < st->st_com->nr_queues; i++) {
		st->tr_rate[i].num = 1;
		st->tr_rate[i].denom = 1;
		edgx_pt_set_framesize(st->parent, i, EDGX_SCHED_DEF_MAX_SDU);
	}
}

static int edgx_sched_strtotimespec64(const char *buf,
				      size_t count,
				      struct timespec64 *time)
{
	char str[64], str_nsec[10];
	char *token, *str_p = str;
	s64 sec;
	long nsec = 0;
	ssize_t ret;

	ret = strscpy(str, buf, sizeof(str));
	if (ret < 0)
		return ret;

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
	return 0;
}

/******************************************************************************
 * User Interface
 *****************************************************************************/

int edgx_st_get_trans_rate_locked(struct edgx_st *st,
				  unsigned int queue_idx,
				  struct edgx_st_tr_rate *tr_rate)
{
	struct edgx_sched *sched;

	if (!st || !tr_rate)
		return -EINVAL;

	if (queue_idx >= st->st_com->nr_queues)
		return -EINVAL;

	sched = edgx_st2sched(st);
	if (!edgx_sched_get_gate_enabled_locked(sched)) {
		tr_rate->num = 1;
		tr_rate->denom = 1;
		return 0;
	}
	*tr_rate = st->tr_rate[queue_idx];
	return 0;
}

int edgx_st_get_trans_rate(struct edgx_st *st,
			   unsigned int queue_idx,
			   struct edgx_st_tr_rate *tr_rate)
{
	struct edgx_sched *sched;

	if (!st)
		return -EINVAL;

	sched = edgx_st2sched(st);
	edgx_sched_lock(sched);
	edgx_st_get_trans_rate_locked(st, queue_idx, tr_rate);
	edgx_sched_unlock(sched);
	return 0;
}

static ssize_t current_time_show(struct device *dev,
				 struct device_attribute *attr,
				 char *buf)
{
	struct edgx_sched *sched = edgx_dev2sched(dev);
	struct timespec64 ts;
	int ret;

	ret = edgx_sched_get_current_time(sched, &ts);
	if (ret == 0)
		ret = scnprintf(buf, PAGE_SIZE, "%lli.%09li\n",
				(long long)ts.tv_sec, ts.tv_nsec);
	return ret;
}

static ssize_t gate_enabled_show(struct device *dev,
				 struct device_attribute *attr,
				 char *buf)
{
	struct edgx_sched *sched = edgx_dev2sched(dev);
	bool gate_enabled;
	int ret;

	gate_enabled = edgx_sched_get_gate_enabled(sched);
	ret = scnprintf(buf, PAGE_SIZE, "%u\n", gate_enabled);
	return ret;
}

static ssize_t gate_enabled_store(struct device *dev,
				  struct device_attribute *attr,
				  const char *buf,
				  size_t count)
{
	int ret = 0;
	struct edgx_sched *sched = edgx_dev2sched(dev);
	bool gate_enabled;

	if (kstrtobool(buf, &gate_enabled))
		return -EINVAL;

	ret = edgx_sched_set_gate_enabled(sched, gate_enabled);
	return ret ? ret : count;
}

static ssize_t admin_gate_states_show(struct device *dev,
				      struct device_attribute *attr,
				      char *buf)
{
	ssize_t ret;
	struct edgx_st *st = edgx_dev2st(dev);
	struct edgx_sched *sched = edgx_dev2sched(dev);
	u8 gate_st_mask = edgx_st_get_gate_mask(st);
	u8 gate_states;

	gate_states = edgx_sched_get_admin_gate_states(sched, gate_st_mask);
	ret = scnprintf(buf, PAGE_SIZE, "0x%X\n", gate_states);
	return ret;
}

static ssize_t admin_gate_states_store(struct device *dev,
				       struct device_attribute *attr,
				       const char *buf,
				       size_t count)
{
	int ret = -EINVAL;
	struct edgx_st *st = edgx_dev2st(dev);
	struct edgx_sched *sched = edgx_dev2sched(dev);
	u8 gate_states;

	if (!kstrtou8(buf, 0, &gate_states)) {
		u8 gate_st_mask = edgx_st_get_gate_mask(st);

		ret = edgx_sched_set_admin_gate_states(sched, gate_states,
						       gate_st_mask);
	}
	return ret ? ret : count;
}

static ssize_t max_sdu_tab_read(struct file *filp, struct kobject *kobj,
				struct bin_attribute *bin_attr,
				char *buf, loff_t ofs, size_t count)
{
	struct edgx_st *st = edgx_dev2st(kobj_to_dev(kobj));
	loff_t idx = 0;

	if (edgx_sysfs_tbl_params(ofs, count, sizeof(u32), &idx) ||
	    idx >= st->st_com->nr_queues)
		return -EINVAL;

	/* NOTE: FLEXDE-2329: Standard Deviation:
	 * FRAMESIZEx registers does not account for VLAN- and R-TAG
	 */
	*((u32 *)buf) = edgx_pt_get_framesize(st->parent, idx);

	return count;
}

static ssize_t max_sdu_tab_write(struct file *filp, struct kobject *kobj,
				 struct bin_attribute *bin_attr,
				 char *buf, loff_t ofs, size_t count)
{
	struct edgx_st *st = edgx_dev2st(kobj_to_dev(kobj));
	loff_t idx = 0;
	u32 val = *((u32 *)buf);

	if (edgx_sysfs_tbl_params(ofs, count, sizeof(u32), &idx) ||
	    idx >= st->st_com->nr_queues)
		return -EINVAL;

	if (val > EDGX_SCHED_DEF_MAX_SDU)
		return -ERANGE;

	/* NOTE: FLEXDE-2329: Standard Deviation:
	 * FRAMESIZEx registers does not account for VLAN- and R-TAG
	 */
	edgx_pt_set_framesize(st->parent, idx, val);
	return count;
}

static ssize_t overrun_sdu_tab_read(struct file *filp, struct kobject *kobj,
				    struct bin_attribute *bin_attr,
				    char *buf, loff_t ofs, size_t count)
{
	struct edgx_st *st = edgx_dev2st(kobj_to_dev(kobj));
	loff_t idx = 0;

	if (edgx_sysfs_tbl_params(ofs, count, sizeof(statw_t), &idx) ||
	    idx >= st->st_com->nr_queues)
		return -EINVAL;

	/* NOTE: FLEXDE-451: Standard Deviation:
	 * The IP provides only one counters per port,
	 * the standard defines one per queue, so 8 per port.
	 */
	*((statw_t *)buf) = edgx_stat_upget(st->hstat, 0);
	return count;
}

static ssize_t admin_ctrl_list_len_show(struct device *dev,
					struct device_attribute *attr,
					char *buf)
{
	struct edgx_sched *sched = edgx_dev2sched(dev);
	u32 list_len;
	int ret;

	ret = edgx_sched_get_ctrl_list_len(sched, EDGX_SCHED_ADMIN, &list_len);
	if (ret == 0)
		ret = scnprintf(buf, PAGE_SIZE, "%u\n", list_len);
	return ret;
}

static ssize_t admin_ctrl_list_len_store(struct device *dev,
					 struct device_attribute *attr,
					 const char *buf,
					 size_t count)
{
	u32 list_len;
	struct edgx_sched *sched = edgx_dev2sched(dev);
	int ret;

	if (kstrtou32(buf, 10, &list_len))
		return -EINVAL;

	ret = edgx_sched_set_ctrl_list_len(sched, list_len);
	return ret ? ret : count;
}

static ssize_t oper_ctrl_list_len_show(struct device *dev,
				       struct device_attribute *attr,
				       char *buf)
{
	struct edgx_sched *sched = edgx_dev2sched(dev);
	u32 list_len;
	int ret;

	ret = edgx_sched_get_ctrl_list_len(sched, EDGX_SCHED_OPER, &list_len);
	if (ret == 0)
		ret = scnprintf(buf, PAGE_SIZE, "%u\n", list_len);
	return ret;
}

static ssize_t admin_ctrl_list_read(struct file *filp, struct kobject *kobj,
				    struct bin_attribute *bin_attr,
				    char *buf, loff_t ofs, size_t count)
{
	struct edgx_sched *sched = edgx_dev2sched(kobj_to_dev(kobj));
	struct edgx_sched_tab_entry *entry = (struct edgx_sched_tab_entry *)buf;
	loff_t idx;
	size_t nelems;
	int ret;

	if (edgx_sysfs_list_params(ofs, count, sizeof(*entry), &idx, &nelems))
		return -EINVAL;
	ret = edgx_sched_get_admin_ctrl_list(sched, idx, entry, nelems);
	return ret ? ret : count;
}

static ssize_t admin_ctrl_list_write(struct file *filp, struct kobject *kobj,
				     struct bin_attribute *bin_attr,
				     char *buf, loff_t ofs, size_t count)
{
	struct edgx_st *st = edgx_dev2st(kobj_to_dev(kobj));
	struct edgx_sched *sched = edgx_st2sched(st);
	struct edgx_sched_tab_entry *entry = (struct edgx_sched_tab_entry *)buf;
	u8 gate_st_mask = edgx_st_get_gate_mask(st);
	loff_t idx;
	size_t i, nelems;
	int ret;

	if (edgx_sysfs_list_params(ofs, count, sizeof(*entry), &idx, &nelems))
		return -EINVAL;
	for (i = 0; i < nelems; i++)
		entry[i].gate_states &= gate_st_mask;
	ret = edgx_sched_set_admin_ctrl_list(sched, idx, entry, nelems);
	return ret ? ret : count;
}

static int edgx_st_get_oper_ctrl_list(struct edgx_st *st,
				      size_t first,
				      struct edgx_sched_tab_entry *entry,
				      size_t count)
{
	struct edgx_sched *sched = edgx_st2sched(st);
	u32 time_ns, prev_time_ns = 0;
	u16 gate_states, new_hold, hold = 0;
	size_t i;
	int ret = 0;
	u16 gate_st_mask = edgx_st_get_gate_mask(st);
	u16 hold_mask = edgx_st_get_hold(st);
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

		entry[i].gate_states = gate_states & gate_st_mask;
		new_hold = gate_states & hold_mask;
		if (!hold && new_hold)
			entry[i].operation_name = EDGX_SCHED_OP_HOLD_MAC;
		else if (hold && !new_hold)
			entry[i].operation_name = EDGX_SCHED_OP_REL_MAC;
		else
			entry[i].operation_name = EDGX_SCHED_OP_SET_GSTATES;
		hold = new_hold;
	}
	for (; i < count; i++)
		entry[i] = edgx_sched_undef_entry;

	return 0;
}

static ssize_t oper_ctrl_list_read(struct file *filp, struct kobject *kobj,
				   struct bin_attribute *bin_attr,
				   char *buf, loff_t ofs, size_t count)
{
	struct edgx_st *st = edgx_dev2st(kobj_to_dev(kobj));
	struct edgx_sched *sched = edgx_st2sched(st);
	struct edgx_sched_tab_entry *entry = (struct edgx_sched_tab_entry *)buf;
	loff_t idx;
	size_t nelems;
	int ret;

	if (edgx_sysfs_list_params(ofs, count, sizeof(*entry), &idx, &nelems))
		return -EINVAL;
	edgx_sched_lock(sched);
	ret = edgx_st_get_oper_ctrl_list(st, idx, entry, nelems);
	edgx_sched_unlock(sched);
	return ret ? ret : count;
}

static ssize_t admin_cycle_time_show(struct device *dev,
				     struct device_attribute *attr,
				     char *buf)
{
	struct edgx_sched *sched = edgx_dev2sched(dev);
	struct edgx_sched_rational cycle_time;
	int ret;

	ret = edgx_sched_get_cycle_time(sched, EDGX_SCHED_ADMIN, &cycle_time);
	if (ret == 0)
		ret = scnprintf(buf, PAGE_SIZE, "%u/%u\n",
				cycle_time.num,
				cycle_time.denom);
	return ret;
}

static ssize_t admin_cycle_time_store(struct device *dev,
				      struct device_attribute *attr,
				      const char *buf,
				      size_t count)
{
	struct edgx_sched *sched = edgx_dev2sched(dev);
	unsigned int num, denom;
	struct edgx_sched_rational cycle_time;
	int ret;

	ret = sscanf(buf, "%u/%u", &num, &denom);
	if (ret == 2) {
		cycle_time.num = num;
		cycle_time.denom = denom;
		ret = edgx_sched_set_cycle_time(sched, &cycle_time);
	}
	return ret ? ret : count;
}

static ssize_t oper_cycle_time_show(struct device *dev,
				    struct device_attribute *attr,
				    char *buf)
{
	struct edgx_sched *sched = edgx_dev2sched(dev);
	struct edgx_sched_rational cycle_time;
	int ret;

	ret = edgx_sched_get_cycle_time(sched, EDGX_SCHED_OPER, &cycle_time);
	if (ret == 0)
		ret = scnprintf(buf, PAGE_SIZE, "%u/%u\n",
				cycle_time.num,
				cycle_time.denom);
	return ret;
}

static ssize_t admin_cycle_time_ext_show(struct device *dev,
					 struct device_attribute *attr,
					 char *buf)
{
	/* NOTE: FLEXDE-5153: Standard Deviation:
	 * Not supported, report as zero.
	 */
	return scnprintf(buf, PAGE_SIZE, "%d\n", 0);
}

static ssize_t admin_cycle_time_ext_store(struct device *dev,
					  struct device_attribute *attr,
					  const char *buf,
					  size_t count)
{
	struct edgx_st *st = edgx_dev2st(dev);
	unsigned int ext_ns;
	int ret;

	/* NOTE: FLEXDE-5153: Standard Deviation:
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
	/* NOTE: FLEXDE-5153: Standard Deviation:
	 * Not supported, report as zero.
	 */
	return scnprintf(buf, PAGE_SIZE, "%d\n", 0);
}

static ssize_t admin_base_time_show(struct device *dev,
				    struct device_attribute *attr,
				    char *buf)
{
	struct edgx_sched *sched = edgx_dev2sched(dev);
	struct timespec64 base_time;
	int ret;

	ret = edgx_sched_get_base_time(sched, EDGX_SCHED_ADMIN, &base_time);
	if (ret == 0)
		ret = scnprintf(buf, PAGE_SIZE, "%lld.%09li\n",
				base_time.tv_sec,
				base_time.tv_nsec);
	return ret;
}

static ssize_t admin_base_time_store(struct device *dev,
				     struct device_attribute *attr,
				     const char *buf,
				     size_t count)
{
	struct edgx_sched *sched = edgx_dev2sched(dev);
	struct timespec64 base_time;
	int ret;

	ret = edgx_sched_strtotimespec64(buf, count, &base_time);
	if (ret)
		return ret;
	ret = edgx_sched_set_base_time(sched, &base_time);
	return ret ? ret : count;
}

static ssize_t oper_base_time_show(struct device *dev,
				   struct device_attribute *attr,
				   char *buf)
{
	struct edgx_sched *sched = edgx_dev2sched(dev);
	struct timespec64 base_time;
	int ret;

	ret = edgx_sched_get_base_time(sched, EDGX_SCHED_OPER, &base_time);
	if (ret == 0)
		ret = scnprintf(buf, PAGE_SIZE, "%lld.%09li\n",
				base_time.tv_sec,
				base_time.tv_nsec);
	return ret;
}

static ssize_t config_change_show(struct device *dev,
				  struct device_attribute *attr,
				  char *buf)
{
	struct edgx_sched *sched = edgx_dev2sched(dev);
	bool config_change;
	int ret;

	ret = edgx_sched_get_config_change(sched, &config_change);
	if (ret == 0)
		ret = scnprintf(buf, PAGE_SIZE, "%u\n", config_change);
	return ret;
}

static ssize_t config_change_store(struct device *dev,
				   struct device_attribute *attr,
				   const char *buf,
				   size_t count)
{
	struct edgx_sched *sched = edgx_dev2sched(dev);
	bool config_change;
	int ret = -EINVAL;

	if (!kstrtobool(buf, &config_change))
		ret = edgx_sched_set_config_change(sched, config_change);
	return ret ? ret : count;
}

static ssize_t config_change_time_show(struct device *dev,
				       struct device_attribute *attr,
				       char *buf)
{
	struct edgx_sched *sched = edgx_dev2sched(dev);
	struct timespec64 conf_change_time;
	int ret;

	ret = edgx_sched_get_config_change_time(sched, &conf_change_time);
	if (ret == 0)
		ret = scnprintf(buf, PAGE_SIZE, "%lld.%09li\n",
				conf_change_time.tv_sec,
				conf_change_time.tv_nsec);
	return ret;
}

static ssize_t tick_granul_show(struct device *dev,
				struct device_attribute *attr,
				char *buf)
{
	struct edgx_sched *sched = edgx_dev2sched(dev);
	const struct edgx_sched_limits *limits = edgx_sched_get_limits(sched);
	int ret;

	ret = scnprintf(buf, PAGE_SIZE, "%u\n", limits->hw_granularity_ns * 10);
	return ret;
}

static ssize_t config_pending_show(struct device *dev,
				   struct device_attribute *attr,
				   char *buf)
{
	struct edgx_sched *sched = edgx_dev2sched(dev);
	bool config_pending;
	int ret = edgx_sched_get_config_pending(sched, &config_pending);

	if (ret == 0)
		ret = scnprintf(buf, PAGE_SIZE, "%u\n", config_pending);
	return ret;
}

static ssize_t config_change_error_show(struct device *dev,
					struct device_attribute *attr,
					char *buf)
{
	struct edgx_sched *sched = edgx_dev2sched(dev);
	u64 cc_error;
	int ret = edgx_sched_get_config_change_error(sched, &cc_error);

	if (ret == 0)
		ret = scnprintf(buf, PAGE_SIZE, "%llu\n", cc_error);
	return ret;
}

static ssize_t supported_list_max_show(struct device *dev,
				       struct device_attribute *attr,
				       char *buf)
{
	struct edgx_sched *sched = edgx_dev2sched(dev);
	const struct edgx_sched_limits *limits = edgx_sched_get_limits(sched);
	int ret;

	ret = scnprintf(buf, PAGE_SIZE, "%u\n", limits->max_entry_cnt);
	return ret;
}

static ssize_t supported_cyc_max_show(struct device *dev,
				      struct device_attribute *attr,
				      char *buf)
{
	struct edgx_sched *sched = edgx_dev2sched(dev);
	const struct edgx_sched_limits *limits = edgx_sched_get_limits(sched);
	struct edgx_sched_rational cycle_time;
	int ret;

	edgx_sched_nsec_to_rational(limits->max_cyc_ms * NSEC_PER_MSEC, 0,
				    &cycle_time);
	ret = scnprintf(buf, PAGE_SIZE, "%u/%u\n",
			cycle_time.num, cycle_time.denom);
	return ret;
}

static ssize_t supported_int_max_show(struct device *dev,
				      struct device_attribute *attr,
				      char *buf)
{
	struct edgx_sched *sched = edgx_dev2sched(dev);
	const struct edgx_sched_limits *limits = edgx_sched_get_limits(sched);
	u32 max_int_ns = limits->hw_granularity_ns * limits->max_int_len;
	int ret;

	ret = scnprintf(buf, PAGE_SIZE, "%u\n", max_int_ns);
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

int edgx_probe_st(struct edgx_pt *pt, struct edgx_st_com *st_com,
		  struct edgx_st **pst)
{
	ptid_t                   ptid = edgx_pt_get_id(pt);
	struct edgx_stat        *sm = edgx_br_get_pt_stat(edgx_pt_get_br(pt),
							  ptid);
	struct edgx_time	*time = edgx_pt_get_time(pt);
	int ret = -EINVAL;

	if (!pt || !st_com || !pst || !time)
		return -EINVAL;

	*pst = kzalloc(sizeof(**pst), GFP_KERNEL);
	if (!(*pst)) {
		edgx_pt_err(pt, "Cannot allocate Scheduled Traffic\n");
		return -ENOMEM;
	}

	(*pst)->st_com = st_com;
	ret = edgx_probe_sched(st_com->sched_com, ptid, &(*pst)->sched_evt);
	if (ret)
		goto out_probe_sched;
	edgx_st_init(*pst, pt);

	(*pst)->hstat = edgx_stat_alloc_hdl(sm, ptid, &_st_statinfo);

	edgx_pt_add_sysfs(pt, &ieee8021_st_group);

	return 0;

out_probe_sched:
	kfree(*pst);
	return -ENODEV;
}

void edgx_shutdown_st(struct edgx_st *st)
{
	if (st) {
		edgx_pt_rem_sysfs(st->parent, &ieee8021_st_group);
		edgx_stat_free_hdl(st->hstat);
		edgx_shutdown_sched(st->sched_evt.sched);
		kfree(st);
	}
}

static const struct edgx_sched_ops edgx_st_sched_ops = {
	.get_sched_offset	= edgx_st_get_sched_offset,
	.prepare_config		= edgx_st_prepare_config,
	.config_changed		= edgx_st_config_changed,
};

int edgx_st_com_probe(struct edgx_br *br, struct edgx_br_irq *irq,
		      const char *drv_name,
		      struct edgx_st_com **pst_com,
		      bool *max_2_speeds)
{
	const struct edgx_ifreq ifreq = { .id = AC_SCHED_ID, .v_maj = 2 };
	const struct edgx_ifdesc *ifd_com = edgx_ac_get_if(&ifreq);
	u16 init_gate_states = EDGX_SCHED_DEF_GATE_STATES;
	int ret = -ENOMEM;

	if (!max_2_speeds)
		return -EINVAL;

	if (!ifd_com || !ifd_com->ptmap)
		return -ENODEV;

	*pst_com = kzalloc(sizeof(**pst_com), GFP_KERNEL);
	if (!*pst_com)
		return -ENOMEM;

	(*pst_com)->nr_queues = edgx_br_get_generic(br, BR_GX_QUEUES);
	init_gate_states &= BIT((*pst_com)->nr_queues) - 1;
	ret = edgx_sched_com_probe(br, irq, EDGX_IRQ_NR_ST_SCHED_TAB, drv_name,
				   ifd_com,
				   &(*pst_com)->sched_com,
				   &edgx_st_sched_ops,
				   init_gate_states);
	if (ret)
		goto out_sched_com;

	*max_2_speeds = true;
	return ret;

out_sched_com:
	kfree(*pst_com);
	return ret;
}

void edgx_st_com_set_params(struct edgx_st_com *st_com,
			    u32 freq_err_abs, u32 sched_granul)
{
	if (st_com && st_com->sched_com)
		edgx_sched_com_set_params(st_com->sched_com,
					  freq_err_abs, sched_granul);
}

void edgx_st_com_shutdown(struct edgx_st_com *st_com)
{
	if (!st_com)
		return;
	edgx_sched_com_shutdown(st_com->sched_com);
	kfree(st_com);
}
