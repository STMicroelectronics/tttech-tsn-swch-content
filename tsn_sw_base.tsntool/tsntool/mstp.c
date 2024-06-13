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

#include "tsn_def.h"
#include "tsn_qbridge.h"
#include "tsn_mstp.h"
#include "tsntool.h"

static const char* const port_state_to_string[MAX_PORT_STATES] = {
    [PORT_DISABLED]        = "disabled",
    [PORT_LISTENING]       = "listening",
    [PORT_LEARNING]        = "learning",
    [PORT_FORWARDING]      = "forwarding",
    [PORT_BLOCKING]        = "blocking",
};

static uint32_t do_mstp_fid2mstid(int argc, char **argv) {
    struct port_hdl *phdl;
    uint32_t mstid, max_vlans, ret = 0u;
    int long long fid;
    phdl = NULL;

    if (argc > 0) {
        if (!strcmp("show", argv[0])) {
            assert_param_cnt(argc, 3);
            assert_param_parse(sscanf(argv[1], "%lld", &fid) != 1);
            if (fid == (uint32_t) fid) {
                phdl = get_if(argv[2]);
                if ((ret = tsn_qbridge_get_max_supported_vlans(phdl, &max_vlans)) != 0u)
                    pr_error("Cannot get maximum number of supported FIDs"
                            " - %d (%s)\n", errno, strerror(errno));
                else  {
                    if (fid > max_vlans - 1) {
                        pr_error("FID %lld is out of range (maximum FID is %u) for "
                                "bridge '%s'\n", fid, max_vlans - 1, tsn_get_name_by_hdl(phdl));
                        ret = ERANGE;
                    }
                    else if ((ret = tsn_mstp_get_fid_to_msti(phdl, (uint32_t) fid, &mstid)) != 0u) {
                        pr_error("Cannot get FID %lld information"
                                " for bridge '%s'\n", fid, tsn_get_name_by_hdl(phdl));
                        pr_error("  Please check if FID %lld is configured "
                                "on bridge '%s'\n", fid, tsn_get_name_by_hdl(phdl));
                    }
                    else
                        pr_output("FID %lld (allocated to MSTID %u) \n", fid, mstid);
                }
                release_if(phdl);
            }
            else {
                pr_error("FID %lld is out of range\n", fid);
                ret = ERANGE;
            }
        }
        else
            assert_param_parse(true);
    }
    else
        assert_param_parse(true);
    return ret;
}

static uint32_t do_mstp_list(int argc, char **argv) {
    struct port_hdl *phdl;
    uint32_t max_mstids, ret = 0u;
    struct msti_list_entry *msti_list, *msti_entry;
    phdl = NULL;

    assert_param_cnt(argc, 2);

    if (!strcmp("show", argv[0])) {
        phdl = get_if(argv[1]);
        if ((ret = tsn_mstp_get_max_msti(phdl, &max_mstids)) != 0u)
            pr_error("Cannot get maximum number of supported MSTIDs"
                            " - %d (%s)\n", errno, strerror(errno));
        else  {
            if (!(msti_list = malloc(sizeof(*msti_list) * max_mstids))) {
                pr_error("Cannot allocate memory for msti list\n");
                ret = ENOMEM;
            }
            else if ((ret = tsn_mstp_get_msti_list(phdl, msti_list)) != 0u)
                pr_error("Cannot retrieve MSTID list for bridge: %s\n", tsn_get_name_by_hdl(phdl));
            else {
                uint32_t j;
                if (msti_list->mstid < max_mstids) {
                    pr_output("Allocated MSTIDs list: ");
                    msti_entry = msti_list;
                    for (j = 0; j < max_mstids; j++) {
                        if (msti_entry->mstid == 0xFFF)
                            break; // no more valid MSTIDs in list
                        pr_output("%u ", (msti_entry++)->mstid);
                    }
                    pr_output("\n");
                }
                else
                    pr_output("Allocated MSTIDs list: none\n");
                free(msti_list);
            }
        }
        release_if(phdl);
    }
    else
        assert_param_parse(true);

    return ret;
}

static uint32_t do_mstp_portstate(int argc, char **argv) {
    struct port_hdl *phdl;
    int long long mstid;
    uint32_t ret = 0u;
    port_mstp_state port_state;
    phdl = NULL;

    if (argc > 0) {
        if (!strcmp("show", argv[0])) {
            assert_param_cnt(argc, 3);
            assert_param_parse(sscanf(argv[1], "%lld", &mstid) != 1);
            if (mstid == (uint32_t) mstid) {
                phdl = get_if(argv[2]);
                if ((ret = tsn_mstp_get_port_state(phdl, (uint32_t) mstid, &port_state)) != 0u) {
                        pr_error("Cannot get MSTID %lld information"
                                    " for port '%s'\n", mstid, tsn_get_name_by_hdl(phdl));
                        pr_error("  Please check if MSTID %lld is configured "
                                    "on the bridge master of port '%s'\n", mstid, tsn_get_name_by_hdl(phdl));
                }
                else
                    pr_output("Port state is %s for MSTID %lld\n", port_state_to_string[port_state], mstid);
                release_if(phdl);
            }
            else {
                pr_error("mstid %lld is out of range\n", mstid);
                return ERANGE;
            }
        }
        else
            assert_param_parse(true);
    }
    else
        assert_param_parse(true);
    return ret;
}

static uint32_t do_mstp_help(int argc, char **argv) {
    pr_output("Usage: %s mstilist show BRDEV\n", _context);
    pr_output("       %s fid2mstid show FID BRDEV\n", _context);
    pr_output("       %s portstate show MSTID DEV\n", _context);
    (void)argc;
    (void)argv;
    exit(0);
}

static const struct cmd mstp_cmds[] = {
    { "mstilist",  do_mstp_list },
    { "fid2mstid", do_mstp_fid2mstid },
    { "portstate", do_mstp_portstate },
    { "help",      do_mstp_help },
    NO_MORE_CMDS
};

uint32_t do_mstp(int argc, char **argv) {
    return do_cmd(argc, argv, mstp_cmds);
}
