/*
 * TTTech sysrepo-plugins common defines
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

#ifndef COMMON_DEFINES_H_
#define COMMON_DEFINES_H_

#define LIT_SET_STR                                 "set"
#define LIT_GET_STR                                 "get"
#define LIT_ADD_STR                                 "add"
#define LIT_CREATE_STR                              "create"
#define LIT_DELETE_STR                              "delete"
#define LIT_READ_STR                                "read"

#define LIT_XPATH_STR                               "xpath"
#define LIT_ENTRY_STR                               "entry"
#define LIT_ENTRIES_STR                             "entries"
#define LIT_PARAMS_STR                              "parameters"
#define LIT_VAL_STR                                 "value"
#define LIT_TABLE_STR                               "table"
#define LIT_LIST_STR                                "list"
#define LIT_LEAF_STR                                "leaf"
#define LIT_NAME_STR                                "name"
#define LIT_ID_STR                                  "id"
#define LIT_CNT_STR                                 "count"

#define REM_OP_NOT_SUPPORTED_FOR_INTERFACE_STR      "(%s) remove operation is not supported for (%s) interface"

#define INF_MODULE_INIT_SUCCESS_STR                 "%s plugin initialized successfully."
#define INF_MODULE_CLEANUP_STR                      "%s plugin plugin cleanup finished."

#define ERR_MODULE_INIT_FAILED_STR                  "%s plugin initialization failed: %s."
#define ERR_FORMING_ITERATOR_FAILED_STR             "Forming iterator for change callback failed."
#define ERR_FORMING_ITERATOR_FAILED_XPATH_STR       "Forming iterator for change callback failed for '%s'."
#define ERR_SR_ERROR_MSG_STR                        "%s: %s." // xpath: sr_strerror(rc)
#define ERR_KEY_VALUE_FAILED_STR                    "Failed to get key value."
#define ERR_KEY_VALUE_FAILED_XPATH_STR              "Failed to get key value for '%s'."
#define ERR_KEY_VALUE_ERR_STR                       "Failed to get %s key value inside %s list"
#define ERR_KEY_VALUE_ERR_XPATH_STR                 "Failed to get '%s' key value from '%s' list at xpath '%s'."
#define ERR_REM_OP_NOT_SUPPORTED_STR                "Remove operation is not supported"
#define ERR_REM_OP_NOT_SUPPORTED_XPATH_STR          "Remove operation is not supported for '%s'."
#define ERR_NOT_CREATED_ROOT_ELEMENT_STR            "root element is not created"
#define ERR_LIBYANG_CONTEXT_HANDLER_STR             "libyang context handler (ctx) is NULL, unable to load module"
#define ERR_NOT_CREATED_ELEMENT_STR                 "element is not created"
#define ERR_MISSING_ELEMENT_STR                     "Expected element is missing"
#define ERR_MISSING_ELEMENT_XPATH_STR               "Expected element is missing at '%s'."
#define ERR_MOD_OP_NOT_SUPPORTED_STR                "Modifications are not supported"
#define ERR_DEL_OP_NOT_SUPPORTED_STR                "Delete operation is not supported"
#define ERR_DEL_OP_NOT_SUPPORTED_XPATH_STR          "Delete operation is not supported for %s"
#define ERR_MOD_OP_NOT_SUPPORTED_XPATH_STR          "Modifications are not supported for '%s'."
#define ERR_OP_NOT_SUPPORTED_XPATH_STR              "ADD or REM operation not supported for '%s'."
#define ERR_ADD_REM_MOD_OP_NOT_SUPPORTED_STR        "Add, remove and modify operations are not supported"
#define ERR_ADD_REM_MOD_OP_NOT_SUPPORTED_XPATH_STR  "Add, remove and modify operations are not supported for %s"
#define ERR_MSG_FILE_OPEN_FAILED_STR                "Failed to open file!"
#define ERR_MSG_STARTUP_FILL_FAILED_STR             "Writing startup configuration to file failed!"
#define ERR_MSG_EMPTY_STARTUP_DS_STR                "Startup datastore is empty."
#define ERR_MSG_UNABLE_TO_LOCATE_STARTUP_DS_STR     "Unable to locate startup datastore."
#define ERR_MSG_LOAD_MODULE_STR                     "Failed to load the module!"
#define ERR_PORT_NOT_OPEN_STR                       "Port could not be open."
#define ERR_PORT_NOT_OPEN_XPATH_STR                 "Port could not be open for '%s'."
#define ERR_FAILED_TO_GET_ITEM_STR                  "Failed to get item value."
#define ERR_FAILED_TO_GET_ITEM_XPATH_STR            "Failed to get %s item."
#define ERR_FAILED_TO_GET_ITEMS_XPATH_STR           "Failed to get %s items."
#define ERR_FAILED_SET_OBJ_STR                      "Failed to set '%s' object value."
#define ERR_FAILED_GET_OBJ_STR                      "Failed to get '%s' object value."
#define ERR_MEMORY_ALLOC_FAILED_STR                 "Memory allocation failed."
#define ERR_MEMORY_ALLOC_FAILED_FOR_STR             "Memory allocation failed for '%s'."
#define ERR_GET_FUNC_FAILED_STR                     "Getter function for '%s' for port '%s' failed."
#define ERR_SET_FUNC_FAILED_STR                     "Setter function for '%s' for port '%s' failed."
#define ERR_SET_FUNC_FAILED_XPATH_STR               "Setter function failed for '%s'."
#define ERR_GET_FUNC_FAILED_XPATH_STR               "Getter function failed for '%s'."
#define ERR_MSG_AND_EXPL_SHT_FMT_STR                "Failed to %s '%s' %s (%s)."
#define ERR_MSG_EXT_FMT_STR                         "Failed to %s '%s' %s for %s '%s'."
#define ERR_MSG_AND_EXPL_EXT_FMT_STR                "Failed to %s '%s' %s for %s '%s' (%s)."
#define ERR_SETTER_EXT_FMT_STR                      "Failed to set '%s' %s for %s '%s' (%s)."
#define ERR_GETTER_EXT_FMT_STR                      "Failed to get '%s' %s for %s '%s' (%s)."
#define ERR_INVALID_VALUE_XPATH_STR                 "Invalid value for '%s'."
#define ERR_FUNCTION_FAILED_STR                     "Function %s failed."
#define ERR_APPLYING_CHANGES_FAILED_MSG             "Applying current configuration failed inside function"
#define ERR_FAILED_GET_SUBTREE_STR                  "Failed to get subtree for '%s'."
#define ERR_DELETED_NODE_FAILED_STR                 "Failed to get deleted %s."
#define ERR_ADDED_NODE_FAILED_STR                   "Failed to get added %s."
#define ERR_ALLOCATION_FAILED_STR                   "Failed to allocate %s entries."
#define ERR_BRIDGE_NAMES_STR                        "Failed to get bridge names."
#define ERR_BRIDE_NAMES_FAILED_STR                  "Failed to get bridge names inside %s function."
#define ERR_INTERFACE_NAMES_FAILED_STR              "Failed to get interface names inside %s function."
#define ERROR_MSG_FUN_NODE_EL_AND_MSG_STR           "%s(): %s %s."
#define ERR_UNSUPPORTED_CASE                        "Unsupported case (%s)."

#define ERR_TSN_MSG_FAIL_XPATH_STR                  "Failed to %s %s at xpath: '%s' - (%s)."
#define ERR_TSN_MSG_FAIL_STR                        "Failed to %s %s %s - (%s)."

#define ERR_FEATURE_NOT_SUPPORTED_XPATH_STR         "The feature '%s' is not supported for '%s'."
#define ERR_GRANULARITY_INTERVALS_STR               "time-interval-value needs to be an integer multiple of tick-granularity(='%u') at '%s'"

#define ERR_ARGUMENT_OF_ROUTINE_STR                 "Value passed as argument of %s routine is NULL!"
#define ERR_NEW_CONNECTION_ROUTINE_STR              "Opening new connection for running datastore refresh inside %s routine failed!"
#define ERR_NEW_SESSION_FAILED_STR                  "Starting new session with running datastore inside %s routine failed!"
#define ERR_REFRESHING_VALUE_STR                    "Refreshing %s %s in running datastore inside routine %s failed!"
#define ERR_DELETING_VALUE_STR                      "Deleting %s %s from running datastore inside routine %s for port %s failed!"
#define ERR_APPLYING_CHANGES_FAILED_STR             "Applying changes made in %s routine failed!\n"
#define ERR_CREATING_THREAD_FAILED_XPATH_STR        "Creating refresh thread failed for '%s'."

#define ERR_SESSION_CTX_FAILED_STR                  "Failed to get session connection libyang context!"
#define ERR_REPLACE_CONFIG_FAILED_STR               "Failed to replace current configuration!"
#define ERR_SWITCH_DATASTORE_FAILED_STR             "Failed to swith datstores in the session!"
#define ERR_COPY_DATASTORE_FAILED_STR               "Failed to copy dastore to the current session!"

#define DBG_FUNCTION_CALL_MSG_STR                   "%s(): Function called for xpath: '%s'."
#define DBG_MSG_FUN_CALLED_STR                      "DEBUG: %s(): is called."
#define DBG_APPLYING_CHANGES_MSG                    "Applying current configuration inside function:"
#define DBG_DISCARDING_CHANGES_MSG                  "Discarding current configuration inside function:"

#define YANG_REPOSITORY_PATH_STR                    "/etc/netopeer2/yang"

#define LONG_STR_LEN                                600
#define MAC_ADDR_B_LEN                              8

/* Convert boolean valu to the string 'true' or 'false' */
#define TRUE_STR    "true"
#define FALSE_STR   "false"
#define bool_to_str(bvalue) ((bvalue) ? TRUE_STR : FALSE_STR)

#endif
