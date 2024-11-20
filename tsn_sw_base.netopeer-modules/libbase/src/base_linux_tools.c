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

#include "base_defines.h"
#include "base_prints.h"
#include "base_interfaces_linux_tools.h"
#include "base_mstp_linux_tools.h"
#include "base_q_bridge_linux_tools.h"
#include "base_private_syscalls.h"

int get_object(module_index_t module_index, char *object_name, base_rc_t rc_flag,
               void *result_array, unsigned int row_num, unsigned int col_num,
               ...)
{
    int ret_value = EXIT_SUCCESS;
    base_status_t status = BASE_FAILED;
    int obj_index = 0;
    va_list args;
    va_start(args, col_num);

    base_print_debug(DEBUG_MSG_FUN_CALLED_WITH_ONE_PARAM, __func__,
                     object_name);

    if (NULL == result_array) {
        base_print_debug(ERROR_MSG_FUN_AND_MSG, __func__,
                         "Invalid input - result_array is NULL.\n");
        ret_value = EXIT_FAILURE;
    } else {
        memset(result_array, 0, row_num * col_num);

        switch (module_index) {
        case IETF_INTERFACES:
            obj_index = find_obj_index_by_name(object_name, if_map, IF_MAX_OBJ_NUMBER);
            status = execute_sys_call(module_index, obj_index, rc_flag,
                                      result_array, row_num, col_num, args);
            break;
        case MSTP:
            obj_index = find_obj_index_by_name(object_name, mstp_map, MSTP_MAX_OBJ_NUMBER);
            status = execute_sys_call(module_index, obj_index, rc_flag,
                                      result_array, row_num, col_num,args);
            break;
        case Q_BRIDGE:
            obj_index = find_obj_index_by_name(object_name, q_bridge_map, Q_BRIDGE_MAX_OBJ_NUMBER);
            status = execute_sys_call(module_index, obj_index, rc_flag,
                                      result_array, row_num, col_num,args);
            break;
        default:
            base_print_warning("%s(): invalid module index (%d)!", __func__,
                               module_index);
            memset(result_array, 0, row_num * col_num);
            ret_value = EXIT_FAILURE;
            break;
        }

        if (
                // validation can pass only on one of combinators
                ((BASE_VALIDATION_FAILED & status) && !(BASE_VALIDATION_SUCCEEDED & status)) ||
                // parsing can pass only on one of combinators
                ((BASE_PARSING_FAILED & status) && !(BASE_PARSING_SUCCEEDED & status)) ||
                // clean fail
                (BASE_FAILED == status)) {
            memset(result_array, 0, row_num * col_num);
            ret_value = EXIT_FAILURE;
        }

        va_end(args);
    }

    return ret_value;
}

int set_object(module_index_t module_index, char *object_name, ...)
{
    int ret_value = EXIT_SUCCESS;
    int obj_index = 0;
    va_list args;
    va_start(args, object_name);

    base_print_debug(DEBUG_MSG_FUN_CALLED_WITH_ONE_PARAM, __func__, object_name);

    switch (module_index) {
    case IETF_INTERFACES:
        obj_index = find_obj_index_by_name(object_name, if_map, IF_MAX_OBJ_NUMBER);
        ret_value = vcmd_execute(if_set_sys_call_commands[obj_index], args);
        if (ret_value)
            base_print_warning("%s: vcmd_execute failed!", __func__);
        break;
    case MSTP:
        obj_index = find_obj_index_by_name(object_name, mstp_map, MSTP_MAX_OBJ_NUMBER);
        ret_value = vcmd_execute(mstp_set_sys_call_commands[obj_index], args);
        if (ret_value)
            base_print_warning("%s: vcmd_execute failed!", __func__);
        break;
    case Q_BRIDGE:
        obj_index = find_obj_index_by_name(object_name, q_bridge_map, Q_BRIDGE_MAX_OBJ_NUMBER);
        ret_value = vcmd_execute(q_bridge_set_sys_call_commands[obj_index], args);
        if (ret_value)
            base_print_warning("%s: vcmd_execute failed!", __func__);
        break;
    default:
        ret_value = EXIT_FAILURE;
        break;
    }
    va_end(args);

    return ret_value;
}
