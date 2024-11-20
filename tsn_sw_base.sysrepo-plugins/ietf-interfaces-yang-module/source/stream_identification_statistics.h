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

#ifndef STREAM_IDENTIFICATION_STATISTICS_H_
#define STREAM_IDENTIFICATION_STATISTICS_H_

/* common includes */
#include "common_defines.h"
#include "common.h"
/* libtsn includes */
#include "libtsn/tsn.h"
#include "libtsn/tsn_sid.h"
#include "libtsn/tsn_def.h"
/* libbase include */
#include "libbase/base_q_bridge_linux_tools.h"
#include "libbase/base_interfaces_linux_tools.h"
#include "libbase/base_private_parsers.h"
/* stream identification plugin defines include */
#include "../../ieee802-dot1cb-stream-identification-yang-module/stream_identification_defines.h"

#define STREAM_ID_STREAM_IDENT_COUNTERS_STR                     "per-port-per-stream-counters"
#define ERR_STREAM_ID_COULD_NOT_FIND_STREAM_HANDLE_STR          "Couldn't find the stream handle in the database for interface %s"
#define ERR_FAILED_TO_GET_BRIDGE_HANDLES_STR                    "Failed to bridge stream-handles"
#define ERR_FAILED_TO_GET_NUMBER_OF_BRIDGE_HANDLES_STR          "Failed to get number of bridge stream-handles"

#define STREAM_ID_INPUT_PACKETS_STR                             "input-pkts"
#define STREAM_ID_OUTPUT_PACKETS_STR                            "output-pkts"

/* container stream-identification-per-port-counters */
#define STREAM_ID_XPATH                                         \
        "/ietf-interfaces:interfaces/interface[name='%s']/statistics/ieee802-dot1cb-stream-identification:stream-id"
#define STREAM_ID_PER_PORT_INPUT_PACKETS_XPATH                  STREAM_ID_XPATH "/per-port-counters/input-pkts"
#define STREAM_ID_PER_PORT_OUTPUT_PACKETS_XPATH                 STREAM_ID_XPATH "/per-port-counters/output-pkts"

/* list stream-identification-per-port-per-stream-counters */
#define STREAM_ID_PER_PORT_PER_STREAM_INPUT_PACKETS_XPATH       STREAM_ID_XPATH "/per-port-per-stream-counters[direction-out-facing='%s'][handle='%s']/input-pkts"
#define STREAM_ID_PER_PORT_PER_STREAM_OUTPUT_PACKETS_XPATH      STREAM_ID_XPATH "/per-port-per-stream-counters[direction-out-facing='%s'][handle='%s']/output-pkts"

/**
 * @brief Creates a new list stream-identification-per-port-per-stream-counters
 * for list 'interface' inside ietf-interfaces yang module.
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
int sid_per_port_per_stream_counters(struct lyd_node **parent, char *if_name);

#endif /* STREAM_IDENTIFICATION_STATISTICS_H_ */
