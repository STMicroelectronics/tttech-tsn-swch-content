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

#ifndef INCLUDE_BASE_Q_BRIDGE_LINUX_TOOLS_H_
#define INCLUDE_BASE_Q_BRIDGE_LINUX_TOOLS_H_

#include "base_private_defines.h"
#include "base_defines.h"

#define Q_BRIDGE_TYPE_CAPAB_NUM 11
#define Q_BRIDGE_CAPAB_NUM      3
#define Q_BRIDGE_TYPE_LEN       40
#define Q_BRIDGE_PORT_MAX_NUM   16

#define Q_BRIDGE_MODULE_NAME            "ieee802-dot1q-bridge"
#define Q_BRIDGE_PREFIX                 "dot1q"
#define Q_BRIDGE_NS                     "urn:ieee:std:802.1Q:yang:ieee802-dot1q-bridge"
#define Q_BRIDGE_RELOAD_FILE_STR        "/q_bridge_relaod_running_datastore.txt"
#define Q_BRIDGE_RELOAD_FILE_PATH_STR   APPS_DIR_PATH Q_BRIDGE_MODULE_NAME Q_BRIDGE_RELOAD_FILE_STR

/* containers, tables, lists, leafs names */
#define Q_BRIDGE_ROOT                           "bridges"
#define Q_BRIDGE_BRIDGE_ENTRY                   "bridge"
#define Q_BRIDGE_NAME_STR                       "name"
#define Q_BRIDGE_ADDRESS_STR                    "address"
#define Q_BRIDGE_TYPE_STR                       "bridge-type"
#define Q_BRIDGE_PORTS_STR                      "ports"
#define Q_BRIDGE_UPTIME_STR                     "up-time"
#define Q_BRIDGE_COMPONENT                      "component"
#define Q_BRIDGE_COMPONENTS                     "components"
#define Q_BRIDGE_BRIDGE_PORTS                   "bridge-port"
#define Q_BRIDGE_COMP_ID                        "id"
#define Q_BRIDGE_COMP_TYPE                      "type"
#define Q_BRIDGE_TC_ENABLED                     "traffic-class-enabled"
#define Q_BRIDGE_FILTERING_DATABASE             "filtering-database"
#define Q_BRIDGE_FILTERING_ENTRY                "filtering-entry"
#define Q_BRIDGE_DATABASE_ID                    "database-id"
#define Q_BRIDGE_LEAF_SIZE                      "size"
#define Q_BRIDGE_STATUS                         "status"
#define Q_BRIDGE_STATIC_ENTRIES_STR             "static-entries"
#define Q_BRIDGE_DYNAMIC_ENTRIES_STR            "dynamic-entries"
#define Q_BRIDGE_VIDS_BY_STATIC_FDB_ADDR_STR    "static-filtering-port-address"
#define Q_BRIDGE_DEFAULT_VIDS_BY_STATIC_FDB_ADDR_STR "default-static-filtering-port-address"
#define Q_BRIDGE_FDB_PORT_NAMES_STR             "fdb-port-names-by-addr-and-vid"
#define Q_BRIDGE_DEFAULT_FDB_PORT_NAMES_STR     "fdb-port-names-by-addr-and-default-vid"
#define Q_BRIDGE_FDB_EXISTS_STR                 "does_static_fdb_entry_exist"
#define Q_BRIDGE_NUM_OF_UNIQUE_FDB_ENTRIES_STR  "get_number_of_unique_static_fdb_entries"

#define Q_BRIDGE_VLAN_REG_ENTRY         "vlan-registration-entry"
#define Q_BRIDGE_PERMANENT_DATABASE     "permanent-database"
#define Q_BRIDGE_VLAN_STR               "bridge-vlan"
#define Q_BRIDGE_MST_STR                "bridge-mst"

#define Q_BRIDGE_ADD_DEV_VID_STR                "bridge_vlan_add_dev_vid"
#define Q_BRIDGE_DEL_DEV_VID_STR                "bridge_vlan_del_dev_vid"
#define Q_BRIDGE_ADD_FDB_SELF_STATIC_STR        "bridge_fdb_add_dev_filt_entry"
#define Q_BRIDGE_ADD_FDB_SELF_STATIC_VLAN_STR   "bridge_fdb_add_dev_vlan_filt_entry"
#define Q_BRIDGE_DEL_FDB_SELF_STATIC_STR        "bridge_fdb_del_dev_filt_entry"
#define Q_BRIDGE_DEL_FDB_SELF_STATIC_VLAN_STR   "bridge_fdb_del_dev_vlan_filt_entry"
#define Q_BRIDGE_STATIC_FDB_ADDRESSES_STR       "get_static_fdb_addresses"
#define Q_BRIDGE_COMP_DYNAMIC_VLAN_ADDR_STR     "get_state_comp_dyn_vlan"

#define Q_BRIDGE_MAX_VIDS_STR               "max-vids"
#define Q_BRIDGE_VID_STR                    "vid"
#define Q_BRIDGE_VIDS_STR                   "vids"
#define Q_BRIDGE_FID_STR                    "fid"
#define Q_BRIDGE_FIDS_STR                   "fids"
#define Q_BRIDGE_MSTID_FID_STR              "fid_by_mstid"
#define Q_BRIDGE_MSTID_STR                  "mstid"
#define Q_BRIDGE_FID2MSTID_STR              "fid-to-mstid"
#define Q_BRIDGE_FID2MSTID_ALLOC_STR        "fid-to-mstid-allocation"
#define Q_BRIDGE_VID2FID_STR                "vid-to-fid"
#define Q_BRIDGE_VID2FID_ALLOC_STR          "vid-to-fid-allocation"
#define Q_BRIDGE_FID2VID_ALLOC_STR          "fid-to-vid-allocation"

#define Q_BRIDGE_COMP_VLAN                              "vlan"
#define Q_BRIDGE_ENTRY_TYPE                             "entry-type"
#define Q_BRIDGE_PORT_MAP                               "port-map"
#define Q_BRIDGE_PORT_REF_STR                           "port-ref"
#define Q_BRIDGE_PORT_NAME_BY_ID_STR                    "get_port_name_by_id"
#define Q_BRIDGE_PORT_NAME_BY_VLAN_STR                  "get_port_name_by_vlan"
#define Q_BRIDGE_PORT_VLANS_BY_NAME_STR                 "get_port_vlan_by_name"
#define Q_BRIDGE_PORT_VLANS_WITHOUT_NAME_STR            "get_port_vlan_without_name"
#define Q_BRIDGE_COMPRESSED_PORT_VLANS_BY_NAME_STR      "get_compressed_port_vlan_by_name"
#define Q_BRIDGE_COMPRESSED_PORT_VLANS_WITHOUT_NAME_STR "get_compressed_port_vlan_without_name"
#define Q_BRIDGE_STATIC_FILT_ENTRY                      "static-filtering-entries"
#define Q_BRIDGE_STATIC_VLAN_REG_ENTRY_STR              "static-vlan-registration-entries"
#define Q_BRIDGE_DYNAMIC_VLAN_REG_ENTRY_STR             "dynamic-vlan-registration-entries"
#define Q_BRIDGE_MAC_ADDR_REG_ENTRY_STR                 "mac-address-registration-entries"
#define Q_BRIDGE_STATIC_VLAN_TRANSMITTED_STR            "vlan-transmitted"
#define Q_BRIDGE_REG_ADMIN_CONTROL                      "registrar-admin-control"
#define Q_BRIDGE_CONTROL_EL_STR                         "control-element"
#define Q_BRIDGE_CONNECT_ID_STR                         "connection-identifier"
#define Q_BRIDGE_COMP_DYN_ADDR_STR                      "bridge-dynamic-address"

#define Q_BRDIGE_EXT_FILTERING              "extended-filtering"
#define Q_BRDIGE_TRAFFIC_CLASSES            "traffic-classes"
#define Q_BRDIGE_STATIC_ENTRY_IND_PORTS     "static-entry-individual-port"
#define Q_BRDIGE_IVL_CAPABLE                "ivl-capable"
#define Q_BRDIGE_SVL_CAPABLE                "svl-capable"
#define Q_BRDIGE_HYBRID_CAPABLE             "hybrid-capable"
#define Q_BRDIGE_CONF_PVID_TAGGING          "configurable-pvid-tagging"
#define Q_BRDIGE_LOCAL_VLAN_CAPABLE         "local-vlan-capable"

#define Q_BIDGE_UNTAGGED_PORTS_STR      "untagged-ports"
#define Q_BIDGE_EGRESS_PORTS_STR        "egress-ports"

#define Q_BRIDGE_COMP_NAME_STR                "component-name"
#define Q_BRIDGE_PORT_TYPE_STR                "port-type"
#define Q_BRIDGE_PORT_PVID_STR                "pvid"
#define Q_BRIDGE_PORT_NUMBER_STR              "port-number"
#define Q_BRIDGE_PORT_COUNT_STR               "port-count"
#define Q_BRIDGE_DEF_PRIORITY                 "default-priority"
#define Q_BRIDGE_PRIO_REG_STR                 "priority-regeneration"
#define Q_BRIDGE_SERVICE_ACCESS_PRIO          "service-access-priority"
#define Q_BRIDGE_TRAFFIC_CLASS_STR            "traffic-class"
#define Q_BRIDGE_ACCEPTABLE_FRAME_STR         "acceptable-frame"
#define Q_BRIDGE_EXTERNAL_STR                 "external"
#define Q_BRIDGE_ENABLE_INGRESS_FILTERING_STR "enable-ingress-filtering"
#define Q_BRIDGE_CAPABILITIES_STR             "capabilities"
#define Q_BRIDGE_TAGGING_STR                  "tagging"
#define Q_BRIDGE_CONFIG_ACC_FRAME_TYPE_STR    "configurable-acceptable-frame-type"
#define Q_BRIDGE_INGRESS_FILTERING_STR        "ingress-filtering"
#define Q_BRIDGE_TYPE_CAPABILITIES_STR        "type-capabilities"
#define Q_BRIDGE_CUSTOMER_VLAN_PORT_STR       "customer-vlan-port"
#define Q_BRIDGE_PROVIDER_NETWORK_PORT_STR    "provider-network-port"
#define Q_BRIDGE_CUSTOMER_NETWORK_PORT_STR    "customer-network-port"
#define Q_BRIDGE_CUSTOMER_EDGE_PORT_STR       "customer-edge-port"
#define Q_BRIDGE_CUSTOMER_BACKBONE_PORT_STR   "customer-backbone-port"
#define Q_BRIDGE_VIRTUAL_INSTANCE_PORT_STR    "virtual-instance-port"
#define Q_BRIDGE_D_BRIDGE_PORT_STR            "d-bridge-port"
#define Q_BRIDGE_REMOTE_CUSTOMER_ACC_PORT_STR "remote-customer-access-port"
#define Q_BRIDGE_STAT_FACING_BRIDGE_PORT_STR  "station-facing-bridge-port"
#define Q_BRIDGE_UPLINK_ACCESS_PORT_STR       "uplink-access-port"
#define Q_BRIDGE_UPLINK_RELAY_PORT_STR        "uplink-relay-port"
#define Q_BRIDGE_TRAFFIC_CLASS_MAP            "traffic-class-map"
#define Q_BRIDGE_TRAFFIC_CLASS_PRIO           "priority"
#define Q_BRIDGE_AVAILALBE_TC                 "available-traffic-class"
#define Q_BRIDGE_NUM_TC                       "num-traffic-class"
#define Q_BRIDGE_STATISTICS                   "statistics"
#define Q_BRIDGE_FRAME_RX                     "frame-rx"
#define Q_BRIDGE_OCTET_RX                     "octets-rx"
#define Q_BRIDGE_FRAME_TX                     "frame-tx"
#define Q_BRIDGE_OCTET_TX                     "octets-tx"
#define Q_BRIDGE_DISCARD_INBOUND              "discard-inbound"

/* error messages */
#define Q_BRIDGE_NOT_CREATED            "The bridge is not created in the xml shema"
#define Q_BRIDGE_ADD_REM_MSG            "It's not possible to add/rem bridge!"
#define Q_BRIDGE_IF_EXIST               "Port_ref (interface) does not exist on system!"
#define Q_BRIDGE_MSTID_EXIST            "msti does not exist on system!"
#define Q_BRIDGE_VID_EXIST              "VID does not exist on system!"
#define Q_BRIDGE_CTRL_EL_MSG_ERR        "Device supports only 'forward' value for <control-element>."
#define Q_BRIDGE_ENTRY_TYPE_ERR         "Entry-type not currently supported."
#define Q_BRIDGE_PVID_MOD_ERR           "It is not possible to manipulate with PVID value with vlan-registration-entry. Current PVID value is '%s' for port '%s'."
#define Q_BRIDGE_GET_TC_ERR             "Failed to get number of traffic classes."
#define Q_BRIDGE_MSG_ERR_TO_MANY_S_F_ENTRIES "Configuration holds more new entries that is supported."
#define Q_BRIDGE_MSG_ERR_GET_FDB_E_NUM_STR "Failed to get number of static fdb entries."

/* leaf values */
#define Q_BRIDGE_C_VLAN_COMP        "c-vlan-component"
#define Q_BRIDGE_TWO_PORT_MAC_RELAY "two-port-mac-relay-bridge"
#define Q_BRIDGE_CUST_VLAN_BRIDGE   "customer-vlan-bridge"
#define Q_BRIDGE_STATIC             "static"
#define Q_BRIDGE_SELF_STATIC        "self static"
#define Q_BRIDGE_DYNAMIC            "dynamic"
#define Q_BRIDGE_NORMAL             "normal"
#define Q_BRIDGE_SELF               "self"
#define Q_BRIDGE_FILTER             "filter"
#define Q_BRIDGE_FORWARD_FILTER     "forward-filter"
#define Q_BRIDGE_FORWARD            "forward"
#define Q_BRIDGE_VLAN_EGRESS        "Egress"
#define Q_BRIDGE_VLAN_UNTAGGED      "Untagged"
#define Q_BRIDGE_VLAN_TAGGED        "Tagged"
#define Q_BRIDGE_VALUE_FID_STR      " FID"
#define Q_BRIDGE_VALUE_PVID_STR     "PVID"

/* q-bridge specific helpers */

/**
 * @brief Adding or removing device(interface, port, bridge) vlan for given
 * device name, vlans and tagged/untagged option.
 *
 * @param[in]   dev_name    Char pointer of the string which holds device name.
 * @param[in]   vids        Char pointer of the string which holds vlans.
 * @param[in]   object_name Char pointer of the string which object name which
 *                              represents remove or add option.
 * @param[in]   option      Char pointer of the string which holds
 *                              tagged/untagged option.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
int bridge_vlan_mod_dev_vid(char *dev_name, char *vids, char *object_name,
                            char *option);

/**
 * @brief Adding the device(interface, port, bridge) filtering entry for given
 * device name, address and vlan ids.
 *
 * @param[in]   dev_name    Char pointer of the string which holds device name.
 * @param[in]   dev_address Char pointer of the string which holds device address.
 * @param[in]   vids        Char pointer of the string which holds vlan ids.
 * @return EXIT_SUCCESS or EXIT_FAILURE.
 */
int bridge_fdb_add_dev_filt_entry(char *dev_name, char *dev_address, char *vids);

/**
 * @brief Removing the device(interface, port, bridge) filtering entry for
 * given device name, address and vlan ids.
 *
 * @param[in]   dev_name    Char pointer of the string which holds device name.
 * @param[in]   dev_address Char pointer of the string which holds device address.
 * @param[in]   vids        Char pointer of the string which holds vlan ids.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
int bridge_fdb_rem_dev_filt_entry(char *dev_name, char *dev_address, char *vids);

/**
 * @brief Collects names of the bridge ports.
 *
 * @param[out]  result_array    Place holder for value(s) of the object(s).
 *                              Caller should provide address of char matrix as
 *                              result_array.
 * @param[in]   row_num         Number of result_array's rows.
 * @param[in]   col_num         Number of result_array's columns.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
int get_bridge_port_names(void *result_array, unsigned int row_num, unsigned int col_num);

/**
 * @brief Collects value of q bridge port PVID.
 *
 * @param[out] result_array     Place holder for value(s) of the object(s).
 *                              Caller should provide address of char matrix as
 *                              result_array.
 * @param[in]   row_num         Number of result_array's rows.
 * @param[in]   col_num         Number of result_array's columns.
 * @param[in]   port_name       Name of bridge's port which PVID is demanded.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
int get_q_bridge_port_pvid(void *result_array, unsigned int row_num, unsigned int col_num,
                           char *port_name);

/**
 * @brief Setter function for PVID.
 *
 * @param[in]   dev_name        Char pointer of the string which holds device name.
 * @param[in]   pvid            Char pointer of the string which holds pvid value.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
int set_q_bridge_port_pvid(char *dev_name, char *pvid);

/**
 * @brief Collects names of the bridges on system.
 *
 * @param[out]  result_array    Place holder for value(s) of the object(s).
 *                              Caller should provide address of char matrix as
 *                              result_array.
 * @param[in]   row_num         Number of result_array's rows.
 * @param[in]   col_num         Number of result_array's columns.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
int get_bridge_names(void *result_array, unsigned int row_num, unsigned int col_num);

/**
 * @brief Get bridge type hardcoded value "customer-vlan-bridge".
 *
 * @param[out]  result_array    Place holder for value(s) of the object(s).
 *                              Caller should provide address of char matrix as
 *                              result_array. This functions fills only first
 *                              string from this array.
 * @param[in]   row_num         Number of result_array's rows.
 * @param[in]   col_num         Number of result_array's columns.
 * @param[in]   bridge_name     Char pointer of the string which holds bridge name.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
int get_bridge_type(void *result_array, unsigned int row_num, unsigned int col_num,
                    char *bridge_name);

/**
 * @brief Getting the system up time value.
 *
 * @param[out]  result_array    Place holder for value(s) of the object(s).
 *                              Caller should provide address of char matrix as
 *                              result_array. This functions fills only first
 *                              string from this array.
 * @param[in]   row_num         Number of result_array's rows.
 * @param[in]   col_num         Number of result_array's columns.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
int get_bridge_uptime(void *result_array, unsigned int row_num, unsigned int col_num);

/**
 * @brief Gathers all vlan ids for all ports of the bridge.
 *
 * @param[out]  bridge_vlans    String array (row_num - number of the strings,
 *                              col_num - length of the string).
 * @param[in]   row_num         Maximum number of the strings.
 * @param[in]   col_num         Length of the string.
 * @param[in]   compressed      If it is 0, the function will fill result_array
 *                              with independent vlan ids (e.g. 1,2,3).
 *                              Otherwise, the function will fill result_array
 *                              with collections of the vlan ids (e.g. 2-5, 7,
 *                              10-15).
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
int get_bridge_vlans(void *bridge_vlans, unsigned int row_num, unsigned int col_num,
                     int compressed);

/**
 * @brief Getting the value of interface index for given interface.
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
int get_interface_if_index(void *result_array, unsigned int row_num, unsigned int col_num,
                           char *interface_name);

/**
 * @brief Getting the value of interface address for given interface.
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
int get_interface_address(void *result_array, unsigned int row_num, unsigned int col_num,
                          char *interface_name);

/**
 * @brief It will set 'value' as bridge address.
 *
 * @param[in]   interface_name  Bridge name.
 * @param[in]   value           New address.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
int set_interface_address(char *interface_name, char *value);

/**
 * @brief Collects the values of MSTP FIDs for given bridge and MSTP Id.
 *
 * @param[out]  result_array    Place holder for value(s) of the object(s).
 *                              Caller should provide address of char matrix as
 *                              result_array.
 * @param[in]   row_num         Number of result_array's rows.
 * @param[in]   col_num         Number of result_array's columns.
 * @param[in]   bridge_name     Char pointer of the string which holds bridge
 *                              name.
 * @param[in]   bridge_msti     Char pointer of the string which holds bridge
 *                              MSTP Id.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
int get_bridge_fids(void *result_array, unsigned int row_num, unsigned int col_num,
                    char *bridge_name, char *bridge_msti);

/**
 * @brief Collects the values of MSTP FIDs for given bridge and MSTP Id.
 *
 * @param[out]  result_array Place holder for value(s) of the object(s). Caller
 *                              should provide address of char matrix as
 *                              result_array. This functions fills only first
 *                              string from this array.
 * @param[in]   row_num         Number of result_array's rows.
 * @param[in]   col_num         Number of result_array's columns.
 * @param[in]   bridge_name     Char pointer of the string which holds bridge
 *                              name.
 * @param[in]   bridge_msti     Char pointer of the string which holds bridge
 *                              MSTP Id.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
int get_bridge_fid_by_mstid(void *result_array, unsigned int row_num, unsigned int col_num,
                            char *bridge_name, char *bridge_msti);

/**
 * @brief Getting all vids from static fdb entries for address.
 *
 * @param[out]  result_array    Place holder for value(s) of the object(s).
 *                              Caller should provide address of char matrix as
 *                              result_array.
 * @param[in]   row_num         Number of result_array's rows.
 * @param[in]   col_num         Number of result_array's columns.
 * @param[in]   address         Char pointer of the string which holds static
 *                              fdb address.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
int get_vids_by_static_fdb_address(void *result_array, unsigned int row_num, unsigned int col_num,
                                   char *address);

/**
 * @brief Getting default vid from static fdb entries for address.
 *
 * @param[out]  result_array    Place holder for value(s) of the object(s).
 *                              Caller should provide address of char matrix as
 *                              result_array.
 * @param[in]   row_num         Number of result_array's rows.
 * @param[in]   col_num         Number of result_array's columns.
 * @param[in]   address         Char pointer of the string which holds static
 *                              fdb address.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
int get_default_vids_by_static_fdb_address(void *result_array, unsigned int row_num, unsigned int col_num,
                                           char *address);

/**
 * @brief Collects unique port addresses from fdb entries.
 *
 * @param[out]  result_array    Place holder for value(s) of the object(s).
 *                              Caller should provide address of char matrix as
 *                              result_array.
 * @param[in]   row_num         Number of result_array's rows.
 * @param[in]   col_num         Number of result_array's columns.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
int get_static_fdb_addresses(void *result_array, unsigned int row_num, unsigned int col_num);

/**
 * @brief Getting all ports name from static fdb entries for address and for vid.
 *
 * @param[out]  result_array    Place holder for value(s) of the object(s).
 *                              Caller should provide address of char matrix as
 *                              result_array.
 * @param[in]   row_num         Number of result_array's rows.
 * @param[in]   col_num         Number of result_array's columns.
 * @param[in]   address         Char pointer of the string which holds static
 *                              fdb address.
 * @param[in]   vid             Char pointer of the string which holds vid.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
int get_fdb_port_names_by_addr_and_vid(void *result_array, unsigned int row_num, unsigned int col_num,
                                       char *address, char *vid);

/**
 * @brief Checks if static filtering entry with address and vid exist in table.
 *
 * @param[in]   address         Char pointer of the string which holds static
 *                              fdb address.
 * @param[in]   vid             Char pointer of the string which holds vid.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
int does_static_fdb_entry_exist(char *address, char *vid);

/**
 * @brief Counts number of different static fdb entries.
 *
 * Classification is done by address, vlan and dev.
 *
 * @param[out]  result_array    Place holder for value(s) of the object(s).
 *                              Caller should provide address of char matrix as
 *                              result_array.
 * @param[in]   row_num         Number of result_array's rows.
 * @param[in]   col_num         Number of result_array's columns.
 * @param[in]   bridge_name     Char pointer of the string which holds bridge
 *                              name.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
int get_number_of_unique_static_fdb_entries(void *result_array, unsigned int row_num, unsigned int col_num,
                                            char *bridge_name);

/**
 * @brief Getting all ports name from static fdb entries for address and for
 * default vid (4095).
 *
 * @param[out]  result_array    Place holder for value(s) of the object(s).
 *                              Caller should provide address of char matrix as
 *                              result_array.
 * @param[in]   row_num         Number of result_array's rows.
 * @param[in]   col_num         Number of result_array's columns.
 * @param[in]   address         Char pointer of the string which holds static
 *                              fdb address.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
int get_fdb_port_names_by_addr_and_default_vid(void *result_array, unsigned int row_num, unsigned int col_num,
                                               char *address);

/**
 * @brief Getting the number of dynamic entries for given bridge.
 *
 * @param[out]  result_array    Place holder for value(s) of the object(s).
 *                              Caller should provide address of char matrix as
 *                              result_array. This functions fills only first
 *                              string from this array.
 * @param[in]   row_num         Number of result_array's rows.
 * @param[in]   col_num         Number of result_array's columns.
 * @param[in]   bridge_name     Char pointer of the string which holds bridge
 *                              name.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
int get_dynamic_entries(void *result_array, unsigned int row_num, unsigned int col_num,
                        char *bridge_name);

/**
 * @brief Getting the number of dynamic vlan entries for given bridge.
 *
 * @param[out]  result_array    Place holder for value(s) of the object(s).
 *                              Caller should provide address of char matrix as
 *                              result_array. This functions fills only first
 *                              string from this array.
 * @param[in]   row_num         Number of result_array's rows.
 * @param[in]   col_num         Number of result_array's columns.
 * @param[in]   bridge_name     Char pointer of the string which holds bridge
 *                              name.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
int get_dynamic_vlan_entries(void *result_array, unsigned int row_num, unsigned int col_num,
                             char *bridge_name);

/**
 * @brief Getting the number of mac address entries for given bridge.
 *
 * @param[out]  result_array    Place holder for value(s) of the object(s).
 *                              Caller should provide address of char matrix as
 *                              result_array. This functions fills only first
 *                              string from this array.
 * @param[in]   row_num         Number of result_array's rows.
 * @param[in]   col_num         Number of result_array's columns.
 * @param[in]   bridge_name     Char pointer of the string which holds bridge
 *                              name.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
int get_mac_address_entries(void *result_array, unsigned int row_num, unsigned int col_num,
                            char *bridge_name);

/**
 * @brief Getting the address of dynamic component for given bridge.
 *
 * @param[out]  result_array    Place holder for value(s) of the object(s).
 *                              Caller should provide address of char matrix as
 *                              result_array. This functions fills only first
 *                              string from this array.
 * @param[in]   row_num         Number of result_array's rows.
 * @param[in]   col_num         Number of result_array's columns.
 * @param[in]   bridge_name     Char pointer of the string which holds bridge
 *                              name.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
int get_state_comp_dyn_address(void *result_array, unsigned int row_num, unsigned int col_num,
                               char *bridge_name);

/**
 * @brief Collects the values of dyninamic vlan IDs for given port and port
 * address.
 *
 * @param[out]  result_array    Place holder for value(s) of the object(s).
 *                              Caller should provide address of char matrix as
 *                              result_array.
 * @param[in]   row_num         Number of result_array's rows.
 * @param[in]   col_num         Number of result_array's columns.
 * @param[in]   port_name       Char pointer of the string which holds port
 *                              name.
 * @param[in]   port_address    Char pointer of the string which holds port
 *                              address.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
int get_state_comp_dyn_vlan(void *result_array, unsigned int row_num, unsigned int col_num,
                            char *port_name, char *port_address);

/**
 * @brief Getting the port name if it is untagged for given port and vid.
 *
 * @param[out]  result_array    Place holder for value(s) of the object(s).
 *                              Caller should provide address of char matrix as
 *                              result_array. This functions fills only first
 *                              string from this array.
 * @param[in]   row_num         Number of result_array's rows.
 * @param[in]   col_num         Number of result_array's columns.
 * @param[in]   port_name       Char pointer of the string which holds port
 *                              name.
 * @param[in]   vid             Char pointer of the string which holds vlan id.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
int get_untagged_port_names(void *result_array, unsigned int row_num, unsigned int col_num,
                            char *port_name, char *vid);

/**
 * @brief Getting the transmitted value for given port. The transmitted value
 * can be "tagged" or "untagged".
 *
 * By default, the transmitted value is "tagged".
 *
 * @param[out]  result_array    Place holder for value(s) of the object(s).
 *                              Caller should provide address of char matrix as
 *                              result_array. This functions fills only first
 *                              string from this array.
 * @param[in]   row_nums        Number of result_array's rows.
 * @param[in]   col_nums        Number of result_array's columns.
 * @param[in]   port_name       Char pointer of the string which holds port name.
 * @param[in]   vid             Char pointer of the string which holds vid.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
int get_vlan_transmitted(void *result_array, unsigned int row_num, unsigned int col_num,
                         char *port_name, char *vid);

/**
 * @brief Getting the control element value for given bridge.
 *
 * @param[out]  result_array    Place holder for value(s) of the object(s).
 *                              Caller should provide address of char matrix as
 *                              result_array. This functions fills only first
 *                              string from this array.
 * @param[in]   row_num         Number of result_array's rows.
 * @param[in]   col_num         Number of result_array's columns.
 * @param[in]   bridge_name     Char pointer of the string which holds bridge
 *                              name.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
int get_control_element(void *result_array, unsigned int row_num, unsigned int col_num,
                        char *bridge_name);

/**
 * @brief Getting the connection identifier value for given interface.
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
int get_connection_identifier(void *result_array, unsigned int row_num, unsigned int col_num,
                              char *interface_name);

/**
 * @brief Getting the port name if it is egress for given port.
 *
 * @param[out]  result_array    Place holder for value(s) of the object(s).
 *                              Caller should provide address of char matrix as
 *                              result_array. This functions fills only first
 *                              string from this array.
 * @param[in]   row_num         Number of result_array's rows.
 * @param[in]   col_num         Number of result_array's columns.
 * @param[in]   port_name       Char pointer of the string which holds port name.
 * @param[in]   vid             Char pointer of the string which holds vlan id.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
int get_egress_port_names(void *result_array, unsigned int row_num, unsigned int col_num,
                          char *port_name, char *vid);

/**
 * @brief Collects the values of MSTP VIDs for given bridge.
 *
 * @param[out]  result_array    Place holder for value(s) of the object(s).
 *                              Caller should provide address of char matrix as
 *                              result_array.
 * @param[in]   row_num         Number of result_array's rows.
 * @param[in]   col_num         Number of result_array's columns.
 * @param[in]   bridge_name     Char pointer of the string which holds bridge
 *                              name.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
int get_bridge_mstp_vids(void *result_array, unsigned int row_num, unsigned int col_num,
                         char *bridge_name);

/**
 * @brief Collects the values of MSTP FIDs for given bridge.
 *
 * @param[out]  result_array    Place holder for value(s) of the object(s).
 *                              Caller should provide address of char matrix as
 *                              result_array.
 * @param[in]   row_num         Number of result_array's rows.
 * @param[in]   col_num         Number of result_array's columns.
 * @param[in]   bridge_name     Char pointer of the string which holds bridge
 *                              name.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
int get_bridge_mstp_fids(void *result_array, unsigned int row_num, unsigned int col_num,
                         char *bridge_name);

/**
 * @brief Collects the values of MSTP FID for given bridge and and VLAN id.
 *
 * @param[out]  result_array    Place holder for value(s) of the object(s).
 *                              Caller should provide address of char matrix as
 *                              result_array.
 * @param[in]   row_num         Number of result_array's rows.
 * @param[in]   col_num         Number of result_array's columns.
 * @param[in]   bridge_name     Char pointer of the string which holds bridge
 *                              name.
 * @param[in]   bridge_vids     Char pointer of the string which holds bridge
 *                              VLAN Id.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
int get_mstp_fid_by_vids(void *result_array, unsigned int row_num, unsigned int col_num,
                         char *bridge_name, char *bridge_vids);

/**
 * @brief Collects the values of MSTP VIDs for given bridge and and FID id.
 *
 * @param[out]  result_array    Place holder for value(s) of the object(s).
 *                              Caller should provide address of char matrix as
 *                              result_array.
 * @param[in]   row_num         Number of result_array's rows.
 * @param[in]   col_num         Number of result_array's columns.
 * @param[in]   bridge_name     Char pointer of the string which holds bridge
 *                              name.
 * @param[in]   bridge_fid      Char pointer of the string which holds bridge
 *                              FID Id.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
int get_mstp_vids_by_fid(void *result_array, unsigned int row_num, unsigned int col_num,
                         char *bridge_name, char *bridge_fid);

/**
 * @brief Getting the value of interface index for given interface.
 *
 * @param[out] result_array     Place holder for value(s) of the object(s).
 *                              Caller should provide address of char matrix as
 *                              result_array. This functions fills only first
 *                              string from this array.
 * @param[in]   row_num         Number of result_array's rows.
 * @param[in]   col_num         Number of result_array's columns.
 * @param[in]   interface_name  Char pointer of the string which holds
 *                              interface name.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
int get_bridge_port_ref(void *result_array, unsigned int row_num, unsigned int col_num,
                        char *interface_name);

/**
 * @brief Getting the interface name by given interface id.
 *
 * @param[out]  result_array    Place holder for value(s) of the object(s).
 *                              Caller should provide address of char matrix as
 *                              result_array.
 * @param[in]   row_num         Number of result_array's rows.
 * @param[in]   col_num         Number of result_array's columns.
 * @param[in]   port_id         Char pointer of the string which holds bridge name.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
int get_interface_name_by_id(void *result_array, unsigned int row_num, unsigned int col_num,
                             char *port_id);

/**
 * @brief Getting the ports name by given port vlan id.
 *
 * @param[out]  result_array    Place holder for value(s) of the object(s).
 *                              Caller should provide address of char matrix as
 *                              result_array.
 * @param[in]   row_num         Number of result_array's rows.
 * @param[in]   col_num         Number of result_array's columns.
 * @param[in]   interface_vlan  Char pointer of the string which holds
 *                              interface vlan id.
 * @param[in]   bridge_name     Char pointer of the string which holds bridge
 *                              name.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
int get_bridge_ports_name_by_vlan(void *result_array, unsigned int row_num, unsigned int col_num,
                                  char *interface_vlan, char *bridge_name);

/**
 * @brief Getting the port vlans by given port name in compressed format (e.g. 2-4).
 *
 * @param[out]  result_array    Place holder for value(s) of the object(s).
 *                              Caller should provide address of char matrix as
 *                              result_array.
 * @param[in]   row_num         Number of result_array's rows.
 * @param[in]   col_num         Number of result_array's columns.
 * @param[in]   compressed      If it is 0, the function will fill result_array
 *                              with independent vlan ids (e.g. 1,2,3).
 *                              Otherwise, the function will fill result_array
 *                              with collections of the vlan ids (e.g. 2-5, 7,
 *                              10-15).
 * @param[in]   interface_name  Char pointer of the string which holds
 *                              interface name.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
int get_port_vlans_by_name(void *result_array, unsigned int row_num, unsigned int col_num,
                           int compressed, char *interface_name);

#endif /* INCLUDE_BASE_Q_BRIDGE_LINUX_TOOLS_H_ */
