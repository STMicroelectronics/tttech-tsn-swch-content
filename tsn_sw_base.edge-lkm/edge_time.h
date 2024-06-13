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

#ifndef _EDGE_TIME_H
#define _EDGE_TIME_H

#include "edge_bridge.h"

struct edgx_time;

enum edgx_clk_role {
	EDGX_TM_ROLE_TS = 0,
	EDGX_TM_ROLE_WRK,
	EDGX_TM_MAX_ROLES,   /* must be last, do not use! */
};

int  edgx_probe_time(const struct edgx_br *br, struct edgx_time **time,
		     ptvec_t *map);
void edgx_shutdown_time(struct edgx_time *time);

int edgx_tm_get_phc(const struct edgx_time *time, unsigned int idx);

int edgx_tm_get_role_phc(const struct edgx_time *time,
			 enum edgx_clk_role role);
#define edgx_tm_get_wrk_phc(_time)                           \
	edgx_tm_get_role_phc(_time, EDGX_TM_ROLE_WRK)
#define edgx_tm_get_ts_phc(_time)                            \
	edgx_tm_get_role_phc(_time, EDGX_TM_ROLE_TS)

int edgx_tm_get_role_time(const struct edgx_time *time,
			  enum edgx_clk_role role, struct timespec64 *ts);
#define edgx_tm_get_wrk_time(_time, _ts)                     \
	edgx_tm_get_role_time(_time, EDGX_TM_ROLE_WRK, _ts)

#define edgx_tm_get_ts_time(_time, _ts)                      \
	edgx_tm_get_role_time(_time, EDGX_TM_ROLE_TS, _ts)

#endif /* _EDGE_TIME_H */
