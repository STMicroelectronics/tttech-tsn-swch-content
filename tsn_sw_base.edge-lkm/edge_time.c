// SPDX-License-Identifier: GPL-2.0
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

#include <linux/ptp_clock_kernel.h>
#include <linux/spinlock.h>
#include <linux/clocksource.h>
#include <linux/version.h>

#include "edge_ac.h"
#include "edge_stat.h"
#include "edge_port.h"
#include "edge_time.h"
#include "edge_util.h"

#define EDGX_TM_NCLOCKS  2 /* number of clocks */
#define EDGX_TM_NXTS     1 /* number of cross-timestampers */
#define EDGX_TM_NTMUX    1 /* number of time-multiplexers */

#define IBC_GPMUX_CTL       0x1000
#define IBC_TMMUX_CTL       0x1100
#define IBC_TMMUX0_MASK     0x1
#define IBC_GX_TMUX         0x2004
#define IBC_AC_NRTC         0x2100
#define IBC_AC_RTC(_frtc)   (0x2110 + ((_frtc) * 2))
#define IBC_AC_XTS          0x2118
#define IBC_AC_SHIFT        16

#define FRTC_CUR_CC_LO      0x1010
#define FRTC_CUR_CC_HI      0x1014

#define FRTC_CUR_NSEC       0x1004
#define FRTC_CUR_SEC_HI     0x100C
#define FRTC_CUR_SEC_LO     0x1008

#define FRTC_ADJ_NSEC       0x1034
#define FRTC_ADJ_SEC_HI     0x103C
#define FRTC_ADJ_SEC_LO     0x1038

#define FRTC_STEP_SNS       0x1020
#define FRTC_STEP_NS        0x1024
#define FRTC_STEP_NS_MAX    63

#define FRTC_CMD            0x1040
#define FRTC_CMD_ADJ_TIME   1
#define FRTC_CMD_ADJ_STEP   2
#define FRTC_CMD_RD_TIME    4

#define FRTC_GX_STEP_SNS    0x2000
#define FRTC_GX_STEP_NS     0x2004

#define FPTS_TRIGGER_CLK_ID 1
#define FPTS_TS_CTL         0x1000
#define FPTS_TS_NSEC        0x1104
#define FPTS_TS_SEC_HI      0x110C
#define FPTS_TS_SEC_LO      0x1108

struct edgx_time;
struct edgx_clk;

struct edgx_clk_cs {
	struct clocksource  cs;
	struct edgx_clk    *clk;
};

struct edgx_clk {
	edgx_io_t             *iobase;
	struct edgx_time      *parent;
	struct ptp_clock_info  ptpinfo;
	struct edgx_clk_cs    *clkcs;
	/* clock-access lock:
	 * Use spinlock, since clk functions are also called from atomic
	 * context. raw_spinlock will also not be transformed into a (sleeping)
	 * mutex when used with RT-patch.
	 */
	raw_spinlock_t         lock;
	struct {
		u32 ns;
		u32 sns;
	}                      step;
	struct ptp_clock      *ptp;
	u16                    tmmux_id;
};

struct edgx_time {
	edgx_io_t            *iobase;
	struct edgx_br       *parent;
	ptvec_t               ptmap;

	struct edgx_clk       clk[EDGX_TM_NCLOCKS];
	struct edgx_clk      *rclk[EDGX_TM_NCLOCKS];
	edgx_io_t            *xtsbase;
};

#define ptp2clk(_ptp)	container_of(_ptp, struct edgx_clk, ptpinfo)
#define clk2br(_clk)	((_clk)->parent->parent)
#define cs2clkcs(_cs)	container_of(_cs, struct edgx_clk_cs, cs)

static inline void edgx_clk_cmd(struct edgx_clk *clk, u16 cmd)
{
	edgx_wr16(clk->iobase, FRTC_CMD, cmd);
}

static int edgx_clk_adjfine(struct ptp_clock_info *ptp, long scaled_ppm)
{
	struct edgx_clk *clk = ptp2clk(ptp);
	u64 adj = ((u64)clk->step.ns << 32) | (u64)clk->step.sns;
	unsigned long flags;

	adj = adjust_by_scaled_ppm(adj, scaled_ppm);
	if ((adj >> 32) > FRTC_STEP_NS_MAX)
		return -EINVAL;

	raw_spin_lock_irqsave(&clk->lock, flags);

	edgx_wr32(clk->iobase, FRTC_STEP_NS, adj >> 32);
	edgx_wr32(clk->iobase, FRTC_STEP_SNS, adj);

	edgx_clk_cmd(clk, FRTC_CMD_ADJ_STEP);
	raw_spin_unlock_irqrestore(&clk->lock, flags);

	return 0;
}

static int edgx_clk_adjtime(struct ptp_clock_info *ptp, s64 delta)
{
	struct edgx_clk *clk = ptp2clk(ptp);
	struct timespec64 ts = ns_to_timespec64(delta);
	u64 adj_sec = ts.tv_sec;
	u32 adj_nsec = ts.tv_nsec;
	unsigned long flags;

	if (delta < 0) {
		/* Delta is negative, so make it positive and calculate 2's
		 * complement manually (bitwise NOT plus 1), because
		 * 1.) shift/mask on signed number is undefined
		 * 2.) representation of negative numbers is undefined
		 */
		adj_sec = (~(-ts.tv_sec)) + 1;
	}

	raw_spin_lock_irqsave(&clk->lock, flags);

	edgx_set32(clk->iobase, FRTC_ADJ_SEC_HI, 15, 0,
		   (adj_sec >> 32) & 0xFFFF);
	edgx_wr32(clk->iobase, FRTC_ADJ_SEC_LO, adj_sec & 0xFFFFFFFF);
	edgx_wr32(clk->iobase, FRTC_ADJ_NSEC, adj_nsec);

	edgx_clk_cmd(clk, FRTC_CMD_ADJ_TIME);
	raw_spin_unlock_irqrestore(&clk->lock, flags);

	return 0;
}

static inline int _edgx_clk_adjtime_ts(struct ptp_clock_info *ptp,
				       struct timespec64 *ts)
{
	return edgx_clk_adjtime(ptp, timespec64_to_ns(ts));
}

static void edgx_clk_read_ts(struct edgx_clk *clk, struct timespec64 *ts)
{
	/* clk->lock must be held when calling this function! */
	edgx_clk_cmd(clk, FRTC_CMD_RD_TIME);
	/* omission of FRTC_CUR_SEC_HI is done by intention */
	ts->tv_sec  = (u64)edgx_rd32(clk->iobase, FRTC_CUR_SEC_LO);

	ts->tv_nsec = edgx_get32(clk->iobase, FRTC_CUR_NSEC, 29, 0);
}

static u64 edgx_clk_cs_read(struct clocksource *cs)
{
	struct edgx_clk_cs    *clkcs = cs2clkcs(cs);
	struct edgx_clk *clk = clkcs->clk;
	u64 cc;
	unsigned long flags;

	raw_spin_lock_irqsave(&clk->lock, flags);

	edgx_clk_cmd(clk, FRTC_CMD_RD_TIME);
	cc = (u64)edgx_get32(clk->iobase, FRTC_CUR_CC_HI, 15, 0) << 32 |
	     (u64)edgx_rd32(clk->iobase, FRTC_CUR_CC_LO);

	raw_spin_unlock_irqrestore(&clk->lock, flags);
	return cc;
}

static void edgx_clk_read_ts_cc(struct edgx_clk *clk, struct timespec64 *ts,
				u64 *cc)
{
	unsigned long flags;

	raw_spin_lock_irqsave(&clk->lock, flags);

	edgx_clk_read_ts(clk, ts);
	*cc = (u64)edgx_get32(clk->iobase, FRTC_CUR_CC_HI, 15, 0) << 32 |
	      (u64)edgx_rd32(clk->iobase, FRTC_CUR_CC_LO);

	raw_spin_unlock_irqrestore(&clk->lock, flags);
}

static int edgx_clk_cs_init(struct edgx_clk *clk)
{
	int ret;
	u32 clkrate;
	struct edgx_clk_cs *clkcs;

	clkcs = kzalloc(sizeof(*clkcs), GFP_KERNEL);
	if (!clkcs)
		return -ENOMEM;

	clkcs->cs.name = "edge_timer";
	clkcs->cs.rating = csrating;
	clkcs->cs.read = edgx_clk_cs_read;
	clkcs->cs.mask = CLOCKSOURCE_MASK(48);
	clkcs->cs.flags = CLOCK_SOURCE_IS_CONTINUOUS;
	clkcs->cs.owner = THIS_MODULE;
	clkcs->clk = clk;
	clk->clkcs = clkcs;

	clkrate = NSEC_PER_SEC / clk->step.ns;
	pr_info("Clkrate calculated: %u Hz\n", clkrate);
	ret = clocksource_register_hz(&clkcs->cs, clkrate);
	if (ret) {
		kfree(clk->clkcs);
		clk->clkcs = NULL;
		return ret;
	}

	return 0;
}

static void edgx_clk_cs_exit(struct edgx_clk *clk)
{
	if (!clk->clkcs)
		return;

	clocksource_unregister(&clk->clkcs->cs);
	kfree(clk->clkcs);
	clk->clkcs = NULL;
}

static int edgx_clk_gettime64(struct ptp_clock_info *ptp, struct timespec64 *ts)
{
	struct edgx_clk *clk = ptp2clk(ptp);
	unsigned long flags;

	raw_spin_lock_irqsave(&clk->lock, flags);
	edgx_clk_read_ts(clk, ts);
	raw_spin_unlock_irqrestore(&clk->lock, flags);

	return 0;
}

static int edgx_clk_getcrosststamp(struct ptp_clock_info *ptp,
				   struct system_device_crosststamp *cts)
{
	struct timespec64 ts_dev;
	struct timespec64 ts_sys;

	edgx_clk_gettime64(ptp, &ts_dev);
	ktime_get_real_ts64(&ts_sys);

	cts->device = ktime_set(ts_dev.tv_sec, ts_dev.tv_nsec);
	cts->sys_realtime = ktime_set(ts_sys.tv_sec, ts_sys.tv_nsec);
	cts->sys_monoraw = ktime_set(0, 0);

	return 0;
}

static int edgx_clk_cs_get(ktime_t *device, struct system_counterval_t *system,
			   void *ctx)
{
	struct edgx_clk *clk = (struct edgx_clk *)ctx;
	struct timespec64 ts_dev;
	u64 cc;

	edgx_clk_read_ts_cc(clk, &ts_dev, &cc);
	*device = ktime_set(ts_dev.tv_sec, ts_dev.tv_nsec);
	system->cycles = cc;
#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 9, 0)
	system->cs = &clk->clkcs->cs;
#else
	system->cs_id = clk->clkcs->cs.id;
#endif

	return 0;
}

static int edgx_clk_cs_getcrosststamp(struct ptp_clock_info *ptp,
				      struct system_device_crosststamp *cts)
{
	struct edgx_clk *clk = ptp2clk(ptp);
	int ret;

	if (!clk->clkcs)
		return edgx_clk_getcrosststamp(ptp, cts);

	ret = get_device_system_crosststamp(edgx_clk_cs_get, clk, NULL, cts);
	if (ret == -ENODEV)
		return edgx_clk_getcrosststamp(ptp, cts);

	return ret;
}

static int edgx_clk_settime64(struct ptp_clock_info *ptp,
			      const struct timespec64 *ts)
{
	struct timespec64 _ts;

	edgx_clk_gettime64(ptp, &_ts);
	_ts = timespec64_sub(*ts, _ts);
	/* adjust by difference between current and requested time */
	return _edgx_clk_adjtime_ts(ptp, &_ts);
}

static const struct ptp_clock_info edgx_tm_ptp_info = {
	.owner          = THIS_MODULE,
	/* Maximum correction in ppb possible, assuming nominal initial value
	 * step size of 10ns or 8ns (typical). Exotic settings of generics
	 * DEFAULT_STEP_NS and DEFAULT_STEP_SNS require explicit calculation
	 */
	.max_adj        = S32_MAX,
	.n_alarm        = 0,
	.n_ext_ts       = 0,
	.n_per_out      = 0,
	.n_pins         = 0,
	.pps            = 0,
	.adjfine        = edgx_clk_adjfine,
	.adjtime        = edgx_clk_adjtime,
	.gettime64      = edgx_clk_gettime64,
	.getcrosststamp = edgx_clk_cs_getcrosststamp,
	.settime64      = edgx_clk_settime64,
};

static int edgx_tm_set_role_phc(struct edgx_time *time, enum edgx_clk_role role,
				int ptpidx)
{
	int id;

	for (id = 0; id < EDGX_TM_NCLOCKS; id++)
		if (ptp_clock_index(time->clk[id].ptp) == ptpidx)
			break;
	if (id >= EDGX_TM_NCLOCKS) {
		edgx_br_err(time->parent, "PHC %d not controlled by device\n",
			    id);
		return -EINVAL;
	}

	/* DE-IP implementation can only mux the worker clock */
	if (role != EDGX_TM_ROLE_WRK) {
		edgx_br_err(time->parent, "Cannot use PHC %d in requested role\n",
			    ptpidx);
		return -ENOTSUPP;
	}

	/* We only support a single time multiplexer (TMMUX) now */
	edgx_wr16(time->iobase, IBC_TMMUX_CTL, time->clk[id].tmmux_id);
	edgx_wr16(time->iobase, IBC_GPMUX_CTL, time->clk[id].tmmux_id);
	time->rclk[role] = &time->clk[id];

	return 0;
}

int edgx_tm_get_phc(const struct edgx_time *time, unsigned int idx)
{
	if (!time || idx >= EDGX_TM_NCLOCKS)
		return -EINVAL;
	return ptp_clock_index(time->clk[idx].ptp);
}

int edgx_tm_get_role_phc(const struct edgx_time *time, enum edgx_clk_role role)
{
	return (time) ? ptp_clock_index(time->rclk[role]->ptp) : -EINVAL;
}

int edgx_tm_get_role_time(const struct edgx_time *time,
			  enum edgx_clk_role role,
			  struct timespec64 *ts)
{
	return (time && ts) ? edgx_clk_gettime64(&time->rclk[role]->ptpinfo, ts)
			    : -EINVAL;
}

struct edgx_phc_xtstamp {
	struct ptp_clock_time ts;
	struct ptp_clock_time wrk;
};

static inline struct edgx_time *edgx_dev2time(struct device *dev)
{
	return edgx_br_get_br_time(edgx_dev2br(dev));
}

static ssize_t phc_xtstamp_read(struct file *filp, struct kobject *kobj,
				edgx_bin_attr *bin_attr,
				char *buf, loff_t ofs, size_t count)
{
	struct edgx_time	*time	 = edgx_dev2time(kobj_to_dev(kobj));
	struct edgx_phc_xtstamp	*xts	 = (struct edgx_phc_xtstamp *)buf;
	/* FRTC0 triggers the FPTS */
	struct edgx_clk		*trigger = &time->clk[0];
	edgx_io_t		*xtsbase = trigger->parent->xtsbase;
	struct timespec64	 ts_trigg;
	struct timespec64	 ts_latch;
	unsigned long		 flags;
	int			 ret;

	if (ofs != 0)
		return -EINVAL;
	if (count != sizeof(*xts))
		return -EINVAL;

	if (time->rclk[EDGX_TM_ROLE_TS] == time->rclk[EDGX_TM_ROLE_WRK]) {
		/* trivial case */
		ret = edgx_tm_get_ts_time(time, &ts_trigg);
		if (ret)
			return ret;

		xts->ts = (struct ptp_clock_time){
			.sec = ts_trigg.tv_sec,
			.nsec = ts_trigg.tv_nsec
		};
		xts->wrk = xts->ts;
		return count;
	}

	raw_spin_lock_irqsave(&trigger->lock, flags);
	edgx_wr16(xtsbase, FPTS_TS_CTL, 1); /* start FPTS recording */

	/* The following reading has the impact, that actual time from
	 * clock latch gets to block FPTS.
	 */
	edgx_clk_read_ts(trigger, &ts_trigg); /* read time = trigger FPTS */

	/* omission of FPTS_TS_SEC_HI is done by intention */
	ts_latch.tv_sec  = (u64)edgx_rd32(xtsbase, FPTS_TS_SEC_LO);
	ts_latch.tv_nsec = edgx_get32(xtsbase, FPTS_TS_NSEC, 29, 0);

	raw_spin_unlock_irqrestore(&trigger->lock, flags);

	xts->ts = (struct ptp_clock_time){
		.sec = ts_trigg.tv_sec,
		.nsec = ts_trigg.tv_nsec
	};
	xts->wrk = (struct ptp_clock_time){
		.sec = ts_latch.tv_sec,
		.nsec = ts_latch.tv_nsec
	};

	return count;
}

EDGX_BIN_ATTR_RO(phc_xtstamp, "phc_xtstamp", sizeof(struct edgx_phc_xtstamp));

static edgx_bin_attr *edgex_ext_binattrs[] = {
        &bin_attr_phc_xtstamp,
        NULL,
};

static struct attribute_group edgex_ext_group = {
        .name = "edgex-ext",
        .bin_attrs = edgex_ext_binattrs,
};

static int edgx_tm_sysfs_add(struct edgx_time *time)
{
	struct edgx_br *br = time->parent;
	struct attribute_group *grp = &edgex_ext_group;
	int ret = edgx_br_sysfs_add(br, grp);

	if (ret)
		edgx_br_warn(br, "Can't create sysfs group %s\n", grp->name);
	return ret;
}

int edgx_probe_time(struct edgx_br *br, struct edgx_time **ptime, ptvec_t *map)
{
	const struct edgx_ifdesc *ifd;
	const struct edgx_ifreq   ifreq = { .id = AC_CLOCK_ID, .v_maj = 1 };
	struct timespec64         now;
	struct edgx_time         *time;
	u16                       reg, nclk, nxts, ntmux;
	int                       i;
	int                       ret;

	if (!br || !ptime)
		return -EINVAL;
	ifd = edgx_ac_get_if(&ifreq);
	if (!ifd)
		return -ENODEV;
	*ptime = kzalloc(sizeof(**ptime), GFP_KERNEL);
	if (!(*ptime))
		return -ENOMEM;
	time = *ptime;

	time->parent = br;
	time->ptmap  = ifd->ptmap;
	*map         = ifd->ptmap;

	time->iobase = ifd->iobase;

	/* Check if TIME interface scaling meets requirements */
	nclk  = edgx_get16(time->iobase, IBC_AC_NRTC, 1, 0);
	nxts  = edgx_get16(time->iobase, IBC_AC_NRTC, 3, 2);
	ntmux = edgx_rd16(time->iobase, IBC_GX_TMUX);
	if (nclk < EDGX_TM_NCLOCKS || nxts < EDGX_TM_NXTS ||
	    ntmux < EDGX_TM_NTMUX) {
		edgx_br_err(br, "Invalid scaling of TIME interface detected!\n");
		edgx_br_err(br, "Detected %d CLK(s), %d XTS, %d TMUX\n",
			    nclk, nxts, ntmux);
		edgx_br_err(br, "Required %d CLK(s), %d XTS, %d TMUX\n",
			    EDGX_TM_NCLOCKS, EDGX_TM_NXTS, EDGX_TM_NTMUX);
		goto out_dev;
	}

	time->xtsbase = time->iobase + (edgx_rd16(time->iobase, IBC_AC_XTS) <<
					IBC_AC_SHIFT);
	for (i = 0; i < EDGX_TM_NCLOCKS; i++) {
		struct edgx_clk *clk = &time->clk[i];

		clk->iobase   = time->iobase + (edgx_rd16(time->iobase,
						IBC_AC_RTC(i)) << IBC_AC_SHIFT);
		clk->parent   = time;
		clk->tmmux_id = i;
		clk->ptpinfo  = edgx_tm_ptp_info;
		scnprintf(clk->ptpinfo.name, 16, "edge_ptpclk_%d", i);
		raw_spin_lock_init(&clk->lock);
		clk->step.ns  = edgx_rd32(clk->iobase, FRTC_GX_STEP_NS);
		clk->step.sns = edgx_rd32(clk->iobase, FRTC_GX_STEP_SNS);
		/* Write stepping as we might just have reloaded and are not
		 * coming out of HW reset.
		 */
		edgx_wr32(clk->iobase, FRTC_STEP_NS,  clk->step.ns);
		edgx_wr32(clk->iobase, FRTC_STEP_SNS, clk->step.sns);

		clk->ptp = ptp_clock_register(&clk->ptpinfo,
					      edgx_br_get_dev(br));

		if (IS_ERR(clk->ptp))
			goto out_clk;

		/* Use system time by default */
		ktime_get_real_ts64(&now);
		edgx_clk_settime64(&clk->ptpinfo, &now);
	}

	reg = edgx_rd16(time->iobase, IBC_TMMUX_CTL);
	reg &= IBC_TMMUX0_MASK;

	/* FRTC0 hardwired timestamper */
	time->rclk[EDGX_TM_ROLE_TS]  = &time->clk[0];
	time->rclk[EDGX_TM_ROLE_WRK] = &time->clk[reg];

	edgx_br_info(br, "Synchronisation mode (syncmode): %s\n",
		     syncmode);

	if (!strncmp(syncmode, "1588", 4)) {
		/* Use FRTC0 as worker (IEEE 1588 setup) */
		edgx_tm_set_role_phc(time, EDGX_TM_ROLE_WRK,
				     ptp_clock_index(time->clk[0].ptp));
	} else {
		if (strncmp(syncmode, "1AS", 3))
			edgx_br_warn(br, "Invalid syncmode set! Using IEEE 802.1AS.\n");

		/* Use FRTC1 as worker by default (802.1AS setup) */
		edgx_tm_set_role_phc(time, EDGX_TM_ROLE_WRK,
				     ptp_clock_index(time->clk[1].ptp));
	}
	if (edgx_clk_cs_init(time->rclk[EDGX_TM_ROLE_WRK])) {
		edgx_br_err(br, "Cannot initialize clocksource!\n");
		goto out_clk;
	}
	edgx_br_info(br, "Clock source rating (csrating): %d\n", csrating);

	ret = edgx_tm_sysfs_add(time);
	if (ret)
		goto out_sysfs;

	edgx_br_info(br, "Timestamper clock: /dev/ptp%d\n",
		     edgx_tm_get_ts_phc(time));
	edgx_br_info(br, "Worker clock: /dev/ptp%d\n",
		     edgx_tm_get_wrk_phc(time));
	edgx_br_info(br, "Setup Time Provider ... done\n");

	return 0;

out_sysfs:
out_clk:
	edgx_shutdown_time(time);
	return -ENODEV;

out_dev:
	kfree(*ptime);
	return -ENODEV;
}

void edgx_shutdown_time(struct edgx_time *time)
{
	if (time) {
		unsigned int i;

		for (i = 0; i < EDGX_TM_NCLOCKS; i++) {
			edgx_clk_cs_exit(&time->clk[i]);
			if (time->clk[i].ptp)
				ptp_clock_unregister(time->clk[i].ptp);
		}
		kfree(time);
	}
}
