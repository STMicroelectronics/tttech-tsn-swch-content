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

#ifndef INCLUDE_BASE_PRIVATE_VALIDATORS_H_
#define INCLUDE_BASE_PRIVATE_VALIDATORS_H_

#include "base_private_combinators.h"
#include "base_private_defines.h"

/* input stream line validators */

/**
 * @brief This validator only check that first string in result_array is a
 * number, and if not erase this string.
 *
 * Note: Works only with first string from result_array!
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
 * @param[in]   row_num         Number of result_array's rows. Unused in this
 *                              function.
 * @param[in]   col_num         Number of result_array's columns.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
base_status_t validate_uint_32_number(module_index_t module_index, int obj_index,
                                      int comb_index, int parser_index,
                                      char *input_str, base_rc_t rc_flag,
                                      void *result_array, unsigned int row_num, unsigned int col_num);

/**
 * @brief Validates string content. Validates only first element of
 * result_array.
 *
 * Validator can be called only after parser(s) which fill result_array.
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
 *                              Unused in this function.
 * @param[in]   rc_flag         If it is BASE_RC_YES, system call structure can
 *                              be changed in runtime.
 * @param[out]  result_array    Place holder for value(s) of the object(s).
 *                              Caller should provide address of char matrix as
 *                              result_array.
 * @param[in]   row_num         Number of result_array's rows. Unused in this
 *                              function.
 * @param[in]   col_num         Number of result_array's columns.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
base_status_t validate_string_content(module_index_t module_index, int obj_index,
                                      int comb_index, int parser_index,
                                      char *input_str, base_rc_t rc_flag,
                                      void *result_array, unsigned int row_num, unsigned int col_num);

/**
 * @brief Validates string length (match case of validator). Validates all
 * elements of array.
 *
 * This validator can be called only after parser(s) which fill result_array.
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
base_status_t array_validate_string_length(module_index_t module_index, int obj_index,
                                           int comb_index, int parser_index,
                                           char *input_str, base_rc_t rc_flag,
                                           void *result_array, unsigned int row_num, unsigned int col_num);

/**
 * @brief Validates is array element lower than predefined interval lower bound
 * (match case of validator).
 *
 * Validates all elements of array. This validator can be called only after
 * parser(s) which fill result_array.
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
base_status_t array_validate_interval_lower_bound(module_index_t module_index, int obj_index,
                                                  int comb_index, int parser_index,
                                                  char *input_str, base_rc_t rc_flag,
                                                  void *result_array, unsigned int row_num, unsigned int col_num);

/**
 * @brief Validates is array element greater than predefined interval upper
 * bound (match case of validator).
 *
 * Validates all elements of array. This validator can be called only after
 * parser(s) which fill result_array.
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
base_status_t array_validate_interval_upper_bound(module_index_t module_index, int obj_index,
                                                  int comb_index, int parser_index,
                                                  char *input_str, base_rc_t rc_flag,
                                                  void *result_array, unsigned int row_num, unsigned int col_num);

#endif /* INCLUDE_BASE_PRIVATE_VALIDATORS_H_ */
