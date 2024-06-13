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

#ifndef INCLUDE_BASE_PRIVATE_COMBINATORS_H_
#define INCLUDE_BASE_PRIVATE_COMBINATORS_H_

#include "base_private_defines.h"
#include "base_private_syscalls.h"

/**
 * @brief Combinator without any combination with other combinators for
 * object_index. Executes always only one callback (parser or validator) -
 * first one.
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
 * @param[in]   output          I/O stream passed from system call. Unused in
 *                              this function.
 * @param[in]   rc_flag         If it is BASE_RC_YES, system call structure can
 *                              be changed in runtime.
 * @param[out   result_array    Place holder for value(s) of the object(s).
 *                              Caller should provide address of char matrix as
 *                              result_array.
 * @param[in]   row_num         Number of result_array's rows.
 * @param[in]   col_num         Number of result_array's columns.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
base_status_t no_combinator(module_index_t module_index, int obj_index,
                            int comb_index, FILE *output, base_rc_t rc_flag,
                            void *result_array, unsigned int row_num, unsigned int col_num);

/**
 * @brief Taking all lines (one by one) from the stream (output) and execute
 * all parsers and validators for this combinator on all lines.
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
 * @param[in]   output          I/O stream passed from system call.
 * @param[in]   rc_flag         If it is BASE_RC_YES, system call structure can
 *                              be changed in runtime.
 * @param[out]  result_array    Place holder for value(s) of the object(s).
 *                              Caller should provide address of char matrix as
 *                              result_array.
 * @param[in]   row_num         Number of result_array's rows.
 * @param[in]   col_num         Number of result_array's columns.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
base_status_t repeat_until_null_with_check(module_index_t module_index,
                                           int obj_index, int comb_index,
                                           FILE *output, base_rc_t rc_flag,
                                           void *result_array, unsigned int row_num, unsigned int col_num);

/**
 * @brief Taking all lines (one by one) from the stream (output) and execute
 * first parser on all lines and get next line if first parser succeeded.
 * Second parser is executed on next line as input parameter.
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
 * @param[in]   output          I/O stream passed from system call.
 * @param[in]   rc_flag         If it is BASE_RC_YES, system call structure can
 *                              be changed in runtime.
 * @param[out]  result_array    Place holder for value(s) of the object(s).
 *                              Caller should provide address of char matrix as
 *                              result_array.
 * @param[in]   row_num         Number of result_array's rows.
 * @param[in]   col_num         Number of result_array's columns.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
base_status_t repeat_check_and_get_next_line(module_index_t module_index,
                                             int obj_index, int comb_index,
                                             FILE *output, base_rc_t rc_flag,
                                             void *result_array, unsigned int row_num, unsigned int col_num);

/**
 * @brief Combinator with OR logic. Taking one line from the stream (output)
 * and execute all parsers and validators for this combinator.
 * @param[in]   module_index    Predefined index of the module (enum
 *                              module_index_t from base_defines.h).
 * @param[in]   obj_index       Predefined object index (function will work
 *                              only with object indices defined in
 *                              base_private_defines.h).
 * @param[in]   comb_index      Predefined combinator index (function will work
 *                              only with combinator indices defined in
 *                              [module]_combinators array in
 *                              base_private_[module]_configuration.c).
 * @param[in]   output          I/O stream passed from system call. Unused in
 *                              this function.
 * @param[in]   rc_flag         If it is BASE_RC_YES, system call structure can
 *                              be changed in runtime.
 * @param[out]  result_array    Place holder for value(s) of the object(s).
 *                              Caller should provide address of char matrix as
 *                              result_array.
 * @param[in]   row_num         Number of result_array's rows.
 * @param[in]   col_num         Number of result_array's columns.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
base_status_t OR_combinator(module_index_t module_index, int obj_index,
                            int comb_index, FILE *output, base_rc_t rc_flag,
                            void *result_array, unsigned int row_num, unsigned int col_num);

/**
 * @brief Combinator with AND logic. Taking all lines from the stream (output)
 * and execute all parsers and validators for this combinator per line.
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
 * @param[in]   output          I/O stream passed from system call.
 * @param[in]   rc_flag         If it is BASE_RC_YES, system call structure can
 *                              be changed in runtime.
 * @param[out]  result_array    Place holder for value(s) of the object(s).
 *                              Caller should provide address of char matrix as
 *                              result_array.
 * @param[in]   row_num         Number of result_array's rows.
 * @param[in]   col_num         Number of result_array's columns.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
base_status_t AND_combinator(module_index_t module_index, int obj_index,
                             int comb_index, FILE *output, base_rc_t rc_flag,
                             void *result_array, unsigned int row_num, unsigned int col_num);

/**
 * @brief This can be used to combine several combinators. Used only to execute
 * other combinators for object_index.
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
 * @param[in]   output          I/O stream passed from system call.
 * @param[in]   rc_flag         If it is BASE_RC_YES, system call structure can
 *                              be changed in runtime.
 * @param[out]  result_array    Place holder for value(s) of the object(s).
 *                              Caller should provide address of char matrix as
 *                              result_array.
 * @param[in]   row_num         Number of result_array's rows.
 * @param[in]   col_num         Number of result_array's columns.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
base_status_t combinator_comb(module_index_t module_index, int obj_index,
                              int comb_index, FILE *output, base_rc_t rc_flag,
                              void *result_array, unsigned int row_num, unsigned int col_num);

/**
 * @brief Taking all lines (one by one) from the stream (output) and execute
 * all parsers and validators for this combinator on all lines and count
 * succeeded parsers.
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
 * @param[in]   output          I/O stream passed from system call.
 * @param[in]   rc_flag         If it is BASE_RC_YES, system call structure can
 *                              be changed in runtime.
 * @param[out]  result_array    Place holder for value(s) of the object(s).
 *                              Caller should provide address of char matrix as
 *                              result_array.
 * @param[in]   row_num         Number of result_array's rows.
 * @param[in]   col_num         Number of result_array's columns.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
base_status_t repeat_until_eof_and_count(module_index_t module_index,
                                         int obj_index, int comb_index,
                                         FILE *output, base_rc_t rc_flag,
                                         void *result_array, unsigned int row_num, unsigned int col_num);

#endif /* INCLUDE_BASE_PRIVATE_COMBINATORS_H_ */
