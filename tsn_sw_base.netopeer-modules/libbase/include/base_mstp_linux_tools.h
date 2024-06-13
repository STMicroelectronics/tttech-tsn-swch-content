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

#ifndef INCLUDE_BASE_MSTP_LINUX_TOOLS_H_
#define INCLUDE_BASE_MSTP_LINUX_TOOLS_H_

#include "base_defines.h"

#define MSTP_PREFIX                 "ieee8021-mstp"
#define MSTP_NAME                   "ieee8021-mstp"
#define MSTP_NS                     "urn:ietf:params:xml:ns:yang:smiv2:ieee8021-mstp"

/* containers, tables, lists, leafs names */
#define MSTPD_STR                   "mstpd"
#define MSTP_BRIDGE_NAMES_STR       "mstp_bridge_names"
#define MSTP_PORT_NAMES_STR         "mstp_bridge_port_names"
#define MSTP_BRIDGE_PORT_NAME_BY_ID_STR "mstp_bridge_port_name_by_id"
#define MSTP_ROOT                   "ieee8021-mstp"
#define MSTP_CIST_TABLE_STR         "ieee8021MstpCistTable"
#define MSTP_CIST_ENTRY             "ieee8021MstpCistEntry"
#define MSTP_CIST_COMP_ID           "ieee8021MstpCistComponentId"
#define MSTP_CIST_MAX_HOPS_STR      "ieee8021MstpCistMaxHops"
#define MSTP_CIST_BRIDGE_ID_STR     "ieee8021MstpCistBridgeIdentifier"
#define MSTP_CIST_TOP_CHANGE_STR    "ieee8021MstpCistTopologyChange"
#define MSTP_CIST_REG_ROOT_ID_STR   "ieee8021MstpCistRegionalRootIdentifier"
#define MSTP_CIST_PATH_COST_STR     "ieee8021MstpCistPathCost"
#define MSTP_CIST_BRIDGE_PRIO_STR   "ieee8021MstpCistBridgePriority"

#define MSTP_TABLE_STR              "ieee8021MstpTable"
#define MSTP_ENTRY_STR              "ieee8021MstpEntry"
#define MSTP_ENTRY_ADD_STR          "ieee8021MstpEntry_add"
#define MSTP_ENTRY_DEL_STR          "ieee8021MstpEntry_del"
#define MSTP_DEL_FIDS_STR           "mstpctl-del-fids"
#define MSTP_SET_FID_2_MSTID_STR    "mstpctl-setfid2mstid"
#define MSTP_SET_VID_2_FID_STR      "mstpctl-setvid2fid"
#define MSTP_COMP_ID                "ieee8021MstpComponentId"
#define MSTP_ID_STR                 "ieee8021MstpId"
#define MSTP_BRIDGE_PRIORITY_STR    "ieee8021MstpBridgePriority"
#define MSTP_ROW_STATUS             "ieee8021MstpRowStatus"
#define MSTP_BRIDGE_ID_STR          "ieee8021MstpBridgeId"
#define MSTP_SINCE_TOP_CHANGE_STR   "ieee8021MstpTimeSinceTopologyChange"
#define MSTP_TOP_CHANGES_STR        "ieee8021MstpTopologyChanges"
#define MSTP_DSGN_ROOT_STR          "ieee8021MstpDesignatedRoot"
#define MSTP_ROOT_PATH_COST_STR     "ieee8021MstpRootPathCost"
#define MSTP_ROOT_PORT_STR          "ieee8021MstpRootPort"

#define MSTP_CIST_PORT_TABLE_STR                "ieee8021MstpCistPortTable"
#define MSTP_CIST_PORT_ENTRY                "ieee8021MstpCistPortEntry"
#define MSTP_CIST_PORT_COMP_ID              "ieee8021MstpCistPortComponentId"
#define MSTP_CIST_PORT_NUM_STR              "ieee8021MstpCistPortNum"
#define MSTP_CIST_PORT_ADMIN_PATH_COST_STR  "ieee8021MstpCistPortAdminPathCost"
#define MSTP_CIST_PORT_ADMIN_EDGE_PORT_STR  "ieee8021MstpCistPortAdminEdgePort"
#define MSTP_CIST_PORT_MAC_ENABLED_STR      "ieee8021MstpCistPortMacEnabled"
#define MSTP_CIST_PORT_RESTR_ROLE_STR       "ieee8021MstpCistPortRestrictedRole"
#define MSTP_CIST_PORT_RESTR_TCN_STR        "ieee8021MstpCistPortRestrictedTcn"
#define MSTP_CIST_PORT_PRTCL_MIGRATION      "ieee8021MstpCistPortProtocolMigration"
#define MSTP_CIST_PORT_ENABLE_BPDURX        "ieee8021MstpCistPortEnableBPDURx"
#define MSTP_CIST_PORT_ENABLE_BPDUTX        "ieee8021MstpCistPortEnableBPDUTx"
#define MSTP_CIST_PORT_PSEUDO_ROOT_ID       "ieee8021MstpCistPortPseudoRootId"
#define MSTP_CIST_PORT_L2GP                 "ieee8021MstpCistPortIsL2Gp"
#define MSTP_CIST_PORT_DSGN_ROOT_STR        "ieee8021MstpCistPortDesignatedRoot"
#define MSTP_CIST_PORT_TOP_CHANGE_ACK_STR   "ieee8021MstpCistPortTopologyChangeAck"

#define MSTP_CIST_PORT_HELLO_TIME_STR       "ieee8021MstpCistPortHelloTime"
#define MSTP_CIST_PORT_OPER_EDGE_PORT_STR   "ieee8021MstpCistPortOperEdgePort"
#define MSTP_CIST_PORT_PORT_ROLE_STR        "ieee8021MstpCistPortRole"
#define MSTP_CIST_PORT_DISPUTED_STR         "ieee8021MstpCistPortDisputed"
#define MSTP_CIST_PORT_REG_ROOT_ID_STR      "ieee8021MstpCistPortCistRegionalRootId"
#define MSTP_CIST_PORT_PATH_COST_STR        "ieee8021MstpCistPortCistPathCost"

#define MSTP_PORT_TABLE_STR         "ieee8021MstpPortTable"
#define MSTP_PORT_ENTRY             "ieee8021MstpPortEntry"
#define MSTP_PORT_COMP_ID           "ieee8021MstpPortComponentId"
#define MSTP_PORT_MSTP_ID           "ieee8021MstpPortMstId"
#define MSTP_PORT_NUM_STR           "ieee8021MstpPortNum"
#define MSTP_PORT_PRIORITY_STR      "ieee8021MstpPortPriority"
#define MSTP_PORT_PATH_COST_STR     "ieee8021MstpPortPathCost"
#define MSTP_PORT_STATE_STR         "ieee8021MstpPortState"
#define MSTP_PORT_DSGN_ROOT_STR     "ieee8021MstpPortDesignatedRoot"
#define MSTP_PORT_DSGN_COST_STR     "ieee8021MstpPortDesignatedCost"
#define MSTP_PORT_DSGN_BRIDGE_STR   "ieee8021MstpPortDesignatedBridge"
#define MSTP_PORT_DSGN_PORT_STR     "ieee8021MstpPortDesignatedPort"
#define MSTP_PORT_ROLE_STR          "ieee8021MstpPortRole"
#define MSTP_PORT_DISPUTED_STR      "ieee8021MstpPortDisputed"

#define MSTP_FID2MSTI_TABLE         "ieee8021MstpFidToMstiTable"
#define MSTP_FID2MSTI_ENTRY         "ieee8021MstpFidToMstiEntry"
#define MSTP_FID2MSTI_COMP_ID       "ieee8021MstpFidToMstiComponentId"
#define MSTP_FID2MSTI_FID           "ieee8021MstpFidToMstiFid"
#define MSTP_FID2MSTI_MSTID         "ieee8021MstpFidToMstiMstId"

#define MSTP_CONF_ID_TABLE_STR      "ieee8021MstpConfigIdTable"
#define MSTP_CONF_ID_ENTRY          "ieee8021MstpConfigIdEntry"
#define MSTP_CONF_ID_COMP_ID        "ieee8021MstpConfigIdComponentId"
#define MSTP_CONF_ID_SELECTOR_STR   "ieee8021MstpConfigIdFormatSelector"
#define MSTP_CONF_ID_CONF_NAME_STR  "ieee8021MstpConfigurationName"
#define MSTP_CONF_ID_REVISION_STR   "ieee8021MstpRevisionLevel"
#define MSTP_CONF_DIGEST_STR        "ieee8021MstpConfigurationDigest"

#define MSTP_PORT_ADMIN_P2P_STR     "admin-point-to-point"

/* define mstp specific values */
#define MSTP_ROOT_STR               "Root"
#define MSTP_ALTERNATE_STR          "Alternate"
#define MSTP_DESIGNATED_STR         "Designated"
#define MSTP_BACKUP_STR             "Backup"
#define MSTP_MAX_HOPS_INTERVAL_LOWER_BOUND_STR "10"
#define MSTP_MAX_HOPS_INTERVAL_UPPER_BOUND_STR "255"

/**
 * @brief Checks is mstpd daemon active on system.
 *
 * @param[out]  result_array    Place holder for value(s) of the object(s).
 *                              Caller should provide address of char matrix as
 *                              result_array.
 * @param[in]   row_num         Number of result_array's rows.
 * @param[in]   col_num         Number of result_array's columns.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
int mstp_daemon_check_status(void *result_array, unsigned int row_num, unsigned int col_num);

/**
 * @brief Collects names of the bridges which are added in mstpd list.
 *
 * @param[out]  result_array    Place holder for value(s) of the object(s).
 *                              Caller should provide address of char matrix as
 *                              result_array.
 * @param[in]   row_num         Number of result_array's rows.
 * @param[in]   col_num         Number of result_array's columns.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
int get_mstp_bridge_names(void *result_array, unsigned int row_num, unsigned int col_num);

/**
 * @brief Getting mstp bridge id for given bridge name.
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
int get_cist_bridge_comp_id(void *result_array, unsigned int row_num, unsigned int col_num,
                            char *bridge_name);

/**
 * @brief Getting mstp topology change value for given bridge name.
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
int get_cist_bridge_topology_change(void *result_array, unsigned int row_num, unsigned int col_num,
                                    char *bridge_name);

/**
 * @brief Getting mstp regional-root value for given bridge name.
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
int get_cist_bridge_reg_root_identifier(void *result_array, unsigned int row_num, unsigned int col_num,
                                        char *bridge_name);

/**
 * @brief Getting mstp bridge path cost value for given bridge name.
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
int get_cist_bridge_path_cost(void *result_array, unsigned int row_num, unsigned int col_num,
                              char *bridge_name);

/**
 * @brief Getting mstp bridge priority value for given bridge name.
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
int get_cist_bridge_priority(void *result_array, unsigned int row_num, unsigned int col_num,
                             char *bridge_name);

/**
 * @brief Getting mstp  bridge max hops value for given bridge name.
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
int get_cist_bridge_max_hops(void *result_array, unsigned int row_num, unsigned int col_num,
                             char *bridge_name);

/**
 * @brief Collects names of the ports for given mstp bridge name.
 *
 * @param[out]  result_array    Place holder for value(s) of the object(s).
 *                              Caller should provide address of char matrix as
 *                              result_array.* @param[in] row_num Number of
 *                              result_array's rows.
 * @param[in]   col_num         Number of result_array's columns.
 * @param[in]   bridge_name     Char pointer of the string which holds bridge name.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
int get_mstp_bridge_port_names(void *result_array, unsigned int row_num, unsigned int col_num,
                               char *bridge_name);

/**
 * @brief Getting mstp port id for given bridge name and port name.
 *
 * @param[out]  result_array    Place holder for value(s) of the object(s).
 *                              Caller should provide address of char matrix as
 *                              result_array. This functions fills only first
 *                              string from this array.
 * @param[in]   row_num         Number of result_array's rows.
 * @param[in]   col_num         Number of result_array's columns.
 * @param[in]   bridge_name     Char pointer of the string which holds bridge name.
 * @param[in]   port_name       Char pointer of the string which holds port name.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
int get_mstp_port_id(void *result_array, unsigned int row_num, unsigned int col_num,
                     char *bridge_name, char *port_name);

/**
 * @brief Getting mstp port admin edge value for given bridge name and port name.
 *
 * @param[out]  result_array    Place holder for value(s) of the object(s).
 *                              Caller should provide address of char matrix as
 *                              result_array. This functions fills only first
 *                              string from this array.
 * @param[in]   row_num         Number of result_array's rows.
 * @param[in]   col_num         Number of result_array's columns.
 * @param[in]   bridge_name     Char pointer of the string which holds bridge name.
 * @param[in]   port_name       Char pointer of the string which holds port name.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
int get_cist_port_admin_edge_port(void *result_array, unsigned int row_num, unsigned int col_num,
                                  char *bridge_name, char *port_name);

/**
 * @brief Getting mstp port admin path cost value for given bridge name and
 * port name.
 *
 * @param[out]  result_array    Place holder for value(s) of the object(s).
 *                              Caller should provide address of char matrix as
 *                              result_array. This functions fills only first
 *                              string from this array.
 * @param[in]   row_num         Number of result_array's rows.
 * @param[in]   col_num         Number of result_array's columns.
 * @param[in]   bridge_name     Char pointer of the string which holds bridge name.
 * @param[in]   port_name       Char pointer of the string which holds port name.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
int get_cist_port_admin_path_cost(void *result_array, unsigned int row_num, unsigned int col_num,
                                  char *bridge_name, char *port_name);

/**
 * @brief Getting mstp port mac enabled value for given bridge name and port
 * name.
 *
 * @param[out]  result_array    Place holder for value(s) of the object(s).
 *                              Caller should provide address of char matrix as
 *                              result_array. This functions fills only first
 *                              string from this array.
 * @param[in]   row_num         Number of result_array's rows.
 * @param[in]   col_num         Number of result_array's columns.
 * @param[in]   bridge_name     Char pointer of the string which holds bridge name.
 * @param[in]   port_name       Char pointer of the string which holds port name.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
int get_cist_port_mac_enabled(void *result_array, unsigned int row_num, unsigned int col_num,
                              char *bridge_name, char *port_name);

/**
 * @brief Getting mstp port restricted role value for given bridge name and port name.
 *
 * @param[out]  result_array    Place holder for value(s) of the object(s).
 *                              Caller should provide address of char matrix as
 *                              result_array. This functions fills only first
 *                              string from this array.
 * @param[in]   row_num         Number of result_array's rows.
 * @param[in]   col_num         Number of result_array's columns.
 * @param[in]   bridge_name     Char pointer of the string which holds bridge name.
 * @param[in]   port_name       Char pointer of the string which holds port name.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
int get_cist_port_restricted_role(void *result_array, unsigned int row_num, unsigned int col_num,
                                  char *bridge_name, char *port_name);

/**
 * @brief Getting mstp port restricted tcn value for given bridge name and port
 * name.
 *
 * @param[out]  result_array    Place holder for value(s) of the object(s).
 *                              Caller should provide address of char matrix as
 *                              result_array. This functions fills only first
 *                              string from this array.
 * @param[in]   row_num         Number of result_array's rows.
 * @param[in]   col_num         Number of result_array's columns.
 * @param[in]   bridge_name     Char pointer of the string which holds bridge name.
 * @param[in]   port_name       Char pointer of the string which holds port name.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
int get_cist_port_restricted_tcn(void *result_array, unsigned int row_num, unsigned int col_num,
                                 char *bridge_name, char *port_name);

/**
 * @brief Getting mstp port designated root value for given bridge name and
 * port name.
 *
 * @param[out]  result_array    Place holder for value(s) of the object(s).
 *                              Caller should provide address of char matrix as
 *                              result_array. This functions fills only first
 *                              string from this array.
 * @param[in]   row_num         Number of result_array's rows.
 * @param[in]   col_num         Number of result_array's columns.
 * @param[in]   bridge_name     Char pointer of the string which holds bridge name.
 * @param[in]   port_name       Char pointer of the string which holds port name.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
int get_cist_port_designated_root(void *result_array, unsigned int row_num, unsigned int col_num,
                                  char *bridge_name, char *port_name);

/**
 * @brief Getting mstp port topology change acknowledgment value for given
 * bridge name and port name.
 *
 * @param[out]  result_array    Place holder for value(s) of the object(s).
 *                              Caller should provide address of char matrix as
 *                              result_array. This functions fills only first
 *                              string from this array.
 * @param[in]   row_num         Number of result_array's rows.
 * @param[in]   col_num         Number of result_array's columns.
 * @param[in]   bridge_name     Char pointer of the string which holds bridge name.
 * @param[in]   port_name       Char pointer of the string which holds port name.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
int get_cist_port_top_change_ack(void *result_array, unsigned int row_num, unsigned int col_num,
                                 char *bridge_name, char *port_name);

/**
 * @brief Getting mstp port hello time value for given bridge name and port
 * name.
 *
 * @param[out]  result_array    Place holder for value(s) of the object(s).
 *                              Caller should provide address of char matrix as
 *                              result_array. This functions fills only first
 *                              string from this array.
 * @param[in]   row_num         Number of result_array's rows.
 * @param[in]   col_num         Number of result_array's columns.
 * @param[in]   bridge_name     Char pointer of the string which holds bridge name.
 * @param[in]   port_name       Char pointer of the string which holds port name.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
int get_cist_port_hello_time(void *result_array, unsigned int row_num, unsigned int col_num,
                             char *bridge_name, char *port_name);

/**
 * @brief Getting mstp oper edge port value for given bridge name and port name.
 *
 * @param[out]  result_array    Place holder for value(s) of the object(s).
 *                              Caller should provide address of char matrix as
 *                              result_array. This functions fills only first
 *                              string from this array.
 * @param[in]   row_num         Number of result_array's rows.
 * @param[in]   col_num         Number of result_array's columns.
 * @param[in]   bridge_name     Char pointer of the string which holds bridge name.
 * @param[in]   port_name       Char pointer of the string which holds port name.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
int get_cist_port_oper_edge_port(void *result_array, unsigned int row_num, unsigned int col_num,
                                 char *bridge_name, char *port_name);

/**
 * @brief Getting mstp port role value for given bridge name and port name.
 *
 * @param[out]  result_array    Place holder for value(s) of the object(s).
 *                              Caller should provide address of char matrix as
 *                              result_array. This functions fills only first
 *                              string from this array.
 * @param[in]   row_num         Number of result_array's rows.
 * @param[in]   col_num         Number of result_array's columns.
 * @param[in]   bridge_name     Char pointer of the string which holds bridge name.
 * @param[in]   port_name       Char pointer of the string which holds port name.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
int get_cist_port_role(void *result_array, unsigned int row_num, unsigned int col_num,
                       char *bridge_name, char *port_name);

/**
 * @brief Getting mstp disputed port value for given bridge name and port name.
 *
 * @param[out]  result_array    Place holder for value(s) of the object(s).
 *                              Caller should provide address of char matrix as
 *                              result_array. This functions fills only first
 *                              string from this array.
 * @param[in]   row_num         Number of result_array's rows.
 * @param[in]   col_num         Number of result_array's columns.
 * @param[in]   bridge_name     Char pointer of the string which holds bridge name.
 * @param[in]   port_name       Char pointer of the string which holds port name.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
int get_cist_port_disputed(void *result_array, unsigned int row_num, unsigned int col_num,
                           char *bridge_name, char *port_name);

/**
 * @brief Getting mstp port regional root id value for given bridge name and
 * port name.
 *
 * @param[out]  result_array    Place holder for value(s) of the object(s).
 *                              Caller should provide address of char matrix as
 *                              result_array. This functions fills only first
 *                              string from this array.
 * @param[in]   row_num         Number of result_array's rows.
 * @param[in]   col_num         Number of result_array's columns.
 * @param[in]   bridge_name     Char pointer of the string which holds bridge name.
 * @param[in]   port_name       Char pointer of the string which holds port name.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
int get_cist_port_reg_root_id(void *result_array, unsigned int row_num, unsigned int col_num,
                              char *bridge_name, char *port_name);

/**
 * @brief Getting mstp port path cost value for given bridge name and port name.
 *
 * @param[out]  result_array    Place holder for value(s) of the object(s).
 *                              Caller should provide address of char matrix as
 *                              result_array. This functions fills only first
 *                              string from this array.
 * @param[in]   row_num         Number of result_array's rows.
 * @param[in]   col_num         Number of result_array's columns.
 * @param[in]   bridge_name     Char pointer of the string which holds bridge name.
 * @param[in]   port_name       Char pointer of the string which holds port name.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
int get_cist_port_path_cost(void *result_array, unsigned int row_num, unsigned int col_num,
                            char *bridge_name, char *port_name);

/**
 * @brief Getting mstp bridge mstpi values for given bridge name.
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
int get_bridge_mstpi(void *result_array, unsigned int row_num, unsigned int col_num,
                     char *bridge_name);

/**
 * @brief Getting mstp bridge priority value for given bridge name and mstpi.
 *
 * @param[out]  result_array    Place holder for value(s) of the object(s).
 *                              Caller should provide address of char matrix as
 *                              result_array. This functions fills only first
 *                              string from this array.
 * @param[in]   row_num         Number of result_array's rows.
 * @param[in]   col_num         Number of result_array's columns.
 * @param[in]   bridge_name     Char pointer of the string which holds bridge name.
 * @param[in]   mstpi           Char pointer of the string which holds mstpi
 *                              value (unsigned int value as string).
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
int get_mstp_bridge_priority(void *result_array, unsigned int row_num, unsigned int col_num,
                             char *bridge_name, char *mstpi);

/**
 * @brief Getting mstp bridge id value for given bridge name and mstpi.
 *
 * @param[out]  result_array    Place holder for value(s) of the object(s).
 *                              Caller should provide address of char matrix as
 *                              result_array. This functions fills only first
 *                              string from this array.
 * @param[in]   row_num         Number of result_array's rows.
 * @param[in]   col_num         Number of result_array's columns.
 * @param[in]   bridge_name     Char pointer of the string which holds bridge name.
 * @param[in]   mstpi           Char pointer of the string which holds mstpi
 *                              value (unsigned int value as string).
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
int get_mstp_bridge_id(void *result_array, unsigned int row_num, unsigned int col_num,
                       char *bridge_name, char *mstpi);

/**
 * @brief Getting mstp time since topology changed value for given bridge name
 * and mstpi.
 *
 * @param[out]  result_array    Place holder for value(s) of the object(s).
 *                              Caller should provide address of char matrix as
 *                              result_array. This functions fills only first
 *                              string from this array.
 * @param[in]   row_num         Number of result_array's rows.
 * @param[in]   col_num         Number of result_array's columns.
 * @param[in]   bridge_name     Char pointer of the string which holds bridge name.
 * @param[in]   mstpi           Char pointer of the string which holds mstpi
 *                              value (unsigned int value as string).
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
int get_mstp_time_since_top_change(void *result_array, unsigned int row_num, unsigned int col_num,
                                   char *bridge_name, char *mstpi);

/**
 * @brief Getting mstp number of topology changes for given bridge name and
 * mstpi.
 *
 * @param[out]  result_array    Place holder for value(s) of the object(s).
 *                              Caller should provide address of char matrix as
 *                              result_array. This functions fills only first
 *                              string from this array.
 * @param[in]   row_num         Number of result_array's rows.
 * @param[in]   col_num         Number of result_array's columns.
 * @param[in]   bridge_name     Char pointer of the string which holds bridge name.
 * @param[in]   mstpi           Char pointer of the string which holds mstpi
 *                              value (unsigned int value as string).
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
int get_mstp_top_changes(void *result_array, unsigned int row_num, unsigned int col_num,
                         char *bridge_name, char *mstpi);

/**
 * @brief Getting mstp designated root value for given bridge name.
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
int get_mstp_designated_root(void *result_array, unsigned int row_num, unsigned int col_num,
                             char *bridge_name);

/**
 * @brief Getting mstp root path cost value for given bridge name and mstpi.
 *
 * @param[out]  result_array    Place holder for value(s) of the object(s).
 *                              Caller should provide address of char matrix as
 *                              result_array. This functions fills only first
 *                              string from this array.
 * @param[in]   row_num         Number of result_array's rows.
 * @param[in]   col_num         Number of result_array's columns.
 * @param[in]   bridge_name     Char pointer of the string which holds bridge name.
 * @param[in]   mstpi           Char pointer of the string which holds mstpi
 *                              value (unsigned int value as string).
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
int get_mstp_root_path_cost(void *result_array, unsigned int row_num, unsigned int col_num,
                            char *bridge_name, char *mstpi);

/**
 * @brief Getting mstp root port value for given bridge name and mstpi.
 *
 * @param[out]  result_array    Place holder for value(s) of the object(s).
 *                              Caller should provide address of char matrix as
 *                              result_array. This functions fills only first
 *                              string from this array.
 * @param[in]    row_num        Number of result_array's rows.
 * @param[in]    col_num        Number of result_array's columns.
 * @param[in]    bridge_name    Char pointer of the string which holds bridge name.
 * @param[in]    mstpi          Char pointer of the string which holds mstpi
 *                              value (unsigned int value as string).
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
int get_mstp_root_port(void *result_array, unsigned int row_num, unsigned int col_num,
                       char *bridge_name, char *mstpi);

/**
 * @brief Getting mstp port priority value for given bridge name, port name and mstpi.
 *
 * @param[out]  result_array    Place holder for value(s) of the object(s).
 *                              Caller should provide address of char matrix as
 *                              result_array. This functions fills only first
 *                              string from this array.
 * @param[in]    row_num        Number of result_array's rows.
 * @param[in]    col_num        Number of result_array's columns.
 * @param[in]    bridge_name    Char pointer of the string which holds bridge name.
 * @param[in]    port_name      Char pointer of the string which holds port name.
 * @param[in]    mstpi          Char pointer of the string which holds mstpi
 *                              value (unsigned int value as string).
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
int get_mstp_port_prioriy(void *result_array, unsigned int row_num, unsigned int col_num,
                          char *bridge_name, char *port_name, char *mstpi);

/**
 * @brief Getting mstp port path cost value for given bridge name, port name and mstpi.
 *
 * @param[out]  result_array    Place holder for value(s) of the object(s).
 *                              Caller should provide address of char matrix as
 *                              result_array. This functions fills only first
 *                              string from this array.
 * @param[in]   row_num         Number of result_array's rows.
 * @param[in]   col_num         Number of result_array's columns.
 * @param[in]   bridge_name     Char pointer of the string which holds bridge name.
 * @param[in]   port_name       Char pointer of the string which holds port name.
 * @param[in]   mstpi           Char pointer of the string which holds mstpi
 *                              value (unsigned int value as string).
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
int get_mstp_port_path_cost(void *result_array, unsigned int row_num, unsigned int col_num,
                            char *bridge_name, char *port_name, char *mstpi);

/**
 * @brief Getting mstp port state value for given bridge name, port name and mstpi.
 *
 * @param[out]  result_array    Place holder for value(s) of the object(s).
 *                              Caller should provide address of char matrix as
 *                              result_array. This functions fills only first
 *                              string from this array.
 * @param[in]   row_num         Number of result_array's rows.
 * @param[in]   col_num         Number of result_array's columns.
 * @param[in]   bridge_name     Char pointer of the string which holds bridge name.
 * @param[in]   port_name       Char pointer of the string which holds port name.
 * @param[in]   mstpi           Char pointer of the string which holds mstpi
 *                              value (unsigned int value as string).
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
int get_mstp_port_state(void *result_array, unsigned int row_num, unsigned int col_num,
                        char *bridge_name, char *port_name, char *mstpi);

/**
 * @brief Getting mstp port designated root value for given bridge name, port
 * name and mstpi.
 *
 * @param[out]  result_array    Place holder for value(s) of the object(s).
 *                              Caller should provide address of char matrix as
 *                              result_array. This functions fills only first
 *                              string from this array.
 * @param[in]   row_num         Number of result_array's rows.
 * @param[in]   col_num         Number of result_array's columns.
 * @param[in]   bridge_name     Char pointer of the string which holds bridge name.
 * @param[in]   port_name       Char pointer of the string which holds port name.
 * @param[in]   mstpi           Char pointer of the string which holds mstpi
 *                              value (unsigned int value as string).
 * @return  EXIT_SUCCESS or EXIT_FAILURE.
 */
int get_mstp_port_dsgn_root(void *result_array, unsigned int row_num, unsigned int col_num,
                            char *bridge_name, char *port_name, char *mstpi);

/**
 * @brief Getting mstp designated path cost value for given bridge name, port
 * name and mstpi.
 *
 * @param[out]  result_array    Place holder for value(s) of the object(s).
 *                              Caller should provide address of char matrix as
 *                              result_array. This functions fills only first
 *                              string from this array.
 * @param[in]   row_num         Number of result_array's rows.
 * @param[in]   col_num         Number of result_array's columns.
 * @param[in]   bridge_name     Char pointer of the string which holds bridge name.
 * @param[in]   port_name       Char pointer of the string which holds port name.
 * @param[in]   mstpi           Char pointer of the string which holds mstpi
 *                              value (unsigned int value as string).
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
int get_mstp_port_dsgn_cost(void *result_array, unsigned int row_num, unsigned int col_num,
                            char *bridge_name, char *port_name, char *mstpi);

/**
 * @brief Getting mstp designated bridge value for given bridge name, port name
 * and mstpi.
 *
 * @param[out]  result_array    Place holder for value(s) of the object(s).
 *                              Caller should provide address of char matrix as
 *                              result_array. This functions fills only first
 *                              string from this array.
 * @param[in]   row_num         Number of result_array's rows.
 * @param[in]   col_num         Number of result_array's columns.
 * @param[in]   bridge_name     Char pointer of the string which holds bridge name.
 * @param[in]   port_name       Char pointer of the string which holds port name.
 * @param[in]   mstpi           Char pointer of the string which holds mstpi
 *                              value (unsigned int value as string).
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
int get_mstp_port_dsgn_bridge(void *result_array, unsigned int row_num, unsigned int col_num,
                              char *bridge_name, char *port_name, char *mstpi);

/**
 * @brief Getting mstp designated port value for given bridge name, port name
 * and mstpi.
 *
 * @param[out]  result_array    Place holder for value(s) of the object(s).
 *                              Caller should provide address of char matrix as
 *                              result_array. This functions fills only first
 *                              string from this array.
 * @param[in]   row_num         Number of result_array's rows.
 * @param[in]   col_num         Number of result_array's columns.
 * @param[in]   bridge_name     Char pointer of the string which holds bridge name.
 * @param[in]   port_name       Char pointer of the string which holds port name.
 * @param[in]   mstpi           Char pointer of the string which holds mstpi
 *                              value (unsigned int value as string).
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
int get_mstp_port_dsgn_port(void *result_array, unsigned int row_num, unsigned int col_num,
                            char *bridge_name, char *port_name, char *mstpi);

/**
 * @brief Getting mstp designated port role value for given bridge name, port
 * name and mstpi.
 *
 * @param[out]  result_array    Place holder for value(s) of the object(s).
 *                              Caller should provide address of char matrix as
 *                              result_array. This functions fills only first
 *                              string from this array.
 * @param[in]   row_num         Number of result_array's rows.
 * @param[in]   col_num         Number of result_array's columns.
 * @param[in]   bridge_name     Char pointer of the string which holds bridge name.
 * @param[in]   port_name       Char pointer of the string which holds port name.
 * @param[in]   mstpi           Char pointer of the string which holds mstpi
 *                              value (unsigned int value as string).
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
int get_mstp_port_role(void *result_array, unsigned int row_num, unsigned int col_num,
                       char *bridge_name, char *port_name, char *mstpi);

/**
 * @brief Getting mstp disputed port value for given bridge name, port name and
 * mstpi.
 *
 * @param[out]  result_array    Place holder for value(s) of the object(s).
 *                              Caller should provide address of char matrix as
 *                              result_array. This functions fills only first
 *                              string from this array.
 * @param[in]   row_num         Number of result_array's rows.
 * @param[in]   col_num         Number of result_array's columns.
 * @param[in]   bridge_name     Char pointer of the string which holds bridge name.
 * @param[in]   port_name       Char pointer of the string which holds port name.
 * @param[in]   mstpi           Char pointer of the string which holds mstpi
 *                              value (unsigned int value as string).
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
int get_mstp_port_disputed(void *result_array, unsigned int row_num, unsigned int col_num,
                           char *bridge_name, char *port_name, char *mstpi);

/**
 * @brief Getting mstp configuration id format selector value for given bridge
 * name.
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
int get_mstp_config_id_format_selector(void *result_array, unsigned int row_num, unsigned int col_num,
                                       char *bridge_name);

/**
 * @brief Getting mstp configuration name for given bridge name.
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
int get_mstp_configuration_name(void *result_array, unsigned int row_num, unsigned int col_num,
                                char *bridge_name);

/**
 * @brief Getting mstp revision level value for given bridge name.
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
int get_mstp_revision_level(void *result_array, unsigned int row_num, unsigned int col_num,
                            char *bridge_name);

/**
 * @brief Getting mstp configuration digest value for given bridge name.
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
int get_mstp_configuration_digest(void *result_array, unsigned int row_num, unsigned int col_num,
                                  char *bridge_name);

/* setter functions */

/**
 * @brief Unregister ieee8021-mstp module from netopeer-server.
 *
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
int remove_mstp_module();

/**
 * @brief Setting mstp bridge max hops value for given mstp bridge id.
 *
 * @param[in]   bridge_id       Mstp bridge id.
 * @param[in]   value           New value of bridge max hops object.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
int set_mstp_cist_max_hops(char *bridge_id, char *value);

/**
 * @brief Setting mstp bridge priority value for given mstp bridge_id and mstpi.
 *
 * @param[in]   bridge_id       Mstp bridge id.
 * @param[in]   mstpi           Value of mstpi for bridge_id.
 * @param[in]   value           New value of bridge priority object.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
int set_mstp_bridge_priority(char *bridge_id,  char *mstpi, char *value);

/**
 * @brief Setting mstp port admin path cost value for given mstp bridge_id and
 * port_id.
 *
 * @param[in]   bridge_id       Mstp bridge id.
 * @param[in]   port_id         Char pointer of the string which holds port id.
 * @param[in]   value           New value of port's admin path cost object.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
int set_mstp_cist_port_admin_path_cost(char *bridge_id, char *port_id, char *value);

/**
 * @brief Setting mstp port admin edge value for given mstp bridge_id and
 * port_id.
 *
 * @param[in]   bridge_id       Mstp bridge id.
 * @param[in]   port_id         Char pointer of the string which holds port id.
 * @param[in]   value           New value of port's admin edge object.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
int set_mstp_cist_port_admin_edge_port(char *bridge_id, char *port_id, char *value);

/**
 * @brief Setting mstp port mac enabled value for given mstp bridge_id and
 * port_id.
 *
 * @param[in]   bridge_id       Mstp bridge id.
 * @param[in]   port_id         Char pointer of the string which holds port id.
 * @param[in]   value           New value of port's mac enabled object.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
int set_mstp_cist_port_mac_enabled(char *bridge_id, char *port_id, char *value);

/**
 * @brief Setting mstp port restricted role value for given mstp bridge_id and
 * port_id.
 *
 * @param[in]   bridge_id       Mstp bridge id.
 * @param[in]   port_id         Char pointer of the string which holds port id.
 * @param[in]   value           New value of port's restricted rola object.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
int set_mstp_cist_port_restricted_role(char *bridge_id, char *port_id, char *value);

/**
 * @brief Setting mstp port restricted tcn value for given mstp bridge_id and
 * port_id.
 *
 * @param[in]   bridge_id       Mstp bridge id.
 * @param[in]   port_id         Char pointer of the string which holds port id.
 * @param[in]   value           New value of port's restricted tcn object.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
int set_mstp_cist_port_restricted_tcn(char *bridge_id, char *port_id, char *value);

/**
 * @brief Setting mstp port priority for given mstp bridge_id, port_id and mstpi.
 *
 * @param[in]   bridge_id       Mstp bridge id.
 * @param[in]   port_id         Char pointer of the string which holds port id.
 * @param[in]   mstpi           Char pointer of the string which holds mstpi.
 * @param[in]   value           New value of port's priority object.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
int set_mstp_port_priority(char *bridge_id, char *port_id, char *mstpi, char *value);

/**
 * @brief Setting mstp port path cost value for given mstp bridge_id, port_id
 * and mstpi.
 *
 * @param[in]   bridge_id       Mstp bridge id.
 * @param[in]   port_id         Char pointer of the string which holds port id.
 * @param[in]   mstpi           Char pointer of the string which holds mstpi.
 * @param[in]   value           New value of port's path cost value object.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
int set_mstp_port_path_cost(char *bridge_id, char *port_id, char *mstpi, char *value);

/**
 * @brief Setting mstp configuration name value for given mstp bridge id.
 *
 * @param[in]   bridge_id       Mstp bridge id.
 * @param[in]   value           New value of configuration name object.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
int set_mstp_conf_name(char *bridge_id, char *value);

/**
 * @brief Setting mstp revision level value for given mstp bridge id.
 *
 * @param[in]   bridge_id       Mstp bridge id.
 * @param[in]   value           New value of revision level object.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
int set_mstp_revision_level(char *bridge_id, char *value);

/**
 * @brief Removes mstp tree for given mstp bridge id and msti.
 *
 * @param[in]   bridge_id       Mstp bridge id.
 * @param[in]   msti            Char pointer of the string which holds msti.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
int remove_mstp_entry(char *bridge_id, char *msti);

/**
 * @brief Adds mstp tree for given mstp bridge id and msti.
 *
 * @param[in]   bridge_id       Mstp bridge id.
 * @param[in]   msti            Char pointer of the string which holds msti.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
int add_mstp_entry(char *bridge_id, char *msti);

/**
 * @brief Activates mstp on bridge.
 *
 * @param[in]   bridge_id       Holds bridge id from list of all bridges.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
int add_bridge_to_mstp(char *bridge_id);

/* helpers - find by index */

/**
 * @brief Finds mstp bridge name for given mstp bridge_id.
 *
 * @param[in]   bridge_id       Mstp bridge id.
 * @param[out]  result          Holds name of the bridge which corresponds to
 *                              bridge id (char[MAX_STR_LEN]).
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
int find_bridge_name_by_id(char *bridge_id, char *result);

/**
 * @brief Finds mstp port name for given mstp bridge_name and port_id.
 *
 * @param[in]   bridge_name     Mstp bridge_name.
 * @param[in]   port_id         Char pointer of the string which holds port id.
 * @param[out]  result          Holds name of the port which corresponds to
 *                              bridge_name and port_id (char[MAX_STR_LEN]).
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
int find_port_name_by_id(char *bridge_name, char *port_id, char *result);

/**
 * @brief Getting mstp port admin point to point value for given bridge name
 * and port name.
 *
 * @param[out]  result_array    Place holder for value(s) of the object(s).
 *                              Caller should provide address of char matrix as
 *                              result_array. This functions fills only first
 *                              string from this array.
 * @param[in]   row_num         Number of result_array's rows.
 * @param[in]   col_num         Number of result_array's columns.
 * @param[in]   bridge_name     Char pointer of the string which holds bridge
 *                              name.
 * @param[in]   port_name       Char pointer of the string which holds port name.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
int get_cist_port_admin_point_to_point(void *result_array, unsigned int row_num, unsigned int col_num,
                                       char *bridge_name, char *port_name);

/**
 * @brief Setting mstp port admin point to point value for given mstp bridge
 * name and port name.
 *
 * @param[in]   bridge_name     Mstp bridge name.
 * @param[in]   port_name       Char pointer of the string which holds port name.
 * @param[in]   value           New value of port's mac enabled object.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
int set_cist_port_admin_point_to_point(char *bridge_name, char *port_name, char *value);

/**
 * @brief Deletes specified fids.
 *
 * @param[in]   bridge_name     Mstp bridge name.
 * @param[in]   value           The fids which will be deleted.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
int mstpctl_delete_fids(char *bridge_name, char *value);

/**
 * @brief Adding specified fids.
 *
 * @param[in]   bridge_name     Mstp bridge name.
 * @param[in]   mstid           The mstid.
 * @param[in]   fids            The fids.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
int mstpctl_setfid2mstid(char *bridge_name, char *mstid, char *fids);

/**
 * @brief Adding specified fid to vid.
 *
 * @param[in]   bridge_name     Mstp bridge name.
 * @param[in]   fid             The fid.
 * @param[in]   vid             The vid.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
int mstpctl_setvid2fid(char *bridge_name, char *fid, char *vid);

#endif /* INCLUDE_BASE_MSTP_LINUX_TOOLS_H_ */
