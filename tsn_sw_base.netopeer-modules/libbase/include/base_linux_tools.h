/*
 * TTTech libbase
 * Copyright(c) 2019 TTTech Computertechnik AG.
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

#ifndef INCLUDE_BASE_LINUX_TOOLS_H_
#define INCLUDE_BASE_LINUX_TOOLS_H_

#include "base_private_defines.h"

/**
 * @brief This function will return value of the demanded object.
 *
 * @param[in]   module_index    Predefined index of the module (enum
 *                              module_index_t from base_defines.h).
 * @param[in]   object_name     Name of the object (function will work only
 *                              with object names defined in
 *                              base_<module_name>_linux_tools.h).
 * @param[out]  result_array    Place holder for value(s) of the object(s).
 *                              Caller should provide address of char matrix as
 *                              result_array.
 * @param[in]   row_num         Number of result_array's rows.
 * @param[in]   col_num         Number of result_array's columns.
 * @param[in]   rc_flag         If it is BASE_RC_YES, system call structure can
 *                              be changed in runtime.
 * @param[in]   ...             List of strings needed for system call command.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
int get_object(module_index_t module_index, char *object_name, base_rc_t rc_flag,
               void *result_array, unsigned int row_num, unsigned int col_num,
               ...);

/**
 * @brief This function will set value of the object identified with
 * object_name.
 *
 * @param[in]   module_index    Predefined index of the module (enum
 *                              module_index_t from base_defines.h).
 * @param[in]   object_name     Name of the object (function will work only
 *                              with object names defined in
 *                              base_<module_name>_linux_tools.h).
 * @param[in]   ...             List of strings needed for system call command.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
int set_object(module_index_t module_index, char *object_name, ...);

#endif /* INCLUDE_BASE_LINUX_TOOLS_H_ */
