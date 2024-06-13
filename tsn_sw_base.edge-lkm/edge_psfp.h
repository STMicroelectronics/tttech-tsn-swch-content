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

#ifndef _EDGE_PSFP_H
#define _EDGE_PSFP_H

#include "edge_bridge.h"

struct edgx_psfp;

#ifndef EDGX_DISABLE_PSFP

int  edgx_probe_psfp(struct edgx_br *br, struct edgx_br_irq *irq,
		     const char *drv_name, struct edgx_psfp **ppsfp);
void edgx_shutdown_psfp(struct edgx_psfp *psfp);

#else /* EDGX_DISABLE_PSFP */

static inline int edgx_probe_psfp(struct edgx_br *br, struct edgx_br_irq *irq,
				  const char *drv_name,
				  struct edgx_psfp **ppsfp)
{
	return -ENODEV;
}

static inline void edgx_shutdown_psfp(struct edgx_psfp *psfp)
{
}

#endif /* EDGX_DISABLE_PSFP */

#endif /* _EDGE_PSFP_H */
