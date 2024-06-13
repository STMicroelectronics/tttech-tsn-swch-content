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

#include <linux/rbtree.h>
#include <linux/random.h>
#include "linux/etherdevice.h"

#include "edge_br_sid.h"
#include "edge_util.h"
#include "edge_port.h"
#include "edge_br_fdb.h"
#include "edge_defines.h"
#include "edge_ac.h"

#define sid_dbg edgx_dbg

#define SID_VID_MAX 4094U
#define SID_INVALID_FDB 0xFF

#define SID_CNT_CAPT0 0x0
#define SID_CNT_CAPT2 0x4
#define SID_CNT_FRAMES_IN_L 0x0300
#define SID_CNT_FRAMES_IN_H 0x0302
#define SID_CNT_FRAMES_OUT_L 0x0304
#define SID_CNT_FRAMES_OUT_H 0x0306
#define SID_MAX_NO_STREAMS 1024U
#define SID_INVALID_RBT 0xFFFF

enum sid_tag {
	SID_TAGGED = 1,
	SID_PRIORITY = 2,
	SID_ALL = 3,
};

enum sid_pos {
	IN_FAC_OUT = 0,
	IN_FAC_IN,
	OUT_FAC_OUT,
	OUT_FAC_IN,
};

struct port_pos {
	u16 port;
	enum sid_pos pos;
} __packed;

struct ident_params {
	u8  addr[ETH_ALEN];
	enum sid_tag tag;
	u16 vid;
	enum sid_ident_type id_type;
	u16 str_hdl;
}  __packed;

struct port_list {
	u16 in_fac_out;
	u16 out_fac_out;
	u16 in_fac_in;
	u16 out_fac_in;
} __packed;

/*Used for bridge attributes related to SID*/
struct edgx_sid_br {
	struct edgx_br *parent;
	u16 max_streams;
	u16 total_cnt;
	edgx_io_t *base_addr;
	struct mutex lock; /*Protects edgx_sid data*/
	struct rb_root sid_root;
	u16 sup_port_mask;
};

struct edgx_sid *edgx_sid_rbt_search(struct rb_root *root, u16 key)
{
	struct rb_node *node = root->rb_node;

	while (node) {
		struct edgx_sid *sid = rb_entry(node, struct edgx_sid, rbnode);

		if (key < sid->str_hdl)
			node = node->rb_left;
		else if (key > sid->str_hdl)
			node = node->rb_right;
		else
			return sid;
	}
	return NULL;
}

static inline int edgx_sid_del_entry(struct edgx_br *br, struct edgx_sid *sid)
{
	struct edgx_sid_br *sid_br = edgx_br_get_sid(br);

	if (edgx_brfdb_sid_del(edgx_br_get_fdb(br), sid->addr, sid->vid)) {
		edgx_br_err(br, "DEL entry error\n");
		return -ENOENT;
	}

	rb_erase(&sid->rbnode, &sid_br->sid_root);
	RB_CLEAR_NODE(&sid->rbnode);
	kfree(sid);
	sid_br->total_cnt--;

	return 0;
}

static u32 _edgx_sid_get_cnt(struct edgx_sid_br *sid, u16 str_hdl, u8 port,
			     enum cnt_type type)
{
	u16 capture = port | (str_hdl << 4);
	u32 val = 0;

	edgx_wr16(sid->base_addr, SID_CNT_CAPT2, capture);

	do {
		capture = edgx_rd16(sid->base_addr, SID_CNT_CAPT0);
	} while (capture & BIT(4));

	if (type == CNT_IN)
		val = (edgx_rd16(sid->base_addr, SID_CNT_FRAMES_IN_L) |
		      (((u32)edgx_rd16(sid->base_addr,
				       SID_CNT_FRAMES_IN_H)) << 16));
	else
		val = (edgx_rd16(sid->base_addr, SID_CNT_FRAMES_OUT_L) |
		      (((u32)edgx_rd16(sid->base_addr,
				       SID_CNT_FRAMES_OUT_H)) << 16));

	return val;
}

u32 edgx_sid_get_cnt(struct edgx_sid_br *sid, u16 str_hdl, u8 port,
		     enum cnt_type type)
{
	u32 ret;

	mutex_lock(&sid->lock);
	ret = _edgx_sid_get_cnt(sid, str_hdl, port, type);
	mutex_unlock(&sid->lock);

	return ret;
}

static int edgx_sid_rbt_insert(struct edgx_sid *newsid, struct edgx_br *br)
{
	struct edgx_sid *sid;
	struct rb_node **pp, *p;
	struct edgx_sid_br *sid_br = edgx_br_get_sid(br);

	pp = &sid_br->sid_root.rb_node;
	p = NULL;

	while (*pp) {
		p = *pp;
		sid = rb_entry(p, struct edgx_sid, rbnode);
		if (newsid->str_hdl < sid->str_hdl)
			pp = &(*pp)->rb_left;
		else if (newsid->str_hdl > sid->str_hdl)
			pp = &(*pp)->rb_right;
		else
			return -EINVAL; /*This should never happen*/
	}

	rb_link_node(&newsid->rbnode, p, pp);
	rb_insert_color(&newsid->rbnode, &sid_br->sid_root);
	sid_br->total_cnt++;

	return 0;
}

static int edgx_sid_check_add(u16 str_hdl, struct port_pos *add,
			      struct edgx_br *br)
{
	struct edgx_sid_br *sid = edgx_br_get_sid(br);

	if (add->port > edgx_br_get_num_ports(br)) {
		edgx_br_warn(br, "ADD: Port doesn't exist\n");
		return -EINVAL;
	} else if (add->pos != OUT_FAC_IN) {
		/* according to 802.1cb Table 8-1 */
		edgx_br_warn(br, "ADD: Only Out-Fac-In position supported\n");
		return -EINVAL;
	} else if (str_hdl >= sid->max_streams) {
		edgx_br_warn(br,
			     "ADD: Stream handle invalid, max supported < %d\n",
			     sid->max_streams);
		return -EINVAL;
	}

	return 0;
}

static int edgx_sid_check_ident(u16 str_hdl, struct ident_params *id,
				struct edgx_br *br)
{
	struct edgx_sid_br *sid = edgx_br_get_sid(br);

	if (id->id_type != SID_NULL) {
		edgx_br_warn(br, "Only Null Stream Identification supported\n");
		return -EINVAL;
	} else if (id->tag != SID_TAGGED) {
		edgx_br_warn(br, "Only VLAN tagged streams supported\n");
		return -EINVAL;
	} else if (id->vid == 0 || id->vid > SID_VID_MAX) {
		edgx_br_warn(br, "Invalid VLAN ID value\n");
		return -EINVAL;
	}  else if (str_hdl >= sid->max_streams) {
		edgx_br_warn(br, "Stream handle invalid, max supported < %d\n",
			     sid->max_streams);
		return -EINVAL;
	}

	return 0;
}

/* Called when SID parameters (Port-Pos or Identification data) are changed */
static int edgx_sid_apply_sid(struct edgx_sid *sid, struct edgx_br *br)
{
	/* If SID parameters and port is set correctly, write config to fdb */
	if (sid->id_type != SID_UNKNOWN && sid->port_mask != 0) {
		if (!edgx_brfdb_sid_add(edgx_br_get_fdb(br), sid)) {
			if (!sid->valid_fdb) {
				edgx_br_err(br,
					    "Stream not added to FDB (no match)\n");
				return -EINVAL;
			}
		} else {
			edgx_br_err(br,
				    "Stream not added to FDB - No entry left\n");
			return -ENOMEM;
		}
	}

	return 0;
}

static int edgx_sid_try_delete(u16 str_hdl, struct edgx_sid_br *sid_br,
			       struct edgx_br *br)
{
	struct edgx_sid *sid;

	if (str_hdl >= sid_br->max_streams) {
		edgx_br_err(br,
			    "DEL: Stream handle invalid, max supported < %d\n",
			    sid_br->max_streams);
		return -EINVAL;
	}

	sid = edgx_sid_rbt_search(&sid_br->sid_root, str_hdl);
	if (sid) {
		if (edgx_sid_del_entry(br, sid))
			return -ENOENT;
	} else {
		edgx_br_err(br, "DEL:Stream doesn't exist in the database\n");
		return -ENOENT;
	}

	return 0;
}

static int edgx_sid_try_del_pt(u16 str_hdl, struct port_pos *del,
			       struct edgx_br *br, struct edgx_sid_br *s_br)
{
	struct edgx_sid *old;

	edgx_sid_check_add(str_hdl, del, br);

	old = edgx_sid_rbt_search(&s_br->sid_root, str_hdl);
	if (old) {
		old->port_mask &= (~BIT(del->port));
		if (old->port_mask == 0) {
			if (edgx_sid_del_entry(br, old))
				return -ENOENT;
		}
	} else {
		edgx_br_err(br, "DEL Port:Stream not found\n");
		return -EINVAL;
	}

	return 0;
}

static int edgx_sid_try_port_add(u16 str_hdl, struct port_pos *add,
				 struct edgx_br *br)
{
	struct edgx_sid *sid;
	struct edgx_sid_br *s_br = edgx_br_get_sid(br);
	int ret = 0;

	if (edgx_sid_check_add(str_hdl, add, br)) {
		edgx_br_err(br, "Port-Pos:Parameters set incorrectly\n");
		return -EINVAL;
	}

	sid = edgx_sid_rbt_search(&s_br->sid_root, str_hdl);
	if (sid) {
		if (sid->port_mask == SID_INVALID_RBT)
			sid->port_mask = 0;
		sid->port_mask |= BIT(add->port);
		//if sid is saved in fdb no need to update port FLEXDE-1105
		if (sid->valid_fdb != 1)
			ret = edgx_sid_apply_sid(sid, br);
		if (ret) {
			//If the entry couldn't be added to FDB, remove setting
			sid->port_mask &= ~(1UL << add->port);
		}
	} else {
		edgx_br_err(br, "Port-Pos Add:Stream doesn't exist!\n");
		return -ENOENT;
	}

	return ret;
}

/*
 * User can add more same identification parameters to different SID nodes,
 * while FDB can accept up to two entries with same MAC-VID combination. This
 * can lead to having more nodes in RBT than is actually in FDB. Possible
 * solution: check each MAC and VID combination and if it exists in the RBT
 * while creating entries
 */
static int edgx_sid_try_ident_add(u16 str_hdl, struct ident_params *id,
				  struct edgx_br *br)
{
	struct edgx_sid *sid;
	struct edgx_sid_br *s_br = edgx_br_get_sid(br);
	int ret = 0;

	if (edgx_sid_check_ident(str_hdl, id, br)) {
		edgx_br_err(br, "Identification parameters set incorrectly\n");
		return -EINVAL;
	}

	sid = edgx_sid_rbt_search(&s_br->sid_root, str_hdl);

	if (sid) {
		if (sid->valid_fdb) {
			edgx_br_err(br, "SID data already written to FDB\n");
			return -EBUSY;
		}

		sid->str_hdl = str_hdl;
		sid->vid = id->vid;
		sid->id_type = id->id_type;
		ether_addr_copy(sid->addr, id->addr);
		ret = edgx_sid_apply_sid(sid, br);
		if (ret)
			sid->id_type = SID_UNKNOWN;
	} else {
		edgx_br_err(br, "Stream doesn't exist in the database\n");
		return -ENOENT;
	}

	return ret;
}

static int edgx_sid_try_create(u16 strhdl, struct edgx_br *br)

{
	struct edgx_sid		*old;
	struct edgx_sid		*new;
	struct edgx_sid_br *s_br = edgx_br_get_sid(br);

	if (strhdl >= s_br->max_streams) {
		edgx_br_err(br,
			    "Create: Stream handle invalid, max supported < %d\n",
			    s_br->max_streams);
		return -EINVAL;
	}

	old = edgx_sid_rbt_search(&s_br->sid_root, strhdl);
	if (old) {
		edgx_br_err(br, "Create: SID Already exists\n");
		return -EEXIST;
	}

	new = kzalloc(sizeof(*new), GFP_KERNEL);
	if (!new)
		return -ENOMEM;

	new->str_hdl = strhdl;
	new->valid_fdb = 0;
	new->port_mask = 0;
	new->id_type = SID_UNKNOWN;

	if (edgx_sid_rbt_insert(new, br)) {
		kfree(new);
		edgx_br_err(br, "Create: Stream handle not added to rbt!\n");
		return -EINVAL;
	}

	return 0;
}

static ssize_t sid_max_sup_show(struct device *dev,
				struct device_attribute *attr,
				char *buf)
{
	struct edgx_sid_br *sid_br = edgx_br_get_sid(edgx_dev2br(dev));
	int ret = 0;

	mutex_lock(&sid_br->lock);
	/* Max. supported stream handle value */
	ret =  scnprintf(buf, PAGE_SIZE, "%u\n",
			 sid_br->max_streams ? sid_br->max_streams - 1 : 0);
	mutex_unlock(&sid_br->lock);

	return ret;
}

static ssize_t sid_sup_ports_show(struct device *dev,
				  struct device_attribute *attr,
				  char *buf)
{
	struct edgx_sid_br *sid_br = edgx_br_get_sid(edgx_dev2br(dev));
	int ret = 0;

	sid_dbg("\n");
	mutex_lock(&sid_br->lock);
	ret =  scnprintf(buf, PAGE_SIZE, "%u\n", sid_br->sup_port_mask);
	mutex_unlock(&sid_br->lock);

	return ret;
}

static ssize_t sid_list_entries_read(struct file *filp, struct kobject *kobj,
				     struct bin_attribute *bin_attr,
				     char *buf, loff_t ofs, size_t count)
{
	struct rb_node *nd;
	u16 *str_list;
	struct edgx_br *br = edgx_dev2br(kobj_to_dev(kobj));
	struct edgx_sid_br *sid_br = edgx_br_get_sid(br);
	int ret = 0;
	u16 i = 0;

	str_list = (u16 *)buf;

	mutex_lock(&sid_br->lock);
	if (sid_br->total_cnt == 0)
		ret = -ENOENT;
	for (nd = rb_first(&sid_br->sid_root); nd; nd = rb_next(nd)) {
		str_list[i] = rb_entry(nd, struct edgx_sid, rbnode)->str_hdl;
		i++;
	}
	mutex_unlock(&sid_br->lock);

	return ret ? ret : (i * sizeof(u16));
}

static ssize_t sid_sup_ident_types_read(struct file *filp, struct kobject *kobj,
					struct bin_attribute *bin_attr,
					char *buf, loff_t ofs, size_t count)
{
	struct edgx_br *br = edgx_dev2br(kobj_to_dev(kobj));
	struct edgx_sid_br *sid_br = edgx_br_get_sid(br);
	loff_t idx;
	u8 *supported = (u8*)buf;

	sid_dbg("\n");

	if (edgx_sysfs_tbl_params(ofs, count, sizeof(u8), &idx) ||
	    idx >= SID_IDENT_MAX || idx == SID_UNKNOWN)
		return -EINVAL;

	mutex_lock(&sid_br->lock);

	if (idx == SID_NULL)
		*supported = 1U;
	else
		*supported = 0;

	mutex_unlock(&sid_br->lock);
	return count;
}

static ssize_t sid_cps_in_read(struct file *filp, struct kobject *kobj,
			       struct bin_attribute *bin_attr,
			       char *buf, loff_t ofs, size_t count)
{
	loff_t	idx = 0;
	struct edgx_pt *pt = edgx_dev2pt(kobj_to_dev(kobj));
	struct edgx_sid_br *sid = edgx_br_get_sid(edgx_pt_get_br(pt));
	int ret = 0;

	if (edgx_sysfs_tbl_params(ofs, count, sizeof(u32), &idx) ||
	    idx >= sid->max_streams)
		return -EINVAL;

	mutex_lock(&sid->lock);
	if (edgx_sid_rbt_search(&sid->sid_root, idx))
		((u32 *)buf)[0] = _edgx_sid_get_cnt(sid, idx,
						    edgx_pt_get_id(pt),
						    CNT_IN);
	else
		ret = -ENOENT;
	mutex_unlock(&sid->lock);

	return ret ? ret : count;
}

static ssize_t sid_cps_out_read(struct file *filp, struct kobject *kobj,
				struct bin_attribute *bin_attr,
				char *buf, loff_t ofs, size_t count)
{
	loff_t	idx = 0;
	struct edgx_pt *pt = edgx_dev2pt(kobj_to_dev(kobj));
	struct edgx_sid_br *sid = edgx_br_get_sid(edgx_pt_get_br(pt));
	int ret = 0;

	if (edgx_sysfs_tbl_params(ofs, count, sizeof(u32), &idx) ||
	    idx >= sid->max_streams)
		return -EINVAL;

	mutex_lock(&sid->lock);
	if (edgx_sid_rbt_search(&sid->sid_root, idx))
		((u32 *)buf)[0] = _edgx_sid_get_cnt(sid, idx,
						    edgx_pt_get_id(pt),
						    CNT_OUT);
	else
		ret =  -ENOENT;
	mutex_unlock(&sid->lock);

	return ret ? ret : count;
}

static ssize_t sid_cnt_show(struct device *dev,
			    struct device_attribute *attr,
			    char *buf)
{
	struct edgx_sid_br *sid_br = edgx_br_get_sid(edgx_dev2br(dev));
	int ret = 0;

	mutex_lock(&sid_br->lock);
	ret = scnprintf(buf, PAGE_SIZE, "%u\n", sid_br->total_cnt);
	mutex_unlock(&sid_br->lock);

	return ret;
}

static ssize_t sid_tab_len_show(struct device *dev,
				struct device_attribute *attr,
				char *buf)
{
	struct edgx_sid_br *sid_br = edgx_br_get_sid(edgx_dev2br(dev));
	int ret = 0;

	sid_dbg("\n");
	mutex_lock(&sid_br->lock);
	/* TODO: Check the correct table size */
	ret = scnprintf(buf, PAGE_SIZE, "%u\n", sid_br->max_streams);
	mutex_unlock(&sid_br->lock);

	return ret;
}

static ssize_t sid_delete_write(struct file *filp, struct kobject *kobj,
				struct bin_attribute *bin_attr,
				char *buf, loff_t ofs, size_t count)
{
	struct edgx_br *br = edgx_dev2br(kobj_to_dev(kobj));
	struct edgx_sid_br *sid_br = edgx_br_get_sid(br);
	int ret = 0;
	loff_t	idx;

	if (edgx_sysfs_tbl_params(ofs, count, sizeof(u16),
				  &idx) || idx >= sid_br->max_streams)
		return -EINVAL;

	mutex_lock(&sid_br->lock);
	ret = edgx_sid_try_delete((u16)idx, sid_br, br);
	mutex_unlock(&sid_br->lock);

	return ret ? ret : count;
}

static ssize_t sid_port_pos_del_write(struct file *filp, struct kobject *kobj,
				      struct bin_attribute *bin_attr,
				      char *buf, loff_t ofs, size_t count)
{
	struct edgx_br *br = edgx_dev2br(kobj_to_dev(kobj));
	struct edgx_sid_br *sid_br = edgx_br_get_sid(br);
	struct port_pos *sid_del = (struct port_pos *)buf;
	loff_t	idx;
	int ret;

	if (edgx_sysfs_tbl_params(ofs, count, sizeof(struct port_pos),
				  &idx) || idx >= sid_br->max_streams)
		return -EINVAL;

	mutex_lock(&sid_br->lock);
	ret = edgx_sid_try_del_pt((u16)idx, sid_del, br, sid_br);
	mutex_unlock(&sid_br->lock);

	return ret ? ret : count;
}

static ssize_t sid_ident_params_write(struct file *filp, struct kobject *kobj,
				     struct bin_attribute *bin_attr,
				     char *buf, loff_t ofs, size_t count)
{
	struct edgx_br *br = edgx_dev2br(kobj_to_dev(kobj));
	struct edgx_sid_br *sid_br = edgx_br_get_sid(br);
	struct ident_params *add = (struct ident_params *)buf;
	loff_t	idx;
	int ret;

	if (edgx_sysfs_tbl_params(ofs, count, sizeof(struct ident_params),
				  &idx) || idx >= sid_br->max_streams)
		return -EINVAL;

	mutex_lock(&sid_br->lock);
	ret = edgx_sid_try_create((u16)idx, br);
	if (!ret)
		ret = edgx_sid_try_ident_add((u16)idx, add, br);
	mutex_unlock(&sid_br->lock);

	return ret ? ret : count;
}

static ssize_t sid_port_pos_write(struct file *filp, struct kobject *kobj,
				  struct bin_attribute *bin_attr,
				  char *buf, loff_t ofs, size_t count)
{
	struct edgx_br *br = edgx_dev2br(kobj_to_dev(kobj));
	struct edgx_sid_br *sid_br = edgx_br_get_sid(br);
	struct port_pos *add = (struct port_pos *)buf;
	loff_t	idx = 0;
	int ret;

	if (edgx_sysfs_tbl_params(ofs, count, sizeof(struct port_pos),
				  &idx) || idx >= sid_br->max_streams) {
		edgx_br_err(br, "ofs=%u, count=%u, idx=%u, ppsize=%u\n",
				(unsigned int)ofs, (unsigned int)count,
				(unsigned int)idx, sizeof(struct port_pos));
		return -EINVAL;
	}

	mutex_lock(&sid_br->lock);
	ret = edgx_sid_try_port_add((u16)idx, add, br);
	mutex_unlock(&sid_br->lock);

	return ret ? ret : count;
}

static ssize_t sid_port_pos_read(struct file *filp, struct kobject *kobj,
				 struct bin_attribute *bin_attr,
				 char *buf, loff_t ofs, size_t count)
{
	loff_t	idx = 0;
	struct edgx_br *br = edgx_dev2br(kobj_to_dev(kobj));
	struct edgx_sid_br *sid_br = edgx_br_get_sid(br);
	struct edgx_sid *sid;
	struct port_list *tmp = (struct port_list *)buf;

	if (edgx_sysfs_tbl_params(ofs, count, sizeof(struct port_list),
				  &idx) || idx >= sid_br->max_streams)
		return -EINVAL;

	mutex_lock(&sid_br->lock);
	sid = edgx_sid_rbt_search(&sid_br->sid_root, idx);
	if (sid) {
		tmp->in_fac_in = 0;
		tmp->in_fac_out = 0;
		tmp->out_fac_in = sid->port_mask;
		tmp->out_fac_out = 0;
	} else {
		mutex_unlock(&sid_br->lock);
		edgx_br_err(br, "PortList:Stream handle doesn't exist\n");
		return -ENOENT;
	}
	mutex_unlock(&sid_br->lock);

	return count;
}

static ssize_t sid_ident_params_read(struct file *filp, struct kobject *kobj,
				     struct bin_attribute *bin_attr,
				     char *buf, loff_t ofs, size_t count)
{
	loff_t	idx = 0;
	struct edgx_br *br = edgx_dev2br(kobj_to_dev(kobj));
	struct edgx_sid_br *sid_br = edgx_br_get_sid(br);
	struct edgx_sid *sid;
	struct ident_params *tmp;

	tmp = (struct ident_params *)buf;

	if (edgx_sysfs_tbl_params(ofs, count, sizeof(struct ident_params),
				  &idx) || idx >= sid_br->max_streams)
		return -EINVAL;

	mutex_lock(&sid_br->lock);
	sid = edgx_sid_rbt_search(&sid_br->sid_root, idx);
	if (!sid) {
		mutex_unlock(&sid_br->lock);
		edgx_br_err(br, "Stream handle doesn't exist\n");
		return -ENOENT;
	}
	if (sid->id_type != SID_UNKNOWN) {
		tmp->str_hdl = sid->str_hdl;
		ether_addr_copy(tmp->addr, sid->addr);
		tmp->tag = SID_TAGGED;
		tmp->vid = sid->vid;
		tmp->id_type = sid->id_type;
	} else {
		tmp->id_type = SID_UNKNOWN;
		tmp->str_hdl = sid->str_hdl;
	}
	mutex_unlock(&sid_br->lock);

	return count;
}

static ssize_t sid_set_strhdl_write(struct file *filp, struct kobject *kobj,
				    struct bin_attribute *bin_attr,
				    char *buf, loff_t ofs, size_t count)
{
	struct edgx_br *br = edgx_dev2br(kobj_to_dev(kobj));
	struct edgx_sid_br *sid_br = edgx_br_get_sid(br);
	struct edgx_sid *sid;
	loff_t	idx;
	int ret = 0;

	if (edgx_sysfs_tbl_params(ofs, count, sizeof(u16),
				  &idx) || (idx >= sid_br->max_streams) ||
				  ((u16)idx != *(u16*)buf))
		return -EINVAL;

	mutex_lock(&sid_br->lock);

	sid = edgx_sid_rbt_search(&sid_br->sid_root, (u16)idx);
	if (!sid) {
		edgx_br_err(br, "DEL:Stream doesn't exist in the database\n");
		ret = -ENOENT;
	}

	mutex_unlock(&sid_br->lock);
	return ret ? ret : count;
}

EDGX_DEV_ATTR_RO(sid_cnt, "tsnSidCnt");
EDGX_DEV_ATTR_RO(sid_tab_len, "tsnSidTableLength");
EDGX_DEV_ATTR_RO(sid_max_sup, "tsnSidMaxSupported");
EDGX_DEV_ATTR_RO(sid_sup_ports, "tsnSidSupportedPorts");

EDGX_BIN_ATTR_RW(sid_ident_params,  "tsnSidIdentParams",
		SID_MAX_NO_STREAMS * sizeof(struct ident_params));
EDGX_BIN_ATTR_WO(sid_delete,  "tsnSidDelete",
		SID_MAX_NO_STREAMS * sizeof(u16));
EDGX_BIN_ATTR_RW(sid_port_pos,  "tsnSidPortPos",
		SID_MAX_NO_STREAMS * sizeof(struct port_pos));
EDGX_BIN_ATTR_WO(sid_port_pos_del,  "tsnSidPortPosDelete",
		SID_MAX_NO_STREAMS * sizeof(struct port_pos));
EDGX_BIN_ATTR_WO(sid_set_strhdl,  "tsnSidSetStrHdl",
		SID_MAX_NO_STREAMS * sizeof(u16));
EDGX_BIN_ATTR_RO(sid_cps_in, "tsnCpsSidInputPackets",
		 SID_MAX_NO_STREAMS * sizeof(u32));
EDGX_BIN_ATTR_RO(sid_cps_out, "tsnCpsSidOutputPackets",
		 SID_MAX_NO_STREAMS * sizeof(u32));
EDGX_BIN_ATTR_RO(sid_list_entries, "tsnListEntries",
		 SID_MAX_NO_STREAMS * sizeof(u16));
EDGX_BIN_ATTR_RO(sid_sup_ident_types, "tsnSupportedIdentTypes",
		 SID_IDENT_MAX * sizeof(u8));

static struct attribute *ieee1cb_sid_attr[] = {
	&dev_attr_sid_cnt.attr,
	&dev_attr_sid_tab_len.attr,
	&dev_attr_sid_max_sup.attr,
	&dev_attr_sid_sup_ports.attr,
	NULL,
};

static struct bin_attribute *ieee1cb_sid_binattr[] = {
	&bin_attr_sid_ident_params,
	&bin_attr_sid_delete,
	&bin_attr_sid_port_pos,
	&bin_attr_sid_port_pos_del,
	&bin_attr_sid_set_strhdl,
	&bin_attr_sid_list_entries,
	&bin_attr_sid_sup_ident_types,
	NULL
};

static struct attribute_group sid_group = {
	.name  = "ieee8021SID",
	.attrs = ieee1cb_sid_attr,
	.bin_attrs = ieee1cb_sid_binattr,
};

static struct bin_attribute *ieee1cb_sid_pt_bin_attr[] = {
	&bin_attr_sid_cps_in,
	&bin_attr_sid_cps_out,
	NULL
};

static struct attribute_group sid_pt_group = {
	.name  = "ieee8021SID",
	.bin_attrs = ieee1cb_sid_pt_bin_attr,
};

int edgx_probe_sid(struct edgx_br *br, edgx_io_t *br_iobase,
		   struct edgx_sid_br **br_sid)
{
	int ret;
	struct edgx_sid_br *sid;
	const struct edgx_ifdesc *ifd;
	struct edgx_ifdesc        pifd;
	ptid_t                    ptid;
	const struct edgx_ifreq   ifreq = { .id = AC_SID_ID, .v_maj = 1 };

	if (!br)
		return -EINVAL;

	ifd = edgx_ac_get_if(&ifreq);
	if (!ifd)
		return -ENODEV;

	sid = kzalloc(sizeof(*sid), GFP_KERNEL);
	if (!(sid)) {
		edgx_br_err(br, "Cannot allocate Stream Identification\n");
		return -ENOMEM;
	}

	/* TODO: AC to be implemented */
	ret = edgx_br_sysfs_add(br, &sid_group);
	if (ret)
		goto out_sid_sysfs_add;

	/* Maximum supported streams is the minimum between number of supported
	 * streams according to register FRER_STREAMS value and number of
	 * entries in the first two columns in FDB (only first two columns
	 * support VLAN tagged frames)
	 */
	sid->parent = br;
	sid->max_streams = edgx_brfdb_sid_get_max_str(edgx_br_get_fdb(br));
	sid->base_addr = ifd->iobase;
	mutex_init(&sid->lock);
	sid->total_cnt = 0;
	sid->sid_root  = RB_ROOT;
	sid->sup_port_mask = (u16)ifd->ptmap << 1; /* TODO: Check port numbering! */

	edgx_ac_for_each_ifpt(ptid, ifd, &pifd) {
		struct edgx_pt *pt = edgx_br_get_brpt(br, ptid);

		if (pt)
			edgx_pt_add_sysfs(pt, &sid_pt_group);
	}

	edgx_br_info(br, "Setup Stream Identification ... done\n");
	*br_sid = sid;
	return 0;

out_sid_sysfs_add:
	kfree(sid);
	return ret;
}

void edgx_shutdown_sid(struct edgx_sid_br *sid)
{
	struct rb_node *next;
	struct edgx_sid *rbt;
	const struct edgx_ifdesc *ifd;
	struct edgx_ifdesc        pifd;
	ptid_t                    ptid;
	const struct edgx_ifreq   ifreq = { .id = AC_SID_ID, .v_maj = 1 };

	if (!sid)
		return;

	next = rb_first(&sid->sid_root);
	while (next) {
		rbt = rb_entry(next, struct edgx_sid, rbnode);
		next = rb_next(&rbt->rbnode);
		rb_erase(&rbt->rbnode, &sid->sid_root);
		RB_CLEAR_NODE(&rbt->rbnode);
		kfree(rbt);
	}

	ifd = edgx_ac_get_if(&ifreq);
	if (!ifd)
		return;

	edgx_ac_for_each_ifpt(ptid, ifd, &pifd) {
		struct edgx_pt *pt = edgx_br_get_brpt(sid->parent, ptid);

		if (pt)
			edgx_pt_rem_sysfs(pt, &sid_pt_group);
	}

	kfree(sid);
}
