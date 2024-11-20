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
#include "tsn_fqtss.h"

#define IEEE8021FQTSS_DIR "ieee8021Fqtss/"

static const uint32_t tsn_module = TSN_FQTSS;
static uint32_t get_return_header(tsn_file_desc *fp);

static const sysfs_fn fqtss_fn[MAX_TSN_FQTSS_FUNCTIONS] = {
    [ADMIN_IDLE_SLOPE_TABLE]    = IEEE8021FQTSS_DIR"adminIdleSlopeTable",
    [OPER_IDLE_SLOPE_TABLE]     = IEEE8021FQTSS_DIR"operIdleSlopeTable",
    [TX_SEL_ALGORITHM_TABLE]    = IEEE8021FQTSS_DIR"TxSelectionAlgorithmTable",
};

tsn_return_value tsn_fqtss_get_tx_selection_table(struct port_hdl *port,
                                                  unsigned int prio,
                                                  uint8_t *txsel_sentry)
{
    tsn_file_desc fd = { .id = TX_SEL_ALGORITHM_TABLE};
    ASSERT_PARAMS(port && txsel_sentry);
    ASSERT_FOPEN(fd, port, fqtss_fn[fd.id], O_RDONLY);
    ASSERT_FRDENTRY(fd, txsel_sentry, prio);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_fqtss_set_tx_selection_table(struct port_hdl *port,
                                                  unsigned int prio,
                                                  const uint8_t txsel_sentry)
{
    tsn_file_desc fd = { .id = TX_SEL_ALGORITHM_TABLE};
    ASSERT_PARAMS(port);
    ASSERT_FOPEN(fd, port, fqtss_fn[fd.id], O_RDWR);
    ASSERT_FWRENTRY(fd, txsel_sentry, prio);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_fqtss_get_admin_slope_table(struct port_hdl *port,
                                                 unsigned int prio,
                                                 uint64_t *admin_sentry)
{
    tsn_file_desc fd = { .id = ADMIN_IDLE_SLOPE_TABLE};
    ASSERT_PARAMS(port && admin_sentry);
    ASSERT_FOPEN(fd, port, fqtss_fn[fd.id], O_RDONLY);
    ASSERT_FRDENTRY(fd, admin_sentry, prio);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_fqtss_set_admin_slope_table(struct port_hdl *port,
                                                 unsigned int prio,
                                                 const uint64_t admin_sentry)
{
    tsn_file_desc fd = { .id = ADMIN_IDLE_SLOPE_TABLE};
    ASSERT_PARAMS(port);
    ASSERT_FOPEN(fd, port, fqtss_fn[fd.id], O_RDWR);
    ASSERT_FWRENTRY(fd, admin_sentry, prio);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_fqtss_get_oper_slope_table(struct port_hdl *port,
                                                unsigned int prio,
                                                uint64_t *oper_sentry)
{
    tsn_file_desc fd = { .id = OPER_IDLE_SLOPE_TABLE};
    ASSERT_PARAMS(port && oper_sentry);
    ASSERT_FOPEN(fd, port, fqtss_fn[fd.id], O_RDONLY);
    ASSERT_FRDENTRY(fd, oper_sentry, prio);
    ASSERT_FCLOSE(fd);
}

const char *tsn_fqtss_get_known_ernno_msg(tsn_return_value return_code)
{
    (void) return_code;
    return read_lib_error_msg(E_TSN_LIB_NO_ERROR);
}

static uint32_t get_return_header(tsn_file_desc *fd)
{
    return (tsn_module<<24|((fd->id)<<16));
}
