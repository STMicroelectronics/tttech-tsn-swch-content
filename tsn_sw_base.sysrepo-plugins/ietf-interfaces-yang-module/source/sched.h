/*
 * TTTech ietf-interfaces-yang-module
 * Copyright(c) 2018TTTech Computertechnik AG.
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
 * Contact: https://tttech.com * support@tttech.com
 * TTTech Computertechnik AG, Schoenbrunnerstrasse 7, 1040 Vienna, Austria
 */

#ifndef SCHED_H_
#define SCHED_H_

/* common includes */
#include "common_defines.h"
#include "common.h"
/* sysrepo includes */
#include <sysrepo.h>
#include <sysrepo/values.h>
#include <sysrepo/xpath.h>
/* libyang includes */
#include <libyang/tree_data.h>
#include <libyang/tree_schema.h>
/* libbase includes */
#include "libbase/base_defines.h"
#include "libbase/base_prints.h"
#include "libbase/base_private_translators.h"
#include "libbase/base_linux_tool_helpers.h"
#include "libbase/base_interfaces_linux_tools.h"
/* libtsn includes */
#include "libtsn/tsn.h"
#include "libtsn/tsn_st.h"
#include "libtsn/tsn_def.h"
#include "libtsn/tsn_error.h"

#define SCHED_MODULE_NAME_STR               "ieee802-dot1q-sched"
#define SCHED_PREFIX_STR                    "sched"
#define SCHED_NS_STR                        "urn:ieee:std:802.1Q:yang:ieee802-dot1q-sched"

/* containers, tables, lists, leafs names */
#define SCHED_MAX_SDU_TABLE_STR             "max-sdu-table"
#define SCHED_TRAFFIC_CLASS_STR             "traffic-class"
#define SCHED_QUEUE_MAX_SDU_STR             "queue-max-sdu"
#define SCHED_TRANSMISSION_OVERRUN_STR      "transmission-overrun"

#define SCHED_GATE_PARAMETERS_STR           "gate-parameters"
#define SCHED_GATE_ENABLED_STR              "gate-enabled"
#define SCHED_ADMIN_GATE_STATES_STR         "admin-gate-states"
#define SCHED_ADMIN_CTRL_LIST_LEN_STR       "admin-control-list-length"
#define SCHED_ADMIN_CTRL_LIST_STR           "admin-control-list"
#define SCHED_OPER_GATE_STATES_STR          "oper-gate-states"
#define SCHED_OPER_CTRL_LIST_LEN_STR        "oper-control-list-length"
#define SCHED_OPER_CTRL_LIST_STR            "oper-control-list"
#define SCHED_INDEX_STR                     "index"
#define SCHED_OP_NAME_STR                   "operation-name"

#define SCHED_SGS_PARAMS_STR                "sgs-params"
#define SCHED_SHM_PARAMS_STR                "shm-params"
#define SCHED_SRM_PARAMS_STR                "srm-params"
#define SCHED_GATE_STATES_VALUE_STR         "gate-states-value"
#define SCHED_TIME_INTERVAL_VALUE_STR       "time-interval-value"

#define SCHED_ADMIN_CYCLE_TIME_STR          "admin-cycle-time"
#define SCHED_OPER_CYCLE_TIME_STR           "oper-cycle-time"
#define SCHED_ACT_NUMERATOR_STR             "numerator"
#define SCHED_ACT_DENOMINATOR_STR           "denominator"
#define SCHED_ADMIN_CYCLE_TIME_EXT_STR      "admin-cycle-time-extension"
#define SCHED_OPER_CYCLE_TIME_EXT_STR       "oper-cycle-time-extension"

#define SCHED_ADMIN_BASE_TIME_STR           "admin-base-time"
#define SCHED_OPER_BASE_TIME_STR            "oper-base-time"
#define SCHED_CONFIG_CHANGE_STR_TIME_STR    "config-change-time"
#define SCHED_ABT_SECONDS_STR               "seconds"
#define SCHED_ABT_FRACT_SECONDS_STR         "fractional-seconds"
#define SCHED_TICK_GRANULARITY_STR          "tick-granularity"

#define SCHED_CURRENT_TIME_STR              "current-time"
#define SCHED_CONFIG_PENDING_STR            "config-pending"
#define SCHED_CONFIG_CHANGE_STR_ERROR_STR   "config-change-error"
#define SCHED_SUPPORTED_LIST_MAX_STR        "supported-list-max"
#define SCHED_CONFIG_CHANGE_STR             "config-change"

/* sched leaf types */
#define SCHED_SET_GATE_STATES_STR           "set-gate-states"
#define SCHED_SET_HOLD_MAC_STR              "set-and-hold-mac"
#define SCHED_SET_RELEASE_MAC_STR           "set-and-release-mac"

/* sched identity types */
#define SCHED_SET_GATE_STATES_IDENTITY_STR  "ieee802-dot1q-sched:set-gate-states"
#define SCHED_SET_HOLD_MAC_IDENTITY_STR     "ieee802-dot1q-sched:set-and-hold-mac"
#define SCHED_SET_RELEASE_MAC_IDENTITY_STR  "ieee802-dot1q-sched:set-and-release-mac"

/* sched specific error messages */
#define ERR_GCL_LEN_MSG_STR                 "gate control list length can not be 0 when operation is ADD for '%s'."
#define ERR_GCL_INDEX_OUT_OF_BONDS_STR      "index is out of bonds of admin control list length for '%s'!"
#define ERR_SCHED_NOT_SUPPORTED_STR         "sched is not supported on interface "
#define ERR_SCHED_MAP_FAILED_STR            "Translator failed to map operation-name."
#define ERR_ACL_WRONG_LEGTH_STR             "admin-control-list-length does not match the number of admin-control-list entries for port '%s'!"
#define ERR_ACL_WRONG_INDEX_STR             "admin-control-list indices are wrong for port '%s'!"
#define ERR_ACL_GET_STR                     "Failed to get admin-control-list for port '%s'!"
#define ERR_GATE_PARAMETERS_GET_STR         "Failed to get container gate-parameters for port '%s'!"
#define ERR_CONFIG_CHANGE_SET_STR           "Cannot trigger config change for port '%s' (%s)."
#define ERR_ACL_MAX_VALUE_STR               "Invalid number of admin-control-list for port '%s'! Values should be from 0 to 2047."
#define ERR_SCHED_CL_ENTRY_SET              "Failed to set '%s' for port '%s' (%s)."

#define GCL_ENTRIES 16 // WARNING: depends on resp. sysfs-file's size!
#define btoa(x) ((x) ? BASE_TRUE : BASE_FALSE)

/* sched */
typedef enum {
    SCHED_MAX_SDU_TABLE = 0,
    SCHED_TRAFFIC_CLASS,
    SCHED_QUEUE_MAX_SDU,
    SCHED_TRANSMISSION_OVERRUN,
    SCHED_GATE_PARAMETERS,
    SCHED_GATE_ENABLED,
    SCHED_ADMIN_GATE_STATES,
    SCHED_ADMIN_CTRL_LIST_LEN,
    SCHED_ADMIN_CTRL_LIST,
    SCHED_OPER_GATE_STATES,
    SCHED_OPER_CTRL_LIST_LEN,
    SCHED_OPER_CTRL_LIST,
    SCHED_INDEX,
    SCHED_OP_NAME,
    SCHED_SGS_PARAMS,
    SCHED_GATE_STATES_VALUE,
    SCHED_TIME_INTERVAL_VALUE,
    SCHED_ADMIN_CYCLE_TIME,
    SCHED_OPER_CYCLE_TIME,
    SCHED_ACT_NUMERATOR,
    SCHED_ACT_DENOMINATOR,
    SCHED_ADMIN_CYCLE_TIME_EXT,
    SCHED_OPER_CYCLE_TIME_EXT,
    SCHED_ADMIN_BASE_TIME,
    SCHED_OPER_BASE_TIME,
    SCHED_CONFIG_CHANGE_TIME,
    SCHED_ABT_SECONDS,
    SCHED_ABT_FRACT_SECONDS,
    SCHED_TICK_GRANULARITY,
    SCHED_CURRENT_TIME,
    SCHED_CONFIG_PENDING,
    SCHED_CONFIG_CHANGE_ERROR,
    SCHED_SUPPORTED_LIST_MAX,
    SCHED_CONFIG_CHANGE,
    SCHED_MAX_OBJ_NUMBER,
} sched_obj_index_t;

/* sched module xpath defines */

#define SCHED_INTERFACE_XPATH                               "/ietf-interfaces:interfaces/interface"

/* list max-sdu-table xpath */
#define SCHED_MAX_SDU_TRANSMISSION_OVERRUN_XPATH            \
        SCHED_INTERFACE_XPATH "[name='%s']/ieee802-dot1q-sched:max-sdu-table[traffic-class='%s']/ieee802-dot1q-sched:transmission-overrun"

/* container gate-parameters */
#define SCHED_GATE_PARAMS_XPATH                             SCHED_INTERFACE_XPATH "[name='%s']/ieee802-dot1q-sched:gate-parameters"
#define SCHED_GATE_PARAMS_OPER_GATE_STATES_XPATH            SCHED_GATE_PARAMS_XPATH "/ieee802-dot1q-sched:oper-gate-states"
#define SCHED_GATE_PARAMS_OPER_CYCLE_TIME_EXTEN_XPATH       SCHED_GATE_PARAMS_XPATH "/ieee802-dot1q-sched:oper-cycle-time-extension"
#define SCHED_GATE_PARAMS_OPER_CONTROL_LIST_LENGTH_XPATH    SCHED_GATE_PARAMS_XPATH "/ieee802-dot1q-sched:oper-control-list-length"
#define SCHED_GATE_PARAMS_TICK_GRANULARITY_XPATH            SCHED_GATE_PARAMS_XPATH "/ieee802-dot1q-sched:tick-granularity"
#define SCHED_GATE_PARAMS_CONFIG_PENDING_XPATH              SCHED_GATE_PARAMS_XPATH "/ieee802-dot1q-sched:config-pending"
#define SCHED_GATE_PARAMS_CONFIG_CHANGE_ERROR_XPATH         SCHED_GATE_PARAMS_XPATH "/ieee802-dot1q-sched:config-change-error"
#define SCHED_GATE_PARAMS_SUPPORTED_LIST_MAX_XPATH          SCHED_GATE_PARAMS_XPATH "/ieee802-dot1q-sched:supported-list-max"
#define SCHED_GATE_PARAMS_SCM_NUMERATOR_XPATH               SCHED_GATE_PARAMS_XPATH "/ieee802-dot1q-sched:supported-cycle-max/ieee802-dot1q-sched:numerator"
#define SCHED_GATE_PARAMS_SCM_DENOMINATOR_XPATH             SCHED_GATE_PARAMS_XPATH "/ieee802-dot1q-sched:supported-cycle-max/ieee802-dot1q-sched:denominator"
#define SCHED_GATE_PARAMS_SUP_INTRVL_MAX_XPATH              SCHED_GATE_PARAMS_XPATH "/ieee802-dot1q-sched:supported-interval-max"
#define SCHED_GATE_PARAMS_GATE_ENABLED_XPATH                SCHED_GATE_PARAMS_XPATH "/ieee802-dot1q-sched:gate-enabled"
#define SCHED_GATE_PARAMS_CONFIG_CHANGE_XPATH               SCHED_GATE_PARAMS_XPATH "/ieee802-dot1q-sched:config-change"
#define SCHED_GATE_PARAMS_CHILD_XPATH                       SCHED_GATE_PARAMS_XPATH "/*"
#define SCHED_GATE_PARAMS_ACL_CHILD_XPATH                   SCHED_GATE_PARAMS_XPATH "/admin-control-list/*"
#define SCHED_GATE_PARAMS_ACL_LENGTH_XPATH                  SCHED_GATE_PARAMS_XPATH "/ieee802-dot1q-sched:admin-control-list-length"
#define SCHED_GATE_PARAMS_CONTAINER_XPATH                   SCHED_GATE_PARAMS_XPATH
#define SCHED_GATE_PARAMS_ACL_INDEX_XPATH                   SCHED_GATE_PARAMS_XPATH "/ieee802-dot1q-sched:admin-control-list/ieee802-dot1q-sched:index"
#define SCHED_GATE_PARAMS_ACL_XPATH                         SCHED_GATE_PARAMS_XPATH "/ieee802-dot1q-sched:admin-control-list[index='%s']"
#define SCHED_GATE_PARAMS_ACL_OP_NAME_XPATH                 SCHED_GATE_PARAMS_ACL_XPATH "/ieee802-dot1q-sched:operation-name"
#define SCHED_GATE_PARAMS_ACL_PARAMS_GSV_XPATH              SCHED_GATE_PARAMS_ACL_XPATH "/ieee802-dot1q-sched:%s/ieee802-dot1q-sched:gate-states-value"
#define SCHED_GATE_PARAMS_ACL_PARAMS_TIV_XPATH              SCHED_GATE_PARAMS_ACL_XPATH "/ieee802-dot1q-sched:%s/ieee802-dot1q-sched:time-interval-value"
#define SCHED_GATE_PARAMS_OCL_INDEX_XPATH                   SCHED_GATE_PARAMS_XPATH "/ieee802-dot1q-sched:oper-control-list/ieee802-dot1q-sched:index"
#define SCHED_GATE_PARAMS_OCL_XPATH                         SCHED_GATE_PARAMS_XPATH "/ieee802-dot1q-sched:oper-control-list[index='%s']"
#define SCHED_GATE_PARAMS_OCL_OP_NAME_XPATH                 SCHED_GATE_PARAMS_OCL_XPATH "/ieee802-dot1q-sched:operation-name"
#define SCHED_GATE_PARAMS_OCL_PARAMS_GSV_XPATH              SCHED_GATE_PARAMS_OCL_XPATH "/ieee802-dot1q-sched:%s/ieee802-dot1q-sched:gate-states-value"
#define SCHED_GATE_PARAMS_OCL_PARAMS_TIV_XPATH              SCHED_GATE_PARAMS_OCL_XPATH "/ieee802-dot1q-sched:%s/ieee802-dot1q-sched:time-interval-value"
#define SCHED_GATE_PARAMS_ACT_NUMERATOR_XPATH               SCHED_GATE_PARAMS_XPATH "/ieee802-dot1q-sched:admin-cycle-time/ieee802-dot1q-sched:numerator"
#define SCHED_GATE_PARAMS_ACT_DENOMINATOR_XPATH             SCHED_GATE_PARAMS_XPATH "/ieee802-dot1q-sched:admin-cycle-time/ieee802-dot1q-sched:denominator"
#define SCHED_GATE_PARAMS_OCT_NUMERATOR_XPATH               SCHED_GATE_PARAMS_XPATH "/ieee802-dot1q-sched:oper-cycle-time/ieee802-dot1q-sched:numerator"
#define SCHED_GATE_PARAMS_OCT_DENOMINATOR_XPATH             SCHED_GATE_PARAMS_XPATH "/ieee802-dot1q-sched:oper-cycle-time/ieee802-dot1q-sched:denominator"
#define SCHED_GATE_PARAMS_ACT_EXTENSION_XPATH               SCHED_GATE_PARAMS_XPATH "/ieee802-dot1q-sched:admin-cycle-time-extension"
#define SCHED_GATE_PARAMS_ABT_SECONDS_XPATH                 SCHED_GATE_PARAMS_XPATH "/ieee802-dot1q-sched:admin-base-time/ieee802-dot1q-sched:seconds"
#define SCHED_GATE_PARAMS_ABT_NANO_SECONDS_XPATH            SCHED_GATE_PARAMS_XPATH "/ieee802-dot1q-sched:admin-base-time/ieee802-dot1q-sched:fractional-seconds"
#define SCHED_GATE_PARAMS_SECONDS_XPATH                     SCHED_GATE_PARAMS_XPATH "/ieee802-dot1q-sched:%s/ieee802-dot1q-sched:seconds"
#define SCHED_GATE_PARAMS_FRACTIONAL_SECONDS_XPATH          SCHED_GATE_PARAMS_XPATH "/ieee802-dot1q-sched:%s/ieee802-dot1q-sched:fractional-seconds"

#define SCHED_GATE_PARAMS_CONFIG_CHANGE_SUB_XPATH           SCHED_INTERFACE_XPATH "/ieee802-dot1q-sched:gate-parameters/config-change"
#define SCHED_GATE_PARAMS_GATE_ENABLED_SUB_XPATH            SCHED_INTERFACE_XPATH "/ieee802-dot1q-sched:gate-parameters/gate-enabled"

/**
 * @brief This function will check the status of link, admin-contro-list exist,
 * admin-base-time and admin-cycle-time elements value.
 *
 * All this informations are necessary for triggering config-change to 'true'.
 * If link is DOWN, it is not possible to trigger config-change to 'true'. If
 * configuration is not complete, it is not possible to trigger config-change
 * to 'true'. Complete configuration requires admin-control-list with all
 * elements, admin-base-time and admin-cycle-time elements value different from
 * '0' and admin-cycle-time-extension element.
 *
 * @param[in]   session         Automatically-created session that can be used
 *                              for obtaining changed data (e.g. by
 *                              sr_get_changes_iter call or sr_get_item -like
 *                              calls). Do not stop this session.
 * @param[in]   node            Pointer on sysrepo node structure element.
 * @param[in]   port            Pointer on port handler structure.
 * @param[in]   if_name         String with interface name.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
int validate_sched_configuration(sr_session_ctx_t *session, sr_val_t *node,
                                 struct port_hdl *port, char *if_name);

/**
 * @brief Creates a new entry for list "max-sdu-table" from ieee802-dot1q-sched
 * yang module.
 *
 * It is used for config data.
 *
 * @param[in]   parent          Pointer to struct lyd_node which is going to be
 *                              parent to created container.
 * @param[in]   ctx             libyang context handler.
 * @param[in]   if_name         Contains the name of the interface.
 * @param[in]   prio            Contains the priority value for max-sdu-table
 *                              entry.
 * @param[in]   traffic_class   Contains the traffic class value for
 *                              max-sdu-table entry.
 * @return      struct lyd_node* or NULL if element not created.
 */
struct lyd_node *new_max_sdu_entry(struct lyd_node *parent, struct ly_ctx *ctx,
                                   char *if_name, int prio, int traffic_class);

/**
 * @brief Creates a new container "gate-parameters" from ieee802-dot1q-sched
 * yang module.
 *
 * It is used for config data.
 *
 * @param[in]   parent          Pointer to struct lyd_node which is going to be
 *                              parent to created container.
 * @param[in]   ctx             libyang context handler.
 * @param[in]   if_name         Contains the name of the interface.
 * @return      struct lyd_node* or NULL if element not created.
 */
struct lyd_node *new_gate_parameters_container(struct lyd_node *parent,
                                               struct ly_ctx *ctx,
                                               char *if_name);

/**
 * @brief Creates a new container max-sdu-table for list 'interface' inside
 * ietf-interfaces yang module.
 *
 * This function is for state data from list 'interface'.
 *
 * @param[in]   parent          Pointer to an existing parent of the requested
 *                              nodes. Is NULL for top-level nodes. Caller is
 *                              supposed to append the requested nodes to this
 *                              data subtree and return either the original
 *                              parent or a top-level node.
 * @param[in]   interface_name  Name of the interface.
 * @param[in]   prio            Contains the value of the interface priority.
 * @param[in]   traffic_class   Contains the value of the interface traffic
 *                              class.
 * @return      Error code (SR_ERR_OK on success).
 */
int new_max_sdu_state_entry(struct lyd_node **parent, char *interface_name,
                            int prio, int traffic_class);

/**
 * @brief Creates a new container gate-parameters for list 'interface' inside
 * ietf-interfaces yang module.
 *
 * This function is for state data from list 'interface'.
 *
 * @param[in]   parent          Pointer to an existing parent of the requested
 *                              nodes. Is NULL for top-level nodes. Caller is
 *                              supposed to append the requested nodes to this
 *                              data subtree and return either the original
 *                              parent or a top-level node.
 * @param[in]   interface_name  Name of the interface.
 * @return      Error code (SR_ERR_OK on success).
 */
int new_gate_parameters_state_container(struct lyd_node **parent, char *interface_name);

/**
 * @brief Creates a new entry for list admin-control-list for list 'interface'
 * inside ietf-interfaces yang module.
 *
 * This function is for state data from list 'interface'.
 *
 * @param[in]   parent          Pointer to an existing parent of the requested
 *                              nodes. Is NULL for top-level nodes. Caller is
 *                              supposed to append the requested nodes to this
 *                              data subtree and return either the original
 *                              parent or a top-level node.
 * @param[in]   interface_name  Name of the interface.
 * @param[in]   i               Contains the value of the index element.
 * @param[in]   gcl             Pointer to TSN control entry structure.
 * @return      Error code (SR_ERR_OK on success).
 */
int new_oper_control_list_state_entry(struct lyd_node **parent,
                                      char *interface_name, int i,
                                      struct st_gate_control_entry *gcl);

/**
 * @brief Creates a new container  oper-cycle-time inside container
 * gate-parameteres for list 'interface' inside ietf-interfaces yang module.
 *
 * This function is for state data from list 'interface'.
 *
 * @param[in]   parent          Pointer to an existing parent of the requested
 *                              nodes. Is NULL for top-level nodes. Caller is
 *                              supposed to append the requested nodes to this
 *                              data subtree and return either the original
 *                              parent or a top-level node.
 * @param[in]   interface_name  Name of the interface.
 * @param[in]   st_rational     Pointer to TSN rational number structure.
 * @return      Error code (SR_ERR_OK on success).
 */
int new_state_oper_cycle_time(struct lyd_node **parent, char *interface_name,
                              struct tsn_st_rational_number *st_rational);

/**
 * @brief Creates a new container  oper-base-time, config-change-time and
 * current-time inside container gate-parameteres for list 'interface' inside
 * ietf-interfaces yang module.
 *
 * This function is for state data from list 'interface'.
 *
 * @param[in]   parent          Pointer to an existing parent of the requested
 *                              nodes. Is NULL for top-level nodes. Caller is
 *                              supposed to append the requested nodes to this
 *                              data subtree and return either the original
 *                              parent or a top-level node.
 * @param[in]   interface_name  Name of the interface.
 * @param[in]   st_time         Pointer to TSN ptp time structure.
 * @param[in]   container_name  Name of the container to be created.
 * @return      Error code (SR_ERR_OK on success).
 */
int new_state_oper_base_time(struct lyd_node **parent, char *interface_name,
                             struct tsn_st_ptp_time *st_time,
                             char *container_name);

/**
 * @brief Callback to be called by the event of changing content of node with
 * "/ietf-interfaces:interfaces/interface/ieee802-dot1q-sched:gate-parameters/ieee802-dot1q-sched:admin-control-list-length"
 * xpath.
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
int interface_sched_acl_length_cb(sr_session_ctx_t *session,
                                  const char *module_name, const char *xpath,
                                  sr_event_t event, uint32_t request_id,
                                  void *private_data);

/**
 * @brief Callback to be called by the event of changing content of node with
 * "/ietf-interfaces:interfaces/interface/ieee802-dot1q-sched:gate-parameters/ieee802-dot1q-sched:admin-control-list"
 * xpath.
 *
 * This callback covers the delete and create operation for list entry.
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
int interface_sched_acl_cb(sr_session_ctx_t *session, const char *module_name,
                           const char *xpath, sr_event_t event,
                           uint32_t request_id, void *private_data);

/**
 * @brief Callback to be called by the event of changing content of node with
 * "/ietf-interfaces:interfaces/interface/ieee802-dot1q-sched:gate-parameters/ieee802-dot1q-sched:admin-control-list"
 * xpath.
 *
 * This callback covers the modify operation for list entry.
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
int interface_sched_acl_mod_cb(sr_session_ctx_t *session,
                               const char *module_name, const char *xpath,
                               sr_event_t event, uint32_t request_id,
                               void *private_data);

/**
 * @brief Callback to be called by the event of changing content of node with
 * "/ietf-interfaces:interfaces/interface/ieee802-dot1q-sched:max-sdu-table/ieee802-dot1q-sched:queue-max-sdu",
 * "/ietf-interfaces:interfaces/interface/ieee802-dot1q-sched:gate-parameters/ieee802-dot1q-sched:gate-enabled",
 * "/ietf-interfaces:interfaces/interface/ieee802-dot1q-sched:gate-parameters/ieee802-dot1q-sched:admin-gate-states",
 * "/ietf-interfaces:interfaces/interface/ieee802-dot1q-sched:gate-parameters/ieee802-dot1q-sched:admin-cycle-time/ieee802-dot1q-sched:numerator",
 * "/ietf-interfaces:interfaces/interface/ieee802-dot1q-sched:gate-parameters/ieee802-dot1q-sched:admin-cycle-time/ieee802-dot1q-sched:denominator",
 * "/ietf-interfaces:interfaces/interface/ieee802-dot1q-sched:gate-parameters/ieee802-dot1q-sched:admin-cycle-time-extension",
 * "/ietf-interfaces:interfaces/interface/ieee802-dot1q-sched:gate-parameters/ieee802-dot1q-sched:admin-base-time/ieee802-dot1q-sched:seconds",
 * "/ietf-interfaces:interfaces/interface/ieee802-dot1q-sched:gate-parameters/ieee802-dot1q-sched:admin-base-time/ieee802-dot1q-sched:fractional-seconds",
 * "/ietf-interfaces:interfaces/interface/ieee802-dot1q-sched:gate-parameters/ieee802-dot1q-sched:config-change"
 * xpaths.
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
int interface_sched_elements_cb(sr_session_ctx_t *session,
                                const char *module_name, const char *xpath,
                                sr_event_t event, uint32_t request_id,
                                void *private_data);

/**
 * @brief Callback to be called by the event of changing content of node with
 * "/ietf-interfaces:interfaces/interface/ieee802-dot1q-sched:gate-parameters/ieee802-dot1q-sched:config-change" and
 * "/ietf-interfaces:interfaces/interface/ieee802-dot1q-sched:gate-parameters/ieee802-dot1q-sched:gate-enabled"
 * xpaths.
 *
 * This handles both config-change and gate-enabled changes, from the same
 * subscription (!), because both can be used to trigger actual List Config
 * state machine changes, depending on their previous values. Private data is
 * used to keep track of collected triggers from CHANGE event call to DONE
 * event call. Successful triggers need to be reset back to false in datastore,
 * for which a separate config-change helper thread is used.
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
int interface_sched_config_change_cb(sr_session_ctx_t *session,
                                     const char *module_name,
                                     const char *xpath, sr_event_t event,
                                     uint32_t request_id, void *private_data);

/**
 * @brief Fix config-change values in datastore at initialization time.
 *
 * This is used at initialization time to reset config-change values
 * for which gate-enabled is also true before datastore settings
 * are applied to system.
 *
 * @param[in]   session         Session to use for manipulating datastore.
 * @param[in]   datastore       Datastore to fix.
 * @return      Error code (SR_ERR_OK on success).
 */
int interface_sched_fix_datastore(sr_session_ctx_t *session,
                                  sr_datastore_t datastore);

#endif /* SCHED_H_ */
