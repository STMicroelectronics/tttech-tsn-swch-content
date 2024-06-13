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

#ifndef BRIDGE_PORT_H_
#define BRIDGE_PORT_H_

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
#include "libbase/base_private_translators.h"
#include "libbase/base_interfaces_linux_tools.h"
#include "libbase/base_q_bridge_linux_tools.h"
/* libtsn includes */
#include "libtsn/tsn.h"
#include "libtsn/tsn_bridge.h"
#include "libtsn/tsn_def.h"
#include "libtsn/tsn_error.h"

/* define bridge-port objects indexes */
typedef enum {
    BRIDGE_PORT_PVID = 0,
    BRIDGE_PORT_DEF_PRIORITY,
    BRIDGE_PORT_PRIO_REG,
    BRIDGE_PORT_TRAFFIC_CLASS,
    BRIDGE_PORT_MAX_OBJ_NUMBER,
} bridge_port_obj_index_t;

/* container bridge-port xpath */
#define BRIDGE_PORT_XPATH                       \
        "/ietf-interfaces:interfaces/interface[name='%s']/ieee802-dot1q-bridge:bridge-port"
#define BRIDGE_PORT_STATISTICS_XPATH            BRIDGE_PORT_XPATH "/ieee802-dot1q-bridge:statistics/ieee802-dot1q-bridge:%s"

#define BRIDGE_PORT_NUM_TC_XPATH                BRIDGE_PORT_XPATH "/num-traffic-class"
#define BRIDGE_PORT_PORT_NUM_XPATH              BRIDGE_PORT_XPATH "/port-number"
#define BRIDGE_PORT_ADDRESS_XPATH               BRIDGE_PORT_XPATH "/address"
#define BRIDGE_PORT_CAPABILITIES_XPATH          BRIDGE_PORT_XPATH "/capabilities"
#define BRIDGE_PORT_TYPE_CAPABILITIES_XPATH     BRIDGE_PORT_XPATH "/type-capabilties"
#define BRIDGE_PORT_EXTERNAL_XPATH              BRIDGE_PORT_XPATH "/external"

#define ERR_PORT_TYPE_SPPORTED_XPATH_STR        "Only c-vlan-bridge-port port-type is supported for '%s'."

/**
 * @brief Creates a new entry for container "bridge-port" from
 * ieee802-dot1q-bridge yang module. It is used for config data.
 *
 * @param[in]   parent      Pointer to struct lyd_node which is going to be parent to created container.
 * @param[in]   ctx         libyang context handler.
 * @param[in]   if_name     Contains the name of the interface.
 * @return      struct lyd_node* or NULL if element not created.
 */
struct lyd_node *new_bridge_port_node(struct lyd_node *parent,
                                      struct ly_ctx *ctx, char *if_name);

/**
 * @brief Creates a new container statistic inside container bridge-port for
 * list 'interface' inside ietf-interfaces yang module. This function is for
 * state data from list 'interface'.
 *
 * @param[in]   parent          Pointer to an existing parent of the requested
 *                              nodes. Is NULL for top-level nodes. Caller is
 *                              supposed to append the requested nodes to this
 *                              data subtree and return either the original
 *                              parent or a top-level node.
 * @param[in]   interface_name  Name of the interface.
 * @return      Error code (SR_ERR_OK on success).
 */
int new_bridge_port_statistic_state_node(struct lyd_node **parent,
                                         char *interface_name);

/**
 * @brief Creates a new augmented container 'bridge-port' for list 'interface'
 * from ietf-interfaces yang module. This function is for state data from list
 * 'interface'.
 *
 * @param[in]   parent          Pointer to an existing parent of the requested
 *                              nodes. Is NULL for top-level nodes. Caller is
 *                              supposed to append the requested nodes to this
 *                              data subtree and return either the original
 *                              parent or a top-level node. @param[in]
 *                              interface_name   Name of the interface.
 * @return      Error code (SR_ERR_OK on success).
 */
int new_bridge_port_state_node(struct lyd_node **parent, char *interface_name);

/**
 * @brief Callback to be called by the event of changing content of all nodes
 * at xpath
 * "/ietf-interfaces:interfaces/interface/ieee802-dot1q-bridge:bridge-port"
 * xpath. Subscribe to it by sr_module_change_subscribe call.
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
int bridge_port_change_cb(sr_session_ctx_t *session, const char *module_name,
                          const char *xpath, sr_event_t event,
                          uint32_t request_id, void *private_data);

/**
 * @brief Callback to be called by the event of changing content of leaf
 * port-type inside bridge-port container inside ieee802-dot1q-bridge yang
 * module. Subscribe to it by sr_module_change_subscribe call.
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
int bridge_port_port_type_cb(sr_session_ctx_t *session, const char *module_name,
                             const char *xpath, sr_event_t event,
                             uint32_t request_id, void *private_data);

#endif /* BRIDGE_PORT_H_ */
