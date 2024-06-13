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

#ifndef _EDGE_BR_SID_H
#define _EDGE_BR_SID_H

#include "edge_bridge.h"

struct edgx_sid_br;
enum cnt_type {
	CNT_OUT = 0,
	CNT_IN
};

int edgx_probe_sid(struct edgx_br *br, edgx_io_t *br_iobase,
		   struct edgx_sid_br **br_sid);
void edgx_shutdown_sid(struct edgx_sid_br *sid);
u32 edgx_sid_get_cnt(struct edgx_sid_br *sid, u16 str_hdl, u8 port,
		     enum cnt_type type);

#endif /*_EDGE_BR_SID_H */
