/*
 * TTTech ieee802-dot1q-fqtss-yang-module
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

#ifndef FQTSS_DEFINES_H_
#define FQTSS_DEFINES_H_

#define FQTSS_MODULE_NAME                           "ieee802-dot1q-fqtss"

/* module information specific defines */
#define FQTSS_PREFIX_STR                            "fqtss"
#define FQTSS_NAME_STR                              "ieee802-dot1q-fqtss"
#define FQTSS_NS_STR                                "urn:ietf:params:xml:ns:yang:smiv2:ieee802-dot1q-fqtss"

/* path to fqtss startup datastore */
#define FQTSS_STARTUP_DS_PATH                       "/etc/sysrepo/data/ieee802-dot1q-fqtss.startup"

/* containers, tables, lists, leafs names */
#define FQTSS_BAP_TABLE_STR                         "ieee8021FqtssBapTable"
#define FQTSS_BAP_ENTRY_STR                         "ieee8021FqtssBapEntry"
#define FQTSS_BRIDGE_ID_STR                         "ieee8021BridgeBaseComponentId"
#define FQTSS_PORT_NAME_STR                         "ieee8021BridgeBasePort"
#define FQTSS_BAP_TRAFFIC_CLASS_STR                 "ieee8021FqtssBAPTrafficClass"
#define FQTSS_ADMIN_SLOPE_MS_STR                    "ieee8021FqtssAdminIdleSlopeMs"
#define FQTSS_ADMIN_SLOPE_LS_STR                    "ieee8021FqtssAdminIdleSlopeLs"
#define FQTSS_OPER_SLOPE_MS_STR                     "ieee8021FqtssOperIdleSlopeMs"
#define FQTSS_OPER_SLOPE_LS_STR                     "ieee8021FqtssOperIdleSlopeLs"
#define FQTSS_TX_SELECTION_ALGORITHM_TABLE_STR      "ieee8021FqtssTxSelectionAlgorithmTable"
#define FQTSS_TX_SELECTION_ALGORITHM_ENTRY_STR      "ieee8021FqtssTxSelectionAlgorithmEntry"
#define FQTSS_TX_SELECTION_ALGORITHM_ID_STR         "ieee8021FqtssTxSelectionAlgorithmID"
#define FQTSS_TRAFFIC_CLASS_STR                     "ieee8021FqtssTrafficClass"

#define BRIDGE_NAMES_FAILED_STR                     "Failed to get bridge names."
#define BRIDGE_PORT_NAMES_FAILED_STR                "Failed to get bridge port names."

/* fqtss specific error messages */
#define FQTSS_BAP_ENTRY_FAILED_STR                  "Failed to create BAP table entry."
#define FQTSS_ALG_ENTRY_FAILED_STR                  "Failed to create Tx Selection Algorithm table entry."

#define FQTSS_BAP_TABLE_ENTRY_XPATH                 "/ieee802-dot1q-fqtss:ieee802-dot1q-fqtss/ieee8021FqtssBapTable/ieee8021FqtssBapEntry[ieee8021BridgeBaseComponentId='%s'][ieee8021BridgeBasePort='%s'][ieee8021FqtssBAPTrafficClass='%d']/%s"

#endif
