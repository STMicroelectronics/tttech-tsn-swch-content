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

#ifndef INCLUDE_BASE_PRIVATE_PARSERS_H_
#define INCLUDE_BASE_PRIVATE_PARSERS_H_

#include "base_private_combinators.h"
#include "base_private_defines.h"
#include "base_private_validators.h"
#include "base_private_syscalls.h"

typedef enum {
    BASE_AND_OP = 0,
    BASE_MULTIPLY_OP,
    BASE_SHIFT_RIGHT_OP,
    BASE_CONVERT_HEX_TO_LONG_INT,
    BASE_ERASE_OP,
} base_parser_op_t;

// Parser callbacks declaration

/**
 * @brief Checks if match case is on line and takes next word.
 *
 * @param[in]   module_index    Predefined index of the module (enum
 *                              module_index_t from base_defines.h).
 * @param[in]   obj_index       Predefined object index (function will work
 *                              only with object indices defined in
 *                              base_private_defines.h).
 * @param[in]   comb_index      Predefined combinator index (function will work
 *                              only with combinator indices defined in
 *                              [module]_combinators array in
 *                              base_private_[module]_configuration.c).
 * @param[in]   parser_index    Predefined parser index (function will work
 *                              only with parser indices defined in
 *                              [module]_num_of_cb array in
 *                              base_private_[module]_configuration.c).
 * @param[in]   input_str       Stream line passed by combinator (system call).
 * @param[in]   rc_flag         If it is BASE_RC_YES, system call structure can
 *                              be changed in runtime.
 * @param[out]  result_array    Place holder for value(s) of the object(s).
 *                              Caller should provide address of char matrix as
 *                              result_array.
 * @param[in]   row_num         Number of result_array's rows.
 * @param[in]   col_num         Number of result_array's columns.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
base_status_t find_and_get_next_word(module_index_t module_index, int obj_index,
                                     int comb_index, int parser_index,
                                     char *input_str, base_rc_t rc_flag,
                                     void *result_array, unsigned int row_num, unsigned int col_num);

/**
 * @brief Checks if match case is on line and takes previous word.
 *
 * @param[in]   module_index    Predefined index of the module (enum
 *                              module_index_t from base_defines.h).
 * @param[in]   obj_index       Predefined object index (function will work
 *                              only with object indices defined in
 *                              base_private_defines.h).
 * @param[in]   comb_index      Predefined combinator index (function will work
 *                              only with combinator indices defined in
 *                              [module]_combinators array in
 *                              base_private_[module]_configuration.c).
 * @param[in]   parser_index    Predefined parser index (function will work
 *                              only with parser indices defined in
 *                              [module]_num_of_cb array in
 *                              base_private_[module]_configuration.c).
 * @param[in]   input_str       Stream line passed by combinator (system call).
 * @param[in]   rc_flag         If it is BASE_RC_YES, system call structure can
 *                              be changed in runtime.
 * @param[out]  result_array    Place holder for value(s) of the object(s).
 *                              Caller should provide address of char matrix as
 *                              result_array.
 * @param[in]   row_num         Number of result_array's rows.
 * @param[in]   col_num         Number of result_array's columns.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
base_status_t find_and_get_previous_word(module_index_t module_index, int obj_index,
                                         int comb_index, int parser_index,
                                         char *input_str, base_rc_t rc_flag,
                                         void *result_array, unsigned int row_num, unsigned int col_num);

/**
 * @brief Takes first word on line (all characters untill first space).
 *
 * @param[in]   module_index    Predefined index of the module (enum
 *                              module_index_t from base_defines.h). Unused in
 *                              this function.
 * @param[in]   obj_index       Predefined object index (function will work
 *                              only with object indices defined in
 *                              base_private_defines.h). Unused in this
 *                              function.
 * @param[in]   comb_index      Predefined combinator index (function will work
 *                              only with combinator indices defined in
 *                              [module]_combinators array in
 *                              base_private_[module]_configuration.c). Unused
 *                              in this function.
 * @param[in]   parser_index    Predefined parser index (function will work
 *                              only with parser indices defined in
 *                              [module]_num_of_cb array in
 *                              base_private_[module]_configuration.c). Unused
 *                              in this function.
 * @param[in]   input_str       Stream line passed by combinator (system call).
 * @param[in]   rc_flag         If it is BASE_RC_YES, system call structure can
 *                              be changed in runtime. Unused in this function.
 * @param[out]  result_array    Place holder for value(s) of the object(s).
 *                              Caller should provide address of char matrix as
 *                              result_array.
 * @param[in]   row_num         Number of result_array's rows.
 * @param[in]   col_num         Number of result_array's columns.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
base_status_t get_first_word(module_index_t module_index, int obj_index,
                             int comb_index, int parser_index,
                             char *input_str, base_rc_t rc_flag,
                             void *result_array, unsigned int row_num, unsigned int col_num);

/**
 * @brief This parser only check that the string is present within the stream
 * line (input_str).
 *
 * @param[in]   module_index    Predefined index of the module (enum
 *                              module_index_t from base_defines.h).
 * @param[in]   obj_index       Predefined object index (function will work
 *                              only with object indices defined in
 *                              base_private_defines.h).
 * @param[in]   comb_index      Predefined combinator index (function will work
 *                              only with combinator indices defined in
 *                              [module]_combinators array in
 *                              base_private_[module]_configuration.c).
 * @param[in]   parser_index    Predefined parser index (function will work
 *                              only with parser indices defined in
 *                              [module]_num_of_cb array in
 *                              base_private_[module]_configuration.c).
 * @param[in]   input_str       Stream line passed by combinator (system call).
 * @param[in]   rc_flag         If it is BASE_RC_YES, system call structure can
 *                              be changed in runtime.
 * @param[out]  result_array    Place holder for value(s) of the object(s).
 *                              Caller should provide address of char matrix as
 *                              result_array. Unused in this function.
 * @param[in]   row_num         Number of result_array's rows. Unused in this
 *                              function.
 * @param[in]   col_num         Number of result_array's columns. Unused in
 *                              this function.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
base_status_t find_on_line(module_index_t module_index, int obj_index,
                           int comb_index, int parser_index,
                           char *input_str, base_rc_t rc_flag,
                           void *result_array, unsigned int row_num, unsigned int col_num);

/**
 * @brief This parser only check that the string is not present within the
 * stream line (input_str).
 *
 * @param[in]   module_index    Predefined index of the module (enum
 *                              module_index_t from base_defines.h).
 * @param[in]   obj_index       Predefined object index (function will work
 *                              only with object indices defined in
 *                              base_private_defines.h).
 * @param[in]   comb_index      Predefined combinator index (function will work
 *                              only with combinator indices defined in
 *                              [module]_combinators array in
 *                              base_private_[module]_configuration.c).
 * @param[in]   parser_index    Predefined parser index (function will work
 *                              only with parser indices defined in
 *                              [module]_num_of_cb array in
 *                              base_private_[module]_configuration.c).
 * @param[in]   input_str       Stream line passed by combinator (system call).
 * @param[in]   rc_flag         If it is BASE_RC_YES, system call structure can
 *                              be changed in runtime.
 * @param[out]  result_array    Place holder for value(s) of the object(s).
 *                              Caller should provide address of char matrix as
 *                              result_array.
 * @param[in]   row_num         Number of result_array's rows.
 * @param[in]   col_num         Number of result_array's columns.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
base_status_t is_not_on_line(module_index_t module_index, int obj_index,
                             int comb_index, int parser_index,
                             char *input_str, base_rc_t rc_flag,
                             void *result_array, unsigned int row_num, unsigned int col_num);

/**
 * @brief This parser check that the match case is present within the stream
 * line (input_str) and fill first element of result_array with that match
 * case.
 *
 * @param[in]   module_index    Predefined index of the module (enum
 *                              module_index_t from base_defines.h).
 * @param[in]   obj_index       Predefined object index (function will work
 *                              only with object indices defined in
 *                              base_private_defines.h).
 * @param[in]   comb_index      Predefined combinator index (function will work
 *                              only with combinator indices defined in
 *                              [module]_combinators array in
 *                              base_private_[module]_configuration.c).
 * @param[in]   parser_index    Predefined parser index (function will work
 *                              only with parser indices defined in
 *                              [module]_num_of_cb array in
 *                              base_private_[module]_configuration.c).
 * @param[in]   input_str       Stream line passed by combinator (system call).
 * @param[in]   rc_flag         If it is BASE_RC_YES, system call structure can
 *                              be changed in runtime.
 * @param[out]  result_array    Place holder for value(s) of the object(s).
 *                              Caller should provide address of char matrix as
 *                              result_array.
 * @param[in]   row_num         Number of result_array's rows.
 * @param[in]   col_num         Number of result_array's columns.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
base_status_t find_on_line_and_fill_result_array(module_index_t module_index, int obj_index,
                                                 int comb_index, int parser_index,
                                                 char *input_str, base_rc_t rc_flag,
                                                 void *result_array, unsigned int row_num, unsigned int col_num);

/**
 * @brief Takes all unsigned integer values from line (separated with spaces).
 * Can be used only on lines as e.g. 0 1 2 3.
 *
 * @param[in]   module_index    Predefined index of the module (enum
 *                              module_index_t from base_defines.h). Unused in
 *                              this function.
 * @param[in]   obj_index       Predefined object index (function will work
 *                              only with object indices defined in
 *                              base_private_defines.h). Unused in this
 *                              function.
 * @param[in]   comb_index      Predefined combinator index (function will work
 *                              only with combinator indices defined in
 *                              [module]_combinators array in
 *                              base_private_[module]_configuration.c). Unused
 *                              in this function.
 * @param[in]   parser_index    Predefined parser index (function will work
 *                              only with parser indices defined in
 *                              [module]_num_of_cb array in
 *                              base_private_[module]_configuration.c). Unused
 *                              in this function.
 * @param[in]   input_str       Stream line passed by combinator (system call).
 * @param[in]   rc_flag         If it is BASE_RC_YES, system call structure can
 *                              be changed in runtime. Unused in this function.
 * @param[out]  result_array    Place holder for value(s) of the object(s).
 *                              Caller should provide address of char matrix as
 *                              result_array.
 * @param[in]   row_num         Number of result_array's rows.
 * @param[in]   col_num         Number of result_array's columns.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
base_status_t parse_dec_values_separated_with_spaces(module_index_t module_index, int obj_index,
                                                     int comb_index, int parser_index,
                                                     char *input_str, base_rc_t rc_flag,
                                                     void *result_array, unsigned int row_num, unsigned int col_num);

/**
 * @brief Takes all lines from output and fills result array elements with
 * those lines.
 *
 * @param[in]   module_index    Predefined index of the module (enum
 *                              module_index_t from base_defines.h). Unused in
 *                              this function.
 * @param[in]   obj_index       Predefined object index (function will work
 *                              only with object indices defined in
 *                              base_private_defines.h). Unused in this
 *                              function.
 * @param[in]   comb_index      Predefined combinator index (function will work
 *                              only with combinator indices defined in
 *                              [module]_combinators array in
 *                              base_private_[module]_configuration.c). Unused
 *                              in this function.
 * @param[in]   parser_index    Predefined parser index (function will work
 *                              only with parser indices defined in
 *                              [module]_num_of_cb array in
 *                              base_private_[module]_configuration.c). Unused
 *                              in this function.
 * @param[in]   input_str       Stream line passed by combinator (system call).
 * @param[in]   rc_flag         If it is BASE_RC_YES, system call structure can
 *                              be changed in runtime. Unused in this function.
 * @param[out]  result_array    Place holder for value(s) of the object(s).
 *                              Caller should provide address of char matrix as
 *                              result_array.
 * @param[in]   row_num         Number of result_array's rows.
 * @param[in]   col_num         Number of result_array's columns.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
base_status_t get_all_lines(module_index_t module_index, int obj_index,
                            int comb_index, int parser_index,
                            char *input_str, base_rc_t rc_flag,
                            void *result_array, unsigned int row_num, unsigned int col_num);

/* array parsers */

/**
 * @brief Takes value from result_array strings after match case (predefined
 * for this parser) is identified in string.
 *
 * Parser can be called only after parser(s) which fill result_array.
 *
 * @param[in]   module_index    Predefined index of the module (enum
 *                              module_index_t from base_defines.h). Unused in
 *                              this function.
 * @param[in]   obj_index       Predefined object index (function will work
 *                              only with object indices defined in
 *                              base_private_defines.h).
 * @param[in]   comb_index      Predefined combinator index (function will work
 *                              only with combinator indices defined in
 *                              [module]_combinators array in
 *                              base_private_[module]_configuration.c).
 * @param[in]   parser_index    Predefined parser index (function will work
 *                              only with parser indices defined in
 *                              [module]_num_of_cb array in
 *                              base_private_[module]_configuration.c).
 * @param[in]   input_str       Stream line passed by combinator (system call).
 *                              Unused in this function.
 * @param[in]   rc_flag         If it is BASE_RC_YES, system call structure can
 *                              be changed in runtime.
 * @param[out]  result_array    Place holder for value(s) of the object(s).
 *                              Caller should provide address of char matrix as
 *                              result_array.
 * @param[in]   row_num         Number of result_array's rows.
 * @param[in]   col_num         Number of result_array's columns.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
base_status_t array_take_value_after(module_index_t module_index, int obj_index,
                                     int comb_index, int parser_index,
                                     char *input_str, base_rc_t rc_flag,
                                     void *result_array, unsigned int row_num, unsigned int col_num);

/**
 * @brief Drops all punctuation from result_array strings.
 *
 * Parser can be called only after parser(s) which fill result_array.
 *
 * @param[in]   module_index    Predefined index of the module (enum
 *                              module_index_t from base_defines.h). Unused in
 *                              this function. Unused in this function.
 * @param[in]   obj_index       Predefined object index (function will work
 *                              only with object indices defined in
 *                              base_private_defines.h). Unused in this
 *                              function.
 * @param[in]   comb_index      Predefined combinator index (function will work
 *                              only with combinator indices defined in
 *                              [module]_combinators array in
 *                              base_private_[module]_configuration.c). Unused
 *                              in this function.
 * @param[in]   parser_index    Predefined parser index (function will work
 *                              only with parser indices defined in
 *                              [module]_num_of_cb array in
 *                              base_private_[module]_configuration.c). Unused
 *                              in this function.
 * @param[in]   input_str       Stream line passed by combinator (system call).
 *                              Unused in this function.
 * @param[in]   rc_flag         If it is BASE_RC_YES, system call structure can
 *                              be changed in runtime. Unused in this function.
 * @param[out]  result_array    Place holder for value(s) of the object(s).
 *                              Caller should provide address of char matrix as
 *                              result_array.
 * @param[in]   row_num         Number of result_array's rows.
 * @param[in]   col_num         Number of result_array's columns.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
base_status_t array_drop_punctuation(module_index_t module_index, int obj_index,
                                     int comb_index, int parser_index,
                                     char *input_str, base_rc_t rc_flag,
                                     void *result_array, unsigned int row_num, unsigned int col_num);

/**
 * @brief Delete string from result_array if it is identified with match case.
 *
 * Parser can be called only after parser(s) which fill result_array.
 *
 * @param[in]   module_index    Predefined index of the module (enum
 *                              module_index_t from base_defines.h). Unused in
 *                              this function.
 * @param[in]   obj_index       Predefined object index (function will work
 *                              only with object indices defined in
 *                              base_private_defines.h).
 * @param[in]   comb_index      Predefined combinator index (function will work
 *                              only with combinator indices defined in
 *                              [module]_combinators array in
 *                              base_private_[module]_configuration.c).
 * @param[in]   parser_index    Predefined parser index (function will work
 *                              only with parser indices defined in
 *                              [module]_num_of_cb array in
 *                              base_private_[module]_configuration.c).
 * @param[in]   input_str       Stream line passed by combinator (system call).
 *                              Unused in this function.
 * @param[in]   rc_flag         If it is BASE_RC_YES, system call structure can
 *                              be changed in runtime.
 * @param[out]  result_array    Place holder for value(s) of the object(s).
 *                              Caller should provide address of char matrix as
 *                              result_array.
 * @param[in]   row_num         Number of result_array's rows.
 * @param[in]   col_num         Number of result_array's columns.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
base_status_t array_delete_element(module_index_t module_index, int obj_index,
                                   int comb_index, int parser_index,
                                   char *input_str, base_rc_t rc_flag,
                                   void *result_array, unsigned int row_num, unsigned int col_num);

/**
 * @brief Performs bitwise and with mask from match case.
 *
 * Parser can be called only after parser(s) which fill result_array.
 *
 * @param[in]   module_index    Predefined index of the module (enum
 *                              module_index_t from base_defines.h). Unused in
 *                              this function.
 * @param[in]   obj_index       Predefined object index (function will work
 *                              only with object indices defined in
 *                              base_private_defines.h).
 * @param[in]   comb_index      Predefined combinator index (function will work
 *                              only with combinator indices defined in
 *                              [module]_combinators array in
 *                              base_private_[module]_configuration.c).
 * @param[in]   parser_index    Predefined parser index (function will work
 *                              only with parser indices defined in
 *                              [module]_num_of_cb array in
 *                              base_private_[module]_configuration.c).
 * @param[in]   input_str       Stream line passed by combinator (system call).
 *                              Unused in this function.
 * @param[in]   rc_flag         If it is BASE_RC_YES, system call structure can
 *                              be changed in runtime.
 * @param[out]  result_array    Place holder for value(s) of the object(s).
 *                              Caller should provide address of char matrix as
 *                              result_array.
 * @param[in]   row_num         Number of result_array's rows.
 * @param[in]   col_num         Number of result_array's columns.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
base_status_t array_bitwise_AND_mask(module_index_t module_index, int obj_index,
                                     int comb_index, int parser_index,
                                     char *input_str, base_rc_t rc_flag,
                                     void *result_array, unsigned int row_num, unsigned int col_num);

/**
 * @brief Multiplies long int result_array values (converted from strings) with
 * match case long int value (converted from string).
 *
 * Parser can be called only after parser(s) which fill result_array.
 *
 * @param[in]   module_index    Predefined index of the module (enum
 *                              module_index_t from base_defines.h). Unused in
 *                              this function.
 * @param[in]   obj_index       Predefined object index (function will work
 *                              only with object indices defined in
 *                              base_private_defines.h).
 * @param[in]   comb_index      Predefined combinator index (function will work
 *                              only with combinator indices defined in
 *                              [module]_combinators array in
 *                              base_private_[module]_configuration.c).
 * @param[in]   parser_index    Predefined parser index (function will work
 *                              only with parser indices defined in
 *                              [module]_num_of_cb array in
 *                              base_private_[module]_configuration.c).
 * @param[in]   input_str       Stream line passed by combinator (system call).
 *                              Unused in this function.
 * @param[in]   rc_flag         If it is BASE_RC_YES, system call structure can
 *                              be changed in runtime.
 * @param[out]  result_array    Place holder for value(s) of the object(s).
 *                              Caller should provide address of char matrix as
 *                              result_array.
 * @param[in]   row_num         Number of result_array's rows.
 * @param[in]   col_num         Number of result_array's columns.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
base_status_t array_multiply_long_int_with(module_index_t module_index, int obj_index,
                                           int comb_index, int parser_index,
                                           char *input_str, base_rc_t rc_flag,
                                           void *result_array, unsigned int row_num, unsigned int col_num);\

/**
 * @brief Shift in right result_array values (converted from strings) for match
 * case places (converted from string).
 *
 * Parser can be called only after parser(s) which fill result_array. This
 * parser assumes that result_array ant match_case contain hex values. If
 * parsing went well, result array will contain integer value as string.
 *
 * @param[in]   module_index    Predefined index of the module (enum
 *                              module_index_t from base_defines.h). Unused in
 *                              this function.
 * @param[in]   obj_index       Predefined object index (function will work
 *                              only with object indices defined in
 *                              base_private_defines.h).
 * @param[in]   comb_index      Predefined combinator index (function will work
 *                              only with combinator indices defined in
 *                              [module]_combinators array in
 *                              base_private_[module]_configuration.c).
 * @param[in]   parser_index    Predefined parser index (function will work
 *                              only with parser indices defined in
 *                              [module]_num_of_cb array in
 *                              base_private_[module]_configuration.c).
 * @param[in]   input_str       Stream line passed by combinator (system call).
 *                              Unused in this function.
 * @param[in]   rc_flag         If it is BASE_RC_YES, system call structure can
 *                              be changed in runtime. Unused in this function.
 * @param[out]  result_array    Place holder for value(s) of the object(s).
 *                              Caller should provide address of char matrix as
 *                              result_array.
 * @param[in]   row_num         Number of result_array's rows.
 * @param[in]   col_num         Number of result_array's columns.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
base_status_t array_shift_right(module_index_t module_index, int obj_index,
                                int comb_index, int parser_index,
                                char *input_str, base_rc_t rc_flag,
                                void *result_array, unsigned int row_num, unsigned int col_num);

/**
 * @brief Converts result_array hex values (converted from strings) into long
 * int values which are translated back into result_array strings.
 *
 * Parser can be called only after parser(s) which fill result_array.
 *
 * @param[in]   module_index    Predefined index of the module (enum
 *                              module_index_t from base_defines.h). Unused in
 *                              this function. Unused in this function.
 * @param[in]   obj_index       Predefined object index (function will work
 *                              only with object indices defined in
 *                              base_private_defines.h). Unused in this
 *                              function.
 * @param[in]   comb_index      Predefined combinator index (function will work
 *                              only with combinator indices defined in
 *                              [module]_combinators array in
 *                              base_private_[module]_configuration.c). Unused
 *                              in this function.
 * @param[in]   parser_index    Predefined parser index (function will work
 *                              only with parser indices defined in
 *                              [module]_num_of_cb array in
 *                              base_private_[module]_configuration.c). Unused
 *                              in this function.
 * @param[in]   input_str       Stream line passed by combinator (system call).
 *                              Unused in this function.
 * @param[in]   rc_flag         If it is BASE_RC_YES, system call structure can
 *                              be changed in runtime. Unused in this function.
 * @param[out]  result_array    Place holder for value(s) of the object(s).
 *                              Caller should provide address of char matrix as
 *                              result_array.
 * @param[in]   row_num         Number of result_array's rows.
 * @param[in]   col_num         Number of result_array's columns.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
base_status_t array_convert_hex_string_to_long_int(module_index_t module_index, int obj_index,
                                                   int comb_index, int parser_index,
                                                   char *input_str, base_rc_t rc_flag,
                                                   void *result_array, unsigned int row_num, unsigned int col_num);

/**
 * @brief Parser match case holds number of characters from begin which need to
 * stay in result_array strings.
 *
 * Parser can be called only after parser(s) which fill result_array.
 *
 * @param[in]   module_index    Predefined index of the module (enum
 *                              module_index_t from base_defines.h). Unused in
 *                              this function.
 * @param[in]   obj_index       Predefined object index (function will work
 *                              only with object indices defined in
 *                              base_private_defines.h).
 * @param[in]   comb_index      Predefined combinator index (function will work
 *                              only with combinator indices defined in
 *                              [module]_combinators array in
 *                              base_private_[module]_configuration.c).
 * @param[in]   parser_index    Predefined parser index (function will work
 *                              only with parser indices defined in
 *                              [module]_num_of_cb array in
 *                              base_private_[module]_configuration.c).
 * @param[in]   input_str       Stream line passed by combinator (system call).
 *                              Unused in this function.
 * @param[in]   rc_flag         If it is BASE_RC_YES, system call structure can
 *                              be changed in runtime.
 * @param[out]  result_array    Place holder for value(s) of the object(s).
 *                              Caller should provide address of char matrix as
 *                              result_array.
 * @param[in]   row_num         Number of result_array's rows.
 * @param[in]   col_num         Number of result_array's columns.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
base_status_t array_shrink_string(module_index_t module_index, int obj_index,
                                  int comb_index, int parser_index,
                                  char *input_str, base_rc_t rc_flag,
                                  void *result_array, unsigned int row_num, unsigned int col_num);

/**
 * @brief This parser takes only unique elements from result_array.
 *
 * Parser can be called only after parser(s) which fill result_array.
 *
 * @param[in]   module_index    Predefined index of the module (enum
 *                              module_index_t from base_defines.h). Unused in
 *                              this function.
 * @param[in]   obj_index       Predefined object index (function will work
 *                              only with object indices defined in
 *                              base_private_defines.h).
 * @param[in]   comb_index      Predefined combinator index (function will work
 *                              only with combinator indices defined in
 *                              [module]_combinators array in
 *                              base_private_[module]_configuration.c).
 * @param[in]   parser_index    Predefined parser index (function will work
 *                              only with parser indices defined in
 *                              [module]_num_of_cb array in
 *                              base_private_[module]_configuration.c).
 * @param[in]   input_str       Stream line passed by combinator (system call).
 * @param[in]   rc_flag         If it is BASE_RC_YES, system call structure can
 *                              be changed in runtime. Unused in this function.
 * @param[out]  result_array    Place holder for value(s) of the object(s).
 *                              Caller should provide address of char matrix as
 *                              result_array.
 * @param[in]   row_num         Number of result_array's rows.
 * @param[in]   col_num         Number of result_array's columns.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
base_status_t array_uniq_elements(module_index_t module_index, int obj_index,
                                  int comb_index, int parser_index,
                                  char *input_str, base_rc_t rc_flag,
                                  void *result_array, unsigned int row_num, unsigned int col_num);

/**
 * @brief This parser returns the number of elements in the result_array.
 *
 * Parser can be called only after parser(s) which fill result_array.
 *
 * @param[in]   module_index    Predefined index of the module (enum
 *                              module_index_t from base_defines.h). Unused in
 *                              this function.
 * @param[in]   obj_index       Predefined object index (function will work
 *                              only with object indices defined in
 *                              base_private_defines.h).
 * @param[in]   comb_index      Predefined combinator index (function will work
 *                              only with combinator indices defined in
 *                              [module]_combinators array in
 *                              base_private_[module]_configuration.c).
 * @param[in]   parser_index    Predefined parser index (function will work
 *                              only with parser indices defined in
 *                              [module]_num_of_cb array in
 *                              base_private_[module]_configuration.c).
 * @param[in]   input_str       Stream line passed by combinator (system call).
 * @param[in]   rc_flag         If it is BASE_RC_YES, system call structure can
 *                              be changed in runtime. Unused in this function.
 * @param[out]  result_array    Place holder for value(s) of the object(s).
 *                              Caller should provide address of char matrix as
 *                              result_array.
 * @param[in]   row_num         Number of result_array's rows.
 * @param[in]   col_num         Number of result_array's columns.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
base_status_t array_count_elements(module_index_t module_index, int obj_index,
                                   int comb_index, int parser_index,
                                   char *input_str, base_rc_t rc_flag,
                                   void *result_array, unsigned int row_num, unsigned int col_num);

/**
 * @brief This parser modify the result array elements to take only characters
 * until match case.
 *
 * Parser can be called only after parser(s) which fill result_array.
 *
 * @param[in]   module_index    Predefined index of the module (enum
 *                              module_index_t from base_defines.h). Unused in
 *                              this function.
 * @param[in]   obj_index       Predefined object index (function will work
 *                              only with object indices defined in
 *                              base_private_defines.h).
 * @param[in]   comb_index      Predefined combinator index (function will work
 *                              only with combinator indices defined in
 *                              [module]_combinators array in
 *                              base_private_[module]_configuration.c).
 * @param[in]   parser_index    Predefined parser index (function will work
 *                              only with parser indices defined in
 *                              [module]_num_of_cb array in
 *                              base_private_[module]_configuration.c).
 * @param[in]   input_str       Stream line passed by combinator (system call).
 * @param[in]   rc_flag         If it is BASE_RC_YES, system call structure can
 *                              be changed in runtime. Unused in this function.
 * @param[out]  result_array    Place holder for value(s) of the object(s).
 *                              Caller should provide address of char matrix as
 *                              result_array.
 * @param[in]   row_num         Number of result_array's rows.
 * @param[in]   col_num         Number of result_array's columns.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
base_status_t array_until_match_case(module_index_t module_index, int obj_index,
                                     int comb_index, int parser_index,
                                     char *input_str, base_rc_t rc_flag,
                                     void *result_array, unsigned int row_num, unsigned int col_num);

/**
 * @brief This parser modify the result array elements to take only array
 * elements after match case.
 *
 * Parser can be called only after parser(s) which fill result_array.
 *
 * @param[in]   module_index    Predefined index of the module (enum
 *                              module_index_t from base_defines.h). Unused in
 *                              this function.
 * @param[in]   obj_index       Predefined object index (function will work
 *                              only with object indices defined in
 *                              base_private_defines.h).
 * @param[in]   comb_index      Predefined combinator index (function will work
 *                              only with combinator indices defined in
 *                              [module]_combinators array in
 *                              base_private_[module]_configuration.c).
 * @param[in]   parser_index    Predefined parser index (function will work
 *                              only with parser indices defined in
 *                              [module]_num_of_cb array in
 *                              base_private_[module]_configuration.c).
 * @param[in]   input_str       Stream line passed by combinator (system call).
 * @param[in]   rc_flag         If it is BASE_RC_YES, system call structure can
 *                              be changed in runtime. Unused in this function.
 * @param[out]  result_array    Place holder for value(s) of the object(s).
 *                              Caller should provide address of char matrix as
 *                              result_array.
 * @param[in]   row_num         Number of result_array's rows.
 * @param[in]   col_num         Number of result_array's columns.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
base_status_t array_take_values_after_match_case(module_index_t module_index, int obj_index,
                                                 int comb_index, int parser_index,
                                                 char *input_str, base_rc_t rc_flag,
                                                 void *result_array, unsigned int row_num, unsigned int col_num);

/**
 * @brief This parser modify the result array elements to take only array
 * elements before match case.
 *
 * Parser can be called only after parser(s) which fill result_array.
 *
 * @param[in]   module_index    Predefined index of the module (enum
 *                              module_index_t from base_defines.h). Unused in
 *                              this function.
 * @param[in]   obj_index       Predefined object index (function will work
 *                              only with object indices defined in
 *                              base_private_defines.h).
 * @param[in]   comb_index      Predefined combinator index (function will work
 *                              only with combinator indices defined in
 *                              [module]_combinators array in
 *                              base_private_[module]_configuration.c).
 * @param[in]   parser_index    Predefined parser index (function will work
 *                              only with parser indices defined in
 *                              [module]_num_of_cb array in
 *                              base_private_[module]_configuration.c).
 * @param[in]   input_str       Stream line passed by combinator (system call).
 * @param[in]   rc_flag         If it is BASE_RC_YES, system call structure can
 *                              be changed in runtime. Unused in this function.
 * @param[out]  result_array    Place holder for value(s) of the object(s).
 *                              Caller should provide address of char matrix as
 *                              result_array.
 * @param[in]   row_num         Number of result_array's rows.
 * @param[in]   col_num         Number of result_array's columns.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
base_status_t array_take_values_before_match_case(module_index_t module_index, int obj_index,
                                                  int comb_index, int parser_index,
                                                  char *input_str, base_rc_t rc_flag,
                                                  void *result_array, unsigned int row_num, unsigned int col_num);

/**
 * @brief This parser writes the result array to the file.
 *
 * Parser can be called only after parser(s) which fill result_array.
 *
 * @param[in]   module_index    Predefined index of the module (enum
 *                              module_index_t from base_defines.h). Unused in
 *                              this function.
 * @param[in]   obj_index       Predefined object index (function will work
 *                              only with object indices defined in
 *                              base_private_defines.h).
 * @param[in]   comb_index      Predefined combinator index (function will work
 *                              only with combinator indices defined in
 *                              [module]_combinators array in
 *                              base_private_[module]_configuration.c).
 * @param[in]   parser_index    Predefined parser index (function will work
 *                              only with parser indices defined in
 *                              [module]_num_of_cb array in
 *                              base_private_[module]_configuration.c).
 * @param[in]   input_str       Stream line passed by combinator (system call).
 * @param[in]   rc_flag         If it is BASE_RC_YES, system call structure can
 *                              be changed in runtime. Unused in this function.
 * @param[out]  result_array    Place holder for value(s) of the object(s).
 *                              Caller should provide address of char matrix as
 *                              result_array.
 * @param[in]   row_num         Number of result_array's rows.
 * @param[in]   col_num         Number of result_array's columns.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
base_status_t array_write_to_file(module_index_t module_index, int obj_index,
                                  int comb_index, int parser_index,
                                  char *input_str, base_rc_t rc_flag,
                                  void *result_array, unsigned int row_num, unsigned int col_num);

#endif /* INCLUDE_BASE_PRIVATE_PARSERS_H_ */
