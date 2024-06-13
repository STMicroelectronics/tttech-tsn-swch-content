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

#include "edge_ac.h"
#include "edge_port.h"
#include "edge_preempt.h"
#include "edge_stat.h"
#include "edge_link.h"
#include "edge_util.h"

enum stat_cnt {
	STAT_SMD_ERR = 0,
	STAT_ASS_ERR,
	STAT_ASS_OK,
	STAT_FRAG_CNT_RX,
	STAT_FRAG_CNT_TX,
	STAT_MAX
};

/* Verification status of preemption operation */
enum verif_status {
	VERIF_UNKNOWN = 1,	/* Verification status is unknown */
	VERIF_INIT,		/* Verification is in initial phase */
	VERIF_INPROCESS,	/* Verification is in progress */
	VERIF_SUCCESS,		/* Verification succeeded */
	VERIF_FAILED,		/* Verification failed */
	VERIF_DISABLED		/* Verification disabled */
};

enum tx_status {
	TX_STATUS_UNKNOWN = 1,
	TX_STATUS_INACTIVE,
	TX_STATUS_ACTIVE
};

/* Base address for retrieving frame/frag counter values */
#define PMT_STAT_PORT_BASE	0x270
/* Register address for setting queues preemption */
#define PMT_TX_PREE0		0x001C
/* Register address for setting general preemption settings */
#define PMT_TX_PREE1		0x001E
/* Max number of verification attempts */
#define PMT_VERIFY_LIMIT	0x3
/* Express traffic - can't be preempted */
#define PMT_EXPRESS		0x1
/* Preemptible traffic - can be preempted by express traffic*/
#define PMT_PREEMPT		0x2
/* AddFragSize is presented with 2 least significant bits */
#define ADD_FRAG_SIZE_MASK	0x3
/* Length of tx_clk cycle in ns for 10Mbit/s line speed */
#define MII_CYCLE_NS_10		400U
/* Length of tx_clk cycle in ns for 100Mbit/s line speed */
#define MII_CYCLE_NS_100	40U
/* Length of tx_clk cycle in ns for 1000Mbit/s line speed */
#define MII_CYCLE_NS_1000	8U
/* Maximum number of queues */
#define MAX_NR_QUEUES		8U

#define ADV_CLK_CYCLES(_reg) ((_reg) & 0xFF)
#define ADV_MII_CYCLES(_reg) (((_reg) >> 8) & 0xFF)

/* Values of masks used for state machine operation. Following shortcuts denote
 * 'in' variables: VER = verified, LFAIL = link fail, ENA = preemption enable,
 * DISVER = disable verify. Result is deducted by bitwise OR operation between
 * in variables:  b0 - p_enable, b1: disable_ver, b2: link_fail, b3: verified
 */
#define SM_VER_ENA		0x9
#define SM_LFAIL_DISVER_ENA	0x7
#define SM_DISVER_ENA		0x3
#define SM_LFAIL_DISVER		0x6
#define SM_DISVER		0x2
#define SM_LFAIL_ENA		0x5
#define SM_LFAIL		0x4
#define SM_ZERO			0x0
#define SM_ENA			0x1

#define PREEMPT_SAFE_SET(_pmt, _reg, _v)				\
	do {								\
		unsigned long irq_flags;				\
		typeof(_pmt) _pp = (_pmt);				\
		spin_lock_irqsave(&_pp->lock, irq_flags);		\
		*(_reg) = _v;						\
		spin_unlock_irqrestore(&_pp->lock, irq_flags);	\
	} while (0)

static const struct edgx_statinfo preempt_stat = {
	.feat_id = EDGX_STAT_FEAT_PREEMPT,
	.rate_ms = 200000,
	.base    = PMT_STAT_PORT_BASE,
	.nwords  = STAT_MAX,
};

/* Base preempt structure */
struct edgx_preempt {
	struct edgx_pt		*parent; /* Pointer to parent - port */
	edgx_io_t		*iobase; /* Base port address */
	struct edgx_com_hdl	*hcom;   /* Communication handle */
	struct edgx_stat_hdl    *hstat;  /* Counters handle */
	struct hrtimer		timer;   /* High resolution timer */
	spinlock_t		lock;    /* Protect edgx_preempt members */
	enum verif_status	ver_state;   /* Verification status */
	bool			disable_ver; /* Disable verification */
	bool			p_enable;    /* Enable preemption */
	bool			link_fail;   /* Link failure */
	bool			verified;    /* Verification ok */
	u8			ver_time;    /* Verification time [ms] */
		   /* Type of traffic (preempt/express) for different queues */
	u16			pmt_status;
};

static void preempt_update_sm(struct edgx_preempt *pmt);

static bool preempt_calc_p_active(struct edgx_preempt *pmt)
{
	return (pmt->p_enable & (pmt->verified | pmt->disable_ver));
}

static void preempt_set_tx(struct edgx_preempt *pmt)
{
	/*if preemption is not active, it is disabled*/
	if (!preempt_calc_p_active(pmt))
		edgx_wr16(pmt->iobase, PMT_TX_PREE0, 0);
	else
		edgx_wr16(pmt->iobase, PMT_TX_PREE0, pmt->pmt_status);
}

static inline struct edgx_preempt *edgx_dev2preempt(struct device *dev)
{
	return edgx_pt_get_preempt(edgx_dev2pt(dev));
}

static ssize_t ver_dis_tx_show(struct device *dev,
			       struct device_attribute *attr,
			       char *buf)
{
	struct edgx_preempt *pmt = edgx_dev2preempt(dev);

	return scnprintf(buf, PAGE_SIZE, "%u\n", pmt->disable_ver);
}

static ssize_t ver_dis_tx_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf,
				size_t count)
{
	ssize_t ret = 0;
	struct edgx_preempt *pmt = edgx_dev2preempt(dev);

	if (kstrtobool(buf, &pmt->disable_ver))
		ret = -EINVAL;
	if (pmt->disable_ver)
		PREEMPT_SAFE_SET(pmt, &pmt->verified, false);
	preempt_update_sm(pmt);

	return ret ? ret : count;
}

static ssize_t enable_tx_show(struct device *dev,
			      struct device_attribute *attr,
			      char *buf)
{
	struct edgx_preempt *pmt = edgx_dev2preempt(dev);

	return scnprintf(buf, PAGE_SIZE, "%u\n", pmt->p_enable);
}

static ssize_t enable_tx_store(struct device *dev,
			       struct device_attribute *attr,
			       const char *buf,
			       size_t count)
{
	ssize_t ret = 0;
	struct edgx_preempt *pmt = edgx_dev2preempt(dev);

	if (kstrtobool(buf, &pmt->p_enable))
		ret = -EINVAL;
	if (!pmt->p_enable)
		PREEMPT_SAFE_SET(pmt, &pmt->verified, false);
	preempt_update_sm(pmt);

	return ret ? ret : count;
}

static ssize_t ver_time_show(struct device *dev,
			     struct device_attribute *attr,
			     char *buf)
{
	struct edgx_preempt *pmt = edgx_dev2preempt(dev);

	return scnprintf(buf, PAGE_SIZE, "%u\n", pmt->ver_time);
}

static ssize_t ver_time_store(struct device *dev,
			      struct device_attribute *attr,
			      const char *buf,
			      size_t count)
{
	struct edgx_preempt *pmt = edgx_dev2preempt(dev);
	u8 loc_ver_time;

	if (kstrtou8(buf, 0, &loc_ver_time))
		return -EINVAL;

	if (loc_ver_time < 1 || loc_ver_time > 128) {
		edgx_pt_err(pmt->parent,
			    "Preempt:Value out of range, allowed: [1,128]\n");
		return -EINVAL;
	}
	pmt->ver_time = loc_ver_time;
	return count;
}

static ssize_t frame_smd_err_show(struct device *dev,
				  struct device_attribute *attr,
				  char *buf)
{
	struct edgx_preempt *pmt = edgx_dev2preempt(dev);

	return scnprintf(buf, PAGE_SIZE, "%llu\n",
			edgx_stat_upget(pmt->hstat, STAT_SMD_ERR));
}

static ssize_t frame_ass_err_show(struct device *dev,
				  struct device_attribute *attr,
				  char *buf)
{
	struct edgx_preempt *pmt = edgx_dev2preempt(dev);

	return scnprintf(buf, PAGE_SIZE, "%llu\n",
			edgx_stat_upget(pmt->hstat, STAT_ASS_ERR));
}

static ssize_t frame_ass_ok_show(struct device *dev,
				 struct device_attribute *attr,
				 char *buf)
{
	struct edgx_preempt *pmt = edgx_dev2preempt(dev);

	return scnprintf(buf, PAGE_SIZE, "%llu\n",
			edgx_stat_upget(pmt->hstat, STAT_ASS_OK));
}

static ssize_t frag_cnt_tx_show(struct device *dev,
				struct device_attribute *attr,
				char *buf)
{
	struct edgx_preempt *pmt = edgx_dev2preempt(dev);

	return scnprintf(buf, PAGE_SIZE, "%llu\n",
			edgx_stat_upget(pmt->hstat, STAT_FRAG_CNT_TX));
}

static ssize_t frag_cnt_rx_show(struct device *dev,
				struct device_attribute *attr,
				char *buf)
{
	struct edgx_preempt *pmt = edgx_dev2preempt(dev);

	return scnprintf(buf, PAGE_SIZE, "%llu\n",
			edgx_stat_upget(pmt->hstat, STAT_FRAG_CNT_RX));
}

static ssize_t support_show(struct device *dev,
			    struct device_attribute *attr,
			    char *buf)
{
	/*If initialization is ok, mac merge is supported*/
	return scnprintf(buf, PAGE_SIZE, "%u\n", 1);
}

static ssize_t stat_verify_show(struct device *dev,
				struct device_attribute *attr,
				char *buf)
{
	struct edgx_preempt *pmt = edgx_dev2preempt(dev);

	return scnprintf(buf, PAGE_SIZE, "%u\n", pmt->ver_state);
}

static ssize_t add_frag_size_show(struct device *dev,
				  struct device_attribute *attr,
				  char *buf)
{
	struct edgx_preempt *pmt = edgx_dev2preempt(dev);
	u16 add_f_sz = edgx_rd16(pmt->iobase, PMT_TX_PREE1);

	return scnprintf(buf, PAGE_SIZE, "%u\n", add_f_sz & ADD_FRAG_SIZE_MASK);
}

static ssize_t add_frag_size_store(struct device *dev,
				   struct device_attribute *attr,
				   const char *buf,
				   size_t count)
{
	struct edgx_preempt *pmt = edgx_dev2preempt(dev);
	u16 reg_val = 0;

	if (kstrtou16(buf, 0, &reg_val))
		return -EINVAL;
	edgx_wr16(pmt->iobase, PMT_TX_PREE1, reg_val & ADD_FRAG_SIZE_MASK);

	return count;
}

static ssize_t stat_tx_show(struct device *dev,
			    struct device_attribute *attr,
			    char *buf)
{
	return scnprintf(buf, PAGE_SIZE, "%u\n", TX_STATUS_UNKNOWN);
}

static ssize_t preempt_active_show(struct device *dev,
				   struct device_attribute *attr, char *buf)
{
	struct edgx_preempt *pmt = edgx_dev2preempt(dev);

	return scnprintf(buf, PAGE_SIZE, "%u\n", preempt_calc_p_active(pmt));
}

static ssize_t rel_adv_show(struct device *dev,
			    struct device_attribute *attr, char *buf)
{
	struct edgx_preempt *pmt = edgx_dev2preempt(dev);
	u32 rel_adv = 0;
	unsigned int mii_cycle = 0;
	u16 reg = 0;
	int clk_cycle = edgx_br_get_cycle_ns(edgx_pt_get_br(pmt->parent));
	struct edgx_br *br = edgx_pt_get_br(pmt->parent);

	switch (edgx_pt_get_speed(pmt->parent)) {
	case SPEED_10:
		reg = edgx_br_get_feature(br, BR_FEAT_RELADV_10);
		mii_cycle = MII_CYCLE_NS_10;
		break;
	case SPEED_100:
		reg = edgx_br_get_feature(br, BR_FEAT_RELADV_100);
		mii_cycle = MII_CYCLE_NS_100;
		break;
	case SPEED_1000:
		reg = edgx_br_get_feature(br, BR_FEAT_RELADV_1000);
		mii_cycle = MII_CYCLE_NS_1000;
		break;
	default:
		edgx_pt_warn(pmt->parent,
			     "Default release advance value used\n");
		break;
	}

	rel_adv = clk_cycle * ADV_CLK_CYCLES(reg) +
		  mii_cycle * ADV_MII_CYCLES(reg);
	return scnprintf(buf, PAGE_SIZE, "%u\n", rel_adv);
}

static ssize_t hold_adv_show(struct device *dev,
			     struct device_attribute *attr,
			     char *buf)
{
	struct edgx_preempt *pmt = edgx_dev2preempt(dev);
	u32 hold_adv = 0;
	unsigned int mii_cycle = 0;
	u16 reg = 0;
	int clk_cycle = edgx_br_get_cycle_ns(edgx_pt_get_br(pmt->parent));
	u16 add_frag = edgx_rd16(pmt->iobase, PMT_TX_PREE1);
	struct edgx_br *br = edgx_pt_get_br(pmt->parent);

	switch (edgx_pt_get_speed(pmt->parent)) {
	case SPEED_10:
		reg = edgx_br_get_feature(br, BR_FEAT_HOLDADV_10);
		mii_cycle = MII_CYCLE_NS_10;
		break;
	case SPEED_100:
		reg = edgx_br_get_feature(br, BR_FEAT_HOLDADV_100);
		mii_cycle = MII_CYCLE_NS_100;
		break;
	case SPEED_1000:
		reg = edgx_br_get_feature(br, BR_FEAT_HOLDADV_1000);
		mii_cycle = MII_CYCLE_NS_1000;
		break;
	default:
		edgx_pt_warn(pmt->parent, "Default hold advance value used\n");
		hold_adv = 0x0;
		break;
	}

	hold_adv = clk_cycle * ADV_CLK_CYCLES(reg) + mii_cycle *
		   (128 + add_frag * 64 + ADV_MII_CYCLES(reg));
	return scnprintf(buf, PAGE_SIZE, "%u\n", hold_adv);
}

static ssize_t frame_preempt_status_read(struct file *filp,
					 struct kobject *kobj,
					 struct bin_attribute *b_attr,
					 char *buf, loff_t ofs,
					 size_t count)
{
	loff_t idx = 0;
	struct edgx_preempt *pmt = edgx_dev2preempt(kobj_to_dev(kobj));
	u16 reg_val = 0;
	u8 nr_queues = edgx_br_get_generic(edgx_pt_get_br(pmt->parent),
					   BR_GX_QUEUES);

	if (edgx_sysfs_tbl_params(ofs, count, sizeof(u8), &idx) ||
	    idx > (nr_queues - 1))
		return -EINVAL;

	reg_val = edgx_rd16(pmt->iobase, PMT_TX_PREE0);

	/*1 must be added to value read from register since 802.1Qbu specifies
	 * {express (1), preempt (2)}, while reg TX_PREE0 returns
	 * 0 -> express and 1 -> preempt traffic
	 */
	((u8 *)buf)[0] = ((reg_val >> idx) & 1) + 1;

	return count;
}

static ssize_t frame_preempt_status_write(struct file *filp,
					  struct kobject *kobj,
					  struct bin_attribute *bin_attr,
					  char *buf,
					  loff_t ofs, size_t count)
{
	loff_t idx = 0;
	struct edgx_preempt *pmt = edgx_dev2preempt(kobj_to_dev(kobj));
	u8 nr_queues = edgx_br_get_generic(edgx_pt_get_br(pmt->parent),
					   BR_GX_QUEUES);

	if (edgx_sysfs_tbl_params(ofs, count, sizeof(u8), &idx) ||
	    idx > (nr_queues - 1) ||
	    *((u8 *)buf) < PMT_EXPRESS || *((u8 *)buf) > PMT_PREEMPT)
		return -EINVAL;

	if (*((u8 *)buf) == PMT_PREEMPT)
		pmt->pmt_status |= (1 << idx);
	else
		pmt->pmt_status &= ~(1 << idx);

	preempt_set_tx(pmt);

	return count;
}

/* Although these attribute functions are interface functions,
 * their arguments are not validated, because the sysfs entries
 * exist only if edgx_probe_preempt() succeeds.
 */
EDGX_DEV_ATTR_RW(ver_dis_tx, "verifyDisableTx");
EDGX_DEV_ATTR_RW(enable_tx, "enableTx");
EDGX_DEV_ATTR_RW(ver_time, "verifyTime");
EDGX_DEV_ATTR_RO(frame_smd_err, "frameSmdErrorCount");
EDGX_DEV_ATTR_RO(frame_ass_err, "frameAssErrCount");
EDGX_DEV_ATTR_RO(frame_ass_ok, "frameAssOkCount");
EDGX_DEV_ATTR_RO(frag_cnt_tx, "fragCountTx");
EDGX_DEV_ATTR_RO(frag_cnt_rx, "fragCountRx");
EDGX_DEV_ATTR_RO(support, "support");
EDGX_DEV_ATTR_RO(stat_verify, "statusVerify");
EDGX_DEV_ATTR_RW(add_frag_size, "addFragSize");
EDGX_DEV_ATTR_RO(stat_tx, "statusTx");
EDGX_DEV_FCT_ATTR_RO(holdcnt, edgx_sysfs_zero, "holdCount");

EDGX_DEV_ATTR_RO(preempt_active, "preemptionActive");
EDGX_DEV_ATTR_RO(hold_adv, "holdAdvance");
EDGX_DEV_ATTR_RO(rel_adv, "releaseAdvance");
EDGX_BIN_ATTR_RW(frame_preempt_status, "framePreemptionStatusTable",
		 MAX_NR_QUEUES * sizeof(u8));
/* DE-IP does not provide this, so we always report 'hold' status */
EDGX_DEV_FCT_ATTR_RO(holdreq, edgx_sysfs_one, "holdRequest");

static struct attribute *ieee8023_mm_attrs[] = {
	&dev_attr_enable_tx.attr,
	&dev_attr_ver_dis_tx.attr,
	&dev_attr_ver_time.attr,
	&dev_attr_frame_smd_err.attr,
	&dev_attr_frame_ass_err.attr,
	&dev_attr_frame_ass_ok.attr,
	&dev_attr_frag_cnt_tx.attr,
	&dev_attr_frag_cnt_rx.attr,
	&dev_attr_support.attr,
	&dev_attr_stat_verify.attr,
	&dev_attr_add_frag_size.attr,
	&dev_attr_stat_tx.attr,
	&dev_attr_holdcnt.attr,
	NULL,
};

static struct attribute *ieee8021_preempt_attr[] = {
	&dev_attr_hold_adv.attr,
	&dev_attr_preempt_active.attr,
	&dev_attr_rel_adv.attr,
	&dev_attr_holdreq.attr,
	NULL,
};

static struct bin_attribute *ieee8021_preempt_binattr[] = {
	&bin_attr_frame_preempt_status,
	NULL,
};

static struct attribute_group ieee8023_mm_group = {
	.name = "ieee8023MacMerge",
	.attrs = ieee8023_mm_attrs,
};

static struct attribute_group ieee8021_preempt_group = {
	.name = "ieee8021Preemption",
	.attrs = ieee8021_preempt_attr,
	.bin_attrs = ieee8021_preempt_binattr,
};

static void preempt_start_ver(struct edgx_preempt *preempt)
{
	ktime_t period = ktime_set(0, preempt->ver_time * NSEC_PER_MSEC);
	u64 delta_ns = (preempt->ver_time * 20u * NSEC_PER_MSEC) / 100u;

	if (!hrtimer_active(&preempt->timer)) {
		hrtimer_start_range_ns(&preempt->timer, period, delta_ns,
				       HRTIMER_MODE_REL);
	}
}

static void preempt_update_sm(struct edgx_preempt *pmt)
{
	u8 result = (pmt->verified << 3) |
		    (pmt->link_fail << 2) |
		    (pmt->disable_ver << 1) |
		    (pmt->p_enable & 0x1);

	switch (result) {
	case SM_VER_ENA:
		PREEMPT_SAFE_SET(pmt, &pmt->ver_state, VERIF_SUCCESS);
		break;
	case SM_LFAIL_DISVER_ENA:
	case SM_DISVER_ENA:
	case SM_LFAIL_DISVER:
	case SM_DISVER:
		PREEMPT_SAFE_SET(pmt, &pmt->ver_state, VERIF_DISABLED);
		break;
	case SM_LFAIL_ENA:
	case SM_LFAIL:
	case SM_ZERO:
		PREEMPT_SAFE_SET(pmt, &pmt->ver_state, VERIF_INIT);
		break;
	case SM_ENA:
		preempt_start_ver(pmt);
		break;
	}
	preempt_set_tx(pmt);
}

static int preempt_send_frame(void *pp, ptflags_t flag)
{
	struct edgx_preempt *preempt = (struct edgx_preempt *)pp;
	struct sk_buff *skb;
	struct ethhdr *eth;
	void *data;

	skb = dev_alloc_skb(ETH_ZLEN + 2);
	if (!skb)
		return -ENOBUFS;

	eth = (struct ethhdr *)skb_push(skb, ETH_HLEN);
	memset(eth, 0, sizeof(*eth));

	skb_set_mac_header(skb, 0);
	skb_set_network_header(skb, skb->len);

	data = skb_push(skb, ETH_ZLEN - ETH_HLEN);
	memset(data, 0, ETH_ZLEN - ETH_HLEN);

	skb->protocol = htons(ETH_P_802_3);
	skb->dev = edgx_pt_get_netdev(preempt->parent);

	edgx_com_xmit(preempt->hcom, skb, flag);

	return 0;
}

static enum hrtimer_restart preempt_verification_fn(struct hrtimer *timer)
{
	struct edgx_preempt *pmt = container_of(timer, struct edgx_preempt,
						timer);
	bool stop_timer = false;
	static u8 ver_cnt;
	ktime_t period = ktime_set(0, pmt->ver_time * NSEC_PER_MSEC);

	stop_timer = pmt->disable_ver | pmt->link_fail | (!pmt->p_enable) |
			  pmt->verified;
	if (stop_timer) {
		ver_cnt = 0;
		return  HRTIMER_NORESTART;
	}

	if (ver_cnt < PMT_VERIFY_LIMIT) {
		PREEMPT_SAFE_SET(pmt, &pmt->ver_state, VERIF_INPROCESS);
		ver_cnt++;
		preempt_send_frame(pmt, COM_FLAG_SMD_V);
	} else {
		PREEMPT_SAFE_SET(pmt, &pmt->verified, false);
		/* After verification fails, timer is restarted with period
		 * which is equal 100xTimerPeriod
		 */
		period = ((u64)pmt->ver_time * NSEC_PER_MSEC * 100);
		ver_cnt = 0;
		PREEMPT_SAFE_SET(pmt, &pmt->ver_state, VERIF_FAILED);
	}

	hrtimer_forward(&pmt->timer, hrtimer_get_expires(&pmt->timer), period);

	return HRTIMER_RESTART;
}

void edgx_preempt_link_chng(struct edgx_pt *pt, int link, int speed)
{
	struct edgx_preempt *pmt = edgx_pt_get_preempt(pt);

	if (!pmt)
		return;

	/* link_fail = 1 if there is no link */
	pmt->link_fail = !link;
	if (pmt->link_fail)
		PREEMPT_SAFE_SET(pmt, &pmt->verified, false);

	preempt_update_sm(pmt);
}

int edgx_probe_preempt(struct edgx_pt *pt, struct edgx_com_hdl *hcom,
		       edgx_io_t *iobase, struct edgx_preempt **ppreempt)
{
	const struct edgx_ifdesc *ifd;
	struct edgx_ifdesc        ptifd;
	const struct edgx_ifreq   ifreq = { .id = AC_PREEMPT_ID, .v_maj = 1 };
	ptid_t                    ptid  = edgx_pt_get_id(pt);

	if (!pt || !ppreempt || !hcom)
		return -EINVAL;
	ifd = edgx_ac_get_ptif(&ifreq, ptid, &ptifd);
	if (!ifd)
		return -ENODEV;
	*ppreempt = kzalloc(sizeof(**ppreempt), GFP_KERNEL);
	if (!(*ppreempt)) {
		edgx_pt_err(pt, "Cannot allocate Frame Preemption\n");
		return -ENOMEM;
	}

	(*ppreempt)->parent = pt;
	(*ppreempt)->hcom   = hcom;
	(*ppreempt)->iobase = iobase;
	(*ppreempt)->ver_time = 10;
	(*ppreempt)->link_fail = !edgx_link_get_state(edgx_pt_get_link(pt));
	(*ppreempt)->ver_state = VERIF_INIT;
	(*ppreempt)->verified = false;
	(*ppreempt)->pmt_status = 0;
	(*ppreempt)->p_enable = false;
	(*ppreempt)->disable_ver = false;
	preempt_set_tx(*ppreempt);

	hrtimer_init(&(*ppreempt)->timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	(*ppreempt)->timer.function = &preempt_verification_fn;

	spin_lock_init(&(*ppreempt)->lock);

	edgx_pt_add_sysfs(pt, &ieee8023_mm_group);
	edgx_pt_add_sysfs(pt, &ieee8021_preempt_group);

	(*ppreempt)->hstat = edgx_stat_alloc_hdl(edgx_br_get_pt_stat
						(edgx_pt_get_br(pt), ptid),
						ptid, &preempt_stat);
	if (!(*ppreempt)->hstat) {
		edgx_pt_err(pt, "Cannot allocate Status Handle\n");
		return -ENOMEM;
	}

	return 0;
}

void edgx_shutdown_preempt(struct edgx_preempt *preempt)
{
	if (preempt) {
		hrtimer_cancel(&preempt->timer);
		edgx_pt_rem_sysfs(preempt->parent, &ieee8023_mm_group);
		edgx_pt_rem_sysfs(preempt->parent, &ieee8021_preempt_group);
		edgx_stat_free_hdl(preempt->hstat);
		kfree(preempt);
	}
}

void edgx_preempt_rcv(struct edgx_preempt *pmt, struct sk_buff *skb,
		      ptflags_t flags)
{
	if (pmt) {
		if (flags == COM_FLAG_SMD_V) {
			preempt_send_frame(pmt, COM_FLAG_SMD_R);
		} else if (flags == COM_FLAG_SMD_R) {
			PREEMPT_SAFE_SET(pmt, &pmt->verified, true);
			preempt_update_sm(pmt);
		}
	}
	(void)skb;
}
