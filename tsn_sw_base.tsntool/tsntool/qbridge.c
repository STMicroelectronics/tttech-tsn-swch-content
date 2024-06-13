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
#include <dirent.h>

#include "tsn_qbridge.h"
#include "tsn_bridge.h"
#include "tsn_mstp.h"
#include "tsntool.h"

static uint32_t do_qbridge_show(int argc, char **argv) {
    struct port_hdl *phdl;
    uint32_t max_vlan_id, max_supported_vlans, num_vlans, ret = 0u;
    struct bridge_attr_and_caps caps = {0};

    assert_param_min_max(argc, 1, 2);
    phdl = get_if(argv[0]);

    if ((ret = tsn_qbridge_get_max_vlan_id(phdl, &max_vlan_id)) != 0u)
        pr_ecode("ERROR: Cannot get maximum VLAN Id - ", ret);

    if ((!ret) && ((ret = tsn_qbridge_get_max_supported_vlans(phdl, &max_supported_vlans)) != 0u))
        pr_ecode("ERROR: Cannot get maximum number of supported VLANs - ", ret);

    if ((!ret) && ((ret = tsn_qbridge_get_num_vlans(phdl, &num_vlans)) != 0u))
        pr_ecode("ERROR: Cannot get current number of VLANs - ", ret);

    if (!ret)
        pr_output(
                "Bridge '%s':\n"
                "   Maximum VLAN Id: %u\n"
                "   Used VLANs:      %u (out of %u)\n",
                argv[0], max_vlan_id, num_vlans, max_supported_vlans);

    if ((!ret) && (argc == 2) && (!strcmp("details", argv[1]))) {
        if ((ret = tsn_bridge_attr(phdl, &caps)) != 0u)
            pr_ecode("ERROR: Cannot get bridge current capabilities - ", ret);
        else
            pr_output(
                    "\nBridge details\n"
                    "   BridgeType:				%d\n"
                    "   BridgeNumPorts:        		%d\n"
                    "   BridgeComponents:      		%s\n"
                    "   BridgeTrafficClassesEnabled:		%s\n"
                    "\nComponent Attributes\n"
                    "   ComponentId:				%u\n"
                    "   ComponentType:			%d\n"
                    "   ComponentNumPorts:			%d\n"
                    "\nFDB Attributes\n"
                    "   FdbSize:				%d\n"
                    "   FdbStaticEntries:			%d\n"
                    "\nCapability Attributes\n"
                    "   ExtendedFilteringServices:		%s\n"
                    "   TrafficClasses:			%s\n"
                    "   StaticEntryIndividualPort:		%s\n"
                    "   IVLCapable:				%s\n"
                    "   SVLCapable:				%s\n"
                    "   HybridCapable:			%s\n"
                    "   ConfigurablePvidTagging:		%s\n"
                    "   LocalVlanCapable:			%s\n",
                caps.BridgeType, caps.BridgeNumPorts, (caps.BridgeComponents == true) ? "true":"false", (caps.BridgeTrafficClassesEnabled == true) ? "true":"false", caps.ComponentId,
                caps.ComponentType, caps.ComponentNumPorts, caps.FdbSize, caps.FdbStaticEntries, (caps.ExtendedFilteringServices == true) ? "true":"false",
                (caps.TrafficClasses == true) ? "true":"false", (caps.StaticEntryIndividualPort == true) ? "true":"false", (caps.IVLCapable == true) ? "true":"false",
                (caps.SVLCapable == true) ? "true":"false", (caps.HybridCapable == true) ? "true":"false", (caps.ConfigurablePvidTagging == true) ? "true":"false",
                (caps.LocalVlanCapable == true) ? "true":"false");
    }
    else if ((!ret) && (argc ==2)) pr_error("\nWrong argument: %s\n", argv[1]);

    release_if(phdl);
    return ret;
}

static uint32_t _vid_show(struct port_hdl *br, uint32_t vid) {
    uint32_t max_vid, fid, ret = 0u;

    /* min value for vid is 1. 1 is default PVID, i.e. untagged frames are assigned to VLAN 1 -
     * get vid 1. vid 0 doesn't exist. Value 0 in VLAN tag means no VLAN, then only PCP field
     * is used. (See 802.1Q-2018, Table 9-2)
     */
    if (vid == 0) {
        pr_error("VID %u is out of range \n", vid);
        return ERANGE;
    }

    if ((ret = tsn_qbridge_get_max_vlan_id(br, &max_vid)) != 0u)
        pr_ecode("ERROR: Cannot get maximum ID of VIDs - ", ret);
    else {
        if (vid > max_vid) {
            pr_error("VID %u is out of range (maximum VID is %u) for "
                     "bridge '%s'\n", vid, max_vid, tsn_get_name_by_hdl(br));
            ret = ERANGE;
        }
        else {
            if ((ret = tsn_qbridge_get_vid2fid(br, vid, &fid)) != 0u) {
                pr_ecode("ERROR: Cannot get VID information - ", ret);
                pr_error("  Please check if VID %u is configured "
                         "on bridge '%s'\n", vid, tsn_get_name_by_hdl(br));
            }
            else pr_output("Allocated FID: %u \n", fid);
        }
    }
    return ret;
}

static uint32_t do_qbridge_vid(int argc, char **argv) {
    struct port_hdl *phdl;
    int long long vid, ret = 0u;

    phdl = NULL;

    if (argc > 0) {
        if (!strcmp("show", argv[0])) {
            assert_param_cnt(argc, 3);
            assert_param_parse(sscanf(argv[1], "%lld", &vid) != 1);
            if (vid != (uint32_t) vid) {
                pr_error("VID %lld is out of range \n", vid);
                ret = EINVAL;
            } else {
                phdl = get_if(argv[2]);
                ret = _vid_show(phdl, (uint32_t) vid);
                release_if(phdl);
            }
        }
        else
            assert_param_parse(true);
    }
    else
        assert_param_parse(true);
    return ret;
}

static uint32_t _fid_show(struct port_hdl *br, uint32_t min_fid, uint32_t max_fid) {
    struct qbr_fid2vid_entry *fid2vid_list;
    uint32_t max_vids, ret = 0u;

    if ((ret = tsn_qbridge_get_max_supported_vlans(br, &max_vids)) != 0u)
        pr_ecode("ERROR: Cannot get maximum number of supported VLANs - ", ret);
    else if (min_fid >= max_vids) {
        pr_error("FID %u is out of range (maximum FID is %u) for "
                 "bridge '%s'\n", min_fid, max_vids - 1, tsn_get_name_by_hdl(br));
        ret = ERANGE;
    }
    else if (!(fid2vid_list = malloc(sizeof(*fid2vid_list) * max_vids))) {
        pr_error("Cannot allocate memory for fid2vid list\n");
        ret = ENOMEM;
    }
    else {
        uint32_t alloc_vids;
        unsigned int i;
        max_fid = (max_fid >= max_vids) ? max_vids - 1: max_fid; // fix upper fid bound

        for (i = min_fid, alloc_vids = 0; i <= max_fid; i++) {
            if ((ret = tsn_qbridge_get_fid2vid(br, i, max_vids, fid2vid_list)) != 0u) {
                pr_ecode("ERROR: Cannot retrieve fid2vid list - ", ret);
                pr_error("Check if FID %u is configured.\n", i);
            }
            else {
                uint32_t j;
                if (fid2vid_list[0].vid == 0) {
                    if ((i == max_fid) && (alloc_vids == 0)) pr_output("No Allocated VIDs\n");
                    continue;
                }
                if (alloc_vids == 0) pr_output("FID\t \t allocated VIDs\n");
                pr_output("%u\t \t", i);
                for (j = 0; j < max_vids; j++) {
                    if (fid2vid_list[j].vid == 0)
                        break; // no more valid VIDs in fid2vid-list
                    else if (j>0) pr_output(",");
                    pr_output(" %u", fid2vid_list[j].vid);
                    ++alloc_vids;
                }
                pr_output("\n");
            }
        }
        free(fid2vid_list);
    }
    return ret;
}

static uint32_t do_qbridge_fid(int argc, char **argv) {
    struct port_hdl *phdl;
    int long long fid, ret = 0u;

    if (argc > 0) {
        if (!strcmp("show", argv[0])) {
            assert_param_cnt(argc, 3);
            assert_param_parse(sscanf(argv[1], "%lld", &fid) != 1);
            if (fid != (uint32_t) fid) {
                pr_error("FID %lld is out of range.\n", fid);
                ret = EINVAL;
            } else {
                phdl = get_if(argv[2]);
                ret = _fid_show(phdl, (uint32_t) fid, (uint32_t) fid);
                release_if(phdl);
            }
        }
        else if (!strcmp("showall", argv[0])) {
            assert_param_cnt(argc, 2);
            phdl = get_if(argv[1]);
            ret = _fid_show(phdl, 0, 0xFFF);
            release_if(phdl);
        }
        else
            assert_param_parse(true);
    }
    else
        assert_param_parse(true);
    return ret;
}

static uint32_t _delay_verify_pt_name(struct port_hdl *phdl_bridge, char *pt_name, uint32_t *pt_id) {
    uint32_t ret = 0u, num_ports;
    struct bridge_attr_and_caps caps = {0};

    /* read number of ports bridge has */
    if ((ret = tsn_bridge_attr(phdl_bridge, &caps)) != 0u) {
        pr_ecode("ERROR: Cannot get bridge current capabilities - ", ret);
        return ret;
    }
    else {
        num_ports = caps.BridgeNumPorts;
    }
    /* get port id for ingress and egress port */
    ret = tsn_get_id_by_name(phdl_bridge, pt_name);
    if (ret <= num_ports) {
        *pt_id = ret;
    } else {
        pr_error("Port name (%s) not valid \n", pt_name);
        return ret;
    }
    return 0;
}

static void _write_delay_tab_head(void) {

    pr_output("\n\t\t\t\t\tIndep delay \tIndep delay\tDep delay\tDep delay\n");
    pr_output("Port-rx \tPort-tx \ttc \tmin (ns) \tmax (ns) \tmin (ps) \tmax (ps) \n");
}

static uint32_t _write_delay_item(struct port_hdl *phdl_bridge,
                                  uint32_t ingress_pt_id,
                                  uint32_t egress_pt_id,
                                  uint32_t tc) {
    struct br_delay_entry delay_entry;
    tsn_return_value ret;
    char *pt_ingress_name, *pt_egress_name;
    char str_indep_max_delay[21], str_indep_min_delay[21], str_dep_min_delay[21],
            str_dep_max_delay[21];

    /* read delay from sysfs */
    ret = tsn_bridge_get_delays(phdl_bridge, ingress_pt_id,
            egress_pt_id, tc, &delay_entry);
    if (ret != 0u) {
        pr_ecode("ERROR: Cannot get delays - ", ret);
    } else {
        pt_ingress_name = tsn_get_name_by_id (phdl_bridge, delay_entry.port_ingress);
        pt_egress_name = tsn_get_name_by_id (phdl_bridge, delay_entry.port_egress);
        snprintf(str_indep_max_delay, sizeof(str_indep_max_delay), "%"PRIu64,
                delay_entry.indep_max_delay_ns);
        snprintf(str_indep_min_delay, sizeof(str_indep_min_delay), "%"PRIu64,
                delay_entry.indep_min_delay_ns);
        snprintf(str_dep_min_delay, sizeof(str_dep_min_delay), "%"PRIu32,
                delay_entry.dep_min_delay_ps);
        snprintf(str_dep_max_delay, sizeof(str_dep_max_delay), "%"PRIu32,
                delay_entry.dep_max_delay_ps);
        pr_output("%s\t\t%s\t\t%u\t%s\t\t%s\t\t%s\t\t%s\n",
                pt_ingress_name, pt_egress_name, delay_entry.traffic_class,
                (delay_entry.indep_min_delay_ns == 0) ? "--" : str_indep_min_delay,
                (delay_entry.indep_max_delay_ns == 0) ? "--" : str_indep_max_delay,
                (delay_entry.dep_min_delay_ps == 0) ? "--" : str_dep_min_delay,
                (delay_entry.dep_max_delay_ps == 0) ? "--" : str_dep_max_delay);
        free(pt_ingress_name);
        free(pt_egress_name);
    }
    return ret;
}

static uint32_t _write_delay_list_tc(struct port_hdl *phdl_bridge,
                                     char *ingress_pt_name,
                                     uint32_t ingress_pt_id,
                                     uint32_t egress_pt_id) {
    struct port_hdl *phdl_port;
    uint32_t num_tc, ret = 0u, i;

    /* read number of traffic classes */
    phdl_port = get_if(ingress_pt_name);
    ret = tsn_bridge_get_num_traffic_classes(phdl_port, &num_tc);
    release_if(phdl_port);
    if ((ret != 0)) {
        pr_ecode("ERROR: Cannot get number of traffic classes - ", ret);
        pr_error("Check port %s  \n", ingress_pt_name);
        return ret;
    }
    /* write delay list items */
    for (i = 0; i < num_tc; i++) {
        ret = _write_delay_item(phdl_bridge, ingress_pt_id, egress_pt_id, i);
        if (ret != 0)
            break;
    }
    return ret;
}

static uint32_t _delay_verify_tc(char *ingress_pt_name,
                                 char *tc_str, uint32_t *tc) {
    uint32_t ret = 0u, num_tc;
    struct port_hdl *phdl_port;

    /* check if parameter for traffic class is within range */

    phdl_port = get_if(ingress_pt_name);
    ret = tsn_bridge_get_num_traffic_classes(phdl_port, &num_tc);
    release_if(phdl_port);

    *tc = atoi(tc_str);
    if ((ret != 0) || !(*tc < num_tc)) {        // traffic classes are counted starting at 0
        pr_error("traffic class (%s) not valid \n", tc_str);
        return 1;       // return value unequal 0 also if ret == 0
    }
    return ret;
}

static uint32_t _delay_all_tc(struct port_hdl *phdl_bridge,
                              char *ingress_pt_name,
                              char *egress_pt_name) {
    uint32_t ret = 0u;
    uint32_t ingress_pt_id, egress_pt_id;

    /* check ingress and egress ports */
    ret = _delay_verify_pt_name(phdl_bridge, ingress_pt_name, &ingress_pt_id);
    if (ret != 0)
        return ret;
    ret = _delay_verify_pt_name(phdl_bridge, egress_pt_name, &egress_pt_id);
    if (ret != 0)
        return ret;
    /* print table */
    _write_delay_tab_head();
    ret = _write_delay_list_tc(phdl_bridge, ingress_pt_name, ingress_pt_id, egress_pt_id);
    return ret;
}

static uint32_t _delay_all_brpts(struct port_hdl *phdl_bridge) {
    uint32_t ret = 0u;
    uint32_t num_ports;
    uint32_t *pt_list = NULL, *pt_ingress, *pt_egress;
    unsigned int i, j;
    char *pt_name;

    /* read list of port id's */
    pt_list = tsn_get_list_port_ids(phdl_bridge, &num_ports);
    if (!pt_list) {
        /* no ports got */
        pr_error("Cannot get list of port id's"
                 " - %d (%s)\n", errno, strerror(errno));
        return E_TSN_LIB_NOSUCHFILE;
    }

    /* iterate through list of port ids with i and j write delays for all pairs of ports */
    pt_ingress = pt_list;
    for (i = 0; i < num_ports; i++) {
        /* get port name for id in pt_ingress - needed to read traffic classes of port */
        pt_name = tsn_get_name_by_id (phdl_bridge, *pt_ingress);
        if (pt_name) {
            /* print table head */
            _write_delay_tab_head();

            pt_egress = pt_list;
            for (j = 0; j < num_ports; j++) {
                if (*pt_ingress != *pt_egress) {
                    ret = _write_delay_list_tc(phdl_bridge, pt_name, *pt_ingress, *pt_egress);
                    pr_output( "\n");
                }
                if (ret != 0u) break;
                pt_egress++;
            }
            free(pt_name);
        } else {
            /* print an error but continue loop */
            pr_error("Cannot get port name to port id %u", *pt_ingress);
        }
        pt_ingress++;
    }

    free(pt_list);
    return ret;
}

static uint32_t _delay_all_egress_pt(struct port_hdl *phdl_bridge,
                                     char *ingress_pt_name) {
    uint32_t ret = 0u;
    uint32_t ingress_pt_id, num_ports;
    uint32_t *pt_list = NULL, *pt_help;
    unsigned int i;

    /* check ingress port */
    ret = _delay_verify_pt_name(phdl_bridge, ingress_pt_name, &ingress_pt_id);
    if (ret != 0)
        return ret;
    /* read list of port id's */
    pt_list = tsn_get_list_port_ids(phdl_bridge, &num_ports);
    if (!pt_list) {
        /* no ports got */
        pr_error("Cannot get list of port id's"
                 " - %d (%s)\n", errno, strerror(errno));
        return E_TSN_LIB_NOSUCHFILE;
    }

    /* print table head */
    _write_delay_tab_head();

    /* iterate through list of port ids and write delays using port id's as rx ports */
    pt_help = pt_list;
    for (i = 0; i < num_ports; i++) {
        if (ingress_pt_id != *pt_help) {
            ret = _write_delay_list_tc(phdl_bridge, ingress_pt_name, ingress_pt_id, *pt_help);
            pr_output( "\n");
        }
        if (ret != 0u) break;
        pt_help++;
    }

    free(pt_list);
    return ret;
}

static uint32_t _delay_single_item(struct port_hdl *phdl_bridge,
                                   char *ingress_pt_name,
                                   char *egress_pt_name,
                                   char *tc_str) {
    uint32_t ret = 0u;
    uint32_t ingress_pt_id, egress_pt_id, tc;

    /* check ingress and egress ports */
    ret = _delay_verify_pt_name(phdl_bridge, ingress_pt_name, &ingress_pt_id);
    if (ret != 0)
        return ret;
    ret = _delay_verify_pt_name(phdl_bridge, egress_pt_name, &egress_pt_id);
    if (ret != 0)
        return ret;
    /* check traffic class */
    ret = _delay_verify_tc(ingress_pt_name, tc_str, &tc);
    if (ret != 0)
        return ret;
    /* print table header */
    _write_delay_tab_head();
    /* write delay  items */
    ret = _write_delay_item(phdl_bridge, ingress_pt_id, egress_pt_id, tc);
    return ret;
}

static uint32_t do_qbridge_delay(int argc, char **argv) {
    struct port_hdl *phdl_bridge;
    uint32_t ret = 0u;

    if (argc > 0) {
        if (!strcmp("show", argv[0])) {
            if (argc > 1) {
                phdl_bridge = get_if(argv[1]);

                switch (argc) {
                case 2:
                    /* user specified only bridge */
                    ret = _delay_all_brpts(phdl_bridge);
                    break;
                case 3:
                    /* user specified bridge and ingress port */
                    ret = _delay_all_egress_pt(phdl_bridge, argv[2]);
                    break;
                case 4:
                    /* user specified bridge, ingress port and egress port */
                    ret = _delay_all_tc(phdl_bridge, argv[2], argv[3]);
                    break;
                case 5:
                    /* user specified bridge, ingress port, egress port and traffic class */
                    ret = _delay_single_item(phdl_bridge, argv[2], argv[3], argv[4]);
                    break;
                default:
                    /* too many parameters */
                    assert_param_parse(true);
                }
                release_if(phdl_bridge);
            }
            else
                assert_param_min_max(argc, 2, 5);
        }
        else
            /* wrong keyword*/
            assert_param_parse(true);
    }
    else
        /* missing parameter*/
        assert_param_parse(true);

    return ret;
}

static uint32_t do_qbridge_help(int argc, char **argv) {
    pr_output("Usage: %s show BRDEV [details]\n", _context);
    pr_output("       %s vid show VID BRDEV\n", _context);
    pr_output("       %s fid show FID BRDEV\n", _context);
    pr_output("       %s fid showall BRDEV\n", _context);
    pr_output("       %s delay show BRDEV [DEV][DEV][TC]\n", _context);
    (void)argc;
    (void)argv;
    exit(0);
}

static const struct cmd qbridge_cmds[] = {
    { "show",  do_qbridge_show },
    { "vid",   do_qbridge_vid },
    { "fid",   do_qbridge_fid },
    { "delay", do_qbridge_delay },
    { "help",  do_qbridge_help },
    NO_MORE_CMDS
};

uint32_t do_qbridge(int argc, char **argv) {
    return do_cmd(argc, argv, qbridge_cmds);
}
