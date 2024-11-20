/*
 * TTTech ietf-interfaces-yang-module
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

#ifndef PREEMPTION_DEFINES_H_
#define PREEMPTION_DEFINES_H_

/* common includes */
#include "common_defines.h"
#include "common.h"
/* libbase includes */
#include "libbase/base_defines.h"
#include "libbase/base_prints.h"
#include "libbase/base_linux_tool_helpers.h"
#include "libbase/base_interfaces_linux_tools.h"
#include "libbase/base_private_translators.h"
/* libtsn includes */
#include "libtsn/tsn.h"
#include "libtsn/tsn_preemp.h"
#include "libtsn/tsn_error.h"

#define PREEMPT_MODULE_NAME_STR             "ieee802-dot1q-preemption"
#define PREEMPT_PREFIX_STR                  "preempt"
#define PREEMPT_NS_STR                      "urn:ieee:std:802.1Q:yang:ieee802-dot1q-preemption"

/* containers, tables, lists, leafs names */
#define PREEMPT_FRAME_PARAMS_STR            "frame-preemption-parameters"
#define PREEMPT_FRAME_STATUS_TABLE_STR      "frame-preemption-status-table"
#define PREEMPT_TRAFFIC_CLASS_STR           "traffic-class"
#define PREEMPT_FRAME_STATUS_STR            "frame-preemption-status"

#define PREEMPT_HOLD_ADVANCE_STR            "hold-advance"
#define PREEMPT_RELEASE_ADVANCE_STR         "release-advance"
#define PREEMPT_ACTIVE_STR                  "preemption-active"
#define PREEMPT_HOLD_REQUEST_STR            "hold-request"

/* preemption specific error messages */
#define ERROR_PREEMPT_UNKNOWN_VALUE_STR     "frame-preemption-status is UNKNOWN."
#define ERROR_PREEMPT_NOT_SUPPORTED_STR     "PREEMPT is not supported on interface "

/* preempt specific value defines */
#define PREEMPT_STATUS_EXPRESS_STR          "express"
#define PREEMPT_STATUS_PREEMPTABLE_STR      "preemptable"
#define PREEMPT_HOLD_STR                    "hold"
#define PREEMPT_RELEASE_STR                 "release"

/* ieee802-dot1q-preemption xpath defines */
#define INTERFACE_PREEMPTION_XPATH                  \
        "/ietf-interfaces:interfaces/interface[name='%s']/ieee802-dot1q-preemption:frame-preemption-parameters"
#define INTERFACE_PREEMPTION_HOLD_ADVANCE_XPATH     INTERFACE_PREEMPTION_XPATH "/hold-advance"
#define INTERFACE_PREEMPTION_RELEASE_ADVANCE_XPATH  INTERFACE_PREEMPTION_XPATH "/release-advance"
#define INTERFACE_PREEMPTION_ACTIVE_XPATH           INTERFACE_PREEMPTION_XPATH "/preemption-active"
#define INTERFACE_PREEMPTION_HOLD_REQUEST_XPATH     INTERFACE_PREEMPTION_XPATH "/hold-request"

/**
 * @brief Creates a new entry for the "frame-preemption-parameters" container
 * from ieee802-dot1q-preemption yang module.
 *
 * It is used for config data.
 *
 * @param[out]  parent          Pointer to struct lyd_node which is going to be
 *                              parent to created container.
 * @param[in]   ctx             libyang context handler.
 * @param[in]   if_name         Contains the name of the interface.
 * @return      struct lyd_node* or NULL if element not created.
 */
struct lyd_node *new_frame_preemption_parameters_container(
        struct lyd_node *parent, struct ly_ctx *ctx, char *if_name);

/**
 * @brief Creates a new entry for the "frame-preemption-status-table" container
 * from ieee802-dot1q-preemption yang module.
 *
 * It is used for config data.
 *
 * @param[out]  parent          Pointer to struct lyd_node which is going to be
 *                              parent to created entry.
 * @param[in]   module          Main schema node structure representing YANG
 *                              module.
 * @param[in]   if_name         Contains the name of the interface.
 * @param[in]   prio            Contains the priority value of the traffic
 *                              class.
 * @param[in]   traffic_class   Contains the value of the traffic class.
 * @return      struct lyd_node* or NULL if element not created.
 */
struct lyd_node *new_frame_preemption_status_table(
        struct lyd_node *parent, const struct lys_module *module,
        char *if_name, int prio, int traffic_class);

/**
 * @brief Creates a new container frame-preemtion-parameters for list
 * 'interface' inside ietf-interfaces yang module.
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
int new_state_preemption_container(struct lyd_node **parent,
                                   char *interface_name);

/* CALLBACKS FOR PREEMPTION CONFIG AND STATE DATA */

/**
 * @brief Callback to be called by the event of changing content of node with
 * "/ietf-interfaces:interfaces/interface/ieee802-dot1q-preemption:frame-preemption-parameters/ieee802-dot1q-preemption:frame-preemption-status-table/ieee802-dot1q-preemption:frame-preemption-status"
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
int interface_frame_preemption_status_cb(sr_session_ctx_t *session,
                                         const char *module_name,
                                         const char *xpath,
                                         sr_event_t event,
                                         uint32_t request_id,
                                         void *private_data);

#endif
