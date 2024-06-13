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

#ifndef INCLUDE_BASE_PRIVATE_SYSCALLS_H_
#define INCLUDE_BASE_PRIVATE_SYSCALLS_H_

#include "base_linux_tool_helpers.h"
#include "base_private_defines.h"
#include <fcntl.h>
#include <unistd.h>
#include <paths.h>
#include <sys/wait.h>
#include <errno.h>

/**
 * @brief Assembling and sending command to shell (system).
 *
 * Has check if command executed successfully.
 *
 * @param       format          Char pointer that holds command.
 * @param       args            va_list of arguments for format.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
int object_set_system_call(const char *format, va_list args);

/**
 * @brief Execute a shell command.
 *
 * Any output on stderr is considered as failure
 *
 * @param       command         command format string
 * @param       args            va_list of arguments for command.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
int vcmd_execute(const char *command, va_list args);

/**
 * @brief Execute a shell command.
 *
 * Any output on stderr is considered as failure
 *
 * @param       command         command format string
 * @param       ...             command parameters
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
int cmd_execute(const char *command, ...);

/**
 * @brief This function will return system call structure (sys_call_spec_t) for
 * object_index.
 *
 * @param[in]   module_index    Predefined module index.
 * @param[in]   obj_index       Predefined object index.
 * @param[in]   rc_flag         If it is BASE_RC_YES, system call structure can
 *                              be changed in runtime.
 * @param[out]  result_array    Place holder for value(s) of the object(s).
 *                              Caller should provide address of char matrix as
 *                              result_array.
 * @param[in]   row_num         Number of result_array's rows.
 * @param[in]   col_num         Number of result_array's columns.
 * @param[in]   args            List of arguments which will be passed to first
 *                              combinator.
 * @returns     Predefined enum value (base_status_t).
 */
base_status_t get_str_sys_call(module_index_t module_index, int obj_index,
                               base_rc_t rc_flag,
                               void *result_array, unsigned int row_num, unsigned int col_num,
                               va_list args);

/**
 * @brief This function will return system call structure (sys_call_spec_t) for
 * object_index.
 *
 * @param[in]   module_index    Predefined module index.
 * @param[in]   obj_index       Predefined object index.
 * @param[in]   rc_flag         If it is BASE_RC_YES, system call structure can
 *                              be changed in runtime.
 * @param[out]  result_array    Place holder for value(s) of the object(s).
 *                              Caller should provide address of char matrix as
 *                              result_array.
 * @param[in]   row_num         Number of result_array's rows.
 * @param[in]   col_num         Number of result_array's columns.
 * @param[in]   args            List of arguments which will be passed to first
 *                              combinator.
 * @returns     Predefined enum value (base_status_t).
 */
base_status_t read_file_sys_call(module_index_t module_index, int obj_index,
                                 base_rc_t rc_flag,
                                 void *result_array, unsigned int row_num, unsigned int col_num,
                                 va_list args);

#endif /* INCLUDE_BASE_PRIVATE_SYSCALLS_H_ */
