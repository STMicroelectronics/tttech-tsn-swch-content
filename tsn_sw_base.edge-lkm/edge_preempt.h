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

#ifndef _EDGE_PREEMPT_H
#define _EDGE_PREEMPT_H

#include "edge_bridge.h"

struct edgx_preempt;

int edgx_probe_preempt(struct edgx_pt *pt, struct edgx_com_hdl *hcom,
		       edgx_io_t *iobase, struct edgx_preempt **ppreempt);
void edgx_shutdown_preempt(struct edgx_preempt *preempt);

void edgx_preempt_rcv(struct edgx_preempt *preempt, struct sk_buff *skb,
		      ptflags_t flags);
void edgx_preempt_link_chng(struct edgx_pt *pt, int link, int speed);

#endif /* _EDGE_PREEMPT_H */
