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

#include <stdio.h>
#include <stdlib.h>

#include "tsntool.h"

static char *_basename;
char _context[256];

static uint32_t do_help(int argc, char **argv) {
    pr_output(
            "Usage: %s [ OPTIONS ] OBJECT { COMMAND | help }\n", _basename
            );
    pr_output(
            "where    OBJECT := { brport | edge | fqtss | frer | mstp | preempt | psfp | qbridge | st | sid | help }\n"
            "         OPTIONS := { -V[ersion] | -h[elp] }\n"
            );
    (void)argc;
    (void)argv;
    exit(0);
}

#define STRINGIZE(x) #x
#define STRINGIZE_VALUE_OF(x) STRINGIZE(x)

static uint32_t do_version(int argc, char **argv) {
    pr_output("%s utility, %s\n", _basename, VER_FULL);
    (void)argc;
    (void)argv;
    exit(0);
}

static const struct cmd main_cmds[] = {
    { "st",      do_st },
    { "preempt", do_preempt },
    { "fqtss",   do_fqtss },
    { "frer",    do_frer },
    { "edge",    do_edge },
    { "brport",  do_brport },
    { "qbridge", do_qbridge },
    { "mstp",    do_mstp },
    { "sid",     do_sid },
    { "frer",    do_frer },
    { "psfp",    do_psfp },
    { "help",    do_help },
    { "-h",      do_help },
    { "--help",  do_help },
    { "-V",      do_version },
    NO_MORE_CMDS
};

int main(int argc, char** argv) {
    _basename = strrchr(argv[0], '/');
    if (_basename == NULL)
        _basename = argv[0];
    else
        _basename++;

    strlcat(_context, _basename, sizeof(_context));
    argc--; argv++;
    return do_cmd(argc, &argv[0], main_cmds);
}
