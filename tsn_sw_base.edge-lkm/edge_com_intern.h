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

#ifndef _EDGE_COM_INTERN_H
#define _EDGE_COM_INTERN_H

#include "edge_com.h"
#include "edge_com_ts.h"
#include "edge_defines.h"

/* Communication flags */
#define COM_TRAILER_LEN   (2)
#define COM_FLAGS_IDX     (0)
#define COM_PORT_IDX      (1)

struct edgx_com_ops {
	void (*shutdown)(struct edgx_com *com);
	netdev_tx_t (*xmit)(struct edgx_com *com, struct sk_buff *skb,
			    ptcom_t ptcom, ptflags_t flags);
	int  (*hwts_set)(struct edgx_com *com, ptcom_t ptcom,
			 struct ifreq *ifr);
	int  (*hwts_get)(struct edgx_com *com, ptcom_t ptcom,
			 struct ifreq *ifr);
	void (*tx_timeout)(struct edgx_com *com, struct net_device *netdev);
	bool (*multiqueue_support)(struct edgx_com *com, u8 *num_tx_queues,
				   u8 *num_rx_queues);
	irqreturn_t (*ts_tx_isr)(struct edgx_com *com);
	irqreturn_t (*dma_tx_isr)(struct edgx_com *com);
	irqreturn_t (*dma_rx_isr)(struct edgx_com *com);
	irqreturn_t (*dma_err_isr)(struct edgx_com *com);
};

struct edgx_com {
	struct edgx_br		*parent;
	pid_t			 mgmt_ptid;
	struct edgx_com_ops	*ops;
	struct edgx_com_ts	 ts;
};

void edgx_com_rx_dispatch(struct edgx_com *com, struct sk_buff *skb,
			  ptcom_t ptcom, ptflags_t flags);

int edgx_com_dma_probe(struct edgx_br *br,
		       edgx_io_t *mngmt_base, struct edgx_com **com);
int edgx_com_xmii_probe(struct edgx_br *br, const char *ifname,
			edgx_io_t *mngmt_base, struct edgx_com **com);
int edgx_com_init(struct edgx_com *com, struct edgx_br *br,
		  struct edgx_com_ops *ops, pid_t ptid, edgx_io_t *mngmt_base);
void edgx_com_release(struct edgx_com *com);

#endif /* _EDGE_COM_INTERN_H */
