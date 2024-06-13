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

#include "tsntool.h"
#include "tsn_preemp.h"
#include "tsn_bridge.h"

static uint32_t do_preempt_show(int argc, char **argv) {
    struct port_hdl *phdl = NULL;
    uint32_t ret = 0u;
    unsigned i;

    // 802.1Qbu objs
    uint32_t hold_adv = 0, rel_adv = 0, hold_req = 0, active = 0, tcs = 0;
    preemp_stat pree_stat[8];

    // 802.3 objs
    bool support = false, tx_en = false, verify_tx_dis = false;
    uint8_t time_ms = 0u;
    preemp_tx_stat status_tx = TXSTAT_INVALID;
    preemp_fragsz frag_size = FRAG_UNKNOWN;
    preemp_vrfy_stat verify_status = VRFY_UNKNOWN;

    // 802.3 counters
    uint32_t asserr_cnt = 0, smderr_cnt = 0, assok_cnt = 0, fragrx_cnt = 0, fragtx_cnt = 0, hold_cnt = 0;

    assert_param_cnt(argc, 1);
    phdl = get_if(argv[0]);

    ret |= tsn_preemp_get_hold_advance(phdl, &hold_adv);
    ret |= tsn_preemp_get_release_advance(phdl, &rel_adv);
    ret |= tsn_preemp_get_hold_request(phdl, &hold_req);
    ret |= tsn_preemp_get_preemption_active(phdl, &active);
    ret |= tsn_bridge_get_num_traffic_classes(phdl, &tcs);
    for (i = 0; i < tcs; i++)
        tsn_preemp_get_status_table(phdl, i, &pree_stat[i]);

    ret |= tsn_preemp_get_support(phdl, &support);
    ret |= tsn_preemp_get_verify_status(phdl, &verify_status);
    ret |= tsn_preemp_get_tx_enabled(phdl, &tx_en);
    ret |= tsn_preemp_get_verify_tx_disabled(phdl, &verify_tx_dis);
    ret |= tsn_preemp_get_status_tx(phdl, &status_tx);
    ret |= tsn_preemp_get_verify_time(phdl, &time_ms);
    ret |= tsn_preemp_get_frag_size(phdl, &frag_size);
    ret |= tsn_preemp_get_asserr_cnt(phdl, &asserr_cnt);
    ret |= tsn_preemp_get_smderr_cnt(phdl, &smderr_cnt);
    ret |= tsn_preemp_get_assok_cnt(phdl, &assok_cnt);
    ret |= tsn_preemp_get_fragrx_cnt(phdl, &fragrx_cnt);
    ret |= tsn_preemp_get_fragtx_cnt(phdl, &fragtx_cnt);
    ret |= tsn_preemp_get_hold_cnt(phdl, &hold_cnt);

    if (ret != 0)
        pr_error("\n *** WARNING: not all necessary objects could be retrieved\n\n");

    pr_output("  Preemption active:    %s\n", (active == ACTIVE) ? "YES" : "NO");
    pr_output("  hold/release advance: %u/%u ns\n", hold_adv, rel_adv);
    pr_output("  hold request status:  ");
    switch(hold_req) {
        case 1:
            pr_output("HOLD\n"); break;
        case 2:
            pr_output("RELEASE\n"); break;
        default:
            pr_output("unknown\n"); break;
    }
    pr_output("  Preemption Status Table:\n");
    pr_output("  Traffic class   Status\n");
    for (i = 0; i < tcs; i++) {
        pr_output("     %u              ", i);
        switch (pree_stat[i]) {
            case PREEMPT:
                pr_output("PREEMPT\n"); break;
            case EXPRESS:
                pr_output("EXPRESS\n"); break;
            default:
                pr_output("unknown\n");
        }
    }
    pr_output("  MAC Merge Entity Status:\n");
    pr_output("    aMACMergeSupport:            %s\n", (support) ? "YES" : "NO");
    pr_output("    aMACMergeStatusVerify:       ");
    switch(verify_status) {
        case VRFY_UNKNOWN:
            pr_output("unknown\n"); break;
        case VRFY_INITIAL:
            pr_output("initial\n"); break;
        case VRFY_VERIFYING:
            pr_output("verifying\n"); break;
        case VRFY_SUCCEEDED:
            pr_output("succeeded\n"); break;
        case VRFY_FAILED:
            pr_output("failed\n"); break;
        case VRFY_DISABLED:
            pr_output("disabled\n"); break;
        default:
            pr_output("<invalid status>\n");
    }
    pr_output("    aMACMergeEnableTx:           %s\n", (tx_en) ? "enabled" : "disabled");
    pr_output("    aMACMergeVerifyDisableTx:    %s\n", (verify_tx_dis) ? "disabled" : "enabled");
    pr_output("    aMACMergeStatusTx:           ");
    switch(status_tx) {
        case TXSTAT_UNKNOWN:
            pr_output("unknown\n"); break;
        case TXSTAT_ACTIVE:
            pr_output("active\n"); break;
        case TXSTAT_INACTIVE:
            pr_output("inactive\n"); break;
        default:
            pr_output("<invalid status>\n");
    }

    pr_output("    aMACMergeVerifyTime:         %u ms\n", time_ms);
    pr_output("    aMACMergeAddFragSize:        ");
    switch (frag_size) {
        case FRAG_64:
            pr_output("64B\n"); break;
        case FRAG_128:
            pr_output("128B\n"); break;
        case FRAG_192:
            pr_output("192B\n"); break;
        case FRAG_256:
            pr_output("256B\n"); break;
        default:
            pr_output("unknown\n");
    }
    pr_output("  MAC Merge Entity Counters\n");
    pr_output("    aMACMergeFrameAssErrorCount: %u\n", asserr_cnt);
    pr_output("    aMACMergeFrameSmdErrorCount: %u\n", smderr_cnt);
    pr_output("    aMACMergeFrameAssOkCount:    %u\n", assok_cnt);
    pr_output("    aMACMergeFragCountRx:        %u\n", fragrx_cnt);
    pr_output("    aMACMergeFragCountTx:        %u\n", fragtx_cnt);
    pr_output("    aMACMergeHoldCount:          %u\n", hold_cnt);

    release_if(phdl);
    return ret;
}

static uint32_t do_preempt_setq(int argc, char **argv) {
    struct port_hdl *phdl = NULL;
    preemp_stat req = UNKNOWN;
    int long long tc;
    uint32_t ret = 0u;

    assert_param_cnt(argc, 3);

    assert_param_parse(sscanf(argv[0], "%lld" , &tc) != 1);
    if (tc != (uint8_t) tc) {
        pr_error("Cannot set preemption status for Traffic Class %lld \n", tc);
        return EINVAL;
    }

    if (!strcmp(argv[1], "express"))
        req = EXPRESS;
    else if (!strcmp(argv[1], "preempt"))
        req = PREEMPT;
    else
        assert_param_parse(true);

    phdl = get_if(argv[2]);

    if ((ret = tsn_preemp_set_status_table(phdl, (uint8_t) tc, req)))
        pr_error("Cannot set preemption status for Traffic Class %u - %d (%s)\n",
                 (uint8_t) tc, errno, strerror(errno));

    release_if(phdl);
    return ret;
}

static uint32_t do_preempt_setfrag(int argc, char **argv) {
    struct port_hdl *phdl = NULL;
    preemp_fragsz fragsz = FRAG_UNKNOWN;
    int long long frag;
    uint32_t ret = 0u;

    assert_param_cnt(argc, 2);
    assert_param_parse(sscanf(argv[0], "%lld" , &frag) != 1);

    switch(frag) {
        case 64:
            fragsz = FRAG_64; break;
        case 128:
            fragsz = FRAG_128; break;
        case 192:
            fragsz = FRAG_192; break;
        case 256:
            fragsz = FRAG_256; break;
        default:
            assert_param_parse(true);
    }

    phdl = get_if(argv[1]);

    if ((ret = tsn_preemp_set_frag_size(phdl, fragsz)))
        pr_error("Cannot set fragment size - %d (%s)\n", errno, strerror(errno));

    release_if(phdl);
    return ret;
}

static uint32_t do_preempt_enable(int argc, char **argv) {
    struct port_hdl *phdl = NULL;
    bool verify_tx_dis = false;;
    int long long time_ms;
    uint32_t ret = 0u;

    assert_param_cnt(argc, 3);
    if (!strcmp(argv[0], "verify"))
        verify_tx_dis = false;
    else if (!strcmp(argv[0], "noverify"))
        verify_tx_dis = true;
    else
        assert_param_parse(true);

    assert_param_parse(sscanf(argv[1], "%lld" , &time_ms) != 1);

    if (time_ms != (uint8_t)time_ms) {
        pr_error("Cannot set verify time %lld \n", time_ms);
        return EINVAL;
    }

    phdl = get_if(argv[2]);

    if ((ret = tsn_preemp_set_verify_tx_disable(phdl, verify_tx_dis)))
        pr_error("Cannot enable TX verification - %d (%s)\n", errno, strerror(errno));
    else if ((ret = tsn_preemp_set_verify_time(phdl, (uint8_t) time_ms)))
        pr_error("Cannot set verify time %d - %d (%s)\n", (uint8_t) time_ms, errno, strerror(errno));
    else if ((ret = tsn_preemp_set_tx_enable(phdl, true)))
        pr_error("Cannot disable TX preemption - %d (%s)\n", errno, strerror(errno));

    release_if(phdl);
    return ret;
}

static uint32_t do_preempt_disable(int argc, char **argv) {
    struct port_hdl *phdl = NULL;
    uint32_t ret = 0u;

    assert_param_cnt(argc, 1);
    phdl = get_if(argv[0]);

    if ((ret = tsn_preemp_set_verify_tx_disable(phdl, false)))
        pr_error("Cannot enable TX verification - %d (%s)\n", errno, strerror(errno));
    else if ((ret = tsn_preemp_set_tx_enable(phdl, false)))
        pr_error("Cannot disable TX preemption - %d (%s)\n", errno, strerror(errno));

    release_if(phdl);
    return ret;
}

static uint32_t do_preempt_help(int argc, char **argv) {
    pr_output("Usage: %s enable [verify | noverify] VERIFYTIME DEV\n", _context);
    pr_output("          where TIME is in milliseconds (1..128)\n");
    pr_output("       %s disable DEV\n", _context);
    pr_output("       %s setq TC [express | preempt] DEV\n", _context);
    pr_output("       %s setfragsize FRAGSIZE DEV\n", _context);
    pr_output("          where FRAGSIZE is one of 64, 128, 192, or 256\n");
    pr_output("       %s show DEV \n", _context);
    (void)argc;
    (void)argv;
    exit(0);
}

static const struct cmd preempt_cmds[] = {
    { "show",    do_preempt_show },
    { "setq",    do_preempt_setq },
    { "setfrag", do_preempt_setfrag },
    { "enable",      do_preempt_enable },
    { "disable",     do_preempt_disable },
    { "help",    do_preempt_help },
    NO_MORE_CMDS
};

uint32_t do_preempt(int argc, char **argv) {
    return do_cmd(argc, argv, preempt_cmds);
}
