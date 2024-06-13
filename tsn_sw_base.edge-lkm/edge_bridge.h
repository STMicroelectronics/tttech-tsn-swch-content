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

#ifndef _EDGE_BRIDGE_H
#define _EDGE_BRIDGE_H

struct edgx_br;

#include <linux/kernel.h>
#include "edge_defines.h"

#define EDGX_BR_GEN_REG		(0x10)
#define EDGX_BR_INT_MASK_CLR	(0x030)
#define EDGX_BR_INT_MASK_SET	(0x034)
#define EDGX_BR_INT_STAT	(0x038)

#define EDGX_BR_DEFAULT_RESMACTC (1)

enum edgx_br_irq_nr {
	EDGX_IRQ_NR_TS_TX = 0,
	EDGX_IRQ_NR_DMA_TX = 4,
	EDGX_IRQ_NR_DMA_RX = 5,
	EDGX_IRQ_NR_DMA_ERR = 6,
	EDGX_IRQ_NR_PSFP_SCHED_TAB = 9,
	EDGX_IRQ_NR_FPTS = 10,
	EDGX_IRQ_NR_ST_SCHED_TAB = 11,
	EDGX_IRQ_NR_ACM_SCHED = 12,
	EDGX_IRQ_NR_ACM_MSG_BUF = 13,
	EDGX_IRQ_CNT = 16
};

enum edgx_br_irq_trig_type {
	EDGX_IRQ_EDGE_TRIG,
	EDGX_IRQ_LEVEL_TRIG
};

enum edgx_lnk_mode {
	EDGX_LNKMOD_INVAL = 0,
	EDGX_LNKMOD_10_100_1000,
	EDGX_LNKMOD_10_100,
	EDGX_LNKMOD_100_1000,
	EDGX_LNKMOD_100
};

struct edgx_br_irq {
	int				irq_vec[EDGX_IRQ_CNT];
	enum edgx_br_irq_trig_type	trig;
	bool				shared;
};

extern int csrating;
extern char *syncmode;
extern struct attribute *ieee8021_brpt_common[];

int edgx_br_probe_one(unsigned int br_id, struct device *dev,
		      void *base, struct edgx_br_irq *irq,
		      struct edgx_br **br_ret, const char *drv_name);
void edgx_br_shutdown(struct edgx_br *br);
void *edgx_br_get_base(struct edgx_br *br);

unsigned int         edgx_br_get_id(const struct edgx_br *br);
struct device       *edgx_br_get_dev(const struct edgx_br *br);
const u8            *edgx_br_get_mac(const struct edgx_br *br);

struct edgx_pt      *edgx_br_get_brpt(const struct edgx_br *br, ptid_t ptid);
struct edgx_pt      *edgx_br_get_eppt(const struct edgx_br *br);
struct edgx_br_vlan *edgx_br_get_vlan(const struct edgx_br *br);
struct edgx_brfdb   *edgx_br_get_fdb(const struct edgx_br *br);
struct edgx_com     *edgx_br_get_com(const struct edgx_br *br);
struct edgx_sid_br  *edgx_br_get_sid(const struct edgx_br *br);
struct edgx_frer    *edgx_br_get_frer(const struct edgx_br *br);
struct edgx_psfp    *edgx_br_get_psfp(const struct edgx_br *br);
struct edgx_st_com  *edgx_br_get_st_com(const struct edgx_br *br);
u16                  edgx_br_get_generic(const struct edgx_br *br, size_t ofs);
u32                  edgx_br_get_generic_pow2(const struct edgx_br *br,
					      size_t ofs);
u32                  edgx_br_get_generic_pow2z(const struct edgx_br *br,
					       size_t ofs);
u16                  edgx_br_get_feature(const struct edgx_br *br, size_t ofs);
u16                  edgx_br_get_num_ports(const struct edgx_br *br);
unsigned int         edgx_br_get_cycle_ns(const struct edgx_br *br);
const char          *edgx_br_get_drv_name(const struct edgx_br *br);
u32                  edgx_br_get_version(const struct edgx_br *br);

struct edgx_time    *edgx_br_get_pt_time(const struct edgx_br *br, ptid_t ptid);
struct edgx_time    *edgx_br_get_br_time(const struct edgx_br *br);
struct edgx_stat    *edgx_br_get_pt_stat(const struct edgx_br *br, ptid_t ptid);
struct workqueue_struct *edgx_br_get_owq(const struct edgx_br *br);

void edgx_br_set_int_mask(struct edgx_br *br, u16 mask);
void edgx_br_clr_int_mask(struct edgx_br *br, u16 mask);
void edgx_br_clr_int_stat(struct edgx_br *br, u16 stat);
u16 edgx_br_get_int_mask(struct edgx_br *br);
u16 edgx_br_get_int_stat(struct edgx_br *br);
void edgx_br_irq_enable(struct edgx_br *br, enum edgx_br_irq_nr irq);
void edgx_br_irq_disable(struct edgx_br *br, enum edgx_br_irq_nr irq);

int     edgx_br_ageing_set(struct edgx_br *br, clock_t ageing_time);
clock_t edgx_br_ageing_get(const struct edgx_br *br);

int     edgx_br_pt_join(struct edgx_br *br, struct net_device *netdev);
void    edgx_br_pt_leave(struct edgx_br *br);

int     edgx_br_sysfs_add(struct edgx_br *br, struct attribute_group *grp);

struct edgx_br *edgx_dev2br(struct device *dev);

u16      edgx_br_rdreg(struct edgx_br *br, size_t ofs);
void     edgx_br_wrreg(struct edgx_br *br, size_t ofs, u16 val);

u16 edgx_get_tc_mgmtraffic(struct edgx_br *br);

#define edgx_br_err(_br, _fmt, ...) pr_err("bridge-%d: " _fmt,        \
					   edgx_br_get_id(_br), ##__VA_ARGS__)

#define edgx_br_warn(_br, _fmt, ...) pr_warn("bridge-%d: " _fmt,   \
					     edgx_br_get_id(_br), ##__VA_ARGS__)

#define edgx_br_info(_br, _fmt, ...) pr_info("bridge-%d: " _fmt,      \
					    edgx_br_get_id(_br), ##__VA_ARGS__)

#endif /* _EDGX_BRIDGE_H */
