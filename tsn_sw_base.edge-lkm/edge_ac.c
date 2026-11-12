// SPDX-License-Identifier: GPL-2.0
/*
 * TTTech EDGE/DE-IP Linux driver
 * Copyright(c) 2018 TTTech Computertechnik AG.
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
 * support@4me.tttech-dependablenetworks.com
 * TTTech Computertechnik AG, Schoenbrunnerstrasse 7, 1040 Vienna, Austria
 */

#include "edge_ac.h"
#include "edge_bridge.h"
#include "edge_util.h"

static struct edgx_ifdesc *_ifm;

int edgx_probe_ac(edgx_io_t *iobase, struct device *dev)
{
	int			 i;
	const struct edgx_if	*itf = (struct edgx_if *)((u8 *)iobase +
							  AC_IFMAP_OFS);
	struct edgx_ifdesc	*ifd;
	struct edgx_ifreq	 selfreq = { .id = AC_AC_ID, .v_maj = 1 };
	u32			 addr;

	if (!iobase)
		return -ENODEV;

	_ifm = kzalloc(sizeof(*_ifm) * AC_MAX_ENTRIES, GFP_KERNEL);
	if (!_ifm)
		return -ENOMEM;

	edgx_info("Enumerating interfaces on device '%s %s'...\n",
		  dev_driver_string(dev), dev_name(dev));
	ifd = &_ifm[0];
	for (i = 0; (itf->id != AC_EOT_ID) && (i < AC_MAX_ENTRIES);
	     itf++, i++) {
		if (itf->id == AC_DISABLED_ID)
			continue;

		addr		= le32_to_cpu(itf->addr);
		ifd->id		= le16_to_cpu(itf->id);
		ifd->ver	= itf->ver;
		ifd->iobase	= iobase + addr;
		ifd->len	= le32_to_cpu(itf->len);
		ifd->ptmap	= le32_to_cpu(itf->ptmap);
		edgx_info("   %d: <0x%02X> @ 0x%08X/0x%06zx, v%u.%u - [0x%X]\n",
			  i, ifd->id, addr, ifd->len,
			  ifd->ver.v_maj, ifd->ver.v_min, ifd->ptmap);
		ifd++;
	}

	ifd->id = AC_EOT_ID;
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
	unsigned int i = 0;

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

const struct edgx_ifdesc *edgx_ac_get_ifver(const struct edgx_ifreq iflist[],
					    const struct edgx_ifpriv **priv)
{
	unsigned int i;
	const struct edgx_ifreq *ifreq;

	if (!_ifm || !iflist)
		return NULL;
	for (i = 0; i < AC_MAX_ENTRIES; i++) {
		struct edgx_ifdesc *itf = &_ifm[i];

		for (ifreq = &iflist[0]; ifreq->id != AC_EOT_ID; ifreq++) {
			if (itf->id        == ifreq->id &&
			    itf->ver.v_maj == ifreq->v_maj) {
				if (priv)
					*priv = ifreq->priv;
				return itf;
			}
		}
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
	ptifdesc->ptmap   = BIT(pt);
	/* External port adapter addresses are used in sequence */
	if (ifdesc->id == AC_EXT_ADPT_ID) {
		u32 pt_idx = hweight32(ifdesc->ptmap & GENMASK(pt, 0)) - 1;

		ptifdesc->len     = ifdesc->len / hweight32(ifdesc->ptmap);
		ptifdesc->iobase  = ifdesc->iobase + (pt_idx * ptifdesc->len);
	} else {
		ptifdesc->len     = ifdesc->len / hweight32(ifdesc->ptmap);
		ptifdesc->iobase  = ifdesc->iobase + (pt * ptifdesc->len);
	}

	return ptifdesc;
}

const struct edgx_ifdesc *edgx_ac_get_ptif(const struct edgx_ifreq *ifreq,
					   ptid_t pt,
					   struct edgx_ifdesc *ptifdesc)
{
	const struct edgx_ifdesc *ifdesc = NULL;
	unsigned int i = 0;

	if (!_ifm || !ifreq)
		return NULL;
	for (i = 0; i < AC_MAX_ENTRIES; i++) {
		struct edgx_ifdesc *itf = &_ifm[i];

		if (itf->id        == ifreq->id &&
		    itf->ver.v_maj == ifreq->v_maj &&
		    itf->ptmap     & BIT(pt)) {
			ifdesc = itf;
		}
	}

	if (!ifdesc)
		return NULL;
	return edgx_ac_if2ptif(ifdesc, pt, ptifdesc);
}

const struct edgx_ifdesc *edgx_ac_get_ptifver(const struct edgx_ifreq iflist[],
					      ptid_t pt,
					      struct edgx_ifdesc *ptifdesc,
					      const struct edgx_ifpriv **priv)
{
	const struct edgx_ifdesc *ifdesc = NULL;
	unsigned int i;
	const struct edgx_ifreq *ifreq;

	if (!_ifm || !iflist)
		return NULL;
	for (i = 0; i < AC_MAX_ENTRIES; i++) {
		struct edgx_ifdesc *itf = &_ifm[i];

		for (ifreq = &iflist[0]; ifreq->id != AC_EOT_ID; ifreq++) {
			if (itf->id        == ifreq->id &&
			    itf->ver.v_maj == ifreq->v_maj &&
			    itf->ptmap     &  BIT(pt)) {
				ifdesc = itf;
				if (priv)
					*priv = ifreq->priv;
				break;
			}
		}
	}

	if (!ifdesc)
		return NULL;
	return edgx_ac_if2ptif(ifdesc, pt, ptifdesc);
}

void edgx_shutdown_ac(void)
{
	kfree(_ifm);
	_ifm = NULL;
}
