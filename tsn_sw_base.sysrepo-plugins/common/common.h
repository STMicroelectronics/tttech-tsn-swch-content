/*
 * TTTech sysrepo-plugins common defines
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

#include <stdarg.h>
#include <stdint.h>
/* libyang includes */
#include <libyang/libyang.h>
/* sysrepo includes */
#include <sysrepo.h>
//#include <sysrepo/plugins.h>
#include <sysrepo/values.h>
#include <sysrepo/xpath.h>
/* libbase includes */
#include "libbase/base_prints.h"
#include "libbase/base_private_translators.h"
/* common includes */
#include "common_defines.h"

/**
 * @brief This function gets key value from xpath.
 *
 * @param[in]   session         Automatically-created session that can be used
 *                              for obtaining changed data (e.g. by
 *                              sr_get_changes_iter call or sr_get_item -like
 *                              calls). Do not stop this session.
 * @param[in]   xpath           Xpath of node.
 * @param[in]   node_name       Contains the name of node.
 * @param[in]   key_name        Contains the name of key.
 * @param[in]   st              State of xpath parsing. User must not modify
 *                              nor rely on the content of the structure.
 * @param[out]  result          Pointer to char that represent result.
 * @return      EXIT_FAILURE or EXIT_SUCCESS
 */
int get_key_value(sr_session_ctx_t *session, char *xpath, char *node_name,
                  char *key_name, sr_xpath_ctx_t *st, char *result);

/**
 * @brief This function will create a new child element in libyang subtree.
 *
 * @param[in]   parent          Pointer to struct lyd_node which is going to be
 *                              parent to created node.
 * @param[in]   module          Main schema node structure representing YANG
 *                              module.
 * @param[in]   name            Contains the name of node to be created.
 * @param[in]   value           Contains the value of node to be created.
 * @return      EXIT_FAILURE or EXIT_SUCCESS
 */
int new_node(struct lyd_node *parent, const struct lys_module *module,
             char *name, char *value);

/**
 * @brief This function will create a new container or list in libyang tree.
 *
 * @param[in]   parent          Pointer to struct lyd_node which is going to be
 *                              parent to created container/list.
 * @param[in]   module          Main schema node structure representing YANG
 *                              module.
 * @param[out]  container       Double pointer to struct lyd_node that
 *                              represents container/list to be created.
 * @param[in]   container_name  Contains the name of container/list to be
 *                              created.
 * @return      EXIT_FAILURE or EXIT_SUCCESS
 */
int new_container(struct lyd_node *parent, struct lyd_node** container,
                  const struct lys_module *module, char *container_name);

/**
 * @brief This function will create a root element in libyang tree.
 *
 * @param[in]   ctx             libyang context handler.
 * @param[out]  root            Double pointer to struct lyd_node that
 *                              represents root element to be created.
 * @param[out]  module          Double pointer to struct lyd_node that
 *                              represents main schema node structure
 *                              representing YANG module.
 * @param[in]   module_name     The module within the root element will be
 *                              created.
 * @param[in]   root_name       Contains the name of root element.
 * @param[in]   repository_path Path to the repository where the module is
 *                              located.
 * @return      EXIT_FAILURE or EXIT_SUCCESS
 */
int create_root(struct ly_ctx **ctx, struct lyd_node **root,
                const struct lys_module **module, char *module_name,
                char *root_name, char *repository_path);

/**
 * @brief This function creates xpath for sr_val_t node.
 *
 * The variable number of arguments represents the variables that will be
 * written to the result based on the format.
 *
 * @param[out]  result          Pointer to char that represents result value.
 * @param[in]   format          Pointer to char that represents format.
 */
void fill_xpath(char *result, const char *format, ...);

/**
 * @brief This function will create a new list of key elements in subtree.
 *
 * @param[in]   parent          Pointer to struct lyd_node which is going to be
 *                              parent to created list.
 * @param[in]   module          Main schema node structure representing YANG
 *                              module.
 * @param[in]   key_name        Contains a list of node names inside list.
 * @param[in]   key_value       Contains a list of node values inside list.
 * @param[in]   key_num         Contains a number of key values of list that is
 *                              going to be created.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
int create_list_keys(struct lyd_node *parent, const struct lys_module *module,
                     char key_name[][MAX_STR_LEN],
                     char key_value[][MAX_STR_LEN],
                     int key_num);

/**
 * @brief This function checks if the file with specified path is empty.
 *
 * @param[in]   path_to_file    Path to the repository where the file is located.
 * @return      Integer.
 */
int check_file(char *path_to_file);

/**
 * @brief Callback to be called by the event of changing content of node with
 * specified xpath.
 *
 * Subscribe to it by sr_module_change_subscribe call.
 *
 * @param[in]   session         Implicit session (do not stop) with information
 *                              about the changed data (retrieved by
 *                              sr_get_changes_iter) the event originator
 *                              session IDs.
 * @param[in]   module_name     Name of the module where the change has
 *                              occurred.
 * @param[in]   xpath           XPath used when subscribing, NULL if the whole
 *                              module was subscribed to.
 * @param[in]   event           Type of the notification event that has
 *                              occurred.
 * @param[in]   request_id      Request ID unique for the specific module_name.
 *                              Connected events for one request (SR_EV_CHANGE
 *                              and SR_EV_DONE, for example) have the same
 *                              request ID.
 * @param[in]   private_data    Private context opaque to sysrepo, as passed to
 *                              sr_module_change_subscribe call.
 * @return      Error code (SR_ERR_OK on success).
 */
int mod_not_supported_cb(sr_session_ctx_t *session, const char *module_name,
                         const char *xpath, sr_event_t event,
                         uint32_t request_id, void *private_data);

/**
 * @brief Callback to be called by the event of changing content of node with
 * specified xpath.
 *
 * Subscribe to it by sr_module_change_subscribe call.
 *
 * @param[in]   session         Implicit session (do not stop) with information
 *                              about the changed data (retrieved by
 *                              sr_get_changes_iter) the event originator
 *                              session IDs.
 * @param[in]   module_name     Name of the module where the change has
 *                              occurred.
 * @param[in]   xpath           XPath used when subscribing, NULL if the whole
 *                              module was subscribed to.
 * @param[in]   event           Type of the notification event that has
 *                              occurred.
 * @param[in]   request_id      Request ID unique for the specific module_name.
 *                              Connected events for one request (SR_EV_CHANGE
 *                              and SR_EV_DONE, for example) have the same
 *                              request ID.
 * @param[in]   private_data    Private context opaque to sysrepo, as passed to
 *                              sr_module_change_subscribe call.
 * @return      Error code (SR_ERR_OK on success).
 */
int add_rem_mod_not_supported_cb(sr_session_ctx_t *session,
                                 const char *module_name, const char *xpath,
                                 sr_event_t event, uint32_t request_id,
                                 void *private_data);

/**
 * @brief This function returns the item value for xpath format with variable
 * number of arguments to fill the proper xpath.
 *
 * @param[in]   session         Automatically-created session that can be used
 *                              for obtaining changed data (e.g. by
 *                              sr_get_changes_iter call or sr_get_item -like
 *                              calls). Do not stop this session.
 * @param[in]   value           Double Pointer to structure for store value.
 * @param[in]   xpath_format    Xpath format which will be filled with variable
 *                              number of arguments.
 * @return      EXIT_FAILURE or EXIT_SUCCESS.
 */
int get_item_value_cb(sr_session_ctx_t *session, sr_val_t **value,
                      const char *xpath_format, ...);

/**
 * @brief Translates string 'value' into new string from 'map' if first string
 * from 'map' pair is same as string 'value'.
 *
 * @param[out]  value           String for manipulation.
 * @param[in]   map             Collection of string pairs key and new value.
 * @param[in]   bvalue          Bool value for manipulation.
 * @return      EXIT_FAILURE or EXIT_SUCCESS
 */
int translate_string_to_bool(char value[MAX_STR_LEN],
                             char map[][2][MAX_STR_LEN], bool *bvalue);

/**
 * @brief Translates string from 'values' into new string from 'map' if first
 * string from 'map' pair is same as string from 'values'.
 *
 * @param[out]  values          Strings for manipulation.
 * @param[in]   map             Collection of string pairs key and new value.
 * @return      EXIT_FAILURE or EXIT_SUCCESS
 */
int translate_strings_using_map(char values[][MAX_STR_LEN],
                                char map[][2][MAX_STR_LEN]);

/**
 * @brief Converts given MAC address to string
 *
 * @param[in]   mac_addr_b      Input MAC address (byte array)
 * @param[out]  mac_addr_str    Output MAC address  (string)
 * @param[in]   delim           Delimter to be placed between octets ('\0' if
 *                              there's no delimiter)
 */
void stringify_mac_addr(uint8_t mac_addr_b[MAC_ADDR_B_LEN],
                        char *mac_addr_str, char delim);
