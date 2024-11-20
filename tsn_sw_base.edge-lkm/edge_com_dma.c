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
#include <linux/dma-mapping.h>
#include <linux/dma-direction.h>
#include <linux/kfifo.h>
#include <linux/interrupt.h>
#include <linux/version.h>

#include "edge_ac.h"
#include "edge_com_intern.h"
#include "edge_util.h"
#include "edge_com_ts.h"

#define EDGX_COM_DMA_DUMP
/* #define EDGX_COM_DMA_DBG */
/* #define EDGX_COM_DMA_DBG_SHORT */
#define EDGX_DMA_EDGE_TRIG_WORKAROUND

#define EDGX_DMA_TX_QUEUE_CNT	(8U)	/* Max number of supported TX queues */
#define EDGX_DMA_RX_QUEUE_CNT	(1U)	/* No support for multi-q RX */

#define DMA_ITF_OFFS			(0xA000)
#define EDGX_DMA_CFG			(0x0)
#define EDGX_DMA_DESC_P(idx)		(0x020 + ((idx) * 2))
#define EDGX_DMA_RING_CFG0(idx)		(0x100 + ((idx) * 8))
#define EDGX_DMA_RING_CFG1(idx)		(EDGX_DMA_RING_CFG0(idx) + 2)
#define EDGX_DMA_TX_IDX(idx)		(idx)
#define EDGX_DMA_RX_IDX(idx)		((idx) + EDGX_DMA_TX_QUEUE_CNT)

#if defined(EDGX_COM_DMA_DBG)
#define EDGX_DMA_DESC_CNT		(16U)
#define EDGX_DMA_DESC_CNT_MASK		(3U)	/* 16 descriptors */
#else
#define EDGX_DMA_DESC_CNT		(256U)
#define EDGX_DMA_DESC_CNT_MASK		(7U)	/* 256 descriptors */
#endif
#define EDGX_DMA_MAX_LEN		(0x7ff)
#define EDGX_DMA_TX_WEIGHT		(64)
#define EDGX_DMA_RX_WEIGHT		(64)

struct edgx_com_dma_desc {
	u16 cfg;
	u16 dummy_1;
	u16 mgmt;
	u16 dummy_2;
	u16 p_data_l;
	u16 p_data_h;
	u16 dummy_3;
	u16 dummy_4;
	u16 ts_ns_l;
	u16 ts_ns_h;
	u16 ts_s_l;
	u16 dummy_5;
	u16 dummy_6;
	u16 dummy_7;
	u16 dummy_8;
	u16 dummy_9;
} __packed;

#define edgx_dma_get_len(pdesc) (le16_to_cpu((pdesc)->cfg) & 0x7ff)
#define edgx_dma_get_lerr(pdesc) (le16_to_cpu((pdesc)->cfg) & BIT(13))
#define edgx_dma_get_err(pdesc) (le16_to_cpu((pdesc)->cfg) & BIT(14))
#define edgx_dma_get_init(pdesc) (le16_to_cpu((pdesc)->cfg) & BIT(15))
#define edgx_dma_get_port(pdesc) (le16_to_cpu((pdesc)->mgmt) & 0xfff)
#define edgx_dma_get_flags(pdesc) (le16_to_cpu((pdesc)->mgmt) >> 8)

static inline u32 edgx_dma_get_pdata(struct edgx_com_dma_desc *pdesc)
{
	return (u32)le16_to_cpu(pdesc->p_data_l) |
	       ((u32)le16_to_cpu(pdesc->p_data_h) << 16);
}

static inline void edgx_dma_clr_lerr(struct edgx_com_dma_desc *pdesc)
{
	pdesc->cfg = __cpu_to_le16(le16_to_cpu(pdesc->cfg) & (~BIT(13)));
}

static inline void edgx_dma_clr_err(struct edgx_com_dma_desc *pdesc)
{
	pdesc->cfg = __cpu_to_le16(le16_to_cpu(pdesc->cfg) & (~BIT(14)));
}

static inline void edgx_dma_set_port(struct edgx_com_dma_desc *pdesc,
				     u16 port_mask, u16 flags)
{
	pdesc->mgmt = __cpu_to_le16((flags << 8) | (port_mask & 0xfff));
}

static inline void edgx_dma_set_init(struct edgx_com_dma_desc *pdesc)
{
	pdesc->cfg = __cpu_to_le16(le16_to_cpu(pdesc->cfg) | BIT(15));
}

static inline void edgx_dma_set_len(struct edgx_com_dma_desc *pdesc,
				    unsigned int len)
{
	pdesc->cfg = __cpu_to_le16((le16_to_cpu(pdesc->cfg) & (~0x7ff)) |
		     (len & 0x7ff));
}

static inline void edgx_dma_set_pdata(struct edgx_com_dma_desc *pdesc,
				      u32 pdata)
{
	pdesc->p_data_l = __cpu_to_le16(pdata & 0xffff);
	pdesc->p_data_h = __cpu_to_le16(pdata >> 16);
}

struct edgx_dma_queue {
	spinlock_t		  lock;		/* Sync. access to dma queue */
	int			  idx;
	enum dma_data_direction   dir;
	edgx_io_t		 *cfg_base;
	struct edgx_com_dma_desc *desc;
	dma_addr_t		  desc_bus_addr;
	int			  enq_pos;
	int			  deq_pos;
	int			  cnt;
	struct sk_buff		**skb;
	DECLARE_KFIFO(pending_devs, struct net_device *, EDGX_BR_MAX_PORTS);
};

struct edgx_com_dma {
	struct edgx_com		 com;
	edgx_io_t		*dma_base;
	struct edgx_dma_queue	 tx_queue[EDGX_DMA_TX_QUEUE_CNT];
	u8			 txq_to_prio[EDGX_DMA_TX_QUEUE_CNT];
	struct edgx_dma_queue	 rx_queue[EDGX_DMA_RX_QUEUE_CNT];
	u8                       rxq_to_prio[EDGX_DMA_RX_QUEUE_CNT];
	u8			 real_num_tx_queues;
	u8			 real_num_rx_queues;
	u8			 q_idx_brpt;
	struct net_device	 napi_dev;
	struct napi_struct	 napi_rx;
};

#define edgx_com_to_dma(pcom)   container_of(pcom, struct edgx_com_dma, com)
#define edgx_napi_to_dma_rx(pnapi) container_of(pnapi, struct edgx_com_dma, \
						napi_rx)

static void edgx_dma_dump(struct edgx_dma_queue *q, int i, const char *str)
{
#if (0)
	for (i = 0; i < EDGX_DMA_DESC_CNT; i++)
#endif
		if (q->skb[i]) {
			u8 *d = (u8 *)q->skb[i]->data;

#if defined(EDGX_COM_DMA_DBG_SHORT)
			edgx_dbg("%s Q%.2d D%.2d: cfg=0x%.4x mgt=0x%.4x dL=0x%.4x dH=0x%.4x %pM %pM %*phN %*phN\n",
				 str,
				 q->idx, i, q->desc[i].cfg, q->desc[i].mgmt,
				 q->desc[i].p_data_h, q->desc[i].p_data_l,
				 &d[0], &d[6], 4, &d[12], 4, &d[16]);
#else
			edgx_dbg("%s Q%.2d D%.2d: l=%d ler=%d er=%d i=%d p=%d f=0x%x %pM %pM %*phN %*phN\n",
				 str, q->idx, i,
				 edgx_dma_get_len(&q->desc[i]),
				 edgx_dma_get_lerr(&q->desc[i]) ? 1 : 0,
				 edgx_dma_get_err(&q->desc[i]) ? 1 : 0,
				 edgx_dma_get_init(&q->desc[i]) ? 1 : 0,
				 edgx_dma_get_port(&q->desc[i]),
				 edgx_dma_get_flags(&q->desc[i]),
				 &d[0], &d[6], 4, &d[12], 4, &d[16]);
#endif
		} else {
#if defined(EDGX_COM_DMA_DBG_SHORT)
			edgx_dbg("D%.2d: cfg=0x%.4x mgt=0x%.4x dL=0x%.4x dH=0x%.4x\n",
				 i, q->desc[i].cfg, q->desc[i].mgmt,
				 q->desc[i].p_data_h, q->desc[i].p_data_l);
#else
			edgx_dbg("%s Q%.2d D%.2d: l=%d ler=%d er=%d i=%d p=%d f=0x%x\n",
				 str, q->idx, i,
				 edgx_dma_get_len(&q->desc[i]),
				 edgx_dma_get_lerr(&q->desc[i]) ? 1 : 0,
				 edgx_dma_get_err(&q->desc[i]) ? 1 : 0,
				 edgx_dma_get_init(&q->desc[i]) ? 1 : 0,
				 edgx_dma_get_port(&q->desc[i]),
				 edgx_dma_get_flags(&q->desc[i]));
#endif
		}
}

static int edgx_dma_queue_init(struct edgx_dma_queue *q, struct device *dev,
			       edgx_io_t *dma_base, int idx,
			       enum dma_data_direction dir)
{
	q->cfg_base = dma_base;
	q->idx = idx;
	q->dir = dir;
	spin_lock_init(&q->lock);

	// TODO: Somewhere set dev->tx_queue_len to EDGX_DMA_DESC_CNT -1??
	/* Allocate descriptors */
	q->desc = dma_alloc_coherent(dev,
				     EDGX_DMA_DESC_CNT *
				     sizeof(struct edgx_com_dma_desc),
				     &q->desc_bus_addr, GFP_KERNEL);

	if (!q->desc)
		return -ENOMEM;

	q->skb = kcalloc(EDGX_DMA_DESC_CNT, sizeof(*q->skb), GFP_KERNEL);

	if (!q->skb)
		goto out_skb;

	INIT_KFIFO(q->pending_devs);
	edgx_wr16(dma_base, EDGX_DMA_DESC_P(idx), EDGX_DMA_DESC_CNT - 1);
	edgx_wr16(dma_base, EDGX_DMA_RING_CFG0(idx),
		  EDGX_DMA_DESC_CNT_MASK | (q->desc_bus_addr & 0xffe0));
	edgx_wr16(dma_base, EDGX_DMA_RING_CFG1(idx), q->desc_bus_addr >> 16);

	// TODO We are leaking some kernel memory layout by using %px here
	edgx_dbg("DMA: queue_init: desc=%px, bus_addr=0x%pad\n",
		 q->desc, &q->desc_bus_addr);
	return 0;

out_skb:
	dma_free_coherent(dev, EDGX_DMA_DESC_CNT * sizeof(*q->desc),
			  q->desc, q->desc_bus_addr);
	return -ENOMEM;
}

static void edgx_dma_queue_release(struct edgx_dma_queue *q, struct device *dev)
{
	int i;

	// TODO: make sure that all pending requests are processed first

	q->cnt = 0;
	for (i = 0; i < EDGX_DMA_DESC_CNT; i++) {
		if (!q->skb[i])
			continue;

		if (q->dir == DMA_FROM_DEVICE)
			dma_unmap_single(dev, edgx_dma_get_pdata(&q->desc[i]),
					 EDGX_DMA_MAX_LEN, q->dir);
		else
			dma_unmap_single(dev, edgx_dma_get_pdata(&q->desc[i]),
					 q->skb[i]->len, q->dir);

		dev_kfree_skb_any(q->skb[i]);
	}

	kfree(q->skb);
	dma_free_coherent(dev, EDGX_DMA_DESC_CNT * sizeof(*q->desc),
			  q->desc, q->desc_bus_addr);
}

static int edgx_dma_enq(struct edgx_dma_queue *q, struct device *dev,
			struct sk_buff *skb, ptcom_t port_mask,
			ptflags_t flags, struct edgx_com_ts *ts)
{
	int ret = -ENOMEM;
	struct edgx_com_dma_desc *desc;
	dma_addr_t phys_addr;
	int old_pos;
	unsigned int len = (q->dir == DMA_TO_DEVICE) ?
			   skb->len : EDGX_DMA_MAX_LEN;

	 /* Due to an IP limitation, keep one empty descriptor in the queue. */
	if (q->cnt < (EDGX_DMA_DESC_CNT - 1)) {
		desc = &q->desc[q->enq_pos];

		if (edgx_dma_get_init(desc)) {
			edgx_err("COM: DMA queuing error!\n");
			return -ENOMEM;
		}

		if (ts)
			skb = edgx_com_ts_xmit(ts, skb, port_mask);

		phys_addr = dma_map_single(dev, skb->data, len, q->dir);

		if (dma_mapping_error(dev, phys_addr)) {
			edgx_err("COM: DMA mapping error!\n");
			return -ENOMEM;
		}

		edgx_dbg("DMA-enq: Q%d, pos=%d, cnt=%d, len=%d, port_mask=0x%x, flags=0x%x\n",
			 q->idx, q->enq_pos, q->cnt, len, port_mask, flags);

		q->skb[q->enq_pos] = skb;
		old_pos = q->enq_pos;
		q->enq_pos = (q->enq_pos + 1) % EDGX_DMA_DESC_CNT;
		q->cnt++;

		dma_rmb();
		edgx_dma_set_len(desc, len);
		edgx_dma_set_port(desc, port_mask, flags);
		edgx_dma_set_pdata(desc, phys_addr);
		dma_wmb();
		edgx_dma_set_init(desc);
		edgx_dma_dump(q, old_pos, "EN:");
		edgx_wr16(q->cfg_base, EDGX_DMA_DESC_P(q->idx), (u16)old_pos);

		/* Return ok only if there is still place in the DMA ring. */
		if (q->cnt < (EDGX_DMA_DESC_CNT - 1))
			ret = 0;
		else
			ret = -ENOSPC;
	} else {
		edgx_dbg("DMA-enq: Q%d, queue full. enq_pos=%d, cnt=%d, len=%d\n",
			 q->idx, q->enq_pos, q->cnt, len);
	}

	return ret;
}

static struct sk_buff *edgx_dma_deq(struct edgx_dma_queue *q,
				    struct device *dev,
				    u64 *error_cnt, ptcom_t *port_mask,
				    ptflags_t *flags)
{
	struct edgx_com_dma_desc *desc;
	struct sk_buff *skb;

	desc = &q->desc[q->deq_pos];
	skb = q->skb[q->deq_pos];

	if (!q->cnt || edgx_dma_get_init(desc)) {
		return NULL;
	}

	dma_rmb();
	if (error_cnt) {
		if (edgx_dma_get_lerr(desc)) {
			edgx_err("COM: DMA length error!\n");
			(*error_cnt)++;
			edgx_dma_clr_lerr(desc);
		}

		if (edgx_dma_get_err(desc)) {
			edgx_err("COM: DMA bus error!\n");
			(*error_cnt)++;
			edgx_dma_clr_err(desc);
		}
	}

	if (q->dir == DMA_FROM_DEVICE) {
		skb_put(skb, edgx_dma_get_len(desc));
		*port_mask = edgx_dma_get_port(desc);
		*flags = edgx_dma_get_flags(desc);
		dma_unmap_single(dev, edgx_dma_get_pdata(desc),
				 EDGX_DMA_MAX_LEN, q->dir);
	} else {
		dma_unmap_single(dev, edgx_dma_get_pdata(desc),
				 skb->len, q->dir);
	}

	edgx_dbg("DMA-deq: Q%d, pos=%d, cnt=%d, pmask=0x%x, flags=0x%x, len=%d, skb_len=%d\n",
		 q->idx, q->deq_pos, q->cnt, edgx_dma_get_port(desc),
		 edgx_dma_get_flags(desc), edgx_dma_get_len(desc), skb->len);

	edgx_dma_dump(q, q->deq_pos, "DE:");

	q->skb[q->deq_pos] = NULL;
	q->deq_pos = (q->deq_pos + 1) % EDGX_DMA_DESC_CNT;
	q->cnt--;

	return skb;
}

static inline int edgx_dma_enq_tx(struct edgx_dma_queue *q,
				  struct device *dev, struct sk_buff *skb,
				  ptcom_t port_mask, ptflags_t flags,
				  struct edgx_com_ts *ts)
{
	int ret;
	unsigned int skb_len_dif;
	unsigned long irq_flags;

	if (skb->len < ETH_ZLEN) {
		skb_len_dif = ETH_ZLEN - skb->len;

		ret = skb_pad(skb, skb_len_dif);
		if (ret)
			return ret;

		memset(skb_put(skb, skb_len_dif), 0, skb_len_dif);
	}

	spin_lock_irqsave(&q->lock, irq_flags);
	ret = edgx_dma_enq(q, dev, skb, port_mask, flags, ts);
	if (ret) {
		edgx_dbg("DMA enq Tx: stop netdev %s\n", skb->dev->name);
		kfifo_put(&q->pending_devs, skb->dev);
		if (edgx_dev2ptid(&skb->dev->dev) == PT_EP_ID)
			netif_stop_subqueue(skb->dev,
					    skb_get_queue_mapping(skb));
		else
			netif_stop_queue(skb->dev);
	}
	spin_unlock_irqrestore(&q->lock, irq_flags);

	return ret;
}

static int edgx_dma_enq_rx(struct edgx_dma_queue *q, struct device *dev)
{
	int ret = -ENOMEM;
	unsigned long irq_flags;
	struct sk_buff *skb = dev_alloc_skb(EDGX_DMA_MAX_LEN);

	if (!skb)
		return -ENOMEM;

	spin_lock_irqsave(&q->lock, irq_flags);
	ret = edgx_dma_enq(q, dev, skb, 0, 0, NULL);
	spin_unlock_irqrestore(&q->lock, irq_flags);

	if (ret == -ENOMEM)
		dev_kfree_skb_any(skb);

	return ret;
}

static inline struct sk_buff *edgx_dma_deq_tx(struct edgx_dma_queue *q,
					      struct device *dev,
					      u64 *error_cnt)
{
	struct net_device *pend_dev;
	struct sk_buff *skb;
	unsigned long irq_flags;

	spin_lock_irqsave(&q->lock, irq_flags);
	skb = edgx_dma_deq(q, dev, error_cnt, NULL, NULL);

	if (skb && kfifo_get(&q->pending_devs, &pend_dev)) {
		edgx_dbg("DMA deq Tx: wake netdev %s\n", pend_dev->name);
		if (edgx_dev2ptid(&pend_dev->dev) == PT_EP_ID)
			netif_wake_subqueue(pend_dev,
					    skb_get_queue_mapping(skb));
		else
			netif_wake_queue(pend_dev);
	}
	spin_unlock_irqrestore(&q->lock, irq_flags);

	return skb;
}

static inline struct sk_buff *edgx_dma_deq_rx(struct edgx_dma_queue *q,
					      struct device *dev,
					      u64 *error_cnt,
					      ptcom_t *port_mask,
					      ptflags_t *flags)
{
	struct sk_buff *skb;
	unsigned long irq_flags;

	spin_lock_irqsave(&q->lock, irq_flags);
	skb = edgx_dma_deq(q, dev, error_cnt, port_mask, flags);
	spin_unlock_irqrestore(&q->lock, irq_flags);

	return skb;
}

static int edgx_dma_process_tx(struct edgx_com_dma *dma, int budget)
{
	struct sk_buff *skb = NULL;
	int i, j;
	u64 err_cnt = 0;
	struct device *dev = edgx_br_get_dev(dma->com.parent);

	for (i = 0, j = 0;
	     (i < budget) && (j < dma->real_num_tx_queues); ++j) {
		do {
			//TODO Based on q prio?
			skb = edgx_dma_deq_tx(&dma->tx_queue[j], dev, &err_cnt);

			if (skb) {
				/* TODO: Add DMA error counters? */
				dev_consume_skb_any(skb);
				++i;
			}
		} while (skb && (i < budget));
		edgx_dbg("DMA process Tx: q:%d budget:%d\n", j, i);
	}
	return i;
}

static int edgx_dma_process_rx(struct edgx_com_dma *dma, int budget)
{
	int ret, i;
	ptcom_t ptcom;
	ptflags_t flags;
	u64 err_cnt = 0;
	struct sk_buff *skb;
	struct device *dev = edgx_br_get_dev(dma->com.parent);

	for (i = 0; i < budget; i++) {
		skb = edgx_dma_deq_rx(&dma->rx_queue[0], dev, &err_cnt,
				      &ptcom, &flags);

		if (!skb)
			break;

		/* TODO: Add DMA error counters? */
		skb_reset_mac_header(skb);
		edgx_com_ts_rx(&dma->com.ts, skb, ptcom);
		edgx_com_rx_dispatch(&dma->com, skb, ptcom, flags);

		ret = edgx_dma_enq_rx(&dma->rx_queue[0], dev);

		if (ret && (ret != -ENOSPC))
			edgx_err("COM DMA: RX enqueue error!\n");
	}
	return i;
}

static netdev_tx_t edgx_com_dma_xmit(struct edgx_com *com, struct sk_buff *skb,
				     ptcom_t ptcom, ptflags_t flags)
{
	int  ret;
	unsigned int q_idx = skb_get_queue_mapping(skb);
	struct device *dev = edgx_br_get_dev(com->parent);
	struct edgx_com_dma *dma = edgx_com_to_dma(com);

	if (edgx_dev2ptid(&skb->dev->dev) != PT_EP_ID) {
		q_idx = dma->q_idx_brpt;
		skb_set_queue_mapping(skb, q_idx);
		edgx_dbg("skb on brport enqueued on q:%d for:%s ID:%d\n",
			 skb->queue_mapping, skb->dev->name,
			 edgx_dev2ptid(&skb->dev->dev));
	}

	edgx_dbg("xmiting on q:%d for:%s\n", q_idx, skb->dev->name);

	ret = edgx_dma_enq_tx(&dma->tx_queue[q_idx], dev, skb, ptcom, flags,
			      &com->ts);

	if (ret == -ENOSPC)
		edgx_dbg("COM DMA: xmit: TX queue (idx:%d) full. (%s)\n",
			 q_idx, skb->dev->name);

	if (ret == -ENOMEM) {
		edgx_dbg("COM DMA: xmit: TX queue (idx:%d) busy. (%s)\n",
			 q_idx, skb->dev->name);
		return NETDEV_TX_BUSY;
	}
	return NETDEV_TX_OK;
}

/*
 * TODO: Use a separate NAPI scheduler for TX and RX. For now keeping
 * a single scheduler because of problems with napi_schedule() experienced
 * with two NAPI schedulers in combination with MQPRIO.
 */
static int edgx_dma_poll_rx(struct napi_struct *napi, int budget)
{
	int used, used_rx, used_tx;
	int post_used_rx = 0;
	int post_used_tx = 0;
	u16 status;
	struct edgx_com_dma *dma = edgx_napi_to_dma_rx(napi);

	edgx_dbg("COM DMA: Poll.\n");

	used_tx = edgx_dma_process_tx(dma, budget);
	if (used_tx < budget) {
		edgx_br_clr_int_stat(dma->com.parent, BIT(EDGX_IRQ_NR_DMA_TX));
		/* TODO: Add a function instead to check if queues are empty. */
		post_used_tx = edgx_dma_process_tx(dma, 1);
	}

	used_rx = edgx_dma_process_rx(dma, budget);
	if (used_rx < budget) {
		edgx_br_clr_int_stat(dma->com.parent, BIT(EDGX_IRQ_NR_DMA_RX));
		/* TODO: Add a function instead to check if queues are empty. */
		post_used_rx = edgx_dma_process_rx(dma, 1);
	}

	if ((post_used_tx) || (post_used_rx))
		return budget;

	used = max(used_tx, used_rx);
	if (used >= budget)
		return budget;

#if defined(EDGX_DMA_EDGE_TRIG_WORKAROUND)
/* Clearing the status bit is unreliable in case if a new frame
 * arrives and the status bit is cleared at the same time. This leads to
 * interrupt loss and to stopped polling. Therefore we keep polling until
 * there are new frames in the DMA ring (the status bit is set) because we
 * do not know if they arrived before or after cleaning of the status bit.
 * Budget is reported even if less frames have been processed
 * to keep NAPI polling.
 */
	status = edgx_br_get_int_stat(dma->com.parent);
	if ((BIT(EDGX_IRQ_NR_DMA_TX) & status) ||
	    (BIT(EDGX_IRQ_NR_DMA_RX) & status)) {
		return budget;
	}
#endif

	if (likely(napi_complete_done(&dma->napi_rx, used))) {
		edgx_br_irq_enable(dma->com.parent, EDGX_IRQ_NR_DMA_RX);
		edgx_br_irq_enable(dma->com.parent, EDGX_IRQ_NR_DMA_TX);
		edgx_dbg("COM DMA: enable irqs, used=%d, budget=%d\n",
			 used, budget);
	}

	return used;
}

static irqreturn_t edgx_dma_tx_isr(struct edgx_com *com)
{
	struct edgx_com_dma *dma = edgx_com_to_dma(com);

	edgx_br_irq_disable(com->parent, EDGX_IRQ_NR_DMA_TX);

	edgx_dbg("COM DMA: TX ISR.\n");
	napi_schedule(&dma->napi_rx);

	return IRQ_HANDLED;
}

static irqreturn_t edgx_dma_rx_isr(struct edgx_com *com)
{
	struct edgx_com_dma *dma = edgx_com_to_dma(com);

	edgx_br_irq_disable(com->parent, EDGX_IRQ_NR_DMA_RX);

	edgx_dbg("COM DMA: RX ISR.\n");
	napi_schedule(&dma->napi_rx);

	return IRQ_HANDLED;
}

static irqreturn_t edgx_dma_err_isr(struct edgx_com *com)
{
	struct edgx_com_dma *dma = edgx_com_to_dma(com);

	edgx_dbg("DMA error IRQ received!\n");
	napi_schedule(&dma->napi_rx);
	edgx_br_clr_int_stat(com->parent, BIT(EDGX_IRQ_NR_DMA_ERR));
	return IRQ_HANDLED;
}

void edgx_com_dma_tx_timeout(struct edgx_com *com, struct net_device *netdev)
{
	struct edgx_com_dma *dma = edgx_com_to_dma(com);
	int i;

	edgx_warn("COM: DMA TX timeout! (%s)\n", netdev->name);
	edgx_dma_process_tx(dma, EDGX_DMA_TX_WEIGHT);
	// Actually this is the port netdev, so it could be either a ep or not

	// TODO Will this actually work?
	for (i = 0; i < netdev->real_num_tx_queues; ++i)
		if (__netif_subqueue_stopped(netdev, i))
			netif_wake_subqueue(netdev, i);//TODO Wake up all quues?
}

bool edgx_com_dma_multiqueue_support(struct edgx_com *com, u8 *num_tx_queues,
				     u8 *num_rx_queues)
{
	struct edgx_com_dma *dma = edgx_com_to_dma(com);

	if (num_tx_queues)
		*num_tx_queues = dma->real_num_tx_queues;
	if (num_rx_queues)
		*num_rx_queues = dma->real_num_rx_queues;

	return dma ? true : false;
}

#if defined(EDGX_COM_DMA_DUMP)
static ssize_t dma_dump_all_show(struct device *dev,
				 struct device_attribute *attr,
				 char *buf)
{
	ssize_t ret;
	int i;
	u16 intmask, intstat, desc_reg;
	struct edgx_br *br = edgx_dev2br(dev);
	struct edgx_com *com = edgx_br_get_com(br);
	struct edgx_com_dma *dma = edgx_com_to_dma(com);
	struct edgx_dma_queue *q = &dma->tx_queue[0];

	intmask = edgx_br_get_int_mask(br);
	intstat = edgx_br_get_int_stat(br);

	ret = scnprintf(buf, PAGE_SIZE, "IRQ STATUS: 0x%x\n", intstat);
	ret += scnprintf(buf + ret, PAGE_SIZE - ret, "IRQ MASK: 0x%x\n",
			 intmask);
	ret += scnprintf(buf + ret, PAGE_SIZE - ret, "Tx cnt: %d\n",
			dma->tx_queue[0].cnt);
	ret += scnprintf(buf + ret, PAGE_SIZE - ret, "Rx cnt: %d\n",
			dma->rx_queue[0].cnt);

	desc_reg = edgx_rd16(dma->tx_queue[0].cfg_base,
			     EDGX_DMA_DESC_P(dma->tx_queue[0].idx));
	ret += scnprintf(buf + ret, PAGE_SIZE - ret,
			 "Tx initialized=%d, used=%d\n",
			 desc_reg & 0xff, desc_reg >> 8);

	desc_reg = edgx_rd16(dma->rx_queue[0].cfg_base,
			     EDGX_DMA_DESC_P(dma->rx_queue[0].idx));
	ret += scnprintf(buf + ret, PAGE_SIZE - ret,
			 "Rx initialized=%d, used=%d\n",
			 desc_reg & 0xff, desc_reg >> 8);

	ret += scnprintf(buf + ret, PAGE_SIZE - ret, "DMA TX QUEUE:\n");
	for (i = 0; i < min(EDGX_DMA_DESC_CNT, 16U); i++)
		if (q->skb[i]) {
			u8 *d = (u8 *)q->skb[i]->data;

			ret += scnprintf(buf + ret, PAGE_SIZE - ret,
					"Q%.2d D%.2d: cfg=0x%.4x mgt=0x%.4x dL=0x%.4x dH=0x%.4x %pM %pM %*phN %*phN\n",
					q->idx, i, q->desc[i].cfg,
					q->desc[i].mgmt, q->desc[i].p_data_h,
					q->desc[i].p_data_l, &d[0], &d[6],
					4, &d[12], 4, &d[16]);
		}

	q = &dma->rx_queue[0];
	ret += scnprintf(buf + ret, PAGE_SIZE - ret, "DMA RX QUEUE:\n");
	for (i = 0; i < min(EDGX_DMA_DESC_CNT, 16U); i++)
		if (q->skb[i] && !edgx_dma_get_init(&q->desc[i])) {
			u8 *d = (u8 *)q->skb[i]->data;

			ret += scnprintf(buf + ret, PAGE_SIZE - ret,
					"Q%.2d D%.2d: cfg=0x%.4x mgt=0x%.4x dL=0x%.4x dH=0x%.4x %pM %pM %*phN %*phN\n",
					q->idx, i, q->desc[i].cfg,
					q->desc[i].mgmt, q->desc[i].p_data_h,
					q->desc[i].p_data_l, &d[0], &d[6],
					4, &d[12], 4, &d[16]);
		}
	return ret;
}

static ssize_t dma_status_show(struct device *dev,
			       struct device_attribute *attr,
			       char *buf)
{
	ssize_t ret;
	struct edgx_br *br = edgx_dev2br(dev);

	ret = scnprintf(buf, PAGE_SIZE, "0x%x\n", edgx_br_get_int_stat(br));
	return ret;
}

static ssize_t dma_status_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf,
				size_t count)
{
	u16 status;
	ssize_t ret = -EINVAL;
	struct edgx_br *br = edgx_dev2br(dev);

	if (!kstrtou16(buf, 0, &status)) {
		ret = count;
		edgx_br_clr_int_stat(br, status);
	}
	return ret;
}

static ssize_t dma_mask_set_show(struct device *dev,
				 struct device_attribute *attr,
				 char *buf)
{
	ssize_t ret;
	struct edgx_br *br = edgx_dev2br(dev);

	ret = scnprintf(buf, PAGE_SIZE, "0x%x\n", edgx_br_get_int_mask(br));
	return ret;
}

static ssize_t dma_mask_set_store(struct device *dev,
				  struct device_attribute *attr,
				  const char *buf,
				  size_t count)
{
	u16 mask;
	ssize_t ret = -EINVAL;
	struct edgx_br *br = edgx_dev2br(dev);

	if (!kstrtou16(buf, 0, &mask)) {
		ret = count;
		edgx_br_set_int_mask(br, mask);
	}
	return ret;
}

static ssize_t dma_mask_clr_show(struct device *dev,
				 struct device_attribute *attr,
				 char *buf)
{
	ssize_t ret;
	struct edgx_br *br = edgx_dev2br(dev);

	ret = scnprintf(buf, PAGE_SIZE, "0x%x\n", edgx_br_get_int_mask(br));
	return ret;
}

static ssize_t dma_mask_clr_store(struct device *dev,
				  struct device_attribute *attr,
				  const char *buf,
				  size_t count)
{
	u16 mask;
	ssize_t ret = -EINVAL;
	struct edgx_br *br = edgx_dev2br(dev);

	if (!kstrtou16(buf, 0, &mask)) {
		ret = count;
		edgx_br_clr_int_mask(br, mask);
	}
	return ret;
}

static ssize_t dma_flush_show(struct device *dev,
			      struct device_attribute *attr,
			      char *buf)
{
	ssize_t ret;
	int used;
	struct edgx_br *br = edgx_dev2br(dev);
	struct edgx_com *com = edgx_br_get_com(br);
	struct edgx_com_dma *dma = edgx_com_to_dma(com);

	used = edgx_dma_process_rx(dma, EDGX_DMA_DESC_CNT);

	ret = scnprintf(buf, PAGE_SIZE, "%d\n", used);
	return ret;
}

EDGX_DEV_ATTR_RO(dma_dump_all, "dump");
EDGX_DEV_ATTR_RW(dma_status, "status");
EDGX_DEV_ATTR_RW(dma_mask_set, "mask-set");
EDGX_DEV_ATTR_RW(dma_mask_clr, "mask-clr");
EDGX_DEV_ATTR_RO(dma_flush, "flush");

static struct attribute_group dma_group = {
	.name = "dma",
	.attrs = (struct attribute*[]){
		&dev_attr_dma_dump_all.attr,
		&dev_attr_dma_status.attr,
		&dev_attr_dma_mask_set.attr,
		&dev_attr_dma_mask_clr.attr,
		&dev_attr_dma_flush.attr,
		NULL
	},
};
#endif

static void edgx_com_dma_shutdown(struct edgx_com *com)
{
	int i;
	struct device *dev = edgx_br_get_dev(com->parent);
	struct edgx_com_dma *dma = edgx_com_to_dma(com);

	edgx_wr16(dma->dma_base, EDGX_DMA_CFG, 0);
	napi_disable(&dma->napi_rx);
	netif_napi_del(&dma->napi_rx);

	for (i = 0; i < dma->real_num_rx_queues; i++)
		edgx_dma_queue_release(&dma->rx_queue[i], dev);

	for (i = 0; i < dma->real_num_tx_queues; i++)
		edgx_dma_queue_release(&dma->tx_queue[i], dev);

	edgx_com_release(com);
	kfree(dma);
}

static struct edgx_com_ops edgx_com_dma_ops = {
	.shutdown	= edgx_com_dma_shutdown,
	.xmit		= edgx_com_dma_xmit,
	.tx_timeout	= edgx_com_dma_tx_timeout,
	.multiqueue_support = edgx_com_dma_multiqueue_support,
	.dma_tx_isr	= edgx_dma_tx_isr,
	.dma_rx_isr	= edgx_dma_rx_isr,
	.dma_err_isr	= edgx_dma_err_isr,
};

static int edgx_com_dma_init(struct edgx_com_dma *dma, struct edgx_br *br,
			     edgx_io_t *dma_base, edgx_io_t *mngmt_base,
			     ptid_t dma_pt)
{
	int i, ret;
	struct device *dev = edgx_br_get_dev(br);

	ret = edgx_com_init(&dma->com, br, &edgx_com_dma_ops, dma_pt,
			    mngmt_base);
	if (ret)
		goto out_err_init_com;

	dma->dma_base = dma_base;

	ret = init_dummy_netdev(&dma->napi_dev);
	if (ret) {
		edgx_err("COM DMA: cannot init dummy netdev! ret=%d\n", ret);
		goto out_err_init_dummy;
	}

	netif_napi_add_weight(&dma->napi_dev, &dma->napi_rx,
			      edgx_dma_poll_rx, EDGX_DMA_RX_WEIGHT);

	dma->real_num_tx_queues = edgx_br_get_generic(br,
						      BR_GX_DMA_TX_DESC_RINGS);

	/* Select 'middle' queue for tx brports, rounded towards the least preferred queues due to integer arithmetics.
	 * Note, that numerically lower DMA queues are expedited over numerically higher DMA queues.
	 */
	dma->q_idx_brpt = dma->real_num_tx_queues - ((dma->real_num_tx_queues - 1) >> 1) - 1;

	for (i = 0; !ret && (i < dma->real_num_tx_queues); i++)
		ret = edgx_dma_queue_init(&dma->tx_queue[i], dev,
					  dma->dma_base, EDGX_DMA_TX_IDX(i),
					  DMA_TO_DEVICE);

	/* For now only TX multiq is enabled, do the same here to enable RX */
	dma->real_num_rx_queues = EDGX_DMA_RX_QUEUE_CNT;

	for (i = 0; !ret && (i < dma->real_num_rx_queues); i++)
		ret = edgx_dma_queue_init(&dma->rx_queue[i], dev,
					  dma->dma_base, EDGX_DMA_RX_IDX(i),
					  DMA_FROM_DEVICE);

	for (i = 0; !ret && (i < dma->real_num_rx_queues); i++)
		while (!ret)
			ret = edgx_dma_enq_rx(&dma->rx_queue[i], dev);

	if (ret != -ENOSPC) {
		edgx_err("COM DMA: Queue initialization failed! ret=%d\n",
			 ret);
		goto out_err_init_queues;
	}

#if defined(EDGX_COM_DMA_DUMP)
	ret = edgx_br_sysfs_add(br, &dma_group);
	if (ret) {
		edgx_err("COM DMA: Cannot add sysfs group! ret=%d\n",
			 ret);
		goto out_err_init_queues;
	}
#endif

	napi_enable(&dma->napi_rx);
	edgx_wr16(dma->dma_base, EDGX_DMA_CFG, 0x03);
	return 0;

out_err_init_queues:
	for (i = 0; i < dma->real_num_rx_queues; i++)
		edgx_dma_queue_release(&dma->rx_queue[i], dev);

	for (i = 0; i < dma->real_num_tx_queues; i++)
		edgx_dma_queue_release(&dma->tx_queue[i], dev);

	netif_napi_del(&dma->napi_rx);
out_err_init_dummy:
	edgx_com_release(&dma->com);
out_err_init_com:
	return ret;
}

int edgx_com_dma_probe(struct edgx_br *br,
		       edgx_io_t *mngmt_base, struct edgx_com **com)
{
	int ret;
	struct edgx_ifdesc ptifd;
	ptid_t dma_pt;
	struct edgx_com_dma *dma;
	struct device *dev = edgx_br_get_dev(br);
	const struct edgx_ifreq   ifreq = { .id = AC_PORT_ID, .v_maj = 2 };

	if (dma_set_mask(dev, DMA_BIT_MASK(32))) {
		edgx_br_warn(br, "COM DMA not supported!\n");
		return -ENODEV;
	}

	if (dma_set_coherent_mask(dev, 0xffffffe0)) {
		edgx_br_warn(br, "COM DMA not supported!\n");
		return -ENODEV;
	}

	dma_pt = ffs(edgx_br_get_generic(br, BR_GX_DMA_PORTS)) - 1;

	if (!edgx_ac_get_ptif(&ifreq, dma_pt, &ptifd))
		return -ENODEV;

	dma = kzalloc(sizeof(*dma), GFP_KERNEL);
	if (!dma)
		return -ENOMEM;

	ret = edgx_com_dma_init(dma, br, ptifd.iobase + DMA_ITF_OFFS,
				mngmt_base, dma_pt);
	if (ret) {
		kfree(dma);
		return ret;
	}

	*com = &dma->com;
	edgx_br_info(br, "Setup COM DMA subsystem ... done\n");
	return 0;
}
