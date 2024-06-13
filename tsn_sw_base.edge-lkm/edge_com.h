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

#ifndef _EDGE_COM_H
#define _EDGE_COM_H

#include "edge_bridge.h"
#include "edge_port.h"

struct edgx_com;
struct edgx_com_hdl;

#define COM_FLAG_NONE         ((ptflags_t)0x0)
#define COM_FLAG_MACSEC       ((ptflags_t)0x80)
#define COM_FLAG_SMD_R        ((ptflags_t)0x60)
#define COM_FLAG_SMD_V        ((ptflags_t)0x40)
#define COM_FLAG_SMD_ES       ((ptflags_t)0x20)
#define COM_FLAG_IPOMARK      ((ptflags_t)0x10)

#define COM_FLAG_PREEMPT_MASK  (COM_FLAG_SMD_R  |         \
				COM_FLAG_SMD_V  |         \
				COM_FLAG_SMD_ES)

#define COM_FLAG_MASK          (COM_FLAG_MACSEC       |   \
				COM_FLAG_PREEMPT_MASK |   \
				COM_FLAG_IPOMARK)

int edgx_com_probe(struct edgx_br *br, const char *ifname,
		   struct edgx_com **com, edgx_io_t *mngmt_base);
void edgx_com_shutdown(struct edgx_com *com);

struct edgx_com_hdl *edgx_com_reg_pt(struct edgx_com *com, struct edgx_pt *pt);
void                 edgx_com_unreg_pt(struct edgx_com_hdl *ch);

bool            edgx_com_is_mgmt_pt(struct edgx_com *com, ptid_t ptid);
ptid_t          edgx_com_get_mgmt_ptid(struct edgx_com *com);
struct edgx_pt *edgx_com_get_mgmt_pt(struct edgx_com_hdl *hcom);

netdev_tx_t edgx_com_xmit(struct edgx_com_hdl *hcom, struct sk_buff *skb,
			  ptflags_t flags);

int edgx_com_hwts_set(struct edgx_com_hdl *hcom, struct ifreq *ifr);
int edgx_com_hwts_get(struct edgx_com_hdl *hcom, struct ifreq *ifr);
void edgx_com_txts_dispatch(struct sk_buff *skb,
			    struct skb_shared_hwtstamps *hwts);
void edgx_com_tx_timeout(struct edgx_com_hdl *hcom, struct net_device *netdev);
bool edgx_multiqueue_support_get(struct edgx_com *com,
				 u8 *num_tx_queues, u8 *num_rx_queues);
irqreturn_t edgx_com_ts_tx_isr(int irq, void *device);
irqreturn_t edgx_com_dma_tx_isr(int irq, void *device);
irqreturn_t edgx_com_dma_rx_isr(int irq, void *device);
irqreturn_t edgx_com_dma_err_isr(int irq, void *device);

#endif /* _EDGE_COM_H */
