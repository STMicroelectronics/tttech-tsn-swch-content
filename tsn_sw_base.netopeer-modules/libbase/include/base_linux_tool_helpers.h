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

#ifndef INCLUDE_BASE_LINUX_TOOL_HELPERS_H_
#define INCLUDE_BASE_LINUX_TOOL_HELPERS_H_

#include <stdarg.h>
#include "base_defines.h"
#include "base_private_defines.h"

// IETF-INTERFACES
extern char if_map[IF_MAX_OBJ_NUMBER][MAX_STR_LEN];
extern combinator_fn_t if_combinators[IF_MAX_OBJ_NUMBER][MAX_COMBINATORS_NUM];
extern int if_num_of_combinators[IF_MAX_OBJ_NUMBER];
extern int if_num_of_cb[IF_MAX_OBJ_NUMBER][MAX_COMBINATORS_NUM];
extern parser_spec_t if_parsing_cb[IF_MAX_OBJ_NUMBER][MAX_COMBINATORS_NUM][MAX_CALLBACKS_NUM];
extern char if_get_sys_call_commands[IF_MAX_OBJ_NUMBER][MAX_STR_LEN];
extern syscall_fn_t if_syscalls[IF_MAX_OBJ_NUMBER];
extern char if_set_sys_call_commands[IF_MAX_OBJ_NUMBER][MAX_STR_LEN];
// MSTP
extern char mstp_map[MSTP_MAX_OBJ_NUMBER][MAX_STR_LEN];
extern combinator_fn_t mstp_combinators[MSTP_MAX_OBJ_NUMBER][MAX_COMBINATORS_NUM];
extern int mstp_num_of_combinators[MSTP_MAX_OBJ_NUMBER];
extern int mstp_num_of_cb[MSTP_MAX_OBJ_NUMBER][MAX_COMBINATORS_NUM];
extern parser_spec_t mstp_parsing_cb[MSTP_MAX_OBJ_NUMBER][MAX_COMBINATORS_NUM][MAX_CALLBACKS_NUM];
extern char mstp_get_sys_call_commands[MSTP_MAX_OBJ_NUMBER][MAX_STR_LEN];
extern syscall_fn_t mstp_syscalls[MSTP_MAX_OBJ_NUMBER];
extern char mstp_set_sys_call_commands[MSTP_MAX_OBJ_NUMBER][MAX_STR_LEN];
// Q-BRIDGE
extern char q_bridge_map[Q_BRIDGE_MAX_OBJ_NUMBER][MAX_STR_LEN];
extern combinator_fn_t q_bridge_combinators[Q_BRIDGE_MAX_OBJ_NUMBER][MAX_COMBINATORS_NUM];
extern int q_bridge_num_of_combinators[Q_BRIDGE_MAX_OBJ_NUMBER];
extern int q_bridge_num_of_cb[Q_BRIDGE_MAX_OBJ_NUMBER][MAX_COMBINATORS_NUM];
extern parser_spec_t q_bridge_parsing_cb[Q_BRIDGE_MAX_OBJ_NUMBER][MAX_COMBINATORS_NUM][MAX_CALLBACKS_NUM];
extern char q_bridge_get_sys_call_commands[Q_BRIDGE_MAX_OBJ_NUMBER][MAX_STR_LEN];
extern syscall_fn_t q_bridge_syscalls[Q_BRIDGE_MAX_OBJ_NUMBER];
extern char q_bridge_set_sys_call_commands[Q_BRIDGE_MAX_OBJ_NUMBER][MAX_STR_LEN];

/**
 * @brief Assembling and sending command to shell (system). Has check if
 * command executed successfully.
 *
 * @param   format              Holds command.
 * @param   ...                 The arguments for format.
 * @return  EXIT_SUCCESS or EXIT_FAILURE.
 */
int set_system_call(const char *format, ...);

/**
 * @brief Checking are object indices valid.
 *
 * If caller need to check only some of the indices, caller can set zeros as
 * other indices (except module_index - it is mandatory).
 *
 * @param[in]   module_index    Predefined index of the module (enum
 *                              module_index_t from base_defines.h).
 * @param[in]   obj_index       Object index. There is different range for
 *                              every module (defined in base_private_defines.h
 *                              as enums [module_name]_obj_index_t).
 * @param[in]   comb_index      Combinator index. Defined in module
 *                              configuration file.
 * @param[in]   parser_index    Parser index in combinator. Defined in module
 *                              configuration file.
 * @return 1 (true) or 0 (false).
 */
int are_object_indices_valid(module_index_t module_index, int obj_index,
                             int comb_index, int parser_index);

/**
 * @brief This function will return index of the object from pointed module.
 *
 * @param[in]   object_name     Name of the object.
 * @param[in]   map             Map between object index and object name.
 * @param[in]   max_obj_number  Max object index for the module.
 * @returns index of the module object
 */
int find_obj_index_by_name(char *object_name, void *map, int max_obj_number);

/**
 * @brief This function will return system call structure (sys_call_spec_t) for
 * object_index.
 *
 * @param[in]   module_index    Predefined module index.
 * @param[in]   object_index    Predefined object index.
 * @param[in]   rc_flag         If it is BASE_RC_YES, system call structure can
 *                              be changed in runtime.
 * @returns sys_call_spec_t structure
 */
sys_call_spec_t assemble_sys_call_structure(module_index_t module_index, int object_index, base_rc_t rc_flag);

/**
 * @brief This function will store runtime configurable system call structure.
 *
 * @param[in]   sys_call_spec   Customized system call structure.
 */
void set_rc_sys_call_spec(sys_call_spec_t sys_call_spec);

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
 * @param[in]   args            List of arguments which will be passed to
 *                              system call.
 * @returns Predefined enum value (base_status_t).
 */
base_status_t execute_sys_call(module_index_t module_index, int obj_index, base_rc_t rc_flag,
                               void *result_array, unsigned int row_num, unsigned int col_num,
                               va_list args);

void free_output(FILE** output);
void free_string(char** str);

/**
 * @brief Setting device down (turn off).
 *
 * @param       dev_name        Char pointer that holds value which will be set.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
int ip_link_set_dev_down(char *dev_name);

/**
 * @brief Setting device up (turn on).
 *
 * @param       dev_name        Char pointer that holds value which will be set.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
int ip_link_set_dev_up(char *dev_name);

/**
 * @brief Setting device address.
 *
 * @param       value           Char pointer that holds value which will be set.
 * @param       dev_name        Char pointer that points to device name used in
 * setting.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
int ip_link_set_dev_address(char *dev_name, char *value);

/**
 * @brief Checks if interface exist.
 * @param       if_name         Interface name.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
int if_interface_exist(char *if_name);

/**
 * @brief Checks if pvid is in vids string.
 *
 * @param[in]   pvid            Holds pvid.
 * @param[in]   vids            Holds vids which will be checked.
 * @return      1 if pvid is in vids string
 * @return      0 if not
 */
int is_pvid_in_vids_string(char *pvid, char *vids);

/* string management */
int is_sub_interval(char *potential_sub_interval);

/**
 * @brief Check address type (ipv4 or ipv6).
 *
 * @param[in]   address         Address which is tested.
 * @param[out]  value           It will contain "1" (ipv4) or "2" (ipv6).
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
int check_address_type(char *address, char *value);

/**
 * @brief Changes all ":" into "-" in static char array (address).
 *
 * @param[out]  address         Static char array (MAC address).
 */
void change_addr_colons_to_dashes(char *address);

/**
 * @brief Changes all "-" into ":" in static char array (address).
 *
 * @param[out]  address         Static char array (MAC address).
 */
void change_addr_dashes_to_colons(char *address);

/**
 * @brief Change first '\n' to '\0' in str.
 *
 * @param[in]   str             The string.
 */
void terminate_string(char *str);

/**
 * @brief Assemble vids string to array of the single vlan ids.
 *
 * @param[out]  result_array Strings array which holds single vlan ids.
 * @param[in]   row_num     Number of result_array's rows.
 * @param[in]   col_num     Number of result_array's columns.
 * @param[in]   vids        Char pointer of the string which holds all vlan ids.
 */
void assemble_vids_as_array(void *result_array, unsigned int row_num, unsigned int col_num,
                            char *vids);

/**
 * @brief Checks if the vids string contains descending interval.
 *
 * @param[in]   vids        Char pointer of the string which holds all vlan ids.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
int does_vids_string_contains_descending_interval(char *vids);

/**
 * @brief Checks if the vids string contains vid greater than 4095.
 *
 * @param[in]   vids        Char pointer of the string which holds all vlan ids.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
int does_vids_string_contains_invalid_vid(char *vids);

/**
 * @brief Encode array of unsigned chars to base64 binary array.
 *
 * @param[in]   input       Contains the array of chars to encode.
 * @param[in]   input_len   Contains the length of the array to encode.
 * @param[in]   encoded     Contains the encoded array.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
int base64_encode(unsigned char *input, int input_len, char *encoded);

#endif /* INCLUDE_BASE_LINUX_TOOL_HELPERS_H_ */
