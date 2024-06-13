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
/**
 * @file base_private_parsers.c
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
#include <stdlib.h>

/* Helper functions for parsers */
static base_status_t find_on_line_helper(module_index_t module_index, int obj_index,
                                         int comb_index, int parser_index,
                                         char *input_str, base_rc_t rc_flag);
static base_status_t base_set_result_array(void *result_array, unsigned int row_num, unsigned int col_num,
                                           char *sub_str);
static void base_store_and_reset_array(void *result_array, unsigned int row_num, unsigned int col_num,
                                       void *old_array);
static void base_store_and_reset_array_element(void *result_array, unsigned int col_num,
                                               void *old_array, int index);
static int base_check_and_set_match_case(char match_case[MAX_STR_LEN],
                                         char *input_str, char** sub_str);
static base_status_t base_check_mask_and_set_result_array(void *result_array, unsigned int row_num, unsigned int col_num,
                                                          char match_case[MAX_STR_LEN], base_parser_op_t op, int base);
static int is_string_valid_hex_or_int(long int value, char *string);
static base_status_t custom_type_array_until_match_case(module_index_t module_index, int obj_index,
                                                        int comb_index, int parser_index, base_rc_t rc_flag,
                                                        void *result_array, unsigned int row_num, unsigned int col_num, void *old_array);
/* end of helper declarations */

base_status_t find_on_line(module_index_t module_index, int obj_index,
                           int comb_index, int parser_index,
                           char *input_str, base_rc_t rc_flag,
                           void *result_array, unsigned int row_num, unsigned int col_num)
{
    (void)result_array;
    (void)row_num;
    (void)col_num;

    base_status_t status;

    base_print_debug(DEBUG_MSG_FUN_CALLED, __func__);

    status = find_on_line_helper(module_index, obj_index, comb_index,
                                 parser_index, input_str, rc_flag);
    return status;
}

base_status_t is_not_on_line(module_index_t module_index, int obj_index,
                             int comb_index, int parser_index,
                             char *input_str, base_rc_t rc_flag,
                             void *result_array, unsigned int row_num, unsigned int col_num)
{
    (void)result_array;
    (void)row_num;
    (void)col_num;

    base_status_t status = BASE_PARSING_SUCCEEDED;

    base_print_debug(DEBUG_MSG_FUN_CALLED, __func__);
    if (BASE_PARSING_SUCCEEDED == find_on_line_helper(module_index, obj_index,
                                                      comb_index, parser_index,
                                                      input_str, rc_flag)) {
        status = BASE_PARSING_FAILED;
    }
    return status;
}

base_status_t find_on_line_and_fill_result_array(module_index_t module_index, int obj_index,
                                                 int comb_index, int parser_index,
                                                 char *input_str, base_rc_t rc_flag,
                                                 void *result_array, unsigned int row_num, unsigned int col_num)
{
    (void)row_num;

    base_status_t status;
    sys_call_spec_t tmp;
    base_print_debug(DEBUG_MSG_FUN_CALLED, __func__);
    status = find_on_line_helper(module_index, obj_index, comb_index, parser_index, input_str, rc_flag);
    if (BASE_PARSING_SUCCEEDED == status) {
        tmp = assemble_sys_call_structure(module_index, obj_index, rc_flag);
        strncpy((char*)result_array, tmp.parse_cb[comb_index][parser_index].match_case, col_num);
    }
    return status;
}

base_status_t find_and_get_next_word(module_index_t module_index, int obj_index,
                                     int comb_index, int parser_index,
                                     char *input_str, base_rc_t rc_flag,
                                     void *result_array, unsigned int row_num, unsigned int col_num)
{
    base_status_t status = BASE_PARSING_FAILED;
    char *sub_str = NULL;
    sys_call_spec_t tmp = assemble_sys_call_structure(module_index, obj_index, rc_flag);

    base_print_debug(DEBUG_MSG_FUN_CALLED, __func__);

    if (EXIT_FAILURE == base_check_and_set_match_case(tmp.parse_cb[comb_index][parser_index].match_case,
                                                      input_str, &sub_str)) {
        return status;
    }

    if (NULL != sub_str) {
        /* take next word as value */
        sub_str = (sub_str + strlen(tmp.parse_cb[comb_index][parser_index].match_case));
        status = base_set_result_array(result_array, row_num, col_num, sub_str);
    }

    return status;
}

base_status_t find_and_get_previous_word(module_index_t module_index, int obj_index,
                                         int comb_index, int parser_index,
                                         char *input_str, base_rc_t rc_flag,
                                         void *result_array, unsigned int row_num, unsigned int col_num)
{
    base_status_t status = BASE_PARSING_FAILED;
    char *sub_str = NULL;
    sys_call_spec_t tmp = assemble_sys_call_structure(module_index, obj_index, rc_flag);

    base_print_debug(DEBUG_MSG_FUN_CALLED, __func__);
    do {
        if (EXIT_FAILURE == base_check_and_set_match_case(tmp.parse_cb[comb_index][parser_index].match_case,
                                                          input_str, &sub_str)) {
            break;
        }

        if (NULL != sub_str) {
            /* take previous word as value */
            sub_str--; // go back one place - delimiter between match case and previous word
            if (NULL != sub_str) {
                if (!(isspace(*sub_str)) && ('\t' != *sub_str)) {
                    // tab or space is expected as word delimiter
                    base_print_debug(DEBUG_MSG_FUN_NAME_AND_MSG, __func__,
                                     "delimiter is invalid");
                    break;
                }
            } else {
                base_print_debug(DEBUG_MSG_FUN_NAME_AND_MSG, __func__,
                                 "delimiter is invalid (NULL)");
                break;
            }
            while (NULL != sub_str) {
                // now, go back until next tab or space - previous word
                sub_str--;
                if (isspace(*sub_str) || ('\t' == *sub_str)) {
                    // tab or space is expected as word delimiter
                    break; // right place
                }
            }
            if (NULL == sub_str) {
                // in case of invalid string
                base_print_debug(DEBUG_MSG_FUN_NAME_AND_MSG, __func__,
                                 "input string is invalid at beginning");
                break;
            }
            /* after we changed location, take next word as value */
            status = base_set_result_array(result_array, row_num, col_num, sub_str);
        } else {
            base_print_debug(DEBUG_MSG_FUN_NAME_AND_MSG, __func__,
                             "input string is invalid");
        }
    } while (0);

    return status;
}

base_status_t get_first_word(module_index_t module_index, int obj_index,
                             int comb_index, int parser_index,
                             char *input_str, base_rc_t rc_flag,
                             void *result_array, unsigned int row_num, unsigned int col_num)
{
    (void)module_index;
    (void)obj_index;
    (void)comb_index;
    (void)parser_index;
    (void)rc_flag;

    base_status_t status;

    base_print_debug(DEBUG_MSG_FUN_CALLED, __func__);
    status = base_set_result_array(result_array, row_num, col_num, input_str);
    return status;
}

base_status_t parse_dec_values_separated_with_spaces(module_index_t module_index, int obj_index,
                                                     int comb_index, int parser_index,
                                                     char *input_str, base_rc_t rc_flag,
                                                     void *result_array, unsigned int row_num, unsigned int col_num)
{
    (void)module_index;
    (void)obj_index;
    (void)comb_index;
    (void)parser_index;
    (void)rc_flag;

    base_status_t status = BASE_PARSING_FAILED;
    char *sub_str = NULL;
    char *tmp_input_str = NULL;
    char *tmp_result_array = NULL;
    int input_flag = 1, bytes_read;
    unsigned int number_value, i = 0;

    base_print_debug(DEBUG_MSG_FUN_CALLED, __func__);
    do {
        if ('\0' == *input_str) {
            // first char in string
            base_print_debug(DEBUG_MSG_FUN_NAME_AND_MSG, __func__,
                             "input string is empty");
            break;
        }
        tmp_result_array = (char*)result_array;
        // only spaces and digits are allowed
        tmp_input_str = input_str;
        while (*tmp_input_str != '\0') {
            if ((!isdigit(*tmp_input_str)) &&
                (!isspace(*tmp_input_str))) {
                *tmp_input_str = '\0';
                *input_str = '\0';
                input_flag = 0; // break main do while
                break; // invalid input string
            }
            tmp_input_str++;
        }
        if (0 == input_flag) {
            base_print_debug(DEBUG_MSG_FUN_NAME_AND_MSG, __func__,
                             "invalid input string");
            break; // invalid input string
        }
        // Remove leading spaces
        sub_str = input_str + strspn(input_str, " ");
        // parse trimed string
        while ((i < row_num) &&
               (sscanf(sub_str, "%u%n", &number_value, &bytes_read) > 0)) {
            if ((10 <= bytes_read) && (0 == number_value)) {
                // UINT_MAX length in chars is 10 ===> wrong value is scaned
                input_flag = 0; // signal to break main do while
                break; // invalid input string
            }

            snprintf(tmp_result_array, col_num, "%u", number_value);
            // return value back (as string) in array
            sub_str += bytes_read;
            while (isspace(*sub_str) && (*sub_str != '\0')) {
                sub_str++;
            }
            i++;
            tmp_result_array = (char*)result_array + (i * col_num);
        }
        if (1 == input_flag) {
            // valid
            status = BASE_PARSING_SUCCEEDED;
            // if this line is reached, parsing went well
        }
    } while (0);

    return status;
}

base_status_t get_all_lines(module_index_t module_index, int obj_index,
                            int comb_index, int parser_index,
                            char *input_str, base_rc_t rc_flag,
                            void *result_array, unsigned int row_num, unsigned int col_num)
{
    (void)module_index;
    (void)obj_index;
    (void)comb_index;
    (void)parser_index;
    (void)rc_flag;

    char *tmp_result_array = NULL;
    char *parsed_value = NULL;
    unsigned int i = 0;
    long int pos = 0;

    base_print_debug(DEBUG_MSG_FUN_CALLED, __func__);

    pos = strspn(input_str, " \t");
    parsed_value = input_str + pos;
    pos = strcspn(parsed_value, "\n");
    *(parsed_value + pos) = '\0';

    tmp_result_array = (char*)result_array;

    while ((i < row_num) && (0 != strncmp("", tmp_result_array, col_num))) {
        i++;
        tmp_result_array = (char*)result_array + (i * col_num);
    }

    strncpy(tmp_result_array, parsed_value, col_num);

    return BASE_PARSING_SUCCEEDED;
}

/* NEXT PARSERS WORKS ON STRING ARRAY
 * THESE PARSERS CAN BE CALLED ONLY AFTER PARSERS WHICH FILL RESULT ARRAY
 */
base_status_t array_take_value_after(module_index_t module_index, int obj_index,
                                     int comb_index, int parser_index,
                                     char *input_str, base_rc_t rc_flag,
                                     void *result_array, unsigned int row_num, unsigned int col_num)
{
    (void)input_str;

    base_status_t status = BASE_PARSING_FAILED;
    char *sub_str = NULL;
    char *parsed_value = NULL;
    char tmp_array_el[BASE_MAX_COL_NUM]; // maximal memory piece
    char *tmp_result_array = NULL;
    unsigned int i = 0;
    sys_call_spec_t tmp = assemble_sys_call_structure(module_index, obj_index, rc_flag);

    base_print_debug(DEBUG_MSG_FUN_CALLED, __func__);

    do {
        tmp_result_array = (char*)result_array;
        while ((i < row_num) && (0 != strncmp("", tmp_result_array, 1))) {
            base_store_and_reset_array_element(result_array, col_num, tmp_array_el, i);
            if ((EXIT_SUCCESS == base_check_and_set_match_case(tmp.parse_cb[comb_index][parser_index].match_case,
                                                               tmp_array_el, &sub_str)) &&
                (NULL != sub_str)) {
                /* take next string as value*/
                sub_str = (sub_str + strlen(tmp.parse_cb[comb_index][parser_index].match_case));
                // Remove leading spaces or punctuation
                while (isspace(*sub_str) || ispunct(*sub_str)) {
                    sub_str++;
                }
                parsed_value = sub_str; // save begin of the string
                // Remove trailing spaces and punctuation
                while (!(isspace(*sub_str)) &&
                       (!ispunct(*sub_str)) &&
                       ((*sub_str) != '\0')) {
                    sub_str++;
                }
                *sub_str = '\0'; // terminate string at position where we found space or punctuation
                sub_str = parsed_value; // return begin of the string
                if (1 <= strlen(sub_str)) {
                    strncpy(tmp_result_array, sub_str, col_num); // to include '\0'
                    status = BASE_PARSING_SUCCEEDED;
                }
            }
            i++;
            tmp_result_array = (char*)result_array + (i * col_num);
        }
    } while (0);

    return status;
}

base_status_t array_drop_punctuation(module_index_t module_index, int obj_index,
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

    base_status_t status = BASE_PARSING_FAILED;
    char *tmp_iterator = NULL;
    char *arr_el_iterator = NULL;
    char *tmp_result_array = NULL;
    char tmp_array_el[BASE_MAX_COL_NUM]; // maximal memory piece
    unsigned int i = 0;

    base_print_debug(DEBUG_MSG_FUN_CALLED, __func__);
    do {
        tmp_result_array = (char*)result_array;
        while ((i < row_num) && (0 != strncmp("", tmp_result_array, 1))) {
            base_store_and_reset_array_element(result_array, col_num, tmp_array_el, i);
            arr_el_iterator = tmp_result_array;
            tmp_iterator = tmp_array_el;

            if (NULL != tmp_iterator) {
                while ((*tmp_iterator) != '\0') {
                    if (ispunct(*tmp_iterator)) {
                        tmp_iterator++;
                    } else {
                        memcpy(arr_el_iterator, tmp_iterator, 1);
                        arr_el_iterator++;
                        tmp_iterator++;
                    }
                }
                *arr_el_iterator = '\0'; // last char is terminator
                if ((NULL != tmp_result_array) &&
                    (1 <= strlen(tmp_result_array))) {
                    status = BASE_PARSING_SUCCEEDED;
                }
            }
            i++;
            tmp_result_array = (char*)result_array + (i * col_num);
        }
    } while (0);

    return status;
}

base_status_t array_delete_element(module_index_t module_index, int obj_index,
                                   int comb_index, int parser_index,
                                   char *input_str, base_rc_t rc_flag,
                                   void *result_array, unsigned int row_num, unsigned int col_num)
{
    (void)input_str;

    unsigned int i = 0;
    int new_array_index = 0;
    char *tmp_resault_array = NULL;
    char old_array[BASE_MAX_ROW_NUM][BASE_MAX_COL_NUM]; // maximal memory piece
    char *tmp_str = NULL;
    sys_call_spec_t tmp = assemble_sys_call_structure(module_index, obj_index, rc_flag);

    base_print_debug(DEBUG_MSG_FUN_CALLED, __func__);
    base_store_and_reset_array(result_array, row_num, col_num, old_array);
    tmp_str = (char*)old_array + (i * col_num);
    while ((i< row_num) && (0 != strncmp(tmp_str, "", col_num))) {
        // if it is same skip it
        if (0 != strncmp(tmp.parse_cb[comb_index][parser_index].match_case, tmp_str, col_num)) {
            tmp_resault_array = (char*)result_array + (new_array_index * col_num);
            strncpy(tmp_resault_array, tmp_str, col_num);
            new_array_index++;
        }
        i++;
        tmp_str = (char*)old_array + (i * col_num);
    }

    return BASE_PARSING_SUCCEEDED;
}

base_status_t array_bitwise_AND_mask(module_index_t module_index, int obj_index,
                                     int comb_index, int parser_index,
                                     char *input_str, base_rc_t rc_flag,
                                     void *result_array, unsigned int row_num, unsigned int col_num)
{
    (void)input_str;

    base_status_t status;
    base_parser_op_t op = BASE_AND_OP;
    sys_call_spec_t tmp = assemble_sys_call_structure(module_index, obj_index, rc_flag);

    base_print_debug(DEBUG_MSG_FUN_CALLED, __func__);
    status = base_check_mask_and_set_result_array(result_array, row_num, col_num,
                                                  tmp.parse_cb[comb_index][parser_index].match_case, op, 16);
    return status;
}

base_status_t array_multiply_long_int_with(module_index_t module_index, int obj_index,
                                           int comb_index, int parser_index,
                                           char *input_str, base_rc_t rc_flag,
                                           void *result_array, unsigned int row_num, unsigned int col_num)
{
    (void)input_str;

    base_status_t status;
    base_parser_op_t op = BASE_MULTIPLY_OP;
    sys_call_spec_t tmp = assemble_sys_call_structure(module_index, obj_index, rc_flag);

    base_print_debug(DEBUG_MSG_FUN_CALLED, __func__);
    status = base_check_mask_and_set_result_array(result_array, row_num, col_num,
                                                  tmp.parse_cb[comb_index][parser_index].match_case, op, 10);
    return status;
}

base_status_t array_shift_right(module_index_t module_index, int obj_index,
                                int comb_index, int parser_index,
                                char *input_str, base_rc_t rc_flag,
                                void *result_array, unsigned int row_num, unsigned int col_num)
{
    (void)input_str;

    base_status_t status;
    base_parser_op_t op = BASE_SHIFT_RIGHT_OP;
    sys_call_spec_t tmp = assemble_sys_call_structure(module_index, obj_index, rc_flag);

    base_print_debug(DEBUG_MSG_FUN_CALLED, __func__);
    status = base_check_mask_and_set_result_array(result_array, row_num, col_num,
                                                  tmp.parse_cb[comb_index][parser_index].match_case, op, 16);
    return status;
}

base_status_t array_convert_hex_string_to_long_int(module_index_t module_index, int obj_index,
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

    base_status_t status;
    base_parser_op_t op = BASE_CONVERT_HEX_TO_LONG_INT;

    base_print_debug(DEBUG_MSG_FUN_CALLED, __func__);
    status = base_check_mask_and_set_result_array(result_array, row_num, col_num, NULL, op, 16);
    return status;
}

base_status_t array_shrink_string(module_index_t module_index, int obj_index,
                                  int comb_index, int parser_index,
                                  char *input_str, base_rc_t rc_flag,
                                  void *result_array, unsigned int row_num, unsigned int col_num)
{
    base_status_t status = BASE_PARSING_FAILED;
    unsigned int i = 0;
    sys_call_spec_t tmp = assemble_sys_call_structure(module_index, obj_index, rc_flag);
    char *tmp_result_array = NULL;
    long int tmp_match_case_value = 0;
    (void)input_str;

    base_print_debug(DEBUG_MSG_FUN_CALLED, __func__);
    do {
        if (0 == strncmp("",tmp.parse_cb[comb_index][parser_index].match_case, 1)) {
            base_print_debug(DEBUG_MSG_FUN_NAME_AND_MSG, __func__, "match case string is empty");
            break;
        }

        tmp_result_array = (char*)result_array;
        // take match case string value as long int
        tmp_match_case_value = strtol(tmp.parse_cb[comb_index][parser_index].match_case, NULL, 10);

        if (tmp_match_case_value <= col_num) {
            while ((i < row_num) && (0 != strncmp("", tmp_result_array, 1))) {
                tmp_result_array = tmp_result_array + (tmp_match_case_value);
                *tmp_result_array = '\0'; // end it here (arrays are starting from index 0)
                i++;
                tmp_result_array = (char*)result_array + (i * col_num);
            }
            if (1 <= i) {
                // at least one string is processed
                status = BASE_PARSING_SUCCEEDED;
            }
        }
    } while (0);

    return status;
}

base_status_t array_uniq_elements(module_index_t module_index, int obj_index,
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

    int status = BASE_PARSING_SUCCEEDED;
    char old_array[BASE_MAX_ROW_NUM][BASE_MAX_COL_NUM]; // maximal memory piece
    char *tmp_str = NULL;
    char *tmp_sub_str = NULL;
    char *tmp_result_array = NULL;
    unsigned int i = 0;
    unsigned int j = 0;
    unsigned int col_index = 0;

    base_print_debug(DEBUG_MSG_FUN_CALLED, __func__);
    base_store_and_reset_array(result_array, row_num, col_num, old_array);
    tmp_str = (char*)old_array + (i * col_num);
    while ((i < row_num) && (0 != strncmp(tmp_str, "", col_num))) {
        for (j = 0 ; j < i; j++) {
            tmp_sub_str = (char*)old_array + (j * col_num);
            if (0 == strncmp(tmp_str, tmp_sub_str, col_num)) {
                break;
            }
        }
        if (j == i) {
            tmp_result_array = (char*)result_array + (col_index * col_num);
            snprintf(tmp_result_array, col_num, "%s", tmp_str);
            col_index++;
        }
        i++;
        tmp_str = (char*)old_array + (i * col_num);
    }

    return status;
}

base_status_t array_count_elements(module_index_t module_index, int obj_index,
                                   int comb_index, int parser_index,
                                   char *input_str, base_rc_t rc_flag,
                                   void *result_array, unsigned int row_num, unsigned int col_num)
{
    int status = BASE_PARSING_SUCCEEDED;
    char old_array[BASE_MAX_ROW_NUM][BASE_MAX_COL_NUM]; // maximal memory piece
    char *tmp_str = NULL;
    unsigned int i = 0;
    (void)module_index;
    (void)obj_index;
    (void)comb_index;
    (void)parser_index;
    (void)input_str;
    (void)rc_flag;

    base_print_debug(DEBUG_MSG_FUN_CALLED, __func__);
    base_store_and_reset_array(result_array, row_num, col_num, old_array);
    tmp_str = (char*)old_array + (i * col_num);
    while ((i < row_num) && (0 != strncmp(tmp_str, "", col_num))) {
        // iteratio is with original row/col size
        i++;
        tmp_str = (char*)old_array + (i * col_num);
    }
    snprintf(result_array, col_num, "%u", i);
    // only first string in result array

    return status;
}

base_status_t array_until_match_case(module_index_t module_index, int obj_index,
                                     int comb_index, int parser_index,
                                     char *input_str, base_rc_t rc_flag,
                                     void *result_array, unsigned int row_num, unsigned int col_num)
{
    int status = BASE_PARSING_SUCCEEDED;
    char interface_old_array[MAX_NUM_OF_INTERFACES][MAX_STR_LEN];
    char vlan_intervals_old_array[BASE_MAX_VID][MAX_STR_LEN];
    char vlan_interval_old_array[BASE_MAX_VID][BASE_TEN];
    char vlan_old_array[BASE_MAX_VID][BASE_FIVE];
    (void)input_str;

    base_print_debug(DEBUG_MSG_FUN_CALLED, __func__);

    if ((MAX_NUM_OF_INTERFACES == row_num) && (MAX_STR_LEN == col_num)) {
        status = custom_type_array_until_match_case(module_index, obj_index, comb_index,
                parser_index, rc_flag, result_array, row_num, col_num, interface_old_array);
    }
    else if ((BASE_MAX_VID == row_num) && (BASE_TEN == col_num)) {
        status = custom_type_array_until_match_case(module_index, obj_index, comb_index,
                parser_index, rc_flag, result_array, row_num, col_num, vlan_interval_old_array);
    }
    else if ((BASE_MAX_VID == row_num) && (MAX_STR_LEN == col_num)) {
        status = custom_type_array_until_match_case(module_index, obj_index, comb_index,
                parser_index, rc_flag, result_array, row_num, col_num, vlan_intervals_old_array);
    }
    else if ((BASE_MAX_VID == row_num) && (BASE_FIVE == col_num)) {
        status = custom_type_array_until_match_case(module_index, obj_index, comb_index,
                parser_index, rc_flag, result_array, row_num, col_num, vlan_old_array);
    } else {
        base_print_warning(ERROR_MSG_FUN_AND_MSG, __func__, "unsupported matrix size");
        status = BASE_PARSING_FAILED;
    }

    return status;
}

base_status_t array_take_values_after_match_case(module_index_t module_index, int obj_index,
                                                 int comb_index, int parser_index,
                                                 char *input_str, base_rc_t rc_flag,
                                                 void *result_array, unsigned int row_num, unsigned int col_num)
{
    (void)input_str;

    char old_array[BASE_MAX_ROW_NUM][BASE_MAX_COL_NUM];
    char *tmp_result_array = NULL;
    char *tmp_str = NULL;
    unsigned int i = 0;
    unsigned int j = 0;
    int match_case_flag = 0;
    sys_call_spec_t tmp = assemble_sys_call_structure(module_index, obj_index, rc_flag);

    base_print_debug(DEBUG_MSG_FUN_CALLED, __func__);

    base_store_and_reset_array(result_array, row_num, col_num, old_array);
    tmp_str = (char*)old_array + (i * col_num);

    do {
        while ((i < row_num) && (0 != strncmp(tmp_str, "", col_num))) {
            if ((0 == match_case_flag) && (NULL != strstr(tmp_str, tmp.parse_cb[comb_index][parser_index].match_case))) {
                match_case_flag = 1;
            }
            i++;
            tmp_str = (char*)old_array + (i * col_num);
            if (1 == match_case_flag) {
                tmp_result_array = (char*)result_array + (j * col_num);
                snprintf(tmp_result_array, col_num, "%s", tmp_str);
                j++;
            }
        }
    } while (0);

    return BASE_PARSING_SUCCEEDED;
}

base_status_t array_take_values_before_match_case(module_index_t module_index, int obj_index,
                                                  int comb_index, int parser_index,
                                                  char *input_str, base_rc_t rc_flag,
                                                  void *result_array, unsigned int row_num, unsigned int col_num)
{
    (void)input_str;

    char old_array[BASE_MAX_ROW_NUM][BASE_MAX_COL_NUM];
    char *tmp_result_array = NULL;
    unsigned int i = 0;
    char *tmp_str = NULL;
    sys_call_spec_t tmp = assemble_sys_call_structure(module_index, obj_index, rc_flag);

    base_print_debug(DEBUG_MSG_FUN_CALLED, __func__);

    base_store_and_reset_array(result_array, row_num, col_num, old_array);
    tmp_str = (char*)old_array + (i * col_num);

    do {
        while ((i < row_num) && (0 != strncmp(tmp_str, "", col_num))) {
            if (NULL != strstr(tmp_str, tmp.parse_cb[comb_index][parser_index].match_case)) {
                break;
            }
            tmp_result_array = (char*)result_array + (i * col_num);
            snprintf(tmp_result_array, col_num, "%s", tmp_str);
            i++;
            tmp_str = (char*)old_array + (i * col_num);
        }
    } while (0);

    return BASE_PARSING_SUCCEEDED;
}

base_status_t array_write_to_file(module_index_t module_index, int obj_index,
                                  int comb_index, int parser_index,
                                  char *input_str, base_rc_t rc_flag,
                                  void *result_array, unsigned int row_num, unsigned int col_num)
{
    (void)input_str;

    unsigned int i = 0;
    char *tmp_str = NULL;
    char file_path[MAX_STR_LEN];
    FILE *fp = NULL;
    sys_call_spec_t tmp = assemble_sys_call_structure(module_index, obj_index, rc_flag);

    base_print_debug(DEBUG_MSG_FUN_CALLED, __func__);

    snprintf(file_path, MAX_STR_LEN, PORT_ADVERTISE_FULL_PATH,
             tmp.parse_cb[comb_index][parser_index].match_case);

    fp = fopen(file_path, "w");
    if (!fp) {
        return BASE_PARSING_FAILED;
    }

    tmp_str = (char*)result_array + (i * col_num);
    while ((i < row_num) && (0 != strncmp(tmp_str, "", col_num))) {
        fprintf(fp, "%s", tmp_str);
        i++;
        tmp_str = (char*)result_array + (i * col_num);
    }

    fclose(fp);
    return BASE_PARSING_SUCCEEDED;
}

/* HELPER FUNCTIONS FOR PARSERS */

static base_status_t find_on_line_helper(module_index_t module_index, int obj_index,
                                         int comb_index, int parser_index,
                                         char *input_str, base_rc_t rc_flag)
{
    base_status_t status = BASE_PARSING_FAILED;
    char *sub_str = NULL;
    sys_call_spec_t tmp = assemble_sys_call_structure(module_index, obj_index, rc_flag);

    base_print_debug(DEBUG_MSG_FUN_CALLED, __func__);

    do {
        if (EXIT_FAILURE == base_check_and_set_match_case(tmp.parse_cb[comb_index][parser_index].match_case, input_str, &sub_str)) {
            break;
        }
        if ((NULL != sub_str) && (1 <= strlen(sub_str))) {
            status = BASE_PARSING_SUCCEEDED;
        }
    } while (0);

    return status;
}

static base_status_t base_set_result_array(void *result_array, unsigned int row_num, unsigned int col_num,
                                           char *sub_str)
{
    base_status_t status = BASE_PARSING_FAILED;
    char *parsed_value = NULL;
    char *tmp_result_array = NULL;
    long int pos = 0;
    unsigned int i = 0;

    base_print_debug(DEBUG_MSG_FUN_CALLED, __func__);

    do {
        /* remove leading and trailing spaces */
        pos = strspn(sub_str, " \t");
        parsed_value = sub_str + pos;
        pos = strcspn(parsed_value, " \t\n");
        *(parsed_value + pos) = '\0';
        tmp_result_array = (char*)result_array;

        while ((i < row_num) &&
               (0 != strncmp("", tmp_result_array, col_num))) {
            i++;
            tmp_result_array = (char*)result_array + (i * col_num);
        }

        if (1 <= strlen(parsed_value)) {
            strncpy(tmp_result_array, parsed_value, col_num);
            status = BASE_PARSING_SUCCEEDED;
        }
    } while (0);

    return status;
}

static void base_store_and_reset_array(void *result_array, unsigned int row_num, unsigned int col_num,
                                       void *old_array)
{
    base_print_debug(DEBUG_MSG_FUN_CALLED, __func__);

    memcpy(old_array, result_array, row_num * col_num);
    memset(result_array, 0, row_num * col_num);
}

static void base_store_and_reset_array_element(void *result_array, unsigned int col_num,
                                               void *old_array, int index)
{
    base_print_debug(DEBUG_MSG_FUN_CALLED, __func__);

    strncpy((char*)old_array, ((char*)result_array + (index * col_num)), col_num);
    memset(((char*)result_array + (index * col_num)), 0, col_num);
}

static int base_check_and_set_match_case(char match_case[MAX_STR_LEN], char *input_str, char** sub_str)
{
    int ret_val = EXIT_SUCCESS;

    base_print_debug(DEBUG_MSG_FUN_CALLED, __func__);

    if ('\0' == match_case[0]) {
        base_print_debug(DEBUG_MSG_FUN_NAME_AND_MSG, __func__, "match case string is empty.");
        return EXIT_FAILURE;
    }
    *sub_str = strstr(input_str, match_case);

    return ret_val;
}

static base_status_t base_check_mask_and_set_result_array(void *result_array, unsigned int row_num, unsigned int col_num,
                                                          char match_case[MAX_STR_LEN], base_parser_op_t op, int base)
{
    base_status_t status = BASE_PARSING_FAILED;
    long int tmp_array_el_value = 0;
    long int tmp_match_case_value = 0;
    char *tmp_result_array = NULL;
    unsigned int i = 0, error_flag = 0;

    base_print_debug(DEBUG_MSG_FUN_CALLED, __func__);

    switch (op) {
    /* if operation is 'erase' the match_case can be NULL, so we do not
     * need the check of the match_case
     */
    case BASE_ERASE_OP:
    case BASE_CONVERT_HEX_TO_LONG_INT:// no need for match case
        break;
    case BASE_AND_OP:
    case BASE_MULTIPLY_OP:
    case BASE_SHIFT_RIGHT_OP:
        if ((NULL == match_case) || ('\0' == match_case[0])) {
            // if operation is not 'erase' we need to check match_case value
            base_print_debug(DEBUG_MSG_FUN_NAME_AND_MSG, __func__, "match case string is empty");
            error_flag++;
            break;
        }
        // take match case string value as long int
        tmp_match_case_value = strtol(match_case, NULL, base);
        if (EXIT_FAILURE == is_string_valid_hex_or_int(tmp_match_case_value, match_case)) {
            base_print_debug(DEBUG_MSG_FUN_NAME_AND_MSG, __func__, "match case string is invalid");
            error_flag++;
            break;
        }
        break;
    default:
        base_print_debug(DEBUG_MSG_FUN_NAME_AND_MSG, __func__, "unpredicted op.");
        error_flag++;
        break;
    }

    tmp_result_array = (char*)result_array;
    while ((i < row_num) &&
           (NULL != tmp_result_array) &&
           (0 != strncmp("", tmp_result_array, col_num)) &&
           (0 == error_flag)) {
        // take array string value as long int
        tmp_array_el_value = strtol(tmp_result_array, NULL, base);
        switch (op) {
        case BASE_AND_OP:
            if (EXIT_FAILURE == is_string_valid_hex_or_int(tmp_array_el_value, tmp_result_array)) {
                base_print_debug(DEBUG_MSG_FUN_NAME_AND_MSG, __func__, "match case string is invalid");
                error_flag++;
                break;
            }
            tmp_array_el_value = tmp_array_el_value & tmp_match_case_value;
            break;
        case BASE_MULTIPLY_OP:
            if (EXIT_FAILURE == is_string_valid_hex_or_int(tmp_array_el_value, tmp_result_array)) {
                base_print_debug(DEBUG_MSG_FUN_NAME_AND_MSG, __func__, "match case string is invalid");
                error_flag++;
                break;
            }
            tmp_array_el_value = tmp_array_el_value * tmp_match_case_value;
            break;
        case BASE_SHIFT_RIGHT_OP:
            if (EXIT_FAILURE == is_string_valid_hex_or_int(tmp_array_el_value, tmp_result_array)) {
                base_print_debug(DEBUG_MSG_FUN_NAME_AND_MSG, __func__, "match case string is invalid");
                error_flag++;
                break;
            }
            tmp_array_el_value = tmp_array_el_value >> tmp_match_case_value;
            break;
        case BASE_CONVERT_HEX_TO_LONG_INT:
            if (EXIT_FAILURE == is_string_valid_hex_or_int(tmp_array_el_value, tmp_result_array)) {
                base_print_debug(DEBUG_MSG_FUN_NAME_AND_MSG, __func__, "match case string is invalid");
                error_flag++;
                break;
            }
            tmp_array_el_value = tmp_array_el_value >> tmp_match_case_value;
            break;
        case BASE_ERASE_OP:
            memset(tmp_result_array, 0, col_num);
            break;
        default:
            base_print_debug("%s(): invalid parse operation (%d)!", __func__, op);
            error_flag++;
            break;
        }

        if (0 == error_flag) {
            snprintf(tmp_result_array, col_num, "%ld", tmp_array_el_value);
            // return value back (as string) in array
            if (1 <= strlen(tmp_result_array)) {
                status = BASE_PARSING_SUCCEEDED;
                i++;
                tmp_result_array = (char*)result_array + (i * col_num);
            } else {
                status = BASE_PARSING_FAILED;
            }
        }
    }

    return status;
}

static int is_string_valid_hex_or_int(long int value, char *string)
{
    int status = EXIT_SUCCESS;
    char *tmp_char = NULL;

    if (0 == value) {
        // if translated value is 0, there is chance that string does not represents valid hex/int value
        tmp_char = string; // beginning of match_case
                           // check does string represents valid hex/int value
        while ('\0' != *tmp_char) {
            if ( (('0' <= *tmp_char) && ('9' >= *tmp_char)) ||
                 (('a' <= *tmp_char) && ('f' >= *tmp_char)) ||
                 (('A' <= *tmp_char) && ('F' >= *tmp_char))) {
                tmp_char++;
            } else {
                status = EXIT_FAILURE;
                break;
            }
        }
    }
    return status;
}

static base_status_t custom_type_array_until_match_case(module_index_t module_index, int obj_index,
                                                        int comb_index, int parser_index,
                                                        base_rc_t rc_flag, void *result_array,
                                                        unsigned int row_num, unsigned int col_num, void *old_array)
{
    int status = BASE_PARSING_SUCCEEDED;
    unsigned int i = 0;
    char *tmp_old_array = NULL;
    char *tmp_result_array = NULL;
    unsigned int pos = 0;
    sys_call_spec_t tmp = assemble_sys_call_structure(module_index, obj_index, rc_flag);

    base_print_debug(DEBUG_MSG_FUN_CALLED, __func__);

    base_store_and_reset_array(result_array, row_num, col_num, old_array);

    if (('\0' == tmp.parse_cb[comb_index][parser_index].match_case[0])) {
        base_print_debug(DEBUG_MSG_WITH_TWO_PARAM, __func__,
                         "match case string is empty");
        status = BASE_PARSING_FAILED;
    } else {
        tmp_old_array = (char*)old_array + (i * col_num); // for first loop iteration
        while (0 != strncmp(tmp_old_array, "", col_num)) {
            pos = strcspn(tmp_old_array, tmp.parse_cb[comb_index][parser_index].match_case);
            if (col_num > pos) {
                // otherwise it is \0 if size is same - end of the string
                *(tmp_old_array + pos) = '\0';
            }
            tmp_result_array = (char*)result_array + (i * col_num);
            snprintf(tmp_result_array, col_num, "%s", tmp_old_array); // save changed value
            i++; // for next loop iteration
            tmp_old_array = (char*)old_array + (i * col_num); // for next loop iteration
        }
    }

    return status;
}
