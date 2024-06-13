/*
 * TTTech ieee802-dot1cb-stream-identification-yang-module
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

#ifndef STREAM_IDENTIFICATION_DEFINES_H_
#define STREAM_IDENTIFICATION_DEFINES_H_

#define STREAM_ID_MODULE_NAME_STR                               "ieee802-dot1cb-stream-identification"

#define STREAM_ID_OUTFACIN_LIST_STR                             "out-fac-in"

#define STREAM_ID_STREAM_IDENT_LIST_STR                         "stream-identity"
#define STREAM_ID_INDEX_STR                                     "index"
#define STREAM_ID_HANDLE_STR                                    "handle"
#define STREAM_ID_IDENTIFICATION_TYPE_STR                       "identification-type"
#define STREAM_ID_OUT_FACING_STR                                "out-facing"
#define STREAM_ID_OUTPUT_PORT_LIST_STR                          "output-port"
#define STREAM_ID_INPUT_PORT_LIST_STR                           "input-port"
#define STREAM_ID_IDENT_STR                                     "identification"

#define STREAM_ID_NULL_ST_ID_STR                                "null-stream-identification"
#define STREAM_ID_SMAC_VLAN_ST_ID_STR                           "smac-vlan-stream-identification"
#define STREAM_ID_DMAC_VLAN_ST_ID_STR                           "dmac-vlan-stream-identification"
#define STREAM_ID_IP_ST_ID_STR                                  "ip-stream-identification"
#define STREAM_ID_DEST_MAC_STR                                  "destination-mac"
#define STREAM_ID_TAGGED_STR                                    "tagged"
#define STREAM_ID_VLAN_STR                                      "vlan"
#define STREAM_ID_SRC_MAC_STR                                   "source-mac"

/* vlan-tag-identification-type enumerations */
#define STREAM_ID_STREAM_TAGGED_TAGGED_STR                      "tagged"
#define STREAM_ID_STREAM_TAGGED_PRIORITY_STR                    "priority"
#define STREAM_ID_STREAM_TAGGED_ALL_STR                         "all"

#define ERR_UT_FACING_INPUT_PORT_LIST_GET_STR                   "Failed to get input-port leaf-list elements inside out-facing container for '%s' bridge!"
#define ERR_NULL_STREAM_SUPPORTED_STR                           "Only '%s' identification-type is supported at '%s'."
#define ERR_NO_SPACE_LEFT_STR                                   "No space left on device"
#define ERR_HASH_COLLISION_STR                                  "Hash table conflict occured"

/* identification-type */
#define STREAM_ID_NULL_STREAM_STR                               "null-stream-identification"
#define STREAM_ID_SOURCE_MAC_STR                                "smac-vlan-stream-identification"
#define STREAM_ID_ACTIVE_ID_STR                                 "dmac-vlan-stream-identification"
#define STREAM_ID_IP_IDENTIFICATION_STR                         "ip-stream-identification"

#define ERR_STREAM_ID_STREAM_IDENT_LIST_ALLOCATE_STR            "Cannot allocate memory for stream-identity list"
#define ERR_STREAM_ID_STREAM_LIST_EMPTY_STR                     "No stream list added"
#define ERR_STREAM_ID_UNKNOWN_POR_POSITION_LIST_STR             "Unknown Port-Position List"
#define ERR_STREAM_ID_UNKNOWN_SID_TYPE_STR                      "Stream identification type not supported on tsntool"
#define ERR_STREAM_ID_ENTRY_EXISTS_STR                          "Entry already exists for '%s'."
#define ERR_STREAM_ID_ENTRY_NOT_CREATED_STR                     "Not able to create entry for '%s'."
#define ERR_STREAM_ID_ENTRY_NOT_DELETED_STR                     "Not able to delete entry for '%s'."
#define ERR_STREAM_ID_POSITION_PARAMETER_WRING_STR              "Position parameter %s not set correctly."
#define ERR_STREAM_ID_MAX_SUPPORTED                             "Failed to get the number of maximum supported streams."
#define ERR_STREAM_ID_MAX_HANDLE_VALUE                          "Handle value '%u' is greater than maximum supported (%u)."
#define ERR_STREAM_ID_HANDLE_VALUE                              "Duplicate handle value (%u)."
#define ERR_STREAM_ID_IDENT_PARAMS                              "Failed to get SID identification parameters for handle %s (%s)."
#define ERR_STREAM_ID_TAGGED                                    "Only 'tagged' vlan identification type is supported"
#define ERR_STREAM_ID_TAGGED_ALL                                "The tagged value 'all' is not supported."
#define ERR_STREAM_ID_TAGGED_PRIORITY                           "The tagged value 'priority' is supported only for vlan 0 value."

/* list stream-identity defines */
#define STREAM_ID_INDEX_XPATH                                   "/ieee802-dot1cb-stream-identification:stream-identity[index='%s']"
#define STREAM_ID_HANDLE_XPATH                                  STREAM_ID_INDEX_XPATH "/handle"
#define STREAM_ID_DESTIONATION_MAC_XPATH                        STREAM_ID_INDEX_XPATH "/null-stream-identification/destination-mac"
#define STREAM_ID_SOURCE_MAC_XPATH                              STREAM_ID_INDEX_XPATH "/smac-vlan-stream-identification/source-mac"
#define STREAM_ID_NULL_STREAM_TAGGED_XPATH                      STREAM_ID_INDEX_XPATH "/null-stream-identification/tagged"
#define STREAM_ID_NULL_STREAM_VLAN_XPATH                        STREAM_ID_INDEX_XPATH "/null-stream-identification/vlan"
#define STREAM_ID_SMAC_TAGGED_XPATH                             STREAM_ID_INDEX_XPATH "/smac-vlan-stream-identification/tagged"
#define STREAM_ID_SMAC_VLAN_XPATH                               STREAM_ID_INDEX_XPATH "/smac-vlan-stream-identification/vlan"
#define STREAM_ID_OUT_FACING_INPUT_PORT_LIST_XPATH              STREAM_ID_INDEX_XPATH "/out-facing/input-port"

#endif
