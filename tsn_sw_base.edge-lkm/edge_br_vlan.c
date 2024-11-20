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

#include <linux/if_bridge.h>
#include <linux/if_vlan.h>
#include <linux/version.h>

#include "edge_br_vlan.h"
#include "edge_br_fdb.h"
#include "edge_util.h"

struct _msti {
	mstid_t          mstid;
	struct list_head fidlist;
	u8               ptstate[EDGX_BR_MAX_PORTS];
};

struct _fid {
	struct list_head  vlanlist;
	struct list_head  entry;
	fid_t             fid;
	struct _msti     *msti;
};

struct _vlan {
	struct list_head entry;
	vid_t            vid;
	struct _fid     *fid;
	ptvec_t          ptvec;
};

struct edgx_br_vlan {
	struct edgx_br *parent;
	edgx_io_t      *iobase;
	u16             nvlans;
	u16             max_vlans;
	u16             max_fids;
	u16             max_mstis;
	struct _vlan   *vid;
	struct _fid    *fid;
	struct _msti   *msti;

	/* vid2fid (v2f) lock protects the VID-to-FID allocation */
	struct mutex    v2f_lock; /* vid2fid */
	/* fid2msti (f2m) lock protects the FID-to-MSTI allocation */
	struct mutex    f2m_lock; /* fid2msti */
	/* register lock protects the HW configuration registers */
	struct mutex    reg_lock; /* register lock */

	/* Locks are usually used to protect lists during the various
	 * processings, most notably moving of a VID to another FID or moving
	 * of an FID to another MSTI as typically actions on all list elements
	 * are involved in the semantic action (e.g., clear FID from a single
	 * or all ports from the dynamic FDB).
	 * A dedicated lock for the HW register set is needed, because semantic
	 * actions that do not need protection from each other (concerning the
	 * data structures), i.e., utilizing v2f- and f2m-lock, still target
	 * the same register set.
	 */
};

/* See 802.1Q, Table 9-2: null VID must not be used */
#define _INVALID_VID   ((vid_t)0)

/* In order to comply to the MIB/YANG model rather, we enumerate FIDs from
 * 0 through N. Note that this is not exactly in line with 802.1Q, Sect. 8.8.8
 * where it is stated that FIDs are numbered starting at 1.
 */

#define _VLAN_N_FID    (64)
#define _VLAN_PER_FID  (64)

#define _CIST_IDX      (0)
#define _TE_IDX        (1)

/* Register offsets are relative to the FES Switch Configuration Registers
 * base.
 * This is because we need registers from different sub-parts, i.e.,
 * -) 'SWITCH' for MAC table (MT) manipulation registers, and
 * -) 'VLAN' for VLAN manipulation registers.
 */

#define EDGX_BR_VLAN_BASE		(0x8000)
#define EDGX_BR_VLAN_CMD_BASE	(EDGX_BR_VLAN_BASE + 0x0)

#define EDGX_BR_VLAN_CMD_READ	(0<<12)
#define EDGX_BR_VLAN_CMD_WRITE	(1<<12)

#define EDGX_BR_VLAN_CMD_PORTS	(1<<13)
#define EDGX_BR_VLAN_CMD_TAG	(1<<14)
#define EDGX_BR_VLAN_CMD_FID	(1<<15)

#define EDGX_BR_VLAN_PORTS_BASE	(EDGX_BR_VLAN_CMD_BASE + 0x8)

#define EDGX_BR_VLAN_TAG_BASE	(EDGX_BR_VLAN_CMD_BASE + 0xA)

#define EDGX_BR_VLAN_FID_BASE	(EDGX_BR_VLAN_CMD_BASE + 0xC)

#define _ENGINEERED_TRAFFIC(boolean)	(((boolean) ? (0x1) : (0x0)) << 15)

#define _MTPORTS_REG      0x12
#define _MTPORTS_ALL      0xFFFF
#define _MTFID_REG        0x14

static inline struct edgx_br_vlan *edgx_dev2brvlan(struct device *dev)
{
	return edgx_br_get_vlan(edgx_dev2br(dev));
}

static inline bool _is_reserved_mstid(mstid_t mstid)
{
	switch (mstid) {
	case EDGX_CIST_MSTID:
	case EDGX_TE_MSTID:
	case EDGX_SPBM_MSTID:
	case EDGX_SPBV_MSTID:
	case EDGX_INVALID_MSTID:
		return true;
	}
	return false;
}

static struct _vlan *_edgx_br_vlan_get_vlan(struct edgx_br_vlan *brvlan,
					    vid_t vid)
{
	int i;

	for (i = 0; i < brvlan->max_vlans; i++)
		if (brvlan->vid[i].vid == vid)
			return &brvlan->vid[i];
	return NULL;
}

#define _edgx_br_vlan_get_free_vlan(brvlan) \
				_edgx_br_vlan_get_vlan(brvlan, _INVALID_VID)

static struct _fid *_edgx_br_vlan_get_fid(struct edgx_br_vlan *brvlan,
					  fid_t fid)
{
	int i;

	for (i = 0; i < brvlan->max_fids; i++)
		if (brvlan->fid[i].fid == fid)
			return &brvlan->fid[i];
	return NULL;
}

static struct _fid *_edgx_br_vlan_get_free_fid(struct edgx_br_vlan *brvlan)
{
	int i;

	for (i = 0; i < brvlan->max_fids; i++)
		if (list_empty(&brvlan->fid[i].vlanlist))
			return &brvlan->fid[i];
	return NULL;
}

static struct _msti *_edgx_br_vlan_get_msti(struct edgx_br_vlan *brvlan,
					    mstid_t mstid)
{
	int i;

	for (i = 0; i < brvlan->max_mstis; i++)
		if (brvlan->msti[i].mstid == mstid)
			return &brvlan->msti[i];
	return NULL;
}

static inline void _edgx_br_vlan_wait_cmd(struct edgx_br_vlan *brvlan)
{
	u16 reg;

	/* Sleep and then wait until flags are cleared again by chip. */
	/* Should be aligned with HW */
	usleep_range(300, 400);
	do {
		reg = edgx_get16(brvlan->iobase,
				 EDGX_BR_VLAN_CMD_BASE, 15, 13);
	} while (reg);
}

#define _edgx_br_vlan_get_free_msti(_brvlan) \
			_edgx_br_vlan_get_msti(_brvlan, EDGX_INVALID_MSTID)

static void _edgx_br_vlan_flush_fid(struct edgx_br_vlan *brvlan, fid_t fid,
				    ptvec_t ptvec)
{
	u16 reg;

	mutex_lock(&brvlan->reg_lock);

	/* Prepare what exactly to clear (FID on given port vector) ... */
	edgx_wr16(brvlan->iobase, _MTFID_REG, fid);
	edgx_wr16(brvlan->iobase, _MTPORTS_REG, ptvec);
	/* and trigger chip action. */
	edgx_set16(brvlan->iobase, EDGX_BR_GEN_REG, 14, 14, 1);
	/* Sleep and then wait until flag is cleared again by chip. */
	/* Should be aligned with HW */
	usleep_range(300, 400);
	do {
		reg = edgx_get16(brvlan->iobase, EDGX_BR_GEN_REG, 14, 14);
	} while (reg);

	mutex_unlock(&brvlan->reg_lock);
}

static void _edgx_br_vlan_set_te_fid(struct edgx_br_vlan *brvlan,
				     struct _fid *fid, bool te)
{
	struct _vlan *vlan;
	struct edgx_pt *pt = NULL;
	int i;

	mutex_lock(&brvlan->reg_lock);
	list_for_each_entry(vlan, &fid->vlanlist, entry) {
		edgx_wr16(brvlan->iobase, EDGX_BR_VLAN_FID_BASE,
				(fid->fid | _ENGINEERED_TRAFFIC(te)));
		edgx_wr16(brvlan->iobase, EDGX_BR_VLAN_CMD_BASE,
				(vlan->vid | EDGX_BR_VLAN_CMD_WRITE
					   | EDGX_BR_VLAN_CMD_FID));
		_edgx_br_vlan_wait_cmd(brvlan);
	}
	mutex_unlock(&brvlan->reg_lock);
	if (te)
		for (i = 0; i < EDGX_BR_MAX_PORTS; i++) {
			pt = edgx_br_get_brpt(brvlan->parent, i);
			if (pt != NULL)
				edgx_pt_set_fid_fwd_state(pt, fid->fid,
							  BR_STATE_FORWARDING);
		}
}

static inline void _edgx_br_move_fid2msti(struct edgx_br_vlan *brvlan,
					  struct _fid *fid, struct _msti *msti)
{
	if (fid->msti == msti)
		return;
	if (fid->msti)
		list_del(&fid->entry);
	fid->msti = msti;
	list_add_tail(&fid->entry, &msti->fidlist);
	if (msti->mstid == EDGX_TE_MSTID) {
		/* When moving an FID to the TE-MSTI, we flush all dynamic
		 * entries (all ports), since learning is prohibited on this
		 * MSTI according to 802.1Qcc
		 */
		_edgx_br_vlan_flush_fid(brvlan, fid->fid, PT_VEC_ALL);
		_edgx_br_vlan_set_te_fid(brvlan, fid, true);
	} else {
		/* Moving an FID to a "learnable" tree, i.e., not TE-MSTI we
		 * don't flush, since SVL/IVL doesn't change with MSTI
		 * allocation. We just have to make sure, that the FIDs in
		 * the new tree are "learnable" too.
		 */
		_edgx_br_vlan_set_te_fid(brvlan, fid, false);
	}
}

static int _edgx_br_set_fid2msti(struct edgx_br_vlan *brvlan, fid_t fid,
				 mstid_t mstid)
{
	struct _msti *msti;

	if (fid >= brvlan->max_vlans)
		return -EINVAL;

	mutex_lock(&brvlan->f2m_lock);
	msti = _edgx_br_vlan_get_msti(brvlan, mstid);
	if (msti)
		_edgx_br_move_fid2msti(brvlan, &brvlan->fid[fid], msti);
	mutex_unlock(&brvlan->f2m_lock);

	return (msti) ? 0 : -EINVAL;
}

static inline void _edgx_br_move_vid2fid(struct edgx_br_vlan *brvlan,
					 struct _vlan *vlan, struct _fid *fid,
					 bool flush_fid)
{
	/* Call this function only when holding v2f lock! */
	if (vlan->fid == fid)
		return;
	if (vlan->fid)
		list_del(&vlan->entry);
	/* Note: Even when no VIDs are allocated to an FID anymore, we still
	 *       keep the allocation of the FID to the MSTI as the members
	 *       of the respective FID have no impact on this.
	 */
	list_add_tail(&vlan->entry, &fid->vlanlist);

	/* FIDs are counted from 1, but chip counts from 0, so we need to
	 * decrement the given FID by one.
	 */
	mutex_lock(&brvlan->reg_lock);
	/* Read VLAN_FID first */
	edgx_wr16(brvlan->iobase, EDGX_BR_VLAN_CMD_BASE,
		  (vlan->vid | EDGX_BR_VLAN_CMD_READ | EDGX_BR_VLAN_CMD_FID));
	_edgx_br_vlan_wait_cmd(brvlan);

	/* Set the new fid to previously adquired VLAN_FID register */
	edgx_set16(brvlan->iobase, EDGX_BR_VLAN_FID_BASE, 5, 0, fid->fid);
	edgx_wr16(brvlan->iobase, EDGX_BR_VLAN_CMD_BASE,
		  (vlan->vid | EDGX_BR_VLAN_CMD_WRITE | EDGX_BR_VLAN_CMD_FID));
	_edgx_br_vlan_wait_cmd(brvlan);
	mutex_unlock(&brvlan->reg_lock);
	if (flush_fid && vlan->fid)
		_edgx_br_vlan_flush_fid(brvlan, vlan->fid->fid, PT_VEC_ALL);
	if (vlan->fid)
		edgx_brfdb_update_vid2fid(edgx_br_get_fdb(brvlan->parent),
					  vlan->vid, fid->fid, vlan->fid->fid);
	vlan->fid = fid;
}

static int _edgx_br_set_vid2fid(struct edgx_br_vlan *brvlan, vid_t vid,
				fid_t fid)
{
	struct _vlan *vlan;

	if (!brvlan || fid >= brvlan->max_fids)
		return -EINVAL;

	mutex_lock(&brvlan->v2f_lock);
	vlan = _edgx_br_vlan_get_vlan(brvlan, vid);
	if (vlan)
		/* It is not clear from 802.1Q if this operation also requires
		 * flushing the FID the VID is allocated to. The decision here
		 * is to flush it, because otherwise *all* entries learned on
		 * this VID on the previous FID would still be used for
		 * forwarding until they age out, although the VID was just
		 * removed.
		 */
		_edgx_br_move_vid2fid(brvlan, vlan, &brvlan->fid[fid], true);
	mutex_unlock(&brvlan->v2f_lock);

	return (vlan) ? 0 : -EINVAL;
}

static int _edgx_br_get_vid2fid(struct edgx_br_vlan *brvlan, vid_t vid,
				fid_t *fid)
{
	struct _vlan *vlan;

	if (!fid)
		return -EINVAL;

	mutex_lock(&brvlan->v2f_lock);
	vlan = _edgx_br_vlan_get_vlan(brvlan, vid);
	if (vlan)
		*fid = vlan->fid->fid;
	mutex_unlock(&brvlan->v2f_lock);

	return (vlan) ? 0 : -EINVAL;
}

static void _edgx_br_vlan_init_msti(struct _msti *msti, mstid_t mstid, u8 state)
{
	int i;

	msti->mstid = mstid;
	for (i = 0; i < EDGX_BR_MAX_PORTS; i++)
		msti->ptstate[i] = state;
}

static int _edgx_br_vlan_create_msti(struct edgx_br_vlan *brvlan, mstid_t mstid)
{
	struct _msti *msti;

	if (_is_reserved_mstid(mstid) || _edgx_br_vlan_get_msti(brvlan, mstid))
		return -EEXIST;
	if (mstid >= EDGX_MAX_MSTID)
		return -EINVAL;
	mutex_lock(&brvlan->f2m_lock);
	msti = _edgx_br_vlan_get_free_msti(brvlan);
	if (msti)
		/* When creating an MSTI we just set everything to DISABLED;
		 * MSTP needs to sort out state changes.
		 */
		_edgx_br_vlan_init_msti(msti, mstid, BR_STATE_DISABLED);
	mutex_unlock(&brvlan->f2m_lock);
	return (msti) ? 0 : -ENOMEM;
}

static int _edgx_br_vlan_delete_msti(struct edgx_br_vlan *brvlan, mstid_t mstid)
{
	struct _msti *msti;
	int r = -EBUSY;

	if (_is_reserved_mstid(mstid))
		return -EACCES;

	mutex_lock(&brvlan->f2m_lock);
	msti = _edgx_br_vlan_get_msti(brvlan, mstid);
	if (msti && list_empty(&msti->fidlist)) {
		msti->mstid = EDGX_INVALID_MSTID;
		r = 0;
	}
	mutex_unlock(&brvlan->f2m_lock);
	return r;
}

int edgx_br_vlan_flush_mstpt(struct edgx_br_vlan *brvlan, mstid_t mstid,
			     ptid_t ptid)
{
	struct _msti *msti;
	struct _fid *fid;

	if (!brvlan)
		return -EINVAL;

	mutex_lock(&brvlan->f2m_lock);
	msti = _edgx_br_vlan_get_msti(brvlan, mstid);
	if (!msti) {
		mutex_unlock(&brvlan->f2m_lock);
		return -EINVAL;
	}

	/* Flushing a MST-port (mstpt) means to flush the port's dynamic FDB
	 * entries in all FIDs allocated to the MSTI the MST port is part of
	 */
	list_for_each_entry(fid, &msti->fidlist, entry)
		_edgx_br_vlan_flush_fid(brvlan, fid->fid, PT_ID2VEC(ptid));

	mutex_unlock(&brvlan->f2m_lock);
	return 0;
}

int edgx_br_vlan_get_mstpt_state(struct edgx_br_vlan *brvlan, mstid_t mstid,
				 ptid_t ptid, u8 *ptstate)
{
	struct _msti *msti;

	if (!brvlan || !ptstate)
		goto out_param;

	mutex_lock(&brvlan->f2m_lock);
	msti = _edgx_br_vlan_get_msti(brvlan, mstid);
	if (!msti)
		goto out_msti;

	*ptstate = msti->ptstate[ptid];

	mutex_unlock(&brvlan->f2m_lock);
	return 0;

out_msti:
	mutex_unlock(&brvlan->f2m_lock);
out_param:
	return -EINVAL;
}

int edgx_br_vlan_set_mstpt_state(struct edgx_br_vlan *brvlan, mstid_t mstid,
				 struct edgx_pt *pt, u8 ptstate)
{
	struct _msti *msti;
	struct _fid *fid;

	if (!brvlan)
		goto out_param;

	mutex_lock(&brvlan->f2m_lock);
	msti = _edgx_br_vlan_get_msti(brvlan, mstid);
	if (!msti)
		goto out_msti;

	list_for_each_entry(fid, &msti->fidlist, entry)
		edgx_pt_set_fid_fwd_state(pt, fid->fid, ptstate);
	msti->ptstate[edgx_pt_get_id(pt)] = ptstate;

	mutex_unlock(&brvlan->f2m_lock);
	return 0;

out_msti:
	mutex_unlock(&brvlan->f2m_lock);
out_param:
	return -EINVAL;
}

int edgx_br_vlan_add_pt(struct edgx_br_vlan *brvlan,
			struct switchdev_obj_port_vlan *v, struct edgx_pt *pt)
{
	struct _vlan *vlan;
	struct _fid  *fid;
	u16           vid;
	bool          untagged = v->flags & BRIDGE_VLAN_INFO_UNTAGGED;
	bool          pvid     = v->flags & BRIDGE_VLAN_INFO_PVID;
	ptid_t        ptid     = edgx_pt_get_id(pt);
	u8            fidfwd;

	if (!brvlan)
		return -EINVAL;
	if (brvlan->nvlans >= brvlan->max_vlans)
		return -ENOMEM;

	/* TODO: 'bridge vlan add pvid' (without 'untagged') causes traceback
	 * set CONFIG_BUG=n to omit.
	 */
	if (pvid && !untagged)
		/* Note: -EOPNOTSUPP causes software-bridge to add vlan! */
		return -EINVAL;

	mutex_lock(&brvlan->v2f_lock);

	vid = v->vid;
	vlan = _edgx_br_vlan_get_vlan(brvlan, vid);
	if (vlan) {
		vlan->ptvec |= BIT(ptid);
		fid = vlan->fid;

		/* Re-adding the current PVID as non-PVID leaves the
		 * port without any PVID. Not that this is equivalent
		 * to "admit only tagged frames"
		 */
		if (vid == edgx_pt_get_pvid(pt) && !pvid)
			edgx_pt_clear_pvid(pt);
	} else {
		vlan = _edgx_br_vlan_get_free_vlan(brvlan);
		fid  = _edgx_br_vlan_get_free_fid(brvlan);
		if (vlan && fid) {
			vlan->vid = vid;
			vlan->ptvec = BIT(ptid);
			/* Allocate (new) FID to CIST first, so that no
			 * unintended MSTP actions happen.
			 */
			_edgx_br_set_fid2msti(brvlan, fid->fid,
					      EDGX_CIST_MSTID);
			/* ... then allocate VID to (new) FID. */
			_edgx_br_move_vid2fid(brvlan, vlan, fid, false);
			brvlan->nvlans++;
		} else {
			goto out_no_vid;
		}
	}
	mutex_lock(&brvlan->reg_lock);
	/* Read the values from VLAN_TAG for current vid */
	edgx_wr16(brvlan->iobase, EDGX_BR_VLAN_CMD_BASE, (vlan->vid
		  | EDGX_BR_VLAN_CMD_READ | EDGX_BR_VLAN_CMD_TAG));
	_edgx_br_vlan_wait_cmd(brvlan);
	edgx_wr16(brvlan->iobase,
		  EDGX_BR_VLAN_PORTS_BASE, vlan->ptvec);
	/* Set the bits on the previously adquired VLAN_TAG register */
	edgx_set16(brvlan->iobase, EDGX_BR_VLAN_TAG_BASE,
		   ptid, ptid, untagged);
	edgx_wr16(brvlan->iobase, EDGX_BR_VLAN_CMD_BASE,
		  (vlan->vid | EDGX_BR_VLAN_CMD_WRITE
			     | EDGX_BR_VLAN_CMD_PORTS
			     | EDGX_BR_VLAN_CMD_TAG));
	_edgx_br_vlan_wait_cmd(brvlan);
	mutex_unlock(&brvlan->reg_lock);

	/* If this FID was already set to TE-MSTI, */
	/* do not change the port fw state */
	if (fid->msti->mstid != EDGX_TE_MSTID) {
		fidfwd = brvlan->msti[_CIST_IDX]
				.ptstate[edgx_pt_get_id(pt)];
		edgx_pt_set_fid_fwd_state(pt, fid->fid, fidfwd);
	}

	if (pvid)
		edgx_pt_set_pvid(pt, vid);

	mutex_unlock(&brvlan->v2f_lock);
	return 0;

out_no_vid:
	mutex_unlock(&brvlan->v2f_lock);
	return -ENOMEM;
}

int edgx_br_vlan_purge_pt(struct edgx_br_vlan *brvlan, struct edgx_pt *pt)
{
	int i;
	ptvec_t ptmask = BIT(edgx_pt_get_id(pt));
	struct switchdev_obj_port_vlan v;

	if (!brvlan)
		return -EINVAL;

	for (i = 0; i < brvlan->max_vlans; i++) {
		struct _vlan *vlan = &brvlan->vid[i];

		v.vid = vlan->vid;
		if (vlan->ptvec & ptmask)
			edgx_br_vlan_del_pt(brvlan, &v, pt);
	}
	return 0;
}

int edgx_br_vlan_del_pt(struct edgx_br_vlan *brvlan,
			struct switchdev_obj_port_vlan *v, struct edgx_pt *pt)
{
	struct _vlan *vlan = NULL;
	u16           vid;
	ptid_t        ptid = edgx_pt_get_id(pt);

	if (!brvlan)
		return -EINVAL;

	mutex_lock(&brvlan->v2f_lock);

	vid = v->vid;
	vlan = _edgx_br_vlan_get_vlan(brvlan, vid);
	if (vlan) {
		vlan->ptvec &= (~BIT(ptid));
		mutex_lock(&brvlan->reg_lock);
		edgx_wr16(brvlan->iobase, EDGX_BR_VLAN_PORTS_BASE,
			  vlan->ptvec);
		edgx_wr16(brvlan->iobase, EDGX_BR_VLAN_CMD_BASE,
			  (vlan->vid | EDGX_BR_VLAN_CMD_WRITE
				     | EDGX_BR_VLAN_CMD_PORTS));
		_edgx_br_vlan_wait_cmd(brvlan);
		mutex_unlock(&brvlan->reg_lock);
		if (!vlan->ptvec) {
			vlan->vid = _INVALID_VID;
			vlan->fid = NULL;
			list_del(&vlan->entry);
			brvlan->nvlans--;
		}
		/* We don't need to update the tagging information,
		 * since the port is not eligible anymore for vlan->vid
		 * tagged traffic.
		 *
		 * It is not clear from 802.1Q if this operation also
		 * requires flushing the FID the VID is allocated
		 * to. The decision here is not to flush it, because
		 * this would trigger a potentially high number of
		 * flooded frames until all end stations with VID
		 * allocated to this FID are re-learned. On the other
		 * hand the only disadvantage of this strategy is a
		 * single dynamic FDB entry that will eventually age
		 * out. Furthermore, since the VID was removed from the
		 * port, frames with the VID are anyway not accepted
		 * anymore on this port.
		 */

		if (vid == edgx_pt_get_pvid(pt))
			edgx_pt_clear_pvid(pt);
	}

	mutex_unlock(&brvlan->v2f_lock);
	return (vlan) ? 0 : -EINVAL;
}

void edgx_br_vlan_lock(struct edgx_br_vlan *brvlan)
{
	mutex_lock(&brvlan->v2f_lock);
}

void edgx_br_vlan_unlock(struct edgx_br_vlan *brvlan)
{
	mutex_unlock(&brvlan->v2f_lock);
}

fid_iter edgx_br_vlan_iter_fid_vlans(struct edgx_br_vlan *brvlan, fid_t fidno,
				     vlan_iter *vlan_iter)
{
	struct _fid *fid = _edgx_br_vlan_get_fid(brvlan, fidno);

	if (fid)
		*vlan_iter = list_first_entry_or_null(&fid->vlanlist,
						      struct _vlan, entry);
	else
		*vlan_iter = NULL;
	return fid;
}

vid_t edgx_br_vlan_get_fid_vlan(fid_iter fid, vlan_iter *vlan_iter)
{
	vid_t vid = (*vlan_iter)->vid;
	if (list_is_last(&(*vlan_iter)->entry, &fid->vlanlist))
		*vlan_iter = NULL;
	else
		*vlan_iter = list_next_entry(*vlan_iter, entry);
	return vid;
}

static ssize_t max_vid_show(struct device *dev,
			    struct device_attribute *attr,
			    char *buf)
{
	return scnprintf(buf, PAGE_SIZE, "4094\n");
}

static ssize_t max_supp_vlans_show(struct device *dev,
				   struct device_attribute *attr,
				   char *buf)
{
	struct edgx_br_vlan *vlan = edgx_dev2brvlan(dev);

	return scnprintf(buf, PAGE_SIZE, "%u\n", vlan->max_vlans);
}

static ssize_t num_vlans_show(struct device *dev,
			      struct device_attribute *attr,
			      char *buf)
{
	struct edgx_br_vlan *vlan = edgx_dev2brvlan(dev);

	return scnprintf(buf, PAGE_SIZE, "%u\n", vlan->nvlans);
}

static ssize_t vlan_ver_show(struct device *dev,
			     struct device_attribute *attr,
			     char *buf)
{
	return scnprintf(buf, PAGE_SIZE, "%u\n", 2);
}

static ssize_t vid2fid_read(struct file *filp, struct kobject *kobj,
			    struct bin_attribute *bin_attr, char *buf,
			    loff_t ofs, size_t count)
{
	struct edgx_br_vlan *brvlan = edgx_dev2brvlan(kobj_to_dev(kobj));
	loff_t idx;
	fid_t fid;

	if (!edgx_sysfs_tbl_params(ofs, count, sizeof(u32), &idx) &&
	    !_edgx_br_get_vid2fid(brvlan, (vid_t)idx, &fid)) {
		*((u32 *)buf) = fid;
		return count;
	}
	return 0;
}

static ssize_t vid2fid_write(struct file *filp, struct kobject *kobj,
			     struct bin_attribute *bin_attr, char *buf,
			     loff_t ofs, size_t count)
{
	struct edgx_br_vlan *brvlan = edgx_dev2brvlan(kobj_to_dev(kobj));
	loff_t idx;

	if (edgx_sysfs_tbl_params(ofs, count, sizeof(u32), &idx) ||
	    _edgx_br_set_vid2fid(brvlan, (vid_t)idx, *((fid_t *)buf)))
		return -EINVAL;

	return count;
}

static ssize_t fid2vid_read(struct file *filp, struct kobject *kobj,
			    struct bin_attribute *bin_attr, char *buf,
			    loff_t ofs, size_t count)
{
	struct edgx_br_vlan *brvlan = edgx_dev2brvlan(kobj_to_dev(kobj));
	int i = 0;
	loff_t idx;
	size_t nelems;
	struct _fid *fid;
	struct _vlan *vlan;

	if (edgx_sysfs_list_params(ofs, count, sizeof(u32) * brvlan->max_vlans,
				   &idx, &nelems) || nelems != 1)
		return 0;

	/* Set v2f-lock to Pevent changes while we read the allocated VIDs */
	mutex_lock(&brvlan->v2f_lock);
	fid = _edgx_br_vlan_get_fid(brvlan, (fid_t)idx);
	if (!fid) {
		mutex_unlock(&brvlan->v2f_lock);
		return 0;
	}

	list_for_each_entry(vlan, &fid->vlanlist, entry)
		((u32 *)buf)[i++] = vlan->vid;
	for (; i < brvlan->max_vlans; i++) /* Fill remaining entries */
		((u32 *)buf)[i++] = _INVALID_VID;
	mutex_unlock(&brvlan->v2f_lock);

	return count;
}

EDGX_DEV_ATTR_RO(max_vid,        "MaxVlanId");
EDGX_DEV_ATTR_RO(max_supp_vlans, "MaxSupportedVlans");
EDGX_DEV_ATTR_RO(num_vlans,      "NumVlans");
EDGX_DEV_ATTR_RO(vlan_ver,       "VlanVersionNumber");

static struct attribute *ieee8021_qbridge_attrs[] = {
	&dev_attr_max_vid.attr,
	&dev_attr_max_supp_vlans.attr,
	&dev_attr_num_vlans.attr,
	&dev_attr_vlan_ver.attr,
	NULL,
};

EDGX_BIN_ATTR_RW(vid2fid, "vid2fid", VLAN_N_VID * sizeof(u32));
EDGX_BIN_ATTR_RO(fid2vid, "fid2vid", _VLAN_N_FID * _VLAN_PER_FID * sizeof(u32));

static struct bin_attribute *ieee8021_qbridge_binattrs[] = {
	&bin_attr_fid2vid,
	/* TODO: Add option here to delete FID alloc?
	 *       This can be done by alloc'ing to (invalid) FID 0
	 *       FID is then replaced by dynamic allocation
	 *       Also needs update of tsntool
	 */
	&bin_attr_vid2fid,
	NULL,
};

static struct attribute_group ieee8021_qbridge_group = {
	.name      = "ieee8021QBridge",
	.attrs     = ieee8021_qbridge_attrs,
	.bin_attrs = ieee8021_qbridge_binattrs,
};

static ssize_t add_msti_store(struct device *dev,
			      struct device_attribute *attr,
			      const char *buf, size_t count)
{
	struct edgx_br_vlan *vlan = edgx_dev2brvlan(dev);
	mstid_t mstid;
	int err;

	if (kstrtou16(buf, 10, &mstid))
		return -EINVAL;
	err = _edgx_br_vlan_create_msti(vlan, mstid);
	return (err) ? err : count;
}

static ssize_t del_msti_store(struct device *dev,
			      struct device_attribute *attr,
			      const char *buf, size_t count)
{
	struct edgx_br_vlan *vlan = edgx_dev2brvlan(dev);
	mstid_t mstid;
	int err;

	if (kstrtou16(buf, 10, &mstid))
		return -EINVAL;
	err = _edgx_br_vlan_delete_msti(vlan, mstid);

	return (err) ? err : count;
}

static ssize_t max_msti_show(struct device *dev,
			     struct device_attribute *attr,
			     char *buf)
{
	struct edgx_br_vlan *vlan = edgx_dev2brvlan(dev);

	return scnprintf(buf, PAGE_SIZE, "%u\n", vlan->max_mstis);
}

static ssize_t fid2msti_write(struct file *filp, struct kobject *kobj,
			      struct bin_attribute *bin_attr, char *buf,
			      loff_t ofs, size_t count)
{
	struct edgx_br_vlan *brvlan = edgx_dev2brvlan(kobj_to_dev(kobj));
	loff_t idx;

	if (edgx_sysfs_tbl_params(ofs, count, sizeof(u32), &idx) ||
	    _edgx_br_set_fid2msti(brvlan, (fid_t)idx, *((mstid_t *)buf)))
		return -EINVAL;

	return count;
}

static ssize_t fid2msti_read(struct file *filp, struct kobject *kobj,
			     struct bin_attribute *bin_attr, char *buf,
			     loff_t ofs, size_t count)
{
	struct edgx_br_vlan *brvlan = edgx_dev2brvlan(kobj_to_dev(kobj));
	struct _fid *fid;
	loff_t idx;

	if (edgx_sysfs_tbl_params(ofs, count, sizeof(u32), &idx))
		return 0;

	mutex_lock(&brvlan->f2m_lock);
	fid = _edgx_br_vlan_get_fid(brvlan, idx);
	if (!fid) {
		mutex_unlock(&brvlan->f2m_lock);
		return 0;
	}
	*((u32 *)buf) = fid->msti->mstid;
	mutex_unlock(&brvlan->f2m_lock);

	return count;
}

static ssize_t msti_list_read(struct file *filp, struct kobject *kobj,
			      struct bin_attribute *bin_attr, char *buf,
			      loff_t ofs, size_t count)
{
	struct edgx_br_vlan *brvlan = edgx_dev2brvlan(kobj_to_dev(kobj));
	int i, j;
	loff_t idx;
	size_t nelems;

	if (edgx_sysfs_list_params(ofs, count, sizeof(u32) * brvlan->max_mstis,
				   &idx, &nelems) || nelems != 1 || idx != 0)
		return 0;

	mutex_lock(&brvlan->f2m_lock);
	for (i = 0, j = 0; i < brvlan->max_mstis; i++)
		if (brvlan->msti[i].mstid != EDGX_INVALID_MSTID)
			((u32 *)buf)[j++] = brvlan->msti[i].mstid;
	for (; j < brvlan->max_mstis; j++) /* Fill remaining entries */
		((u32 *)buf)[j] = EDGX_INVALID_MSTID;
	mutex_unlock(&brvlan->f2m_lock);

	return count;
}

static ssize_t static_trees_supported_show(struct device *dev,
					   struct device_attribute *attr,
					   char *buf)
{
	return scnprintf(buf, PAGE_SIZE, "%u\n", true);
}

EDGX_DEV_ATTR_WO(add_msti, "createMsti");
EDGX_DEV_ATTR_WO(del_msti, "deleteMsti");
EDGX_DEV_ATTR_RO(max_msti, "maxMsti");
EDGX_DEV_ATTR_RO(static_trees_supported, "staticTreesSupported");

static struct attribute *ieee8021_mstp_attrs[] = {
	&dev_attr_add_msti.attr,
	&dev_attr_del_msti.attr,
	&dev_attr_max_msti.attr,
	&dev_attr_static_trees_supported.attr,
	NULL,
};

EDGX_BIN_ATTR_RW(fid2msti,  "fid2msti", EDGX_MAX_SUPP_MSTI * sizeof(u32));
EDGX_BIN_ATTR_RO(msti_list, "mstiList", EDGX_MAX_SUPP_MSTI * sizeof(u32));

static struct bin_attribute *ieee8021_mstp_binattrs[] = {
	&bin_attr_fid2msti,
	&bin_attr_msti_list,
	NULL,
};

static struct attribute_group ieee8021_mstp_group = {
	.name      = "ieee8021Mstp",
	.attrs     = ieee8021_mstp_attrs,
	.bin_attrs = ieee8021_mstp_binattrs,
};

int edgx_br_init_vlan(struct edgx_br *br, edgx_io_t *iobase,
		      struct edgx_br_vlan **brvlan)
{
	struct edgx_br_vlan *vlan;
	int i;

	if (!br || !brvlan)
		return -EINVAL;

	vlan = kzalloc(sizeof(*vlan), GFP_KERNEL);
	if (!vlan)
		return -ENOMEM;

	vlan->parent    = br;
	vlan->iobase    = iobase;
	vlan->nvlans    = 0;

	/* We set the maximum number of supported VLANs to the number of
	 * supported FIDs (the latter being limited by hardware). The
	 * reason is, that an MST-bridge is required to support Individual
	 * VLAN learning (IVL). Because of this the dynamic allocation of
	 * a newly set VID to a port must provide an empty FID. This can
	 * only be enforced if there are at least (in our case exactly)
	 * as many FIDs than VIDs.
	 * Furthermore an MST bridge may support Shared VLAN learning (SVL),
	 * which we also do by allowing multiple VIDs to be allocated to one
	 * FID (see 802.1Q, Sect. 8.8.8)
	 */
	vlan->max_vlans = edgx_br_get_generic_pow2z(br, BR_GX_NFIDS);
	vlan->max_fids  = vlan->max_vlans;
	vlan->max_mstis = min(vlan->max_vlans, EDGX_MAX_SUPP_MSTI);

	mutex_init(&vlan->v2f_lock);
	mutex_init(&vlan->f2m_lock);
	mutex_init(&vlan->reg_lock);

	if (vlan->max_mstis < 2) {
		edgx_br_err(br, "Cannot instantiate minimum number of MSTIs\n");
		goto out_max_msti;
	}

	vlan->vid = kcalloc(vlan->max_vlans, sizeof(*vlan->vid), GFP_KERNEL);
	if (!vlan->vid)
		goto out_vlan;
	vlan->fid = kcalloc(vlan->max_fids, sizeof(*vlan->fid), GFP_KERNEL);
	if (!vlan->fid)
		goto out_fid;
	vlan->msti = kcalloc(vlan->max_mstis, sizeof(*vlan->msti), GFP_KERNEL);
	if (!vlan->msti)
		goto out_msti;

	for (i = 0; i < vlan->max_mstis; i++) {
		INIT_LIST_HEAD(&vlan->msti[i].fidlist);
		vlan->msti[i].mstid = EDGX_INVALID_MSTID;
	}

	/* Create the two default trees, i.e., CIST and TE;
	 * CIST we set everything to disabled, which will be changed pretty
	 * soon after loading.
	 * TE on the other hand is a MSTI that MSTP must ignore (see IEEE
	 * 802.1Qcc, Sect. 12.32.3.1); since it's about engineered traffic,
	 * the user must take care during network engineering that no logical
	 * loops will exist. Therefore we can just keep all port states on
	 * the TE-MSTI at FORWARDING
	 */
	_edgx_br_vlan_init_msti(&vlan->msti[_CIST_IDX], EDGX_CIST_MSTID,
				BR_STATE_DISABLED);
	_edgx_br_vlan_init_msti(&vlan->msti[_TE_IDX], EDGX_TE_MSTID,
				BR_STATE_FORWARDING);

	for (i = 0; i < vlan->max_fids; i++) {
		INIT_LIST_HEAD(&vlan->fid[i].vlanlist);
		vlan->fid[i].fid = i;
		mutex_lock(&vlan->f2m_lock);
		_edgx_br_move_fid2msti(vlan, &vlan->fid[i],
				       &vlan->msti[_CIST_IDX]);
		mutex_unlock(&vlan->f2m_lock);
	}

	/* Reset all port VLAN membership memberships */
	for (i = 0; i < VLAN_N_VID; i++) {
		mutex_lock(&vlan->reg_lock);
		edgx_wr16(vlan->iobase, EDGX_BR_VLAN_PORTS_BASE, 0x0);
		edgx_wr16(vlan->iobase, EDGX_BR_VLAN_CMD_BASE,
			  (i | EDGX_BR_VLAN_CMD_WRITE
			     | EDGX_BR_VLAN_CMD_PORTS));
		_edgx_br_vlan_wait_cmd(vlan);
		mutex_unlock(&vlan->reg_lock);
	}

	*brvlan = vlan;

	edgx_br_sysfs_add(br, &ieee8021_qbridge_group);
	edgx_br_sysfs_add(br, &ieee8021_mstp_group);
	edgx_br_info(br, "Instantiating VLAN management (%u VLANs %u FIDs. %u MSTIs) ... done\n",
		     vlan->max_vlans, vlan->max_fids, vlan->max_mstis);

	return 0;

out_msti:
	kfree(vlan->fid);
out_fid:
	kfree(vlan->vid);
out_vlan:
out_max_msti:
	kfree(vlan);
	return -ENOMEM;
}

void edgx_br_shutdown_vlan(struct edgx_br_vlan *brvlan)
{
	kfree(brvlan);
}
