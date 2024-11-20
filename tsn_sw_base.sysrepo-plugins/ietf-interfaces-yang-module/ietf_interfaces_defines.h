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

#ifndef IETF_INTERFACES_DEFINES_H_
#define IETF_INTERFACES_DEFINES_H_

#define INTERFACES_MODULE_NAME                          "ietf-interfaces"

/* path to ietf-interfaces startup datastore */
#define IF_STARTUP_DS_PATH                              "/etc/sysrepo/data/ietf-interfaces.startup"

#define IF_ETH_CSMACD_STR                               "iana-if-type:ethernetCsmacd"

/* list interface */
#define INTERFACE_XPATH                                 "/ietf-interfaces:interfaces/interface[name='%s']"
#define INTERFACE_IF_INDEX_XPATH                        INTERFACE_XPATH "/if-index"
#define INTERFACE_PHYS_ADDRESS_XPATH                    INTERFACE_XPATH "/phys-address"
#define INTERFACE_STATISTICS_SPEED_XPATH                INTERFACE_XPATH "/speed"

/* list interface, container statistics */
#define INTERFACE_STATISTICS_IN_OCTETS_XPATH            INTERFACE_XPATH "/statistics/in-octets"
#define INTERFACE_STATISTICS_IN_UNICAST_PKTS_XPATH      INTERFACE_XPATH "/statistics/in-unicast-pkts"
#define INTERFACE_STATISTICS_IN_MULTICAST_PKTS_XPATH    INTERFACE_XPATH "/statistics/in-multicast-pkts"
#define INTERFACE_STATISTICS_IN_DISCARDS_XPATH          INTERFACE_XPATH "/statistics/in-discards"
#define INTERFACE_STATISTICS_IN_ERRORS_XPATH            INTERFACE_XPATH "/statistics/in-errors"
#define INTERFACE_STATISTICS_OUT_OCTETS_XPATH           INTERFACE_XPATH "/statistics/out-octets"
#define INTERFACE_STATISTICS_OUT_UNICAST_PKTS_XPATH     INTERFACE_XPATH "/statistics/out-unicast-pkts"
#define INTERFACE_STATISTICS_OUT_MULTICAST_PKTS_XPATH   INTERFACE_XPATH "/statistics/out-multicast-pkts"
#define INTERFACE_STATISTICS_OUT_DISCARDS_XPATH         INTERFACE_XPATH "/statistics/out-discards"
#define INTERFACE_STATISTICS_OUT_ERRORS_XPATH           INTERFACE_XPATH "/statistics/out-errors"

#define IF_INDEX_STR                                    "if-index"
#define IF_PHYS_ADDRESS_STR                             "phys-address"
#define IF_SPEED_STR                                    "speed"
#define IF_IN_OCTETS_STR                                "in-octets"
#define IF_IN_UNICAST_PKTS_STR                          "in-unicast-pkts"
#define IF_IN_MULTICAST_PKTS_STR                        "in-multicast-pkts"
#define IF_IN_DISCARDS_STR                              "in-discards"
#define IF_IN_ERRORS_STR                                "in-errors"
#define IF_OUT_OCTETS_STR                               "out-octets"
#define IF_OUT_UNICAST_PKTS_STR                         "out-unicast-pkts"
#define IF_OUT_MULTICAST_PKTS_STR                       "out-multicast-pkts"
#define IF_OUT_DISCARDS_STR                             "out-discards"
#define IF_OUT_ERRORS_STR                               "out-errors"

/* speed macros */
#define IF_SPEED_TEN_STR                                "10Mb/s"
#define IF_SPEED_ONE_HUNDRED_STR                        "100Mb/s"
#define IF_SPEED_ONE_THOUSAN_STR                        "1000Mb/s"

#define ERR_FAILED_TO_GET_STATISTICS                    "Failed to get interface rx and tx statistics!"

static char if_xml_enable_map[3][2][MAX_STR_LEN] = {
    // 3 pairs
    { BASE_LOWER_UP  ,  BASE_TRUE },
    { BASE_NO_CARRIER,  BASE_TRUE },
    { ""             ,  BASE_FALSE },
};

static char if_enable_map[2][2][MAX_STR_LEN] = {
    // 2 pairs
    { BASE_TRUE, BASE_UP },
    { BASE_FALSE, BASE_DOWN },
};

static char *bool_map[2] = {
    // 2 pairs
    BASE_FALSE,
    BASE_TRUE
};

static char interface_speed_map[3][2][MAX_STR_LEN] = {
    // 4 pairs
    { IF_SPEED_TEN_STR,         "10000000" },
    { IF_SPEED_ONE_HUNDRED_STR, "100000000" },
    { IF_SPEED_ONE_THOUSAN_STR, "1000000000" }
};

#endif
