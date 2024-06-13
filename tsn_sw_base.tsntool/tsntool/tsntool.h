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

#ifndef TSN_TOOL_H_
#define TSN_TOOL_H_

#include <inttypes.h>
// cppcheck-suppress  preprocessorErrorDirective
#include <bsd/string.h>
#include "tsn.h"

#if 1
    #define GO_LIVE
#else
    #undef GO_LIVE
#endif

#define NO_MORE_CMDS    { NULL, NULL }

extern char _context[256];

struct cmd {
    const char *cmd;
    uint32_t (*func)(int argc, char **argv);
};

int match(const char* opt, const char* pattern);
uint32_t do_cmd(int argc, char **argv, const struct cmd *cmds);

void assert_param_cnt(int argc, int reqargs);
void assert_param_min_max(int argc, int minargc, int maxargc);
void assert_param_parse(bool assert);

uint32_t do_st(int argc, char **argv);
uint32_t do_preempt(int argc, char **argv);
uint32_t do_fqtss(int argc, char **argv);
uint32_t do_frer(int argc, char **argv);
uint32_t do_edge(int argc, char **argv);
uint32_t do_brport(int argc, char **argv);
uint32_t do_qbridge(int argc, char **argv);
uint32_t do_mstp(int argc, char **argv);
uint32_t do_sid(int argc, char **argv);
uint32_t do_psfp(int argc, char **argv);

struct port_hdl *get_if(const char *ifname);
void release_if(struct port_hdl *phdl);
void pr_msg(FILE *fp, ...);

char *trim(char *str);
uint32_t str_to_ptp_time(const char *buf, struct tsn_st_ptp_time *ptp_time);

#define pr_output(...)              pr_msg(stdout, __VA_ARGS__)
#define pr_error(...)               pr_msg(stderr, __VA_ARGS__)

#define pr_ecode(msg, ret)                                                     \
    do {                                                                       \
        pr_error(msg "tsnlib reported %d (%s)\n",                              \
                 TSN_LIB_ERROR_CODE(ret),                                      \
                 read_lib_error_msg(ret));                                     \
        pr_error(msg "driver reported %d (%s)\n",                              \
                 TSN_ERRNO_ERROR_CODE(ret),                                    \
                 tsn_lib_get_known_ernno_msg(ret));                            \
    } while (0)

#endif /*TSN_TOOL_H_*/
