/*
 * TTTech libbase
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

#ifndef INCLUDE_BASE_DEFINES_H_
#define INCLUDE_BASE_DEFINES_H_

#include <stdint.h>

/* number defines */
#define MAX_STR_LEN           255
#define MAX_UNINT_32_VALUE    4294967296
#define MAX_NUMBER_OF_ACL     2047
#define DEFAULT_VID           "4095"
#define IF_STAT_LEN           16
#define IF_STAT_RES_TIME      21
#define ID_LEN                12
#define MAX_NUM_OF_INTERFACES 30
#define BASE_MAX_VID          4096
#define BASE_MAX_NUM_OF_STATIC_FDB_ENTRIES 200
#define BASE_MIN_VID          0
#define BASE_ONE              1
#define BASE_TWO              2
#define BASE_THREE            3
#define BASE_FIVE             5
#define BASE_EIGHT            8
#define BASE_TEN              10
/* usual max size of row/column */
#define BASE_MAX_ROW_NUM      ((BASE_MAX_VID > BASE_MAX_NUM_OF_STATIC_FDB_ENTRIES) ? BASE_MAX_VID : BASE_MAX_NUM_OF_STATIC_FDB_ENTRIES)
#define BASE_MAX_COL_NUM      ((MAX_STR_LEN > BASE_TEN) ? MAX_STR_LEN : BASE_TEN)
#define BASE_ZERO_STR         "0"
#define BASE_ONE_STR          "1"
#define BASE_TWO_STR          "2"
#define BASE_THREE_STR        "3"
#define BASE_FOUR_STR         "4"
#define BASE_FIVE_STR         "5"
#define BASE_SIX_STR          "6"
#define BASE_SEVEN_STR        "7"
#define BASE_EIGHT_STR        "8"
#define BASE_NINE_STR         "9"
#define BASE_TEN_STR          "10"
#define BASE_THIRTY_STR       "30"
#define BASE_THIRTY_TWO_STR   "32"
#define BASE_254_STR          "254"
#define BASE_ONE_HUNDRED_STR  "100"
#define BASE_ONE_THOUSAND_STR "1000"
#define BASE_TEN_THOUSAND_STR "10000"
#define Q_BRIDGE_STR_4096     "4096"
#define BASE_SPEED_TEN_STR          "10Mb/s"
#define BASE_SPEED_ONE_HUNDRED_STR  "100Mb/s"
#define BASE_SPEED_ONE_THOUSAND_STR "1000Mb/s"
#define BASE_SPEED_TEN_THOUSAND_STR "10000Mb/s"
/* define strings */
#define BASE_TRUE            "true"
#define BASE_FALSE           "false"
#define BASE_ON              "on"
#define BASE_OFF             "off"
#define BASE_NAME            "name"
#define BASE_UP              "up"
#define BASE_DOWN            "down"
#define BASE_YES             "yes"
#define BASE_NO              "no"
#define BASE_LOWER_UP        "LOWER_UP"
#define BASE_NO_CARRIER      "NO-CARRIER"
#define BASE_UP_U            "UP"
#define BASE_DOWN_U          "DOWN"
#define BASE_TESTING         "TESTING"
#define BASE_UNKNOWN         "UNKNOWN"
#define BASE_DORMANT         "DORMANT"
#define BASE_NOT_PRESENT     "NOT_PRESENT"
#define BASE_LOWER_DOWN      "LOWER_DOWN"
/* used for mstp */
#define BASE_ROLE_DISABLED_STR    "Disabled"
#define BASE_ROOT_STR        "Root"
#define BASE_ALTERNATE_STR   "Alternate"
#define BASE_DESIGNATED_STR  "Designated"
#define BASE_BACKUP_STR      "Backup"
#define BASE_DISABLED_STR    "disabled"
#define BASE_LISTENING_STR   "listening"
#define BASE_LEARNING_STR    "learning"
#define BASE_FORWARDING_STR  "forwarding"
#define BASE_BLOCKING_STR    "blocking"
#define BASE_DISCARDING_STR  "discarding"
/* used for ethernet */
#define BASE_ETH_AUTO_NEG       "Auto-negotiation:"
#define BASE_ETH_DUPLEX         "Duplex:"
#define BASE_ETH_DUPLEX_HALF    "half"
#define BASE_ETH_DUPLEX_FULL    "full"
#define BASE_ETH_SPEED          "Speed:"
#define BASE_ETH_LINK_DETECTED  "Link detected:"
#define BASE_MTU                "mtu"
#define BASE_IF_RX              "RX:"
#define BASE_IF_TX              "TX:"
/* used for q_bridge */
#define BASE_LINK_ETHER      "link/ether"
#define BASE_UNTAGGED        "untagged"
#define BASE_TAGGED          "tagged"
#define BASE_AUTO            "auto"
#define BASE_ACL_LENGTH      "admin-control-list-length"
#define BASE_PORT_STR        "port"
#define BASE_DEV_STR         "dev"

/* Indicates module */
typedef enum {
    IETF_INTERFACES = 0,
    MSTP,
    SPANNING,
    Q_BRIDGE,
} module_index_t;

#define APPS_DIR_PATH               "/usr/share/applications/"
#define PORT_ADVERTISE_FILE_PATH    "/%s_advertise_link_modes.txt"
#define PORT_ADVERTISE_FULL_PATH    APPS_DIR_PATH PORT_ADVERTISE_FILE_PATH

/* ethtool */
#define SUPPORTED_10baseT_Half       (1 << 0)
#define SUPPORTED_10baseT_Full       (1 << 1)
#define SUPPORTED_100baseT_Half      (1 << 2)
#define SUPPORTED_100baseT_Full      (1 << 3)
#define SUPPORTED_1000baseT_Half     (1 << 4)
#define SUPPORTED_1000baseT_Full     (1 << 5)
#define SUPPORTED_10000baseT_Full    (1 << 12)
#define SUPPORTED_2500baseX_Full     (1 << 15)
#define SUPPORTED_1000baseKX_Full    (1 << 17)
#define SUPPORTED_10000baseKX4_Full  (1 << 18)
#define SUPPORTED_10000baseKR_Full   (1 << 19)
#define SUPPORTED_20000baseMLD2_Full (1 << 21)
#define SUPPORTED_20000baseKR2_Full  (1 << 22)
#define SUPPORTED_40000baseKR4_Full  (1 << 23)
#define SUPPORTED_40000baseCR4_Full  (1 << 24)
#define SUPPORTED_40000baseSR4_Full  (1 << 25)
#define SUPPORTED_40000baseLR4_Full  (1 << 26)

#define ALL_LINK_MODES_SUPPORTED     0
#define LINK_MODE_ADVERTISING        0

#endif /* INCLUDE_BASE_DEFINES_H_ */
