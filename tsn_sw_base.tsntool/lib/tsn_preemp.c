/*
 * TTTech TSN Library (libtsn)
 * Copyright(c) 2017 TTTech Industrial Automation AG.
 *
 * ALL RIGHTS RESERVED.
 * Usage of this software, including source code, netlists, documentation,
 * is subject to restrictions and conditions of the applicable license
 * agreement with TTTech Industrial Automation AG or its affiliates.
 *
 * All trademarks used are the property of their respective owners.
 *
 * TTTech Industrial Automation AG and its affiliates do not assume any liability
 * arising out of the application or use of any product described or shown
 * herein. TTTech Industrial Automation AG and its affiliates reserve the right to
 * make changes, at any time, in order to improve reliability, function or
 * design.
 *
 * Contact Information:
 * support@tttech-industrial.com
 * TTTech Industrial Automation AG, Schoenbrunnerstrasse 7, 1040 Vienna, Austria
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <inttypes.h>

#include "tsn_private.h"
#include "tsn_preemp.h"

#define IEEE8021PREEMP_DIR   "ieee8021Preemption/"
#define IEEE8023MACMERGE_DIR "ieee8023MacMerge/"

static const uint32_t tsn_module = TSN_PREEMP;

static const sysfs_fn preemp_fn[MAX_TSN_PREEMP_FUNCTIONS] = {
    [HOLD_ADVANCE]          = IEEE8021PREEMP_DIR"holdAdvance",
    [HOLD_REQUEST]          = IEEE8021PREEMP_DIR"holdRequest",
    [PREEMPTION_ACTIVE]     = IEEE8021PREEMP_DIR"preemptionActive",
    [RELEASE_ADVANCE]       = IEEE8021PREEMP_DIR"releaseAdvance",
    [STATUS_TABLE]          = IEEE8021PREEMP_DIR"framePreemptionStatusTable",
    [SUPPORT]               = IEEE8023MACMERGE_DIR"support",
    [VERIFY_STATUS]         = IEEE8023MACMERGE_DIR"statusVerify",
    [TX_EN]                 = IEEE8023MACMERGE_DIR"enableTx",
    [VERIFY_TX_DIS]         = IEEE8023MACMERGE_DIR"verifyDisableTx",
    [STATUS_TX]             = IEEE8023MACMERGE_DIR"statusTx",
    [TIME_MS]               = IEEE8023MACMERGE_DIR"verifyTime",
    [FRAG_SIZE]             = IEEE8023MACMERGE_DIR"addFragSize",
    [ASSERR_CNT]            = IEEE8023MACMERGE_DIR"frameAssErrCount",
    [SMDERR_CNT]            = IEEE8023MACMERGE_DIR"frameSmdErrorCount",
    [ASSOK_CNT]             = IEEE8023MACMERGE_DIR"frameAssOkCount",
    [FRAGRX_CNT]            = IEEE8023MACMERGE_DIR"fragCountRx",
    [FRAGTX_CNT]            = IEEE8023MACMERGE_DIR"fragCountTx",
    [HOLD_CNT]              = IEEE8023MACMERGE_DIR"holdCount",
};

tsn_return_value tsn_preemp_get_status_table(struct port_hdl *port,
                                             uint8_t prio,
                                             preemp_stat *preemp_status)
{
    uint8_t status = 0x01;
    tsn_file_desc fd = { .id = STATUS_TABLE};
    ASSERT_PARAMS(port && preemp_status);
    ASSERT_FOPEN(fd, port, preemp_fn[fd.id], O_RDONLY);
    ASSERT_FRDENTRY(fd, &status, prio);
    *preemp_status = (preemp_stat)status;
    if ((*preemp_status != EXPRESS) && (*preemp_status != PREEMPT)) {
        *preemp_status = UNKNOWN;
    }
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_preemp_set_status_table(struct port_hdl *port,
                                             uint8_t prio,
                                             const preemp_stat preemp_status)
{
    uint8_t status = (uint8_t)preemp_status;
    tsn_file_desc fd = { .id = STATUS_TABLE};
    ASSERT_PARAMS(port);
    ASSERT_FOPEN(fd, port, preemp_fn[fd.id], O_RDWR);
    ASSERT_FWRENTRY(fd, status, prio);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_preemp_get_hold_advance(struct port_hdl *port,
                                             uint32_t *hold_advance)
{
    tsn_file_desc fd = { .id = HOLD_ADVANCE};
    ASSERT_PARAMS(port && hold_advance);
    ASSERT_FOPEN(fd, port, preemp_fn[fd.id],O_RDONLY);
    ASSERT_FSCAN(fd, "%"SCNu32, 1, hold_advance);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_preemp_get_hold_request(struct port_hdl *port,
                                             uint32_t *hold_request)
{
    tsn_file_desc fd = { .id = HOLD_REQUEST};
    ASSERT_PARAMS(port && hold_request);
    ASSERT_FOPEN(fd, port, preemp_fn[fd.id], O_RDONLY);
    ASSERT_FSCAN(fd, "%"SCNu32, 1, hold_request);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_preemp_get_preemption_active(struct port_hdl *port,
                                                  frame_preemp_active *preemp_active)
{
    tsn_file_desc fd = { .id = PREEMPTION_ACTIVE};
    bool _preemp_active;
    ASSERT_PARAMS(port && preemp_active);
    ASSERT_FOPEN(fd, port, preemp_fn[fd.id], O_RDONLY);
    ASSERT_FSCAN(fd, "%"SCNu8, 1, (uint8_t *)&_preemp_active);
    *preemp_active = ((_preemp_active == true) ? ACTIVE : IDLE);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_preemp_get_release_advance(struct port_hdl *port,
                                                uint32_t *rel_advance)
{
    tsn_file_desc fd = { .id = RELEASE_ADVANCE};
    ASSERT_PARAMS(port && rel_advance);
    ASSERT_FOPEN(fd, port, preemp_fn[fd.id], O_RDONLY);
    ASSERT_FSCAN(fd, "%"SCNu32, 1, rel_advance);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_preemp_get_support(struct port_hdl *port, bool *support)
{
    tsn_file_desc fd = { .id = SUPPORT};
    bool _support;
    ASSERT_PARAMS(port && support);
    ASSERT_FOPEN(fd, port, preemp_fn[fd.id], O_RDONLY);
    ASSERT_FSCAN(fd, "%"SCNu8, 1, (uint8_t *)&_support);
    *support = _support;
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_preemp_get_verify_status(struct port_hdl *port,
                                              preemp_vrfy_stat *verify_status)
{
    tsn_file_desc fd = { .id = VERIFY_STATUS};
    ASSERT_PARAMS(port && verify_status);
    ASSERT_FOPEN(fd, port, preemp_fn[fd.id], O_RDONLY);
    ASSERT_FSCAN(fd, "%u", 1, verify_status); /* enums should not be bigger than unsigned int */
    if ((*verify_status < VRFY_UNKNOWN) || (*verify_status > VRFY_DISABLED))
        *verify_status = VRFY_INVALID;
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_preemp_get_tx_enabled(struct port_hdl *port, bool *tx_en)
{
    tsn_file_desc fd = { .id = TX_EN};
    bool _tx_en;
    ASSERT_PARAMS(port && tx_en);
    ASSERT_FOPEN(fd, port, preemp_fn[fd.id], O_RDONLY);
    ASSERT_FSCAN(fd, "%"SCNu8, 1, (uint8_t *)&_tx_en);
    *tx_en = _tx_en;
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_preemp_set_tx_enable(struct port_hdl *port,
                                          const bool tx_en)
{
    tsn_file_desc fd = { .id = TX_EN};
    ASSERT_PARAMS(port);
    ASSERT_FOPEN(fd, port, preemp_fn[fd.id], O_RDWR);
    ASSERT_FPRINT(fd, "%"PRIu8, (uint8_t)tx_en);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_preemp_get_verify_tx_disabled(struct port_hdl *port,
                                                   bool *verify_tx_dis)
{
    tsn_file_desc fd = { .id = VERIFY_TX_DIS};
    bool _verify_tx_dis;
    ASSERT_PARAMS(port && verify_tx_dis);
    ASSERT_FOPEN(fd, port, preemp_fn[fd.id], O_RDONLY);
    ASSERT_FSCAN(fd, "%"SCNu8, 1, (uint8_t *)&_verify_tx_dis);
    *verify_tx_dis = _verify_tx_dis;
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_preemp_set_verify_tx_disable(struct port_hdl *port,
                                                  const bool verify_tx_dis)
{
    tsn_file_desc fd = { .id = VERIFY_TX_DIS};
    ASSERT_PARAMS(port);
    ASSERT_FOPEN(fd, port, preemp_fn[fd.id], O_RDWR);
    ASSERT_FPRINT(fd, "%"PRIu8, (uint8_t)verify_tx_dis);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_preemp_get_status_tx(struct port_hdl *port,
                                          preemp_tx_stat *status_tx)
{
    tsn_file_desc fd = { .id = STATUS_TX};
    ASSERT_PARAMS(port && status_tx);
    ASSERT_FOPEN(fd, port, preemp_fn[fd.id], O_RDONLY);
    ASSERT_FSCAN(fd, "%u", 1, status_tx); /* enums should not be bigger than unsigned int */
    if ((*status_tx < TXSTAT_UNKNOWN) || (*status_tx > TXSTAT_ACTIVE))
        *status_tx = TXSTAT_INVALID;
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_preemp_get_verify_time(struct port_hdl *port,
                                            uint8_t *time_ms)
{
    tsn_file_desc fd = { .id  = TIME_MS};
    ASSERT_PARAMS(port && time_ms);
    ASSERT_FOPEN(fd, port, preemp_fn[fd.id], O_RDONLY);
    ASSERT_FSCAN(fd, "%"SCNu8, 1, time_ms);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_preemp_set_verify_time(struct port_hdl *port,
                                            const uint8_t time_ms)
{
    tsn_file_desc fd = { .id  = TIME_MS};
    ASSERT_PARAMS(port);
    ASSERT_FOPEN(fd, port, preemp_fn[fd.id], O_RDWR);
    ASSERT_FPRINT(fd, "%"PRIu8, time_ms);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_preemp_get_frag_size(struct port_hdl *port,
                                          preemp_fragsz *frag_size)
{
    tsn_file_desc fd = { .id  = FRAG_SIZE};;
    ASSERT_PARAMS(port && frag_size);
    ASSERT_FOPEN(fd, port, preemp_fn[fd.id], O_RDONLY);
    ASSERT_FSCAN(fd, "%u", 1, frag_size); /* enums should not be bigger than unsigned int */
    if ((*frag_size < FRAG_64) || (*frag_size > FRAG_256))
        *frag_size = FRAG_UNKNOWN;
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_preemp_set_frag_size(struct port_hdl *port,
                                          const preemp_fragsz frag_size)
{
    tsn_file_desc fd = { .id  = FRAG_SIZE};
    ASSERT_PARAMS(port);
    ASSERT_FOPEN(fd, port, preemp_fn[fd.id], O_RDWR);
    ASSERT_FPRINT(fd, "%u", (unsigned int)frag_size); /* enums should not be bigger than unsigned int */
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_preemp_get_asserr_cnt(struct port_hdl *port,
                                           uint32_t *asserr_cnt)
{
    tsn_file_desc fd = { .id  = ASSERR_CNT};
    ASSERT_PARAMS(port && asserr_cnt);
    ASSERT_FOPEN(fd, port, preemp_fn[fd.id], O_RDONLY);
    ASSERT_FSCAN(fd, "%"SCNu32, 1, asserr_cnt);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_preemp_get_smderr_cnt(struct port_hdl *port,
                                           uint32_t *smderr_cnt)
{
    tsn_file_desc fd = { .id = SMDERR_CNT};
    ASSERT_PARAMS(port && smderr_cnt);
    ASSERT_FOPEN(fd, port, preemp_fn[fd.id], O_RDONLY);
    ASSERT_FSCAN(fd, "%"SCNu32, 1, smderr_cnt);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_preemp_get_assok_cnt(struct port_hdl *port,
                                          uint32_t *assok_cnt)
{
    tsn_file_desc fd = { .id = ASSOK_CNT};
    ASSERT_PARAMS(port && assok_cnt);
    ASSERT_FOPEN(fd, port, preemp_fn[fd.id], O_RDONLY);
    ASSERT_FSCAN(fd, "%"SCNu32, 1, assok_cnt);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_preemp_get_fragrx_cnt(struct port_hdl *port,
                                           uint32_t *fragrx_cnt)
{
    tsn_file_desc fd = { .id = FRAGRX_CNT};
    ASSERT_PARAMS(port && fragrx_cnt);
    ASSERT_FOPEN(fd, port, preemp_fn[fd.id], O_RDONLY);
    ASSERT_FSCAN(fd, "%"SCNu32, 1, fragrx_cnt);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_preemp_get_fragtx_cnt(struct port_hdl *port,
                                           uint32_t *fragtx_cnt)
{
    tsn_file_desc fd = { .id = FRAGTX_CNT};
    ASSERT_PARAMS(port && fragtx_cnt);
    ASSERT_FOPEN(fd, port, preemp_fn[fd.id], O_RDONLY);
    ASSERT_FSCAN(fd, "%"SCNu32, 1, fragtx_cnt);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_preemp_get_hold_cnt(struct port_hdl *port,
                                         uint32_t *hold_cnt)
{
    tsn_file_desc fd = { .id = FRAGTX_CNT};
    ASSERT_PARAMS(port && hold_cnt);
    ASSERT_FOPEN(fd, port, preemp_fn[fd.id], O_RDONLY);
    ASSERT_FSCAN(fd, "%"SCNu32, 1, hold_cnt);
    ASSERT_FCLOSE(fd);
}

const char *tsn_preemp_get_known_ernno_msg(tsn_return_value return_code)
{
    (void) return_code;
    return read_lib_error_msg(E_TSN_LIB_NO_ERROR);
}

static uint32_t get_return_header(tsn_file_desc *fd)
{
    return ((tsn_module<<24)|((fd->id)<<16));
}
