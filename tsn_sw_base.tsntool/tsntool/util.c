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

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include "tsntool.h"

int match(const char* opt, const char* pattern) {
    size_t len = strlen(pattern);
    return strncmp(opt, pattern, len);
}

uint32_t do_cmd(int argc, char **argv, const struct cmd *cmds) {
    if (argc > 0) {
        while (cmds->cmd != NULL) {
            if (match(argv[0], cmds->cmd) == 0) {
                if (strcmp(argv[0], "help")) {
                    /* help uses the _context, so don't collect! */
                    strlcat(_context, " ", sizeof(_context));
                    strlcat(_context, argv[0], sizeof(_context));
                }
                return cmds->func(argc-1, &argv[1]);
            }
            cmds++;
        }
        pr_error("Object/Command '%s' is unknown, try '%s help'\n", argv[0], _context);
        return EPERM;
    }
    pr_error("No Object/Command found, try '%s help'\n", _context);
    return EPERM;
}

static inline void _do_assert(const char *msg) {
    char *cmd = strrchr(_context, ' ');
    *cmd = '\0';
    cmd++;
    pr_error("%s for command '%s', try '%s help'\n",
             msg, cmd, _context
            );
    exit(-1);
}

void assert_param_cnt(int argc, int reqargc) {
    if (argc != reqargc)
        _do_assert("Wrong number of arguments");
}

void assert_param_min_max(int argc, int minargc, int maxargc) {
    if ((argc < minargc) || (argc > maxargc))
        _do_assert("Wrong number of arguments");
}

void assert_param_parse(bool assert) {
    if (assert)
        _do_assert("Wrong parameter syntax");
}

struct port_hdl *get_if(const char *ifname) {
#ifdef GO_LIVE
    struct port_hdl *phdl = tsn_port_open_by_name(ifname);
    if (!phdl) {
        pr_error("Cannot open device '%s'\n", ifname);
        exit(-1);
    }
    return phdl;
#else
    pr_output("%s: '%s' libtsn not linked yet!!!\n", __func__, ifname);
    return NULL;
#endif
}

void release_if(struct port_hdl *phdl) {
#ifdef GO_LIVE
    tsn_port_close(phdl);
#else
    (void)phdl;
    pr_output("%s: libtsn not linked yet!!!\n", __func__);
#endif
}

void pr_msg(FILE *fp, ...)
{
    va_list args;
    va_start(args, fp);
    vfprintf(fp, va_arg(args, char *), args);
    va_end(args);
}

// https://stackoverflow.com/questions/122616/how-do-i-trim-leading-trailing-whitespace-in-a-standard-way
char *trim(char *str)
{
    char *end;
    while (isspace((unsigned char)*str)) str++; // leading spaces
    if (*str == 0)  // All spaces?
        return str;

    end = str + strlen(str) - 1;     // trailing space
    while (end > str && isspace((unsigned char)*end)) end--;

    // null terminator
    *(end+1) = 0;
    return str;
}

uint32_t str_to_ptp_time(const char *buf, struct tsn_st_ptp_time *ptp_time)
{
    char str[64] = "";
    char str_nsec[] = "000000000";
    char *token;
    struct tsn_st_ptp_time t = {0, 0};

    if (strlen(buf) >= sizeof(str))
        return EINVAL;
    strcpy(str, buf);
    token = strtok(str, ".");
    if (!token)
        return EINVAL;

    errno = 0;
    t.sec = (uint64_t)strtoull(token, NULL, 10);
    if (errno)
        return EINVAL;

    token = strtok(NULL, "\n\r\t ");
    if (token) {
        size_t token_len = strlen(token);
        if (token_len > 9)
            return EINVAL;
        /* Add trailing zeros to nsec part */
        memcpy(str_nsec, token, token_len);
        t.nsec = (uint32_t)strtoul(str_nsec, NULL, 10);
        if (errno)
            return EINVAL;
    }
    *ptp_time = t;
    return 0;
}
