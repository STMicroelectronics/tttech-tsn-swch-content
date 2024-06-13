/*
 * TTTech ieee802-dot1ab-lldp-yang-module
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

#ifndef LLDP_DEFINES_H_
#define LLDP_DEFINES_H_

#define LLDP_MODULE_NAME                                "ieee802-dot1ab-lldp"

/* CREATED */

#define LLDP_XPATH                                      "/ieee802-dot1ab-lldp:lldp"

/* LIST PORT */
#define LLDP_PORT_XPATH                                 LLDP_XPATH "/port[name='%s'][dest-mac-address='%s']"
#define LLDP_PORT_LIST_PORT_ID_XPATH                    LLDP_PORT_XPATH "/port-id"
#define LLDP_PORT_LIST_PORT_ID_SUBTYPE_XPATH            LLDP_PORT_XPATH "/port-id-subtype"
#define LLDP_PORT_LIST_PORT_DESC                        LLDP_PORT_XPATH "/port-desc"

/* LIST REMOTE SYSTEMS DATA */
#define LLDP_REMOTE_SYS_DATA_XPATH                      LLDP_PORT_XPATH "/remote-systems-data[time-mark='%u'][remote-index='%s']"
#define LLDP_REMOTE_SYS_DATA_CHASSIS_ID_SUBTYPE_XPATH   LLDP_REMOTE_SYS_DATA_XPATH "/chassis-id-subtype"
#define LLDP_REMOTE_SYS_DATA_CHASSIS_ID_XPATH           LLDP_REMOTE_SYS_DATA_XPATH "/chassis-id"
#define LLDP_REMOTE_SYS_DATA_PORT_ID_SUBTYPE_XPATH      LLDP_REMOTE_SYS_DATA_XPATH "/port-id-subtype"
#define LLDP_REMOTE_SYS_DATA_PORT_ID_XPATH              LLDP_REMOTE_SYS_DATA_XPATH "/port-id"
#define LLDP_REMOTE_SYS_DATA_PORT_DESC_XPATH            LLDP_REMOTE_SYS_DATA_XPATH "/port-desc"
#define LLDP_REMOTE_SYS_DATA_SYS_NAME_XPATH             LLDP_REMOTE_SYS_DATA_XPATH "/system-name"
#define LLDP_REMOTE_SYS_DATA_SYS_DESC_XPATH             LLDP_REMOTE_SYS_DATA_XPATH "/system-description"
#define LLDP_REMOTE_SYS_DATA_SYS_CAP_XPATH              LLDP_REMOTE_SYS_DATA_XPATH "/system-capabilities-supported"
#define LLDP_REMOTE_SYS_DATA_SYS_CAP_EN_XPATH           LLDP_REMOTE_SYS_DATA_XPATH "/system-capabilities-enabled"

/* LIST MANAGEMENT ADDRESS */
#define LLDP_MANAG_ADDRESS_ENTRY_XPATH                  LLDP_REMOTE_SYS_DATA_XPATH "/management-address[address-subtype='%s'][address='%s']"

/* CONTAINER LOCAL SYSTEM DATA */
#define LLDP_LOC_SYS_DATA_XPATH                         LLDP_XPATH "/local-system-data"
#define LLDP_LOC_SYS_DATA_CHASSIS_ID_SUBTYPE_XPATH      LLDP_LOC_SYS_DATA_XPATH "/chassis-id-subtype"
#define LLDP_LOC_SYS_DATA_CHASSIS_ID_XPATH              LLDP_LOC_SYS_DATA_XPATH "/chassis-id"
#define LLDP_LOC_SYS_DATA_SYS_NAME_XPATH                LLDP_LOC_SYS_DATA_XPATH "/system-name"
#define LLDP_LOC_SYS_DATA_SYS_DESC_XPATH                LLDP_LOC_SYS_DATA_XPATH "/system-description"
#define LLDP_LOC_SYS_DATA_SYS_CAP_XPATH                 LLDP_LOC_SYS_DATA_XPATH "/system-capabilities-supported"
#define LLDP_LOC_SYS_DATA_SYS_CAP_EN_XPATH              LLDP_LOC_SYS_DATA_XPATH "/system-capabilities-enabled"

/* CONTAINER REMOTE STATISTICS */
#define LLDP_REMOTE_STAT_XPATH                          "/ieee802-dot1ab-lldp:lldp/remote-statistics"
#define LLDP_REMOTE_STAT_RM_INSTERTS_XPATH              LLDP_REMOTE_STAT_XPATH "/remote-inserts"
#define LLDP_REMOTE_STAT_RM_DELETES_XPATH               LLDP_REMOTE_STAT_XPATH "/remote-deletes"
#define LLDP_REMOTE_STAT_RM_DROPS_XPATH                 LLDP_REMOTE_STAT_XPATH "/remote-drops"
#define LLDP_REMOTE_STAT_RM_AGEOUTS_XPATH               LLDP_REMOTE_STAT_XPATH "/remote-ageouts"

/* LIST PORT CONTAINER RX STATISTICS */
#define LLDP_PORT_RX_STAT_XPATH                         LLDP_PORT_XPATH "/rx-statistics"
#define LLDP_PORT_RX_STAT_TOT_AGEOUTS_XPATH             LLDP_PORT_RX_STAT_XPATH "/total-ageouts"
#define LLDP_PORT_RX_STAT_TOT_DIS_FRAMES_XPATH          LLDP_PORT_RX_STAT_XPATH "/total-discarded-frames"
#define LLDP_PORT_RX_STAT_TOT_FRAMES_XPATH              LLDP_PORT_RX_STAT_XPATH "/total-frames"
#define LLDP_PORT_RX_STAT_TOT_DIS_TLVS_XPATH            LLDP_PORT_RX_STAT_XPATH "/total-discarded-tlvs"
#define LLDP_PORT_RX_STAT_TOT_UN_TLVS_XPATH             LLDP_PORT_RX_STAT_XPATH "/total-unrecognized-tlvs"

/* LIST PORT CONTAINER TX STATISTICS */
#define LLDP_PORT_TX_STAT_TOT_FRAMES_XPATH              LLDP_PORT_XPATH "/tx-statistics/total-frames"

/* path to lldp startup datastore */
#define LLDP_STARTUP_DS_PATH                            "/etc/sysrepo/data/ieee802-dot1ab-lldp.startup"

/* defines from xml_base */

/* containers, tables, lists, leafs names */
#define LLDP_MODULE_NAME_STR                "ieee802-dot1ab-lldp"
#define LLDP_ROOT_STR                       "lldp"
#define LLDP_BRIDGE_STR                     "bridge"
#define LLDP_CONF_STR                       "grouping lldp-cfg"
#define LLDP_MSG_FAST_TX_STR                "message-fast-tx"
#define LLDP_TX_HOLD_MULTIPLIER_STR         "message-tx-hold-multiplier"
#define LLDP_MSG_TX_INTERVAL_STR            "message-tx-interval"
#define LLDP_REINIT_DELAY_STR               "reinit-delay"
#define LLDP_TX_CREDIT_MAX_STR              "tx-credit-max"
#define LLDP_TX_FAST_INIT_STR               "tx-fast-init"
#define LLDP_NOTIF_INTERVAL_STR             "notification-interval"

#define LLDP_REMOTE_STATS_STR               "remote-statistics"
#define LLDP_LAST_CHANGE_TIM_STR            "last-change-time"
#define LLDP_REMOTE_INSTERTS_STR            "remote-inserts"
#define LLDP_REMOTE_DELETES_STR             "remote-deletes"
#define LLDP_REMOTE_DROPS_STR               "remote-drops"
#define LLDP_REMOTE_AGEOUTS_STR             "remote-ageouts"

#define LLDP_LOCAL_SYS_DATA_STR             "local-system-data"
#define LLDP_CHASSIS_ID_SYBTYPE_STR         "chassis-id-subtype"
#define LLDP_CHASSIS_ID_STR                 "chassis-id"
#define LLDP_SYS_NAME_STR                   "system-name"
#define LLDP_SYS_DESCR_STR                  "system-description"
#define LLDP_SYS_CAP_SUPPPORTED_STR         "system-capabilities-supported"
#define LLDP_SYS_CAP_ENABLED_STR            "system-capabilities-enabled"

#define LLDP_PORT_STR                       "port"
#define LLDP_PORT_NAME_STR                  "name"
#define LLDP_DEST_MAC_ADDRESS_STR           "dest-mac-address"
#define LLDP_MAN_ADDR_TX_PORT_STR           "management-address-tx-port"
#define LLDP_ADDR_SUBTYPE_STR               "address-subtype"
#define LLDP_ADDRESS_STR                    "address"
#define LLDP_ADDR_LEN_STR                   "addr-len"
#define LLDP_PORT_ID_SUBTYPE_STR            "port-id-subtype"
#define LLDP_PORT_ID_STR                    "port-id"
#define LLDP_PORT_DESRC_STR                 "port-desc"

#define LLDP_TX_STATISTICS_STR              "tx-statistics"
#define LLDP_TOTAL_FRAMES_STR               "total-frames"

#define LLDP_RX_STATISTICS_STR              "rx-statistics"
#define LLDP_TOTAL_AGEOUTS_STR              "total-ageouts"
#define LLDP_TOTAL_DSCRD_FRAMES_STR         "total-discarded-frames"
#define LLDP_TOTAL_FRAMES_STR               "total-frames"
#define LLDP_TOTAL_DSCRD_TLVS_STR           "total-discarded-tlvs"
#define LLDP_TOTAL_UNRCGNZ_TLVS_STR         "total-unrecognized-tlvs"

#define LLDP_REMOTE_SYS_DATA_STR            "remote-systems-data"
#define LLDP_TIME_MARK_STR                  "time-mark"
#define LLDP_REMOTE_INDEX_STR               "remote-index"
#define LLDP_PORT_ID_SUBTYPE_STR            "port-id-subtype"
#define LLDP_PORT_ID_STR                    "port-id"
#define LLDP_PORT_DESCR_STR                 "port-desc"
#define LLDP_SYS_MAN_ADDR_STR               "management-address"
#define LLDP_MAN_ADDRESS_STR                "man-address"

#define SYS_CAPABILITY_LENGTH               20
#define SYS_CAPABILITY_MAX_NUM              10

#define LLPD_ACTIVATE_MSG_STR               "It is not possible to activate/deactivate lldp on the interface!"

#define LLPD_MGMT_ADD_TX_PORT_MSG_STR       "It is not possible to modify management-address-tx-port list!"

#define LLDP_CAP_OTHER_STR                  "other"
#define LLDP_CAP_REPEATER_STR               "repeater"
#define LLDP_CAP_BRIDGE_STR                 "bridge"
#define LLDP_CAP_WLAN_STR                   "wlan-access-point"
#define LLDP_CAP_ROUTER_STR                 "router"
#define LLDP_CAP_TELEPHONE_STR              "telephone"
#define LLDP_CAP_DOCSYS_CABLE_STR           "docsis-cable-device"
#define LLDP_CAP_STATION_STR                "station-only"

#define LLDP_PORT_STR                       "port"
#define LLDP_PORT_NAME_STR                  "name"
#define LLDP_DEST_MAC_ADDRESS_STR           "dest-mac-address"
#define LLDP_MAN_ADDR_TX_PORT_STR           "management-address-tx-port"
#define LLDP_ADDR_SUBTYPE_STR               "address-subtype"
#define LLDP_ADDRESS_STR                    "address"
#define LLDP_ADDR_LEN_STR                   "addr-len"
#define LLDP_PORT_ID_SUBTYPE_STR            "port-id-subtype"
#define LLDP_PORT_ID_STR                    "port-id"
#define LLDP_PORT_DESRC_STR                 "port-desc"

#define LLDP_TX_STATISTICS_STR              "tx-statistics"
#define LLDP_RX_STATISTICS_STR              "rx-statistics"

#define LLDP_REMOTE_SYS_DATA_STR            "remote-systems-data"
#define LLDP_TIME_MARK_STR                  "time-mark"
#define LLDP_REMOTE_INDEX_STR               "remote-index"

/* lldp port subtype defines */
#define LLDP_CHASSIS_COMPONENT_STR          "chassis-component"
#define LLDP_INTERFACE_ALIAS_STR            "interface-alias"
#define LLDP_PORT_COMPONENT_STR             "port-component"
#define LLDP_MAC_STR                        "mac-address"
#define LLDP_NETWORK_ADDRESS_STR            "network-address"
#define LLDP_INTERFACE_NAME_STR             "interface-name"
#define LLDP_AGENT_CIRCUIT_ID_STR           "agent-circuit-id"
#define LLDP_LOCAL_STR                      "local"

/* lldp port subtype defines */
#define LLDPCTL_CHASSIS_COMPONENT_STR       "unhandled"
#define LLDPCTL_INTERFACE_ALIAS_STR         "ifalias"
#define LLDPCTL_PORT_COMPONENT_STR          "unhandled"
#define LLDPCTL_MAC_STR                     "mac"
#define LLDPCTL_NETWORK_ADDRESS_STR         "ip"
#define LLDPCTL_INTERFACE_NAME_STR          "ifname"
#define LLDPCTL_LOCAL_STR                   "local"
#define LLDPCTL_AGENT_CIRCUIT_ID_STR        "unhandled"

#define LLDP_WRONG_DATA_TYPE_STR            "AB-LLDP management wrong data type."

/* lldp-ab */
typedef enum {
    LLDP_ROOT = 0,
    LLDP_BRIDGE,
    LLDP_CONF,
    LLDP_MSG_FAST_TX,
    LLDP_TX_HOLD_MULTIPLIER,
    LLDP_MSG_TX_INTERVAL,
    LLDP_REINIT_DELAY,
    LLDP_TX_CREDIT_MAX,
    LLDP_TX_FAST_INIT,
    LLDP_NOTIF_INTERVAL,
    LLDP_REMOTE_STATS,
    LLDP_LAST_CHANGE_TIME,
    LLDP_REMOTE_INSTERTS,
    LLDP_REMOTE_DELETES,
    LLDP_REMOTE_DROPS,
    LLDP_REMOTE_AGEOUTS,
    LLDP_LOCAL_SYS_DATA,
    LLDP_CHASSIS_ID_SYBTYPE,
    LLDP_CHASSIS_ID,
    LLDP_SYS_NAME,
    LLDP_SYS_DESCR,
    LLDP_SYS_CAP_SUPPPORTED,
    LLDP_SYS_CAP_ENABLED,
    LLDP_PORT,
    LLDP_PORT_NAME,
    LLDP_DEST_MAC_ADDRESS,
    LLDP_MAN_ADDR_TX_PORT,
    LLDP_ADDR_SUBTYPE,
    LLDP_MAN_ADDRESS,
    LLDP_ADDR_LEN,
    LLDP_PORT_ID_SUBTYPE,
    LLDP_PORT_ID,
    LLDP_PORT_DESRC,
    LLDP_TX_STATISTICS,
    LLDP_TOTAL_FRAMES,
    LLDP_RX_STATISTICS,
    LLDP_TOTAL_AGEOUTS,
    LLDP_TOTAL_DSCRD_FRAMES,
    LLDP_TOTAL_DSCRD_TLVS,
    LLDP_TOTAL_UNRCGNZ_TLVS,
    LLDP_REMOTE_SYS_DATA,
    LLDP_TIME_MARK,
    LLDP_REMOTE_INDEX,
    LLDP_PORT_DESCR,
    LLDP_SYS_MAN_ADDR,
    LLDP_ADDRESS,
    LLDP_MAX_OBJ_NUMBER,
} lldp_obj_index_t;

/* port-id-subtype mapping */
static char port_id_subtype_map[7][2][MAX_STR_LEN] = {
    { LLDPCTL_INTERFACE_ALIAS_STR,  LLDP_INTERFACE_ALIAS_STR },
    { LLDPCTL_PORT_COMPONENT_STR,   LLDP_PORT_COMPONENT_STR },
    { LLDPCTL_MAC_STR,              LLDP_MAC_STR },
    { LLDPCTL_NETWORK_ADDRESS_STR,  LLDP_NETWORK_ADDRESS_STR },
    { LLDPCTL_INTERFACE_NAME_STR,   LLDP_INTERFACE_NAME_STR },
    { LLDPCTL_AGENT_CIRCUIT_ID_STR, LLDP_AGENT_CIRCUIT_ID_STR },
    { LLDPCTL_LOCAL_STR,            LLDP_LOCAL_STR },
};

/* chassis-id-subtype mapping */
static char chassis_id_subtype_map[7][2][MAX_STR_LEN] = {
    { LLDPCTL_CHASSIS_COMPONENT_STR,    LLDP_CHASSIS_COMPONENT_STR },
    { LLDPCTL_INTERFACE_ALIAS_STR,      LLDP_INTERFACE_ALIAS_STR },
    { LLDPCTL_PORT_COMPONENT_STR,       LLDP_PORT_COMPONENT_STR },
    { LLDPCTL_MAC_STR,                  LLDP_MAC_STR },
    { LLDPCTL_NETWORK_ADDRESS_STR,      LLDP_NETWORK_ADDRESS_STR },
    { LLDPCTL_INTERFACE_NAME_STR,       LLDP_INTERFACE_NAME_STR },
    { LLDPCTL_LOCAL_STR,                LLDP_LOCAL_STR },
};

/* map between object index and object name for lldp-ab module */
char lldp_map[LLDP_MAX_OBJ_NUMBER][MAX_STR_LEN] = {
    [LLDP_TX_HOLD_MULTIPLIER]        = LLDP_TX_HOLD_MULTIPLIER_STR,
    [LLDP_MSG_TX_INTERVAL]           = LLDP_MSG_TX_INTERVAL_STR,
};

static char sys_cap_str_array[BASE_EIGHT][SYS_CAPABILITY_LENGTH] = {
    LLDP_CAP_OTHER_STR,
    LLDP_CAP_REPEATER_STR,
    LLDP_CAP_BRIDGE_STR,
    LLDP_CAP_WLAN_STR,
    LLDP_CAP_ROUTER_STR,
    LLDP_CAP_TELEPHONE_STR,
    LLDP_CAP_DOCSYS_CABLE_STR,
    LLDP_CAP_STATION_STR
};

static uint8_t sys_cap_bits[BASE_EIGHT] = {
    LLDP_CAP_OTHER,
    LLDP_CAP_REPEATER,
    LLDP_CAP_BRIDGE,
    LLDP_CAP_WLAN,
    LLDP_CAP_ROUTER,
    LLDP_CAP_TELEPHONE,
    LLDP_CAP_DOCSIS,
    LLDP_CAP_STATION
};

#endif /* LLDP_DEFINES_H_ */
