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

#include <linux/jiffies.h>
#include <linux/workqueue.h>

#include "edge_ac.h"
#include "edge_port.h"
#include "edge_stat.h"
#include "edge_util.h"

struct _edgx_ptstat {
	edgx_io_t    *base;
	/* Protect the control register for the respective port's delta
	 * counter area.
	 */
	spinlock_t   lock;
};

struct edgx_stat {
	const struct edgx_br *parent;
	struct _edgx_ptstat   ptstat[EDGX_BR_MAX_PORTS];
};

static u8 edgx_stat_feat_offset[EDGX_STAT_FEAT_MAX - EDGX_STAT_FEAT_MIN] = {
	0u,
	44u,
	28u,
	36u,
	48u,
};

/* Mask for selector bits in counter control register (bits 8-12).*/
#define _STAT_SEL_MASK  0x1F00
#define _STAT_CMD_REG	0x2
#define _STAT_CNT_LO	0x8
#define _STAT_CNT_HI	0xA
#define _STAT_CMD_READ  (1<<15)

struct edgx_stat_hdl {
	struct edgx_stat           *parent;
	const struct edgx_statinfo *info;
	ptid_t                      ptid;
	/* We need to protect:
	 *  1. user-invoked against rate-invoked access (and vice versa)
	 *  2. read-access from client against (rate-invoked) update
	 *
	 *  In other words: this causes quite some locking effort, because
	 *  every access to the accumulation array needs to be protected.
	 */
	spinlock_t                 lock;
	statw_t                    *data;
	struct delayed_work         update;
};

static inline struct _edgx_ptstat *__edgx_stat_get_pts(struct edgx_stat *sm,
						       ptid_t ptid)
{
	return &sm->ptstat[ptid];
}

static int __edgx_stat_update(struct edgx_stat_hdl *sh, bool wait)
{
	int di; /* di: data index (in SW) */
	u8 *cnt_base;
	u16 trigger, reg;
	struct _edgx_ptstat *pts;

	trigger = ((~BIT(sh->info->feat_id)) & _STAT_SEL_MASK) | BIT(0);

	if (!spin_trylock_bh(&sh->lock)) {
		if (wait) {
			/* we have to wait (spin_lock_bh blocks), but since an
			 * update was just in progress, we also release the lock
			 * immediately again as data is just fresh.
			 * (used by user-invoked update)
			 */
			spin_lock_bh(&sh->lock);
			spin_unlock_bh(&sh->lock);
		}
		/* bail out, other side is just updating
		 * (used by status worker)
		 */
		return 0;
	}

	pts = __edgx_stat_get_pts(sh->parent, sh->ptid);

	/* Acquire lock on PTS, making sure that no other stat-hdl is currently
	 * controlling its portion of the counters
	 */
	spin_lock_bh(&pts->lock);

	edgx_wr16(pts->base, 0, trigger);

	cnt_base = &edgx_stat_feat_offset[
			sh->info->feat_id - EDGX_STAT_FEAT_MIN];
	/* Locking takes 300 clock cycles * port count.
	 * We just take 40us so that we are sure to cover 10ns clock at
	 * 12 ports (36us).
	 * Upper limit is just a guess.
	 */
	udelay(40);

	/* get all delta counters and increment corresponding
	 * status words in buffer.
	 */
	for (di = *cnt_base; di < (sh->info->nwords + *cnt_base); di++) {
		edgx_wr16(pts->base, _STAT_CMD_REG, (di | _STAT_CMD_READ));
		/* Wait until flags are cleared again by chip. */
		do {
			reg = edgx_get16(pts->base, _STAT_CMD_REG, 15, 15);
		} while (reg);
		sh->data[di - *cnt_base] += edgx_rd32(pts->base, _STAT_CNT_LO);
	}
	spin_unlock_bh(&pts->lock);
	spin_unlock_bh(&sh->lock);

	return 0;
}

int edgx_stat_update(struct edgx_stat_hdl *sh)
{
	int r;

	if (!sh)
		return -EINVAL;

	/* Wait if update (2nd parameter) is in progress ensure fresh data */
	r = __edgx_stat_update(sh, true);

	/* We just did an update so we try to cancel the delayed work if
	 * possible and reschedule it at the next rate.
	 */
	if (cancel_delayed_work(&sh->update))
		schedule_delayed_work(&sh->update,
				      msecs_to_jiffies(sh->info->rate_ms));
	return r;
}

static void edgx_stat_update_worker(struct work_struct *work)
{
	struct delayed_work *delay = to_delayed_work(work);
	struct edgx_stat_hdl *sh   = container_of(delay, struct edgx_stat_hdl,
						  update);

	/* Don't wait on update if it's just ongoing */
	__edgx_stat_update(sh, false);
	schedule_delayed_work(&sh->update,
			      msecs_to_jiffies(sh->info->rate_ms));
}

statw_t edgx_stat_get(struct edgx_stat_hdl *sh, size_t idx)
{
	statw_t r = 0;

	if (!sh || idx > sh->info->nwords)
		return r; /* default: 0 */

	/* We need to lock access to data area while reading, since otherwise
	 * an update may write the respective status word and we cannot
	 * really assume that 64bit read access is atomic.
	 * TODO: making sh->data volatile and doing double-reading was another
	 *   option, but Documentation/volatile-considered-harmful.txt
	 *   and style checker reject this idea.
	 */
	spin_lock_bh(&sh->lock);
	r = sh->data[idx];
	spin_unlock_bh(&sh->lock);

	return r;
}

struct edgx_stat_hdl *edgx_stat_alloc_hdl(struct edgx_stat *sm, ptid_t ptid,
					  const struct edgx_statinfo *info)
{
	struct edgx_stat_hdl *sh = NULL;

	if (!sm || !info || !PT_IS_BRP_ID(ptid))
		return NULL;
	if (info->feat_id >= EDGX_STAT_FEAT_MAX || info->feat_id < 0)
		return NULL;
	sh = kzalloc(sizeof(*sh), GFP_KERNEL);
	if (!sh)
		return NULL;

	sh->data = kcalloc(info->nwords, sizeof(*sh->data), GFP_KERNEL);
	if (!sh->data)
		goto out_free;

	sh->parent = sm;
	sh->info = info;
	sh->ptid = ptid;
	spin_lock_init(&sh->lock);

	/* TODO: Use dedicated work-queue or stay on system_wq? */
	INIT_DEFERRABLE_WORK(&sh->update, edgx_stat_update_worker);
	schedule_delayed_work(&sh->update, msecs_to_jiffies(info->rate_ms));

	return sh;

out_free:
	kfree(sh);
	return NULL;
}

void edgx_stat_free_hdl(struct edgx_stat_hdl *sh)
{
	if (sh) {
		cancel_delayed_work_sync(&sh->update);
		kfree(sh->data);
		kfree(sh);
	}
}

int edgx_probe_stat(const struct edgx_br *br, struct edgx_stat **pstat,
		    ptvec_t *map)
{
	const struct edgx_ifdesc *ifd;
	struct edgx_ifdesc        pifd;
	ptid_t                    ptid;
	struct edgx_ifreq         ifreq = { .id = AC_STAT_ID, .v_maj = 2 };

	if (!br || !pstat)
		return -EINVAL;

	ifd = edgx_ac_get_if(&ifreq);
	if (!ifd)
		return -ENODEV;

	*pstat = kzalloc(sizeof(**pstat), GFP_KERNEL);
	if (!(*pstat))
		return -ENOMEM;

	(*pstat)->parent = br;
	*map             = ifd->ptmap;

	edgx_ac_for_each_ifpt(ptid, ifd, &pifd) {
		struct _edgx_ptstat *pts = __edgx_stat_get_pts((*pstat), ptid);

		pts->base = pifd.iobase;
		spin_lock_init(&pts->lock);
	}

	edgx_br_info(br, "Setup Status Manager ... done");
	return 0;
}

void edgx_shutdown_stat(struct edgx_stat *stat)
{
	kfree(stat);
}
