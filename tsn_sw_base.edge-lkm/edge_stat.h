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

#ifndef _EDGE_STAT_H
#define _EDGE_STAT_H

#include "edge_bridge.h"

struct edgx_stat;
struct edgx_stat_hdl;

enum edgx_stat_feat_id {
	EDGX_STAT_FEAT_MIN     = 8,
	EDGX_STAT_FEAT_PORT    = 8,
	EDGX_STAT_FEAT_HSRPRP  = 9,
	EDGX_STAT_FEAT_PREEMPT = 10,
	EDGX_STAT_FEAT_MACSEC  = 11,
	EDGX_STAT_FEAT_ST      = 12,
	EDGX_STAT_FEAT_MAX,
};

struct edgx_statinfo {
	enum edgx_stat_feat_id feat_id;
	/* Rate should be about 1/2 the time the counters need to saturate */
	unsigned int           rate_ms;
	size_t                 base;    /* base offset in port counter area */
	size_t                 nwords;  /* number of 32bit words */
};

typedef u64 statw_t;

int  edgx_probe_stat(const struct edgx_br *br, struct edgx_stat **pstat,
		     ptvec_t *map);
void edgx_shutdown_stat(struct edgx_stat *stat);

struct edgx_stat_hdl *edgx_stat_alloc_hdl(struct edgx_stat *sm, ptid_t ptid,
					  const struct edgx_statinfo *info);
void                  edgx_stat_free_hdl(struct edgx_stat_hdl *sh);

int                   edgx_stat_update(struct edgx_stat_hdl *sh);

statw_t               edgx_stat_get(struct edgx_stat_hdl *sh, size_t idx);

static inline statw_t edgx_stat_upget(struct edgx_stat_hdl *sh, size_t idx)
{
	edgx_stat_update(sh);
	return edgx_stat_get(sh, idx);
}

#endif /* _EDGE_STAT_H */
