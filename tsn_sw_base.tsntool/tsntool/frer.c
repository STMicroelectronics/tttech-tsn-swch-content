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
#include "tsn_frer.h"
#include "tsntool.h"
#include "tsn_def.h"
#include "tsn_sid.h"

#define FRER_SHOW_ALL 0xFFFF
#define _SEQGEN_INVALID 0xFFFF
#define _SEQENC_INVALID 0xFFFF
#define _SEQREC_INV_BOOL 0xFF
#define _SEQREC_ALG_UNKOWN 0x2
#define _SEQREC_INV_U16 0xFFFF
#define FRER_MAX_PORT_CNT (16u)

enum _seqenc_active {
    ENC_PASSIVE = 0,
    ENC_ACTIVE = 1,
    ENC_UNKNOWN = 2,//used when entry is added but active not set yet
    ENC_UNDEFINED = 3,//used at init, entry doesnt exist

};
enum _seqenc_dir {
    DIR_IN_FAC = 0,
    DIR_OUT_FAC,
    DIR_UNKNOWN,
};
enum _seqenc_type {
    TYPE_UNKNOWN = 0,
    TYPE_R_TAG = 1, //Only R-tag used
    TYPE_HSR_TAG = 2,
    TYPE_PRP_TRAILER = 3,
};

static void seqenc_set_active_str(uint8_t active, char *str)
{
    if (active == ENC_ACTIVE)
        strncpy(str, "seqenc function Active", 23);
    else if (active == ENC_PASSIVE)
        strncpy(str, "seqenc function Passive", 24);
    else
        strncpy(str, "no seqenc function", 19);
}

static void seqenc_set_dir_str(uint8_t direction, char *dir_str, size_t length)
{
    if (direction == DIR_IN_FAC)
        strncpy(dir_str, "IN-FACING", length);
    else if (direction == DIR_OUT_FAC)
        strncpy(dir_str, "OUT-FACING", length);
    else
        strncpy(dir_str, "UNKNOWN", length);
}

static void seqenc_set_type_str(uint8_t type, char *type_str, size_t length)
{
    if (type == TYPE_R_TAG)
        strncpy(type_str, "R_TAG", length);
    else if (type == TYPE_HSR_TAG)
        strncpy(type_str, "HSR_TAG", length);
    else if (type == TYPE_PRP_TRAILER)
        strncpy(type_str, "PRP_TRAILER", length);
    else
        strncpy(type_str, "UNKNOWN", length);
}

static uint32_t seqenc_show_port2str(struct port_hdl *port, struct port_hdl *brdev,
                                     char *port_str)
{
    uint16_t *strlist;
    uint32_t ret = 0u;
    uint32_t max_streams, i;
    uint32_t active, type;
    uint8_t direction;
    char str[24] = "\0";
    char dir_str[11] = "\0";
    char type_str[12] = "\0";

    ret = tsn_sid_get_max_sup(brdev, &max_streams);
    if (ret) {
        pr_error("Cannot get maximum number of supported Stream Handles"
                 " - %d (%s)\n", errno, strerror(errno));
        return ENODATA;
    }
    max_streams++;

    ret = tsn_frer_seqenc_getactive(port, &active);
    if (ret) {
        pr_error("Cannot get ACTIVE status of port %s"
                 " - %d (%s)\n", port_str, errno, strerror(errno));
        return ENODATA;
    }
    ret = tsn_frer_seqenc_getdir(port, &direction);
    if (ret) {
        pr_error("Cannot get DIR of port %s"
                 " - %d (%s)\n", port_str, errno, strerror(errno));
        return ENODATA;
    }
    ret = tsn_frer_seqenc_gettype(port, &type);
    if (ret) {
        pr_error("Cannot get TYPE of port %s"
                 " - %d (%s)\n", port_str, errno, strerror(errno));
        return ENODATA;
    }
    seqenc_set_active_str(active, str);
    seqenc_set_dir_str(direction, dir_str, sizeof(dir_str));
    seqenc_set_type_str(type, type_str, sizeof(type_str));
    pr_output("Port %s: %s, direction %s, type %s\n",
              port_str, str, dir_str, type_str);

    if ((strlist =  malloc(sizeof(*strlist) * max_streams)) == NULL) {
        pr_error("ERROR: Cannot allocate memory for SeqGen Streams list\n");
        return ENOMEM;
    }

    ret = tsn_frer_seqenc_port2str(port, max_streams, strlist);
    if (ret) {
        pr_error("Cannot get streams added to %s - %d (%s)\n", port_str, errno, strerror(errno));
    } else {
        pr_output("Streams assigned to encode-decode function: \n");
        for (i = 0; i < max_streams; i++) {
            if (strlist[i] == _SEQENC_INVALID) {
                if (i == 0)
                    pr_output("NONE\n");
                break;
            }
            pr_output("%u\n", strlist[i]);
        }
        pr_output("\n");
    }

    free(strlist);
    return ret;
}

static uint32_t do_frer_seqenc_setactive(int argc, char **argv) {

    uint32_t ret = 0u;
    struct port_hdl *port;
    long long int active;

    /* ACTIVE PORT */
    assert_param_cnt(argc, 2);

    assert_param_parse(sscanf(argv[0], "%lld", &active) != 1);
    if (active != (uint8_t) active) {
        pr_error("ACTIVE %lld out of range - %d (%s)\n", active, errno, strerror(errno));
        return EOVERFLOW;
    }
    port = get_if(argv[1]);

    ret = tsn_frer_seqenc_setactive(port, (uint8_t) active);

    if (ret) {
        pr_error("Error setting Active parameter for Sequence Encode-Decode entry - %d (%s)\n", errno, strerror(errno));
    }

    release_if(port);
    return ret;

}

static uint32_t do_frer_seqenc_create(int argc, char **argv) {

    uint32_t ret = 0u;
    struct port_hdl *port;
    int long long dir;
    int long long type;
    int long long active;

    /*  DIR ACTIVE DEV */
    assert_param_cnt(argc, 4);

    assert_param_parse(sscanf(argv[0], "%lld", &dir) != 1);
    if (dir != (uint8_t)dir) {
        pr_error("DIR %lld out of range\n", dir);
        return EOVERFLOW;
    }
    assert_param_parse(sscanf(argv[1], "%lld", &type) != 1);
    if (type != (uint8_t) type) {
        pr_error("TYPE %lld out of range \n", type);
        return EOVERFLOW;
    }
    assert_param_parse(sscanf(argv[2], "%lld", &active) != 1);
    if(active != (uint8_t)active) {
        pr_error("ACTIVE %lld out of range\n", active);
        return EOVERFLOW;
    }

    port = get_if(argv[3]);

    ret = tsn_frer_seqenc_create(port, (uint8_t) dir);

    if (ret) {
        pr_error("Error creating Sequence Encode-Decode entry - %d (%s)\n", errno, strerror(errno));
    } else {
    	ret = tsn_frer_seqenc_settype(port, (uint8_t) type);
    	if (ret) {
            pr_error("Error setting type param for Sequence Encode-Decode entry - %d (%s)\n",
                     errno, strerror(errno));
            //rollback: delete what has already been written
            tsn_frer_seqenc_delport(port);
    	} else {
            ret = tsn_frer_seqenc_setactive(port, (uint8_t) active);
            if (ret) {
                pr_error("Error setting active param for Sequence Encode-Decode entry - %d (%s)\n",
                         errno, strerror(errno));
                //rollback: delete what has already been written
                tsn_frer_seqenc_delport(port);
            }
        }
    }
    release_if(port);
    return ret;
}

static uint32_t do_frer_seqenc_addstr(int argc, char **argv) {

    uint32_t ret = 0u;
    struct port_hdl *port;
    int long long strhdl;

    /* STREAM_HDL TYPE PORT */
    assert_param_cnt(argc, 2);

    assert_param_parse(sscanf(argv[0], "%lld", &strhdl) != 1);
    if (strhdl != (uint16_t) strhdl) {
        pr_error("STREAM_HDL %lld out of range \n", strhdl);
        return EOVERFLOW;
    }

    port = get_if(argv[1]);

    ret = tsn_frer_seqenc_add_str(port, (uint16_t) strhdl);
    if (ret) {
        pr_error("Error adding stream to Sequence Encode-Decode entry - %d (%s)\n",
                 errno, strerror(errno));
    }

    release_if(port);
    return ret;
}

static uint32_t do_frer_seqenc_delport(int argc, char **argv) {

    uint32_t ret = 0u;
    struct port_hdl *port;

    assert_param_cnt(argc, 1);

    port = get_if(argv[0]);

    ret = tsn_frer_seqenc_delport(port);

    if (ret)
        pr_error("Error deleting Sequence Encode-Decode entry - %d (%s)\n", errno, strerror(errno));

    release_if(port);
    return ret;

}

static uint32_t do_frer_seqenc_delstr(int argc, char **argv) {

    uint32_t ret = 0u;
    struct port_hdl *port;
    int long long strhdl;

    /* STREAM_HDL PORT */
    assert_param_cnt(argc, 2);

    assert_param_parse(sscanf(argv[0], "%lld", &strhdl) != 1);
    if (strhdl != (uint16_t) strhdl) {
        pr_error("STREAM_HDL %lld out of range \n", strhdl);
        return EOVERFLOW;
    }
    port = get_if(argv[1]);

    ret = tsn_frer_seqenc_delstr(port, (uint16_t) strhdl);

    if (ret)
        pr_error("Error deleting stream from Sequence Encode-Decode entry - %d (%s)\n", errno, strerror(errno));

    release_if(port);
    return ret;

}

static uint32_t do_frer_seqenc_str2port(int argc, char **argv) {

    struct port_hdl *brdev;
    char *port_name;
    char str[24] = "\0";
    uint32_t active;
    uint16_t  i, portmask;
    long long int strhdl;
    uint32_t ret = 0u;
    struct port_hdl *port;

    /* STREAM_HDL brdev */
    assert_param_cnt(argc, 2);

    assert_param_parse(sscanf(argv[0], "%lld", &strhdl) != 1);
    if (strhdl != (uint16_t) strhdl) {
        pr_error("STREAM_HDL %lld out of range \n", strhdl);
        return EOVERFLOW;
    }
    brdev = get_if(argv[1]);

    ret = tsn_frer_seqenc_str2port(brdev, (uint16_t) strhdl, &portmask);

    if (ret)
        pr_error("Error getting Sequence Encode-Decode str2port - %d (%s)\n", errno, strerror(errno));
    else {

        for (i = 0; i < FRER_MAX_PORT_CNT; i++) {
            if ((portmask << 1) & (1 << i)) {
                port_name = tsn_get_name_by_id(brdev, i);
                if (port_name) {
                    port = get_if(port_name);
                    ret = tsn_frer_seqenc_getactive(port, &active);
                    seqenc_set_active_str(active, str);
                    pr_output("%s %s\n", port_name, str);
                    release_if(port);
                    free(port_name);
                } else {
                    pr_error("Cannot get name of port %u! \n", i);
                    break;
                }
            }
        }
        pr_output("\n");
    }

    release_if(brdev);
    return ret;
}

static uint32_t do_frer_seqenc_port2str(int argc, char **argv) {

    uint32_t ret = 0u;
    struct port_hdl *port;
    struct port_hdl *brdev;
    uint32_t numofports, i;
    char *port_name;

    /*BRDEV [str-hdl | fn-idx NUM]*/
    if (argc < 1 || argc > 2) {
        pr_error("ERROR: Incorrect number of input arguments\n");
        return EINVAL;
    }

    /* BRDEV [DEV]*/
    brdev = get_if(argv[0]);
    if (argc > 1) {
        port = get_if(argv[1]);
        ret = seqenc_show_port2str(port, brdev, argv[1]);
        release_if(port);
    } else {
        /*show data for all ports*/
        ret = tsn_frer_get_max_ports(brdev, &numofports);

        for (i = 1; i <= numofports; i++) {
            port_name = tsn_get_name_by_id(brdev, i);
            if (port_name) {
                port = get_if(port_name);
                ret = seqenc_show_port2str(port, brdev, port_name);
                release_if(port);
                free(port_name);
            } else {
                pr_error("Cannot get name of port %u!\n", i);
                break;
            }
        }
    }

    if (ret)
        pr_error("Error getting Sequence Encode-Decode port2str - %d (%s)\n", errno, strerror(errno));

    release_if(brdev);
    return ret;
}

static uint32_t do_frer_seqgen_create(int argc, char **argv) {

    uint32_t ret = 0u;
    struct port_hdl *brdev;
    int long long fnidx, dir;

    /* FN_IDX DIR BRDEV */
    assert_param_cnt(argc, 3);

    assert_param_parse(sscanf(argv[0], "%lld", &fnidx) != 1);
    if (fnidx != (uint16_t) fnidx) {
        pr_error("FN_IDX %lld out of range \n", fnidx);
        return EOVERFLOW;
    }
    assert_param_parse(sscanf(argv[1], "%lld", &dir) != 1);
    if (dir != (uint8_t) dir) {
        pr_error("DIR %lld out of range \n", dir);
        return EOVERFLOW;
    }
    brdev = get_if(argv[2]);

    ret = tsn_frer_seqgen_create(brdev, (uint16_t) fnidx);

    if (ret) {
        pr_error("Error creating Sequence Generation Function entry %lld - %d (%s)\n",
                 fnidx, errno, strerror(errno));
        release_if(brdev);
        return ret;
    } else {
        ret = tsn_frer_seqgen_add_dir(brdev, (uint16_t) fnidx, (uint8_t) dir);
        if (ret) {
            pr_error("Error adding DIR %lld to Function entry %lld - %d (%s)\n",
                     dir, fnidx, errno, strerror(errno));
            tsn_frer_seqgen_del_fn(brdev, (uint16_t) fnidx);
        }
    }

    release_if(brdev);
    return ret;
}

static uint32_t do_frer_seqgen_strhdl_add(int argc, char **argv) {
    struct port_hdl *brdev;
    int long long strhdl, fnidx;
    uint32_t ret = 0u;

    /*STREAM_HDL FN_IDX BRDEV*/
    assert_param_cnt(argc, 3);
    assert_param_parse(sscanf(argv[0], "%lld", &strhdl) != 1);
    if (strhdl != (uint16_t) strhdl) {
        pr_error("STREAM_HDL %lld out of range\n", strhdl);
        return EOVERFLOW;
    }
    assert_param_parse(sscanf(argv[1], "%lld", &fnidx) != 1);
    if (fnidx != (uint16_t) fnidx) {
        pr_error("FN_IDX %lld out of range\n", fnidx);
        return EOVERFLOW;
    }

    brdev = get_if(argv[2]);
    ret = tsn_frer_seqgen_add_strhdl(brdev, (uint16_t) fnidx, (uint16_t) strhdl);

    if (ret)
        pr_error("Error adding Stream Handle %lld to Sequence Generation Function %lld - %d (%s)\n",
                 strhdl, fnidx, errno, strerror(errno));

    release_if(brdev);
    return ret;
}

static uint32_t do_frer_seqgen_del(int argc, char **argv) {
    uint32_t ret = 0u;
    struct port_hdl *brdev;
    int long long idx = 0;

    /* add  BRDEV {stream-hdl | fn-idx} NUM */
    assert_param_cnt(argc, 3);

    if (!((!strcmp("str-hdl", argv[0])) || ((!strcmp("fn-idx", argv[0])))))
        // parameter argv[0] neither equal "str-hdl" nor equal "fn-idx"
        assert_param_parse(true);

    assert_param_parse(sscanf(argv[1], "%lld", &idx) != 1);
    if (idx != (uint16_t) idx) {
        pr_error("NUM %lld out of range \n", idx);
        return EOVERFLOW;
    }

    brdev = get_if(argv[2]);

    if (!strcmp("str-hdl", argv[0])) {
        ret = tsn_frer_seqgen_del_strhdl(brdev, (uint16_t) idx);
    } else {   //(argv[0]) equal "fn-idx", )
        ret = tsn_frer_seqgen_del_fn(brdev, (uint16_t) idx);
    }

    if (ret)
        pr_error("Error deleting str-hdl/fn-idx %lld - %d (%s)\n", idx, errno, strerror(errno));

    release_if(brdev);
    return ret;

}

static uint32_t seqgen_show_sid2fn(struct port_hdl *brdev, uint16_t strhdl)
{
    uint32_t ret = 0u;
    uint16_t fn;
    uint8_t dir;

    ret = tsn_frer_seqgen_sid2fn(brdev, strhdl, &fn);
    if (!ret) {
        if (tsn_frer_seqgen_fn2dir(brdev, fn, &dir)) {
            pr_error("Cannot get direction for SeqGenFn %u\n", fn);
            return EINVAL;
        }
        pr_output("Stream Handle %u assigned to SeqGenFn: %u (%s)\n",
                  strhdl, fn, dir ? "OUT-FACING" : "IN-FACING");
        return 0;
    } else
        return ret;
}

static uint32_t seqgen_show_fn2sid(struct port_hdl *brdev, uint16_t fnidx)
{
    uint16_t *strlist;
    uint32_t ret = 0u;
    uint32_t max_streams, i;
    uint8_t dir;

    ret = tsn_sid_get_max_sup(brdev, &max_streams);
    if (ret) {
        pr_error("Cannot get maximum number of supported Stream Handles"
                 " - %d (%s)\n", errno, strerror(errno));
        return ENODATA;
    }
    max_streams++;

    if ((strlist =  malloc(sizeof(*strlist) * max_streams)) == NULL) {
        pr_error("ERROR: Cannot allocate memory for SeqGen Streams list\n");
        return ENOMEM;
    }

    if (!ret) {
        if (tsn_frer_seqgen_fn2sid(brdev, fnidx, strlist, max_streams))
            pr_error("SeqGen:Cannot get Stream list for function %u\n", fnidx);
        else {
            if (tsn_frer_seqgen_fn2dir(brdev, fnidx, &dir)) {
                pr_error("Cannot get direction for SeqGenFn %u\n", fnidx);
                ret = EINVAL;
            }
            pr_output("\nSeqGenFn %u (%s)\n",
                      fnidx,
                      dir ? "OUT-FACING" : "IN-FACING");
            pr_output("Stream handles: ");
            for (i = 0; i < max_streams; i++) {
                if (strlist[i] == _SEQGEN_INVALID)
                    break;
                pr_output("%u  ", strlist[i]);
            }
            if (i == 0)
                pr_output("NONE");
            pr_output("\n");
        }
    }
    free(strlist);
    return ret;
}

static uint32_t seqgen_show_fn_list(struct port_hdl *brdev)
{
    uint16_t cnt = 0, i = 0;
    uint16_t *fn_list;
    uint32_t ret = 0u;

    tsn_frer_seqgen_fn_cnt(brdev, &cnt);

    if (cnt == 0) {
        pr_error("No Sequence Generation functions added\n");
        return ENOENT;
    }

    if ((fn_list = malloc(sizeof(uint16_t) * cnt)) == NULL) {
        pr_error("ERROR: Cannot allocate memory for Sequence Generation function list\n");
        return ENOMEM;
    }

    if (tsn_frer_seqgen_list_fn(brdev, cnt, fn_list)) {
        free(fn_list);
        return EINVAL;
    }

    for (i = 0; i < cnt; i++) {
        seqgen_show_fn2sid(brdev, fn_list[i]);
    }

    free(fn_list);

    return ret;
}

static uint32_t do_frer_seqgen_show(int argc, char **argv) {

    uint32_t ret = 0u;
    struct port_hdl *brdev;
    int long long idx;

    /*BRDEV [str-hdl | fn-idx NUM]*/
    if ((argc != 1) && (argc != 3))
        assert_param_parse(true);

    pr_output("\n");
    if (argc == 1) {
        brdev = get_if(argv[0]);
        ret = seqgen_show_fn_list(brdev);
        release_if(brdev);
    } else {  // (argc == 3)
        if (!((!strcmp("str-hdl", argv[1])) || (!strcmp("fn-idx", argv[1]))))
            assert_param_parse(true);
        assert_param_parse(sscanf(argv[2], "%lld", &idx) != 1);
        if (idx != (uint16_t) idx) {
            pr_error("NUM %lld out of range \n", idx);
            return EOVERFLOW;
        }
        brdev = get_if(argv[0]);
        if (!strcmp("str-hdl", argv[1])) {
            ret = seqgen_show_sid2fn(brdev, (uint16_t) idx);
        } else if (!strcmp("fn-idx", argv[1])) {
            ret = seqgen_show_fn2sid(brdev, (uint16_t) idx);
        }
        release_if(brdev);
    }

    if (ret)
        pr_error("Error Showing Sequence Generation Function - %d (%s)\n", errno, strerror(errno));

    pr_output("\n");
    return ret;
}

static uint32_t do_frer_seqrec_create(int argc, char **argv) {

    uint32_t ret = 0u;
    struct port_hdl *brdev;
    long long int fnidx;

    /* FN_IDX BRDEV */
    assert_param_cnt(argc, 2);

    assert_param_parse(sscanf(argv[0], "%lld", &fnidx) != 1);
    if (fnidx != (uint16_t) fnidx) {
        pr_error("FN_IDX %lld out of range \n", fnidx);
        return EOVERFLOW;
    }
    brdev = get_if(argv[1]);

    ret = tsn_frer_seqrec_create(brdev, (uint16_t) fnidx);

    if (ret)
        pr_error("Error creating Sequence Recovery entry - %d (%s)\n", errno, strerror(errno));

    release_if(brdev);
    return ret;
}

static uint32_t do_frer_seqrec_addstr(int argc, char **argv) {

    uint32_t ret = 0u;
    struct port_hdl *brdev;
    long long int fnidx, str;

    /* STREAM_HDL FN_IDX BRDEV */
    assert_param_cnt(argc, 3);

    assert_param_parse(sscanf(argv[0], "%lld", &str) != 1);
    if (str != (uint16_t) str) {
        pr_error("STREAM_HDL %lld out of range \n", str);
        return EOVERFLOW;
    }
    assert_param_parse(sscanf(argv[1], "%lld", &fnidx) != 1);
    if (fnidx != (uint16_t) fnidx) {
        pr_error("FN_IDX %lld out of range \n", fnidx);
        return EOVERFLOW;
    }
    brdev = get_if(argv[2]);

    ret = tsn_frer_seqrec_addstr(brdev, (uint16_t) fnidx, (uint16_t) str);

    if (ret) {
        pr_error("Error adding stream to Sequence Recovery entry - %d (%s)\n", errno, strerror(errno));
    }

    release_if(brdev);
    return ret;

}

static uint32_t do_frer_seqrec_del(int argc, char **argv) {

    uint32_t ret = 0u;
    struct port_hdl *brdev;
    struct port_hdl *port;
    int long long fnidx, str;

    /* BRDEV FN_IDX [{port DEV} | {str NUM}] */
    if ((argc != 2) && (argc != 4)) {
        pr_error("ERROR: Incorrect number of input arguments\n");
        return EINVAL;
    }

    assert_param_parse(sscanf(argv[1], "%lld", &fnidx) != 1);
    if (fnidx != (uint16_t)fnidx) {
        pr_error("FN_IDX %lld out of range\n", fnidx);
        return EOVERFLOW;
    }
    if (argc > 2) {
        if (!((!strcmp("port", argv[2])) || (!strcmp("str-hdl", argv[2]))))
            assert_param_parse(true);
    }

    brdev = get_if(argv[0]);

    if (argc == 2)
        ret = tsn_frer_seqrec_delfn(brdev, (uint16_t) fnidx);
    else {
        if (!strcmp("port", argv[2])) {
            port = get_if(argv[3]);
            ret = tsn_frer_seqrec_delport(port, (uint16_t) fnidx);
            release_if(port);
        } else if (!strcmp("str-hdl", argv[2])) {
            assert_param_parse(sscanf(argv[3], "%lld", &str) != 1);
            if (str == (uint16_t) str)
                ret = tsn_frer_seqrec_delstr(brdev, (uint16_t) fnidx, (uint16_t) str);
            else {
                ret = EOVERFLOW;
                errno = EOVERFLOW;
            }
        }
    }

    if (ret)
        pr_error("Error deleting data from Sequence Recovery entry - %d (%s)\n", errno, strerror(errno));

    release_if(brdev);
    return ret;

}

static uint32_t do_frer_seqrec_addport(int argc, char **argv) {

    uint32_t ret = 0u;
    struct port_hdl *port;
    long long int fnidx;

    /* DEV FN_IDX*/
    assert_param_cnt(argc, 2);

    assert_param_parse(sscanf(argv[1], "%lld", &fnidx) != 1);
    if (fnidx != (uint16_t) fnidx) {
        pr_error("FN_IDX %lld out of range \n", fnidx);
        return EOVERFLOW;
    }
    port = get_if(argv[0]);

    ret = tsn_frer_seqrec_addport(port, (uint16_t) fnidx);

    if (ret)
        pr_error("Error adding port to Sequence Recovery entry - %d (%s)\n", errno, strerror(errno));

    release_if(port);
    return ret;
}

static uint32_t do_frer_seqrec_setparam(int argc, char **argv) {

    uint32_t ret = EINVAL;
    struct port_hdl *brdev;
    long long int fnidx;
    long long int val;

    /*FN_IDX {alg | dir | histlen | rstms | tns | indrec | laterr} VAL BRDEV*/
    assert_param_cnt(argc, 4);

    assert_param_parse(sscanf(argv[0], "%lld", &fnidx) != 1);
    if (fnidx != (uint16_t) fnidx) {
        pr_error("FN_IDX %lld out of range \n", fnidx);
        return EOVERFLOW;
    }
    assert_param_parse(sscanf(argv[2], "%lld", &val) != 1);
    if (((!strcmp("alg", argv[1])) && (val != (uint32_t) val)) ||
            ((!strcmp("dir", argv[1])) && (val != (uint8_t) val))||
            ((!strcmp("histlen", argv[1])) && (val != (uint8_t) val))||
            ((!strcmp("rstms", argv[1])) && (val != (uint16_t) val))||
            ((!strcmp("tns", argv[1])) && (val != (uint8_t) val))||
            ((!strcmp("indrec", argv[1])) && (val != (uint8_t) val))||
            ((!strcmp("laterr", argv[1])) && (val != (uint8_t) val))) {
        pr_error("VAL %lld out of range \n", val);
        return EOVERFLOW;
    }
    brdev = get_if(argv[3]);

    if (!strcmp("alg", argv[1]))
        ret = tsn_frer_seqrec_set_alg(brdev, (uint16_t) fnidx, (uint32_t) val);
    else if (!strcmp("dir", argv[1]))
        ret = tsn_frer_seqrec_set_dir(brdev, (uint16_t) fnidx, (uint8_t)val);
    else if (!strcmp("histlen", argv[1]))
        ret = tsn_frer_seqrec_set_histlen(brdev, (uint16_t) fnidx, (uint8_t)val);
    else if (!strcmp("rstms", argv[1]))
        ret = tsn_frer_seqrec_set_recreset(brdev, (uint16_t) fnidx, (uint16_t)val);
    else if (!strcmp("tns", argv[1]))
        ret = tsn_frer_seqrec_set_takenoseq(brdev, (uint16_t) fnidx, (uint8_t)val);
    else if (!strcmp("indrec", argv[1]))
        ret = tsn_frer_seqrec_set_indrcv(brdev, (uint16_t) fnidx, (uint8_t)val);
    else if (!strcmp("laterr", argv[1]))
        ret = tsn_frer_seqrec_set_laterr(brdev, (uint16_t) fnidx, (uint8_t)val);
    else
        pr_error("Unknown parameter\n");

    if (ret)
        pr_error("Error setting data for Sequence Recovery Entry - %d (%s)\n", errno, strerror(errno));

    release_if(brdev);
    return ret;
}

static uint32_t seqrec_show_fn2sid(struct port_hdl *brdev, uint16_t fnidx)
{
    uint16_t *strlist;
    uint32_t ret = 0u;
    uint32_t max_streams, i;

    ret = tsn_sid_get_max_sup(brdev, &max_streams);
    if (ret) {
        pr_error("Cannot get maximum number of supported Stream Handles"
                 " - %d (%s)\n", errno, strerror(errno));
        return ENODATA;
    }
    max_streams++;

    if ((strlist =  malloc(sizeof(*strlist) * max_streams)) == NULL) {
        pr_error("ERROR: Cannot allocate memory for nSeqRec Streams list\n");
        return ENOMEM;
    }

    if (!ret) {
        ret = tsn_frer_seqrec_fn2sid(brdev, fnidx, strlist, max_streams);
        if (ret)
            pr_error("SeqRec:Cannot get Stream list for function %u\n", fnidx);
        else {
            pr_output("Stream handles assigned to SeqRecFn: ");
            for (i = 0; i < max_streams; i++) {
                if (strlist[i] == _SEQGEN_INVALID)
                    break;
                pr_output("%u  ", strlist[i]);
            }
            if (i == 0)
                pr_output("NONE");
            pr_output("\n");
        }
    }
    pr_output("\n");
    free(strlist);
    return ret;
}

static uint32_t seqrec_show_params(struct port_hdl *brdev, uint16_t fnidx)
{
    uint8_t val8;
    uint32_t val32, ret = 0u;
    uint16_t val16;

    /* First check if the function exists, if it doesnt, return with err message
     * Check can be done by call of any 'get' function, since they will all
     * return same code if the SeqRecFn doesn't exist -ENOENT */
    tsn_frer_seqrec_get_alg(brdev, fnidx, &val32);
    if ((errno == ENOENT) || errno == EINVAL) {
        pr_error("SeqRecFn %u doesn't exist\n", fnidx);
        return ENOENT;
    }

    pr_output("SeqRecFn %u:\n", fnidx);
    if (!tsn_frer_seqrec_get_dir(brdev, fnidx, &val8)){
        if (val8 != DIR_UNKNOWN)
            pr_output("Direction: (%s)\n", val8 ? "OUT-FACING" : "IN-FACING");
        else
            pr_output("Direction: UNKNOWN\n");
    } else
        pr_error("Error SeqRec get Direction - %d (%s)\n", errno, strerror(errno));

    if (!tsn_frer_seqrec_get_alg(brdev, fnidx, &val32)) {
        if (val32 != _SEQREC_ALG_UNKOWN)
            pr_output("%s\n", val32 ? "Match Recovery Algorithm" : "Vectory Recovery Algorithm");
        else
            pr_output("Algorithm: UNKNOWN\n");
    } else
        pr_error("Error SeqRec get Algorithm - %d (%s)\n", errno, strerror(errno));

    if (!tsn_frer_seqrec_get_histlen(brdev, fnidx, &val8)) {
        if (val8 != 0)
            pr_output("History length: (%u)\n", val8);
        else
            pr_output("History length: UNKNOWN\n");
    } else
        pr_error("Error SeqRec get History Length - %d (%s)\n", errno, strerror(errno));

    if (!tsn_frer_seqrec_get_recreset(brdev, fnidx, &val16)) {
        if (val16 != _SEQREC_INV_U16)
            pr_output("Recovery Reset Timeout [ms]: (%u)\n", val16);
        else
            pr_output("Recovery Reset Timeout [ms]: UNKNOWN\n");
    } else
        pr_error("Error SeqRec get Recovery Reset - %d (%s) fnidx = %u\n", errno, strerror(errno), fnidx);

    if (!tsn_frer_seqrec_get_takenoseq(brdev, fnidx, &val8)) {
        if (val8 != _SEQREC_INV_BOOL)
            pr_output("Take No Sequence: %s\n", val8 ? "True" : "False");
        else
            pr_output("Take No Sequence: UNKNOWN\n");
    } else
        pr_error("Error SeqRec get Take No Sequence - %d (%s)\n", errno, strerror(errno));

    if (!tsn_frer_seqrec_get_indrcv(brdev, fnidx, &val8)) {
        if (val8 != _SEQREC_INV_BOOL)
            pr_output("Individual Recovery: %s\n", val8 ? "True" : "False");
        else
            pr_output("Individual Recovery: UNKNOWN\n");
    } else
        pr_error("Error SeqRec get Individual Recovery - %d (%s)\n", errno, strerror(errno));

    if (!tsn_frer_seqrec_get_laterr(brdev, fnidx, &val8)) {
        if (val8 != _SEQREC_INV_BOOL)
            pr_output("Latent Error: %s\n", val8 ? "True" : "False");
        else
            pr_output("Latent Error: UNKNOWN\n");
    } else
        pr_error("Error SeqRec get Latent Error - %d (%s)\n", errno, strerror(errno));

    return ret;
}

static void seqrec_show_ports(struct port_hdl *brdev, uint16_t fnidx)
{
    uint16_t val16;
    uint8_t i;
    char *port_name;

    if (!tsn_frer_seqrec_get_ptmask(brdev, fnidx, &val16)) {
        pr_output("Function is instantiated on ports: ");
        /*val16 is port mask where bit 1 corresponds to port 2 (pt 1 is internal)*/
        for (i = 0; i < FRER_MAX_PORT_CNT; i++) {
            if (val16 &  (1 << i)) {
                port_name = tsn_get_name_by_id(brdev, i);
                if (port_name) {
                    pr_output("%s ", port_name);
                    free(port_name);
                } else {
                    pr_output("\n");
                    pr_error("Cannot get name of port %u!\n", i);
                    break;
                }
            }
        }
        pr_output("\n");
    } else
        pr_error("Error SeqRec get port list - %d (%s)\n", errno, strerror(errno));
}

static uint32_t seqrec_show_list(struct port_hdl *brdev)
{
    uint16_t cnt = 0, i = 0;
    uint16_t *fn_list;
    uint32_t ret = 0u;

    tsn_frer_seqrec_fn_cnt(brdev, &cnt);

    if (cnt == 0) {
        pr_error("No Sequence Recovery functions added\n");
        return ENOENT;
    }

    if ((fn_list = malloc(sizeof(uint16_t) * cnt)) == NULL) {
        pr_error("ERROR: Cannot allocate memory for Sequence Recovery function list\n");
        return ENOMEM;
    }

    if (tsn_frer_seqrec_list_fn(brdev, cnt, fn_list)) {
        pr_error("ERROR: Cannot retrieve Sequence Generation function list\n");
        free(fn_list);
        return EINVAL;
    }

    for (i = 0; i < cnt; i++) {
        ret = seqrec_show_params(brdev, fn_list[i]);
        /*ret == TRUE only if SeqRecFn doesn't exist*/
        if (!ret) {
            seqrec_show_ports(brdev, fn_list[i]);
            seqrec_show_fn2sid(brdev, fn_list[i]);
        }
    }
    free(fn_list);
    return ret;
}

static uint32_t do_frer_seqrec_show(int argc, char **argv) {
    struct port_hdl *brdev;
    long long int fnidx;
    uint32_t ret = 0u;

    /* BRDEV [FN_IDX]*/
    if (argc < 1 || argc > 2) {
        pr_error("ERROR: Incorrect number of input arguments\n");
        return EINVAL;
    }

    if (argc == 2) {
        assert_param_parse(sscanf(argv[1], "%lld", &fnidx) != 1);
        if (fnidx != (uint16_t) fnidx) {
            pr_error("FN_IDX %lld out of range \n", fnidx);
            return EOVERFLOW;
        }

        brdev = get_if(argv[0]);
        ret = seqrec_show_params(brdev, (uint16_t) fnidx);
        /*ret == TRUE only if SeqRecFn doesn't exist*/
        if (!ret) {
            seqrec_show_ports(brdev, (uint16_t) fnidx);

            /* Since there is a lot of parameters to be retrieved and checked,
             * each function will print the error code to the output but return
             * value of the function will be 0. In case one of the function
             * fails, it makes more sense to continue execution and print rest
             * of the information All of this is valid if the SeqRecFn exists
             */
            seqrec_show_fn2sid(brdev, (uint16_t) fnidx);
        }
    } else {
        brdev = get_if(argv[0]);
        seqrec_show_list(brdev);
    }
    release_if(brdev);
    return ret;
}

static uint32_t do_frer_seqrec_fn_cnt(int argc, char **argv) {

    uint32_t ret = 0u;
    struct port_hdl *brdev;
    int long long fnidx;
    uint32_t cnt;

    /*  FN_IDX BRDEV */
    assert_param_cnt(argc, 2);

    assert_param_parse(sscanf(argv[0], "%lld", &fnidx) != 1);
    if (fnidx != (uint16_t) fnidx) {
        pr_error("FN_IDX %lld out of range \n", fnidx);
        return EOVERFLOW;
    }
    brdev = get_if(argv[1]);

    ret = tsn_frer_seqrec_cnt_lost(brdev, (uint16_t) fnidx, &cnt);
    if (!ret) {
        pr_output("frerCpsSeqRcvyLostPackets = %u\n", cnt);
        ret = tsn_frer_seqrec_cnt_resets(brdev, (uint16_t) fnidx, &cnt);
        pr_output("frerCpsSeqRcvyResets = %u\n", cnt);
    }

    if (ret) {
        pr_error("Error getting Frer Counter Value for SeqRecFn %lld, - %d (%s)\n", fnidx, errno, strerror(errno));
    }

    release_if(brdev);
    return ret;
}

static uint32_t do_frer_seqrec_pt_cnt(int argc, char **argv) {

    uint32_t ret = 0u;
    struct port_hdl *port;
    int long long str;
    uint32_t cnt;

    /* DEV STRHDL */
    assert_param_cnt(argc, 2);

    assert_param_parse(sscanf(argv[1], "%lld", &str) != 1);
    if (str != (uint16_t) str) {
        pr_error("STREAM_HDL %lld out of range\n", str);
        return EOVERFLOW;
    }

    port = get_if(argv[0]);
    ret = tsn_frer_seqrec_cnt_outoforder(port, (uint16_t) str, &cnt);

    if (!ret) {
        pr_output("frerCpsSeqRcvyOutOfOrderPackets = %u\n", cnt);
        ret = tsn_frer_seqrec_cnt_tagless(port, (uint16_t) str, &cnt);
        if (!ret) {
            pr_output("frerCpsSeqRcvyTaglessPackets = %u\n", cnt);
            ret = tsn_frer_seqrec_cnt_discarded(port, (uint16_t) str, &cnt);
            if (!ret) {
                pr_output("frerCpsSeqRcvyDiscardedPackets = %u\n", cnt);
                ret = tsn_frer_seqrec_cnt_rogue(port, (uint16_t) str, &cnt);
                if (!ret) {
                    pr_output("frerCpsSeqRcvyRoguePackets = %u\n", cnt);
                    ret = tsn_frer_seqrec_cnt_passed(port, (uint16_t) str, &cnt);
                    if (!ret) {
                        pr_output( "frerCpsSeqRcvyPassedPackets = %u\n", cnt);
                    }
                }
            }
        }
    }

    if (ret) {
        pr_error("Error getting Frer Counter Value - %d (%s)\n", errno, strerror(errno));
    }

    release_if(port);
    return ret;
}

static uint32_t do_frer_help(int argc, char **argv) {
    pr_output("       %s seqgencreate FN_IDX DIR BRDEV \n", _context);
    pr_output("       %s seqgenaddstr STREAM_HDL FN_IDX BRDEV\n", _context);
    pr_output("       %s seqgendel {str-hdl | fn-idx} NUM BRDEV\n", _context);
    pr_output("       %s seqgenshow BRDEV [{str-hdl | fn-idx} NUM] \n\n", _context);
    pr_output("       %s seqenccreate DIR TYPE ACTIVE DEV\n", _context);
    pr_output("       %s seqencaddstr STREAM_HDL DEV\n", _context);
    pr_output("       %s seqencsetactive ACTIVE DEV\n", _context);
    pr_output("       %s seqencdelport DEV\n", _context);
    pr_output("       %s seqencdelstr STREAM_HDL DEV\n", _context);
    pr_output("       %s seqencshow BRDEV [DEV]\n", _context);
    pr_output("       %s seqencstrshow STREAM_HDL BRDEV\n\n", _context);
    pr_output("       %s seqreccreate FN_IDX BRDEV\n", _context);
    pr_output("       %s seqrecaddstr STREAM_HDL FN_IDX BRDEV\n", _context);
    pr_output("       %s seqrecaddport DEV FN_IDX\n", _context);
    pr_output("       %s seqrecset FN_IDX {dir | alg | histlen | rstms | tns | indrec | laterr} VAL BRDEV\n", _context);
    pr_output("       %s seqrecdel BRDEV FN_IDX [{port DEV} | {str-hdl STREAM_HDL}]\n", _context);
    pr_output("       %s seqrecshow BRDEV [FN_IDX]\n", _context);
    pr_output("       %s cntperfn FN_IDX BRDEV\n", _context);
    pr_output("       %s cntportstream DEV STREAM_HDL\n", _context);
    pr_output("     \nwhere: DIR is: 0 -> in-facing or 1-> out-facing\n");

    (void)argc;
    (void)argv;
    exit(0);
}

static const struct cmd sid_cmds[] = {
    { "seqgencreate", do_frer_seqgen_create },
    { "seqgenaddstr", do_frer_seqgen_strhdl_add },
    { "seqgendel", do_frer_seqgen_del },
    { "seqgenshow", do_frer_seqgen_show },

    { "seqenccreate", do_frer_seqenc_create },
    { "seqencaddstr", do_frer_seqenc_addstr },
    { "seqencsetactive", do_frer_seqenc_setactive },
    { "seqencdelport", do_frer_seqenc_delport },
    { "seqencdelstr", do_frer_seqenc_delstr },
    { "seqencstrshow", do_frer_seqenc_str2port },
    { "seqencshow", do_frer_seqenc_port2str },

    { "seqreccreate", do_frer_seqrec_create },
    { "seqrecaddstr", do_frer_seqrec_addstr },
    { "seqrecaddport", do_frer_seqrec_addport },
    { "seqrecset", do_frer_seqrec_setparam },
    { "seqrecdel", do_frer_seqrec_del},
    { "seqrecshow", do_frer_seqrec_show},
    { "cntperfn", do_frer_seqrec_fn_cnt},
    { "cntportstream", do_frer_seqrec_pt_cnt},
    { "help", do_frer_help },
    NO_MORE_CMDS
};

uint32_t do_frer(int argc, char **argv) {
    return do_cmd(argc, argv, sid_cmds);
}
