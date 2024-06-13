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

#ifndef ETHERNET_DEFINES_H_
#define ETHERNET_DEFINES_H_

#include <pthread.h>
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
#include "libbase/base_private_translators.h"
#include "libbase/base_interfaces_linux_tools.h"
/* libtsn includes */
#include "libtsn/tsn.h"
#include "libtsn/tsn_preemp.h"
#include "libtsn/tsn_def.h"

#define ETH_MODULE_NAME_STR                     "ieee802-ethernet-interface"
#define ETH_NS_STR                              "urn:ieee:std:802.3:yang:ieee802-ethernet-interface"

/* containers, tables, lists, leafs names */
#define ETH_ETHERNET_CONTAINER_STR              "ethernet"
#define ETH_AUTO_NEGOTIATION_STR                "auto-negotiation"
#define ETH_IF_ENABLE_STR                       "enable"
#define ETH_MAX_FRAME_LEN_STR                   "max-frame-length"
#define ETH_SPEED_STR                           "speed"
#define ETH_DUPLEX_STR                          "duplex"
#define ETH_ADVERTISE_STR                       "advertise"
#define ETH_SUPPORTED_IF_MODE_STR               "sup_mode"
#define ETH_LINK_DETECTED_STR                   "link-detected"
#define ETH_ADVERTISED_IF_MODE_STR              "store-advertised-modes"
#define ETH_STARTUP_ADVERTISED_MODES_STR        "startup-advertised-modes"
#define ETH_FRAME_STR                           "frame"
#define ETH_IN_TOTAL_FRAMES_STR                 "in-total-frames"
#define ETH_IN_TOTAL_OCTETS_STR                 "in-total-octets"
#define ETH_IN_FRAMES_STR                       "in-frames"
#define ETH_IN_MULTICAST_FRAMES_STR             "in-multicast-frames"
#define ETH_OUT_FRAMES_STR                      "out-frames"

/* ieee802-ethernet-interface specific values */
#define ETH_DUPLEX_HALF_STR                     "Half"
#define ETH_DUPLEX_FULL_STR                     "Full"
#define ETH_SPEED_TEN_STR                       "0.010"
#define ETH_SPEED_ONE_HUNDRED_STR               "0.100"
#define ETH_SPEED_ONE_THOUSAND_STR              "1.000"
#define ETH_SPEED_TEN_THOUSAND_STR              "10.000"
#define ETH_ADVERTISE_STR                       "advertise"
#define ETH_BASE_T_STR                          "baseT"
#define ETH_SUPPORTS_AUTO_NEG_STR               "Supports auto-negotiation:"
#define ETH_ADVERTISE_PAUSE_FRAME_STR           "Advertised pause frame use:"

/* ieee802-ethernet-interface xpath defines */
#define INTERFACE_ETHERNET_XPATH                    \
        "/ietf-interfaces:interfaces/interface[name='%s']/ieee802-ethernet-interface:ethernet"
#define INTERFACE_MAX_FRAME_LENGHT_XPATH        INTERFACE_ETHERNET_XPATH "/max-frame-length"

#define INTERFACE_IN_TOTAL_FRAMES_XPATH         INTERFACE_ETHERNET_XPATH "/statistics/frame/in-total-frames"
#define INTERFACE_IN_TOTAL_OCTETS_XPATH         INTERFACE_ETHERNET_XPATH "/statistics/frame/in-total-octets"
#define INTERFACE_IN_FRAMES_XPATH               INTERFACE_ETHERNET_XPATH "/statistics/frame/in-frames"
#define INTERFACE_IN_MULTICAST_FRAMES_XPATH     INTERFACE_ETHERNET_XPATH "/statistics/frame/in-multicast-frames"
#define INTERFACE_OUT_FRAMES_XPATH              INTERFACE_ETHERNET_XPATH "/statistics/frame/out-frames"

#define INTERFACE_SPEED_XPATH                   INTERFACE_ETHERNET_XPATH "/speed"
#define INTERFACE_DUPLEX_XPATH                  INTERFACE_ETHERNET_XPATH "/duplex"

/**
 * @brief This function checks if there is different advertised link modes then
 * advertised_mode that is passed as argument (created using speed and duplex).
 *
 * @param[in]   objects          Array of advertised link modes.
 * @param[in]   advertised_mode  Advertised link mode to be checked.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
int multiple_advertised_link_modes(char objects[][MAX_STR_LEN],
                                   char *advertised_mode);

/**
 * @brief Creates container "ethernet" from ieee802-ethernet-interface yang
 * module.
 *
 * It is used for config data.
 *
 * @param[out]  parent          Pointer to struct lyd_node which is going to be
 *                              parent to created container.
 * @param[in]   ctx             libyang context handler.
 * @param[in]   if_name         Contains the name of the interface.
 * @return      struct lyd_node* or NULL if element not created.
 */
struct lyd_node *new_ethernet_container(struct lyd_node *parent,
                                        struct ly_ctx *ctx, char *if_name);

/**
 * @brief Creates container "auto-negotiation" from ieee802-ethernet-interface
 * yang module.
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
struct lyd_node *new_auto_negotiation(struct lyd_node *parent,
                                      const struct lys_module *module,
                                      char *if_name);

/**
 * @brief Creates a new container statistic inside container ehternet for list
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
int new_state_ethernet_statistics_container(struct lyd_node **parent,
                                            char *interface_name);

/**
 * @brief Creates a new container ethernet for list 'interface' inside
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
int new_state_ethernet_container(struct lyd_node **parent,
                                 char *interface_name);

/**
 * @brief Subroutine that is executed by the thread.
 *
 * @param[in]   callback_value  Pointer to void that contains the arguments of
 *                              the function speed_and_duplex_thread.
 * @return      NULL.
 */
void *speed_and_duplex_thread(void *callback_value);

/* CALLBACKS FOR PREEMPTION CONFIG DATA */

/**
 * @brief Callback to be called by the event of changing content of node with
 * next xpath:
 * "/ietf-interfaces:interfaces/interface/ieee802-ethernet-interface:ethernet/speed"
 * "/ietf-interfaces:interfaces/interface/ieee802-ethernet-interface:ethernet/duplex"
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
int interface_ethernet_duplex_speed_cb(sr_session_ctx_t *session,
                                       const char *module_name,
                                       const char *xpath, sr_event_t event,
                                       uint32_t request_id, void *private_data);

#endif /* ETHERNET_DEFINES_H_ */
