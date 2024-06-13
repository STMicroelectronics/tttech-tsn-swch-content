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

#include <linux/interrupt.h>

#include "edge_ac.h"
#include "edge_util.h"
#include "edge_time.h"
#include "edge_com_ts.h"
#include "edge_com.h"

#define EDGX_COM_TS_PORT_OFS	 (0x2000)
#define EDGX_COM_TS_CMD_BASE	 (0x0)
#define EDGX_COM_TX_TS_READ_CMD  ((0x0) << 4)
#define EDGX_COM_RX_TS_READ_CMD  ((0x1) << 4)
#define EDGX_COM_TRANSFER_TS_CMD ((0x1) << 14)

#define EDGX_COM_TS_RX_STATUS	 (0x10)
#define EDGX_COM_TS_TX_STATUS    (0x12)
#define EDGX_COM_TS_HDR_LEN	 (16U)     /* NOTE: error in the IP manual */
#define EDGX_COM_TS_TIME_LO	 (0x0008)
#define EDGX_COM_TS_TIME_HI	 (0x000a)

#define EDGX_COM_TS_SEC_MASK     (0xc000)
#define EDGX_COM_TS_NSEC_MASK	 (0x3fff)
#define EDGX_COM_TS_HDR_MSG_0	 (0xc)
#define EDGX_COM_TS_HDR_MSG_1	 (0xe)

#define EDGX_COM_TS_FULLSEC_MASK (0x03)
#define EDGX_COM_TS_FULL_NSEC_MASK (0x3fffffff)

#define edgx_work_to_ts(_work) container_of(_work, struct edgx_com_ts, work_tx)

static const u8 *edgx_com_get_ptp_hdr(struct sk_buff *skb)
{
	const struct ethhdr *eth = (struct ethhdr *)skb_mac_header(skb);
	const u8 *ptp_hdr;

	if (eth->h_proto != htons(ETH_P_1588))
		return NULL;

	ptp_hdr = (u8 *)(eth + 1);
	if ((ptp_hdr[0] & 0x0f) > 3)
		return NULL;

	edgx_dbg("get_ptp_hdr: %*ph\n", 2*EDGX_COM_TS_HDR_LEN, ptp_hdr);
	return ptp_hdr;
}

static int edgx_com_ts_read(edgx_io_t *ts_base, struct timespec64 *cur_time,
			    ktime_t *timestamp, u16 *hdr, u16 hdr_size)
{
	u64 sec, full_sec, full_sec_tmp;
	u32 nsec;
	u16 edgx_hdr[2] = {0};
	u16 time_hi, time_lo;
	u8  *u8hdr = (u8*)hdr;
	struct timespec64 hwts;

	memset(&hdr[0], 0, EDGX_COM_TS_HDR_LEN);
	edgx_hdr[0] = edgx_rd16(ts_base, EDGX_COM_TS_HDR_MSG_0);
	edgx_hdr[1] = edgx_rd16(ts_base, EDGX_COM_TS_HDR_MSG_1);

	/* As specified in the IP registers, using u8 to avoid endianness */
	u8hdr[0] = (u8)(edgx_hdr[0] & 0xff);
	u8hdr[4] = (u8)(edgx_hdr[0] >> 8);
	/* This is already provided in network endianness */
	hdr[15] = edgx_hdr[1];
	time_hi = edgx_rd16(ts_base, EDGX_COM_TS_TIME_HI);
	time_lo = edgx_rd16(ts_base, EDGX_COM_TS_TIME_LO);

	edgx_dbg("%s: edgx_hdr=%x:%x | time_hi:%x | time_lo:%x", __func__,
		 edgx_hdr[0], edgx_hdr[1], time_hi, time_lo);

	sec = ((time_hi & EDGX_COM_TS_SEC_MASK) >> 14);
	nsec = (((time_hi & EDGX_COM_TS_NSEC_MASK) << 16) | time_lo);
	if (nsec >= NSEC_PER_SEC) {
		nsec -= NSEC_PER_SEC;
		++sec;
	}
	/*
	 * Get full seconds for bitwidth-limited timestamper seconds.
	 * There is no way to know amount of time passed between timestamper
	 * and current time from PTP clock, assume max. 1 s has passed.
	 *
	 * Normally cur_time is captured after the timestamp is taken. However,
	 * a new timestamp can arrive also in the small time window after
	 * cur_time is captured. Therefore full_sec can be either larger
	 * or smaller than sec and the match must be evaluated in both
	 * directions.
	 */
	full_sec = cur_time->tv_sec;

	if ((full_sec & EDGX_COM_TS_FULLSEC_MASK) != sec) {
		full_sec_tmp = full_sec - 1;
		if ((full_sec_tmp & EDGX_COM_TS_FULLSEC_MASK) != sec) {
			full_sec_tmp = full_sec + 1;
			if ((full_sec_tmp & EDGX_COM_TS_FULLSEC_MASK) != sec)
				return -EINVAL;
		}
		full_sec = full_sec_tmp;
	}

	nsec &= EDGX_COM_TS_FULL_NSEC_MASK;
	set_normalized_timespec64(&hwts, full_sec, (long)nsec);
	*timestamp = timespec64_to_ktime(hwts);
	return 0;
}

static int edgx_com_ts_read_rx(struct edgx_com_pts *pts,
			       struct timespec64 *cur_time,
			       ktime_t *timestamp, u16 *hdr, u16 hdr_size)
{
	int ret;
	u16 reg;

	spin_lock_bh(pts->ts_lock);
	/* Write command to read the value from RX timestamp for current port */
	edgx_wr16(pts->iobase, EDGX_COM_TS_CMD_BASE,
		  (pts->ptid | EDGX_COM_RX_TS_READ_CMD
			     | EDGX_COM_TRANSFER_TS_CMD));
	/* Wait till transfer bit is cleared, no sleeping here */
	do {
		reg = edgx_get16(pts->iobase, EDGX_COM_TS_CMD_BASE,
				 EDGX_COM_TRANSFER_TS_CMD,
				 EDGX_COM_TRANSFER_TS_CMD);
	} while (reg);

	ret = edgx_com_ts_read(pts->iobase, cur_time, timestamp, hdr, hdr_size);
	spin_unlock_bh(pts->ts_lock);
	edgx_dbg("ts_read_rx: %*ph\n", 2*EDGX_COM_TS_HDR_LEN, (u8 *)hdr);
	return ret;
}

static int edgx_com_ts_read_tx(struct edgx_com_pts *pts,
			       struct timespec64 *cur_time,
			       ktime_t *timestamp, u16 *hdr, u16 hdr_size)
{
	int ret;
	u16 reg;

	spin_lock_bh(pts->ts_lock);
	/* Write command to read the value from TX timestamp for current port */
	edgx_wr16(pts->iobase, EDGX_COM_TS_CMD_BASE,
		  (pts->ptid | EDGX_COM_TX_TS_READ_CMD
			     | EDGX_COM_TRANSFER_TS_CMD));
	/* Sleep and then wait until transfer bit is cleared again by chip. */
	do {
		reg = edgx_get16(pts->iobase, EDGX_COM_TS_CMD_BASE,
				 EDGX_COM_TRANSFER_TS_CMD,
				 EDGX_COM_TRANSFER_TS_CMD);
	} while (reg);

	ret = edgx_com_ts_read(pts->iobase, cur_time, timestamp, hdr, hdr_size);
	spin_unlock_bh(pts->ts_lock);
	edgx_dbg("ts_read_tx: %*ph\n", 2*EDGX_COM_TS_HDR_LEN, (u8 *)hdr);
	return ret;
}

static void edgx_com_ts_find_tx(struct edgx_com_pts *pts,
				struct timespec64 *cur_time)
{
	int i = 0;
	int ts_ret;
	const u8 *hdr, *ts_hdr_u8, hdr_position[4] = {0, 4, 30, 31};
	struct skb_shared_hwtstamps hwts;
	struct sk_buff *found_skb;
	u16 ts_hdr[EDGX_COM_TS_HDR_LEN] = {0};
	u16 ts_ctrl;

	edgx_dbg("COM TS: find_tx.\n");
	/* First read the register for the TX timestamp status */
	ts_ctrl = edgx_rd16(pts->iobase, EDGX_COM_TS_TX_STATUS);
	/* If the port bit is set there is a timestamp ready to be used */
	while (ts_ctrl & BIT(pts->ptid)) {
		found_skb = NULL;
		ts_ret = edgx_com_ts_read_tx(pts, cur_time,
					     &hwts.hwtstamp,
					     ts_hdr, EDGX_COM_TS_HDR_LEN);

		while (kfifo_out_spinlocked(&pts->tx_queue, &found_skb,
					    1, &pts->lock)) {
			hdr = edgx_com_get_ptp_hdr(found_skb);
			ts_hdr_u8 = (u8 *)&ts_hdr[0];
			for (i = 0; i < 4; ++i)
				if (memcmp(&ts_hdr_u8[hdr_position[i]],
					   &hdr[hdr_position[i]],
					   sizeof(u8))) {
					dev_kfree_skb_any(found_skb);
					found_skb = NULL;
					break;
				}
			if (found_skb)
				break;
			edgx_dbg("%s: tx removed from kfifo kfifo_pos=%u, port=%u\n",
				 __func__, i++, pts->ptid);
		}

		if (found_skb) {
			/* Matching frame found, but timestamp is invalid. */
			if (ts_ret) {
				edgx_warn("%s: Cannot calculate port%u timestamp.\n",
					 __func__, pts->ptid);
				dev_kfree_skb_any(found_skb);
			} else {
				edgx_dbg("%s: ts=%llu, pos=%u\n",
					 __func__, hwts.hwtstamp, i);
				edgx_com_txts_dispatch(found_skb, &hwts);
			}
		} else
			edgx_warn("%s(): TX timestamp lost.\n", __func__);
		/* Read TX timestamp status for next loop */
		ts_ctrl = edgx_rd16(pts->iobase, EDGX_COM_TS_TX_STATUS);
	}
}

static void edgx_com_ts_find_rx(struct edgx_com_pts *pts, const u16 *hdr,
				struct timespec64 *cur_time, ktime_t *time)
{
	const u8 *hdr_u8, *ts_hdr_u8, hdr_position[4] = {0, 4, 30, 31};
	int i = 0;
	int ts_ret;
	u16 ts_hdr[EDGX_COM_TS_HDR_LEN] = {0};
	ktime_t tmp_time;
	u16 ts_ctrl;

	edgx_dbg("COM TS: find_rx.\n");
	hdr_u8 = (u8 *)hdr;
	/* First read the register for the RX timestamp status */
	ts_ctrl = edgx_rd16(pts->iobase, EDGX_COM_TS_RX_STATUS);
	/* If the port bit is set there is a timestamp ready to be used */
	while (ts_ctrl & BIT(pts->ptid)) {
		ts_ret = edgx_com_ts_read_rx(pts, cur_time, &tmp_time,
					     ts_hdr, EDGX_COM_TS_HDR_LEN);
		ts_hdr_u8 = (u8 *)&ts_hdr[0];
		for (i = 0; i < 4; ++i)
			if (memcmp(&ts_hdr_u8[hdr_position[i]],
				   &hdr_u8[hdr_position[i]], sizeof(u8)))
				break;
		if (i == 4) {
			/* Matching frame found, but timestamp is invalid. */
			if (ts_ret) {
				edgx_err("%s: Cannot calculate port%u timestamp!\n",
					 __func__, pts->ptid);
				break;
			}
			*time = tmp_time;
			edgx_dbg("%s: ts=%lld, port=%d\n",
				 __func__, *time, pts->ptid);
			break;
		}
		edgx_err("%s: rx not found: port=%u\n", __func__, pts->ptid);
		/* Read RX timestamp status for next loop */
		ts_ctrl = edgx_rd16(pts->iobase, EDGX_COM_TS_RX_STATUS);
	}
}

int edgx_com_ts_cfg_get(struct edgx_com_ts *ts, ptcom_t ptcom,
			struct ifreq *ifr)
{
	struct edgx_com_pts *pts = ts->pts[ffs(ptcom) - 1];

	if (!pts)
		return -EINVAL;

	return copy_to_user(ifr->ifr_data, &pts->hwts_cfg,
			    sizeof(pts->hwts_cfg)) ? -EFAULT : 0;
}

/* Set per port HW timestamp configuration.
 *  NOTE: There is no possibility to set per port timestamp in the IP.
 *  It is a property of the switch. Therefore it is not possible to switch
 *  between L2 and L4 from the port interface without affecting other ports.
 */
int edgx_com_ts_cfg_set(struct edgx_com_ts *ts, ptcom_t ptcom,
			struct ifreq *ifr)
{
	struct edgx_com_pts *pts = ts->pts[ffs(ptcom) - 1];
	struct hwtstamp_config cfg;

	if (!pts)
		return -EINVAL;

	if (copy_from_user(&cfg, ifr->ifr_data, sizeof(cfg)))
		return -EFAULT;

	if (cfg.flags)
		return -EINVAL;

	switch (cfg.tx_type) {
	case HWTSTAMP_TX_OFF:
	case HWTSTAMP_TX_ON:
		pts->hwts_cfg.tx_type = cfg.tx_type;
		break;
	default:
		return -ERANGE;
	}

	switch (cfg.rx_filter) {
	case HWTSTAMP_FILTER_NONE:
		pts->hwts_cfg.rx_filter = cfg.rx_filter;
		break;
	/* Only V2 L2 is supported now. L4 requires special IPO rules */
	case HWTSTAMP_FILTER_PTP_V2_L2_SYNC:
	case HWTSTAMP_FILTER_PTP_V2_L2_DELAY_REQ:
		/* With V2 type filters which specify a Sync or Delay Request,
		 * Path Delay Request/Response messages are also time stamped
		 * by hardware so notify the caller the requested packets plus
		 * some others are time stamped.
		 */
		cfg.rx_filter = HWTSTAMP_FILTER_SOME;
		pts->hwts_cfg.rx_filter = HWTSTAMP_FILTER_PTP_V2_EVENT;
		break;
	case HWTSTAMP_FILTER_PTP_V2_L2_EVENT:
		pts->hwts_cfg.rx_filter = cfg.rx_filter;
		break;
	default:
		return -ERANGE;
	}

	return copy_to_user(ifr->ifr_data, &cfg, sizeof(cfg)) ?
			    -EFAULT : 0;
}

struct sk_buff *edgx_com_ts_xmit(struct edgx_com_ts *ts,
				 struct sk_buff *skb, ptcom_t ptcom)
{
	const u8            *ptp_hdr;
	struct sk_buff      *skb_backup = NULL;
	struct edgx_com_pts *pts;
	ptid_t 		     ptid = ffs(ptcom) - 1;

	if ((ptid == PT_EP_ID) || !ts->pts[ptid])
		return skb;

	pts = ts->pts[ptid];

	ptp_hdr = edgx_com_get_ptp_hdr(skb);
	if (!ptp_hdr)
		return skb;

	skb_backup = skb_clone(skb, GFP_ATOMIC);
	if (!skb_backup) {
		edgx_err("%s(): Cannot clone skb!\n", __func__);
		return skb;
	}

	if (!kfifo_in_spinlocked(&pts->tx_queue, &skb, 1, &pts->lock)) {
		dev_kfree_skb_any(skb_backup);
		edgx_warn("%s(): Tx timestamp dropped.\n", __func__);
		return skb;
	}

	skb_shinfo(skb)->tx_flags |= SKBTX_IN_PROGRESS;
	skb_tx_timestamp(skb);
	edgx_dbg("%s: queued. pos=%d\n", __func__, kfifo_len(&pts->tx_queue));
	return skb_backup;
}

static void edgx_com_ts_work_tx(struct work_struct *work)
{
	ptid_t ptid;
	struct timespec64 cur_time;
	struct edgx_time *time;
	struct edgx_com_pts *pts;
	struct edgx_com_ts *ts = edgx_work_to_ts(work);

	edgx_dbg("COM TS: work_tx.\n");
	edgx_br_clr_int_stat(ts->parent, BIT(EDGX_IRQ_NR_TS_TX));

	for (ptid = 0; ptid < EDGX_BR_MAX_PORTS; ptid++) {
		pts = ts->pts[ptid];
		if (pts) {
			time = edgx_br_get_pt_time(ts->parent, ptid);
			edgx_tm_get_ts_time(time, &cur_time);
			edgx_com_ts_find_tx(pts, &cur_time);
		}
	}
	edgx_br_irq_enable(ts->parent, EDGX_IRQ_NR_TS_TX);
}

irqreturn_t edgx_com_ts_isr(struct edgx_com_ts *ts)
{
	edgx_dbg("COM TS: Isr.\n");
	edgx_br_irq_disable(ts->parent, EDGX_IRQ_NR_TS_TX);
	queue_work(ts->wq_tx, &ts->work_tx);

	return IRQ_HANDLED;
}

void edgx_com_ts_rx(struct edgx_com_ts *ts, struct sk_buff *skb, ptcom_t ptcom)
{
	u16                          ptid = ffs(ptcom) - 1;
	struct edgx_com_pts         *pts = ts->pts[ptid];
	struct skb_shared_hwtstamps *hwts;
	const u8                    *ptp_hdr;
	struct timespec64            cur_time;
	struct edgx_time            *time;

	hwts = skb_hwtstamps(skb);
	hwts->hwtstamp = ktime_set(0, 0);

	if (!pts)
		return;

	ptp_hdr = edgx_com_get_ptp_hdr(skb);
	if (!ptp_hdr)
		return;

	time = edgx_br_get_pt_time(ts->parent, ptid);
	edgx_tm_get_ts_time(time, &cur_time);
	edgx_com_ts_find_rx(pts, (const u16 *)ptp_hdr,
			    &cur_time, &hwts->hwtstamp);
}

int edgx_com_ts_init(struct edgx_com_ts *ts, edgx_io_t *mngmnt_base,
		     struct edgx_br *br)
{
	int                       ret, i;
	const struct edgx_ifdesc *ifd;
	struct edgx_ifdesc        pifd;
	ptid_t                    ptid;
	const struct edgx_ifreq   ifreq = { .id = AC_PORT_TS_ID, .v_maj = 2 };
	struct edgx_com_pts      *pts;

	ifd = edgx_ac_get_if(&ifreq);
	if (!ifd)
		return -ENODEV;

	ts->parent = br;

	INIT_WORK(&ts->work_tx, &edgx_com_ts_work_tx);
	ts->wq_tx = alloc_workqueue("ts-wq",
				    WQ_HIGHPRI | WQ_MEM_RECLAIM, 0);
	if (!ts->wq_tx) {
		edgx_err("COM TS: alloc_workqueue failed!\n");
		ret = -ENOMEM;
		goto out_err_alloc_work;
	}

	spin_lock_init(&ts->ts_lock);

	edgx_ac_for_each_ifpt(ptid, ifd, &pifd) {
		if (!strncmp(syncmode, "1AS", 3) && (ptid == 0))
			continue;

		pts = kzalloc(sizeof(*pts), GFP_KERNEL);
		if (!pts) {
			edgx_err("COM TS: allocation failed!\n");
			ret = -ENOMEM;
			goto out_err_alloc;
		}
		spin_lock_init(&pts->lock);
		pts->iobase = mngmnt_base + EDGX_COM_TS_PORT_OFS;
		pts->ptid = ptid;
		pts->ts_lock = &ts->ts_lock;
		ts->pts[ptid] = pts;
	}
	for (i = 0; i < EDGX_BR_MAX_PORTS; i++)
		if (ts->pts[i])
			INIT_KFIFO(ts->pts[i]->tx_queue);

	return 0;

out_err_alloc:
	for (i = 0; i < EDGX_BR_MAX_PORTS; i++)
		kfree(ts->pts[i]);
	destroy_workqueue(ts->wq_tx);
out_err_alloc_work:
	return ret;
}

void edgx_com_ts_shutdown(struct edgx_com_ts *ts)
{
	int i;
	struct edgx_com_pts *pts;
	struct sk_buff *skb = NULL;

	destroy_workqueue(ts->wq_tx);

	for (i = 0; i < EDGX_BR_MAX_PORTS; i++) {
		pts = ts->pts[i];
		if (pts) {
			while (kfifo_out_spinlocked(&pts->tx_queue,
						    &skb, 1, &pts->lock)) {
				dev_kfree_skb_any(skb);
			}
			kfree(pts);
		}
	}
}
