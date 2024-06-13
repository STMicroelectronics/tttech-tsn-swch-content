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
#include <time.h>
#include <stdio.h>
#include <stdlib.h>

#include "tsn_bridge.h"
#include "tsn_psfp.h"
#include "tsntool.h"

#define MAX_REC         (100+1)
#define GCF_OP_SGI      "sgi"
#define GCF_OP_UNKNOWN  "<unknown>"

enum gcl_select {
    ADMIN_GCL,
    OPER_GCL,
};

void assert_param_parse_bool(const char *str, bool *opt) {
    if (!strcmp("on", str))
        *opt = true;
    else if (!strcmp("off", str))
        *opt = false;
    else
        assert_param_parse(true);
}

static uint32_t do_psfp_filter_create(int argc, char **argv) {
    uint32_t flt_id;
    struct psfp_flt_params params;
    struct port_hdl *brdev;
    uint32_t ret = 0u;

    assert_param_cnt(argc, 9);
    assert_param_parse(sscanf(argv[0], "%"SCNu32, &flt_id) != 1);
    if (!strcmp("any", argv[1]))
        params.str_hdl = PSFP_FLT_ANY_STR_HDL;
    else
        assert_param_parse(sscanf(argv[1], "%"SCNd32, &params.str_hdl) != 1 ||
                           params.str_hdl < 0);
    if (!strcmp("any", argv[2]))
        params.prio = PSFP_FLT_ANY_PRIO;
    else
        assert_param_parse(sscanf(argv[2], "%"SCNd32, &params.prio) != 1 ||
                           params.prio < 0);
    assert_param_parse(sscanf(argv[3], "%"SCNu32, &params.gate_id) != 1);
    assert_param_parse(sscanf(argv[4], "%"SCNu32, &params.max_sdu) != 1);
    if (!strcmp("none", argv[5]))
        params.fmtr_id = PSFP_FLT_NO_FMTR;
    else
        assert_param_parse(sscanf(argv[5], "%"SCNd32, &params.fmtr_id) != 1 ||
                           params.fmtr_id < 0);
    assert_param_parse_bool(argv[6], &params.blk_oversz_ena);
    assert_param_parse_bool(argv[7], &params.blk_oversz);

    brdev = get_if(argv[8]);
    ret = tsn_psfp_flt_create(brdev, flt_id, &params);
    if (TSN_ERRNO_ERROR_CODE(ret) == EEXIST) {
        pr_output("Entry already exists\n");
    }
    else if (ret) {
        pr_error("Not able to create filter %" PRIu32 " - %d (%s)\n",
                 flt_id, errno, strerror(errno));
    }

    release_if(brdev);
    return ret;
}

static uint32_t do_psfp_filter_delete(int argc, char **argv) {
    uint32_t flt_id = 0;
    struct port_hdl *brdev;
    uint32_t ret = 0u;

    assert_param_cnt(argc, 2);
    assert_param_parse(sscanf(argv[0], "%"SCNu32, &flt_id) != 1);

    brdev = get_if(argv[1]);
    if ((ret = tsn_psfp_flt_delete(brdev, flt_id)) != 0u)
        pr_error("Cannot delete PSFP filter %" PRIu32 " - %d (%s)\n",
                 flt_id, errno, strerror(errno));

    release_if(brdev);
    return ret;
}

static uint32_t do_psfp_filter_show_single(
        uint32_t flt_id,
        const struct psfp_flt_params *params) {
    uint32_t ret = 0u;

    pr_output("Filter %"PRIu32 ":\n", flt_id);
    if (params->str_hdl == PSFP_FLT_ANY_STR_HDL)
        pr_output("    StreamHandleSpec:                      %s\n", "any");
    else
        pr_output("    StreamHandleSpec:                      %"PRId32 "\n",
                  params->str_hdl);
    if (params->prio == PSFP_FLT_ANY_PRIO)
        pr_output("    PrioritySpec:                          %s\n", "any");
    else
        pr_output("    PrioritySpec:                          %"PRId32 "\n",
                  params->prio);
    pr_output("    StreamGateInstanceID:                  %"PRIu32 "\n",
              params->gate_id);
    pr_output("    FilterSpecificationList:\n");
    pr_output("        Maximum SDU size:                  %"PRIu32 "\n",
              params->max_sdu);
    if (params->fmtr_id == PSFP_FLT_NO_FMTR)
        pr_output("        Flow meter instance ID:            %s\n", "none");
    else
        pr_output("        Flow meter instance ID:            %"PRIu32 "\n",
                  params->fmtr_id);
    pr_output("    StreamBlockedDueToOversizeFrameEnable: %s\n",
              params->blk_oversz_ena ? "true" : "false");
    pr_output("    StreamBlockedDueToOversizeFrame:       %s\n",
              params->blk_oversz ? "true" : "false");
    pr_output("\n");
    return ret;
}

static uint32_t do_psfp_filter_show(int argc, char **argv) {
    uint32_t flt_id = 0;
    struct port_hdl *brdev;
    uint32_t max_filters;
    struct psfp_flt_params params;
    uint32_t ret = 0u;

    assert_param_min_max(argc, 1, 2);

    brdev = get_if(argv[0]);
    if (argc == 2) {
        assert_param_parse(sscanf(argv[1], "%"SCNu32, &flt_id) != 1);
        if ((ret = tsn_psfp_flt_get_params(brdev, flt_id, &params)) != 0u)
            pr_error("Cannot get PSFP filter parameters for filter %" PRIu32 " - %d (%s)\n",
                     flt_id, errno, strerror(errno));
        else
            do_psfp_filter_show_single(flt_id, &params);
    }
    else {
        ret = tsn_psfp_get_max_filters(brdev, &max_filters);
        if (ret) {
            pr_error("Cannot determine max. number of PSFP filters - %d (%s)\n",
                     errno, strerror(errno));
        }
        else {
            for (flt_id = 0; flt_id < max_filters; flt_id++) {
                ret = tsn_psfp_flt_get_params(brdev, flt_id, &params);
                if (TSN_ERRNO_ERROR_CODE(ret) == ENOENT)
                    continue;
                if (ret != 0u) {
                    pr_error("Cannot get PSFP filter parameters for filter %" PRIu32 " - %d (%s)\n",
                             flt_id, errno, strerror(errno));
                    break;
                }
                else {
                    do_psfp_filter_show_single(flt_id, &params);
                }
            }
        }
    }

    release_if(brdev);
    return ret;
}

static uint32_t do_psfp_filter_cntshow(int argc, char **argv) {
    struct port_hdl *brdev = NULL;
    uint32_t flt_id;
    uint32_t matching_frames;
    uint32_t passing_frames;
    uint32_t not_passing_frames;
    uint32_t passing_sdu;
    uint32_t not_passing_sdu;
    uint32_t red_frames;
    uint32_t ret = 0u;

    assert_param_cnt(argc, 2);
    assert_param_parse(sscanf(argv[0], "%"SCNu32, &flt_id) != 1);
    brdev = get_if(argv[1]);

    pr_output("Filter %"PRIu32 " counters:\n", flt_id);

    ret = tsn_psfp_flt_get_matching_frames_cnt(brdev, flt_id,
                                               &matching_frames);
    if (ret)
        pr_ecode("    MatchingFramesCount:     ERROR: ", ret);
    else
        pr_output("    MatchingFramesCount:     %"PRIu32 "\n",
                  matching_frames);

    ret = tsn_psfp_flt_get_passing_frames_cnt(brdev, flt_id, &passing_frames);
    if (ret)
        pr_ecode("    PassingFramesCount:      ERROR: ", ret);
    else
        pr_output("    PassingFramesCount:      %"PRIu32 "\n", passing_frames);

    ret = tsn_psfp_flt_get_not_passing_frames_cnt(brdev, flt_id,
                                                  &not_passing_frames);
    if (ret)
        pr_ecode("    NotPassingFramesCount:   ERROR: ", ret);
    else
        pr_output("    NotPassingFramesCount:   %"PRIu32 "\n",
                  not_passing_frames);

    ret = tsn_psfp_flt_get_passing_sdu_cnt(brdev, flt_id, &passing_sdu);
    if (ret)
        pr_ecode("    PassingSDUCount:         ERROR: ", ret);
    else
        pr_output("    PassingSDUCount:         %"PRIu32 "\n", passing_sdu);

    ret = tsn_psfp_flt_get_not_passing_sdu_cnt(brdev, flt_id, &not_passing_sdu);
    if (ret)
        pr_ecode("    NotPassingSDUCount:      ERROR: ", ret);
    else
        pr_output("    NotPassingSDUCount:      %"PRIu32 "\n", not_passing_sdu);

    ret = tsn_psfp_flt_get_red_frames_cnt(brdev, flt_id, &red_frames);
    if (ret)
        pr_ecode("    REDFrames:               ERROR: ", ret);
    else
        pr_output("    REDFrames:               %"PRIu32 "\n", red_frames);

    release_if(brdev);
    return ret;
}

static uint32_t do_psfp_filter_help(int argc, char **argv) {
    pr_output("Usage: %s create FILTER_ID STREAM_HDL PRIORITY GATE_ID MAX_SDU FLOWMETER_ID BLOCKED_DUE_TO_OVERSIZE_FRAME_ENABLE BLOCKED_DUE_TO_OVERSIZE_FRAME BRDEV\n", _context);
    pr_output("           where FILTER_ID is NUM\n");
    pr_output("           where STREAM_HDL is one of {any | NUM}\n");
    pr_output("           where PRIORITY is one of {any | 0..7}\n");
    pr_output("           where GATE_ID is NUM\n");
    pr_output("           where MAX_SDU is NUM (46..1506)\n");
    pr_output("           where FLOWMETER_ID is one of {none | NUM}\n");
    pr_output("           where BLOCKED_DUE_TO_OVERSIZE_FRAME_ENABLE is one of {on | off}\n");
    pr_output("           where BLOCKED_DUE_TO_OVERSIZE_FRAME is one of {on | off}\n");
    pr_output("       %s delete FILTER_ID BRDEV\n", _context);
    pr_output("       %s show BRDEV [FILTER_ID]\n", _context);
    pr_output("       %s cntshow FILTER_ID BRDEV\n", _context);
    (void)argc;
    (void)argv;
    exit(0);
}

static const struct cmd psfp_filter_cmds[] = {
    { "create", do_psfp_filter_create },
    { "delete", do_psfp_filter_delete },
    { "show", do_psfp_filter_show },
    { "cntshow", do_psfp_filter_cntshow },
    { "help", do_psfp_filter_help },
    NO_MORE_CMDS
};

static uint32_t do_psfp_filter(int argc, char **argv) {
    return do_cmd(argc, argv, psfp_filter_cmds);
}

static uint32_t parse_record(char *s, struct psfp_gate_control_entry *gce,
                             unsigned int lineno) {
    char *tok;
    char *endptr;
    unsigned long int gsv;
    long int ipv;

    // parse <operation>
    tok = strtok(s, " ");
    if (!tok) {
        pr_error("WARNING: Missing <operation> in line %u\n", lineno);
        return EINVAL;
    }
    if (strcmp(tok, GCF_OP_SGI) == 0)
        gce->op = TSN_PSFP_GT_OP_SET_GATE_AND_IPV;
    else if (strcmp(tok, GCF_OP_UNKNOWN) == 0) {
        pr_error("WARNING: Ignoring reserved <operation> in line %u: '%s'\n",
                 lineno, tok);
        return ENOENT;
    }
    else {
        pr_error("ERROR: Unknown <operation> in line %u: '%s'\n", lineno, tok);
        return EINVAL;
    }

    // parse <interval>
    tok = strtok(NULL, " ");
    if (!tok) {
        pr_error("WARNING: Missing <interval> in line %u\n", lineno);
        return EINVAL;
    }
    // allow decimal only to avoid confusion.
    gce->interval = strtoul(tok, &endptr, 10);
    if ((*tok != '\0') && (*endptr != '\0')) {
        pr_error("ERROR: Unable to parse <interval> in line %u: '%s'\n",
                 lineno, tok);
        return EINVAL;
    }

    // parse <gsv>
    tok = strtok(NULL, " ");
    if (!tok) {
        pr_error("WARNING: Missing <gsv> in line %u\n", lineno);
        return EINVAL;
    }
    gsv = strtoul(tok, &endptr, 0);
    if ((*tok != '\0') && (*endptr != '\0')) {
        pr_error("ERROR: Unable to parse <gsv> in line %u: '%s'\n",
                 lineno, tok);
        return EINVAL;
    }
    if (gsv > UINT8_MAX) {
        pr_error("ERROR: Invalid <gsv> in line %u: '%s'\n", lineno, tok);
        return EINVAL;
    }
    gce->gsv = (uint8_t)gsv;

    // parse <ipv>
    tok = strtok(NULL, " ");
    if (!tok) {
        pr_error("WARNING: Missing <ipv> in line %u\n", lineno);
        return EINVAL;
    }
    ipv = strtol(tok, &endptr, 0);
    if ((*tok != '\0') && (*endptr != '\0')) {
        pr_error("ERROR: Unable to parse <ipv> in line %u: '%s'\n",
                 lineno, tok);
        return EINVAL;
    }
    if (ipv < INT8_MIN || ipv > INT8_MAX) {
        pr_error("ERROR: Invalid <ipv> in line %u: '%s'\n", lineno, tok);
        return EINVAL;
    }
    gce->ipv = (int8_t)ipv;

    // check on excess record elements
    tok = strtok(NULL, " ");
    if (tok) {
        pr_error("ERROR: Excess record elements in line %u: '%s'\n",
                 lineno, tok);
        return EINVAL;
    }

    return 0;
}

static struct psfp_gate_control_entry *read_gcf(const char *fn,
                                                uint32_t *gcl_len,
                                                uint32_t max_len) {
    struct psfp_gate_control_entry *gcl;
    int ch;
    char gce[MAX_REC];
    char *trimmed_gce;
    unsigned int nlines = 0;
    unsigned int lineno;
    FILE *fp = fopen(fn, "r");
    int ret;

    if (fp == NULL) {
        pr_error("ERROR: Cannot open input file '%s' - %d (%s)\n",
                 fn, errno, strerror(errno));
        return NULL;
    }

    do {
        ch = fgetc(fp);
        if (ch == '\n')
            nlines++;
    } while (ch != EOF);
    // just in case the last line doesn't end with \n
    nlines++;
    if (nlines > max_len) {
        // If there are more entries than supported just trim it to the max
        nlines = max_len;
        pr_error("ERROR: Tried to read more entries than supported by implementation, defaulting to %u entries\n",
                 nlines);
    }
    fseek(fp, 0, SEEK_SET);
    if ((gcl = calloc(nlines, sizeof(*gcl))) == NULL) {
        fclose(fp);
        pr_error("ERROR: Cannot allocate memory\n");
        return NULL;
    }

    *gcl_len = 0;
    lineno = 0;
    while (fgets(gce, MAX_REC, fp)) {
        lineno++;
        trimmed_gce = trim(gce);
        if (strlen(trimmed_gce) == 0) // empty line
            continue;
        if (trimmed_gce[0] == '#') // comment
            continue;

        ret = parse_record(trimmed_gce, &gcl[*gcl_len], lineno);
        if (ret == ENOENT)
            continue;

        if (ret) {
            fclose(fp);
            free(gcl);
            return NULL;
        }
        (*gcl_len)++;
    }
    fclose(fp);
    return gcl;
}

static uint32_t write_gcf(struct psfp_gate_control_entry *gcl,
                          uint32_t gcl_len,
                          const char *fn) {
    FILE *fp = fn ? fopen(fn, "w") : stdout;

    if (!fp) {
        int err = errno;
        pr_error("ERROR: Cannot write file '%s' - %d (%s)\n",
                 fn ? fn : "(standard output)", errno, strerror(errno));
        return err;
    }

    for (uint32_t i = 0; i < gcl_len; i++, gcl++) {
        const char *gcf_op;
        switch (gcl->op) {
        case TSN_PSFP_GT_OP_SET_GATE_AND_IPV:
            gcf_op = GCF_OP_SGI;
            break;
        default:
            gcf_op = GCF_OP_UNKNOWN;
            pr_error("ERROR: Unknown operation in record #%u\n", i+1);
            break;
        }
        pr_msg(fp, "%s  %u  0x%02X %" PRIi8 "\n",
               gcf_op, gcl->interval, gcl->gsv, gcl->ipv);
    }
    if (fn)
        fclose(fp);
    return 0;
}

static uint32_t do_psfp_gate_wrcl(int argc, char **argv) {
    struct port_hdl *phdl;
    uint32_t gate;
    struct psfp_gate_control_entry* gcl;
    uint32_t gcl_len, supp_list_max;
    int ret = 0;

    assert_param_cnt(argc, 3);
    assert_param_parse(sscanf(argv[0], "%"SCNu32, &gate) != 1);
    phdl = get_if(argv[1]);

    ret = tsn_psfp_get_supported_list_max(phdl, &supp_list_max);
    if (ret != E_TSN_LIB_NO_ERROR) {
        pr_ecode("ERROR: Cannot get max admin control list length - ", ret);
        release_if(phdl);
        return ret;
    }
    if (supp_list_max == 0u) {
        pr_error("ERROR: Max admin control list length is 0\n");
        release_if(phdl);
        return EINVAL;
    }
    gcl = read_gcf(argv[2], &gcl_len, supp_list_max);
    if (gcl == NULL) {
        release_if(phdl);
        return EPERM;
    }

    ret = tsn_psfp_gt_set_admin_control_list_length(phdl, gate, gcl_len);
    if (ret != E_TSN_LIB_NO_ERROR) {
        pr_ecode("ERROR: Cannot set admin control list length - ", ret);
    }
    else {
        ret = tsn_psfp_gt_set_admin_control_list(phdl, gate, gcl_len, gcl);
        if (ret != E_TSN_LIB_NO_ERROR)
            pr_ecode("ERROR: Cannot set admin control list - ", ret);
    }
    free(gcl);
    release_if(phdl);

    return ret;
}

static uint32_t _do_psfp_gt_rd_gcl(int argc, char **argv, enum gcl_select sel) {
    uint32_t supp_list_max, gcl_len = 1;
    struct psfp_gate_control_entry* gcl;
    int ret = 0;
    struct port_hdl *phdl;
    uint32_t gate;

    assert_param_min_max(argc, 2, 3);
    assert_param_parse(sscanf(argv[0], "%"SCNu32, &gate) != 1);
    phdl = get_if(argv[1]);

    if (sel == ADMIN_GCL) {
        ret = tsn_psfp_gt_get_admin_control_list_length(phdl, gate, &gcl_len);
        if (ret != E_TSN_LIB_NO_ERROR) {
            pr_ecode("ERROR: Cannot get admin control list length - ", ret);
            release_if(phdl);
            return ret;
        }
    }
    else {
        ret = tsn_psfp_gt_get_oper_control_list_length(phdl, gate, &gcl_len);
        if (ret != E_TSN_LIB_NO_ERROR) {
            pr_ecode("ERROR: Cannot get oper. control list length - ", ret);
            release_if(phdl);
            return ret;
        }
    }

    if (!gcl_len) {
        pr_error("Control list length is set to 0 - no entries read.\n");
        release_if(phdl);
        return ret;
    }
    ret = tsn_psfp_get_supported_list_max(phdl, &supp_list_max);
    if (ret != E_TSN_LIB_NO_ERROR) {
        pr_ecode("ERROR: Cannot get max admin control list length - ", ret);
        release_if(phdl);
        return ret;
    }
    if (gcl_len > supp_list_max) {
        // If there are more entries than supported just trim it to the max
        gcl_len = supp_list_max;
        pr_error("ERROR: Tried to read more entries than supported by implementation, defaulting to %" SCNu32 " entries\n",
                 gcl_len);
    }
    gcl = malloc(sizeof(*gcl) * gcl_len);
    if (gcl == NULL) {
        pr_error("ERROR: Cannot allocate memory for control list\n");
        release_if(phdl);
        return EPERM;
    }

    if (sel == ADMIN_GCL) {
        ret = tsn_psfp_gt_get_admin_control_list(phdl, gate, gcl_len, gcl);
        if (ret != E_TSN_LIB_NO_ERROR) {
            pr_ecode("ERROR: Cannot get admin control list - ", ret);
        }
    }
    else {
        ret = tsn_psfp_gt_get_oper_control_list(phdl, gate, gcl_len, gcl);
        if (ret != E_TSN_LIB_NO_ERROR) {
            pr_ecode("ERROR: Cannot get oper. control list - ", ret);
        }
    }
    release_if(phdl);

    if (!ret) {
        if (argc > 2)
            ret = write_gcf(gcl, gcl_len, argv[2]);
        else
            ret = write_gcf(gcl, gcl_len, NULL);
    }
    free(gcl);

    return ret;
}

static uint32_t do_psfp_gate_rd_admin_gcl(int argc, char **argv) {
    return _do_psfp_gt_rd_gcl(argc, argv, ADMIN_GCL);
}

static uint32_t do_psfp_gate_rd_oper_gcl(int argc, char **argv) {
    return _do_psfp_gt_rd_gcl(argc, argv, OPER_GCL);
}

static uint32_t do_psfp_gate_disable(int argc, char **argv) {
    struct port_hdl *phdl = NULL;
    uint32_t gate;
    uint32_t ret = 0u;

    assert_param_cnt(argc, 2);
    assert_param_parse(sscanf(argv[0], "%"SCNu32, &gate) != 1);
    phdl = get_if(argv[1]);

    ret = tsn_psfp_gt_set_gate_enabled(phdl, gate, false);
    if (ret != E_TSN_LIB_NO_ERROR)
        pr_ecode("ERROR: Cannot disable gates - ", ret);
    release_if(phdl);
    return ret;
}

static uint32_t do_psfp_gate_reset(int argc, char **argv) {
    struct port_hdl *phdl = NULL;
    uint32_t gate;
    uint32_t ret = 0u;

    assert_param_cnt(argc, 2);
    assert_param_parse(sscanf(argv[0], "%"SCNu32, &gate) != 1);
    phdl = get_if(argv[1]);

    /* set all gates to open in admin control object before gate disable */
    ret = tsn_psfp_gt_set_admin_gate_states(phdl, gate, SCHED_DEF_GATE_STATES);
    if (ret != E_TSN_LIB_NO_ERROR)
        pr_ecode("ERROR: Cannot set admin gate states - ", ret);
    if (ret == E_TSN_LIB_NO_ERROR) {
        ret = tsn_psfp_gt_set_admin_ipv(phdl, gate, PSFP_GT_IPV_NULL);
        if (ret != E_TSN_LIB_NO_ERROR)
            pr_ecode("ERROR: Cannot set admin IPV - ", ret);
    }
    if (ret == E_TSN_LIB_NO_ERROR) {
        ret = tsn_psfp_gt_set_gate_enabled(phdl, gate, false);
        if (ret != E_TSN_LIB_NO_ERROR)
            pr_ecode("ERROR: Cannot disable gates - ", ret);
    }
    release_if(phdl);
    return ret;
}

static uint32_t do_psfp_gate_configure(int argc, char **argv) {
    struct port_hdl *phdl = NULL;
    uint32_t gate;
    struct tsn_st_ptp_time basetime;
    struct tsn_st_rational_number cycletime;
    long long int numerator, denominator;
    long long int ctextension;
    int items = 0;
    uint32_t ret = 0u;

    assert_param_cnt(argc, 5);
    assert_param_parse(sscanf(argv[0], "%"SCNu32, &gate) != 1);
    phdl = get_if(argv[4]);

    if (*argv[1] == '+' || *argv[1] == '-') {
        // Resolve absolute base time from relative offset given
        struct tsn_st_ptp_time bt;
        struct tsn_st_ptp_time cur;

        if (!str_to_ptp_time(argv[1], &bt))
            items += 2;

        tsn_psfp_gt_get_current_time(phdl, gate, &cur);

        if ( (((signed)cur.sec + (signed)bt.sec) < 0) ||
             ((((signed)cur.sec + (signed)bt.sec) <= 0) && (cur.nsec < bt.nsec))
           ) {
            pr_error("ERROR: Relative time before beginning of PTP epoch\n");
            release_if(phdl);
            return EPERM;
        }

        basetime.sec  = cur.sec  + bt.sec;
        if (bt.sec > 0) {
            basetime.nsec = cur.nsec + bt.nsec;
            if (basetime.nsec >= 1000000000) {
                basetime.nsec -= 1000000000;
                basetime.sec++;
            }
        }
        else {
            if (cur.nsec > bt.nsec)
                basetime.nsec = cur.nsec - bt.nsec;
            else {
                basetime.nsec = cur.nsec + 1000000000 - bt.nsec;
                basetime.sec--;
            }
        }
    } else {
        // Absolute base time given
        if (!str_to_ptp_time(argv[1], &basetime))
            items += 2;
    }

    if (sscanf(argv[2], "%lld/%lld", &numerator, &denominator) == 2) {
        if ((numerator != (uint32_t) numerator) ||
            (denominator != (uint32_t) denominator)) {
            pr_error(" NUMERATOR/DENOMINATOR %lld/%lld out of range\n",
                     numerator, denominator);
            ret = EINVAL;
        } else {
            items = items + 2;
            cycletime.numerator = (uint32_t) numerator;
            cycletime.denominator = (uint32_t) denominator;
        }
    } else {
        pr_error("Wrong parameter syntax");
        ret = EINVAL;
    }
    if (sscanf(argv[3], "%lld", &ctextension) == 1) {
        if (ctextension != (uint32_t) ctextension)  {
            pr_error(" CYCLTIME_EXT %lld out of range\n", ctextension);
            ret = EINVAL;
        } else {
            items++;
        }
    } else {
        pr_error("Wrong parameter syntax");
        ret = EINVAL;
    }

    if ((items != 5) || ret != 0) {
        release_if(phdl);
        return ret;
    }

    ret = tsn_psfp_gt_set_admin_base_time(phdl, gate, &basetime);
    if (ret != E_TSN_LIB_NO_ERROR)
        pr_ecode("ERROR: Cannot set admin base time - ", ret);
    if (ret == E_TSN_LIB_NO_ERROR) {
        ret = tsn_psfp_gt_set_admin_cycle_time(phdl, gate, &cycletime);
        if (ret != E_TSN_LIB_NO_ERROR)
            pr_ecode("ERROR: Cannot set admin cycle time - ", ret);
    }
    if (ret == E_TSN_LIB_NO_ERROR) {
        ret = tsn_psfp_gt_set_admin_cycle_time_extension(phdl, gate,
                                                         (uint32_t)ctextension);
        if (ret != E_TSN_LIB_NO_ERROR)
            pr_ecode("ERROR: Cannot set admin cycle time extension time - ",
                     ret);
    }
    if (ret == E_TSN_LIB_NO_ERROR) {
        ret = tsn_psfp_gt_set_gate_enabled(phdl, gate, true);
        if (ret != E_TSN_LIB_NO_ERROR)
            pr_ecode("ERROR: Cannot enable gates - ", ret);
    }
    if (ret == E_TSN_LIB_NO_ERROR) {
        ret = tsn_psfp_gt_set_config_change(phdl, gate, true);
        if (ret != E_TSN_LIB_NO_ERROR)
            pr_ecode("ERROR: Cannot trigger config change - ", ret);
    }
    release_if(phdl);
    return ret;
}

static uint32_t do_psfp_gate_set(int argc, char **argv) {
    struct port_hdl *phdl = NULL;
    uint32_t gate;
    uint8_t admingatestate;
    int8_t adminipv;
    uint32_t ret = 0u;

    assert_param_cnt(argc, 4);
    assert_param_parse(sscanf(argv[0], "%"SCNu32, &gate) != 1);
    assert_param_parse(sscanf(argv[1], "%"SCNu8, &admingatestate) != 1);
    assert_param_parse(sscanf(argv[2], "%"SCNi8, &adminipv) != 1);
    phdl = get_if(argv[3]);

    ret = tsn_psfp_gt_set_admin_gate_states(phdl, gate, admingatestate);
    if (ret != E_TSN_LIB_NO_ERROR) {
        pr_ecode("ERROR: Cannot set admin gate states - ", ret);
    }
    else {
        ret = tsn_psfp_gt_set_admin_ipv(phdl, gate, adminipv);
        if (ret != E_TSN_LIB_NO_ERROR)
            pr_ecode("ERROR: Cannot set admin IPV - ", ret);
    }
    release_if(phdl);
    return ret;
}

static uint32_t do_psfp_gate_closed_inv_rx(int argc, char **argv) {
    struct port_hdl *phdl = NULL;
    uint32_t gate;
    bool closed_inv_rx_ena;
    bool closed_inv_rx;
    uint32_t ret = 0u;

    assert_param_cnt(argc, 4);
    assert_param_parse(sscanf(argv[0], "%"SCNu32, &gate) != 1);
    assert_param_parse_bool(argv[1], &closed_inv_rx_ena);
    assert_param_parse_bool(argv[2], &closed_inv_rx);
    phdl = get_if(argv[3]);

    ret = tsn_psfp_gt_set_closed_inv_rx_enable(phdl, gate, closed_inv_rx_ena);
    if (ret != E_TSN_LIB_NO_ERROR) {
        pr_ecode("ERROR: Cannot set gate closed due to invalid RX enable - ",
                 ret);
    }
    else {
        ret = tsn_psfp_gt_set_closed_inv_rx(phdl, gate, closed_inv_rx);
        if (ret != E_TSN_LIB_NO_ERROR)
            pr_ecode("ERROR: Cannot set gate closed due to invalid RX - ", ret);
    }
    release_if(phdl);
    return ret;
}

static uint32_t do_psfp_gate_show(int argc, char **argv) {
    struct port_hdl *phdl = NULL;
    uint32_t gate;
    bool gate_en, config_pend;
    time_t adminbase_sec, operbase_sec, current_sec, cctime_sec;
    struct tsn_st_ptp_time adminbase, operbase, current, cctime;
    struct tsn_st_rational_number admincycle, opercycle;
    uint8_t admingatestate;
    int8_t adminipv;
    bool closed_inv_rx_ena;
    bool closed_inv_rx;
    uint32_t admincycleext, opercycleext, list_len;
    uint32_t granularity;
    uint64_t ccerr;
    char buf[100];
    uint32_t ret = 0u;

    assert_param_cnt(argc, 2);
    assert_param_parse(sscanf(argv[0], "%"SCNu32, &gate) != 1);
    phdl = get_if(argv[1]);

    ret = tsn_psfp_gt_get_gate_enabled(phdl, gate, &gate_en);
    if (ret != E_TSN_LIB_NO_ERROR)
        pr_ecode("GatesEnabled:                   ERROR: ", ret);
    else
        pr_output("GatesEnabled:                   %s\n",
                  gate_en ? "YES" : "NO");

    ret = tsn_psfp_gt_get_admin_gate_states(phdl, gate, &admingatestate);
    if (ret != E_TSN_LIB_NO_ERROR)
        pr_ecode("AdminGateStates:                ERROR: ", ret);
    else
        pr_output("AdminGateStates:                %u\n", admingatestate);

    ret = tsn_psfp_gt_get_admin_ipv(phdl, gate, &adminipv);
    if (ret != E_TSN_LIB_NO_ERROR)
        pr_ecode("AdminIPV:                       ERROR: ", ret);
    else
        pr_output("AdminIPV:                       %d\n", adminipv);

    ret = tsn_psfp_gt_get_closed_inv_rx_enable(phdl, gate, &closed_inv_rx_ena);
    if (ret != E_TSN_LIB_NO_ERROR)
        pr_ecode("GateClosedDueToInvalidRxEnable: ERROR: ", ret);
    else
        pr_output("GateClosedDueToInvalidRxEnable: %s\n",
                  closed_inv_rx_ena ? "YES" : "NO");

    ret = tsn_psfp_gt_get_closed_inv_rx(phdl, gate, &closed_inv_rx);
    if (ret != E_TSN_LIB_NO_ERROR)
        pr_ecode("GateClosedDueToInvalidRx:       ERROR: ", ret);
    else
        pr_output("GateClosedDueToInvalidRx:       %s\n",
                  closed_inv_rx ? "YES" : "NO");

    ret = tsn_psfp_gt_get_config_pending(phdl, gate, &config_pend);
    if (ret != E_TSN_LIB_NO_ERROR)
        pr_ecode("ConfigPending                   ERROR: ", ret);
    else
        pr_output("ConfigPending:                  %s\n",
                  config_pend ? "YES" : "NO");

    ret = tsn_psfp_gt_get_config_change_error(phdl, gate, &ccerr);
    if (ret != E_TSN_LIB_NO_ERROR)
        pr_ecode("ConfigChangeError:              ERROR: ", ret);
    else
        pr_output("ConfigChangeError:              %d\n", ccerr);
    pr_output("\n");

    ret = tsn_psfp_gt_get_admin_cycle_time(phdl, gate, &admincycle);
    if (ret != E_TSN_LIB_NO_ERROR)
        pr_ecode("AdminCycleTime:                 ERROR: ", ret);
    else
        pr_output("AdminCycleTime:                 %u/%u sec\n",
                  admincycle.numerator, admincycle.denominator);

    ret = tsn_psfp_gt_get_admin_cycle_time_extension(phdl, gate,
                                                     &admincycleext);
    if (ret != E_TSN_LIB_NO_ERROR)
        pr_ecode("AdminCycleTimeExtension:        ERROR: ", ret);
    else
        pr_output("AdminCycleTimeExtension:        %u nsec\n", admincycleext);

    ret = tsn_psfp_gt_get_admin_control_list_length(phdl, gate, &list_len);
    if (ret != E_TSN_LIB_NO_ERROR)
        pr_ecode("AdminControlListLength:         ERROR: ", ret);
    else
        pr_output("AdminControlListLength:         %u entries\n", list_len);

    ret = tsn_psfp_gt_get_oper_cycle_time(phdl, gate, &opercycle);
    if (ret != E_TSN_LIB_NO_ERROR) {
        switch (TSN_GET_ERROR_INDEX(ret)) {
        case TSN_PSFP_RET_NOT_ACTIVE:
            pr_output("OperCycleTime:                  n/a\n");
            break;
        default:
            pr_ecode("OperCycleTime:                  ERROR: ", ret);
        }
    }
    else {
        pr_output("OperCycleTime:                  %u/%u sec\n",
                  opercycle.numerator, opercycle.denominator);
    }

    ret = tsn_psfp_gt_get_oper_cycle_time_extension(phdl, gate, &opercycleext);
    if (ret != E_TSN_LIB_NO_ERROR) {
        switch (TSN_GET_ERROR_INDEX(ret)) {
        case TSN_PSFP_RET_NOT_ACTIVE:
            pr_output("OperCycleTimeExtension:         n/a\n");
            break;
        default:
            pr_ecode("OperCycleTimeExtension:         ERROR: ", ret);
        }
    }
    else {
        pr_output("OperCycleTimeExtension:         %u nsec\n", opercycleext);
    }
    ret = tsn_psfp_gt_get_oper_control_list_length(phdl, gate, &list_len);
    if (ret != E_TSN_LIB_NO_ERROR)
        pr_ecode("OperControlListLength:          ERROR: ", ret);
    else
        pr_output("OperControlListLength:          %u entries\n\n", list_len);

    pr_output("\n");

    ret = tsn_psfp_gt_get_current_time(phdl, gate, &current);
    if (ret != E_TSN_LIB_NO_ERROR) {
        pr_ecode("CurrentTime (TAI):              ERROR: ", ret);
    }
    else {
        current_sec = (time_t)current.sec;
        strftime(buf, 100, "%F %T", gmtime(&current_sec));
        pr_output("CurrentTime (TAI):              %s.%09"PRIu32
                  " (%"PRIu64".%09"PRIu32")\n",
                  buf, current.nsec, current.sec, current.nsec);
    }

    ret = tsn_psfp_gt_get_admin_base_time(phdl, gate, &adminbase);
    if (ret != E_TSN_LIB_NO_ERROR) {
        pr_ecode("AdminBaseTime (TAI):            ERROR: ", ret);
    } else {
        adminbase_sec = (time_t)adminbase.sec;
        strftime(buf, 100, "%F %T", gmtime(&adminbase_sec));
        pr_output("AdminBaseTime (TAI):            %s.%09"PRIu32 "\n",
                  buf, adminbase.nsec);
    }

    ret = tsn_psfp_gt_get_oper_base_time(phdl, gate, &operbase);
    if (ret != E_TSN_LIB_NO_ERROR) {
        switch (TSN_GET_ERROR_INDEX(ret)) {
        case TSN_PSFP_RET_NOT_ACTIVE:
            pr_output("OperBaseTime (TAI):             n/a\n");
            break;
        default:
            pr_ecode("OperBaseTime (TAI):             ERROR: ", ret);
        }
    }
    else {
        operbase_sec = (time_t)operbase.sec;
        strftime(buf, 100, "%F %T", gmtime(&operbase_sec));
        pr_output("OperBaseTime (TAI):             %s.%09"PRIu32 "\n",
                  buf, operbase.nsec);
    }

    ret = tsn_psfp_gt_get_config_change_time(phdl, gate, &cctime);
    if (ret != E_TSN_LIB_NO_ERROR) {
        pr_ecode("ConfigChangeTime (TAI):         ERROR: ", ret);
        return ret;
    }
    else {
        cctime_sec = (time_t)cctime.sec;
        strftime(buf, 100, "%F %T", gmtime(&cctime_sec));
        pr_output("ConfigChangeTime (TAI):         %s.%09"PRIu32 " ",
                  buf, cctime.nsec);

        if (config_pend) {
            double diff = difftime(cctime.sec, current.sec);
            time_t difft = (time_t)diff;
            strftime(buf, 100, "%T", gmtime(&difft));

            if (difft > 86400)
                pr_output("        (activation in 1d+)\n");
            else
                pr_output("        (activation in %s)\n", buf);
        }
    }
    pr_output("\n\n");

    ret = tsn_psfp_gt_get_tick_granularity(phdl, gate, &granularity);
    if (ret != E_TSN_LIB_NO_ERROR)
        pr_ecode("TickGranularity:                ERROR: ", ret);
    else
        pr_output("TickGranularity:                %"PRIu32 " 1/10 nsec\n",
                  granularity);

    release_if(phdl);
    return ret;
}

static uint32_t do_psfp_gate_help(int argc, char **argv) {
    pr_output("Usage: %s wrcl GATE_ID BRDEV FILENAME\n", _context);
    pr_output("       %s rdacl GATE_ID BRDEV [FILENAME]\n", _context);
    pr_output("       %s rdocl GATE_ID BRDEV [FILENAME]\n", _context);
    pr_output("       %s configure GATE_ID BASETIME CYCLETIME CYCLETIME_EXT BRDEV\n",
              _context);
    pr_output("          where formats are as following:\n");
    pr_output("               BASETIME [PTP-time]   := [+|-]SECONDS.NANOSECONDS\n");
    pr_output("               CYCLETIME [s]         := NUMERATOR/DENOMINATOR\n");
    pr_output("               CYCLETIME_EXT [ns]    := NANOSECONDS\n");
    pr_output("       %s set GATE_ID STATE IPV BRDEV\n", _context);
    pr_output("       %s closedinvrx GATE_ID ENABLED CLOSED BRDEV\n", _context);
    pr_output("          where:\n");
    pr_output("               ENABLED := {on | off}\n");
    pr_output("               CLOSED  := {on | off}\n");
    pr_output("       %s show GATE_ID BRDEV\n", _context);
    pr_output("       %s disable GATE_ID BRDEV\n", _context);
    pr_output("       %s reset GATE_ID BRDEV\n", _context);
    (void)argc;
    (void)argv;
    exit(0);
}

static const struct cmd psfp_gate_cmds[] = {
    { "wrcl",        do_psfp_gate_wrcl },
    { "rdacl",       do_psfp_gate_rd_admin_gcl },
    { "rdocl",       do_psfp_gate_rd_oper_gcl },
    { "disable",     do_psfp_gate_disable },
    { "reset",       do_psfp_gate_reset },
    { "configure",   do_psfp_gate_configure },
    { "set",         do_psfp_gate_set },
    { "closedinvrx", do_psfp_gate_closed_inv_rx },
    { "show",        do_psfp_gate_show },
    { "help",        do_psfp_gate_help },
    NO_MORE_CMDS
};

static uint32_t do_psfp_gate(int argc, char **argv) {
    return do_cmd(argc, argv, psfp_gate_cmds);
}

static uint32_t do_psfp_hit_gate(int argc, char **argv) {
    struct port_hdl *brdev = NULL;
    unsigned int gate;
    uint32_t ret = 0u;

    assert_param_cnt(argc, 3);
    assert_param_parse(strcmp("set", argv[0]));
    assert_param_parse(sscanf(argv[1], "%u", &gate) != 1);
    brdev = get_if(argv[2]);

    ret = tsn_psfp_hit_set_gate(brdev, gate);
    if (ret)
        pr_error("Not able to set hit counter gate - %d (%s)\n",
                 errno, strerror(errno));

    release_if(brdev);
    return ret;
}

static uint32_t do_psfp_hit_th(int argc, char **argv) {
    struct port_hdl *brdev = NULL;
    enum th_type { th_low, th_high } th_type = th_low;
    unsigned int th_ns;
    uint32_t ret = 0u;

    assert_param_cnt(argc, 4);
    assert_param_parse(strcmp("set", argv[0]));
    if (strcmp("low", argv[1]) == 0)
        th_type = th_low;
    else if (strcmp("high", argv[1]) == 0)
        th_type = th_high;
    else
        assert_param_parse(true);
    assert_param_parse(sscanf(argv[2], "%u", &th_ns) != 1);
    brdev = get_if(argv[3]);

    if (th_type == th_low)
        ret = tsn_psfp_hit_set_low_th(brdev, th_ns);
    else
        ret = tsn_psfp_hit_set_high_th(brdev, th_ns);
    if (ret)
        pr_ecode("ERROR: Not able to set hit counter threshold - ", ret);

    release_if(brdev);
    return ret;
}

static uint32_t do_psfp_hit_show(int argc, char **argv) {
    struct port_hdl *brdev = NULL;
    unsigned int th_low_ns, th_high_ns, gate;
    uint32_t ret = 0u;

    assert_param_cnt(argc, 1);
    brdev = get_if(argv[0]);

    ret = tsn_psfp_hit_get_gate(brdev, &gate);
    if (ret)
        pr_ecode("Selected gate:  ERROR: ", ret);
    else
        pr_output("Selected gate:  %u\n", gate);

    ret = tsn_psfp_hit_get_low_th(brdev, &th_low_ns);
    if (ret)
        pr_ecode("Low threshold:  ERROR: ", ret);
    else
        pr_output("Low threshold:  %u ns\n", th_low_ns);

    ret = tsn_psfp_hit_get_high_th(brdev, &th_high_ns);
    if (ret)
        pr_ecode("High threshold: ERROR: ", ret);
    else
        pr_output("High threshold: %u ns\n", th_high_ns);

    release_if(brdev);
    return ret;
}

static uint32_t do_psfp_hit_cntshow(int argc, char **argv) {
    struct port_hdl *ptdev = NULL;
    uint32_t str_hdl;
    unsigned int cnt_low, cnt_high;
    uint32_t ret = 0u;

    assert_param_cnt(argc, 2);
    assert_param_parse(sscanf(argv[0], "%"SCNu32, &str_hdl) != 1);
    ptdev = get_if(argv[1]);

    pr_output("Stream %"PRIu32 ":\n", str_hdl);

    ret = tsn_psfp_hit_get_low_cnt(ptdev, str_hdl, &cnt_low);
    if (ret)
        pr_ecode("    Low threshold violation counter:  ERROR: ", ret);
    else
        pr_output("    Low threshold violation counter:  %"PRIu32 "\n",
                  cnt_low);

    ret = tsn_psfp_hit_get_high_cnt(ptdev, str_hdl, &cnt_high);
    if (ret)
        pr_ecode("    High threshold violation counter: ERROR: ", ret);
    else
        pr_output("    High threshold violation counter: %"PRIu32 "\n",
                  cnt_high);

    release_if(ptdev);
    return ret;
}

static uint32_t do_psfp_hit_help(int argc, char **argv) {
    pr_output("Usage: %s th set {low | high} THRESHOLD_NS BRDEV\n", _context);
    pr_output("       %s gate set GATE_ID BRDEV\n", _context);
    pr_output("       %s show BRDEV\n", _context);
    pr_output("       %s cntshow STREAM_HDL DEV\n", _context);
    (void)argc;
    (void)argv;
    exit(0);
}

static const struct cmd psfp_hit_cmds[] = {
    { "th", do_psfp_hit_th },
    { "gate", do_psfp_hit_gate },
    { "show", do_psfp_hit_show },
    { "cntshow", do_psfp_hit_cntshow },
    { "help", do_psfp_hit_help },
    NO_MORE_CMDS
};

static uint32_t do_psfp_hit(int argc, char **argv) {
    return do_cmd(argc, argv, psfp_hit_cmds);
}

static uint32_t do_psfp_show(int argc, char **argv) {
    struct port_hdl *brdev = NULL;
    uint32_t max_filters;
    uint32_t max_gates;
    uint32_t max_flow_meters;
    uint32_t max_list;
    struct tsn_st_rational_number max_cyc;
    uint32_t max_int_ns;
    uint32_t ret = 0u;

    assert_param_cnt(argc, 1);
    brdev = get_if(argv[0]);

    pr_output("PSFPStreamParameterTable:\n");

    ret = tsn_psfp_get_max_filters(brdev, &max_filters);
    if (ret)
        pr_ecode("    MaxStreamFilterInstances:    ERROR: ", ret);
    else
        pr_output("    MaxStreamFilterInstances:    %"PRIu32 "\n", max_filters);

    ret = tsn_psfp_get_max_gates(brdev, &max_gates);
    if (ret)
        pr_ecode("    MaxStreamGateInstances:      ERROR: ", ret);
    else
        pr_output("    MaxStreamGateInstances:      %"PRIu32 "\n", max_gates);

    ret = tsn_psfp_get_max_flow_meters(brdev, &max_flow_meters);
    if (ret)
        pr_ecode("    MaxFlowMeterInstances:       ERROR: ", ret);
    else
        pr_output("    MaxFlowMeterInstances:       %"PRIu32 "\n",
                  max_flow_meters);

    ret = tsn_psfp_get_supported_list_max(brdev, &max_list);
    if (ret)
        pr_ecode("    SupportedListMax:            ERROR: ", ret);
    else
        pr_output("    SupportedListMax:            %"PRIu32 "\n", max_list);

    ret = tsn_psfp_get_supported_cycle_max(brdev, &max_cyc);
    if (ret)
        pr_ecode("    SupportedCycleMax:           ERROR: ", ret);
    else
        pr_output("    SupportedCycleMax:           %"PRIu32 "/%"PRIu32 " s\n",
                  max_cyc.numerator, max_cyc.denominator);

    ret = tsn_psfp_get_supported_interval_max(brdev, &max_int_ns);
    if (ret)
        pr_ecode("    SupportedIntervalMax:        ERROR: ", ret);
    else
        pr_output("    SupportedIntervalMax:        %"PRIu32 " ns\n",
                  max_int_ns);

    release_if(brdev);
    return ret;
}

static uint32_t do_psfp_help(int argc, char **argv) {
    pr_output("Usage: %s show BRDEV\n", _context);
    pr_output("       %s filter { COMMAND | help }\n", _context);
    pr_output("       %s gate { COMMAND | help }\n", _context);
    pr_output("       %s hit { COMMAND | help }\n", _context);
    (void)argc;
    (void)argv;
    exit(0);
}

static const struct cmd psfp_cmds[] = {
    { "filter", do_psfp_filter },
    { "hit", do_psfp_hit },
    { "gate", do_psfp_gate },
    { "show", do_psfp_show },
    { "help",   do_psfp_help },
    NO_MORE_CMDS
};

uint32_t do_psfp(int argc, char **argv) {
    return do_cmd(argc, argv, psfp_cmds);
}
