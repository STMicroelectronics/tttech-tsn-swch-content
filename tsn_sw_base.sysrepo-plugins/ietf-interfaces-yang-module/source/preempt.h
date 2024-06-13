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

#ifndef PREEMPT_DEFINES_H_
#define PREEMPT_DEFINES_H_

/* common includes */
#include "common_defines.h"
#include "common.h"
/* sysrepo includes */
#include <sysrepo.h>
#include <sysrepo/values.h>
#include <sysrepo/xpath.h>
/* libbase includes */
#include "libbase/base_defines.h"
#include "libbase/base_prints.h"
#include "libbase/base_linux_tool_helpers.h"
#include "libbase/base_interfaces_linux_tools.h"
#include "libbase/base_private_translators.h"
/* libtsn includes */
#include "libtsn/tsn.h"
#include "libtsn/tsn_preemp.h"
#include "libtsn/tsn_def.h"
#include "libtsn/tsn_error.h"

#define PREE_MACMERGE_MODULE_NAME_STR               "ieee802-dot3br-preempt"
#define PREE_MACMERGE_PREFIX_STR                    "preempt1"
#define PREE_MACMERGE_NS_STR                        "urn:ieee:std:802.1Q:yang:ieee802-dot3br-preempt"

/* containers, tables, lists, leafs names */
#define PREE_MACMERGE_MAC_MERGE_STR                 "mac-merge"

#define PREE_MACMERGE_ADMIN_CONTROL_STR             "admin-control"
#define PREE_MACMERGE_MERGE_ENABLE_TX_STR           "merge-enable-tx"
#define PREE_MACMERGE_VERIFY_DISABLE_TX_STR         "verify-disable-tx"
#define PREE_MACMERGE_VERIFY_TIME_STR               "verify-time"
#define PREE_MACMERGE_FRAG_SIZE_STR                 "frag-size"

#define PREE_MACMERGE_ADMIN_STATUS_STR              "admin-status"
#define PREE_MACMERGE_MERGE_SUPPORT_STR             "merge-support"
#define PREE_MACMERGE_VERIFY_STATUS_STR             "verify-status"
#define PREE_MACMERGE_STATUS_TX_STR                 "status-tx"

#define PREE_MACMERGE_STATISTICS_STR                "statistics"
#define PREE_MACMERGE_ASSEMBLY_ERR_CNT_STR          "assembly-error-count"
#define PREE_MACMERGE_SMD_ERR_CNT_STR               "smd-error-count"
#define PREE_MACMERGE_ASSEMBLY_OK_CNT_STR           "assembly-ok-count"
#define PREE_MACMERGE_FRAGMENT_CNT_RX_STR           "fragment-count-rx"
#define PREE_MACMERGE_FRAGMENT_CNT_TX_STR           "fragment-count-tx"
#define PREE_MACMERGE_HOLD_CNT_STR                  "hold-count"

/* preempt(mac-merge) specific error messages */
#define ERROR_PREE_MAC_MERGE_NOT_SUPPORTED_STR      "preempt mac-merge is not supported on interface "

/* values defines */
#define PREE_MACMERGE_ENABLED_STR                   "Enabled"
#define PREE_MACMERGE_DISABLED_STR                  "Disabled"
#define PREE_MACMERGE_SUPPPORTED_STR                "Supported"
#define PREE_MACMERGE_NOT_SUPPORTED_STR             "Not Supported"
#define PREE_MACMERGE_STATUS_UNKNOWN_STR            "unknown"
#define PREE_MACMERGE_STATUS_INITIAL_STR            "initial"
#define PREE_MACMERGE_STATUS_VERIFYING_STR          "verifying"
#define PREE_MACMERGE_STATUS_SUCCEEDED_STR          "succeeded"
#define PREE_MACMERGE_STATUS_FAILED_STR             "failed"
#define PREE_MACMERGE_STATUS_DISABLED_STR           "disabled"
#define PREE_MACMERGE_STATUS_INACTIVE_STR           "inactive"
#define PREE_MACMERGE_STATUS_ACTIVE_STR             "active"

/* ieee802-dot3br-preempt xpath defines */
#define INTERFACE_PREEMPT_XPATH                     \
        "/ietf-interfaces:interfaces/interface[name='%s']/ieee802-dot3br-preempt:mac-merge"
#define INTERFACE_PREEMPT_MERGE_SUPPORT_XPATH       INTERFACE_PREEMPT_XPATH "/admin-status/merge-support"
#define INTERFACE_PREEMPT_VERIFY_STATUS_XPATH       INTERFACE_PREEMPT_XPATH "/admin-status/verify-status"
#define INTERFACE_PREEMPT_STATUS_TX_XPATH           INTERFACE_PREEMPT_XPATH "/admin-status/status-tx"

#define INTERFACE_PREEMPT_ASSEMBLY_ERR_CNT_XPATH    INTERFACE_PREEMPT_XPATH "/statistics/assembly-error-count"
#define INTERFACE_PREEMPT_SMD_ERR_CNT_XPATH         INTERFACE_PREEMPT_XPATH "/statistics/smd-error-count"
#define INTERFACE_PREEMPT_ASSEMBLY_OK_CNT_XPATH     INTERFACE_PREEMPT_XPATH "/statistics/assembly-ok-count"
#define INTERFACE_PREEMPT_FRAGMENT_CNT_TX_XPATH     INTERFACE_PREEMPT_XPATH "/statistics/fragment-count-tx"
#define INTERFACE_PREEMPT_FRAGMENT_CNT_RX_XPATH     INTERFACE_PREEMPT_XPATH "/statistics/fragment-count-rx"
#define INTERFACE_PREEMPT_HOLD_CNT_XPATH            INTERFACE_PREEMPT_XPATH "/statistics/hold-count"

/* PREEMPT */
typedef enum {
    PREE_MACMERGE_MERGE_ENABLE_TX = 0,
    PREE_MACMERGE_VERIFY_DISABLE_TX,
    PREE_MACMERGE_VERIFY_TIME,
    PREE_MACMERGE_FRAG_SIZE,
    PREE_MACMERGE_MAX_OBJ_NUMBER,
} mac_merge_obj_index_t;

/**
 * @brief Creates container "mac-merge" from ieee802-dot3br-preempt yang module.
 *
 * It is used for config data.
 *
 * @param[out]  parent          Pointer to struct lyd_node which is going to be
 *                              parent to created container.
 * @param[in]   ctx             libyang context handler.
 * @param[in]   if_name         Contains the name of the interface.
 * @return      struct lyd_node* or NULL if element not created.
 */
struct lyd_node *new_mac_merge_container(struct lyd_node *parent,
                                         struct ly_ctx *ctx, char *if_name);

/**
 * @brief Creates container "admin-control" from ieee802-dot3br-preempt yang
 * module.
 *
 * It is used for config data.
 *
 * @param[out]  parent          Pointer to struct lyd_node which is going to be
 *                              parent to created container.
 * @param[in]   module          Main schema node structure representing YANG
 *                              module.
 * @param[in]   if_name         Contains the name of the interface.
 * @return      struct lyd_node* or NULL if element not created.
 */
struct lyd_node *new_admin_control(struct lyd_node *parent,
                                   const struct lys_module *module,
                                   char *if_name);

/**
 * @brief Creates a new admin-status container inside container mac-merge
 * inside ieee802-dot3br-preempt yang module.
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
int new_admin_status(struct lyd_node **parent, char *interface_name);

/**
 * @brief Creates a new statistics container inside container mac-merge inside
 * ieee802-dot3br-preempt yang module.
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
int new_statistics(struct lyd_node **parent, char *interface_name);

/**
 * @brief Creates a new mac-merge for list 'interface' inside ietf-interfaces
 * yang module.
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
int new_state_mac_merge_container(struct lyd_node **parent,
                                  char *interface_name);

/* CALLBACKS FOR PREEMPT CONFIG AND STATE DATA */

/**
 * @brief Callback to be called by the event of changing content of node with
 * next xpath:
 * "/ietf-interfaces:interfaces/interface/ieee802-dot3br-preempt:mac-merge/ieee802-dot3br-preempt:admin-control/merge-enable-tx",
 * "/ietf-interfaces:interfaces/interface/ieee802-dot3br-preempt:mac-merge/ieee802-dot3br-preempt:admin-control/verify-disable-tx",
 * "/ietf-interfaces:interfaces/interface/ieee802-dot3br-preempt:mac-merge/ieee802-dot3br-preempt:admin-control/verify-time",
 * "/ietf-interfaces:interfaces/interface/ieee802-dot3br-preempt:mac-merge/ieee802-dot3br-preempt:admin-control/frag-size".
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
int interface_admin_control_cb(sr_session_ctx_t *session,
                               const char *module_name, const char *xpath,
                               sr_event_t event, uint32_t request_id,
                               void *private_data);

#endif /* PREEMPT_DEFINES_H_ */
