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

#ifndef _EDGE_COM_TS_H
#define _EDGE_COM_TS_H

#include <linux/kfifo.h>
#include <linux/net_tstamp.h>
#include <linux/netdevice.h>
#include "edge_defines.h"

#define EDGX_COM_TS_CTRL_CNT	(4U)
#define EDGX_COM_TS_KFIFO_LEN	((2U) * EDGX_COM_TS_CTRL_CNT)

struct edgx_com_pts {
	struct hwtstamp_config	 hwts_cfg;
	edgx_io_t		*iobase;
	u8			 ptid;
	DECLARE_KFIFO(tx_queue, struct sk_buff*, EDGX_COM_TS_KFIFO_LEN);
	u8			 tx_ts_pos;
	spinlock_t		 lock;	    /* Sync. access to tx_queue */
	spinlock_t		*ts_lock;   /* Sync. access to shared IP reg. */
};

struct edgx_com_ts {
	struct edgx_br		*parent;
	struct edgx_com_pts	*pts[EDGX_BR_MAX_PORTS];
	struct workqueue_struct	*wq_tx;
	struct work_struct	 work_tx;
	spinlock_t		 ts_lock; /* Sync. access to shared IP reg. */
};

int edgx_com_ts_init(struct edgx_com_ts *ts, edgx_io_t *mngmnt_base,
		     struct edgx_br *br);
void edgx_com_ts_shutdown(struct edgx_com_ts *ts);
int edgx_com_ts_cfg_get(struct edgx_com_ts *ts, ptcom_t ptcom,
			struct ifreq *ifr);
int edgx_com_ts_cfg_set(struct edgx_com_ts *ts, ptcom_t ptcom,
			struct ifreq *ifr);
struct sk_buff *edgx_com_ts_xmit(struct edgx_com_ts *ts,
				 struct sk_buff *skb, ptcom_t ptcom);
void edgx_com_ts_rx(struct edgx_com_ts *ts,
		    struct sk_buff *skb, ptcom_t ptcom);
irqreturn_t edgx_com_ts_isr(struct edgx_com_ts *ts);

#endif /* _EDGE_COM_TS_H */
