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

#ifndef _EDGE_ST_H
#define _EDGE_ST_H

#include "edge_port.h"

#define EDGX_ST_MAX_QUEUES	(8U)

/** Per queue transmission rate defined as
 * CycleTime / (Sum of all control list intervals for the given queue)
 */
struct edgx_st_tr_rate {
	u64 num;
	u64 denom;
};

struct edgx_st;
struct edgx_st_com;

int edgx_st_com_probe(struct edgx_br *br, struct edgx_br_irq *irq,
		      const char *drv_name,
		      struct edgx_st_com **pst_com,
		      bool *max_2_speeds);
void edgx_st_com_set_params(struct edgx_st_com *st_com,
			    u32 freq_err_abs, u32 sched_granul);
void edgx_st_com_shutdown(struct edgx_st_com *st_com);

int edgx_probe_st(struct edgx_pt *pt, struct edgx_st_com *st_com,
		  struct edgx_st **pst);
void edgx_shutdown_st(struct edgx_st *st);

int edgx_st_get_trans_rate_locked(struct edgx_st *st,
				  unsigned int queue_idx,
				  struct edgx_st_tr_rate *tr_rate);
int edgx_st_get_trans_rate(struct edgx_st *st,
			   unsigned int queue_idx,
			   struct edgx_st_tr_rate *tr_rate);

int edgx_sched_calc_delays(struct edgx_st *st);
u64 edgx_st_get_part_indep_tx_dly_hi_min(struct edgx_st *st);
u64 edgx_st_get_part_indep_tx_dly_hi_max(struct edgx_st *st);
u64 edgx_st_get_part_indep_tx_dly_lo_min(struct edgx_st *st);
u64 edgx_st_get_part_indep_tx_dly_lo_max(struct edgx_st *st);
u64 edgx_st_get_freq_err_per_cyc(struct edgx_st *st);

#endif /* _EDGE_ST_H */
