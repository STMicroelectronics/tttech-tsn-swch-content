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

#ifndef _EDGE_UTIL_H
#define _EDGE_UTIL_H

#include <linux/kernel.h>
#include "edge_defines.h"

static inline u16 edgx_rd16(edgx_io_t *iobase, size_t ofs)
{
	return readw(iobase + ofs);
}

static inline void edgx_wr16(edgx_io_t *iobase, size_t ofs, u16 val)
{
	writew(val, iobase + ofs);
}

static inline u32 edgx_rd32(edgx_io_t *iobase, size_t ofs)
{
	return readl(iobase + ofs);
}

static inline void edgx_wr32(edgx_io_t *iobase, size_t ofs, u32 val)
{
	writel(val, iobase + ofs);
}

static inline void edgx_set32(edgx_io_t *iobase, size_t ofs, int bithi,
			      int bitlo, u32 val)
{
	u32 mask = ((1 << ((bithi - bitlo + 1))) - 1) << bitlo;
	u32 v    = (val << bitlo) & mask;

	BUILD_BUG_ON(bithi < bitlo);
	edgx_wr32(iobase, ofs, (edgx_rd32(iobase, ofs) & (~mask)) | v);
}

static inline u32 edgx_get32(edgx_io_t *iobase, size_t ofs, int bithi,
			     int bitlo)
{
	u32 mask = ((1 << ((bithi - bitlo + 1))) - 1);

	BUILD_BUG_ON(bithi < bitlo);
	return ((edgx_rd32(iobase, ofs) >> bitlo) & mask);
}

static inline void _edgx_set16(edgx_io_t *iobase, size_t ofs, int bithi,
			       int bitlo, u16 val)
{
	u16 mask = ((1 << ((bithi - bitlo + 1))) - 1) << bitlo;
	u16 v    = (val << bitlo) & mask;

	edgx_wr16(iobase, ofs, (edgx_rd16(iobase, ofs) & (~mask)) | v);
}

#define edgx_set16(iobase, ofs, bithi, bitlo, val) ({           \
	if (__is_constexpr(bithi) && __is_constexpr(bitlo))     \
		BUILD_BUG_ON(bithi < bitlo);                    \
	_edgx_set16(iobase, ofs, bithi, bitlo, val);            \
})

static inline u16 _edgx_get16(edgx_io_t *iobase, size_t ofs, int bithi,
			      int bitlo)
{
	u16 mask = ((1 << ((bithi - bitlo + 1))) - 1);

	return ((edgx_rd16(iobase, ofs) >> bitlo) & mask);
}

#define edgx_get16(iobase, ofs, bithi, bitlo) ({                \
	if (__is_constexpr(bithi) && __is_constexpr(bitlo))     \
		BUILD_BUG_ON(bithi < bitlo);                    \
	_edgx_get16(iobase, ofs, bithi, bitlo);                 \
})

#define ether_addr_cmp(mac1, mac2) memcmp((mac1), (mac2), ETH_ALEN)

#define _EDGX_PERM_RW VERIFY_OCTAL_PERMISSIONS(0644)
#define _EDGX_PERM_RO VERIFY_OCTAL_PERMISSIONS(0444)
#define _EDGX_PERM_WO VERIFY_OCTAL_PERMISSIONS(0200)

#define EDGX_BIN_ATTR_RW(_name, _fname, _size)                 \
	static struct bin_attribute bin_attr_##_name = {       \
	.attr  = { .name = _fname,                             \
		   .mode = _EDGX_PERM_RW,                      \
		 },                                            \
	.read  = _name##_read,                                 \
	.write = _name##_write,                                \
	.size  = _size                                         \
	}

#define EDGX_BIN_ATTR_RO(_name, _fname, _size)		       \
	static struct bin_attribute bin_attr_##_name = {       \
	.attr = { .name = _fname,                              \
		  .mode = _EDGX_PERM_RO,                       \
		},                                             \
	.read = _name##_read,                                  \
	.size = _size                                          \
	}

#define EDGX_BIN_ATTR_WO(_name, _fname, _size)                 \
	static struct bin_attribute bin_attr_##_name = {       \
	.attr  = { .name = _fname,                             \
		   .mode = _EDGX_PERM_WO,                      \
		 },                                            \
	.write = _name##_write,                                \
	.size  = _size                                         \
	}

#define EDGX_DEV_ATTR_RW(_name, _fname)                        \
	static struct device_attribute dev_attr_##_name = {    \
	.attr  = { .name = _fname,                             \
		   .mode = _EDGX_PERM_RW,                      \
		 },                                            \
	.show  = _name##_show,                                 \
	.store = _name##_store                                 \
	}

#define EDGX_DEV_FCT_ATTR_RO(_name, _fct, _fname)              \
	static struct device_attribute dev_attr_##_name = {    \
	.attr = {.name = _fname,                               \
		 .mode = _EDGX_PERM_RO,                        \
		},                                             \
	.show = _fct##_show,                                   \
	}

#define EDGX_DEV_ATTR_RO(_name, _fname)                        \
	EDGX_DEV_FCT_ATTR_RO(_name, _name, _fname)

#define EDGX_DEV_ATTR_WO(_name, _fname)                        \
	static struct device_attribute dev_attr_##_name = {    \
	.attr  = {.name = _fname,                              \
		  .mode = _EDGX_PERM_WO,                       \
		 },                                            \
	.store = _name##_store,                                \
	}

static inline ssize_t edgx_sysfs_one_show(struct device *dev,
					  struct device_attribute *attr,
					  char *buf)
{
	return scnprintf(buf, PAGE_SIZE, "1\n");
}

static inline ssize_t edgx_sysfs_zero_show(struct device *dev,
					   struct device_attribute *attr,
					   char *buf)
{
	return scnprintf(buf, PAGE_SIZE, "0\n");
}

static inline ssize_t edgx_sysfs_true_show(struct device *dev,
					   struct device_attribute *attr,
					   char *buf)
{
	return edgx_sysfs_one_show(dev, attr, buf);
}

static inline ssize_t edgx_sysfs_false_show(struct device *dev,
					    struct device_attribute *attr,
					    char *buf)
{
	return edgx_sysfs_zero_show(dev, attr, buf);
}

static inline int edgx_sysfs_tbl_params(loff_t ofs, size_t cnt, size_t tsz,
					loff_t *idx)
{
	u64 _idx, _idx_rem;

	_idx     = ofs;
	_idx_rem = do_div(_idx, tsz);

	/* SysFS Table ABI requires alignment on element boundary, i.e.,
	 * offset reminder must be 0.
	 */
	if (_idx_rem)
		return -EINVAL;

	/* SysFS Table ABI only allows access to a single table element, i.e.,
	 * the number of bytes (cnt) must be equal to the size of a table
	 * element.
	 */
	if (cnt != tsz)
		return -EINVAL;

	*idx = (loff_t)_idx;
	return 0;
}

static inline int edgx_sysfs_list_params(loff_t ofs, size_t cnt, size_t lsz,
					 loff_t *idx, size_t *nelems)
{
	u64 _idx, _idx_rem;
	u64 _nelems, _nelems_rem;

	_idx        = ofs;
	_idx_rem    = do_div(_idx, lsz);
	_nelems     = cnt;
	_nelems_rem = do_div(_nelems, lsz);

	/* SysFS List ABI requires alignment on element boundary (i.e.,
	 * offset reminder must be 0) and an integer number of elements
	 * (i.e., count reminder must be 0).
	 */
	if (_idx_rem || _nelems_rem)
		return -EINVAL;

	*idx    = (loff_t)_idx;
	*nelems = (size_t)_nelems;
	return 0;
}

#endif /* _EDGE_UTIL_H */
