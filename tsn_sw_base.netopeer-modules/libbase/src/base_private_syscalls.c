/*
 * TTTech libbase
 * Copyright(c) 2019 TTTech Industrial Automation AG.
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

#include "base_private_defines.h"
#include "base_private_parsers.h"
#include "base_private_syscalls.h"
#include "base_prints.h"
#include "base_defines.h"

int vcmd_execute(const char *command, va_list args)
{
    int ret;
    char *cmd;
    int perr[2];
    int pout[2];
    pid_t child, pid;
    char c;

    /* check if command valid */
    if (!command)
        return EXIT_FAILURE;

    /* create full cmd string */
    ret = vasprintf(&cmd, command, args);
    if (ret < 0)
        return EXIT_FAILURE;

    /* create pipe for stdout */
    ret = pipe2(pout, O_NONBLOCK);
    if (ret)
        goto cmdfree;

    /* create pipe for stderr */
    ret = pipe2(perr, O_NONBLOCK);
    if (ret)
        goto closepout;

    ret = EXIT_FAILURE;

    child = vfork();
    switch (child) {
    case -1: /* Error */
        goto closeperr;
    case 0: /* Child */
        /* attach stdout to pipe */
        if (pout[1] != STDOUT_FILENO) {
            (void) dup2(pout[1], STDOUT_FILENO);
            (void) close(pout[1]);
        }
        (void) close(pout[0]);

        /* attach stderr to pipe */
        if (perr[1] != STDERR_FILENO) {
            (void) dup2(perr[1], STDERR_FILENO);
            (void) close(perr[1]);
        }
        (void) close(perr[0]);

        execl(_PATH_BSHELL, "sh", "-c", cmd, NULL);
        _exit(127);
        /* NOTREACHED */
    }

    /* Parent */
    do {
        int status;
        pid = waitpid(child, &status, 0);
    } while (pid == -1 && errno == EINTR);

    /* check for any stderr output of cmd */
    if (read(perr[0], &c, sizeof (c)) > 0)
        base_print_warning("%s(): %s returned error!", __func__, cmd);
    else
        ret = EXIT_SUCCESS;

closeperr:
    close(perr[0]);
    close(perr[1]);
closepout:
    close(pout[0]);
    close(pout[1]);
cmdfree:
    free(cmd);
    return ret ? EXIT_FAILURE : EXIT_SUCCESS;
}

int cmd_execute(const char *command, ...)
{
    int ret;
    va_list args;

    va_start(args, command);
    ret = vcmd_execute(command, args);
    va_end(args);

    return ret;
}

base_status_t get_str_sys_call(module_index_t module_index, int obj_index, base_rc_t rc_flag,
                               void *result_array, unsigned int row_num, unsigned int col_num,
                               va_list args)
{
    FILE *output = NULL;
    char str[MAX_STR_LEN];
    base_status_t status = BASE_FAILED;
    sys_call_spec_t tmp = assemble_sys_call_structure(module_index, obj_index, rc_flag);

    base_print_debug(DEBUG_MSG_FUN_CALLED, __func__);

    if (are_object_indices_valid(module_index, obj_index, 0, 0)) {
        do {
            vsprintf(str, tmp.sys_call_command, args);
            base_print_debug("DEBUG: %s(): cmd: %s\n", __func__, tmp.sys_call_command);
            base_print_debug("DEBUG: %s(): str: %s\n", __func__, str);

            (output) = popen(str, "r");

            if (NULL == (output)) {
                base_print_warning("%s: failed to open stream!", __func__);
                break;
            } else {
                // COMBINATOR, always first
                status = (tmp.combinator[0])(module_index, obj_index,
                                             0, output, rc_flag,
                                             result_array, row_num, col_num);
            }
        } while (0);
        free_output(&output);
    }

    return status;
}

base_status_t read_file_sys_call(module_index_t module_index, int obj_index, base_rc_t rc_flag,
                                 void *result_array, unsigned int row_num, unsigned int col_num,
                                 va_list args)
{
    FILE *output = NULL;
    char str[MAX_STR_LEN];
    base_status_t status;
    sys_call_spec_t tmp = assemble_sys_call_structure(module_index, obj_index, rc_flag);

    base_print_debug(DEBUG_MSG_FUN_CALLED, __func__);

    if (!are_object_indices_valid(module_index, obj_index, 0, 0)) {
        return BASE_FAILED;
    }

    vsprintf(str, tmp.sys_call_command, args);
    base_print_debug("DEBUG: %s(): file path: %s\n", __func__, tmp.sys_call_command);
    base_print_debug("DEBUG: %s(): str: %s\n", __func__, str);

    output = fopen(str, "r");
    if (!output) {
        base_print_warning("%s: failed to open file!", __func__);
        return BASE_FAILED;
    }

    // COMBINATOR, always first
    status = (tmp.combinator[0])(module_index, obj_index,
                                 0, output, rc_flag,
                                 result_array, row_num, col_num);

    fclose(output);
    return status;
}
