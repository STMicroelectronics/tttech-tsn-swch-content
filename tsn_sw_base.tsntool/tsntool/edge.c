/*
 * TTTech TSN Command Line Tool (tsntool)
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

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include "tsn_edgex.h"
#include "tsn_bridge.h"
#include "tsntool.h"

static uint32_t do_edge_show(int argc, char **argv) {
    struct port_hdl *phdl;
    bool     ct;
    int      mirror_pt_id = -1;
    uint32_t ret = 0u;

    assert_param_cnt(argc, 1);
    phdl = get_if(argv[0]);

    if ((ret = tsn_edgex_get_cut_through(phdl, &ct)) != 0u)
       pr_error("Cannot get cut-through settings - %d (%s)\n",
                errno, strerror(errno));
    if ((!ret) && ((ret = tsn_edgex_get_mirror_portid(phdl, &mirror_pt_id)) != 0u))
       pr_error("Cannot get port-mirror  settings - %d (%s)\n",
                errno, strerror(errno));
    if (!ret)
    pr_output(
            "Port '%s':\n"
            "   Management Traffic Class: : 1\n"
            "   Cut-through                 %s\n"
            "   Port-Mirroring              %s\n",
            argv[0],
            (ct) ? "ON" : "OFF",
            (mirror_pt_id >= 0) ? "ON" : "OFF"
            );

    release_if(phdl);
    return ret;

}

static uint32_t do_edge_ct(int argc, char **argv) {
    struct port_hdl *phdl;
    bool on = true;
    uint32_t ret = 0u;

    assert_param_cnt(argc, 2);
    if (!strcmp("on", argv[0]))
        on = true;
    else if (!strcmp("off", argv[0]))
        on = false;
    else
        assert_param_parse(true);

    phdl = get_if(argv[1]);
    if ((ret = tsn_edgex_set_cut_through(phdl, on)) != 0u)
        pr_error("Cannot set ct - %d (%s)\n", errno, strerror(errno));
    release_if(phdl);

    return ret;
}

static uint32_t do_edgx_mirror(int argc, char **argv) {
    struct port_hdl *phdl;
    struct port_hdl *mirror;
    unsigned int mirror_pt_id;
    uint32_t ret = 0u;

    assert_param_cnt(argc, 2);
    if (!strcmp("off", argv[0]))
        mirror_pt_id = -1;
    else {
        mirror = get_if(argv[0]);
        if ((ret != tsn_bridge_port_id(mirror, &mirror_pt_id)) != 0u) {
            pr_error("Cannot get ID for mirror port - %d (%s)\n",
                     errno, strerror(errno));
            release_if(mirror);
            return ret;
        }
        release_if(mirror);
    }

    phdl = get_if(argv[1]);
    if ((ret != tsn_edgex_set_mirror_portid (phdl, mirror_pt_id)) != 0u)
        pr_error("Cannot set mirror port - %d (%s)\n",
                 errno, strerror(errno));
    release_if(phdl);

    return ret;
}

static uint32_t do_edge_help(int argc, char **argv) {
    pr_output("Usage: %s show DEV\n", _context);
    pr_output("       %s ct  {on | off}  DEV\n", _context);
    pr_output("       %s mirror {MIRROR_DEV | off}  DEV\n", _context);
    (void)argc;
    (void)argv;
    exit(0);
}

static const struct cmd edge_cmds[] = {
    { "ct",     do_edge_ct },
    { "mirror", do_edgx_mirror },
    { "show",   do_edge_show },
    { "help",   do_edge_help },
    NO_MORE_CMDS
};

uint32_t do_edge (int argc, char **argv) {
    return do_cmd(argc, argv, edge_cmds);
}
