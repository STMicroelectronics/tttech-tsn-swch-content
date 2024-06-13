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

#ifndef _EDGE_AC_H
#define _EDGE_AC_H

#include "edge_defines.h"

struct edgx_ifver {
	u8 v_min;
	u8 v_maj;
} __packed;

struct edgx_ifreq {
	u16 id;
	u8  v_maj;
};

struct edgx_ifdesc {
	u16                id;
	struct edgx_ifver  ver;
	edgx_io_t         *iobase;
	size_t             len;
	ptvec_t            ptmap;
};

#define AC_MAX_ENTRIES     (20)
#define AC_IFMAP_OFS       (0x1000)

/* TODO: This should actually go into edge_ac.c, but need it here for
 *       now to fake the interface map on the sim-device
 */
struct edgx_if {
	u16                id;
	struct edgx_ifver  ver;
	u32                addr;
	u32                len;
	u32                ptmap;
} __packed;

#define AC_PORTMASK_ALL   ((u32)-1)
#define AC_MAX_IFS        (16)

#define AC_EOT_ID         (0x0)
#define AC_AC_ID          (0x1)
#define AC_MGMT_ID        (0x10)
#define AC_PARAM_ID       (0x11)
#define AC_PORT_ID        (0x12)
#define AC_STAT_ID        (0x13)
#define AC_PREEMPT_ID     (0x14)
#define AC_SHAPERS        (0x15)
#define AC_PORT_TS_ID     (0x19)
#define AC_SID_ID         (0x1A)
#define AC_FRER_ID        (0x1B)
#define AC_PSFP_ID        (0x1C)
#define AC_SCHED_ID       (0x20)
#define AC_PSFP_GATING_ID (0x21)
#define AC_CLOCK_ID       (0x30)
#define AC_ADPT_ID        (0x40)
#define AC_DMA_ID         (0x60)

int  edgx_probe_ac(edgx_io_t *iobase, struct device *dev);
void edgx_shutdown_ac(void);

#define edgx_ac_for_each_ifpt(_p, _aifd, _apifd)                             \
	for ((_p) = find_first_bit((unsigned long *)&(_aifd)->ptmap,         \
				   EDGX_BR_MAX_PORTS),                       \
			edgx_ac_if2ptif(_aifd, _p, _apifd);                  \
	     (_p) < EDGX_BR_MAX_PORTS;                                       \
	     (_p) = find_next_bit((unsigned long *)&(_aifd)->ptmap,          \
				  EDGX_BR_MAX_PORTS, (_p) + 1),              \
			edgx_ac_if2ptif(_aifd, _p, _apifd))

const struct edgx_ifdesc *edgx_ac_get_if(const struct edgx_ifreq *ifreq);
const struct edgx_ifdesc *edgx_ac_get_ptif(const struct edgx_ifreq *ifreq,
					   ptid_t pt,
					   struct edgx_ifdesc *ptifdesc);
const struct edgx_ifdesc *edgx_ac_if2ptif(const struct edgx_ifdesc *ifdesc,
					  ptid_t pt,
					  struct edgx_ifdesc *portifdesc);

#endif /* _EDGE_AC_H */
