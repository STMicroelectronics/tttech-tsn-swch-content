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

#ifndef _EDGE_FQTSS_H
#define _EDGE_FQTSS_H

#include "edge_bridge.h"
#include "edge_st.h"

struct edgx_fqtss;

int edgx_probe_fqtss(struct edgx_pt *pt,
		     edgx_io_t *iobase, struct edgx_fqtss **pfqtss);

void edgx_shutdown_fqtss(struct edgx_fqtss *fqtss);

void edgx_fqtss_st_change(struct edgx_fqtss *fqtss, struct edgx_st *st);

#endif /* _EDGE_FQTSS_H */
