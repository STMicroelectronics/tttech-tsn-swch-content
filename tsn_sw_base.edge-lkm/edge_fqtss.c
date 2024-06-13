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

#include "edge_fqtss.h"
#include "edge_port.h"
#include "edge_ac.h"
#include "edge_util.h"
#include "edge_bridge.h"

/*Offset calculated for shapers register address*/
#define OFFS(IDX)	(0x30 + (IDX) * 2)
/*1000 mbps*/
#define MAX_LINE_SPEED	(1000000000)
/*Mac value to be written into shapers register in case of strict prio*/
#define MAX_ADDEND		(65535)
/*Speed is returned in Mbps, this value is used for conversion to bps*/
#define MEGA_VAL	(1000000)
/*Transmission selection algorithm CBS */
#define CBS		(1)
/*Transmission selection algorithm Strict priority */
#define STRICT_PRIO	(0)
/*Calculation of percentage*/
#define PERCENTAGE(NUM, PERC) ((NUM) * (PERC) / 100)

struct edgx_fqtss {
	struct edgx_pt		*parent;
	edgx_io_t		*iobase;
			/* Transmission selection algorithm table*/
	u8			tsa_tbl[EDGX_BR_MAX_TC];
			/* table,used for admin slope val*/
	u32			adminidleslope[EDGX_BR_MAX_TC];
			/* table used for internal values of idleslopes*/
	u32			idleslope[EDGX_BR_MAX_TC];
			/* addend is the rate at which credit increases*/
	u16			addend[EDGX_BR_MAX_TC];
			/*Protect writing to shapers register*/
	struct mutex		lock;
};

static inline struct edgx_fqtss *edgx_dev2fqtss(struct device *dev)
{
	return edgx_pt_get_fqtss(edgx_dev2pt(dev));
}

/**
 * Calculate addend value based on bitrate. Addend defines the rate at
 * which credit increases which is also the rate the queue is shaped to.
 * This is the only configurable value to be written to shaper register
 * The resolution of the rate adjustment is:
 * Rate (resolution) = 1/4096 Bytes * clk_freq Hz / 8
 */
static u16 edgx_fqtss_calc_addend(u32 bitrate, u32 clk)
{
	u64 addend_tmp;

	/* rate x 8 x 4096 / (clk x 8) = (rate x 2^12) / clk  */
	addend_tmp = ((uint64_t)bitrate << 12) + clk - 1u;
	do_div(addend_tmp, clk);
	if (addend_tmp == 1)
		pr_warn("Min bitrate value set in shaper reg\n");

	return (uint16_t)addend_tmp;
}

/**
 * Write addend value to shaper register in case corresponding TSA is set
 * to CBS. If it is not, set the val to MAX val. Save value of addend to
 * addend table
 */
static void edgx_fqtss_write_addend(struct edgx_fqtss *fqtss,
				    u16 addend, u8 idx, u32 clk)
{
	mutex_lock(&fqtss->lock);
	fqtss->addend[idx] = addend;
	if (fqtss->tsa_tbl[idx] == CBS)
		edgx_wr16(fqtss->iobase, OFFS(idx), addend);
	else if (fqtss->tsa_tbl[idx] == STRICT_PRIO)
		edgx_wr16(fqtss->iobase, OFFS(idx), MAX_ADDEND);
	mutex_unlock(&fqtss->lock);
}

/*period in ns, we need frequency in Hz -> 1/(cycle*10^-9) = 10^9/cycle */
static u32 edgx_fqtss_get_clk(struct edgx_fqtss *fqtss)
{
	int cycle = edgx_br_get_cycle_ns(edgx_pt_get_br(fqtss->parent));

	return (1000000000 / cycle);
}

/**
 * Function does the calculation of Idle slope based on the values retrieved
 * from scheduler, derived from IEEE 802.1.Q [IEEE18] section 8.6.8.2
 * idleSlope = (operIdleSlope(N) x OperCycleTime / GateOpenTime)
 */
static u16 edgx_fqtss_calc_idle_slope(struct edgx_fqtss *fqtss, u8 idx,
				      u64 num, u64 dnm)
{
	u16 addend = 0;
	u64 tmp = 0;
	u32 bitrate = 0;
	int clk = edgx_fqtss_get_clk(fqtss);
	int link_speed;

	if ((!num) || (!dnm))
		return 0;

	if ((num == 1) && (dnm == 1))
		return fqtss->addend[idx];

	link_speed = edgx_pt_get_speed(fqtss->parent);
	bitrate = fqtss->adminidleslope[idx];
	/* Overflow check: a*b > c if and only if a > c/b */
	if (bitrate > (div64_u64(U64_MAX, num))) {
		/* try to first divide and then multiply */
		if (div64_u64 (num, dnm) > 0) {
			tmp = (div64_u64 (num, dnm));
			bitrate = (u32)tmp * bitrate;
		} else {
			// a/b=0 -> b>a -> x=b/a in order to see how much is
			// b bigger than a
			tmp = div64_u64 (dnm, num);
			num = num * tmp;
			tmp = div64_u64 (num, dnm);
			bitrate = (u32)tmp * bitrate;
			bitrate = div64_u64 (bitrate, tmp);
		}
	} else {
		tmp = ((u64)bitrate * num);
		bitrate = div64_u64(tmp, dnm);
	}

	if ((link_speed == SPEED_UNKNOWN) && (bitrate > MAX_LINE_SPEED)) {
		bitrate = MAX_LINE_SPEED;
		edgx_pt_warn(fqtss->parent, "TSA: Speed unknown\n");
	} else if (bitrate > (link_speed * MEGA_VAL)) {
		bitrate = link_speed * MEGA_VAL;
		edgx_pt_warn(fqtss->parent, "TSA:set max line speed\n");
	}

	mutex_lock(&fqtss->lock);
	fqtss->idleslope[idx] = bitrate;
	mutex_unlock(&fqtss->lock);
	addend = edgx_fqtss_calc_addend(bitrate, clk);

	return addend;
}

void edgx_fqtss_st_change(struct edgx_fqtss *fqt, struct edgx_st *st)
{
	u8	i = 0;
	u16	addend = 0;
	int	clk;
	u8	nr_queues;
	struct edgx_st_tr_rate tr_rate;

	if (!fqt || !st)
		return;

	clk = edgx_fqtss_get_clk(fqt);
	nr_queues = edgx_br_get_generic(edgx_pt_get_br(fqt->parent),
					BR_GX_QUEUES);

	/*Recalculate value in shaper reg in case TSA = CBS*/
	for (i = 0; i < nr_queues; i++) {
		/* This function is called in the context of sched - that means
		 * that the scheduler data already locked. Thus here the
		 * function edgx_sched_get_trans_rate_locked can/must be
		 * called, because the call of edgx_sched_get_trans_rate
		 * would result in a dead lock.
		 */
		edgx_st_get_trans_rate_locked(st, i, &tr_rate);
		addend = edgx_fqtss_calc_idle_slope(fqt, i, tr_rate.num,
						    tr_rate.denom);
		edgx_fqtss_write_addend(fqt, addend, i, clk);
	}
}

static ssize_t admin_slope_tbl_read(struct file *filp, struct kobject *kobj,
				    struct bin_attribute *bin_attr,
				    char *buf, loff_t ofs, size_t count)
{
	loff_t	idx = 0;
	struct	edgx_fqtss *fqt = edgx_dev2fqtss(kobj_to_dev(kobj));
	u8	nr_queues = edgx_br_get_generic(edgx_pt_get_br(fqt->parent),
						BR_GX_QUEUES);

	if (edgx_sysfs_tbl_params(ofs, count, sizeof(u64), &idx) ||
	    idx > (nr_queues - 1))
		return -EINVAL;

	((u64 *)buf)[0] = fqt->adminidleslope[idx];
	return count;
}

static ssize_t oper_slope_tbl_read(struct file *filp, struct kobject *kobj,
				   struct bin_attribute *bin_attr,
				   char *buf, loff_t ofs, size_t count)
{
/* operIdleSlope and adminIdleSlope are equal as SRP is not in operation.*/
	return admin_slope_tbl_read(filp, kobj, bin_attr, buf, ofs, count);
}

static ssize_t admin_slope_tbl_write(struct file *filp, struct kobject *kobj,
				     struct bin_attribute *bin_attr,
				     char *buf, loff_t ofs, size_t count)
{
	loff_t	idx = 0;
	u16	addend;
	struct edgx_fqtss *fqtss = edgx_dev2fqtss(kobj_to_dev(kobj));
	u32	clk = edgx_fqtss_get_clk(fqtss);
	int	link_speed;
	struct edgx_st_tr_rate tr_rate;
	struct edgx_st *st = edgx_pt_get_st(fqtss->parent);
	u8	nr_queues = edgx_br_get_generic(edgx_pt_get_br(fqtss->parent),
						BR_GX_QUEUES);

	if (edgx_sysfs_tbl_params(ofs, count, sizeof(u64), &idx) ||
	    idx > (nr_queues - 1))
		return -EINVAL;

	link_speed = edgx_pt_get_speed(fqtss->parent);
	/*Setting slope to value greater than link speed is forbidden*/
	if (link_speed == SPEED_UNKNOWN) {
		edgx_pt_warn(fqtss->parent, "Unknown port speed\n");
		if (*((u32 *)buf) > MAX_LINE_SPEED)
			return -EINVAL;
	} else if (*((u32 *)buf) > (link_speed * MEGA_VAL)) {
		return -EINVAL;
	}

	addend = edgx_fqtss_calc_addend(*((u32 *)buf), clk);

	mutex_lock(&fqtss->lock);
	fqtss->adminidleslope[idx] = *((u32 *)buf);
	fqtss->idleslope[idx] = *((u32 *)buf);
	fqtss->addend[idx] = addend;
	mutex_unlock(&fqtss->lock);

	edgx_st_get_trans_rate(st, idx, &tr_rate);
	addend = edgx_fqtss_calc_idle_slope(fqtss, idx, tr_rate.num,
					    tr_rate.denom);
	edgx_fqtss_write_addend(fqtss, addend, (u8)idx, clk);

	return count;
}

static ssize_t tsa_tbl_read(struct file *filp, struct kobject *kobj,
			    struct bin_attribute *bin_attr,
			    char *buf, loff_t ofs, size_t count)
{
	loff_t	idx = 0;
	struct	edgx_fqtss *fqtss = edgx_dev2fqtss(kobj_to_dev(kobj));
	u8	nr_queues = edgx_br_get_generic(edgx_pt_get_br(fqtss->parent),
						BR_GX_QUEUES);

	if (edgx_sysfs_tbl_params(ofs, count, sizeof(u8), &idx) ||
	    idx > (nr_queues - 1))
		return -EINVAL;

	((u8 *)buf)[0] =  fqtss->tsa_tbl[idx];

	return count;
}

static ssize_t tsa_tbl_write(struct file *filp, struct kobject *kobj,
			     struct bin_attribute *bin_attr,
			     char *buf, loff_t ofs, size_t count)
{
	loff_t	i = 0;
	struct edgx_fqtss *fqt = edgx_dev2fqtss(kobj_to_dev(kobj));
	unsigned int clk = edgx_fqtss_get_clk(fqt);
	u8	nr_queues = edgx_br_get_generic(edgx_pt_get_br(fqt->parent),
						BR_GX_QUEUES);

	if (edgx_sysfs_tbl_params(ofs, count, sizeof(u8), &i) ||
	    i > (nr_queues - 1) ||
	    *((u8 *)buf) > CBS)
		return -EINVAL;

	fqt->tsa_tbl[i] = *((u8 *)buf);
	edgx_fqtss_write_addend(fqt, fqt->addend[i], i, clk);

	return count;
}

EDGX_BIN_ATTR_RO(oper_slope_tbl,  "operIdleSlopeTable",
		EDGX_BR_MAX_TC * sizeof(u64));
EDGX_BIN_ATTR_RW(admin_slope_tbl, "adminIdleSlopeTable",
		EDGX_BR_MAX_TC * sizeof(u64));
EDGX_BIN_ATTR_RW(tsa_tbl, "TxSelectionAlgorithmTable",
		EDGX_BR_MAX_TC * sizeof(u8));

static struct bin_attribute *ieee8021fqtss_binattrs[] = {
	&bin_attr_oper_slope_tbl,
	&bin_attr_admin_slope_tbl,
	&bin_attr_tsa_tbl,
	NULL
	};

static struct attribute_group ts_algorithm_group = {
	.name  = "ieee8021Fqtss",
	.bin_attrs = ieee8021fqtss_binattrs,
};

int edgx_probe_fqtss(struct edgx_pt *pt,
		     edgx_io_t *iobase, struct edgx_fqtss **pt_fqtss)
{
	const struct edgx_ifdesc *ifd_pt;
	const struct edgx_ifreq	ifr = {.id = AC_SHAPERS, .v_maj = 1};
	ptid_t			ptid = edgx_pt_get_id(pt);
	struct edgx_ifdesc	ptifd;
	u16			i = 0;
	struct edgx_fqtss	*fqtss;
	unsigned int		clk = 0;
	u8 nr_queues = edgx_br_get_generic(edgx_pt_get_br(pt), BR_GX_QUEUES);

	/* Get the port specific part of the HW ITF */
	ifd_pt = edgx_ac_get_ptif(&ifr, ptid, &ptifd);
	if (!ifd_pt)
		return -ENODEV;

	fqtss = kzalloc(sizeof(*fqtss), GFP_KERNEL);
	if (!(fqtss)) {
		pr_info("Cannot allocate memory\n ");
		return -ENOMEM;
	}

	(fqtss)->iobase = iobase;
	(fqtss)->parent = pt;
	mutex_init(&fqtss->lock);
	clk = edgx_fqtss_get_clk(fqtss);

	/* addend and slope values were already initialized to 0 by usage of
	 * kzalloc for fqtss; Therefore just initialize addend values in IP */
	for (i = 0; i < nr_queues; i++) {
		edgx_wr16(fqtss->iobase, OFFS(i), MAX_ADDEND);
	}

	*pt_fqtss = fqtss;
	edgx_pt_add_sysfs(pt, &ts_algorithm_group);

	return 0;
}

void edgx_shutdown_fqtss(struct edgx_fqtss *fqt)
{
	if (fqt) {
		edgx_pt_rem_sysfs(fqt->parent, &ts_algorithm_group);
		kfree(fqt);
	}
}
