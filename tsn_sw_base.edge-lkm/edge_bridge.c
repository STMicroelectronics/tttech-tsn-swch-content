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

#include <linux/bitops.h>
#include <linux/if_bridge.h>
#include <linux/etherdevice.h>
#include <linux/ethtool.h>
#include <linux/delay.h>

#include "edge_bridge.h"
#include "edge_ac.h"
#include "edge_br_fdb.h"
#include "edge_br_vlan.h"
#include "edge_bridge.h"
#include "edge_link.h"
#include "edge_com.h"
#include "edge_port.h"
#include "edge_psfp.h"
#include "edge_stat.h"
#include "edge_time.h"
#include "edge_util.h"
#include "edge_st.h"
#include "edge_br_sid.h"
#include "edge_frer.h"

struct edgx_br {
	struct {
		struct net_device *dev;
		unsigned int       refs;
	} bridge;
	struct {
		u8           base_mac[ETH_ALEN];
		unsigned int id;
	} hw;

	struct list_head     entry;
	struct list_head     grp_list;
	struct device       *pdev;

	struct edgx_time    *time;
	ptvec_t              time_map;

	struct edgx_stat    *stat;
	ptvec_t              stat_map;

	struct edgx_com     *com;

	edgx_io_t           *brmgmt;
	edgx_io_t           *brparam;
	struct edgx_brfdb   *fdb;
	struct edgx_br_vlan *vlan;

	struct edgx_pt      *br_pt[EDGX_BR_MAX_PORTS];
	struct edgx_pt      *ep_pt;

	edgx_io_t           *iobase;

	clock_t              ageing_time;
	unsigned int         cycle_ns;

	struct edgx_st_com	*st_com;
	struct edgx_sid_br	*sid;
	struct edgx_frer	*frer;
	struct edgx_psfp	*psfp;

	spinlock_t		 lock; /* Sync. access to IRQ mask */
	struct workqueue_struct	*owq;

	struct edgx_br_irq	*irq;

	enum edgx_lnk_mode	lnk_mode;
	const char          *driver;
};

struct _sysfs_grp {
	struct attribute_group *grp;
	struct list_head        list;
};

static char *netif = "(none)";
static unsigned int resmactc = EDGX_BR_DEFAULT_RESMACTC;
int csrating;
char *syncmode = "1AS";
static char *linkmode = "10/100/1000";
static unsigned int ppm = 0;

module_param(netif, charp, 0444);
MODULE_PARM_DESC(netif, "Bridge- and End Station tunnelling network interface (xMII connectivity only!)");
module_param(resmactc, uint, 0444);
MODULE_PARM_DESC(resmactc, "Traffic Class for Reserved MAC traffic; defaults to traffic class 1");
module_param(syncmode, charp, 0444);
MODULE_PARM_DESC(syncmode, "Time synchronization mode; IEEE 802.1AS (\"1AS\" - default) or IEEE 1588 (\"1588\")");
module_param(csrating, int, 0444);
MODULE_PARM_DESC(csrating, "Clock-source rating; defaults to 0 (lowest rating)");
module_param(linkmode, charp, 0444);
MODULE_PARM_DESC(linkmode, "Link mode defines the set of link speeds to use (\"10/100/1000\" - default) or \"10/100\" or \"100/1000\"");
module_param(ppm, uint, 0444);
MODULE_PARM_DESC(ppm, "PPM; defaults to 25 ppm");

/* list of instantiated bridges */
static LIST_HEAD(br_list);

u16 edgx_get_tc_mgmtraffic(struct edgx_br *br)
{
	return resmactc;
}

struct edgx_br *edgx_dev2br(struct device *dev)
{
	struct list_head *pos;
	struct net_device *netdev = container_of(dev, struct net_device, dev);

	list_for_each(pos, &br_list) {
		struct edgx_br *br = list_entry(pos, struct edgx_br, entry);

		if (br->bridge.dev == netdev)
			return br;
	}
	return NULL;
}

static inline bool _is_memb(ptvec_t vec, ptid_t ptid)
{
	return (PT_IS_BRP_ID(ptid) && (vec & BIT(ptid)));
}

int edgx_br_ageing_set(struct edgx_br *br, clock_t ageing_time)
{
	br->ageing_time = ageing_time;
	return 0;
}

clock_t edgx_br_ageing_get(const struct edgx_br *br)
{
	return br->ageing_time;
}

struct edgx_pt *edgx_br_get_brpt(const struct edgx_br *br, ptid_t ptid)
{
	return ((br) && (ptid < EDGX_BR_MAX_PORTS)) ? br->br_pt[ptid] : NULL;
}

struct edgx_pt *edgx_br_get_eppt(const struct edgx_br *br)
{
	return (br) ? br->ep_pt : NULL;
}

struct edgx_time *edgx_br_get_pt_time(const struct edgx_br *br, ptid_t ptid)
{
	return (br && _is_memb(br->time_map, ptid)) ? br->time : NULL;
}

struct edgx_time *edgx_br_get_br_time(const struct edgx_br *br)
{
	return (br) ? br->time : NULL;
}

struct edgx_stat *edgx_br_get_pt_stat(const struct edgx_br *br, ptid_t ptid)
{
	return (br && _is_memb(br->stat_map, ptid)) ? br->stat : NULL;
}

struct workqueue_struct *edgx_br_get_owq(const struct edgx_br *br)
{
	return (br) ? br->owq : NULL;
}

struct edgx_br_vlan *edgx_br_get_vlan(const struct edgx_br *br)
{
	return (br) ? br->vlan : NULL;
}

struct edgx_brfdb *edgx_br_get_fdb(const struct edgx_br *br)
{
	return (br) ? br->fdb : NULL;
}

struct edgx_com *edgx_br_get_com(const struct edgx_br *br)
{
	return (br) ? br->com : NULL;
}

struct edgx_st_com *edgx_br_get_st_com(const struct edgx_br *br)
{
	return (br) ? br->st_com : NULL;
}

struct edgx_sid_br *edgx_br_get_sid(const struct edgx_br *br)
{
	return (br) ? br->sid : NULL;
}

struct edgx_frer *edgx_br_get_frer(const struct edgx_br *br)
{
	return (br) ? br->frer : NULL;
}

struct edgx_psfp *edgx_br_get_psfp(const struct edgx_br *br)
{
	return (br) ? br->psfp : NULL;
}

const u8 *edgx_br_get_mac(const struct edgx_br *br)
{
	return (br) ? br->hw.base_mac : NULL;
}

u16 edgx_br_get_generic(const struct edgx_br *br, size_t ofs)
{
	return (br) ? edgx_rd16(br->brparam, BR_GX_BASE + ofs) : 0;
}

u32 edgx_br_get_generic_pow2(const struct edgx_br *br, size_t ofs)
{
	return (br) ? BIT(edgx_rd16(br->brparam, BR_GX_BASE + ofs)) : 0;
}

u32 edgx_br_get_generic_pow2z(const struct edgx_br *br, size_t ofs)
{
	u32 value = 0;

	if (br)
		value = edgx_rd16(br->brparam, BR_GX_BASE + ofs);
	return value ? BIT(value) : 0;
}

u16 edgx_br_get_num_ports(const struct edgx_br *br)
{
	/* Provide dedicated function, as we need to add 1. */
	return (edgx_br_get_generic(br, BR_GX_PORT_HIGH) + 1);
}

unsigned int edgx_br_get_cycle_ns(const struct edgx_br *br)
{
	/* Provide dedicated function, FEATURE only delivers frequency. */
	return (br) ? br->cycle_ns : 0;
}

const char *edgx_br_get_drv_name(const struct edgx_br *br)
{
	return (br) ? br->driver : EDGX_SW_CORE_NAME;
}

u32 edgx_br_get_version(const struct edgx_br *br)
{
	return (br) ? edgx_rd32(br->brmgmt, 0x8) : 0;
}

u16 edgx_br_get_feature(const struct edgx_br *br, size_t ofs)
{
	return (br) ? edgx_rd16(br->brparam, BR_FEAT_BASE + ofs) : 0;
}

unsigned int edgx_br_get_id(const struct edgx_br *br)
{
	return (br) ? br->hw.id : -1;
}

struct device *edgx_br_get_dev(const struct edgx_br *br)
{
	return (br) ? br->pdev : NULL;
}

static void edgx_br_sysfs_update(struct edgx_br *br, bool joined)
{
	struct list_head *pos;

	if (br->bridge.refs == 2 && joined)  /* 2nd port just joined */
		list_for_each(pos, &br->grp_list) {
			struct _sysfs_grp *e =
				list_entry(pos, struct _sysfs_grp, list);

			if (sysfs_create_group(&br->bridge.dev->dev.kobj,
					       e->grp))
				edgx_br_err(br, "Cannot create sysfs group '%s'\n",
					    e->grp->name);
		}

	if (br->bridge.refs == 1 && !joined) /* not a bridge anymore */
		list_for_each(pos, &br->grp_list) {
			struct _sysfs_grp *e =
				list_entry(pos, struct _sysfs_grp, list);

			sysfs_remove_group(&br->bridge.dev->dev.kobj, e->grp);
		}
}

int edgx_br_sysfs_add(struct edgx_br *br, struct attribute_group *grp)
{
	struct _sysfs_grp *g;

	if (!br || !grp)
		return -EINVAL;
	g = kzalloc(sizeof(*g), GFP_KERNEL);
	if (!g)
		return -ENOMEM;
	g->grp = grp;
	list_add_tail(&g->list, &br->grp_list);
	return 0;
}

static void edgx_br_sysfs_clear(struct edgx_br *br)
{
	struct list_head *pos;
	struct list_head *tmp;

	list_for_each_safe(pos, tmp, &br->grp_list) {
		struct _sysfs_grp *e = list_entry(pos, struct _sysfs_grp, list);

		if (br->bridge.dev)
			sysfs_remove_group(&br->bridge.dev->dev.kobj, e->grp);
		list_del(pos);
		kfree(e);
	}
}

static void edgx_br_sysfs_init(struct edgx_br *br)
{
	INIT_LIST_HEAD(&br->grp_list);
}

int  edgx_br_pt_join(struct edgx_br *br, struct net_device *netdev)
{
	if (!br->bridge.dev)
		br->bridge.dev = netdev;
	else if (br->bridge.dev != netdev)
		return -EINVAL;
	br->bridge.refs++;
	edgx_br_sysfs_update(br, true);
	return 0;
}

void edgx_br_pt_leave(struct edgx_br *br)
{
	if ((--br->bridge.refs) == 0)
		br->bridge.dev = NULL;
	edgx_br_sysfs_update(br, false);
}

void edgx_br_set_int_mask(struct edgx_br *br, u16 mask)
{
	edgx_wr16(br->brmgmt, EDGX_BR_INT_MASK_SET, mask);
}

void edgx_br_clr_int_mask(struct edgx_br *br, u16 mask)
{
	edgx_wr16(br->brmgmt, EDGX_BR_INT_MASK_CLR, ~mask);
}

u16 edgx_br_get_int_mask(struct edgx_br *br)
{
	return edgx_rd16(br->brmgmt, EDGX_BR_INT_MASK_SET);
}

void edgx_br_clr_int_stat(struct edgx_br *br, u16 stat)
{
	edgx_wr16(br->brmgmt, EDGX_BR_INT_STAT, ~stat);
}

u16 edgx_br_get_int_stat(struct edgx_br *br)
{
	return edgx_rd16(br->brmgmt, EDGX_BR_INT_STAT);
}

void edgx_br_irq_enable(struct edgx_br *br, enum edgx_br_irq_nr irq)
{
	if (br->irq->trig == EDGX_IRQ_LEVEL_TRIG)
		edgx_br_set_int_mask(br, BIT(irq));
}

void edgx_br_irq_disable(struct edgx_br *br, enum edgx_br_irq_nr irq)
{
	if (br->irq->trig == EDGX_IRQ_LEVEL_TRIG)
		edgx_br_clr_int_mask(br, BIT(irq));
}

static irqreturn_t edgx_br_single_isr(int irq, void *device)
{
	u16 intmask, intstat;
	struct edgx_br *br = (struct edgx_br *)device;

	intmask = edgx_br_get_int_mask(br);
	if (!intmask)
		return IRQ_NONE;

	intstat = edgx_br_get_int_stat(br);
	if (!intstat)
		return IRQ_NONE;

	edgx_dbg("ISR m0x%x s0x%x.\n", intmask, intstat);

	if ((intmask & BIT(EDGX_IRQ_NR_TS_TX)) &&
	    (intstat & BIT(EDGX_IRQ_NR_TS_TX)))
		edgx_com_ts_tx_isr(irq, br->com);

	if ((intmask & BIT(EDGX_IRQ_NR_DMA_TX)) &&
	    (intstat & BIT(EDGX_IRQ_NR_DMA_TX)))
		edgx_com_dma_tx_isr(irq, br->com);

	if ((intmask & BIT(EDGX_IRQ_NR_DMA_RX)) &&
	    (intstat & BIT(EDGX_IRQ_NR_DMA_RX)))
		edgx_com_dma_rx_isr(irq, br->com);

	if ((intmask & BIT(EDGX_IRQ_NR_DMA_ERR)) &&
	    (intstat & BIT(EDGX_IRQ_NR_DMA_ERR)))
		edgx_com_dma_err_isr(irq, br->com);

	return IRQ_HANDLED;
}

static int edgx_irq_init(struct edgx_br *br)
{
	int ret;
	u16 intmask;

	if (br->irq->shared) {
		ret = request_irq(br->irq->irq_vec[0], &edgx_br_single_isr,
				  IRQF_SHARED, dev_name(br->pdev), br);
		if (ret) {
			edgx_err("request_irq failed! ret=%d, irq=%d\n",
				 ret, br->irq->irq_vec[0]);
			goto out_err_init_single;
		}
	} else {
		ret = request_irq(br->irq->irq_vec[EDGX_IRQ_NR_TS_TX],
				  &edgx_com_ts_tx_isr,
				  0, dev_name(br->pdev), br->com);
		if (ret) {
			edgx_err("request_irq failed! ret=%d, irq=%d\n",
				 ret, br->irq->irq_vec[EDGX_IRQ_NR_TS_TX]);
			goto out_err_init_ts_tx;
		}

		ret = request_irq(br->irq->irq_vec[EDGX_IRQ_NR_DMA_TX],
				  &edgx_com_dma_tx_isr,
				  0, dev_name(br->pdev), br->com);
		if (ret) {
			edgx_err("request_irq failed! ret=%d, irq=%d\n",
				 ret, br->irq->irq_vec[EDGX_IRQ_NR_DMA_TX]);
			goto out_err_init_dma_tx;
		}

		ret = request_irq(br->irq->irq_vec[EDGX_IRQ_NR_DMA_RX],
				  &edgx_com_dma_rx_isr,
				  0, dev_name(br->pdev), br->com);
		if (ret) {
			edgx_err("request_irq failed! ret=%d, irq=%d\n",
				 ret, br->irq->irq_vec[EDGX_IRQ_NR_DMA_RX]);
			goto out_err_init_dma_rx;
		}

		ret = request_irq(br->irq->irq_vec[EDGX_IRQ_NR_DMA_ERR],
				  &edgx_com_dma_err_isr,
				  0, dev_name(br->pdev), br->com);
		if (ret) {
			edgx_err("request_irq failed! ret=%d, irq=%d\n",
				 ret, br->irq->irq_vec[EDGX_IRQ_NR_DMA_ERR]);
			goto out_err_init_dma_err;
		}
	}

	edgx_br_clr_int_mask(br, ~(u16)0U);
	intmask = BIT(EDGX_IRQ_NR_TS_TX) | BIT(EDGX_IRQ_NR_DMA_TX) |
		  BIT(EDGX_IRQ_NR_DMA_RX) | BIT(EDGX_IRQ_NR_DMA_ERR);
	edgx_br_clr_int_stat(br, intmask);
	edgx_br_set_int_mask(br, intmask);

	return 0;

out_err_init_dma_err:
	free_irq(br->irq->irq_vec[EDGX_IRQ_NR_DMA_RX], br->com);
out_err_init_dma_rx:
	free_irq(br->irq->irq_vec[EDGX_IRQ_NR_DMA_TX], br->com);
out_err_init_dma_tx:
	free_irq(br->irq->irq_vec[EDGX_IRQ_NR_TS_TX], br->com);
out_err_init_ts_tx:
out_err_init_single:
	return ret;
}

static void edgx_irq_shutdown(struct edgx_br *br)
{
	if (br->irq->shared) {
		free_irq(br->irq->irq_vec[0], br);
		return;
	}
	free_irq(br->irq->irq_vec[EDGX_IRQ_NR_TS_TX], br->com);
	free_irq(br->irq->irq_vec[EDGX_IRQ_NR_DMA_TX], br->com);
	free_irq(br->irq->irq_vec[EDGX_IRQ_NR_DMA_RX], br->com);
	free_irq(br->irq->irq_vec[EDGX_IRQ_NR_DMA_ERR], br->com);
}

static int edgx_probe_bridge(struct edgx_br *br)
{
	const struct edgx_ifdesc *ifd;
	const struct edgx_ifreq ifreq_param = {.id = AC_PARAM_ID, .v_maj = 1};
	const struct edgx_ifreq ifreq_mgmt = {.id = AC_MGMT_ID, .v_maj = 3};
	int err;

	ifd = edgx_ac_get_if(&ifreq_param);
	if (!ifd)
		return -ENODEV;

	br->brparam = ifd->iobase;

	ifd = edgx_ac_get_if(&ifreq_mgmt);
	if (!ifd)
		return -ENODEV;

	/*Address: SWITCH General switch configuration registers*/
	br->brmgmt = ifd->iobase;
	/* Trigger Reset @ SWITCH+0x10[15] */
	edgx_set16(br->brmgmt, EDGX_BR_GEN_REG, 15, 15, 1);
	usleep_range(200, 300); /* wait 200us for reset to finish */

	edgx_info("FES version: ID0: 0x%x, ID1: 0x%x, FES_CFG_ID: 0x%x, FES REV ID0: 0x%x, FES REV ID1: 0x%x, FES REV ID2: 0x%x\n",
		  edgx_rd16(br->brmgmt, 0x0),
		  edgx_rd16(br->brmgmt, 0x2),
		  edgx_rd16(br->brmgmt, 0x4),
		  edgx_rd16(br->brmgmt, 0x6),
		  edgx_rd16(br->brmgmt, 0x8),
		  edgx_rd16(br->brmgmt, 0xa)
		  );
	/* Always use 16bit mgmt trailer (COM_TRAILER_LEN) */
	edgx_set16(br->brmgmt, EDGX_BR_GEN_REG, 3, 2, 1);

	/* Enable PTP timestamping */
	edgx_set16(br->brmgmt, EDGX_BR_GEN_REG, 13, 11, 6);

	edgx_info("Initializing brfdb...\n");
	err = edgx_brfdb_init(br, br->brmgmt, &br->fdb);
	if (err)
		goto out_fdb;

	edgx_info("Initializing vlan...\n");
	err = edgx_br_init_vlan(br, br->brmgmt, &br->vlan);
	if (err)
		goto out_vlan;

	return 0;

out_vlan:
	edgx_brfdb_shutdown(br->fdb);
out_fdb:
	return err;
}

static inline ssize_t type_show(struct device *dev,
				struct device_attribute *attr,
				char *buf)
{
	/* C-Vlan Component code = 3
	 * (see 802.1Q, Sect.17.7.2 ieee8021BridgeBaseComponentType)
	 */
	return scnprintf(buf, PAGE_SIZE, "3\n");
}

static inline ssize_t num_ports_show(struct device *dev,
				     struct device_attribute *attr,
				     char *buf)
{
	struct edgx_br *br = edgx_dev2br(dev);
	int i, nports;

	for (i = 0, nports = 0; i < EDGX_BR_MAX_PORTS; i++)
		nports += (br->br_pt[i]) ? 1 : 0;
	return scnprintf(buf, PAGE_SIZE, "%u\n", nports);
}

static inline ssize_t fdb_sz_show(struct device *dev,
				  struct device_attribute *attr,
				  char *buf)
{
	struct edgx_br *br = edgx_dev2br(dev);

	return scnprintf(buf, PAGE_SIZE, "%u\n", edgx_brfdb_sz(br->fdb));
}

static inline ssize_t fdb_smac_sz_show(struct device *dev,
				       struct device_attribute *attr,
				       char *buf)
{
	struct edgx_br *br = edgx_dev2br(dev);

	return scnprintf(buf, PAGE_SIZE, "%u\n", edgx_brfdb_nsmac(br->fdb));
}

struct edgx_delay_item {
	u16		ingress_port;
	u16		egress_port;
	u16		traffic_class;
	u64		indep_min_delay_ns;	// in nanoseconds
	u64		indep_max_delay_ns;	// in nanoseconds
	u32		dep_min_delay_ps;	// in picoseconds
	u32		dep_max_delay_ps;	// in picoseconds
} __packed;

struct edgx_delay_tc {
	struct edgx_delay_item tc_delay[EDGX_BR_MAX_TC];
} __packed;

struct edgx_delays {
	struct edgx_delay_tc pt_delay[EDGX_BR_MAX_PORTS][EDGX_BR_MAX_PORTS - 1];
} __packed;

static int _edgx_br_get_delays(struct edgx_br *br, loff_t idx,
			       struct edgx_delay_item *delays)
{
	u64 rem, help_idx = (u64)idx, help_speed;
	struct edgx_pt *pt_ingress, *pt_egress;
	struct edgx_link *lnk_ingress, *lnk_egress;
	int speed_ingress, speed_egress;

	/* find out ingress port, egress port and traffic class */
	/* delays->ingress_port = idx /
	 * .........(sizeof(struct edgx_delay_tc) * (EDGX_BR_MAX_PORTS - 1));
	 * help_calc = idx %
	 *          (sizeof(struct edgx_delay_tc) * (EDGX_BR_MAX_PORTS - 1));
	 */
	rem = do_div(help_idx,
		     sizeof(struct edgx_delay_tc) * (EDGX_BR_MAX_PORTS - 1));
	delays->ingress_port = (u16)(help_idx + 1);
	/* delays->egress_port = help_calc / (sizeof(struct edgx_delay_tc));
	 * help_calc = help_calc %  (sizeof(struct edgx_delay_tc));
	 */
	help_idx = rem;
	rem = do_div(help_idx, sizeof(struct edgx_delay_tc));
	if ((help_idx + 1) >= delays->ingress_port)
		delays->egress_port = (u16)(help_idx + 2);
	else
		delays->egress_port = (u16)(help_idx + 1);

	/* delays->traffic_class = help_calc / sizeof(struct edgx_delay_item);*/
	help_idx = rem;
	rem = do_div(help_idx, sizeof(struct edgx_delay_item));
	delays->traffic_class = (u16)help_idx;

	/* check if ingress port and egress port are available
	 * in current bridge
	 */
	pt_ingress = br->br_pt[delays->ingress_port - 1];
	pt_egress = br->br_pt[delays->egress_port - 1];
	if (!pt_ingress || !pt_egress)
		return -EINVAL;

	/* check if ports have links where we can get delay values from */
	lnk_ingress = edgx_pt_get_link(pt_ingress);
	lnk_egress = edgx_pt_get_link(pt_egress);
	if (!lnk_ingress || !lnk_egress)
		return -EINVAL;

	/* check if traffic class exists in actual system */
	if (delays->traffic_class >= edgx_br_get_generic(br, BR_GX_QUEUES))
		return -EINVAL;

/* Technology primer says dependent delays are measured in picoseconds.
 * In this implementation the results of functions edgx_pt_get_i2gmin are
 * of type "ktime_t" which is said to be the "Nanosecond scalar
 * representation for kernel time values" in ktime.h
 */
/* The functions edgx_link_get_tx_delay, edgx_link_get_rx_delay already care
 * about speed. We don't need to care about traffic class, because delays
 * are the same for all traffic classes.
 */
	speed_ingress = edgx_link_get_speed(lnk_ingress);
	speed_egress = edgx_link_get_speed(lnk_egress);
	if (speed_ingress != SPEED_UNKNOWN &&
	    speed_egress != SPEED_UNKNOWN) {
		/* calculate independent min delay: tx delay of egress port,
		 * rx delay of ingress port, I_TO_G_MIN and G_TO_O_MIN
		 */
		delays->indep_min_delay_ns =
				edgx_link_get_tx_delay_min(lnk_egress) +
				edgx_link_get_rx_delay_min(lnk_ingress) +
				edgx_pt_get_i2gmin(pt_ingress) +
				edgx_pt_get_g2omin(pt_egress);
		/* calculate independent max delay */
		delays->indep_max_delay_ns =
				edgx_link_get_tx_delay_max(lnk_egress) +
				edgx_link_get_rx_delay_max(lnk_ingress) +
				edgx_pt_get_i2gmax(pt_ingress) +
				edgx_pt_get_g2omax(pt_egress);
		/* calculate dependent min delay: - no dependent delay portion
		 *     on output port
		 * multiply with 8 for 8 bits of a byte
		 * multiply with 1000 to convert from nanoseconds to picoseconds
		 * results in constant delays:
		 * 10 Mbit: 800 000 picos
		 * 100 Mbit: 80 000 picos
		 * 1 Gbit:    8 000 picos
		 */
		help_speed = 1000;
		rem = do_div(help_speed, speed_ingress);
		delays->dep_min_delay_ps = help_speed * 8 * 1000;
		/* calculate dependent max delay: equal to min delay - no
		 * dependent delay portion on output port
		 */
		delays->dep_max_delay_ps = delays->dep_min_delay_ps;
	} else {
		/* ingress or egress speed are unknown ->
		 * return 0 delay values
		 */
		delays->indep_min_delay_ns = 0;
		delays->indep_max_delay_ns = 0;
		delays->dep_min_delay_ps = 0;
		delays->dep_max_delay_ps = 0;
	}

	return 0;
}

static ssize_t delays_read(struct file *filp, struct kobject *kobj,
			   struct bin_attribute *bin_attr, char *buf,
			   loff_t ofs, size_t count)
{
	struct edgx_br *br = edgx_dev2br(kobj_to_dev(kobj));
	loff_t idx;
	struct edgx_delay_item delays;
	int ret;

	ret = edgx_sysfs_tbl_params(ofs, count,
				    sizeof(struct edgx_delay_item), &idx);
	if (ret)
		return ret;

	ret = _edgx_br_get_delays(br, idx, &delays);
	if (ret)
		return ret;

	*((struct edgx_delay_item *)buf) = delays;

	return count;
}

/* Bridge Attributes */
EDGX_DEV_FCT_ATTR_RO(br_type,   type,            "BridgeType");
EDGX_DEV_FCT_ATTR_RO(br_nports, num_ports,       "BridgeNumPorts");
EDGX_DEV_FCT_ATTR_RO(br_comps,  edgx_sysfs_true, "BridgeComponents");
EDGX_DEV_FCT_ATTR_RO(br_tc_en,  edgx_sysfs_true, "BridgeTrafficClassesEnabled");

/* Component Attributes */
EDGX_DEV_FCT_ATTR_RO(comp_id,     edgx_sysfs_one, "ComponentId");
EDGX_DEV_FCT_ATTR_RO(comp_type,   type,           "ComponentType");
EDGX_DEV_FCT_ATTR_RO(comp_nports, num_ports,      "ComponentNumPorts");

/* FDB Attributes */
EDGX_DEV_ATTR_RO(fdb_sz,      "FdbSize");
EDGX_DEV_ATTR_RO(fdb_smac_sz, "FdbStaticEntries");

/* Capability Attributes */
EDGX_DEV_FCT_ATTR_RO(efs,     edgx_sysfs_false, "ExtendedFilteringServices");
EDGX_DEV_FCT_ATTR_RO(tc,      edgx_sysfs_true,  "TrafficClasses");
EDGX_DEV_FCT_ATTR_RO(seip,    edgx_sysfs_false, "StaticEntryIndividualPort");
EDGX_DEV_FCT_ATTR_RO(ivl,     edgx_sysfs_true,  "IVLCapable");
EDGX_DEV_FCT_ATTR_RO(svl,     edgx_sysfs_true,  "SVLCapable");
EDGX_DEV_FCT_ATTR_RO(hybrid,  edgx_sysfs_true,  "HybridCapable");
EDGX_DEV_FCT_ATTR_RO(pvidtag, edgx_sysfs_true,  "ConfigurablePvidTagging");
EDGX_DEV_FCT_ATTR_RO(lvlan,   edgx_sysfs_false, "LocalVlanCapable");

/* Common attributes for bridge and every port */
struct attribute *ieee8021_brpt_common[] = {
	&dev_attr_efs.attr,
	&dev_attr_tc.attr,
	&dev_attr_seip.attr,
	&dev_attr_ivl.attr,
	&dev_attr_svl.attr,
	&dev_attr_hybrid.attr,
	&dev_attr_pvidtag.attr,
	&dev_attr_lvlan.attr,
	NULL,
};

static struct attribute *ieee8021_bridge_attrs[] = {
	/* Bridge Attributes */
	&dev_attr_br_type.attr,
	&dev_attr_br_nports.attr,
	&dev_attr_br_comps.attr,
	&dev_attr_br_tc_en.attr,
	/* Component Attributes */
	&dev_attr_comp_id.attr,
	&dev_attr_comp_type.attr,
	&dev_attr_comp_nports.attr,
	/* FDB Attributes */
	&dev_attr_fdb_sz.attr,
	&dev_attr_fdb_smac_sz.attr,
	/* Capability Attributes */
	&dev_attr_efs.attr,
	&dev_attr_tc.attr,
	&dev_attr_seip.attr,
	&dev_attr_ivl.attr,
	&dev_attr_svl.attr,
	&dev_attr_hybrid.attr,
	&dev_attr_pvidtag.attr,
	&dev_attr_lvlan.attr,
	NULL,
};

EDGX_BIN_ATTR_RO(delays, "delays", EDGX_BR_MAX_PORTS *
		 (EDGX_BR_MAX_PORTS - 1) * EDGX_BR_MAX_TC *
		 sizeof(struct edgx_delays));

static struct bin_attribute *ieee8021_bridge_binattrs[] = {
	&bin_attr_delays,
	NULL,
};

static struct attribute_group ieee8021_bridge_group = {
	.name      = "ieee8021Bridge",
	.attrs     = ieee8021_bridge_attrs,
	.bin_attrs = ieee8021_bridge_binattrs,
};

static void edgx_shutdown_bridge(struct edgx_br *br)
{
	edgx_br_shutdown_vlan(br->vlan);
	edgx_brfdb_shutdown(br->fdb);
}

static int edgx_br_check_lnk_mode(struct edgx_br *br, bool max_2_speeds,
				  u32 *freq_err_abs, u32 *sched_granul)
{
	if (!strncmp(linkmode, "10/100/1000", 11)) {
		if (max_2_speeds) {
			edgx_br_warn(br, "Link mode 10/100/1000 not supported! Switching to 100/1000 link mode.\n");
			br->lnk_mode = EDGX_LNKMOD_100_1000;
		} else {
			br->lnk_mode = EDGX_LNKMOD_10_100_1000;
		}
	} else if (!strncmp(linkmode, "100/1000", 11)) {
		br->lnk_mode = EDGX_LNKMOD_100_1000;
	} else if (!strncmp(linkmode, "10/100", 11)) {
		br->lnk_mode = EDGX_LNKMOD_10_100;
	} else {
		br->lnk_mode = EDGX_LNKMOD_INVAL;
		edgx_br_err(br, "Invalid link mode %s\n", linkmode);
		return -EINVAL;
	}

	if (!edgx_br_get_generic(br, BR_GX_GIGABIT)) {
		if (br->lnk_mode == EDGX_LNKMOD_10_100_1000)
			br->lnk_mode = EDGX_LNKMOD_10_100;
		else if (br->lnk_mode == EDGX_LNKMOD_100_1000)
			br->lnk_mode = EDGX_LNKMOD_100;
	}

	if ((br->lnk_mode == EDGX_LNKMOD_100_1000) ||
	    (br->lnk_mode == EDGX_LNKMOD_100)) {
		*sched_granul = 320;
	} else if (br->lnk_mode == EDGX_LNKMOD_10_100) {
		*sched_granul = 3200;
	} else {
		*sched_granul = 0;
	}

	if (!ppm) {
		*freq_err_abs = 25;
		edgx_br_info(br, "Using default PPM value %u ppm.\n",
			     *freq_err_abs);
	} else if (ppm > 25) {
		edgx_br_err(br, "PPM value %d too high!\n", ppm);
		return -EINVAL;
	} else {
		*freq_err_abs = ppm;
	}

	return 0;
}

int edgx_br_probe_one(unsigned int br_id, struct device *dev,
		      void *base, struct edgx_br_irq *irq,
		      struct edgx_br **br_ret, const char *drv_name)
{
	int ret;
	struct edgx_br *br;
	bool max_2_speeds = false;
	u32 freq_err_abs;
	u32 sched_granul;

	if (!dev)
		return -EINVAL;

	br = kzalloc(sizeof(*br), GFP_KERNEL);
	if (!br)
		return -ENOMEM;

	br->iobase       = base;
	br->pdev         = dev;
	br->bridge.dev   = NULL;
	br->bridge.refs  = 0;
	br->hw.id        = br_id;
	br->irq		 = irq;
	br->driver   = drv_name;

	spin_lock_init(&br->lock);
	eth_random_addr(br->hw.base_mac);
	br->hw.base_mac[ETH_ALEN - 1] = 0;
	br->ageing_time = 300;
	edgx_br_sysfs_init(br);

	ret = edgx_probe_ac(br->iobase, br->pdev);
	if (ret)
		goto out_ac;

	edgx_info("Initializing device 'bridge-%d' on device '%s'...\n",
		  br_id, dev_name(dev));

	br->owq = alloc_ordered_workqueue(dev_name(dev), WQ_MEM_RECLAIM);
	if (!br->owq)
		goto out_owq;

	ret = edgx_probe_bridge(br);
	if (ret)
		goto out_bridge;

	br->cycle_ns = 1000 / edgx_br_get_feature(br, BR_FEAT_CLKFREQ);

	ret = edgx_br_sysfs_add(br, &ieee8021_bridge_group);
	if (ret)
		goto out_sysfs;

	ret = edgx_com_probe(br, netif, &br->com, br->brmgmt);
	if (ret)
		goto out_com;

	ret = edgx_probe_time(br, &br->time, &br->time_map);
	if (ret)
		goto out_time;

	ret = edgx_probe_stat(br, &br->stat, &br->stat_map);
	if (ret)
		goto out_stat;

	ret = edgx_st_com_probe(br, irq, dev_name(dev),
				&br->st_com, &max_2_speeds);
	if (ret && ret != -ENODEV)
		goto out_st_com;

	ret = edgx_br_check_lnk_mode(br, max_2_speeds,
				     &freq_err_abs, &sched_granul);
	if (ret)
		goto out_st_com;

	edgx_st_com_set_params(br->st_com, freq_err_abs, sched_granul);

	if ((resmactc < 0) || (resmactc >= edgx_br_get_generic(br, BR_GX_QUEUES))) {
		edgx_info("Traffic class for reserved MAC traffic out of range: %d,"
				" set to default value %d \n", resmactc, EDGX_BR_DEFAULT_RESMACTC);
		resmactc = EDGX_BR_DEFAULT_RESMACTC;
	}

	ret = edgx_probe_brports(br, br->lnk_mode, br->br_pt);
	if (ret)
		goto out_brports;

	ret = edgx_init_epport(br, br->lnk_mode, &br->ep_pt);
	if (ret)
		goto out_epports;

	ret = edgx_probe_psfp(br, irq, dev_name(dev), &br->psfp);
	if (ret && ret != -ENODEV)
		goto out_psfp;

	ret = edgx_probe_sid(br, br->brmgmt, &br->sid);
	if (ret && ret != -ENODEV)
		goto out_sid;

	ret = edgx_probe_frer(br, &br->frer);
	if (ret && ret != -ENODEV)
		goto out_frer;

	ret = edgx_irq_init(br);
	if (ret)
		goto out_irq;

	ret = edgx_brfdb_init_dmac_sync(br->fdb);
	if (ret)
		goto out_dmac;

	list_add_tail(&br->entry, &br_list);
	*br_ret = br;

	edgx_br_info(br, "Setup Bridge %d ... done\n", br_id);
	return 0;

out_dmac:
	edgx_irq_shutdown(br);
out_irq:
	edgx_shutdown_frer(br->frer);
out_frer:
	edgx_shutdown_sid(br->sid);
out_sid:
	edgx_shutdown_psfp(br->psfp);
out_psfp:
	edgx_shutdown_epport(br->ep_pt);
out_epports:
	edgx_shutdown_brports(br->br_pt);
out_brports:
	edgx_st_com_shutdown(br->st_com);
out_st_com:
	edgx_shutdown_stat(br->stat);
out_stat:
	edgx_shutdown_time(br->time);
out_time:
	edgx_com_shutdown(br->com);
out_com:
	edgx_br_sysfs_clear(br);
out_sysfs:
	edgx_shutdown_bridge(br);
out_bridge:
	kfree(br->owq);
out_owq:
	edgx_shutdown_ac();
out_ac:
	kfree(br);
	return ret;
}

void edgx_br_shutdown(struct edgx_br *br)
{
	if (!br)
		return;

	edgx_brfdb_shutdown_dmac_sync(br->fdb);
	edgx_irq_shutdown(br);
	edgx_shutdown_frer(br->frer);
	edgx_shutdown_sid(br->sid);
	edgx_shutdown_psfp(br->psfp);
	edgx_shutdown_epport(br->ep_pt);
	edgx_shutdown_brports(br->br_pt);
	edgx_st_com_shutdown(br->st_com);
	edgx_shutdown_stat(br->stat);
	edgx_shutdown_time(br->time);
	edgx_com_shutdown(br->com);
	edgx_shutdown_bridge(br);

	edgx_br_sysfs_clear(br);
	list_del(&br->entry);

	edgx_br_info(br, "Shutdown Bridge %d ... done\n", br->hw.id);

	edgx_shutdown_ac();
	kfree(br);
}

void *edgx_br_get_base(struct edgx_br *br)
{
	if (!br)
		return NULL;

	return br->iobase;
}
