/* SPDX-License-Identifier: GPL-2.0 */
/*
 * TTTech EDGE/DE-IP Linux driver
 * Copyright(c) 2019 TTTech Industrial Automation AG.
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

#ifndef _EDGE_H
#define _EDGE_H

#include <linux/ktime.h>

struct platform_device;

struct edgx_br *edgx_get_edgx_br(struct platform_device *pdev);
/* in kernel access top PTP worker time */
ktime_t edgx_ktime_get_worker_ptp(struct platform_device *pdev);

#endif /* _EDGE_H */
