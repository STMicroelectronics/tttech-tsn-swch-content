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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <paths.h>
#include <sys/wait.h>
#include "base_linux_tool_helpers.h"
#include "base_q_bridge_linux_tools.h"
#include "base_interfaces_linux_tools.h"
#include "base_private_syscalls.h"
#include "base_defines.h"
#include "base_prints.h"

// needed for runtime configurations of syscall chain
sys_call_spec_t rc_sys_call_spec;

int are_object_indices_valid(module_index_t module_index, int obj_index,
                             int comb_index, int parser_index)
{
    int status = 1; // true

    base_print_debug(DEBUG_MSG_FUN_CALLED, __func__);

    switch (module_index) {
    case IETF_INTERFACES:
        if (IF_MAX_OBJ_NUMBER <= obj_index) {
            status = 0;
            base_print_warning("%s(): invalid object index (%d) for module index (%d)!",
                               __func__, obj_index, module_index);
            break;
        }
        if (MAX_COMBINATORS_NUM <= comb_index) {
            status = 0;
            base_print_warning("%s(): invalid comb_index (%d) for module index (%d)!",
                               __func__, obj_index, comb_index);
            break;
        }
        if (MAX_CALLBACKS_NUM <= parser_index) {
            status = 0;
            base_print_warning("%s(): invalid parser_index (%d) for module index (%d)!",
                               __func__, obj_index, parser_index);
            break;
        }
        break;
    case MSTP:
        if (MSTP_MAX_OBJ_NUMBER <= obj_index) {
            status = 0;
            base_print_warning("%s(): invalid object index (%d) for module index (%d)!",
                               __func__, obj_index, module_index);
            break;
        }
        if (MAX_COMBINATORS_NUM <= comb_index) {
            status = 0;
            base_print_warning("%s(): invalid comb_index (%d) for module index (%d)!",
                               __func__, obj_index, comb_index);
            break;
        }
        if (MAX_CALLBACKS_NUM <= parser_index) {
            status = 0;
            base_print_warning("%s(): invalid parser_index (%d) for module index (%d)!",
                               __func__, obj_index, parser_index);
            break;
        }
        break;
    case Q_BRIDGE:
        if (Q_BRIDGE_MAX_OBJ_NUMBER <= obj_index) {
            status = 0;
            base_print_warning("%s(): invalid object index (%d) for module index (%d)!",
                               __func__, obj_index, module_index);
            break;
        }
        if (MAX_COMBINATORS_NUM <= comb_index) {
            status = 0;
            base_print_warning("%s(): invalid comb_index (%d) for module index (%d)!",
                               __func__, obj_index, comb_index);
            break;
        }
        if (MAX_CALLBACKS_NUM <= parser_index) {
            status = 0;
            base_print_warning("%s(): invalid parser_index (%d) for module index (%d)!",
                               __func__, obj_index, parser_index);
            break;
        }
        break;
    default:
        base_print_warning("%s(): invalid module index (%d)!",
                           __func__, module_index);
        status = 0;
        break;
    }
    return status;
}

int find_obj_index_by_name(char *object_name, void *map, int max_obj_number)
{
    int i = 0;
    int ret_value = 0;

    base_print_debug(DEBUG_MSG_FUN_CALLED, __func__);
    for (i = 0; i < max_obj_number; i++) {
        if (0 == strncmp(((char*)map + i*MAX_STR_LEN), object_name,
                         MAX_STR_LEN)) {
            ret_value = i;
            break;
        }
    }

    return ret_value;
}

sys_call_spec_t assemble_sys_call_structure(module_index_t module_index, int object_index, base_rc_t rc_flag)
{
    sys_call_spec_t tmp_sys_call_spec;
    memset(&tmp_sys_call_spec, 0, sizeof(tmp_sys_call_spec));

    base_print_debug(DEBUG_MSG_FUN_CALLED, __func__);

    if (rc_flag) {
        memcpy(&tmp_sys_call_spec, &rc_sys_call_spec, sizeof(rc_sys_call_spec));
    } else {
        switch (module_index) {
        case IETF_INTERFACES:
            if (IF_MAX_OBJ_NUMBER <= object_index) {
                base_print_warning("%s(): invalid object index (%d) for module index (%d)!", __func__,
                                   object_index, module_index);
                break;
            } else {
                tmp_sys_call_spec.sys_call = (syscall_fn_t*)&if_syscalls[object_index];
                tmp_sys_call_spec.sys_call_command = (char*)if_get_sys_call_commands[object_index];
                tmp_sys_call_spec.num_of_combinators = if_num_of_combinators[object_index];
                tmp_sys_call_spec.combinator = (combinator_fn_t*)if_combinators[object_index];
                memcpy(tmp_sys_call_spec.num_of_cb, if_num_of_cb[object_index], sizeof(if_num_of_cb[object_index]));
                memcpy(tmp_sys_call_spec.parse_cb, if_parsing_cb[object_index], sizeof(if_parsing_cb[object_index]));
                break;
            }
        case MSTP:
            if (MSTP_MAX_OBJ_NUMBER <= object_index) {
                base_print_warning("%s(): invalid object index (%d) for module index (%d)!", __func__,
                                   object_index, module_index);
                break;
            } else {
                tmp_sys_call_spec.sys_call = (syscall_fn_t*)&mstp_syscalls[object_index];
                tmp_sys_call_spec.sys_call_command = (char*)mstp_get_sys_call_commands[object_index];
                tmp_sys_call_spec.num_of_combinators = mstp_num_of_combinators[object_index];
                tmp_sys_call_spec.combinator = (combinator_fn_t*)mstp_combinators[object_index];
                memcpy(tmp_sys_call_spec.num_of_cb, mstp_num_of_cb[object_index], sizeof(mstp_num_of_cb[object_index]));
                memcpy(tmp_sys_call_spec.parse_cb, mstp_parsing_cb[object_index], sizeof(mstp_parsing_cb[object_index]));
                break;
            }
        case Q_BRIDGE:
            if (Q_BRIDGE_MAX_OBJ_NUMBER <= object_index) {
                base_print_warning("%s(): invalid object index (%d) for module index (%d)!", __func__,
                                   object_index, module_index);
                break;
            } else {
                tmp_sys_call_spec.sys_call = (syscall_fn_t*)&q_bridge_syscalls[object_index];
                tmp_sys_call_spec.sys_call_command = (char*)q_bridge_get_sys_call_commands[object_index];
                tmp_sys_call_spec.num_of_combinators = q_bridge_num_of_combinators[object_index];
                tmp_sys_call_spec.combinator = (combinator_fn_t*)q_bridge_combinators[object_index];
                memcpy(tmp_sys_call_spec.num_of_cb, q_bridge_num_of_cb[object_index], sizeof(q_bridge_num_of_cb[object_index]));
                memcpy(tmp_sys_call_spec.parse_cb, q_bridge_parsing_cb[object_index], sizeof(q_bridge_parsing_cb[object_index]));
                break;
            }
        default:
            base_print_warning("%s(): invalid module index (%d)!", __func__,
                               module_index);
            break;
        }
    }

    return tmp_sys_call_spec;
}

void set_rc_sys_call_spec(sys_call_spec_t sys_call_spec)
{
    memcpy(&rc_sys_call_spec, &sys_call_spec, sizeof(sys_call_spec));
}

base_status_t execute_sys_call(module_index_t module_index, int obj_index, base_rc_t rc_flag,
                               void *result_array, unsigned int row_num, unsigned int col_num,
                               va_list args)
{
    base_status_t status = BASE_FAILED;
    sys_call_spec_t tmp = assemble_sys_call_structure(module_index, obj_index, rc_flag);

    base_print_debug(DEBUG_MSG_FUN_CALLED, __func__);

    if (are_object_indices_valid(module_index, obj_index, 0, 0)) {
        status = (*tmp.sys_call)(module_index, obj_index, rc_flag,
                                 result_array, row_num, col_num, args);
    }

    return status;
}

/**
 * @brief      Freeing string.
 *
 * @param str  Double pointer that points to string.
 * @return     Void.
 */
void free_string(char** str)
{
    if (NULL != *str) {
         free(*str);
         *str = NULL;
    }
}

/**
 * @brief         Freeing object of FILE type with pclose.
 *
 * @param output  Double pointer to FILE object.
 * @return        Void.
 */
void free_output(FILE** output)
{
    if (NULL != *output) {
        pclose(*output);
        *output = NULL;
    }
}

int ip_link_set_dev_down(char *dev_name)
{
    base_print_debug(DEBUG_MSG_FUN_CALLED_WITH_ONE_PARAM, __func__, dev_name);

    return cmd_execute("ip link set dev %s down", dev_name);
}

int ip_link_set_dev_up(char *dev_name)
{
    base_print_debug(DEBUG_MSG_FUN_CALLED_WITH_ONE_PARAM, __func__, dev_name);

    return cmd_execute("ip link set dev %s up", dev_name);
}

int ip_link_set_dev_address(char *dev_name, char *value)
{
    base_print_debug(DEBUG_MSG_FUN_CALLED_WITH_TWO_PARAM, __func__, dev_name, value);

    return cmd_execute("ip link set dev %s address %s", dev_name, value);
}

int if_interface_exist(char *if_name)
{
    char interface_names[MAX_NUM_OF_INTERFACES][MAX_STR_LEN];
    int exist = 0; /* 0 = not exist, 1 = exist*/
    int i = 0;

    base_print_debug(DEBUG_MSG_FUN_CALLED_WITH_ONE_PARAM, __func__, if_name);

    if (EXIT_FAILURE == get_interface_names(&interface_names, MAX_NUM_OF_INTERFACES, MAX_STR_LEN)) {
        base_print_warning("if_interface_exist(): get_interface_names() returns EXIT_FAILURE!");
        return EXIT_FAILURE;
    }

    while ((i < MAX_NUM_OF_INTERFACES) &&
           (0 != strncmp(interface_names[i], "", MAX_STR_LEN))) {
        if (0 == strncmp(interface_names[i], if_name, MAX_STR_LEN)) {
            exist = 1;
        }
        i++;
    }

    if (0 == exist) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int is_pvid_in_vids_string(char *pvid, char *vids)
{
    int ret_value = 0;
    int i = 0;
    char vids_array[BASE_MAX_VID][BASE_FIVE];

    base_print_debug(DEBUG_MSG_FUN_CALLED, __func__);

    assemble_vids_as_array(vids_array, BASE_MAX_VID, BASE_FIVE, vids);
    while ((i < BASE_MAX_VID) &&
           (0 != strncmp(vids_array[i], "", BASE_FIVE))) {
        /* Check is current PVID is equal to vid*/
        if (0 == strncmp(vids_array[i], pvid, BASE_FIVE)) {
            ret_value = 1;
            break;
        }
        i++;
    }

    return ret_value;
}

/**
 * Helper function
 * Checks if string is sub-interval of vids
 */
int is_sub_interval(char *potential_sub_interval)
{
    const char *minus = "-";
    char *p = potential_sub_interval;

    while ('\0' != *p) {
        if (0 != strchr(minus, *p)) {
            return 1;
        }
        p++;
    }
    return 0;
}

/*
 * Check is address ipv4 or ipv6
 */
int check_address_type(char *address, char *value)
{
    int ivp4_cnt = 0;
    int ivp6_cnt = 0;

    base_print_debug(DEBUG_MSG_FUN_CALLED, __func__);

    for (int i = 0; address[i] != 0; i++) {
        if (address[i] == '.') {
            ivp4_cnt++;
        }
        if (address[i] == ':') {
            ivp6_cnt++;
        }
    }

    if (0 != ivp4_cnt) {
        strncpy(value, "1", BASE_TWO); // termination character
    }
    else if (0 != ivp6_cnt) {
        strncpy(value, "2", BASE_TWO); // termination character
    } else {
        base_print_warning("Failed to define address type (ipv4 or ipv6)!");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

/*
 * change ':' to '-'
 */
void change_addr_colons_to_dashes(char *address)
{
    for (int i = 0; address[i] != '\0'; i++) {
        if (address[i] == ':') {
            address[i] = '-';
        }
    }
}

/*
 * change '-' to ':'
 */
void change_addr_dashes_to_colons(char *address)
{
    for (int i = 0; address[i] != '\0'; i++) {
        if (address[i] == '-') {
            address[i] = ':';
        }
    }
}

/*
 * change '\n' to '\0'
 */
void terminate_string(char *str)
{
    int i = 0;

    for (i = 0; str[i] != '\0'; i++) {
        if (str[i] == '\n') {
            str[i] = '\0';
            break;
        }
    }
}

void assemble_vids_as_array(void *result_array, unsigned int row_num, unsigned int col_num,
                            char *vids)
{
    char *tmp_vids = NULL;
    char *sub_str_ptr = NULL;
    char *sub_str_start = NULL;
    char *sub_str_end = NULL;
    char sub_str[BASE_TEN] = "";
    char *tmp_array = NULL;
    char min_str[BASE_FIVE] = "";
    char max_str[BASE_FIVE] = "";
    char *min_start = NULL;
    char *min_end = NULL;
    char *max_start = NULL;
    char *max_end = NULL;
    char *back_directions = NULL;
    char *forward_directions = NULL;
    long int min = 0;
    long int max = 0;
    long int i = 0;
    unsigned int j = 0;

    base_print_debug(DEBUG_MSG_FUN_CALLED_WITH_ONE_PARAM, __func__, vids);

    memset(result_array, 0, row_num  * col_num);

    if (NULL == vids) {
        base_print_warning(ERROR_MSG_FUN_AND_MSG, __func__, ERROR_MSG_NULL_VIDS);
        return;
    }
    if (MAX_STR_LEN < strlen(vids)) {
        base_print_warning(ERROR_MSG_FUN_AND_MSG, __func__, ERROR_MSG_TOO_LONG_VIDS);
        return;
    }
    tmp_vids = vids;
    sub_str_start = vids; // first position of first string separated with ,
    sub_str_ptr = vids;
    while (NULL != sub_str_ptr) {
        if ((',' == *sub_str_ptr) || (('\0' == *sub_str_ptr))) {
            // take sub_str between ,
            strncpy(sub_str, sub_str_start, sub_str_end - sub_str_start + 1);
            sub_str_start = sub_str_ptr; //'\0' or ','
            sub_str_start++; // potential new start
            /* if we got e.g. 10-15 */
            if (1 == is_sub_interval(sub_str)) {
                tmp_vids = sub_str;
                min_start = tmp_vids;
                // global loop for the string
                while (*tmp_vids != '\0') {
                    if (('-' == *tmp_vids)) {
                        back_directions = tmp_vids; // take minus position
                        forward_directions = tmp_vids; // take minus position
                        back_directions--;
                        min_end = back_directions; // character before minus
                        forward_directions++;
                        max_start = forward_directions; // character after minus
                    }
                    tmp_vids++;
                }
                max_end = --tmp_vids; // go back one place
                if ((BASE_FIVE < (min_end - min_start + 1)) ||
                    (BASE_FIVE < (max_end - max_start + 1))) {
                    base_print_debug(DEBUG_MSG_FUN_NAME_AND_MSG, __func__,
                                     "vid in vid interval is too long");
                    return;
                }
                strncpy(min_str, min_start, min_end - min_start + 1);
                strncpy(max_str, max_start, max_end - max_start + 1);
                min = strtol(min_str, NULL, 10);
                max = strtol(max_str, NULL, 10);
                if (max < min) {
                    base_print_debug(DEBUG_MSG_FUN_NAME_AND_MSG, __func__,
                                     "max is LOWER than min in vid interval");
                    return;
                }
                min_end = NULL; // reset
                max_start = NULL; // reset
                max_end = NULL; // reset
                for (i = min; i <= max; i++) {
                    tmp_array = (char*)result_array + (j * col_num);
                    snprintf(tmp_array, col_num, "%ld", i);
                    if (j < row_num) {
                        j++;
                    } else {
                        break;
                    }
                }
            } else {
                tmp_array = (char*)result_array + (j * col_num);
                snprintf(tmp_array, col_num, "%s", sub_str);
                if (j < row_num) {
                    j++;
                } else {
                    break;
                }
            }
            memset(sub_str, 0, BASE_TEN); // erase sub_str
            if ('\0' == *sub_str_ptr) {
                break; // end of vids
            }
        }
        /* next */
        sub_str_end = sub_str_ptr; // save potential end, if next char is ,
        sub_str_ptr++;
    }
}

int does_vids_string_contains_descending_interval(char *vids)
{
    char *tmp_vids = NULL;
    char min_str[BASE_FIVE] = "";
    char max_str[BASE_FIVE] = "";
    char *min_start = NULL;
    char *min_end = NULL;
    char *max_start = NULL;
    char *max_end = NULL;
    char *back_directions = NULL;
    char *forward_directions = NULL;
    int ret_value = EXIT_FAILURE;//
    int interval_is_detected = 0;
    long int min = 0;
    long int max = 0;

    base_print_debug(DEBUG_MSG_FUN_CALLED_WITH_ONE_PARAM, __func__, vids);

    if (NULL == vids) {
        return ret_value;
    }
    tmp_vids = vids;
    min_start = vids; // first character of the vids
    if (!isdigit(*tmp_vids)) {
        // if first character is not digit, vids string is invalid
        base_print_debug(DEBUG_MSG_FUN_NAME_AND_MSG, __func__,
                         "invalid vid interval - first character is not a digit");
        return ret_value;
    }
    while (*tmp_vids != '\0') {
        // global loop for the string
        if (('-' == *tmp_vids)) {
            interval_is_detected = 1; // potentially valid interval is detected
            back_directions = tmp_vids; // take minus position
            forward_directions = tmp_vids; // take minus position
            back_directions--;
            min_end = back_directions; // character before minus
            forward_directions++;
            max_start = forward_directions; // character after minus
            if (!isdigit(*max_start)) {
                // invalid vids string, minus can not be last
                base_print_debug(DEBUG_MSG_FUN_NAME_AND_MSG, __func__,
                                 "invalid vid interval - no digit after minus");
                return ret_value;
            }
            // local loop for the interval
            while (*tmp_vids != '\0') {
                if (',' == *tmp_vids) {
                    forward_directions = tmp_vids;
                    forward_directions--;
                    max_end = forward_directions; // character before minus is last of the max
                    // comma means one vid/vids is identified
                    break;
                }
                tmp_vids++; // next character
                if ('\0' == *tmp_vids) {
                    forward_directions = tmp_vids;
                    forward_directions--;
                    max_end = forward_directions; // character before minus is last of the max
                    break; // end of string means last vid/vids is identified
                }
            }
        }
        if (interval_is_detected) {
            // check if interval is valid
            interval_is_detected = 0; // reset
            if ((BASE_FIVE < (min_end - min_start + 1)) ||
                (BASE_FIVE < (max_end - max_start + 1))) {
                base_print_debug(DEBUG_MSG_FUN_NAME_AND_MSG, __func__,
                                 "vid in vid interval is too long");
                return ret_value;
            }
            strncpy(min_str, min_start, min_end - min_start + 1);
            strncpy(max_str, max_start, max_end - max_start + 1);
            min = strtol(min_str, NULL, 10);
            max = strtol(max_str, NULL, 10);
            if (max < min) {
                base_print_debug(DEBUG_MSG_FUN_NAME_AND_MSG, __func__,
                                 "max is LOWER than min in vid interval");
                return ret_value;
            }
            if ('\0' != *tmp_vids) {
                min_start = tmp_vids;
                min_start++; // new min start is after comma
                min_end = NULL; // reset
                max_start = NULL; // reset
                max_end = NULL; // reset
            }
        }
        if (',' == *tmp_vids) {
            /* the case when current character is comma and we did not find any
             * interval until now
             */
            min_start = tmp_vids;
            min_start++; // new min start is after comma
        }
        if ('\0' != *tmp_vids) {
            tmp_vids++; // next character
        }
    }
    ret_value = EXIT_SUCCESS;

    return ret_value;
}

int does_vids_string_contains_invalid_vid(char *vids)
{
    char vid_array[BASE_MAX_VID][BASE_FIVE];
    int ret_value = EXIT_SUCCESS;
    int i = 0;

    base_print_debug(DEBUG_MSG_FUN_CALLED_WITH_ONE_PARAM, __func__, vids);

    assemble_vids_as_array(vid_array, BASE_MAX_VID, BASE_FIVE, vids);
    while ((i < BASE_MAX_VID) && (0 != strncmp(vid_array[i], "", BASE_FIVE))) {
        if ((BASE_MAX_VID <= strtol(vid_array[i], NULL, 10)) ||
            (BASE_MIN_VID > strtol(vid_array[i], NULL, 10))) {
            ret_value = EXIT_FAILURE;
            // this is invalid vid, lower than zero is already covered with validation
            break;
        }
        i++;
    }

    return ret_value;
}

static const char base64_encode_table[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

int base64_encode(unsigned char *input, int input_len, char *encoded)
{
    int i = 0;
    int j = 0;
    int k = 0;
    unsigned char tmp_input[3] = {0};

    base_print_debug(DEBUG_MSG_FUN_CALLED, __func__);

    for (i = 0 ; i < input_len ; i++) {
        tmp_input[j++] = input[i];
        if (j == 3) {
            j = 0;
            encoded[k++] = base64_encode_table[(tmp_input[j] >> 2) & 0x3f];
            encoded[k++] = base64_encode_table[((tmp_input[j] & 0x03) << 4) | (tmp_input[j+1] & 0xf0) >> 4];
            encoded[k++] = base64_encode_table[((tmp_input[j+1] & 0xf) << 2) | (tmp_input[j+2] & 0xc0) >> 6];
            encoded[k++] = base64_encode_table[tmp_input[j+2] & 0x3f];
            j = 0;
        }
    }

    if (j > 0) {
        tmp_input[2] = 0;
        if (j == 1) {
            tmp_input[1] = 0;
        }
        encoded[k++] = base64_encode_table[(tmp_input[0] >> 2) & 0x3f];
        encoded[k++] = base64_encode_table[((tmp_input[0] & 0x03) << 4) | (tmp_input[1] & 0xf0) >> 4];

        if (j == 2) {
            encoded[k++] = base64_encode_table[((tmp_input[1] & 0x0f) << 2)];
        } else {
            encoded[k++] = '=';
        }
        encoded[k++] = '=';
    }
    encoded[k] = '\0';

    return EXIT_SUCCESS;
}
