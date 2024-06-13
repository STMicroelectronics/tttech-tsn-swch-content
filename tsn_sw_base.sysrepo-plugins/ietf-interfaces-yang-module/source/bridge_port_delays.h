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

#ifndef BRIDGE_PORT_DELAYS_H_
#define BRIDGE_PORT_DELAYS_H_

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

#define BRIDGE_PORT_DELAYS_MODULE_NAME          "ieee802-dot1q-bridge-delays"

#define BRIDGE_PORT_DELAYS_XPATH      \
        "/ietf-interfaces:interfaces/interface[name='%s']/ieee802-dot1q-bridge:bridge-port/ieee802-dot1q-bridge-delays:bridge-port-delays[port-speed='%u']"
#define BRIDGE_PORT_DEP_RX_DELAY_MIN_XPATH      BRIDGE_PORT_DELAYS_XPATH "/dependentRxDelayMin"
#define BRIDGE_PORT_DEP_RX_DELAY_MAX_XPATH      BRIDGE_PORT_DELAYS_XPATH "/dependentRxDelayMax"
#define BRIDGE_PORT_INDEP_RX_DELAY_MIN_XPATH    BRIDGE_PORT_DELAYS_XPATH "/independentRxDelayMin"
#define BRIDGE_PORT_INDEP_RX_DELAY_MAX_XPATH    BRIDGE_PORT_DELAYS_XPATH "/independentRxDelayMax"
#define BRIDGE_PORT_INDEP_RLY_DELAY_MIN_XPATH   BRIDGE_PORT_DELAYS_XPATH "/independentRlyDelayMin"
#define BRIDGE_PORT_INDEP_RLY_DELAY_MAX_XPATH   BRIDGE_PORT_DELAYS_XPATH "/independentRlyDelayMax"
#define BRIDGE_PORT_INDEP_TX_DELAY_MIN_XPATH    BRIDGE_PORT_DELAYS_XPATH "/independentTxDelayMin"
#define BRIDGE_PORT_INDEP_TX_DELAY_MAX_XPATH    BRIDGE_PORT_DELAYS_XPATH "/independentTxDelayMax"

/**
 * @brief Creates a new container statistic inside container bridge-port for
 * list 'interface' inside ietf-interfaces yang module.
 *
 * This function is for state data from list 'interface'.
 *
 * @param[in]   parent          Pointer to an existing parent of the requested
 *                              nodes. Is NULL for top-level nodes. Caller is
 *                              supposed to append the requested nodes to this
 *                              data subtree and return either the original
 *                              parent or a top-level node.
 * @param[in]   interface_name  The name of the interface.
 * @param[in]   interface_name  The value of the supported speed.
 * @return      Error code (SR_ERR_OK on success).
 */
int new_bridge_port_delays(struct lyd_node **parent, char *interface_name,
                           uint32_t supported_port_speed);

#endif /* BRIDGE_PORT_DELAYS_H_ */
