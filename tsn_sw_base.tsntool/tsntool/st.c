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
#include <time.h>
#include <string.h>
#include "tsn_st.h"
#include "tsntool.h"
#include "tsn_bridge.h"

/* record format: see example configuration file. */

#define MAX_REC (100+1)
#define GCF_OP_SGS "sgs"
#define GCF_OP_SHM "shm"
#define GCF_OP_SRM "srm"
#define GCF_OP_UNKNOWN "<unknown>"
#define TSN_ST_MAX_STR_BUF (64)

enum gcl_select {
    ADMIN_GCL,
    OPER_GCL,
};

static uint32_t parse_record(char *s, struct st_gate_control_entry *gce,
                             unsigned int lineno) {
    char *tok;
    char *endptr;
    unsigned long gsv;

    // parse <operation>
    tok = strtok(s, " ");
    if (!tok) {
        pr_error("WARNING: Missing <operation> in line %u\n", lineno);
        return EINVAL;
    }
    if (strcmp(tok, GCF_OP_SGS) == 0)
        gce->op = TSN_ST_OP_SET_GATE_STATES;
    else if (strcmp(tok, GCF_OP_SHM) == 0)
        gce->op = TSN_ST_OP_SET_AND_HOLD_MAC;
    else if (strcmp(tok, GCF_OP_SRM) == 0)
        gce->op = TSN_ST_OP_SET_AND_RELEASE_MAC;
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
    // allow decimal only to avoid confusion
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

    // check on excess record elements
    tok = strtok(NULL, " ");
    if (tok) {
        pr_error("ERROR: Excess record elements in line %u: '%s'\n",
                 lineno, tok);
        return EINVAL;
    }

    return 0;
}

static struct st_gate_control_entry *read_gcf(const char *fn, uint32_t *gcl_len,
                                              uint32_t max_len) {
    struct st_gate_control_entry *gcl;
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
    nlines++; // just in case the last line doesn't end with \n
    if (nlines > max_len) {
        // If there are more entries than supported just trim it to the max
        nlines = max_len;
        pr_error("ERROR: Tried to read more entries than supported by implementation, defaulting to %u entries\n",
                 nlines);
    }
    fseek(fp, 0, SEEK_SET);
    if ((gcl = malloc(sizeof(*gcl) * nlines)) == NULL) {
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

static uint32_t write_gcf(struct st_gate_control_entry *gcl, uint32_t gcl_len,
                          const char *fn) {
    FILE *fp = fopen(fn, "w");

    (void)gcl;

    if (fp == NULL) {
        int err = errno;
        pr_error("ERROR: Cannot write file '%s' - %d (%s)\n",
                 fn, errno, strerror(errno));
        return err;
    }

    for (uint32_t i = 0; i < gcl_len; i++, gcl++) {
        const char *gcf_op;
        switch (gcl->op) {
        case TSN_ST_OP_SET_GATE_STATES:
            gcf_op = GCF_OP_SGS;
            break;
        case TSN_ST_OP_SET_AND_HOLD_MAC:
            gcf_op = GCF_OP_SHM;
            break;
        case TSN_ST_OP_SET_AND_RELEASE_MAC:
            gcf_op = GCF_OP_SRM;
            break;
        case TSN_ST_OP_EXCESS: /* Skip excess elements */
            continue;
            break;
        default:
            gcf_op = GCF_OP_UNKNOWN;
            pr_error("ERROR: Unknown operation in record #%u\n", i+1);
            break;
        }
        pr_msg(fp, "%s  %u  0x%02X\n", gcf_op, gcl->interval, gcl->gsv);
    }
    fclose(fp);
    return 0;
}

static uint32_t do_st_wrcl(int argc, char **argv) {
    struct port_hdl *phdl;
    struct st_gate_control_entry* gcl;
    uint32_t gcl_len, supp_list_max;
    int ret = 0;

    assert_param_cnt(argc, 2);

    phdl = get_if(argv[0]);
    ret = tsn_st_get_supported_list_max(phdl, &supp_list_max);
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
    gcl = read_gcf(argv[1], &gcl_len, supp_list_max);
    if (gcl == NULL) {
        release_if(phdl);
        return EPERM;
    }

#ifdef GO_LIVE
    ret = tsn_st_set_admin_control_list_length(phdl, gcl_len);
    if (ret != E_TSN_LIB_NO_ERROR) {
        pr_ecode("ERROR: Cannot set admin control list length - ", ret);
    }
    else {
        ret = tsn_st_set_admin_control_list(phdl, gcl_len, gcl);
        if (ret != E_TSN_LIB_NO_ERROR)
            pr_ecode("ERROR: Cannot set admin control list - ", ret);
    }
#endif
    free(gcl);
    release_if(phdl);

    return ret;
}

static uint32_t _do_st_rd_gcl(int argc, char **argv, enum gcl_select sel) {
    uint32_t supp_list_max, gcl_len = 1;
    struct st_gate_control_entry* gcl;
    int ret = 0;
    struct port_hdl *phdl;

    assert_param_min_max(argc, 1, 2);

    phdl = get_if(argv[0]);
    (void)phdl;

#ifdef GO_LIVE
    if (sel == ADMIN_GCL) {
        ret = tsn_st_get_admin_control_list_length(phdl, &gcl_len);
        if (ret != E_TSN_LIB_NO_ERROR) {
            pr_ecode("ERROR: Cannot get admin control list length - ", ret);
            release_if(phdl);
            return ret;
        }
    }
    else {
        ret = tsn_st_get_oper_control_list_length(phdl, &gcl_len);
        if (ret != E_TSN_LIB_NO_ERROR) {
            pr_ecode("ERROR: Cannot get oper. control list length - ", ret);
            release_if(phdl);
            return ret;
        }
    }
#else
    (void)sel;
#endif

    if (!gcl_len) {
        pr_error("Control list length is set to 0 - no entries read.\n");
        release_if(phdl);
        return ret;
    }
    ret = tsn_st_get_supported_list_max(phdl, &supp_list_max);
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

#ifdef GO_LIVE
    if (sel == ADMIN_GCL) {
        ret = tsn_st_get_admin_control_list(phdl, gcl_len, gcl);
        if (ret != E_TSN_LIB_NO_ERROR)
            pr_ecode("ERROR: Cannot get admin control list - ", ret);
    }
    else {
        ret = tsn_st_get_oper_control_list(phdl, gcl_len, gcl);
        if (ret != E_TSN_LIB_NO_ERROR)
            pr_ecode("ERROR: Cannot get oper. control list - ", ret);
    }
#endif
    release_if(phdl);

    if (!ret) {
        if (argc > 1)
            ret = write_gcf(gcl, gcl_len, argv[1]);
        else
            ret = write_gcf(gcl, gcl_len, "/dev/stdout");
    }
    free(gcl);

    return ret;
}

static uint32_t do_st_rd_admin_gcl(int argc, char **argv) {
    return _do_st_rd_gcl(argc, argv, ADMIN_GCL);
}

static uint32_t do_st_rd_oper_gcl(int argc, char **argv) {
    return _do_st_rd_gcl(argc, argv, OPER_GCL);
}

static uint32_t do_st_disable(int argc, char **argv) {
    struct port_hdl *phdl = NULL;
    uint32_t ret = 0u;

    assert_param_cnt(argc, 1);

    phdl = get_if(argv[0]);
    ret = tsn_st_set_gate_enabled(phdl, false);
    if (ret != E_TSN_LIB_NO_ERROR)
        pr_ecode("ERROR: Cannot disable gates - ", ret);
    release_if(phdl);
    return ret;
}

static uint32_t do_st_reset(int argc, char **argv) {
    struct port_hdl *phdl = NULL;
    uint32_t ret = 0u;

    assert_param_cnt(argc, 1);

    phdl = get_if(argv[0]);
    /*set all gates to open in admin control object before gate disable */
    ret = tsn_st_set_admin_gate_states(phdl, SCHED_DEF_GATE_STATES);
    if (ret != E_TSN_LIB_NO_ERROR) {
        pr_ecode("ERROR: Cannot set admin gate states - ", ret);
    } else {
        ret = tsn_st_set_gate_enabled(phdl, false);
        if (ret != E_TSN_LIB_NO_ERROR)
            pr_ecode("ERROR: Cannot disable gates - ", ret);
    }
    release_if(phdl);
    return ret;
}

static uint32_t do_st_configure(int argc, char **argv) {
    struct tsn_st_ptp_time basetime;
    struct tsn_st_rational_number cycletime;
    int long long numerator, denominator;
    int long long ctextension;
    struct port_hdl *phdl = NULL;
    int items = 0;
    uint32_t ret = 0u;
    uint32_t gacl_len = 0;

    assert_param_cnt(argc, 4);

    phdl = get_if(argv[3]);
    if (*argv[0] == '+' || *argv[0] == '-') {
        // Resolve absolute base time from relative offset given
        struct tsn_st_ptp_time bt;
        struct tsn_st_ptp_time cur;

        if (!str_to_ptp_time(argv[0], &bt))
            items += 2;

        tsn_st_get_current_time(phdl, &cur);

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
        if (!str_to_ptp_time(argv[0], &basetime))
            items += 2;
    }

    if (sscanf(argv[1], "%lld/%lld", &numerator, &denominator) == 2) {
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
    if (sscanf(argv[2], "%lld", &ctextension) == 1) {
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
    /* check if a gate admin control list with list length > 0 exists */
    ret = tsn_st_get_admin_control_list_length(phdl, &gacl_len);
    if ((ret != E_TSN_LIB_NO_ERROR) || (gacl_len == 0)) {
        pr_error("No gate admin control list available.\n");
        ret = EINVAL;
    }
    if ((items != 5) || ret != 0) {
        release_if(phdl);
        return ret;
    }

#ifdef GO_LIVE
    ret = tsn_st_set_admin_base_time(phdl, &basetime);
    if (ret != E_TSN_LIB_NO_ERROR)
        pr_ecode("ERROR: Cannot set admin base time - ", ret);
    if (ret == E_TSN_LIB_NO_ERROR) {
        ret = tsn_st_set_admin_cycle_time(phdl, &cycletime);
        if (ret != E_TSN_LIB_NO_ERROR)
            pr_ecode("ERROR: Cannot set admin cycle time - ", ret);
    }
    if (ret == E_TSN_LIB_NO_ERROR) {
        ret = tsn_st_set_admin_cycle_time_extension(phdl,
                                                    (uint32_t) ctextension);
        if (ret != E_TSN_LIB_NO_ERROR)
            pr_ecode("ERROR: Cannot set admin cycle time extension time - ",
                     ret);
    }
    if (ret == E_TSN_LIB_NO_ERROR) {
        ret = tsn_st_set_gate_enabled(phdl, true);
        if (ret != E_TSN_LIB_NO_ERROR)
            pr_ecode("ERROR: Cannot enable gates - ", ret);
    }
    if (ret == E_TSN_LIB_NO_ERROR) {
        ret = tsn_st_set_config_change(phdl, true);
        if (ret != E_TSN_LIB_NO_ERROR)
            pr_ecode("ERROR: Cannot trigger config change - ", ret);
    }
#else
    pr_output("basetime    = %ju.%u\n", basetime.sec, basetime.nsec);
    pr_output("cycletime   = %u/%u\n",  cycletime.numerator, cycletime.denominator);
    pr_output("ctextension = %u\n", ctextension);
    pr_output(">> %s - scanned %d items (of 5), phdl = %p\n", __func__, items, phdl);
#endif
    release_if(phdl);
    return ret;
}

static uint32_t do_st_show(int argc, char **argv) {
    struct port_hdl *phdl = NULL;
    bool gate_en, config_pend;
    time_t adminbase_sec, operbase_sec, current_sec, cctime_sec;
    struct tsn_st_ptp_time adminbase, operbase, current, cctime;
    struct tsn_st_rational_number admincycle, opercycle, sup_cycle_max;
    uint32_t admincycleext, opercycleext, num_tcs, max_int_ns, list_max_len,
             tick_granularity, list_len;
    int ccerr;
    char buf[100];
    uint32_t ret = 0u;
    assert_param_cnt(argc, 1);
    phdl = get_if(argv[0]);

    ret = tsn_st_get_gate_enabled(phdl, &gate_en);
    if (ret != E_TSN_LIB_NO_ERROR)
        pr_ecode("GatesEnabled:            ERROR: ", ret);
    else
        pr_output("GatesEnabled:            %s\n", gate_en ? "YES" : "NO");

    ret = tsn_st_get_config_pending(phdl, &config_pend);
    if (ret != E_TSN_LIB_NO_ERROR)
        pr_ecode("ConfigPending            ERROR: ", ret);
    else
        pr_output("ConfigPending:           %s\n", config_pend ? "YES" : "NO");

    ret = tsn_st_get_config_change_error(phdl, &ccerr);
    if (ret != E_TSN_LIB_NO_ERROR)
        pr_ecode("ConfigChangeError:       ERROR: ", ret);
    else
        pr_output("ConfigChangeError:       %d\n", ccerr);
    pr_output("\n");

    ret = tsn_st_get_admin_cycle_time(phdl, &admincycle);
    if (ret != E_TSN_LIB_NO_ERROR)
        pr_ecode("AdminCycleTime:          ERROR: ", ret);
    else
        pr_output("AdminCycleTime:          %u/%u sec\n",
                  admincycle.numerator, admincycle.denominator);

    ret = tsn_st_get_admin_cycle_time_extension(phdl, &admincycleext);
    if (ret != E_TSN_LIB_NO_ERROR)
        pr_ecode("AdminCycleTimeExtension: ERROR: ", ret);
    else
        pr_output("AdminCycleTimeExtension: %u nsec\n", admincycleext);

    ret = tsn_st_get_admin_control_list_length(phdl, &list_len);
    if (ret != E_TSN_LIB_NO_ERROR)
        pr_ecode("AdminControlListLength:  ERROR: ", ret);
    else
        pr_output("AdminControlListLength:  %u entries\n", list_len);

    ret = tsn_st_get_oper_cycle_time(phdl, &opercycle);
    if (ret != E_TSN_LIB_NO_ERROR) {
        switch (TSN_GET_ERROR_INDEX(ret)) {
        case TSN_ST_RET_NOT_ACTIVE:
            pr_output("OperCycleTime:           n/a\n");
            break;
        default:
            pr_ecode("OperCycleTime:           ERROR: ", ret);
        }
    }
    else {
        pr_output("OperCycleTime:           %u/%u sec\n",
                  opercycle.numerator, opercycle.denominator);
    }

    ret = tsn_st_get_oper_cycle_time_extension(phdl, &opercycleext);
    if (ret != E_TSN_LIB_NO_ERROR) {
        switch (TSN_GET_ERROR_INDEX(ret)) {
        case TSN_ST_RET_NOT_ACTIVE:
            pr_output("OperCycleTimeExtension:  n/a\n");
            break;
        default:
            pr_ecode("OperCycleTimeExtension:  ERROR: ", ret);
        }
    }
    else {
        pr_output("OperCycleTimeExtension:  %u nsec\n", opercycleext);
    }

    ret = tsn_st_get_oper_control_list_length(phdl, &list_len);
    if (ret != E_TSN_LIB_NO_ERROR)
        pr_ecode("OperControlListLength:   ERROR: ", ret);
    else
        pr_output("OperControlListLength:   %u entries\n\n", list_len);

    ret = tsn_st_get_current_time(phdl, &current);
    if (ret != E_TSN_LIB_NO_ERROR) {
        pr_ecode("CurrentTime (TAI):       ERROR: ", ret);
    }
    else {
        current_sec = (time_t)current.sec;
        strftime(buf, 100, "%F %T", gmtime(&current_sec));
        pr_output("CurrentTime (TAI):       %s.%09"PRIu32
                  " (%"PRIu64".%09"PRIu32")\n",
                  buf, current.nsec, current.sec, current.nsec);
    }

    ret = tsn_st_get_admin_base_time(phdl, &adminbase);
    if (ret != E_TSN_LIB_NO_ERROR) {
        pr_ecode("AdminBaseTime (TAI):     ERROR: ", ret);
    } else {
        adminbase_sec = (time_t)adminbase.sec;
        strftime(buf, 100, "%F %T", gmtime(&adminbase_sec));
        pr_output("AdminBaseTime (TAI):     %s.%09"PRIu32 "\n",
                  buf, adminbase.nsec);
    }

    ret = tsn_st_get_oper_base_time(phdl, &operbase);
    if (ret != E_TSN_LIB_NO_ERROR) {
        switch (TSN_GET_ERROR_INDEX(ret)) {
        case TSN_ST_RET_NOT_ACTIVE:
            pr_output("OperBaseTime (TAI):      n/a\n");
            break;
        default:
            pr_ecode("OperBaseTime (TAI):      ERROR: ", ret);
        }
    }
    else {
        operbase_sec = (time_t)operbase.sec;
        strftime(buf, 100, "%F %T", gmtime(&operbase_sec));
        pr_output("OperBaseTime (TAI):      %s.%09"PRIu32 "\n",
                  buf, operbase.nsec);
    }

    ret = tsn_st_get_config_change_time(phdl, &cctime);
    if (ret != E_TSN_LIB_NO_ERROR) {
        pr_ecode("ConfigChangeTime (TAI):  ERROR: ", ret);
    }
    else {
        cctime_sec = (time_t)cctime.sec;
        strftime(buf, 100, "%F %T", gmtime(&cctime_sec));
        pr_output("ConfigChangeTime (TAI):  %s.%09"PRIu32 " ",
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
        else {
            pr_output("\n");
        }
    }

    pr_output("\n");
    ret = tsn_st_get_supported_cycle_max(phdl, &sup_cycle_max);
    if (ret != E_TSN_LIB_NO_ERROR) {
        pr_ecode("SupportedCycleMax:       ERROR: ", ret);
    }
    else {
        pr_output("SupportedCycleMax:       %u/%u sec\n",
                  sup_cycle_max.numerator, sup_cycle_max.denominator);
    }

    ret = tsn_st_get_supported_interval_max(phdl, &max_int_ns);
    if (ret != E_TSN_LIB_NO_ERROR) {
        pr_ecode("SupportedIntervalMax:    ERROR: ", ret);
    }
    else {
        pr_output("SupportedIntervalMax:    %u nsec\n", max_int_ns);
    }

    ret = tsn_st_get_supported_list_max(phdl, &list_max_len);
    if (ret != E_TSN_LIB_NO_ERROR) {
        pr_ecode("SupportedListMax:        ERROR: ", ret);
    }
    else {
        pr_output("SupportedListMax:        %u entries\n", list_max_len);
    }

    ret = tsn_st_get_tick_granularity(phdl, &tick_granularity);
    if (ret != E_TSN_LIB_NO_ERROR) {
        pr_ecode("TickGranularity:         ERROR: ", ret);
    }
    else {
        pr_output("TickGranularity:         %u 1/10 nsec\n", tick_granularity);
    }
    pr_output("\n");

    ret = tsn_bridge_get_num_traffic_classes(phdl, &num_tcs);
    if (ret != E_TSN_LIB_NO_ERROR) {
        pr_ecode("portNumTrafficClasses:   ERROR: ", ret);
    }
    else {
        pr_output("portNumTrafficClasses:   %u\n\n", num_tcs);
        pr_output("queueMaxSDUTable:\n");
        pr_output("TrafficClass   queueMaxSDU   TransmissionOverruns\n");
        for (unsigned int tc = 0; tc < num_tcs; tc++) {
            uint32_t max_sdu;
            uint64_t overrun;

            pr_output("%2u", tc);

            ret = tsn_st_get_max_sdu(phdl, tc, &max_sdu);
            if (ret != E_TSN_LIB_NO_ERROR)
                pr_ecode("             ERROR: ", ret);
            else
                pr_output("                   %4u", max_sdu);

            ret = tsn_st_get_overrun(phdl, tc, &overrun);
            if (ret != E_TSN_LIB_NO_ERROR)
                pr_ecode("   ERROR: ", ret);
            else
                pr_output("   %" SCNu64 , overrun);

            pr_output("\n");
        }
    }

    release_if(phdl);
    return ret;
}

static uint32_t do_st_setsdu(int argc, char **argv) {
    int long long maxsdu;
    int long long tc;
    uint32_t num_tcs;
    struct port_hdl *phdl = NULL;
    uint32_t ret = 0u;

    assert_param_cnt(argc, 3);

    assert_param_parse(sscanf(argv[0], "%lld", &tc) != 1);
    if (tc != (uint8_t) tc) {
        pr_error("Traffic class %lld out of range\n", tc);
        return EINVAL;
    }
    assert_param_parse(sscanf(argv[1], "%lld", &maxsdu) != 1);
    if (maxsdu != (uint32_t) maxsdu) {
        pr_error("MAX_SDU %lld out of range\n", maxsdu);
        return EINVAL;
    }

    phdl = get_if(argv[2]);

    ret = tsn_bridge_get_num_traffic_classes(phdl, &num_tcs);
    if (ret != E_TSN_LIB_NO_ERROR) {
        pr_ecode("ERROR: Cannot set max. SDU - Cannot read the number of traffic classes: ",
                 ret);
        release_if(phdl);
        return ret;
    }

    if (tc > (num_tcs - 1)) {
        pr_error("ERROR: Cannot set max. SDU - Traffic class out of range [0..%" SCNu32 "]\n",
                 num_tcs);
        release_if(phdl);
        return EINVAL;
    }

    ret = tsn_st_set_max_sdu(phdl, (uint8_t) tc, (uint32_t) maxsdu);
    if (ret != E_TSN_LIB_NO_ERROR)
        pr_ecode("ERROR: Cannot set max. SDU - ", ret);

    release_if(phdl);
    return ret;
}

static uint32_t do_st_help(int argc, char **argv) {
    pr_output("Usage: %s wrcl DEV FILENAME\n", _context);
    pr_output("       %s rdacl DEV [FILENAME]\n", _context);
    pr_output("       %s rdocl DEV [FILENAME]\n", _context);
    pr_output("       %s configure BASETIME CYCLETIME CYCLTIME_EXT DEV\n", _context);
    pr_output("          where formats are as following:\n");
    pr_output("               BASETIME [PTP-time] := SECONDS.NANOSECONDS\n");
    pr_output("               CYCLETIME [s] := NUMERATOR/DENOMINATOR\n");
    pr_output("               CYCLETIME_EXT [ns]:= NANOSECONDS\n");
    pr_output("       %s show DEV \n", _context);
    pr_output("       %s disable DEV \n", _context);
    pr_output("       %s reset DEV \n", _context);
    pr_output("       %s setsdu TRAFFIC_CLASS MAX_SDU DEV\n", _context);
    (void)argc;
    (void)argv;
    exit(0);
}

static const struct cmd st_cmds[] = {
    { "wrcl",      do_st_wrcl },
    { "rdacl",     do_st_rd_admin_gcl },
    { "rdocl",     do_st_rd_oper_gcl },
    { "configure", do_st_configure },
    { "disable",   do_st_disable },
    { "show",      do_st_show },
    { "setsdu",    do_st_setsdu },
    { "help",      do_st_help },
    { "reset",     do_st_reset },
    NO_MORE_CMDS
};

uint32_t do_st(int argc, char **argv) {
    return do_cmd(argc, argv, st_cmds);
}
