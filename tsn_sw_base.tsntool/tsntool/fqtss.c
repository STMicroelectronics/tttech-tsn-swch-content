/*
 * TTTech TSN Command Line Tool (tsntool)
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

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include "tsn_fqtss.h"
#include "tsntool.h"
#include "tsn_def.h"

#define _FQTSS_STRICT 0
#define _FQTSS_CBS    1

// slope <tc> <bitrate> <interface>
static uint32_t do_fqtss_set_slope(int argc, char **argv) {
    struct port_hdl *phdl;
    int long long tc, bitrate;
    uint32_t ret = 0u;

    assert_param_cnt(argc, 3);

    assert_param_parse(sscanf(argv[0], "%lld", &tc) != 1);
    if ((tc != (uint8_t) tc) || !(tc < MAX_NO_TRAFFIC_CLASSES)) {
        pr_error("Traffic Class out of range [0..7]\n");
        return ERANGE;
    }
    assert_param_parse(sscanf(argv[1], "%lld", &bitrate) != 1);
    if (bitrate < 0) {
        pr_error("BITRATE out of range \n");
        return ERANGE;
    }

    phdl = get_if(argv[2]);
    if ((ret = tsn_fqtss_set_admin_slope_table(phdl, (uint8_t) tc, (uint64_t) bitrate)) != 0u)
        pr_error("Cannot set admin idle slope - %d (%s)\n", errno, strerror(errno));
    release_if(phdl);
    return ret;
}

static uint32_t do_fqtss_set_tsa(int argc, char **argv) {
    struct port_hdl *phdl;
    int long long tc;
    uint32_t ret = 0u;
    uint8_t tsa = UINT8_MAX;

    assert_param_cnt(argc, 3);

    assert_param_parse(sscanf(argv[0], "%lld", &tc) != 1);
    if ((tc != (uint8_t) tc) || !(tc < MAX_NO_TRAFFIC_CLASSES)) {
        pr_error("Traffic Class out of range [0..7]\n");
        return ERANGE;
    }

    if (!strcmp("strict", argv[1]))
        tsa = _FQTSS_STRICT;
    else if (!strcmp("cbs", argv[1]))
        tsa = _FQTSS_CBS;
    else
        assert_param_parse(true);

    phdl = get_if(argv[2]);
    if ((ret = tsn_fqtss_set_tx_selection_table(phdl, (uint8_t) tc, tsa)) != 0u)
        pr_error("Cannot set Transmission Selection Algorithm - %d (%s)\n", errno, strerror(errno));
    release_if(phdl);
    return ret;
}

// show <interface>
static uint32_t do_fqtss_show_slopes(int argc, char **argv) {
    struct port_hdl *phdl;
    uint32_t ret = 0u;

    assert_param_cnt(argc, 1);
    phdl = get_if(argv[0]);

    pr_output("Traffic Class        Algorithm            operIdleSlope\n");
    for (unsigned int tc = 0; tc < MAX_NO_TRAFFIC_CLASSES; tc++) {
        uint64_t ois;
        uint8_t tis;

        ret += tsn_fqtss_get_tx_selection_table(phdl, tc, &tis);
        ret += tsn_fqtss_get_oper_slope_table(phdl, tc, &ois);

        if (ret == 0u) {
            if (tis == _FQTSS_STRICT)
                pr_output("      %u              Strict prio          n/a\n", tc);
            else if (tis == _FQTSS_CBS)
                pr_output("      %u              CBS                  %" SCNu64 " bps\n", tc, ois);
        }
    }

    release_if(phdl);
    return ret;
}

static uint32_t do_fqtss_help(int argc, char **argv) {
    pr_output("Usage: %s show DEV\n", _context);
    pr_output("       %s slope TC BITRATE DEV\n", _context);
    pr_output("       %s tsa TC {strict | cbs} DEV\n", _context);
    (void)argc;
    (void)argv;
    exit(0);
}

static const struct cmd fqtss_cmds[] = {
    { "slope", do_fqtss_set_slope },
    { "show", do_fqtss_show_slopes },
    { "tsa", do_fqtss_set_tsa },
    { "help", do_fqtss_help },
    NO_MORE_CMDS
};

uint32_t do_fqtss(int argc, char **argv) {
    return do_cmd(argc, argv, fqtss_cmds);
}
