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

#ifndef Q_BRIDGE_DEFINES_H_
#define Q_BRIDGE_DEFINES_H_

#define BRIDGE_MODULE_NAME                              "ieee802-dot1q-bridge"

#define ERR_BRIDE_PORT_NAMES_FAILED_STR                 "Failed to get bridge port names."
#define ERR_BRIDE_VLAN_FAILED_STR                       "Failed to get bridge vlans."
#define ERR_BRIDE_MSTP_FIDS_FAILED_STR                  "Failed to get bridge mstp fids."
#define ERR_BRIDE_MSTP_VIDS_FAILED_STR                  "Failed to get bridge mstp vids."
#define ERR_BRIDE_MSTP_FID_BY_VIDS_FAILED_STR           "Failed to get bridge mstp fid by vids."
#define ERR_BRIDE_PORT_CONTROL_ELEMENT_STR              "Only forward type of the filtering entry is supported"
#define ERR_BRIDGE_DESCENDING_VIDS_XPATH_STR            "The vids [%s] contains descending interval for '%s'."
#define ERR_BRIDGE_INVALID_RANGE_VIDS_XPATH_STR         "The vids [%s] contains invalid range for '%s'."
#define ERR_BRIDGE_PORT_MAP_MISSING_XPATH_STR           "The port-map is missing for vids '%s' at '%s'."
#define ERR_BRIDGE_DATABE_ID_SUPPORETD_XPATH_STR        "Only database-id '%s' is supported for '%s'."
#define ERR_BRIDGE_TOO_MANY_VIDS_XPATH_STR              "The number of different vids needs to be less than %u for '%s'."
#define ERR_BRIDGE_PVID_IN_VIDS_XPATH_STR               "This is not allowed: the pvid is in vids string for '%s'."
#define ERR_BRIDGE_NO_PVID_XPATH_STR                    "The pvid does not exist for '%s'."
#define ERR_BRIDGE_FAILED_TO_GET_VLAN_LIST_STR          "Failed to get vlan entries inside callback for %s."
#define ERR_BRIDGE_FAILED_TO_SET_VLAN_VID_STR           "Failed to set vid for desired vlan inside callback for %s."
#define ERR_BRIDGE_FAILED_TO_GET_VID2FID_LIST_STR       "Failed to get vid-to-fid-allocation entries inside callback for %s."
#define ERR_BRIDGE_FAILED_TO_SET_VID2FID_VIDS_STR       "Failed to set vids for desired fid inside callback for %s."
#define ERR_BRIDGE_FAILED_TO_GET_FID2VID_LIST_STR       "Failed to get fid-to-vid-allocation entries inside callback for %s."
#define ERR_BRIDGE_FAILED_TO_SET_FID2VID_FID_STR        "Failed to set fid for desired vids inside callback for %s."
#define ERR_BRIDGE_FAILED_TO_ALLOCATE_MEMORY_STR        "Failed to allocate memory for storing %s entries inside %s callback."

/* list bridge */
#define BRIDGE_XPATH                                    "/ieee802-dot1q-bridge:bridges/bridge[name='%s']"
#define BRIDGE_PORTS_XPATH                              BRIDGE_XPATH "/ports"
#define BRIDGE_UP_TIME_XPATH                            BRIDGE_XPATH "/up-time"
#define BRIDGE_COMPONENTS_XPATH                         BRIDGE_XPATH "/components"
#define BRIDGE_ADDRESS_XPATH                            BRIDGE_XPATH "/address"
#define BRIDGE_COMPONENT_XPATH                          BRIDGE_XPATH "/component[name='%s']"
#define BRIDGE_FILTERING_DATABASE_XPATH                 BRIDGE_COMPONENT_XPATH "/filtering-database//*"

/* list component */
#define BRIDGE_COMPONENT_PORTS_XPATH                    BRIDGE_COMPONENT_XPATH "/ports"
#define BRIDGE_COMPONENT_ADDRESS_XPATH                  BRIDGE_COMPONENT_XPATH "/address"
#define BRIDGE_COMPONENT_BRIDGE_PORT_XPATH              BRIDGE_COMPONENT_XPATH "/bridge-port"

/* defines used to know if bridge or component entries where deleted or added */
#define BRIDGE_OPERATION                                BRIDGE_XPATH ""
#define BRIDGE_COMPONENT_OPERATION                      BRIDGE_COMPONENT_XPATH ""
#define BRIDGE_PORT_MAP_OPERATION                       BRIDGE_COMPONENT_XPATH "/filtering-database/vlan-registration-entry[database-id='%s'][vids='%s']/port-map[port-ref='%s']"

/* list component, container capabilities */
#define BRIDGE_COMPONENT_EXTENDED_FILTERING_XPATH       BRIDGE_COMPONENT_XPATH "/capabilities/extended-filtering"
#define BRIDGE_COMPONENT_TRAFFIC_CLASS_XPATH            BRIDGE_COMPONENT_XPATH "/capabilities/traffic-classes"
#define BRIDGE_COMPONENT_STAT_ENTRY_IND_PORT_XPATH      BRIDGE_COMPONENT_XPATH "/capabilities/static-entry-individual-port"
#define BRIDGE_COMPONENT_IVL_CAPABLE_XPATH              BRIDGE_COMPONENT_XPATH "/capabilities/ivl-capable"
#define BRIDGE_COMPONENT_SVL_CAPABLE_XPATH              BRIDGE_COMPONENT_XPATH "/capabilities/svl-capable"
#define BRIDGE_COMPONENT_HYBRID_CAPABLE_XPATH           BRIDGE_COMPONENT_XPATH "/capabilities/hybrid-capable"
#define BRIDGE_COMPONENT_CONF_PVID_TAGGING_XPATH        BRIDGE_COMPONENT_XPATH "/capabilities/configurable-pvid-tagging"
#define BRIDGE_COMPONENT_LOCAL_VLAN_CAPABLE_XPATH       BRIDGE_COMPONENT_XPATH "/capabilities/local-vlan-capable"

/* list component, container filtering database, list vlan-registration-entry and filtering-entry, list port map */
#define BRIDGE_NAME_XPATH                               BRIDGE_XPATH "/name"
#define BRIDGE_COMPONENT_ID_XPATH                       BRIDGE_COMPONENT_XPATH "/id"
#define BRIDGE_FDB_XPATH                                BRIDGE_COMPONENT_XPATH "/filtering-database"
#define BRIDGE_COMPONENT_SIZE_XPATH                     BRIDGE_FDB_XPATH "/size"
#define BRIDGE_COMPONENT_STATIC_ENTRIES_XPATH           BRIDGE_FDB_XPATH "/static-entries"
#define BRIDGE_COMPONENT_DYNAMIC_ENTRIES_XPATH          BRIDGE_FDB_XPATH "/dynamic-entries"
#define BRIDGE_COMPONENT_STAT_VLAN_REG_ENTRIES_XPATH    BRIDGE_FDB_XPATH "/static-vlan-registration-entries"
#define BRIDGE_COMPONENT_DYNAM_VLAN_REG_ENTRIES_XPATH   BRIDGE_FDB_XPATH "/dynamic-vlan-registration-entries"
#define BRIDGE_COMPONENT_MAC_ADDRESS_REG_ENTRIES_XPATH  BRIDGE_FDB_XPATH "/mac-address-registration-entries"
#define BRIDGE_PORT_REF_XPATH                           BRIDGE_FDB_XPATH "/vlan-registration-entry[database-id='%s'][vids='%s']/port-map/port-ref"
#define BRIDGE_PORT_MAP_XPATH                           BRIDGE_FDB_XPATH "/vlan-registration-entry[database-id='%s'][vids='%s']/port-map"
#define BRIDGE_VLAN_TRNASMITTED_XPATH                   BRIDGE_FDB_XPATH "/vlan-registration-entry[database-id='%s'][vids='%s']/port-map[port-ref='%s']/static-vlan-registration-entries/vlan-transmitted"
#define BRIDGE_FILT_ENTRY_PORT_MAP_XPATH                BRIDGE_FDB_XPATH "/filtering-entry[database-id='%s'][vids='%s'][address='%s']/port-map[port-ref='%s']"
#define BRIDGE_FILTERING_ENTRY_XPATH                    BRIDGE_FDB_XPATH "/filtering-entry[database-id='%s'][vids='%s'][address='%s']"
#define BRIDGE_FILTERING_ENTRY_DATABASE_ID_XPATH        BRIDGE_FDB_XPATH "/filtering-entry[database-id='%s'][vids='%s'][address='%s']/database-id"
#define BRIDGE_FILTERING_ENTRY_ADDRESS_XPATH            BRIDGE_FDB_XPATH "/filtering-entry[database-id='%s'][vids='%s'][address='%s']/address"
#define BRIDGE_FILTERING_ENTRY_VIDS_XPATH               BRIDGE_FDB_XPATH "/filtering-entry[database-id='%s'][vids='%s'][address='%s']/vids"
#define BRIDGE_FILTERING_ENTRY_ENTRY_TYPE_XPATH         BRIDGE_FDB_XPATH "/filtering-entry[database-id='%s'][vids='%s'][address='%s']/entry-type"
#define BRIDGE_FILTERING_ENTRY_CONTROL_ELEMENT_XPATH    BRIDGE_FDB_XPATH "/filtering-entry[database-id='%s'][vids='%s'][address='%s']/port-map[port-ref='%s']/static-filtering-entries/control-element"

/* list component, container bridge-vlan */
#define BRIDGE_VLAN_XPATH                               BRIDGE_COMPONENT_XPATH "/bridge-vlan"
#define BRIDGE_MAX_VIDS_XPATH                           BRIDGE_VLAN_XPATH "/max-vids"
#define BRIDGE_MAX_VID_VAL_XPATH                        BRIDGE_VLAN_XPATH "/max-vid-value"
#define BRIDGE_OVERRIDE_DEFAULT_PVID_XPATH              BRIDGE_VLAN_XPATH "/override-default-pvid"
#define BRIDGE_MAX_MSTI_XPATH                           BRIDGE_VLAN_XPATH "/max-msti"

#define BRIDGE_COMPONENT__XPATH                         BRIDGE_VLAN_XPATH "/vid-to-fid[vid='%s']"
#define BRIDGE_VID_TO_FID_FID_XPATH                     BRIDGE_VLAN_XPATH "/vid-to-fid[vid='%s']/fid"
#define BRIDGE_VLAN_NAME_XPATH                          BRIDGE_VLAN_XPATH "/vlan[vid='%s']/name"
#define BRIDGE_VLAN_UNTAGGED_PORTS_XPATH                BRIDGE_VLAN_XPATH "/vlan[vid='%s']/untagged-ports"
#define BRIDGE_VLAN_EGRESS_PORTS_XPATH                  BRIDGE_VLAN_XPATH "/vlan[vid='%s']/egress-ports"
#define BRIDGE_VID2FID_ALLOCATION_FID_XPATH             BRIDGE_VLAN_XPATH "/vid-to-fid-allocation[vids='%s']/fid"
#define BRIDGE_FID2VID_ALLOCATION_VID_XPATH             BRIDGE_VLAN_XPATH "/fid-to-vid-allocation[fid='%s']/vid"
#define BRIDGE_VID2FID_ALLOCATION_LIST_XPATH            BRIDGE_VLAN_XPATH "/vid-to-fid-allocation/*"
#define BRIDGE_FID2VID_ALLOCATION_LIST_XPATH            BRIDGE_VLAN_XPATH "/fid-to-vid-allocation/*"
#define BRIDGE_VLAN_LIST_XPATH                          BRIDGE_VLAN_XPATH "/vlan/*"

/* list component, container bridge-mst */
#define BRIDGE_FID2MSTID_ALLOCATION_MSTID_XPATH         BRIDGE_COMPONENT_XPATH "/bridge-mst/fid-to-mstid-allocation[fids='%s']/mstid"

/* The bridge-type in the yang module and in the tsntool are currently match
 * only one value 'custumer-vlan-bridge', so in the bridge type map we will
 * have only that value for now. NOTE: Consider to modify the bridge-type and
 * type leafs and their types in the q-bridge yang module in the future.
 */
static char bridge_type_map[4][2][MAX_STR_LEN] = {
    // 4 pairs
    { BASE_THREE_STR, Q_BRIDGE_CUST_VLAN_BRIDGE },
};

static char component_type_map[4][2][MAX_STR_LEN] = {
    // 4 pairs
    { BASE_THREE_STR, "c-vlan-component" },
    { BASE_FOUR_STR, "s-vlan-component" },
    { BASE_FIVE_STR, "d-bridge-component" },
    { BASE_SIX_STR, "edge-relay-component" },
};

static char transmitted_map[2][2][MAX_STR_LEN] = {
    // 2 pairs
    { Q_BRIDGE_VLAN_UNTAGGED, BASE_UNTAGGED },
    { Q_BRIDGE_VLAN_TAGGED, BASE_TAGGED },
};

static char control_element_map[2][2][MAX_STR_LEN] = {
    // 2 pairs
    { BASE_FORWARDING_STR, Q_BRIDGE_FORWARD }, // only forward is supported
    { BASE_DISABLED_STR, Q_BRIDGE_FORWARD }, // only forward is supported - disabled is not supported
};

/* q-bridge specific error messages */
#define ERR_FILTERING_FILL_FAIL_STR         "Failed to fill filtering-entry structure at '%s'."

#endif /* BRIDGE_DEFINES_H_ */
