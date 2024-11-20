/*
 * TTTech ieee802-dot1cb-frer-yang-module
 * Copyright(c) 2020 TTTech Computertechnik AG.
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

#ifndef FRER_DEFINES_H_
#define FRER_DEFINES_H_

#define FRER_SHOW_ALL       0xFFFF
#define _SEQGEN_INVALID     0xFFFF
#define _SEQENC_INVALID     0xFFFF
#define _SEQREC_INV_BOOL    0xFF
#define _SEQREC_ALG_UNKOWN  0x2
#define _SEQREC_INV_U16     0xFFFF

#define FRER_SHOW_ALL       0xFFFF
#define _SEQGEN_INVALID     0xFFFF
#define _SEQENC_INVALID     0xFFFF
#define _SEQREC_INV_BOOL    0xFF
#define _SEQREC_ALG_UNKOWN  0x2
#define _SEQREC_INV_U16     0xFFFF

#define FRER_MODULE_NAME_STR                                "ieee802-dot1cb-frer"
#define FRER_PREFIX_STR                                     "dot1cb-frer"

/* frer containers, tables, lists, leafs names */
#define FRER_CONTAINER_STR                                  "frer"
#define FRER_ALG_CONTAINER_STR                              "algorithm"
#define FRER_SEQ_GEN_LIST_STR                               "sequence-generation"
#define FRER_DIRECTION_STR                                  "direction-out-facing"
#define FRER_STREAM_LIST_STR                                "stream"
#define FRER_SEQ_RECOVERY_LIST_STR                          "sequence-recovery"
#define FRER_SEQ_ID_LIST_STR                                "sequence-identification"
#define FRER_ENC_STR                                        "encapsulation"
#define FRER_ENC_TYPE_R_TAG_STR                             "r-tag"
#define FRER_ENC_TYPE_HSR_SEQ_TAG_STR                       "hsr-sequence-tag"
#define FRER_ENC_TYPE_PRP_SEQ_TRAILER_STR                   "prp-sequence-trailer"
#define FRER_INDEX_STR                                      "index"
#define FRER_PORT_LIST_STR                                  "port"
#define FRER_RESET_STR                                      "reset"
#define FRER_ALG_STR                                        "algorithm"
#define FRER_ALG_VECTOR_STR                                 "vector"
#define FRER_ALG_MATCH_STR                                  "match"
#define FRER_HISTORY_LENGTH_STR                             "history-length"
#define FRER_RESET_TIMEOUT_STR                              "reset-timeout"
#define FRER_INVALID_SEQ_VAL_STR                            "invalid-sequence-value"
#define FRER_TAKE_NO_SEQ_STR                                "take-no-sequence"
#define FRER_INVALID_RECOVERY_STR                           "individual-recovery"
#define FRER_PERIOD_STR                                     "period"
#define FRER_PATHS_STR                                      "paths"
#define FRER_RESET_PERIOD_STR                               "reset-period"
#define FRER_PORT_STR                                       "port"
#define FRER_ACTIVE_STR                                     "active"
#define FRER_PATH_ID_LAN_ID_STR                             "path-id-lan-id"
#define FRER_INPUT_SPLIT_LIST_STR                           "input-split-list"
#define FRER_LATENT_ERROR_DETECION_STR                      "latent-error-detection"

#define FRER_VECTOR_ALG_STR                                 "vector"
#define FRER_MATCH_ALG_STR                                  "match"

#define ERR_DELETING_ENTRY_STR                              "Deleting %s entry inside running datastore inside routine %s failed!"

#define ERR_FRER_PORT_LIST_STR                              "Failed to get Sequence Recovery port list"
#define ERR_FRER_STREAM_LIST_STR                            "Cannot get maximum number of supported Stream Handles"
#define ERR_FRER_GET_STREAM_LIST_STR                        "Cannot get Stream list for function %d"
#define ERR_FRER_ADD_STREAM_LIST_STR                        "Error adding string for Sequence Recovery entry %s"
#define ERR_FRER_ADD_PORT_LIST_STR                          "Error adding port to Sequence Recovery entry %s"
#define ERR_FRER_STREAM_LIST_ALLOCATE_STR                   "Cannot allocate memory for SeqGen Streams list"

/* sequence-generation-list error messages */
#define ERR_FRER_SEQ_GEN_LIST_EMPTY_STR                     "No Sequence Generation functions added"
#define ERR_FRER_SEQ_GEN_LIST_ADD_STREAM_LIST_STR           "Error adding Stream Handle to Sequence Generation Function %s"
#define ERR_FRER_SEQ_GEN_LIST_ENTRY_DATA_STR                "Error deleting data from Sequence Generation Function %s"
#define ERR_FRER_SEQ_GEN_LIST_CREATE_STR                    "Error creating Sequence Generation Function entry (Invalid argument)"
#define ERR_FRER_SEQ_GEN_LIST_DIRECTION_MODIFY_STR          "Error creating Sequence Generation Function entry (Device or resource busy) %s"
#define ERR_FRER_SEQ_GENERATION_LIST_ALLOCATE_STR           "Cannot allocate memory for Sequence Generation function list"
#define ERR_FRER_SEQ_GEN_LIST_DIRECTION_STR                 "Cannot get direction for SeqGenFn %d"

/* sequence-recovery-list error messages */
#define ERR_FRER_SEQ_REC_LIST_EMPTY_STR                     "No Sequence Recovery functions added"
#define ERR_FRER_DEL_SEQ_REC_ENTRY_DATA_STR                 "Error deleting data from Sequence Recovery entry %s"
#define ERR_FRER_SEQ_RECOVERY_LIST_ALLOCATE_STR             "Cannot allocate memory for Sequence Recovery function list"
#define ERR_FRER_SEQ_REC_ENTRY_DATA_STR                     "Error setting data for Sequence Recovery Entry %s"
#define ERR_FRER_SEQ_REC_LIST_STREAM_LIST_ALLOCATE_STR      "Cannot allocate memory for Sequence Recovery Streams list"
#define ERR_FRER_SEQUENCE_RECOVERY_LIST_ADD_STR             "Error creating Sequence Recovery entry (Invalid argument) %s"
#define ERR_FRER_SEQUENCE_RECOVERY_LIST_DELETE_STR          "Error deleting data from Sequence Recovery entry (Invalid argument) %s"

/* sequence-identification-list error messages */
#define ERR_FRER_SEQ_IDENT_LIST_EMPTY_STR                   "No Sequence Identification functions added"
#define ERR_FRER_SEQ_IDENT_LIST_ENTRY_GET_STR               "Error deleting data from Sequence Identification entry"
#define ERR_FRER_ACTIVE_GET_STR                             "Error retrieving port2str active"
#define ERR_FRER_PORT2STR_GET_STR                           "Error retrieving port2str"
#define ERR_FRER_SEQ_IDENT_LIST_ENTRY_DEL_STR               "Error deleting Sequence Encode-Decode entry %s"
#define ERR_FRER_SEQ_IDENT_LIST_ACTIVE_STR                  "Error setting Active parameter for Sequence Encode-Decode entry %s"
#define ERR_FRER_SEQ_IDENT_LIST_CREATE_STR                  "Error creating Sequence Encode-Decode entry %s"
#define ERR_FRER_SEQ_IDENT_LIST_SEQ_ENC_DEC_STR             "Error getting Sequence Encode-Decode port2str"
#define ERR_FRER_SEQ_IDENT_LIST_ENTRY_DATA_STR              "Error deleting string from Sequence Encode-Decode entry %s"
#define ERR_FRER_SEQ_IDENT_LIST_ADD_STREAM_LIST_STR         "Error adding string to Sequence Encode-Decode entry %s"

/* list sequence-generation-list xpath */
#define FRER_XPATH                                          "/ieee802-dot1cb-frer:frer"
#define FRER_SEQ_GEN_LIST_LEAF_DIRECTION_XPATH              FRER_XPATH "/sequence-generation[index='%s']/direction-out-facing"
#define FRER_SEQ_IDENT_LIST_XPATH                           FRER_XPATH "/sequence-identification/*"
#define FRER_SEQ_IDENT_LIST_STREAM_LIST_XPATH               FRER_XPATH "/sequence-identification[index='%s']/stream/*"

/* list sequence-identification-list xpath */
#define FRER_SEQ_IDENT_LIST_LEAF_PORT_XPATH                 FRER_XPATH "/sequence-identification[port='%s']/port"
#define FRER_SEQ_IDENT_LIST_LEAF_ACTIVE_XPATH               FRER_XPATH "/sequence-identification[port='%s']/active"
#define FRER_SEQ_IDENT_LIST_LEAF_DIRECTION_XPATH            FRER_XPATH "/sequence-identification[port='%s']/direction-out-facing"
#define FRER_SEQ_IDENT_LIST_LEAF_ENCAP_XPATH                FRER_XPATH "/sequence-identification[port='%s']/encapsulation"
#define FRER_SEQ_IDENT_LIST_ENC_R_TAG_XPATH                 FRER_XPATH "/sequence-identification[port='%s']/encapsulation/r-tag"
#define FRER_SEQ_IDENT_LIST_ENC_HSR_SEQ_TAG_XPATH           FRER_XPATH "/sequence-identification[port='%s']/encapsulation/hsr-sequence-tag"
#define FRER_SEQ_IDENT_LIST_ENC_PRP_SEQ_TRAILER_XPATH       FRER_XPATH "/sequence-identification[port='%s']/encapsulation/prp-sequence-trailer"

/* sequence-recovery-list */
#define FRER_SEQ_REC_LIST_XPATH                             FRER_XPATH "/sequence-recovery[index='%s']"
#define FRER_SEQ_REC_LIST_PORT_LIST_XPATH                   FRER_XPATH "/sequence-recovery[index='%s']/*"
/* sequence-generation-list */
#define FRER_SEQ_GEN_LIST_XPATH                             FRER_XPATH "[index='%s']"
#define FRER_SEQ_GEN_LIST_STREAM_LIST_XPATH                 FRER_XPATH "[index='%s']/*"

#endif
