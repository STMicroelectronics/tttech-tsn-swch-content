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

#ifndef FRER_STATISTICS_H_
#define FRER_STATISTICS_H_

/* common includes */
#include "common_defines.h"
#include "common.h"
/* libtsn includes */
#include "libtsn/tsn.h"
#include "libtsn/tsn_sid.h"
#include "libtsn/tsn_frer.h"
#include "libtsn/tsn_def.h"
/* libbase include */
#include "libbase/base_q_bridge_linux_tools.h"
#include "libbase/base_interfaces_linux_tools.h"
#include "../../ieee802-dot1cb-stream-identification-yang-module/stream_identification_defines.h"

#define FRER_FRAME_REP_AND_EL_STREAM_COUNT_STR          "per-port-per-stream-counters"
#define FRER_HANDLE_STR                                 "handle"
#define FRER_DIRECTION_STR                              "direction-out-facing"
#define FRER_PER_PORT_PER_STREAM_STR                    "per-port-per-stream"
#define FRER_GENERATION_RESET_STR                       "generation-reset"
#define FRER_REC_OUT_ORDER_PCKTS_STR                    "rx-out-of-order-pkts"
#define FRER_REC_ROUGE_PCKTS_STR                        "rx-rogue-pkts"
#define FRER_REC_PASSED_PCKTS_STR                       "rx-passed-pkts"
#define FRER_REC_DISCARDED_PCKTS_STR                    "rx-discarded-pkts"
#define FRER_REC_LOST_PCKTS_STR                         "rx-lost-pkts"
#define FRER_REC_TAGLESS_PCKTS_STR                      "rx-tagless-pkts"
#define FRER_REC_RESETS_STR                             "rx-resets"
#define FRER_REC_LATENT_ERR_RESET_STR                   "rx-latent-error-resets"
#define FRER_ENC_ERRORED_PCKTS_STR                      "encode-errored-pkts"

#define ERR_COULD_NOT_FIND_STREAM_HANDLE_STR            "Couldn't find the stream handle in the database"
#define ERR_FAILED_TO_GET_BRIDGE_HANDLES_STR            "Failed to bridge stream-handles"
#define ERR_FAILED_TO_GET_NUMBER_OF_BRIDGE_HANDLES_STR  "Failed to get number of bridge stream-handles"

#define FRER_XPATH                                      \
        "/ietf-interfaces:interfaces/interface[name='%s']/statistics/ieee802-dot1cb-frer:frer"

/* container frame-replication-and-elimination-per-port-counters xpaths */
#define FRER_PER_PORT_XPATH                             FRER_XPATH "/per-port-counters/%s"

/* list frame-replication-and-elimination-per-port-per-stream-counters objects xpaths */
#define FRER_PER_PORT_PER_STREAM_XPATH                  FRER_XPATH "/per-port-per-stream-counters[direction-out-facing='%s'][handle='%s']/%s"

/**
 * @brief Creates a new container
 * 'frame-replication-and-elimination-per-port-per-stream-counters' and
 * 'frame-replication-and-elimination-per-port-counters' for list 'interface'
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
 * @return      Error code (SR_ERR_OK on success).
 */
int frame_replication_and_elimination_per_port_per_stream_counters(
        struct lyd_node **parent, char *if_name);

#endif /* FRER_STATISTICS_H_ */
