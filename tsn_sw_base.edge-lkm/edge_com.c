// SPDX-License-Identifier: GPL-2.0
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

#include <linux/netdevice.h>
#include <linux/rtnetlink.h>

#include "edge_com.h"
#include "edge_com_intern.h"
#include "edge_link.h"

struct edgx_com_hdl {
	struct edgx_com  *com;

	ptcom_t           trailer;
};

netdev_tx_t edgx_com_xmit(struct edgx_com_hdl *ch, struct sk_buff *skb,
			  ptflags_t flags)
{
	struct edgx_com *com = ch->com;

	return com->ops->xmit(com, skb, ch->trailer, flags);
}

void edgx_com_txts_dispatch(struct sk_buff *skb,
			    struct skb_shared_hwtstamps *hwts)
{
	struct edgx_link *lnk = edgx_net2link(skb->dev);
	ktime_t dly = edgx_link_get_tx_delay_min(lnk);

	hwts->hwtstamp = ktime_add(hwts->hwtstamp, dly);
	skb_tstamp_tx(skb, hwts);
	dev_consume_skb_any(skb);
}

/* TODO: This function should take one 16-bit trailer and split it up itself.
 *  DMA and xMII behave the same way and when we switch to 9+ port switches we
 *  only need to update the trailer disassembly into flags and ptcom here.
 */
void edgx_com_rx_dispatch(struct edgx_com *com, struct sk_buff *skb,
			  ptcom_t ptcom, ptflags_t flags)
{
	struct edgx_pt *pt = NULL;
	struct skb_shared_hwtstamps *hwts;

	/* clear reserved bits from flags first */
	flags &= COM_FLAG_MASK;

	/* Only IPO rules capturing frames destined to bridge ports, i.e.,
	 * reserved or individual DMAC, set the IPO-mark.
	 * Everything else is for the end station.
	 */
	if (flags & COM_FLAG_IPOMARK)
		pt = edgx_br_get_brpt(com->parent, ffs(ptcom) - 1);
	else
		pt = edgx_br_get_eppt(com->parent);

	if (!pt) {
		edgx_err("%s: reception on unknown port\n", __func__);
		dev_kfree_skb_any(skb);
		return;
	}
	/* Clear IPO-mark, so that it doesn't disturb dispatching in port */
	flags &= ~COM_FLAG_IPOMARK;

	hwts = skb_hwtstamps(skb);
	if (ktime_to_ns(hwts->hwtstamp)) {
		struct edgx_link *lnk = edgx_pt_get_link(pt);
		ktime_t dly = edgx_link_get_rx_delay_min(lnk);

		hwts->hwtstamp = ktime_sub(hwts->hwtstamp, dly);
	}

	edgx_pt_rcv(pt, skb, flags);
}

int edgx_com_init(struct edgx_com *com, struct edgx_br *br,
		  struct edgx_com_ops *ops, pid_t ptid, edgx_io_t *mngmt_base)
{
	int ret;

	com->parent    = br;
	com->mgmt_ptid = ptid;
	com->ops       = ops;

	ret = edgx_com_ts_init(&com->ts, mngmt_base, br);
	if (ret)
		edgx_err("Cannot initialize timestamping! ret=%d\n",
			 ret);

	return ret;
}

void edgx_com_release(struct edgx_com *com)
{
	edgx_com_ts_shutdown(&com->ts);
}

int edgx_com_probe(struct edgx_br *br, const char *ifname,
		   struct edgx_com **com, edgx_io_t *mngmt_base)
{
	int ret;

	ret = edgx_com_dma_probe(br, mngmt_base, com);
	if (ret == -ENODEV)
		ret = edgx_com_xmii_probe(br, ifname, mngmt_base, com);

	if (ret)
		edgx_br_err(br, "Cannot initialize communication subsystem\n");

	return ret;
}

void edgx_com_shutdown(struct edgx_com *com)
{
	if (com)
		com->ops->shutdown(com);
}

void edgx_com_unreg_pt(struct edgx_com_hdl *ch)
{
	if (!ch)
		return;
	kfree(ch);
}

struct edgx_com_hdl *edgx_com_reg_pt(struct edgx_com *com,
				     struct edgx_pt *pt)
{
	struct edgx_com_hdl *ch;
	ptid_t               ptid;

	if (!com || !pt)
		return NULL;
	ch = kzalloc(sizeof(*ch), GFP_KERNEL);
	if (!ch)
		return NULL;

	ptid        = edgx_pt_get_id(pt);
	ch->com     = com;
	ch->trailer = (PT_IS_EP_ID(ptid)) ? 0 : BIT(ptid);

	return ch;
}

bool edgx_com_is_mgmt_pt(struct edgx_com *com, ptid_t ptid)
{
	return (com->mgmt_ptid == ptid);
}

ptid_t edgx_com_get_mgmt_ptid(struct edgx_com *com)
{
	return (com) ? com->mgmt_ptid : PT_INV_ID;
}

struct edgx_pt *edgx_com_get_mgmt_pt(struct edgx_com_hdl *hcom)
{
	struct edgx_com *com;

	if (!hcom)
		return NULL;

	com = hcom->com;
	return edgx_br_get_brpt(com->parent, com->mgmt_ptid);
}

int edgx_com_hwts_set(struct edgx_com_hdl *hcom, struct ifreq *ifr)
{
	struct edgx_com *com = hcom->com;

	return edgx_com_ts_cfg_set(&com->ts, hcom->trailer, ifr);
}

int edgx_com_hwts_get(struct edgx_com_hdl *hcom, struct ifreq *ifr)
{
	struct edgx_com *com = hcom->com;

	return edgx_com_ts_cfg_get(&com->ts, hcom->trailer, ifr);
}

void edgx_com_tx_timeout(struct edgx_com_hdl *hcom, struct net_device *netdev)
{
	struct edgx_com *com = hcom->com;

	if (com->ops->tx_timeout)
		com->ops->tx_timeout(com, netdev);
}

bool edgx_multiqueue_support_get(struct edgx_com *com,
				 u8 *num_tx_queues, u8 *num_rx_queues)
{
	return com->ops->multiqueue_support(com, num_tx_queues, num_rx_queues);
}

irqreturn_t edgx_com_ts_tx_isr(int irq, void *device)
{
	struct edgx_com *com = (struct edgx_com *)device;

	return edgx_com_ts_isr(&com->ts);
}

irqreturn_t edgx_com_dma_tx_isr(int irq, void *device)
{
	struct edgx_com *com = (struct edgx_com *)device;

	if (com->ops->dma_tx_isr)
		return com->ops->dma_tx_isr(com);

	return IRQ_NONE;
}

irqreturn_t edgx_com_dma_rx_isr(int irq, void *device)
{
	struct edgx_com *com = (struct edgx_com *)device;

	if (com->ops->dma_rx_isr)
		return com->ops->dma_rx_isr(com);

	return IRQ_NONE;
}

irqreturn_t edgx_com_dma_err_isr(int irq, void *device)
{
	struct edgx_com *com = (struct edgx_com *)device;

	if (com->ops->dma_err_isr)
		return com->ops->dma_err_isr(com);

	return IRQ_NONE;
}
