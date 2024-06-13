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

#include "tsn_bridge.h"
#include "tsn_sid.h"
#include "tsntool.h"
#include "tsn_def.h"

#define SID_INVALID_STRHDL (0xFFFF)
#define BIT(nr)            (1UL << (nr))
#define SID_MAX_PORT_CNT   (16u)
#define SID_MAX_ORD        ((uint16_t)(-1))

static const char *sid_ident_str[SID_IDENT_MAX] = {
    [SID_NULL]   = "null_stream",
    [SID_SOURCE] = "source_stream",
    [SID_ACTIVE] = "active_stream",
    [SID_IPSTR]  = "ip_stream"
};

static sid_pos sid_get_position (char *str)
{
    if (!strcmp("out-fac-in", str))
        return OUT_FAC_IN;
    else if (!strcmp("in-fac-in", str))
        return IN_FAC_IN;
    else if (!strcmp("out-fac-out", str))
        return OUT_FAC_OUT;
    else if (!strcmp("in-fac-out", str))
        return IN_FAC_OUT;
    else {
        pr_error("Position parameter %s not set correctly,\n", str);
        return EPERM;
    }
}

static void print_identification(uint16_t ord, struct ident_params *sid)
{
    //check if Identification parameters are set
    pr_output("\n");
    pr_output("ORD = %d\n ", ord);
    if (sid->str_hdl != SID_INVALID_STRHDL)
        pr_output("Stream handle = %d\n ", sid->str_hdl);
    else
        pr_output("   Stream handle not set\n");
    if (sid->id_type != 0) {
        if (sid->id_type == SID_NULL)
            pr_output("   Null Stream Identification\n");
        else if (sid->id_type == SID_SOURCE)
            pr_output("   Source Stream Identification\n");
    } else
        pr_output("   Unknown Stream Identification parameters\n");
}

static void print_position(struct port_hdl *brdev, uint16_t port_mask,
                           sid_pos pos)
{
    unsigned i;
    char *port_name;

    //OUT_FAC_IN->only supported position, future: add print for other positions
    //if applicable

    if (port_mask == 0)
        pr_output("    Unknown Port-Position List\n");
    else if (pos == OUT_FAC_IN) {
        pr_output("    OutFacInputPortList = ");
        for (i = 0; i < SID_MAX_PORT_CNT; i++) {
            if ((port_mask & (1 << i))) {
                port_name = tsn_get_name_by_id(brdev, i);
                if (port_name) {
                    pr_output("%s ", port_name);
                    free(port_name);
                } else {
                    pr_error("Cannot get name of port %u! \n", i);
                    break;
                }
            }
        }
        pr_output("\n");
    }
}

static void print_ident_params(struct ident_params *sid)
{
    if (sid->id_type != 0) {
        pr_output("Identification parameters: \n");
        pr_output("    MAC = %02x:%02x:%02x:%02x:%02x:%02x, ",
                  sid->addr[0], sid->addr[1], sid->addr[2], sid->addr[3],
                  sid->addr[4], sid->addr[5]);

        if (sid->tag == SID_TAGGED)
            pr_output("Tagged");
        else if (sid->tag == SID_PRIORITY)
            pr_output("Priority (Untagged or VID = 0)");
        else if (sid->tag == SID_ALL)
            pr_output("All");

        pr_output(", VLAN ID = %d\n\n",sid->vid);
    } else {
        pr_output("\n");
    }
}

static void sid_print_entry(struct port_hdl *brdev, uint16_t ord,
                            struct ident_params *ident, uint16_t port_mask)
{
    print_identification(ord, ident);
    print_position(brdev, port_mask, OUT_FAC_IN);
    print_ident_params(ident);

}

static uint32_t do_sid_show_max_sup(int argc, char **argv) {
    struct port_hdl *phdl;
    uint32_t max_sup;
    uint32_t ret = 0u;
    assert_param_cnt(argc, 1);

    phdl = get_if(argv[0]);
    ret = tsn_sid_get_max_sup(phdl, &max_sup);
    pr_output("Max supported stream handle = %u\n", max_sup);
    release_if(phdl);

    return ret;
}

static uint32_t do_sid_show_tab_len(int argc, char **argv) {
    struct port_hdl *brdev;
    uint32_t len;
    uint32_t ret = 0u;
    assert_param_cnt(argc, 1);

    brdev = get_if(argv[0]);
    ret = tsn_sid_get_tab_length(brdev, &len);
    if (ret) {
        pr_ecode("ERROR: Cannot get table length! - ", ret);
    } else {
        pr_output("Table length = %u\n", len);
    }
    release_if(brdev);

    return ret;
}

static uint32_t do_sid_show_ports(int argc, char **argv) {
    struct port_hdl *brdev;
    uint16_t port_mask;
    uint32_t ret;
    unsigned i;
    char *port_name;

    assert_param_cnt(argc, 1);
    brdev = get_if(argv[0]);
    ret = tsn_sid_get_supported_ports(brdev, &port_mask);
    if (ret) {
        pr_ecode("ERROR: Cannot get supported ports! - ", ret);
        release_if(brdev);
        return ret;
    }
    pr_output("Supported ports: ");
    for (i = 0; i < SID_MAX_PORT_CNT; i++) {
        if ((port_mask & (1 << i))) {
            port_name = tsn_get_name_by_id(brdev, i);
            if (port_name) {
                pr_output("%s ", port_name);
                free(port_name);
            } else {
                pr_error("\nCannot get name of port %u!", i);
                ret = EINVAL;
                break;
            }
        }
    }
    pr_output("\n");
    release_if(brdev);

    return ret;
}

static uint32_t do_sid_show_types(int argc, char **argv) {
    struct port_hdl *brdev;
    sid_ident_type i;
    uint32_t ret;
    bool supported = false;

    assert_param_cnt(argc, 1);
    brdev = get_if(argv[0]);

    pr_output("Supported identification types: ");
    for (i = SID_UNKNOWN + 1; i < SID_IDENT_MAX; i++) {
        ret = tsn_sid_get_ident_type(brdev, i, &supported);
        if (ret) {
            pr_ecode("ERROR: Cannot get identification type! - ", ret);
            break;
        }
        if (supported)
            pr_output("%s ", sid_ident_str[i]);
    }
    pr_output("\n");
    release_if(brdev);
    return ret;
}

static uint32_t show_ident_params(struct port_hdl *brdev, uint16_t ord) {

    struct ident_params ident;
    uint16_t port_mask;
    uint32_t ret = 0u;

    ret = tsn_sid_get_ident_params(brdev, ord, &ident);
    //Only Out-fac_in supported by the driver
    if (!ret)
        ret = tsn_sid_get_outfacin(brdev, ord, &port_mask);

    if (!ret)
        sid_print_entry(brdev, ord, &ident, port_mask);
    else if (TSN_ERRNO_ERROR_CODE(ret) == ENOENT)
        pr_error("ORD %u does not exist.\n", ord);
    else
        pr_error("Reading SID database - %d (%s)\n", errno, strerror(errno));

    return ret;
}

static uint32_t get_ord_list(struct port_hdl *brdev) {
    uint32_t cnt = 0, i = 0;
    uint32_t ret = 0u;
    uint16_t *ord;

    tsn_sid_get_current_cnt (brdev, &cnt);

    ord = malloc(sizeof(*ord) * cnt);
    if (ord == NULL) {
        pr_error("Cannot allocate memory for ORD list\n");
        return ENOMEM;
    }

    if ((ret = tsn_sid_get_entries_list(brdev, cnt, ord)))
         pr_error("Cannot retrieve SID ORD list\n");

    for (i = 0; i < cnt; i++)
        ret = show_ident_params(brdev, ord[i]);

    free(ord);

    return ret;
}

static uint32_t do_sid_create(int argc, char **argv) {

    uint32_t ret = 0u;
    struct port_hdl *brdev;
    struct ident_params sid;
    int long long ord, vid;
    int retmac, i;
    char *ptr;
    char *start_ptr;
    int long hexval;
    /*ORD {NULL | SRC} IDENT_PARAMS BRDEV
     * IDENT_PARAMS is MAC_ADDR VLAN_TAGGED VID
     * */

    assert_param_cnt(argc, 6);

    assert_param_parse(sscanf(argv[0], "%lld", &ord) != 1);
    if ((unsigned)ord > SID_MAX_ORD) {
        pr_error("ORD %lld out of range\n", ord);
        return EINVAL;
    }

    if (!strcmp("NULL", argv[1]))
        sid.id_type = SID_NULL;
    /*Only null identification is currently supported.*/
    else if (!strcmp("SRC", argv[1]) || !strcmp("DST", argv[1]) ||
             !strcmp("IP", argv[1])) {
        pr_error("Stream identification '%s' currently not supported.\n", argv[1]);
        return EINVAL;
    } else {
        pr_error("Invalid stream identification method '%s'\n", argv[1]);
        return EINVAL;
    }

    retmac = sscanf(argv[2], "%" SCNx8 ":%" SCNx8 ":%" SCNx8 ":%" SCNx8 ":%" SCNx8 ":%" SCNx8,
             &sid.addr[0], &sid.addr[1],
             &sid.addr[2], &sid.addr[3],
             &sid.addr[4], &sid.addr[5]);
    if (retmac == 6) {
        /* sscanf read 6 hex strings, but any of the hex strings may be cut off if it included
         * a non hex character or was longer than 2 characters
         */
        start_ptr = argv[2];
        for (i = 0; i < TSN_ETH_ALEN; i++) {
            if (i > 0) {
                ptr++; // skip char ":"
                start_ptr = ptr;
            }
            hexval = strtol(start_ptr, &ptr, 16);
            if ((hexval != sid.addr[i]) || ((*ptr != ':') && (*ptr != '\0'))) {
                pr_error("MAC_ADDR out of range\n");
                return EINVAL;
            }
        }

    } else {
        pr_error("MAC_ADDR out of range\n");
        return EINVAL;
    }

    if (!strcmp("tagged", argv[3]))
        sid.tag = SID_TAGGED;
    else if (!strcmp("priority", argv[3]))
        sid.tag = SID_PRIORITY;
    else if (!strcmp("all", argv[3])) {
        sid.tag = SID_ALL;
    } else {
        pr_error("Invalid VLAN_TAGGED parameter: '%s'.\n", argv[3]);
        return EINVAL;
    }

    assert_param_parse(sscanf(argv[4], "%lld", &vid) != 1);
    if (vid != (uint16_t) vid) {
        pr_error("VID %lld out of range\n", vid);
        return EINVAL;
    } else {
        sid.vid = (uint16_t) vid;
    }

    brdev = get_if(argv[5]);
    ret = tsn_sid_add_ident(brdev, (uint16_t)ord, &sid);

    if (TSN_ERRNO_ERROR_CODE(ret) == EEXIST) {
        pr_error("Entry already exists\n");
    } else if (TSN_ERRNO_ERROR_CODE(ret) == ENOSPC) {
        pr_error("Hash table conflict occured. Please check the kernel log for more information.\n");
    }
    else if (ret) {
        pr_error("Not able to create entry - %d (%s)\n", errno, strerror(errno));
        tsn_sid_delete_entry (brdev, (uint16_t)ord);
    }

    release_if(brdev);
    return ret;
}

static uint32_t do_sid_addport(int argc, char **argv) {

    uint32_t ret = 0u;
    struct port_hdl *brdev;
    int long long ord;
    struct port_pos sid; /*Only set fields related to add command*/

    /* add ORD POS DEV BRDEV */
    /* POS is one of  {in-fac-out | in-fac-in | out-fac-out | out-fac-in*/

    assert_param_cnt(argc, 4);

    assert_param_parse(sscanf(argv[0], "%lld", &ord) != 1);
    if ((unsigned)ord > SID_MAX_ORD) {
        pr_error("ORD %lld out of range\n", ord);
        return EINVAL;
    }

    brdev = get_if(argv[3]);
    sid.port = tsn_get_id_by_name(brdev, argv[2]);
    sid.pos = sid_get_position(argv[1]);

    if ((sid.port == TSN_PORT_ID_INVALID) || (sid.pos >= POS_UNKNOWN))
        ret = EINVAL;
    else
        ret = tsn_sid_add_port_pos(brdev, ord, &sid);

    if (ret)
        pr_error("Not able to add entry - %d (%s)\n", errno, strerror(errno));

    release_if(brdev);
    return ret;

}

static uint32_t do_sid_delport(int argc, char **argv) {

    uint32_t ret = 0u;
    struct port_hdl *brdev;
    int long long ord;
    struct port_pos sid; /*Only set fields related to add command*/
    /*ORD POS DEV BRDEV*/
    assert_param_cnt(argc, 4);

    assert_param_parse(sscanf(argv[0], "%lld", &ord) != 1);
    if ((unsigned)ord > SID_MAX_ORD) {
        pr_error("ORD %lld out of range\n", ord);
        return EINVAL;
    }

    brdev = get_if(argv[3]);
    sid.port = tsn_get_id_by_name(brdev, argv[2]);
    sid.pos = sid_get_position(argv[1]);

    if ((sid.port == TSN_PORT_ID_INVALID) || (sid.pos >= POS_UNKNOWN))
        ret = EINVAL;
    else
        ret = tsn_sid_del_port_pos (brdev, ord, &sid);

    if (TSN_ERRNO_ERROR_CODE(ret) == ENOENT) {
        pr_error("Entry does not exist.\n");
    } else if (ret) {
        pr_error("Not able to delete port - %d (%s)\n", errno, strerror(errno));
    }

    release_if(brdev);
    return ret;

}

static uint32_t do_sid_delete(int argc, char **argv) {

    uint32_t ret = 0u;
    struct port_hdl *brdev;
    int long long ord; /*Only set fields related to add command*/
    /*ORD POS DEV BRDEV*/
    assert_param_cnt(argc, 2);

    assert_param_parse(sscanf(argv[0], "%lld", &ord) != 1);
    if ((unsigned)ord > SID_MAX_ORD) {
        pr_error("ORD %lld out of range\n", ord);
        return EINVAL;
    }

    brdev = get_if(argv[1]);
    ret = tsn_sid_delete_entry(brdev, (uint16_t) ord);

    if (TSN_ERRNO_ERROR_CODE(ret) == ENOENT) {
        pr_error("Entry does not exist.\n");
    } else if (ret) {
        pr_error("Not able to delete entry - %d (%s)\n",
                 errno, strerror(errno));
    }

    release_if(brdev);
    return ret;

}

static uint32_t do_sid_show(int argc, char **argv) {

    struct port_hdl *brdev;
    int long long ord;
    uint32_t ret = 0u;

    /*BRDEV [ORD]*/
    if (argc == 1) {
        brdev = get_if(argv[0]);
        ret = get_ord_list(brdev);
        release_if(brdev);
    }
    else {
        assert_param_parse(sscanf(argv[1], "%lld", &ord) != 1);
        if ((unsigned)ord > SID_MAX_ORD) {
            pr_error("ORD %lld out of range\n", ord);
            ret = EINVAL;
        } else {
            brdev = get_if(argv[0]);
            ret = show_ident_params(brdev, (uint16_t) ord);
            release_if(brdev);
        }
    }

    return ret;
}

static uint32_t do_sid_show_cnt(int argc, char **argv) {
    struct port_hdl *port;
    uint32_t cnt_in, cnt_out = 0;
    int long long str;
    uint32_t ret = 0u;
    assert_param_cnt(argc, 2);

    assert_param_parse(sscanf(argv[1], "%lld", &str) != 1);
    if (str != (uint16_t) str) {
        pr_error("STREAM_HDL %lld out of range\n", str);
        return EINVAL;
    }

    port = get_if(argv[0]);
    ret = tsn_sid_get_input_cps_cnt(port, (uint16_t) str, &cnt_in);
    if (!ret)
        ret = tsn_sid_get_output_cps_cnt(port, (uint16_t) str, &cnt_out);

    if (ret) {
        pr_error("Couldn't find the stream handle in the database\n");
    } else {
        pr_output("Per Port Per Stream Stream Identification Counters:\n");
        pr_output("Input Counter Value =  %u\nOutput Counter Value = %u, \n", cnt_in, cnt_out);
    }

    release_if(port);
    return ret;
}

static uint32_t do_sid_setstreamhdl(int argc, char **argv) {

    uint32_t ret = 0u;
    struct port_hdl *brdev;
    int long long ord;
    int long long str_hdl;

    /* setstreamhdl ORD STREAM_HDL BRDEV */

    assert_param_cnt(argc, 3);

    assert_param_parse(sscanf(argv[0], "%lld", &ord) != 1);
    if ((unsigned)ord > SID_MAX_ORD) {
        pr_error("ORD %lld out of range\n", ord);
        return EINVAL;
    }

    assert_param_parse(sscanf(argv[1], "%lld", &str_hdl) != 1);
    if (str_hdl != (uint16_t) str_hdl) {
        pr_error("Stream handle %lld out of range\n", str_hdl);
        return EINVAL;
    }

    brdev = get_if(argv[2]);

    ret = tsn_sid_set_strhdl(brdev, ord, (uint16_t)str_hdl);

    if (TSN_ERRNO_ERROR_CODE(ret) == ENOENT) {
        pr_error("Entry does not exist.\n");
    } else if (ret) {
        pr_error("Not able to set stream handle - %d (%s)\n",
                 errno, strerror(errno));
    }

    release_if(brdev);
    return ret;

}

static uint32_t do_sid_help(int argc, char **argv) {
    pr_output("Usage: %s showmax BRDEV\n", _context);
    pr_output("       %s create ORD {NULL | SRC | DEST | IP} IDENT_PARAMS BRDEV \n", _context);
    pr_output("                   where: IDENT_PARAMS is MAC_ADDR VLAN_TAGGED VID\n");
    pr_output("                          VLAN_TAGGED is one of {tagged | priority | all}\n");
    pr_output("       %s delete ORD BRDEV \n", _context);
    pr_output("       %s addport ORD POS DEV BRDEV \n", _context);
    pr_output("                   where: POS is one of  {in-fac-out | in-fac-in | out-fac-out | out-fac-in}\n");
    pr_output("       %s delport ORD POS DEV BRDEV \n", _context);
    pr_output("       %s setstreamhdl ORD STREAM_HDL BRDEV \n", _context);
    pr_output("       %s show BRDEV [ORD] \n", _context);
    pr_output("       %s cntshow DEV STREAM_HDL\n", _context);
    pr_output("       %s tablen BRDEV \n", _context);
    pr_output("       %s ports BRDEV \n", _context);
    pr_output("       %s types BRDEV \n", _context);
    (void)argc;
    (void)argv;
    exit(0);
}

static const struct cmd sid_cmds[] = {
    { "showmax", do_sid_show_max_sup },
    { "create", do_sid_create },
    { "delete", do_sid_delete },
    { "addport", do_sid_addport },
    { "delport", do_sid_delport },
    { "setstreamhdl", do_sid_setstreamhdl },
    { "show", do_sid_show },
    { "cntshow", do_sid_show_cnt },
    { "tablen", do_sid_show_tab_len },
    { "ports", do_sid_show_ports },
    { "types", do_sid_show_types },
    { "help", do_sid_help },
    NO_MORE_CMDS
};

uint32_t do_sid(int argc, char **argv) {
    return do_cmd(argc, argv, sid_cmds);
}
