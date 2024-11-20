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

#include "tsn_bridge.h"
#include "tsntool.h"
#include "tsn_def.h"

static uint32_t do_brport_show(int argc, char **argv) {
    struct port_hdl *phdl;
    uint32_t tcs;
    uint32_t dflt_prio, ret = 0u;
    struct brport_attr_and_caps caps = {0};

    assert_param_min_max(argc, 1, 2);
    phdl = get_if(argv[0]);

    if ((ret = tsn_bridge_get_num_traffic_classes(phdl, &tcs)) != 0u)
        pr_error("Cannot get number of traffic classes.\n");
    if ((!ret) && ((ret = tsn_bridge_get_default_prio(phdl, &dflt_prio)) !=0u))
        pr_error("Cannot get default priority.\n");

    if (!ret)
        pr_output(
                "Port '%s':\n"
                "   Number of traffic classes: %u\n"
                "   Default priority:          %u\n",
                argv[0], tcs, dflt_prio);

   if ((!ret) && (argc == 2) && (!strcmp("details", argv[1]))) {
        if ((ret = tsn_bridge_port_attr(phdl, &caps)) != 0u)
         pr_error("Cannot get current port capabilities.\n");
        else
         pr_output(
                 "\nPort details\n"
                 "\nPort Attributes\n"
                 "   portExternal:				%d\n"
                 "   portType:                                	%d\n"
                 "   portAcceptableFrameTypes:			%d\n"
                 "   portEnableIngressFiltering:			%s\n"
                 "   portTaggingCapable:				%s\n"
                 "   portConfigurableAcceptableFrameTypes:	%s\n"
                 "   portIngressFilteringCapable:			%s\n"
                 "\nPort Capabilities\n"
                 "   portTypeCapCustomerVlan:			%s\n"
                 "   portTypeCapProviderNetwork:			%s\n"
                 "   portTypeCapCustomerNetwork:			%s\n"
                 "   portTypeCapCustomerEdge:			%s\n"
                 "   portTypeCapCustomerBackbone:			%s\n"
                 "   portTypeCapVirtualInstance:			%s\n"
                 "   portTypeCapDBridge:				%s\n"
                 "   portTypeCapRemoteCustomerAccess:		%s\n"
                 "   portTypeCapStationFacing:			%s\n"
                 "   portTypeCapUplinkAccess:			%s\n"
                 "   portTypeCapUplinkRelay:			%s\n",
             caps.portExternal, caps.portType, caps.portAcceptableFrameTypes, (caps.portEnableIngressFiltering == true) ? "true":"false",
             (caps.portTaggingCapable == true) ? "true":"false", (caps.portConfigurableAcceptableFrameTypes == true) ? "true":"false",
             (caps.portIngressFilteringCapable  == true) ? "true":"false", (caps.portTypeCapCustomerVlan == true) ? "true":"false",
             (caps.portTypeCapProviderNetwork == true) ? "true":"false", (caps.portTypeCapCustomerNetwork == true) ? "true":"false",
             (caps.portTypeCapCustomerEdge == true) ? "true":"false", (caps.portTypeCapCustomerBackbone == true) ? "true":"false",
             (caps.portTypeCapVirtualInstance == true) ? "true":"false", (caps.portTypeCapDBridge == true) ? "true":"false",
             (caps.portTypeCapRemoteCustomerAccess == true) ? "true":"false", (caps.portTypeCapStationFacing == true) ? "true":"false",
             (caps.portTypeCapUplinkAccess == true) ? "true":"false", (caps.portTypeCapUplinkRelay == true) ? "true":"false");
   }
   else if ((!ret) && (argc ==2))  pr_error("\nWrong argument: %s\n", argv[1]);

    release_if(phdl);
    return ret;
}

static uint32_t do_brport_prio(int argc, char **argv) {
    struct port_hdl *phdl;
    uint32_t ret = 0u;
    int long long prio;

    assert_param_cnt(argc, 3);
    assert_param_parse(strcmp("set", argv[0]));
    assert_param_parse(sscanf(argv[1], "%lld", &prio) != 1);
    if (prio != (uint32_t) prio) {
        pr_error("PRIO %lld out of range \n", prio);
        return ERANGE;
    }

    phdl = get_if(argv[2]);
    if ((ret = tsn_bridge_set_default_prio(phdl, (uint32_t) prio)) != 0u)
         pr_error("Cannot set default priority.\n");

    release_if(phdl);
    return ret;
}

static uint32_t do_brport_show_pr_reg(int argc, char **argv) {
    struct port_hdl *phdl;
    uint32_t ret = 0u;

    assert_param_cnt(argc, 1);

    phdl = get_if(argv[0]);

    pr_output("PCP        Priority\n");

    for (unsigned int pcp = 0; pcp < MAX_NR_QUEUES; pcp++) {
        uint8_t prio = 0;

        ret += tsn_fqtss_get_prio_regen_table(phdl, pcp, &prio);

        if (ret == 0u)
            pr_output(" %u            %u \n", pcp, prio);

    }
    release_if(phdl);
    return ret;
}

static uint32_t do_brport_show_tc_table(int argc, char **argv) {
    struct port_hdl *phdl;
    uint32_t ret = 0u;

    assert_param_cnt(argc, 1);

    phdl = get_if(argv[0]);

    pr_output("Priority        Traffic Class\n");

    for (unsigned int prio = 0; prio < MAX_NR_QUEUES; prio++) {
        uint8_t tc = 0;

        ret += tsn_fqtss_get_tc_table(phdl, prio, &tc);

        if (ret == 0)
            pr_output(" %u               %u \n", prio, tc);

    }
    release_if(phdl);
    return ret;
}

static uint32_t do_brport_set_pr_reg(int argc, char **argv) {
    struct port_hdl *phdl;
    int long long pcp, prio;
    uint32_t ret = 0u;

    assert_param_cnt(argc, 3);

    assert_param_parse(sscanf(argv[0], "%lld", &pcp) !=1);
    if (pcp != (uint8_t) pcp) {
        pr_error("PCP %lld out of range\n", pcp);
        return ERANGE;
    }
    assert_param_parse(sscanf(argv[1], "%lld", &prio) != 1);
    if ((prio != (uint8_t) prio) || (prio > MAX_NR_QUEUES - 1)) {
        pr_error("PRIORITY %lld out of range [0..%d]\n", prio, MAX_NR_QUEUES - 1);
        return ERANGE;
    }

    phdl = get_if(argv[2]);
    if ((ret = tsn_fqtss_set_prio_regen_table(phdl, (uint8_t) pcp, (uint8_t) prio)) != 0u)
        pr_error("Cannot set priority regeneration table for PCP value %lld.\n", pcp);
    release_if(phdl);
    return ret;
}

static uint32_t do_brport_set_tc_tbl(int argc, char **argv) {
    struct port_hdl *phdl;
    int long long prio, tc;
    uint32_t tc_def;
    uint32_t ret = 0u;

    assert_param_cnt(argc, 3);

    assert_param_parse(sscanf(argv[0], "%lld", &prio) != 1);
    if (prio != (uint8_t) prio) {
        pr_error("PRIO %lld out of range\n", prio);
        return ERANGE;
    }
    assert_param_parse(sscanf(argv[1], "%lld", &tc) != 1);
    if (tc != (uint8_t) tc) {
        pr_error("Traffic class %lld out of range\n", tc);
        return ERANGE;
    }

    phdl = get_if(argv[2]);
    if ((ret = tsn_bridge_get_num_traffic_classes(phdl, &tc_def)) != 0u) {
        pr_error("Cannot get number of traffic classes.\n");
        release_if(phdl);
        return ERANGE;
    } else {
        if (tc >= tc_def) {
            pr_error("Traffic class out of range [0..%d]\n", tc_def - 1);
            release_if(phdl);
            return ERANGE;
        }
    }

    if ((ret = tsn_fqtss_set_tc_table(phdl, (uint8_t) prio, (uint8_t) tc)) != 0u)
        pr_error("Cannot set priority mapping for priority %lld.\n", prio);
    release_if(phdl);
    return ret;
}

static void _write_port_delay_tab_head(void) {

    pr_output("\n      \t     \tDep rx\t\tIndep rx\tIndep rly\tindep tx\n");
    pr_output("Port\tspeed\tmin/ max(ps)\tmin/ max(ns)\tmin/ max(ns)\tmin/ max(ns)\n");
}

static uint32_t _write_port_delay_item(struct port_hdl *port,
                       uint32_t speed) {
    struct pt_delay_entry delay_entry;
    tsn_return_value ret;
    char *pt_name;

    char str_dep_rx_delay_min_ps[21]; //picoseconds
    char str_dep_rx_delay_max_ps[21]; //picoseconds
    char str_indep_rx_delay_min_ns[21];
    char str_indep_rx_delay_max_ns[21];
    char str_indep_rly_delay_min_ns[21];
    char str_indep_rly_delay_max_ns[21];
    char str_indep_tx_delay_min_ns[21];
    char str_indep_tx_delay_max_ns[21];

    /* read port delays from sysfs */
    ret = tsn_bridge_get_port_delays(port, speed, &delay_entry);

    if (ret != 0u) {
        pr_error("Cannot get delays"
             " - %d (%s)\n", errno, strerror(errno));
    } else {
        pt_name = tsn_get_name_by_hdl(port);

        snprintf(str_dep_rx_delay_min_ps, sizeof(str_dep_rx_delay_min_ps),
             "%"PRIu64, delay_entry.dep_rx_delay_min_ps);
        snprintf(str_dep_rx_delay_max_ps,sizeof(str_dep_rx_delay_max_ps),
             "%"PRIu64, delay_entry.dep_rx_delay_max_ps);
        snprintf(str_indep_rx_delay_min_ns, sizeof(str_indep_rx_delay_min_ns),
             "%"PRIu32, delay_entry.indep_rx_delay_min_ns);
        snprintf(str_indep_rx_delay_max_ns, sizeof(str_indep_rx_delay_max_ns),
             "%"PRIu32, delay_entry.indep_rx_delay_max_ns);
        snprintf(str_indep_rly_delay_min_ns, sizeof(str_indep_rly_delay_min_ns),
             "%"PRIu32, delay_entry.indep_rly_delay_min_ns);
        snprintf(str_indep_rly_delay_max_ns, sizeof(str_indep_rly_delay_max_ns),
             "%"PRIu32, delay_entry.indep_rly_delay_max_ns);
        snprintf(str_indep_tx_delay_min_ns, sizeof(str_indep_tx_delay_min_ns),
             "%"PRIu32, delay_entry.indep_tx_delay_min_ns);
        snprintf(str_indep_tx_delay_max_ns, sizeof(str_indep_tx_delay_max_ns),
             "%"PRIu32, delay_entry.indep_tx_delay_max_ns);

        pr_output("%s\t%u\t%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\n",
            pt_name, delay_entry.speed,
            (delay_entry.dep_rx_delay_min_ps == 0) ? "--" :
                str_dep_rx_delay_min_ps,
            (delay_entry.dep_rx_delay_max_ps == 0) ? "--" :
                str_dep_rx_delay_max_ps,
            (delay_entry.indep_rx_delay_min_ns == 0) ? "--" :
                str_indep_rx_delay_min_ns,
            (delay_entry.indep_rx_delay_max_ns == 0) ? "--" :
                str_indep_rx_delay_max_ns,
            (delay_entry.indep_rly_delay_min_ns == 0) ? "--" :
                str_indep_rly_delay_min_ns,
            (delay_entry.indep_rly_delay_max_ns == 0) ? "--" :
                str_indep_rly_delay_max_ns,
            (delay_entry.indep_tx_delay_min_ns == 0) ? "--" :
                str_indep_tx_delay_min_ns,
            (delay_entry.indep_tx_delay_max_ns == 0) ? "--" :
                str_indep_tx_delay_max_ns);
    }
    return ret;
}

static uint32_t _write_delay_list_speed(struct port_hdl *port) {
    uint32_t ret = 0u;

    ret = _write_port_delay_item(port, 10);
    if (ret != 0)
        return ret;

    ret = _write_port_delay_item(port, 100);
    if (ret != 0)
        return ret;

    ret = _write_port_delay_item(port, 1000);
    return ret;
}

static uint32_t _do_delay_one_port(int argc, char **argv, struct port_hdl *port){
    uint32_t ret;
    _write_port_delay_tab_head();
    if(argc > 1){
        ret = _write_port_delay_item(port, atoi(argv[1]));
    } else {
        ret = _write_delay_list_speed(port);
    }
    return ret;
}

static uint32_t do_brport_delay(int argc, char **argv) {
    struct port_hdl *bridge, *port;
    uint32_t ret = 0u, i;
    uint32_t num_ports;
    uint32_t *pt_list = NULL, *pt_help;

    if (argc > 0) {
        if(!(strncmp(argv[0], "br", 2))) {
            bridge = get_if(argv[0]);
            pt_list = tsn_get_list_port_ids(bridge, &num_ports);
            release_if(bridge);
            if (!pt_list) {
                /* no ports got */
                pr_error("Cannot get list of port id's"
                     " - %d (%s)\n", errno, strerror(errno));
                return E_TSN_LIB_NOSUCHFILE;
            }
            pt_help = pt_list;
            for (i = 0; i < num_ports; i++) {
                port = get_if(tsn_get_name_by_id(bridge, *pt_help));
                ret = _do_delay_one_port(argc, argv, port);
                release_if(port);
                if (ret != 0u) break;
                pt_help ++;
            }
            free(pt_list);

        } else {
            port = get_if(argv[0]);
            ret = _do_delay_one_port(argc, argv, port);
            release_if(port);
        }
    }
    else {
        /* missing parameter*/
        assert_param_parse(true);
    }

    return ret;
}

static uint32_t do_brport_help(int argc, char **argv) {
    pr_output("Usage: %s show DEV [details]\n", _context);
    pr_output("       %s prio set PRIO DEV\n", _context);
    pr_output("       %s rdpriotbl DEV\n", _context);
    pr_output("       %s rdtctbl DEV\n", _context);
    pr_output("       %s wrpriotbl PCP PRIO DEV\n", _context);
    pr_output("       %s wrtctbl PRIO TC DEV\n", _context);
    pr_output("       %s getdelays DEV [SPEED]\n", _context);
    (void)argc;
    (void)argv;
    exit(0);
}

static const struct cmd brport_cmds[] = {
    { "show",  do_brport_show },
    { "prio",  do_brport_prio },
    { "rdpriotbl", do_brport_show_pr_reg},
    { "rdtctbl", do_brport_show_tc_table},
    { "wrpriotbl", do_brport_set_pr_reg},
    { "wrtctbl", do_brport_set_tc_tbl},
    { "getdelays", do_brport_delay},
    { "help",  do_brport_help },
    NO_MORE_CMDS
};

uint32_t do_brport(int argc, char **argv) {
    return do_cmd(argc, argv, brport_cmds);
}
