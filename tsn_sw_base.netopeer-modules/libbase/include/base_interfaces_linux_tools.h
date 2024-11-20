/*
 * TTTech libbase
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

#ifndef INCLUDE_BASE_INTERFACES_LINUX_TOOLS_H_
#define INCLUDE_BASE_INTERFACES_LINUX_TOOLS_H_

#include "base_private_defines.h"
#include "base_defines.h"

/* ietf-interfaces - strings */
#define IF_MODULE_NAME_STR              "ietf-interfaces"
#define IF_PREFIX_STR                   "if"
#define IF_NS_STR                       "urn:ietf:params:xml:ns:yang:ietf-interfaces"
#define IF_RELOAD_FILE_STR              "/reload_ietf_interfaces.txt"
#define IF_RELOAD_FILE_PATH_STR         APPS_DIR_PATH IF_MODULE_NAME_STR IF_RELOAD_FILE_STR

/* containers, lists, leafs names */
#define IF_INTERFACES_CONTAINER         "interfaces"
#define IF_INTERFACE_LIST               "interface"
#define IF_NAME_STR                     "name"
#define IF_TYPE_STR                     "type"
#define IF_ENABLE_STR                   "enabled"
#define IF_COMPONENT_NAME               "component-name"
#define IF_STATISTICS_STR               "statistics"
#define IF_TX_STATISTICS_STR            "tx-statistics"
#define IF_RX_STATISTICS_STR            "rx-statistics"

/* ietf-interfaces specific values */
#define IF_ETH_CSMACD                   "ianaift:ethernetCsmacd"

/* ieee802-ethernet-interface - strings */
#define ETH_PREFIX                  "ieee802-eth-if"
#define ETH_MODULE_NAME             "ieee802-ethernet-interface"
#define ETH_NS                      "urn:ieee:std:802.3:yang:ieee802-ethernet-interface"

/* containers, tables, lists, leafs names */
#define ETH_ETHERNET_CONTAINER              "ethernet"
#define ETH_AUTO_NEGOTIATION                "auto-negotiation"
#define ETH_IF_ENABLE_STR                   "enable"
#define ETH_MAX_FRAME_LEN_STR               "max-frame-length"
#define ETH_SPEED_STR                       "speed"
#define ETH_DUPLEX_STR                      "duplex"
#define ETH_ADVERTISE_STR                   "advertise"
#define ETH_SUPPORTED_IF_MODE_STR           "sup_mode"
#define ETH_LINK_DETECTED_STR               "link-detected"
#define ETH_ADVERTISED_IF_MODE_STR          "store-advertised-modes"
#define ETH_SUPPORTED_MODES_STR         "store-supported-modes"
#define ETH_STARTUP_ADVERTISED_MODES_STR    "startup-advertised-modes"
#define ETH_FRAME                           "frame"
#define ETH_IN_TOTAL_FRAMES                 "in-total-frames"
#define ETH_IN_TOTAL_OCTETS                 "in-total-octets"
#define ETH_IN_FRAMES                       "in-frames"
#define ETH_IN_MULTICAST_FRAMES             "in-multicast-frames"
#define ETH_OUT_FRAMES                      "out-frames"

/* ieee802-ethernet-interface specific values */
#define ETH_DUPLEX_HALF             "Half"
#define ETH_DUPLEX_FULL             "Full"
#define ETH_SPEED_TEN               "0.010"
#define ETH_SPEED_ONE_HUNDRED       "0.100"
#define ETH_SPEED_ONE_THOUSAND      "1.000"
#define ETH_SPEED_TEN_THOUSAND      "10.000"
#define ETH_ADVERTISE_STR           "advertise"
#define ETH_BASE_T                  "baseT"
#define ETH_SUPPORTS_AUTO_NEG       "Supports auto-negotiation:"
#define ETH_ADVERTISE_PAUSE_FRAME   "Advertised pause frame use:"
#define ETH_SUPPORTED_PORTS     "Supported ports:"
#define ETH_SUPPORTED_PAUSE_FRAME   "Supported pause frame use:"

typedef struct {
    char mode[MAX_STR_LEN];
    char duplex[MAX_STR_LEN];
    char speed[MAX_STR_LEN];
    unsigned int mask;
} link_mode_t;

/* ietf-interfaces - functions */

/**
 * @brief Getting names of all interfaces n the system.
 *
 * @param[out]  result_array    Place holder for value(s) of the object(s).
 *                              Caller should provide address of char matrix as
 *                              result_array.
 * @param[in]   row_num         Number of result_array's rows.
 * @param[in]   col_num         Number of result_array's columns.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
int get_interface_names(void *result_array, unsigned int row_num, unsigned int col_num);

/**
 * @brief Getting value of enabled object (true or false).
 *
 * @param[out]  result_array    Place holder for value(s) of the object(s).
 *                              Caller should provide address of char matrix as
 *                              result_array. This functions fills only first
 *                              string from this array (can be BASE_LOWER_UP,
 *                              BASE_NO_CARRIER, "").
 * @param[in]   row_num         Number of result_array's rows.
 * @param[in]   col_num         Number of result_array's columns.
 * @param[in]   interface_name  Char pointer of the string which holds
 *                              interface name.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
int get_interface_enabled(void *result_array, unsigned int row_num, unsigned int col_num,
                          char *interface_name);

/**
 * @brief Setting value of enabled object (true or false).
 *
 * @param[in]   interface_name  Char pointer of the string which holds
 *                              interface name.
 * @param[in]   value           New value of enabled object (can be "up" or
 *                              "down").
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
int set_interface_enabled(char *interface_name, char *value);

/**
 * @brief Getting values of interface transmitted statistics objects.
 *
 * @param[out]  result_array    Place holder for value(s) of the object(s).
 *                              Caller should provide address of char matrix as
 *                              result_array. This functions fills only first
 *                              string from this array.
 * @param[in]   row_num         Number of result_array's rows.
 * @param[in]   col_num         Number of result_array's columns.
 * @param[in]   interface_name  Char pointer of the string which holds
 *                              interface name.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
int get_interface_tx_statistics(void *result_array, unsigned int row_num, unsigned int col_num,
                                char *interface_name);

/**
 * @brief Getting values of interface reveived statistics objects.
 *
 * @param[out]  result_array    Place holder for value(s) of the object(s).
 *                              Caller should provide address of char matrix as
 *                              result_array. This functions fills only first
 *                              string from this array.
 * @param[in]   row_num         Number of result_array's rows.
 * @param[in]   col_num         Number of result_array's columns.
 * @param[in]   interface_name  Char pointer of the string which holds
 *                              interface name.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
int get_interface_rx_statistics(void *result_array, unsigned int row_num, unsigned int col_num,
                                char *interface_name);

/**
 * @brief Getting value of ethernet auto-negotitation enable object (true or
 * false).
 *
 * @param[out]  result_array    Place holder for value(s) of the object(s).
 *                              Caller should provide address of char matrix as
 *                              result_array. This functions fills only first
 *                              string from this array.
 * @param[in]   row_num         Number of result_array's rows.
 * @param[in]   col_num         Number of result_array's columns.
 * @param[in]   interface_name  Char pointer of the string which holds
 *                              interface name.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
int get_eth_if_auto_neg_enable(void *result_array, unsigned int row_num, unsigned int col_num,
                               char *interface_name);

/**
 * @brief Getting value of ethernet duplex object (half or full).
 *
 * @param[out]  result_array    Place holder for value(s) of the object(s).
 *                              Caller should provide address of char matrix as
 *                              result_array. This functions fills only first
 *                              string from this array.
 * @param[in]   row_num         Number of result_array's rows.
 * @param[in]   col_num         Number of result_array's columns.
 * @param[in]   interface_name  Char pointer of the string which holds
 *                              interface name.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
int get_eth_if_duplex(void *result_array, unsigned int row_num, unsigned int col_num,
                      char *interface_name);

/**
 * @brief Getting value of ethernet speed object.
 *
 * @param[out]  result_array    Place holder for value(s) of the object(s).
 *                              Caller should provide address of char matrix as
 *                              result_array. This functions fills only first
 *                              string from this array.
 * @param[in]   row_num         Number of result_array's rows.
 * @param[in]   col_num         Number of result_array's columns.
 * @param[in]   interface_name  Char pointer of the string which holds
 *                              interface name.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
int get_eth_if_speed(void *result_array, unsigned int row_num, unsigned int col_num,
                     char *interface_name);

/**
 * @brief Getting value of ethernet max frame length object.
 *
 * @param[out]  result_array    Place holder for value(s) of the object(s).
 *                              Caller should provide address of char matrix as
 *                              result_array. This functions fills only first
 *                              string from this array.
 * @param[in]   row_num         Number of result_array's rows.
 * @param[in]   col_num         Number of result_array's columns.
 * @param[in]   interface_name  Char pointer of the string which holds
 *                              interface name.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
int get_ethernet_max_frame_length(void *result_array, unsigned int row_num, unsigned int col_num,
                                  char *interface_name);

/**
 * @brief Setting value of ethernet auto-negotitation enable object (true or
 * false).
 *
 * @param[in]   interface_name  Char pointer of the string which holds
 *                              interface name.
 * @param[in]   value New       value of enabled object.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
int set_eth_if_auto_neg_enable(char *interface_name, char *value);

/**
 * @brief Setting value of ethernet duplex and speed objects.
 *
 * @param[in]   object_name         Char pointer of the string which holds
 *                                  object name.
 * @param[in]   interface_name      Char pointer of the string which holds
 *                                  interface name.
 * @param[in]   duplex              New value of duplex object.
 * @param[in]   speed               New value of speed object.
 * @param[in]   with_advertising    Contains value which represents is speed
 *                                  and duplex with advertising or without
 *                                  advertising.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
int set_eth_if_duplex_and_speed(char *object_name, char *interface_name,
                                char *duplex, char *speed, int with_advertising);

/**
 * @brief Checking are speed and duplex supported on interface.
 *
 * @param[in]   rc_flag         If it is BASE_RC_YES, duplex need to be
 *                              translated.
 * @param[in]   interface_name  Char pointer of the string which holds
 *                              interface name.
 * @param[in]   duplex          New value of duplex object.
 * @param[in]   speed           New value of speed object.
 * @param[in]   mode            Char pointer of the string which holds
 *                              interface mode.
 * @param[in]   obj_name        Char pointer of the string which holds object
 *                              mode.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
int check_link_modes(base_rc_t rc_flag, char *interface_name,
                     char *speed, char *duplex, char *mode, char *obj_name);

/**
 * @brief Function which holds all supported modes for interface speed and
 * duplex.
 *
 * @param[in]   interface_name      Char pointer of the string which holds
 *                                  interface name.
 * @param[in]   with_advertising    Flag for supported or advertise link modes.
 *                                  0 for supported link modes, 1 for advertise
 *                                  link modes.
 * @return      Supported modes for input interface.
 */
uint32_t set_interface_link_modes(char *interface_name, int with_advertising);

/**
 * @brief Getting value of ethernet Link detected object.
 *
 * @param[out]  result_array    Place holder for value(s) of the object(s).
 *                              Caller should provide address of char matrix as
 *                              result_array. This functions fills only first
 *                              string from this array.
 * @param[in]   row_num         Number of result_array's rows.
 * @param[in]   col_num         Number of result_array's columns.
 * @param[in]   interface_name  Char pointer of the string which holds
 *                              interface name.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
int get_eth_link_detected(void *result_array, unsigned int row_num, unsigned int col_num,
                          char *interface_name);

/**
 * @brief This function will take all advertised link modes for one interface
 * and store it to the file.
 *
 * @param[out]  result_array    Place holder for value(s) of the object(s).
 *                              Caller should provide address of char matrix as
 *                              result_array. This functions fills only first
 *                              string from this array.
 * @param[in]   row_num         Number of result_array's rows.
 * @param[in]   col_num         Number of result_array's columns.
 * @param[in]   interface_name  Char pointer of the string which holds
 *                              interface name.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
int store_advertised_link_modes(void *result_array, unsigned int row_num, unsigned int col_num,
                                char *interface_name);

/**
 * @brief This function will take all SUPPORTED link modes for one interface and store it to the file.
 *
 * @param[out]  result_array    Place holder for value(s) of the object(s).
 *                              Caller should provide address of char matrix as
 *                              result_array. This functions fills only first
 *                              string from this array.
 * @param[in]   row_num         Number of result_array's rows.
 * @param[in]   col_num         Number of result_array's columns.
 * @param[in]   interface_name  Char pointer of the string which holds
 *                              interface name.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
int get_supported_link_modes(void *result_array, unsigned int row_num, unsigned int col_num,
                             char *interface_name);

#endif /* INCLUDE_BASE_INTERFACES_LINUX_TOOLS_H_ */
