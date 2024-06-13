/*
 * TTTech ieee802-dot1q-bridge-yang-module
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

#ifndef STREAM_FILTER_GATES_H_
#define STREAM_FILTER_GATES_H_

#include <pthread.h>
/* common includes */
#include "common_defines.h"
#include "common.h"
#include "config_change.h"
/* sysrepo includes */
#include <sysrepo.h>
#include <sysrepo/values.h>
#include <sysrepo/xpath.h>
/* libbase includes */
#include "libbase/base_defines.h"
#include "libbase/base_private_translators.h"
#include "libbase/base_linux_tool_helpers.h"
#include "libbase/base_q_bridge_linux_tools.h"
/* libtsn includes */
#include "libtsn/tsn.h"
#include "libtsn/tsn_psfp.h"
#include "libtsn/tsn_def.h"

#define SFG_MODULE_NAME_STR                         "ieee802-dot1q-stream-filters-gates"
#define SFG_PREFIX_STR                              "sfsg"

/* containers, tables, lists, leafs names */
#define SFG_STREAM_FILTERS_STR                      "stream-filters"
#define SFG_STREAM_FILTER_INSTANCE_STR              "stream-filter-instance-table"
#define SFG_STREAM_FILTER_INSTANCE_ID_STR           "stream-filter-instance-id"
#define SFG_STREAM_HANDLE_STR                       "stream-handle"
#define SFG_PRIORITY_SPEC_STR                       "priority-spec"
#define SFG_MAX_SDU_SIZE_STR                        "max-sdu-size"
#define SFG_BLK_OVERZ_ENA_STR                       "stream-blocked-due-to-oversize-frame-enabled"
#define SFG_BLK_OVERZ_STR                           "stream-blocked-due-to-oversize-frame"
#define SFG_STREAM_GATE_REF_STR                     "stream-gate-ref"
#define SFG_MAX_FILTERS_STR                         "max-stream-filter-instances"

#define SFG_STREAM_GATES_STR                        "stream-gates"
#define SFG_STREAM_GATE_INSTANCE_STR                "stream-gate-instance-table"
#define SFG_STREAM_GATE_INSTANCE_ID_STR             "stream-gate-instance-id"
#define SFG_GATE_ENABLE_STR                         "gate-enable"
#define SFG_ADMIN_GATE_STATES_STR                   "admin-gate-states"
#define SFG_ADMIN_IPV_STR                           "admin-ipv"
#define SFG_MAX_GATES_STR                           "max-stream-gate-instances"

#define SFG_FILTERS_XPATH                           "/ieee802-dot1q-bridge:bridges/bridge[name='%s']/component[name='%s']/ieee802-dot1q-stream-filters-gates:stream-filters"
#define SFG_GATES_XPATH                             "/ieee802-dot1q-bridge:bridges/bridge[name='%s']/component[name='%s']/ieee802-dot1q-stream-filters-gates:stream-gates"
#define SFG_MAX_FILTERS_XPATH                       SFG_FILTERS_XPATH "/max-stream-filter-instances"
#define SFG_MAX_GATES_XPATH                         SFG_GATES_XPATH "/max-stream-gate-instances"
#define SFG_FILTERS_INST_TABLE_XPATH                SFG_FILTERS_XPATH "/stream-filter-instance-table[stream-filter-instance-id='%s']"
#define SFG_STREAM_HANDLE_XPATH                     SFG_FILTERS_INST_TABLE_XPATH "/stream-handle"
#define SFG_PRIORITY_SPEC_XPATH                     SFG_FILTERS_INST_TABLE_XPATH "/priority-spec"
#define SFG_MAX_SDU_SIZE_XPATH                      SFG_FILTERS_INST_TABLE_XPATH "/max-sdu-size"
#define SFG_BLK_OVERZ_ENA_XPATH                     SFG_FILTERS_INST_TABLE_XPATH "/stream-blocked-due-to-oversize-frame-enabled"
#define SFG_BLK_OVERZ_XPATH                         SFG_FILTERS_INST_TABLE_XPATH "/stream-blocked-due-to-oversize-frame"
#define SFG_STREAM_GATE_REF_XPATH                   SFG_FILTERS_INST_TABLE_XPATH "/stream-gate-ref"

#define SFG_GATES_INST_TABLE_XPATH                  SFG_GATES_XPATH "/stream-gate-instance-table[stream-gate-instance-id='%s']"
#define SFG_GATE_ENABLE_XPATH                       SFG_GATES_INST_TABLE_XPATH "/gate-enable"
#define SFG_ADMIN_GATE_STATES_XPATH                 SFG_GATES_INST_TABLE_XPATH "/admin-gate-states"
#define SFG_ADMIN_IPV_XPATH                         SFG_GATES_INST_TABLE_XPATH "/admin-ipv"

#define PSFP_MATCH_FRAMES_CNT_XPATH                 SFG_FILTERS_INST_TABLE_XPATH "/ieee802-dot1q-psfp:matching-frames-count"
#define PSFP_PASS_FRAMES_CNT_XPATH                  SFG_FILTERS_INST_TABLE_XPATH "/ieee802-dot1q-psfp:passing-frames-count"
#define PSFP_NOT_PASS_FRAMES_CNT_XPATH              SFG_FILTERS_INST_TABLE_XPATH "/ieee802-dot1q-psfp:not-passing-frames-count"
#define PSFP_RED_FRAMES_CNT_XPATH                   SFG_FILTERS_INST_TABLE_XPATH "/ieee802-dot1q-psfp:red-frames-count"
#define PSFP_PASS_SDU_FRAMES_CNT_XPATH              SFG_FILTERS_INST_TABLE_XPATH "/ieee802-dot1q-psfp:passing-sdu-count"
#define PSFP_NOT_PASS_SDU_FRAMES_CNT_XPATH          SFG_FILTERS_INST_TABLE_XPATH "/ieee802-dot1q-psfp:not-passing-sdu-count"

#define ERR_SFG_ENTRY_EXISTS_STR                    "Entry already exists for '%s'."
#define ERR_SFG_ENTRY_NOT_CREATED_STR               "Not able to create entry for '%s'."
#define ERR_SFG_ENTRY_NOT_DELETED_STR               "Not able to delete entry for '%s'."

#define PSFP_MODULE_NAME_STR                        "ieee802-dot1q-psfp"
#define PSFP_PREFIX_STR                             "psfp"

#define PSFP_SUP_LIST_MAX_STR                       "supported-list-max"
#define PSFP_SUP_CYCLE_MAX_STR                      "supported-cycle-max"
#define PSFP_NUMERATOR_STR                          "numerator"
#define PSFP_DENOMINATOR_STR                        "denominator"
#define PSFP_SUP_INTRVL_MAX_STR                     "supported-interval-max"
#define PSFP_OPER_GATE_STATES_STR                   "oper-gate-states"
#define PSFP_OPER_IPV_STR                           "oper-ipv"

#define PSFP_ADMIN_CTRL_LIST_STR                    "admin-control-list"
#define PSFP_OPER_CTRL_LIST_STR                     "oper-control-list"
#define PSFP_GCL_ENTRY_STR                          "gate-control-entry"
#define PSFP_INDEX_STR                              "index"
#define PSFP_OP_NAME_STR                            "operation-name"
#define PSFP_SET_GATE_AND_IPV_STR                   "set-gate-and-ipv"
#define PSFP_TIME_INTERVAL_VALUE_STR                "time-interval-value"
#define PSFP_GATE_STATE_VALUE_STR                   "gate-state-value"
#define PSFP_IPV_SPEC_STR                           "ipv-spec"
#define PSFP_INT_MAX_OCET_STR                       "interval-octet-max"

#define PSFP_ADMIN_CYCLE_TIME_STR                   "admin-cycle-time"
#define PSFP_OPER_CYCLE_TIME_STR                    "oper-cycle-time"
#define PSFP_ADMIN_CYCLE_TIME_EXT_STR               "admin-cycle-time-extension"
#define PSFP_OPER_CYCLE_TIME_EXT_STR                "oper-cycle-time-extension"

#define PSFP_ADMIN_BASE_TIME_STR                    "admin-base-time"
#define PSFP_OPER_BASE_TIME_STR                     "oper-base-time"
#define PSFP_ABT_SECONDS_STR                        "seconds"
#define PSFP_ABT_FRACT_SECONDS_STR                  "nanoseconds"

#define PSFP_CONFIG_CHANGE_STR                      "config-change"
#define PSFP_CONFIG_CHANGE_STR_TIME_STR             "config-change-time"
#define PSFP_TICK_GRANULARITY_STR                   "tick-granularity"
#define PSFP_CURRENT_TIME_STR                       "current-time"
#define PSFP_CONFIG_PENDING_STR                     "config-pending"
#define PSFP_CONFIG_CHANGE_ERROR_STR                "config-change-error"

#define PSFP_FLOW_METERS_STR                        "flow-meters"
#define PSFP_MAX_FLOW_METERS_STR                    "max-flow-meter-instances"

#define PSFP_GT_CLOSED_INVALID_RX_ENA_STR           "gate-closed-due-to-invalid-rx-enable"
#define PSFP_GT_CLOSED_INVALID_RX_STR               "gate-closed-due-to-invalid-rx"
#define PSFP_GT_CLOSED_OCTETS_EXCD_ENA_STR          "gate-closed-due-octets-exceeded-enable"
#define PSFP_GT_CLOSED_OCTETS_EXCD_STR              "gate-closed-due-octets-exceeded"

#define PSFP_OPER_GATE_STATES_XPATH                 SFG_GATES_INST_TABLE_XPATH "/ieee802-dot1q-psfp:oper-gate-states"
#define PSFP_OPER_IPV_XPATH                         SFG_GATES_INST_TABLE_XPATH "/ieee802-dot1q-psfp:oper-ipv"
#define PSFP_OCL_OP_NAME_XPATH                      SFG_GATES_INST_TABLE_XPATH "/ieee802-dot1q-psfp:oper-control-list/gate-control-entry[index='%s']/operation-name"
#define PSFP_OCL_TIME_INT_VAL_XPATH                 SFG_GATES_INST_TABLE_XPATH "/ieee802-dot1q-psfp:oper-control-list/gate-control-entry[index='%s']/time-interval-value"
#define PSFP_OCL_GATE_STATE_XPATH                   SFG_GATES_INST_TABLE_XPATH "/ieee802-dot1q-psfp:oper-control-list/gate-control-entry[index='%s']/gate-state-value"
#define PSFP_OCL_IPV_SPEC_XPATH                     SFG_GATES_INST_TABLE_XPATH "/ieee802-dot1q-psfp:oper-control-list/gate-control-entry[index='%s']/ipv-spec"
#define PSFP_OCT_NUMERATOR_XPATH                    SFG_GATES_INST_TABLE_XPATH "/ieee802-dot1q-psfp:oper-cycle-time/numerator"
#define PSFP_OCT_DENOMINATOR_XPATH                  SFG_GATES_INST_TABLE_XPATH "/ieee802-dot1q-psfp:oper-cycle-time/denominator"
#define PSFP_OCT_EXTENSION_XPATH                    SFG_GATES_INST_TABLE_XPATH "/ieee802-dot1q-psfp:oper-cycle-time-extension"
#define PSFP_SECONDS_XPATH                          SFG_GATES_INST_TABLE_XPATH "/ieee802-dot1q-psfp:%s/seconds"
#define PSFP_NANOSECONDS_XPATH                      SFG_GATES_INST_TABLE_XPATH "/ieee802-dot1q-psfp:%s/nanoseconds"
#define PSFP_TICK_GRANULARITY_XPATH                 SFG_GATES_INST_TABLE_XPATH "/ieee802-dot1q-psfp:tick-granularity"
#define PSFP_CONFIG_PENDING_XPATH                   SFG_GATES_INST_TABLE_XPATH "/ieee802-dot1q-psfp:config-pending"
#define PSFP_CONFIG_CHANGE_ERROR_XPATH              SFG_GATES_INST_TABLE_XPATH "/ieee802-dot1q-psfp:config-change-error"

#define PSFP_ACL_CONTAINER_XPATH                    SFG_GATES_INST_TABLE_XPATH "/ieee802-dot1q-psfp:admin-control-list/*"
#define PSFP_GCL_LIST_XPATH                         SFG_GATES_INST_TABLE_XPATH "/ieee802-dot1q-psfp:admin-control-list/gate-control-entry/*"
#define PSFP_GCL_OP_NAME_XPATH                      SFG_GATES_INST_TABLE_XPATH "/ieee802-dot1q-psfp:admin-control-list/gate-control-entry[index='%s']/operation-name"
#define PSFP_GCL_TIME_INTERVAL_XPATH                SFG_GATES_INST_TABLE_XPATH "/ieee802-dot1q-psfp:admin-control-list/gate-control-entry[index='%s']/time-interval-value"
#define PSFP_GCL_GATE_STATE_XPATH                   SFG_GATES_INST_TABLE_XPATH "/ieee802-dot1q-psfp:admin-control-list/gate-control-entry[index='%s']/gate-state-value"
#define PSFP_GCL_IPV_SPEC_XPATH                     SFG_GATES_INST_TABLE_XPATH "/ieee802-dot1q-psfp:admin-control-list/gate-control-entry[index='%s']/ipv-spec"
#define PSFP_ABT_SECONDS_XPATH                      SFG_GATES_INST_TABLE_XPATH "/ieee802-dot1q-psfp:admin-base-time/seconds"
#define PSFP_ABT_NANOSECONDS_XPATH                  SFG_GATES_INST_TABLE_XPATH "/ieee802-dot1q-psfp:admin-base-time/nanoseconds"
#define PSFP_ACT_NUMERATOR_XPATH                    SFG_GATES_INST_TABLE_XPATH "/ieee802-dot1q-psfp:admin-cycle-time/numerator"
#define PSFP_ACT_DENOMINATOR_XPATH                  SFG_GATES_INST_TABLE_XPATH "/ieee802-dot1q-psfp:admin-cycle-time/denominator"
#define PSFP_ACT_EXTENSION_XPATH                    SFG_GATES_INST_TABLE_XPATH "/ieee802-dot1q-psfp:admin-cycle-time-extension"
#define PSFP_CONFIG_CHANGE_XPATH                    SFG_GATES_INST_TABLE_XPATH "/ieee802-dot1q-psfp:config-change"

#define SFG_GATES_INST_TABLE_SUB_XPATH              "/ieee802-dot1q-bridge:bridges/bridge/component/ieee802-dot1q-stream-filters-gates:stream-gates/stream-gate-instance-table"
#define SFG_GATE_ENABLE_SUB_XPATH                   SFG_GATES_INST_TABLE_SUB_XPATH "/gate-enable"
#define PSFP_CONFIG_CHANGE_SUB_XPATH                SFG_GATES_INST_TABLE_SUB_XPATH "/ieee802-dot1q-psfp:config-change"

#define ERR_ACL_CONTAINER_STR                       "Failed to get container admin-control-list for bridge '%s', gate-id '%s'!"
#define ERR_GCL_WRONG_INDEX_STR                     "gate-control-entry indices are wrong for bridge '%s', gate-id '%s'!"
#define ERR_GCL_WRONG_LEGTH_STR                     "admin-control-list-length does not match the number of admin-control-list entries for port '%s'!"
#define ERR_GCL_MAX_VALUE_STR                       "Invalid number of gate-control-entry for bridge '%s', gate-id '%s'! Values should be from 0 to 2047."
#define ERR_GCL_INDEX_BONDS_OUT_STR                 "index is out of bonds of gate-control-entry length for bridge '%s', gate-id '%s'!"
#define ERR_PSFP_MAP_FAILED_STR                     "Translator failed to map psfp operation-name."
#define ERR_PSFP_GCL_LEN_MSG_STR                    "gate control list length can not be 0 when operation is ADD for bridge '%s', gate-id '%s'."
#define ERR_PSFP_ACL_MISSING_STR                    "It is not possible to set ConfigChange to True while configuration is not complete. AdminControlList is missing for bridge '%s', gate-id '%s'."

#define btoa(x) ((x) ? BASE_TRUE : BASE_FALSE)

/* psfp */
typedef enum {
    PSFP_OP_NAME = 0,
    PSFP_TIME_INTERVAL,
    PSFP_GATE_STATE,
    PSFP_IPV_SPEC,
    PSFP_GATE_ENABLED,
    PSFP_AGS,
    PSFP_ADMIN_IPV,
    PSFP_STREAM_HDL,
    PSFP_PRIO_SPEC,
    PSFP_MAX_SDU_SIZE,
    PSFP_STR_BLCKD_FRAME_ENBLD,
    PSFP_STR_BLCKD_FRAME,
    PSFP_STR_GATE_REF,
    PSFP_INVALID_RX_ENBLD,
    PSFP_INVALID_RX,
    PSFP_MAX_OBJ_NUMBER,
} psfp_obj_index_t;

/**
 * @brief Creates a new entry for the "stream-filter-instance-table" list from
 * ieee802-dot1q-stream-filters-gates yang module.
 *
 * It is used for config data.
 *
 * @param[in]   parent          Pointer to struct lyd_node which is going to be
 *                              parent to created entry.
 * @param[in]   module          Main schema node structure representing YANG
 *                              module.
 * @param[in]   port            Poiner to struct port_hdl struct, that
 *                              represents opened port.
 * @param[in]   flt_id          uint32_t.
 * @return      struct lyd_node *or NULL if element not created.
 */
struct lyd_node *new_stream_filter_entry(struct lyd_node *parent,
                                         const struct lys_module *module,
                                         struct port_hdl *port,
                                         uint32_t flt_id);

/**
 * @brief Creates a new "stream-filters" container from
 * ieee802-dot1q-stream-filters-gates yang module.
 *
 * It is used for config data.
 *
 * @param[in]   ctx             libyang context handler.
 * @param[in]   parent          Pointer to struct lyd_node which is going to be
 *                              parent to created container.
 * @param[in]   bridge_name     Contains the name of the interface.
 * @return      struct lyd_node *or NULL if element not created.
 */
struct lyd_node *new_stream_filters_container(struct ly_ctx *ctx,
                                              struct lyd_node *parent,
                                              char *bridge_name);

/**
 * @brief Creates a new entry for the "stream-gate-instance-table" list from
 * ieee802-dot1q-stream-filters-gates yang module.
 *
 * It is used for config data.
 *
 * @param[in]   parent          Pointer to struct lyd_node which is going to be
 *                              parent to created entry.
 * @param[in]   module          Main schema node structure representing YANG
 *                              module.
 * @param[in]   port            Poiner to struct port_hdl struct, that
 *                              represents opened port.
 * @param[in]   gate_id         uint32_t.
 * @return      struct lyd_node *or NULL if element not created.
 */
struct lyd_node *new_stream_gate_entry(struct ly_ctx *ctx,
                                       struct lyd_node *parent,
                                       const struct lys_module *module,
                                       struct port_hdl *port,
                                       uint32_t gate_id);

/**
 * @brief Creates a new "stream-gates" container from
 * ieee802-dot1q-stream-filters-gates yang module.
 * It is used for config data.
 *
 * @param[in]   ctx             libyang context handler.
 * @param[in]   parent          Pointer to struct lyd_node which is going to be
 *                              parent to created container.
 * @param[in]   bridge_name     Contains the name of the interface.
 * @return      struct lyd_node *or NULL if element not created.
 */
struct lyd_node *new_stream_gates_container(struct ly_ctx *ctx,
                                            struct lyd_node *parent,
                                            char *bridge_name);

/**
 * @brief  Callback to be called by the event of changing content of nodes with
 * "/ieee802-dot1q-bridge:bridges/bridge/component/ieee802-dot1q-stream-filters-gates:stream-filters/stream-filter-instance-table/"
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
int sfg_stream_filters_cb(sr_session_ctx_t *session, const char *module_name,
                          const char *xpath, sr_event_t event,
                          uint32_t request_id, void *private_data);

/**
 * @brief Callback to be called by the event of changing content of nodes with
 * "/ieee802-dot1q-bridge:bridges/bridge/component/ieee802-dot1q-stream-filters-gates:stream-filters/stream-filter-instance-table/"
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
int sfg_stream_filters_mod_cb(sr_session_ctx_t *session, const char *module_name,
                              const char *xpath, sr_event_t event,
                              uint32_t request_id, void *private_data);

/**
 * @brief Callback to be called by the event of changing content of nodes with
 * "/ieee802-dot1q-bridge:bridges/bridge/component/ieee802-dot1q-stream-filters-gates:stream-gates/stream-gate-instance-table/ieee802-dot1q-psfp:*"
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
int psfp_stream_gates_cb(sr_session_ctx_t *session, const char *module_name,
                         const char *xpath, sr_event_t event,
                         uint32_t request_id, void *private_data);

/**
 * @brief Callback to be called by the event of changing content of node with
 * "/ieee802-dot1q-bridge:bridges/bridge/component/ieee802-dot1q-stream-filters-gates:stream-gates/stream-gate-instance-table/ieee802-dot1q-psfp:config-change"
 * xpath.
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
int psfp_config_change_cb(sr_session_ctx_t *session, const char *module_name,
                          const char *xpath, sr_event_t event,
                          uint32_t request_id, void *private_data);

/**
 * @brief  Callback to be called by the event of changing content of nodes with
 * "/ieee802-dot1q-bridge:bridges/bridge/component/ieee802-dot1q-stream-filters-gates:stream-gates/stream-gate-instance-table/"
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
int sfg_stream_gates_mod_cb(sr_session_ctx_t *session, const char *module_name,
                            const char *xpath, sr_event_t event,
                            uint32_t request_id, void *private_data);

/**
 * @brief This function will check the status of psfp stream gate entry values.
 *
 * All this informations are necessary for triggering config-change to 'true'.
 * If configuration is not complete, it is not possible to trigger
 * config-change to 'true'. Complete configuration requires admin-control-list
 * with all elements, admin-base-time and admin-cycle-time elements value
 * different from '0' and admin-cycle-time-extension element.
 *
 * @param[in]   session         Automatically-created session that can be used
 *                              for obtaining changed data (e.g. by
 *                              sr_get_changes_iter call or sr_get_item -like
 *                              calls). Do not stop this session.
 * @param[in]   node            Pointer on sysrepo node structure element.
 * @param[in]   brdev           Pointer on bridge handler structure.
 * @param[in]   bridge_name     String with interface name.
 * @param[in]   gate_id         String with gate id value.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
int psfp_gcl_validation(sr_session_ctx_t *session, sr_val_t *node,
                        struct port_hdl *brdev, char *bridge_name,
                        char *gate_id);

/**
 * @brief Callback to be called by the event of changing content of nodes with
 * "/ieee802-dot1q-bridge:bridges/bridge/component/ieee802-dot1q-stream-filters-gates:stream-gates/stream-gate-instance-table/ieee802-dot1q-psfp:admin-control-list/gate-control-entry/"
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
int psfp_acl_cb(sr_session_ctx_t *session, const char *module_name,
                const char *xpath, sr_event_t event,
                uint32_t request_id, void *private_data);

/**
 * @brief Callback to be called by the event of changing content of nodes with
 * "/ieee802-dot1q-bridge:bridges/bridge/component/ieee802-dot1q-stream-filters-gates:stream-gates/stream-gate-instance-table/ieee802-dot1q-psfp:admin-control-list/gate-control-entry/"
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
int psfp_acl_mod_cb(sr_session_ctx_t *session, const char *module_name,
                    const char *xpath, sr_event_t event,
                    uint32_t request_id, void *private_data);

/**
 * @brief Creates a new state data leaves defined at sfsg and psfp modules
 * inside list component inside ieee802-dot1q-bridge module.
 *
 * @param[in]   parent          Pointer to an existing parent of the requested
 *                              nodes. Is NULL for top-level nodes. Caller is
 *                              supposed to append the requested nodes to this
 *                              data subtree and return either the original
 *                              parent or a top-level node.
 * @param[in]   bridge_name     Bridge name.
 * @return      Error code (SR_ERR_OK on success).
 */
int get_sfsg_state_data(struct lyd_node **parent, char *bridge_name);

/**
 * @brief Creates a new "flow-meters" container from ieee802-dot1q-psfp yang
 * module.
 *
 * It is used for config data.
 *
 * @param[in]   ctx             libyang context handler.
 * @param[in]   parent          Pointer to struct lyd_node which is going to be
 *                              parent to created container.
 * @param[in]   bridge_name     Contains the name of the interface.
 * @return      struct lyd_node *or NULL if element not created.
 */
struct lyd_node *new_flow_meters_container(struct ly_ctx *ctx,
                                           struct lyd_node *parent,
                                           char *bridge_name);

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
int psfp_fix_datastore(sr_session_ctx_t *session, sr_datastore_t datastore);

#endif /* STREAM_FILTER_GATES_H_ */
