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

#ifndef _EDGE_DEFINES_H
#define _EDGE_DEFINES_H

#include <linux/if_ether.h>
#include <linux/stringify.h>

#ifndef _EDGX_GIT
	#define _EDGX_GIT
#endif
#define EDGX_GIT "[" __stringify(_EDGX_GIT) "]"

#ifndef EDGX_SW_CORE_VERSION
	#define EDGX_SW_CORE_VERSION ""
#endif

#define EDGX_SW_CORE_NAME    "edgx-sw"

#define EDGX_BR_PFX "EDGX-BR-CORE: "

#define edgx_info(_fmt, ...) pr_info(EDGX_BR_PFX _fmt, ##__VA_ARGS__)
#define edgx_warn(_fmt, ...) pr_warn(EDGX_BR_PFX _fmt, ##__VA_ARGS__)
#define edgx_err(_fmt, ...)  pr_err(EDGX_BR_PFX _fmt, ##__VA_ARGS__)
#define edgx_dbg(_fmt, ...)  pr_debug(_fmt, ##__VA_ARGS__)

#define EDGX_BR_MAX_PORTS	(16)
#define EDGX_BR_MAX_TC		(8) // max number of traffic classes/queues
#define EDGX_BR_NUM_PCP		(8) // number of pcp colums for streams

typedef u16 vid_t;
typedef u16 fid_t;
typedef u16 mstid_t;

typedef u32 ptvec_t;
typedef int ptid_t;
typedef u8  ptflags_t;

/* TODO shouldn't we use the already defined ptvec_t??? */
typedef u16 ptcom_t;

#define PT_INV_ID           (-2)
#define PT_EP_ID            (-1)
#define PT_IS_BRP_ID(ptid)  ((ptid) >  PT_EP_ID)
#define PT_IS_EP_ID(ptid)   ((ptid) == PT_EP_ID)
#define PT_ID2VEC(ptid)     BIT(ptid)
#define PT_VEC_ALL          (0xFFFF)

//TODO: #error "remove typedef and use phys_addr_t directly instead!"
typedef void __iomem edgx_io_t;

/* See 802.1Q, Sect. 8.6.1 */
#define EDGX_CIST_MSTID    ((mstid_t)0x0)
#define EDGX_TE_MSTID      ((mstid_t)0xFFE)
#define EDGX_SPBM_MSTID    ((mstid_t)0xFFC)
#define EDGX_SPBV_MSTID    ((mstid_t)0xFFD)
#define EDGX_INVALID_MSTID ((mstid_t)0xFFF)
#define EDGX_MAX_MSTID     EDGX_INVALID_MSTID

/* See 802.1Q, Sect. 5.4.1.1, item b) */
#define EDGX_MAX_SUPP_MSTI ((u16)64)

/* BR_GX_BASE resolved in edgx_br_get_generic */
/* User Manual Sect. 5.1.4 Feature Registers */
#define BR_GX_BASE           (0x0)
#define BR_GX_PORT_HIGH      (0x0)
#define BR_GX_COUNTERS	     (0x2)
#define BR_GX_TS_PORTS       (0x6)
#define BR_GX_SMAC_ROWS      (0x8)
#define BR_GX_POLICERS       (0xC)
#define BR_GX_QUEUES         (0xE)
#define BR_GX_CBS            (0x12)
#define BR_GX_GIGABIT        (0x14)
#define BR_GX_FRER_PORTS     (0x24)
#define BR_GX_FRER_ENTRIES   (0x26)
#define BR_GX_NSTREAMS	     (0x28)
#define BR_GX_NFIDS          (0x2A)
#define BR_GX_DMA_PORTS      (0x2C)
#define BR_GX_DMA_TX_DESC_RINGS (0x2E)
#define BR_GX_DMA_RX_DESC_RINGS (0x30)
#define BR_GX_PSFP_NSTREAMS  (0x38)
#define BR_GX_STREAM_NGATES  (0x3A)
#define BR_GX_SID_TAB_NROWS  (0x3C)

/* BR_FEAT_BASE resolved in edgx_br_get_feature */
/* User Manual Sect. 4.1.4 Feature Registers */
#define BR_FEAT_BASE          (BR_GX_BASE + 0x100)
#define BR_FEAT_CLKFREQ       (0x0)
#define BR_FEAT_HOLDADV_10    (0x100)
#define BR_FEAT_HOLDADV_100   (0x102)
#define BR_FEAT_HOLDADV_1000  (0x104)
#define BR_FEAT_RELADV_10     (0x108)
#define BR_FEAT_RELADV_100    (0x10A)
#define BR_FEAT_RELADV_1000   (0x10C)

#define BR_FEAT_I2G_BASE      (0x110)
#define BR_FEAT_G2O_BASE      (0x120)
#define BR_FEAT_I2SG_BASE     (0x130)
#define BR_FEAT_DLY_SPD_STEP  (2)
#define BR_FEAT_DLY_MAX_OFS   (8)

#endif /* _EDGE_DEFINES_H */
