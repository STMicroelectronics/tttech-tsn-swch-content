/*
 * TTTech TSN Library (libtsn)
 * Copyright(c) 2017 TTTech Computertechnik AG.
 *
 * ALL RIGHTS RESERVED.
 * Usage of this software, including source code, netlists, documentation,
 * is subject to restrictions and conditions of the applicable license
 * agreement with TTTech Computertechnik AG or its affiliates.
 *
 * All trademarks used are the property of their respective owners.
 *
 * TTTech Computertechnik AG and its affiliates do not assume any liability
 * arising out of the application or use of any product described or shown
 * herein. TTTech Computertechnik AG and its affiliates reserve the right to
 * make changes, at any time, in order to improve reliability, function or
 * design.
 *
 * Contact Information:
 * support@4me.tttech-dependablenetworks.com
 * TTTech Computertechnik AG, Schoenbrunnerstrasse 7, 1040 Vienna, Austria
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <inttypes.h>

#include "tsn_private.h"
#include "tsn_frer.h"
#include "tsn_def.h"

#define IEEE8021SID_DIR "ieee8021FRER/"

static const uint32_t tsn_module = TSN_FRER;

static const sysfs_fn frer_fn [MAX_TSN_FRER_FUNCTIONS] = {
	[FRER_SEQGEN_CREATE] = IEEE8021SID_DIR"tsnFrerSeqGenCreate",
	[FRER_SEQGEN_ADD_STRHDL] = IEEE8021SID_DIR"tsnFrerSeqGenAddStream",
	[FRER_SEQGEN_ADD_DIR] = IEEE8021SID_DIR"tsnFrerSeqGenAddDir",
	[FRER_SEQGEN_DEL_STRHDL] = IEEE8021SID_DIR"tsnFrerSeqGenDelStrhdl",
	[FRER_SEQGEN_DEL_FN] = IEEE8021SID_DIR"tsnFrerSeqGenDelFn",
	[FRER_SEQGEN_FN_CNT] = IEEE8021SID_DIR"tsnfrerSeqGenCnt",
	[FRER_SEQGEN_FN_LIST] = IEEE8021SID_DIR"tsnFrerSeqGenList",
	[FRER_SEQGEN_SID2FN] = IEEE8021SID_DIR"tsnFrerSeqGenSid2Func",
	[FRER_SEQGEN_FN2DIR] = IEEE8021SID_DIR"tsnFrerSeqGenFunc2Dir",
	[FRER_SEQGEN_FN2SID] = IEEE8021SID_DIR"tsnFrerSeqGenFunc2Sid",
	[FRER_SEQENC_CREATE] = IEEE8021SID_DIR"tsnFrerSeqEncCreate",
	[FRER_SEQENC_ACTIVE] = IEEE8021SID_DIR"tsnFrerSeqEncActive",
	[FRER_SEQENC_TYPE] = IEEE8021SID_DIR"tsnFrerSeqEncType",
	[FRER_SEQENC_ADDSTR] = IEEE8021SID_DIR"tsnFrerSeqEncAddStr",
	[FRER_SEQENC_DELSTR] = IEEE8021SID_DIR"tsnFrerSeqEncDelStr",
	[FRER_SEQENC_DELPORT] = IEEE8021SID_DIR"tsnFrerSeqEncDelPortFn",
	[FRER_SEQENC_PORT2STR] = IEEE8021SID_DIR"tsnFrerSeqEncPort2Str",
	[FRER_SEQENC_STR2PORT] = IEEE8021SID_DIR"tsnFrerSeqEncStr2Port",
	[FRER_SEQREC_CREATE] = IEEE8021SID_DIR"tsnFrerSeqRecCreate",
	[FRER_SEQREC_ADDSTR] = IEEE8021SID_DIR"tsnFrerSeqRecAddStr",
	[FRER_SEQREC_ADDPORT] = IEEE8021SID_DIR"tsnFrerSeqRecAddPort",
	[FRER_SEQREC_TAKENOSEQ] = IEEE8021SID_DIR"tsnFrerSeqRecTakeNoSeq",
	[FRER_SEQREC_INDRCV] = IEEE8021SID_DIR"tsnFrerSeqRecIndRecv",
	[FRER_SEQREC_LATERR] = IEEE8021SID_DIR"tsnFrerSeqRecLatentErr",
	[FRER_SEQREC_ALG] = IEEE8021SID_DIR"tsnFrerSeqRecAlg",
	[FRER_SEQREC_HISTLEN] = IEEE8021SID_DIR"tsnFrerSeqRecHistLen",
	[FRER_SEQREC_DIR] = IEEE8021SID_DIR"tsnFrerSeqRecDir",
	[FRER_SEQREC_RCVRESET] = IEEE8021SID_DIR"tsnFrerSeqRecRstTime",
	[FRER_SEQREC_PTMASK] = IEEE8021SID_DIR"tsnFrerSeqRecPtMask",
	[FRER_SEQREC_DELPORT] = IEEE8021SID_DIR"tsnFrerSeqRecDelPort",
	[FRER_SEQREC_DELSTR] = IEEE8021SID_DIR"tsnFrerSeqRecDelStr",
	[FRER_SEQREC_DELFN] = IEEE8021SID_DIR"tsnfrerSeqRecDelFn",
	[FRER_SEQREC_FN2SID] = IEEE8021SID_DIR"tsnFrerSeqRecFn2Sid",
	[FRER_SEQREC_FN_CNT] = IEEE8021SID_DIR"tsnfrerSeqRecCnt",
	[FRER_SEQREC_FN_LIST] = IEEE8021SID_DIR"tsnFrerSeqRecList",
	[FRER_SEQREC_CNT_OOO] = IEEE8021SID_DIR"frerCpsSeqRcvyOutOfOrderPackets",
	[FRER_SEQREC_CNT_TAGLESS] = IEEE8021SID_DIR"frerCpsSeqRcvyTaglessPackets",
	[FRER_SEQREC_CNT_LOST] = IEEE8021SID_DIR"frerCpsSeqRcvyLostPackets",
	[FRER_SEQREC_CNT_RESETS] = IEEE8021SID_DIR"frerCpsSeqRcvyResets",
	[FRER_SEQREC_CNT_ROGUE] = IEEE8021SID_DIR"frerCpsSeqRcvyRoguePackets",
	[FRER_SEQREC_CNT_DISCARDED] = IEEE8021SID_DIR"frerCpsSeqRcvyDiscardedPackets",
	[FRER_SEQREC_CNT_PASSED] = IEEE8021SID_DIR"frerCpsSeqRcvyPassedPackets",
	[FRER_NUM_PORTS] = IEEE8021SID_DIR"tsnFrerNumPorts",
};

tsn_return_value tsn_frer_seqrec_create(struct port_hdl *brdev, uint16_t fnidx)
{
    tsn_file_desc fd = { .id = FRER_SEQREC_CREATE};
    ASSERT_PARAMS(brdev);
    ASSERT_FOPEN(fd, brdev, frer_fn[fd.id], O_WRONLY);
    ASSERT_FPRINT(fd, "%"PRIu16, (unsigned int)fnidx);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_frer_seqrec_addstr(struct port_hdl *brdev, uint16_t fnidx,
                                        const uint16_t strhdl)
{
    tsn_file_desc fd = { .id = FRER_SEQREC_ADDSTR};
    ASSERT_PARAMS(brdev);
    ASSERT_FOPEN(fd, brdev, frer_fn[fd.id], O_WRONLY);
    ASSERT_FWRENTRY(fd, strhdl, fnidx);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_frer_seqrec_addport(struct port_hdl *port, uint16_t fnidx)
{
    tsn_file_desc fd = { .id = FRER_SEQREC_ADDPORT};
    ASSERT_PARAMS(port);
    ASSERT_FOPEN(fd, port, frer_fn[fd.id], O_WRONLY);
    ASSERT_FPRINT(fd, "%"PRIu16, fnidx);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_frer_seqrec_set_alg(struct port_hdl *brdev, uint16_t fnidx,
                                         const uint32_t alg)
{
    tsn_file_desc fd = { .id = FRER_SEQREC_ALG};
    ASSERT_PARAMS(brdev);
    ASSERT_FOPEN(fd, brdev, frer_fn[fd.id], O_RDWR);
    ASSERT_FWRENTRY(fd, alg, fnidx);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_frer_seqrec_get_alg(struct port_hdl *brdev,
                                         unsigned int fnidx,
                                         uint32_t *alg)
{
    tsn_file_desc fd = { .id = FRER_SEQREC_ALG};
    ASSERT_PARAMS(brdev && alg);
    ASSERT_FOPEN(fd, brdev, frer_fn[fd.id], O_RDONLY);
    ASSERT_FRDENTRY(fd, alg, fnidx);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_frer_seqrec_set_dir(struct port_hdl *brdev, uint16_t fnidx,
                                         const uint8_t dir)
{
    tsn_file_desc fd = { .id = FRER_SEQREC_DIR};
    ASSERT_PARAMS(brdev);
    ASSERT_FOPEN(fd, brdev, frer_fn[fd.id], O_WRONLY);
    ASSERT_FWRENTRY(fd, dir, fnidx);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_frer_seqrec_get_dir(struct port_hdl *brdev, uint16_t fnidx,
                                         uint8_t *dir)
{
    tsn_file_desc fd = { .id = FRER_SEQREC_DIR};
    ASSERT_PARAMS(brdev && dir);
    ASSERT_FOPEN(fd, brdev, frer_fn[fd.id], O_RDONLY);
    ASSERT_FRDENTRY(fd, dir, fnidx);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_frer_seqrec_set_histlen(struct port_hdl *brdev,
                                             uint16_t fnidx,
                                             const uint8_t histlen)
{
    tsn_file_desc fd = { .id = FRER_SEQREC_HISTLEN};
    ASSERT_PARAMS(brdev);
    ASSERT_FOPEN(fd, brdev, frer_fn[fd.id], O_RDWR);
    ASSERT_FWRENTRY(fd, histlen, fnidx);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_frer_seqrec_get_histlen(struct port_hdl *brdev,
                                             unsigned int fnidx,
                                             uint8_t *histlen)
{
    tsn_file_desc fd = { .id = FRER_SEQREC_HISTLEN};
    ASSERT_PARAMS(brdev && histlen);
    ASSERT_FOPEN(fd, brdev, frer_fn[fd.id],O_RDONLY);
    ASSERT_FRDENTRY(fd, histlen, fnidx);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_frer_seqrec_set_recreset(struct port_hdl *brdev,
                                              uint16_t fnidx,
                                              const uint16_t rsttime)
{
    tsn_file_desc fd = { .id = FRER_SEQREC_RCVRESET};
    ASSERT_PARAMS(brdev);
    ASSERT_FOPEN(fd, brdev, frer_fn[fd.id], O_RDWR);
    ASSERT_FWRENTRY(fd, rsttime, fnidx);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_frer_seqrec_get_recreset(struct port_hdl *brdev,
                                              unsigned int fnidx,
                                              uint16_t *rsttime)
{
    tsn_file_desc fd = { .id = FRER_SEQREC_RCVRESET};
    ASSERT_PARAMS(brdev && rsttime);
    ASSERT_FOPEN(fd, brdev, frer_fn[fd.id], O_RDONLY);
    ASSERT_FRDENTRY(fd, rsttime, fnidx);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_frer_seqrec_set_takenoseq(struct port_hdl *brdev,
                                               uint16_t fnidx,
                                               const uint8_t tns)
{
    tsn_file_desc fd = { .id = FRER_SEQREC_TAKENOSEQ};;
    ASSERT_PARAMS(brdev);
    ASSERT_FOPEN(fd, brdev, frer_fn[fd.id], O_RDWR);
    ASSERT_FWRENTRY(fd, tns, fnidx);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_frer_seqrec_get_takenoseq(struct port_hdl *brdev,
                                               unsigned int fnidx,
                                               uint8_t *tns)
{
    tsn_file_desc fd = { .id = FRER_SEQREC_TAKENOSEQ};
    ASSERT_PARAMS(brdev && tns);
    ASSERT_FOPEN(fd, brdev, frer_fn[fd.id], O_RDONLY);
    ASSERT_FRDENTRY(fd, tns, fnidx);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_frer_seqrec_set_indrcv(struct port_hdl *brdev,
                                            uint16_t fnidx,
                                            const uint8_t indrcv)
{
    tsn_file_desc fd = { .id = FRER_SEQREC_INDRCV};
    ASSERT_PARAMS(brdev);
    ASSERT_FOPEN(fd, brdev, frer_fn[fd.id], O_RDWR);
    ASSERT_FWRENTRY(fd, indrcv, fnidx);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_frer_seqrec_get_indrcv(struct port_hdl *brdev,
                                            unsigned int fnidx,
                                            uint8_t *indrcv)
{
    tsn_file_desc fd = { .id = FRER_SEQREC_INDRCV};
    ASSERT_PARAMS(brdev && indrcv);
    ASSERT_FOPEN(fd, brdev, frer_fn[fd.id], O_RDONLY);
    ASSERT_FRDENTRY(fd, indrcv, fnidx);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_frer_seqrec_set_laterr(struct port_hdl *brdev,
                                            uint16_t fnidx,
                                            const uint8_t laterr)
{
    tsn_file_desc fd = { .id = FRER_SEQREC_LATERR};
    ASSERT_PARAMS(brdev);
    ASSERT_FOPEN(fd, brdev, frer_fn[fd.id], O_RDWR);
    ASSERT_FWRENTRY(fd, laterr, fnidx);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_frer_seqrec_get_laterr(struct port_hdl *brdev,
                                            unsigned int fnidx,
                                            uint8_t *laterr)
{
    tsn_file_desc fd = { .id = FRER_SEQREC_LATERR};
    ASSERT_PARAMS(brdev && laterr);
    ASSERT_FOPEN(fd, brdev, frer_fn[fd.id], O_RDONLY);
    ASSERT_FRDENTRY(fd, laterr, fnidx);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_frer_seqrec_get_ptmask(struct port_hdl *brdev,
                                            unsigned int fnidx,
                                            uint16_t *port)
{
    tsn_file_desc fd = { .id = FRER_SEQREC_PTMASK};
    ASSERT_PARAMS(brdev && port);
    ASSERT_FOPEN(fd, brdev, frer_fn[fd.id], O_RDONLY);
    ASSERT_FRDENTRY(fd, port, fnidx);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_frer_seqrec_delport(struct port_hdl *port, uint16_t fnidx)
{
    tsn_file_desc fd = { .id = FRER_SEQREC_DELPORT};
    ASSERT_PARAMS(port);
    ASSERT_FOPEN(fd, port, frer_fn[fd.id], O_WRONLY);
    ASSERT_FPRINT(fd, "%"PRIu16, fnidx);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_frer_seqrec_delstr(struct port_hdl *brdev, uint16_t fnidx,
                                        const uint16_t strhdl)
{
    tsn_file_desc fd = { .id = FRER_SEQREC_DELSTR};
    ASSERT_PARAMS(brdev);
    ASSERT_FOPEN(fd, brdev, frer_fn[fd.id], O_WRONLY);
    ASSERT_FWRENTRY(fd, strhdl, fnidx);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_frer_seqrec_delfn(struct port_hdl *brdev, uint16_t fnidx)
{
    tsn_file_desc fd = { .id = FRER_SEQREC_DELFN};
    ASSERT_PARAMS(brdev);
    ASSERT_FOPEN(fd, brdev, frer_fn[fd.id], O_WRONLY);
    ASSERT_FPRINT(fd, "%"PRIu16, fnidx);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_frer_seqrec_fn2sid(struct port_hdl *brdev,
                                        unsigned int idx,
                                        uint16_t *strlist, uint32_t cnt)
{
    tsn_file_desc fd = { .id = FRER_SEQREC_FN2SID};
    ASSERT_PARAMS(brdev && strlist);
    ASSERT_FOPEN(fd, brdev, frer_fn[fd.id], O_RDONLY);
    ASSERT_FRDSUBLIST(fd, strlist, idx, cnt, cnt);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_frer_seqrec_fn_cnt(struct port_hdl *brdev, uint16_t *cnt)
{
    tsn_file_desc fd = { .id = FRER_SEQREC_FN_CNT};
    ASSERT_PARAMS(brdev && cnt);
    ASSERT_FOPEN(fd, brdev, frer_fn[fd.id], O_RDONLY);
    ASSERT_FSCAN(fd, "%"SCNu16, 1, cnt);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_frer_seqrec_list_fn(struct port_hdl *brdev, uint32_t cnt,
                            uint16_t *fn_list)
{
    tsn_file_desc fd = { .id = FRER_SEQREC_FN_LIST};
    ASSERT_PARAMS(brdev && fn_list);
    ASSERT_FOPEN_R(fd, brdev, frer_fn[fd.id]);
    ASSERT_FRDLIST(fd, fn_list, cnt);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_frer_seqrec_cnt_outoforder(struct port_hdl *port,
                                                unsigned int fnidx,
                                                uint32_t *cnt)
{
    tsn_file_desc fd = { .id = FRER_SEQREC_CNT_OOO};
    ASSERT_PARAMS(port && cnt);
    ASSERT_FOPEN(fd, port, frer_fn[fd.id], O_RDONLY);
    ASSERT_FRDENTRY(fd, cnt, fnidx);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_frer_seqrec_cnt_tagless(struct port_hdl *port,
                                             unsigned int strhdl,
                                             uint32_t *cnt)
{
    tsn_file_desc fd = { .id = FRER_SEQREC_CNT_TAGLESS};
    ASSERT_PARAMS(port && cnt);
    ASSERT_FOPEN(fd, port, frer_fn[fd.id], O_RDONLY);
    ASSERT_FRDENTRY(fd, cnt, strhdl);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_frer_seqrec_cnt_passed(struct port_hdl *port,
                                            unsigned int strhdl,
                                            uint32_t *cnt)
{
    tsn_file_desc fd = { .id = FRER_SEQREC_CNT_PASSED};
    ASSERT_PARAMS(port && cnt);
    ASSERT_FOPEN(fd, port, frer_fn[fd.id], O_RDONLY);
    ASSERT_FRDENTRY(fd, cnt, strhdl);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_frer_seqrec_cnt_discarded(struct port_hdl *port,
                                               unsigned int strhdl,
                                               uint32_t *cnt)
{
    tsn_file_desc fd = { .id = FRER_SEQREC_CNT_DISCARDED};
    ASSERT_PARAMS(port && cnt);
    ASSERT_FOPEN(fd, port, frer_fn[fd.id], O_RDONLY);
    ASSERT_FRDENTRY(fd, cnt, strhdl);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_frer_seqrec_cnt_rogue(struct port_hdl *port,
                                           unsigned int strhdl,
                                           uint32_t *cnt)
{
    tsn_file_desc fd = { .id = FRER_SEQREC_CNT_ROGUE};
    ASSERT_PARAMS(port && cnt);
    ASSERT_FOPEN(fd, port, frer_fn[fd.id], O_RDONLY);
    ASSERT_FRDENTRY(fd, cnt, strhdl);
    ASSERT_FCLOSE(fd);
}
tsn_return_value tsn_frer_seqrec_cnt_lost(struct port_hdl *brdev,
                                          unsigned int fnidx,
                                          uint32_t *cnt)
{
    tsn_file_desc fd = { .id = FRER_SEQREC_CNT_LOST};
    ASSERT_PARAMS(brdev && cnt);
    ASSERT_FOPEN(fd, brdev, frer_fn[fd.id], O_RDONLY);
    ASSERT_FRDENTRY(fd, cnt, fnidx);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_frer_seqrec_cnt_resets(struct port_hdl *brdev,
                                            unsigned int fnidx,
                                            uint32_t *cnt)
{
    tsn_file_desc fd = { .id = FRER_SEQREC_CNT_RESETS};
    ASSERT_PARAMS(brdev && cnt);
    ASSERT_FOPEN(fd, brdev, frer_fn[fd.id], O_RDONLY);
    ASSERT_FRDENTRY(fd, cnt, fnidx);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_frer_seqenc_create(struct port_hdl *port, uint8_t dir)
{
    tsn_file_desc fd = { .id = FRER_SEQENC_CREATE};
    ASSERT_PARAMS(port);
    ASSERT_FOPEN(fd, port, frer_fn[fd.id], O_WRONLY);
    ASSERT_FPRINT(fd, "%"PRIu8, dir);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_frer_seqenc_getdir(struct port_hdl *port,
                                        uint8_t *direction)
{
    tsn_file_desc fd = { .id = FRER_SEQENC_CREATE};
    ASSERT_PARAMS(port && direction);
    ASSERT_FOPEN(fd, port, frer_fn[fd.id], O_RDONLY);
    ASSERT_FSCAN(fd, "%"SCNu8, 1, direction);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_frer_seqenc_setactive(struct port_hdl *port,
                                           uint8_t active)
{
    tsn_file_desc fd = { .id = FRER_SEQENC_ACTIVE};
    ASSERT_PARAMS(port);
    ASSERT_FOPEN(fd, port, frer_fn[fd.id], O_RDWR);
    ASSERT_FPRINT(fd, "%"PRIu8, active);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_frer_seqenc_settype(struct port_hdl *port, uint8_t type)
{
    tsn_file_desc fd = { .id = FRER_SEQENC_TYPE};
    ASSERT_PARAMS(port);
    ASSERT_FOPEN(fd, port, frer_fn[fd.id], O_RDWR);
    ASSERT_FPRINT(fd, "%"PRIu8, type);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_frer_seqenc_getactive(struct port_hdl *port,
                                           uint32_t *active)
{
    tsn_file_desc fd = { .id = FRER_SEQENC_ACTIVE};
    ASSERT_PARAMS(port && active);
    ASSERT_FOPEN(fd, port, frer_fn[fd.id], O_RDONLY);
    ASSERT_FSCAN(fd, "%"SCNu32, 1, active);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_frer_seqenc_gettype(struct port_hdl *port, uint32_t *type)
{
    tsn_file_desc fd = { .id = FRER_SEQENC_TYPE};
    ASSERT_PARAMS(port && type);
    ASSERT_FOPEN(fd, port, frer_fn[fd.id], O_RDONLY);
    ASSERT_FSCAN(fd, "%"SCNu32, 1, type);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_frer_seqenc_add_str(struct port_hdl *port, uint16_t strhdl)
{
    tsn_file_desc fd = { .id = FRER_SEQENC_ADDSTR};
    ASSERT_PARAMS(port);
    ASSERT_FOPEN(fd, port, frer_fn[fd.id], O_WRONLY);
    ASSERT_FPRINT(fd, "%"PRIu16, strhdl);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_frer_seqenc_delstr(struct port_hdl *port, uint16_t strhdl)
{
    tsn_file_desc fd = { .id = FRER_SEQENC_DELSTR};
    ASSERT_PARAMS(port);
    ASSERT_FOPEN(fd, port, frer_fn[fd.id], O_WRONLY);
    ASSERT_FPRINT(fd, "%"PRIu16, strhdl);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_frer_seqenc_delport(struct port_hdl *port)
{
    tsn_file_desc fd = { .id = FRER_SEQENC_DELPORT};
    ASSERT_PARAMS(port);
    ASSERT_FOPEN(fd, port, frer_fn[fd.id], O_WRONLY);
    ASSERT_FPRINT(fd, "%u", (unsigned int)0);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_frer_seqenc_port2str(struct port_hdl *port, uint32_t cnt,
                            uint16_t *strlist)
{
    tsn_file_desc fd = { .id = FRER_SEQENC_PORT2STR};
    ASSERT_PARAMS(port && strlist);
    ASSERT_FOPEN_R(fd, port, frer_fn[fd.id]);
    ASSERT_FRDLIST(fd, strlist, cnt);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_frer_seqenc_str2port(struct port_hdl *brdev,
                                          unsigned int strhdl,
                                          uint16_t *portmask)
{
    tsn_file_desc fd = { .id = FRER_SEQENC_STR2PORT};
    ASSERT_PARAMS(brdev && portmask);
    ASSERT_FOPEN(fd, brdev, frer_fn[fd.id], O_RDONLY);
    ASSERT_FRDENTRY(fd, portmask, strhdl);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_frer_seqgen_create(struct port_hdl *brdev, uint16_t fnidx)
{
    tsn_file_desc fd = { .id = FRER_SEQGEN_CREATE};
    ASSERT_PARAMS(brdev);
    ASSERT_FOPEN(fd, brdev, frer_fn[fd.id], O_WRONLY);
    ASSERT_FPRINT(fd, "%"PRIu16, fnidx);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_frer_seqgen_add_strhdl(struct port_hdl *brdev,
                                            unsigned int idx,
                                            const uint16_t strhdl)
{
    tsn_file_desc fd = { .id = FRER_SEQGEN_ADD_STRHDL};
    ASSERT_PARAMS(brdev);
    ASSERT_FOPEN(fd, brdev, frer_fn[fd.id], O_WRONLY);
    ASSERT_FWRENTRY(fd, strhdl, idx);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_frer_seqgen_add_dir(struct port_hdl *brdev,
                                         unsigned int idx,
                                         const uint8_t dir)
{
    tsn_file_desc fd = { .id = FRER_SEQGEN_ADD_DIR};
    ASSERT_PARAMS(brdev);
    ASSERT_FOPEN(fd, brdev, frer_fn[fd.id], O_WRONLY);
    ASSERT_FWRENTRY(fd, dir, idx);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_frer_seqgen_del_strhdl(struct port_hdl *brdev,
                                            uint16_t strhdl)
{
    tsn_file_desc fd = { .id = FRER_SEQGEN_DEL_STRHDL};
    ASSERT_PARAMS(brdev);
    ASSERT_FOPEN(fd, brdev, frer_fn[fd.id], O_WRONLY);
    ASSERT_FPRINT(fd, "%"PRIu16, strhdl);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_frer_seqgen_del_fn(struct port_hdl *brdev, uint16_t fnidx)
{
    tsn_file_desc fd = { .id = FRER_SEQGEN_DEL_FN};
    ASSERT_PARAMS(brdev);
    ASSERT_FOPEN(fd, brdev, frer_fn[fd.id], O_WRONLY);
    ASSERT_FPRINT(fd, "%"PRIu16, fnidx);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_frer_seqgen_fn_cnt(struct port_hdl *brdev, uint16_t *cnt)
{
    tsn_file_desc fd = { .id = FRER_SEQGEN_FN_CNT};
    ASSERT_PARAMS(brdev && cnt);
    ASSERT_FOPEN(fd, brdev, frer_fn[fd.id], O_RDONLY);
    ASSERT_FSCAN(fd, "%"SCNu16, 1, cnt);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_frer_seqgen_list_fn(struct port_hdl *brdev, uint32_t cnt,
                                         uint16_t *fn_list)
{
    tsn_file_desc fd = { .id = FRER_SEQGEN_FN_LIST};
    ASSERT_PARAMS(brdev && fn_list);
    ASSERT_FOPEN_R(fd, brdev, frer_fn[fd.id]);
    ASSERT_FRDLIST(fd, fn_list, cnt);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_frer_seqgen_sid2fn(struct port_hdl *brdev,
                                        unsigned int idx,
                                        uint16_t *fn)
{
    tsn_file_desc fd = { .id = FRER_SEQGEN_SID2FN};
    ASSERT_PARAMS(brdev && fn);
    ASSERT_FOPEN(fd, brdev, frer_fn[fd.id], O_RDONLY);
    ASSERT_FRDENTRY(fd, fn, idx);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_frer_seqgen_fn2dir(struct port_hdl *brdev,
                                        unsigned int idx,
                                        uint8_t *dir)
{
    tsn_file_desc fd = { .id = FRER_SEQGEN_FN2DIR};
    ASSERT_PARAMS(brdev && dir);
    ASSERT_FOPEN(fd, brdev, frer_fn[fd.id], O_RDONLY);
    ASSERT_FRDENTRY(fd, dir, idx);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_frer_seqgen_fn2sid(struct port_hdl *brdev,
                                        unsigned int idx,
                                        uint16_t *strlist, uint32_t cnt)
{
    tsn_file_desc fd = { .id = FRER_SEQGEN_FN2SID};
    ASSERT_PARAMS(brdev && strlist);
    ASSERT_FOPEN(fd, brdev, frer_fn[fd.id], O_RDONLY);
    ASSERT_FRDSUBLIST(fd, strlist, idx, cnt, cnt);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_frer_get_max_ports(struct port_hdl *brdev,
                                        uint32_t *max_ports)
{
    tsn_file_desc fd = { .id = FRER_NUM_PORTS};
    ASSERT_PARAMS(brdev && max_ports);
    ASSERT_FOPEN(fd, brdev, frer_fn[fd.id], O_RDONLY);
    ASSERT_FSCAN(fd, "%"SCNu32, 1, max_ports);
    ASSERT_FCLOSE(fd);
}

const char *tsn_frer_get_known_ernno_msg(tsn_return_value return_code)
{
    (void) return_code;
    return read_lib_error_msg(E_TSN_LIB_NO_ERROR);
}

static uint32_t get_return_header(tsn_file_desc *fd)
{
    return ((tsn_module<<24)|((fd->id)<<16));
}
