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

#ifndef INCLUDE_BASE_PRINTS_H_
#define INCLUDE_BASE_PRINTS_H_

#include <stdio.h>
#include "base_defines.h"

/* typedef for client printing functions */
typedef void (*print_fn_t)(const char *format, ...);

/* debug messages */
#define DEBUG_MSG_WITH_ONE_PARAM              "DEBUG: %s.\n"
#define DEBUG_MSG_WITH_TWO_PARAM              "DEBUG: %s %s.\n"
#define DEBUG_MSG_FUN_CALLED                  "DEBUG: %s(): is called.\n"
#define DEBUG_MSG_FUN_CALLED_WITH_ONE_PARAM   "DEBUG: %s(): (%s) is called.\n"
#define DEBUG_MSG_FUN_CALLED_WITH_TWO_PARAM   "DEBUG: %s(): (%s)(%s) is called.\n"
#define DEBUG_MSG_FUN_CALLED_WITH_THREE_PARAM "DEBUG: %s(): (%s)(%s)(%s) is called.\n"
#define DEBUG_MSG_FUN_CALLED_WITH_FOUR_PARAM  "DEBUG: %s(): (%s)(%s)(%s)(%s) is called.\n"
#define DEBUG_MSG_FUN_NAME_AND_MSG            "DEBUG: %s(): %s.\n"
#define DEBUG_MSG_FUN_NAME_AND_TWO_MSG        "DEBUG: %s(): %s %s.\n"
#define DEBUG_MSG_SYSTEM_CALL_CMD             "\t  |--> %s() for command:\n\t  %s \n"

/* error messages */
#define ERROR_MSG_FUN_AND_MSG               "%s(): %s."
#define SNMP_MSG_FUN_AND_MSG                "%s(): %s.\n"
#define ERROR_MSG_FUN_XML_EL_AND_MSG        "%s(): %s %s."
#define SNMP_MSG_FUN_XML_EL_AND_MSG         "%s(): %s %s.\n"
#define ERROR_MSG_FUN_XML_TWO_EL_AND_MSG    "%s(): %s and %s %s."
#define SNMP_MSG_FUN_MSG_AND_TWO_OBJ        "%s(): %s %s and %s.\n"
#define ERROR_MSG_MOD_FUNC_MSG              "Module \"%s\" %s(): %s"
#define ERROR_MSG_MOD_FUNC_EL_MSG           "Module '%s' %s(): %s %s"
#define ERROR_MSG_SETTER_FAILED             "Setter function failed."
#define ERROR_MSG_GETTER_FAILED             "Getter function failed."
#define ERROR_MSG_FUN_SETTER_FAILED         "%s(): Setter function failed."
#define ERROR_MSG_FUN_GETTER_FAILED         "%s(): Getter function failed."
#define WARNING_MSG_FUN_AND_LEAF            "%s(): leaf (%s) is not created."
#define ERR_MSG_NO_BRIDGES                  "There is no one bridge on the system."
#define ERR_MSG_MORE_BRIDGES                "There is more than one bridge on the system."
#define ERR_MSG_INVALID_VALUE               "Invalid value for element"
#define ERR_MSG_ETH_NOT_SUPPORTED           "UNSUPPORTED for baseT link mode."
#define ERR_MSG_STREAM_OPEN_FAILED          "failed to open stream!"
#define ERR_MSG_REAL_MAC_ADDR_FAILED        "Real MAC address is not retrieved for port %s!"
#define ERROR_MSG_ACL_MISSING               "It is not possible to set ConfigChange to True while configuration is not complete. AdminControlList is missing for interface '%s'."
#define ERROR_MSG_LINK_NOT_DETECTED         "It is not possible to set ConfigChange to True while link is not detected for interface '%s'."
#define ERROR_MSG_ADMIN_TIME_VALUES         "It is not possible to set ConfigChange to True while %s has value '0' for interface '%s'."
#define ERROR_MSG_PORT_MAP_MISSING          "port-map is missing for vids '%s'."
#define ERROR_MSG_DATABASE_ID               "Currently only \"1\" is supported as database-id."
#define ERROR_MSG_DESC_VID_INTERVAL         "System does not support descending vid intervals."
#define ERROR_MSG_INVALID_VID               "System does not support vid greater than 4095 and lower than 0."
#define ERROR_MSG_PVID_IN_VIDS              "The pvid is in vids string - this is not allowed."
#define ERROR_MSG_NO_PVID                   "The pvid does not exist."
#define ERROR_MSG_TOO_MANY_VIDS             "The number of different vids needs to be less than %u."
#define ERROR_MSG_STORED_LINK_MODES         "Failed to store advertise link modes."
#define ERROR_MSG_NULL_VIDS                 "vids string is NULL."
#define ERROR_MSG_TOO_LONG_VIDS             "vids string is too long (more than MAX_STR_LEN)."

// used in debugging process
#ifdef DEBUG
    #define base_print_debug(...) do { fprintf(stderr, __VA_ARGS__); } while (0)
#else
    #define base_print_debug(...)
#endif

/**
 * @brief This function will execute client error printing function which
 * address is stored in error printing pointer.
 *
 * @param[in]   error_message   The error message which will be printed.
 */
void base_execute_print_error_fn(const char *error_message);

/**
 * @brief This function will execute client warning printing function which
 * address is stored in warning printing pointer.
 *
 * @param[in]   error_message   The error message which will be printed.
 */
void base_execute_print_warning_fn(const char *error_message);

/**
 * @brief This function is wrapper for the base_execute_print_error_fn
 * function.
 *
 * @param[in]   format          Format of the message which will be printed.
 * @param[in]   ...             The parameters for the format.
 */
void base_print_error(const char *format, ...);

/**
 * @brief This function is wrapper for the base_execute_print_warning_fn
 * function.
 *
 * @param[in]   format          Format of the message which will be printed.
 * @param[in]   ...             The parameters for the format.
 */
void base_print_warning(const char *format, ...);

/**
 * @brief This function will set global pointer as address of client error
 * printing function.
 *
 * This pointer is used in the base_execute_print_error_fn function.
 *
 * @param[in]   custom_print_fn_ptr     The pointer of client error printing
 *                                      function.
 */
void set_base_print_error_fn_ptr(print_fn_t custom_print_fn_ptr);

/**
 * @brief This function will set global pointer as address of client warning
 * printing function.
 *
 * This pointer is used in the base_execute_print_warning_fn function.
 *
 * @param[in]   custom_print_fn_ptr     The pointer of client warning printing
 *                                      function.
 */
void set_base_print_warning_fn_ptr(print_fn_t custom_print_fn_ptr);

#endif
