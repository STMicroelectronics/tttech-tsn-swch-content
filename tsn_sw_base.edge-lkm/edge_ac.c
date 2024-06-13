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
#include "edge_bridge.h"
#include "edge_util.h"

static struct edgx_ifdesc *_ifm;

int edgx_probe_ac(edgx_io_t *iobase, struct device *dev)
{
	int                i;
	struct edgx_if    *itf;
	struct edgx_ifreq  selfreq = { .id = AC_AC_ID, .v_maj = 1 };

	if (!iobase)
		return -ENODEV;

	_ifm = kzalloc(sizeof(*_ifm) * AC_MAX_ENTRIES, GFP_KERNEL);
	if (!_ifm)
		return -ENOMEM;

	edgx_info("Enumerating interfaces on device '%s %s'...\n",
		  dev_driver_string(dev), dev_name(dev));
	for (i = 0, itf = (struct edgx_if *)(((u8 *)iobase) + AC_IFMAP_OFS);
	     (itf->id != AC_EOT_ID) && (i < AC_MAX_ENTRIES);
	     itf++, i++) {
		u32 addr       = itf->addr;

		_ifm[i].id     = le16_to_cpu(itf->id);
		_ifm[i].ver    = itf->ver;
		_ifm[i].iobase = iobase + le32_to_cpu(itf->addr);
		_ifm[i].len    = le32_to_cpu(itf->len);
		_ifm[i].ptmap  = le32_to_cpu(itf->ptmap);
		edgx_info("   %d: <0x%02X> @ 0x%08X/0x%06zx, v%u.%u - [0x%X]\n",
			  i, _ifm[i].id, addr, _ifm[i].len,
			  _ifm[i].ver.v_maj, _ifm[i].ver.v_min, _ifm[i].ptmap);
	}

	_ifm[i].id = AC_EOT_ID;
	if (!edgx_ac_get_if(&selfreq)) {
		edgx_shutdown_ac();
		edgx_err("Interface-Map self-check ... FAILED\n");
		return -ENODEV;
	}
	edgx_info("Interface-Map self-check ... done\n");
	return 0;
}

const struct edgx_ifdesc *edgx_ac_get_if(const struct edgx_ifreq *ifreq)
{
	int i = 0;

	if (!_ifm || !ifreq)
		return NULL;
	for (i = 0; i < AC_MAX_ENTRIES; i++) {
		struct edgx_ifdesc *itf = &_ifm[i];

		if (itf->id        == ifreq->id &&
		    itf->ver.v_maj == ifreq->v_maj)
			return itf;
	}
	return NULL;
}

const struct edgx_ifdesc *edgx_ac_if2ptif(const struct edgx_ifdesc *ifdesc,
					  ptid_t pt,
					  struct edgx_ifdesc *ptifdesc)
{
	if (!ifdesc || !ptifdesc || (!(ifdesc->ptmap & BIT(pt))))
		return NULL;

	ptifdesc->id      = ifdesc->id;
	ptifdesc->ver     = ifdesc->ver;
	/* TODO: Change to do_div! */
	ptifdesc->len     = ifdesc->len / hweight_long(ifdesc->ptmap);
	ptifdesc->iobase  = ifdesc->iobase + (pt * ptifdesc->len);
	ptifdesc->ptmap   = BIT(pt);

	return ptifdesc;
}

const struct edgx_ifdesc *edgx_ac_get_ptif(const struct edgx_ifreq *ifreq,
					   ptid_t pt,
					   struct edgx_ifdesc *ptifdesc)
{
	const struct edgx_ifdesc *ifdesc = edgx_ac_get_if(ifreq);

	if (!ifdesc)
		return NULL;
	return edgx_ac_if2ptif(ifdesc, pt, ptifdesc);
}

void edgx_shutdown_ac(void)
{
	kfree(_ifm);
	_ifm = NULL;
}
