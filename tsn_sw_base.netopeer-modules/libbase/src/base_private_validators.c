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

/**
 * @file base_private_validators.c
 * @brief Base library parsers functions and their helpers (as static functions).
 * Input parameters checks are done in combinators layer for next input parameters:
 * module_index,
 * obj_indexm,
 * comb_index,
 * parser_index,
 * input_str.
 * Caller provides result_array (should point to char matrix), row_num (number of rows in result_array matrix)
 * and col_num (number of columns in result_array matrix).
 * Caller is responsible that row_num and col_num input parameters correspondence to the result_array.
 * Validation of result_array is done in linux tools (get_object function).
 */

#include "base_private_parsers.h"
#include "base_prints.h"

//declarations of static functions
static base_status_t array_validate_interval_helper(int lower, module_index_t module_index, int obj_index,
                                                    int comb_index, int parser_index,
                                                    char *input_str, base_rc_t rc_flag,
                                                    void *result_array, unsigned int row_num, unsigned int col_num);

base_status_t validate_uint_32_number(module_index_t module_index, int obj_index,
                                      int comb_index, int parser_index,
                                      char *input_str, base_rc_t rc_flag,
                                      void *result_array, unsigned int row_num, unsigned int col_num)
{
    (void)module_index;
    (void)obj_index;
    (void)comb_index;
    (void)parser_index;
    (void)input_str;
    (void)rc_flag;
    (void)row_num;

    int status = BASE_VALIDATION_SUCCEEDED;
    unsigned int i = 0;
    long int str_uint_32_value = 0;
    char *tmp_result_array = NULL;

    base_print_debug(DEBUG_MSG_FUN_CALLED, __func__);

    tmp_result_array = result_array;
    if ('\0' == *tmp_result_array) {
        // check if first character indicates empty string
        base_print_debug(DEBUG_MSG_FUN_NAME_AND_MSG, __func__,
                         "Given input is NOT a number. It is empty string");
        status = BASE_VALIDATION_FAILED;
    } else {
        while ((i < col_num) &&
               ('\0' != *(tmp_result_array + i))) {
            // break on string terminator or max size of the string
            if (!isdigit(*(tmp_result_array + i))) {
                base_print_debug(DEBUG_MSG_FUN_NAME_AND_MSG, __func__,
                                 "Given input is NOT a number");
                *tmp_result_array  = '\0'; // first position in string
                status = BASE_VALIDATION_FAILED;
                break;
            }
            i++;
        }
    }

    if (BASE_VALIDATION_SUCCEEDED == status) {
        // if string is valid number, check if it is valid uint_32 value
        str_uint_32_value = strtol(tmp_result_array, NULL, 10);
        if (MAX_UNINT_32_VALUE < str_uint_32_value) {
            base_print_debug(DEBUG_MSG_FUN_NAME_AND_MSG, __func__,
                             "Given input is NOT a number. Value is greater than max uint_32");
            status = BASE_VALIDATION_FAILED;
        }
    }

    return status;
}

base_status_t validate_string_content(module_index_t module_index, int obj_index,
                                      int comb_index, int parser_index,
                                      char *input_str, base_rc_t rc_flag,
                                      void *result_array, unsigned int row_num, unsigned int col_num)
{
    (void)input_str;
    (void)row_num;

    base_status_t status = BASE_VALIDATION_SUCCEEDED;
    sys_call_spec_t tmp = assemble_sys_call_structure(module_index, obj_index, rc_flag);
    // can return structure filled with 0s

    base_print_debug(DEBUG_MSG_FUN_CALLED, __func__);

    if (0 != strncmp(tmp.parse_cb[comb_index][parser_index].match_case,
                     result_array, col_num)) {
        // validate string, so we take value[0]
        base_print_debug("%s(): String content is invalid ((%s) vs (%s)).\n",
                __func__, tmp.parse_cb[comb_index][parser_index].match_case,
                (char*)result_array);
        status = BASE_VALIDATION_FAILED;
    }

    return status;
}

base_status_t array_validate_string_length(module_index_t module_index, int obj_index,
                                           int comb_index, int parser_index,
                                           char *input_str, base_rc_t rc_flag,
                                           void *result_array, unsigned int row_num, unsigned int col_num)
{
    (void)input_str;

    base_status_t status = BASE_VALIDATION_FAILED;
    unsigned int i = 0;
    sys_call_spec_t tmp = assemble_sys_call_structure(module_index, obj_index, rc_flag);
    // can return structure filled with 0s
    long int tmp_match_case_value = 0;
    char *tmp_result_array = NULL;

    base_print_debug(DEBUG_MSG_FUN_CALLED, __func__);

    // take match case string value as long int
    tmp_match_case_value = strtol(tmp.parse_cb[comb_index][parser_index].match_case, NULL, 10);
    tmp_result_array = result_array;
    while ((i < row_num) && (0 != strncmp("", tmp_result_array, col_num))) {
        if ((long int)strlen(tmp_result_array) <= tmp_match_case_value) {
            status |= BASE_VALIDATION_SUCCEEDED;
            // erase BASE_VALIDATION_FAILED (default value)
            status = status & (~BASE_VALIDATION_FAILED);
        } else {
            status |= BASE_VALIDATION_FAILED;
            break;
        }
        i++;
        tmp_result_array = (char*)result_array + (i * col_num);
    }

    return status;
}

base_status_t array_validate_interval_lower_bound(module_index_t module_index, int obj_index,
                                                  int comb_index, int parser_index,
                                                  char *input_str, base_rc_t rc_flag,
                                                  void *result_array, unsigned int row_num, unsigned int col_num)
{
    // 1 means it is lower bound
    return array_validate_interval_helper(1, module_index, obj_index,
                                          comb_index, parser_index,
                                          input_str, rc_flag,
                                          result_array, row_num, col_num);
}

base_status_t array_validate_interval_upper_bound(module_index_t module_index, int obj_index, int comb_index, int parser_index,
        char *input_str, base_rc_t rc_flag, void *result_array, unsigned int row_num, unsigned int col_num)
{
    // 0 means it is upper bound
    return array_validate_interval_helper(0, module_index, obj_index,
                                          comb_index, parser_index,
                                          input_str, rc_flag,
                                          result_array, row_num, col_num);
}

static base_status_t array_validate_interval_helper(int lower, module_index_t module_index, int obj_index,
                                                    int comb_index, int parser_index,
                                                    char *input_str, base_rc_t rc_flag,
                                                    void *result_array, unsigned int row_num, unsigned int col_num)
{
    (void)input_str;

    base_status_t status = BASE_VALIDATION_SUCCEEDED;
    unsigned int i = 0;
    sys_call_spec_t tmp = assemble_sys_call_structure(module_index, obj_index, rc_flag);
    // can return structure filled with 0s
    long int tmp_match_case_value = 0;
    long int tmp_array_element_value = 0;
    char *tmp_result_array = NULL;

    base_print_debug(DEBUG_MSG_FUN_CALLED, __func__);

    // take match case string value as long int
    tmp_match_case_value = strtol(tmp.parse_cb[comb_index][parser_index].match_case, NULL, 10);
    tmp_result_array = result_array;
    while ((i < row_num) && (0 != strncmp("", tmp_result_array, col_num))) {
        // take array element value as long int
        tmp_array_element_value = strtol(tmp_result_array, NULL, 10);
        if (lower) {
            // for lower bound
            if (tmp_array_element_value < tmp_match_case_value) {
                status |= BASE_VALIDATION_FAILED;
                break;
            }
        } else {
            if (tmp_match_case_value < tmp_array_element_value) {
                status |= BASE_VALIDATION_FAILED;
                break;
            }
        }
        i++;
        tmp_result_array = (char*)result_array + (i * col_num);
    }

    return status;
}
