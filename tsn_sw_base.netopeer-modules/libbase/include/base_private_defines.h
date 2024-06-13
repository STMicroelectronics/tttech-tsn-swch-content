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

#ifndef INCLUDE_BASE_PRIVATE_DEFINES_H_
#define INCLUDE_BASE_PRIVATE_DEFINES_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "base_defines.h"

// Strings used in callbacks
#define BASE_GENERIC_IF_NAME       "sit0@NONE"
#define COLON_AND_SPACE_STR        ": "
#define COLON_STR                  ":"
#define LESS_THAN_STR              "<"
#define PORT_ID_STR                "port id"
#define POINT_STR                  "."
#define COMMA_STR                  ","

// mstp/spanning
#define X0F_MASK_STR                    "0x0F"
#define CIST_INFO_STR                   "CIST info"
#define FORMAT_SELECTOR_STR             "Format Selector:"
#define MAX_HOPS_STR                    "max hops"
#define BRIDGE_ID_STR                   "bridge id"
#define TIME_SINCE_TOPOLGY_CHANGED_STR  "time since topology change"
#define TOPOLOGY_CHANGE_COUNT_STR       "topology change count"
#define INTERNAL_PATH_COST_STR          "internal path cost"
#define ROOT_PORT_STR                   "root port"
#define STATE_STR                       "state"
#define DSGN_REGIONAL_ROOT_STR          "dsgn regional root"
#define DSGN_INTERNAL_ROOT_STR          "dsgn internal cost"
#define DSGN_BRIDGE_STR                 "designated bridge"
#define DSGN_PORT_STR                   "designated port"
#define ROLE_STR                        "role"
#define INTERNAL_PORT_COST_STR          "internal port cost"
#define DISPUTED_STR                    "disputed"
#define CONFIGURATION_NAME_STR          "Configuration Name:"
#define REVISION_LEVEL_STR              "Revision Level:"
#define CONFIG_DIGEST_STR               "Configuration Digest:"
#define GREP_STR                        "grep"
#define PRIORITY_STR                    "priority"
#define UPTIME_ELAPSED                  "ELAPSED"

/* Values which will decide is system call runtime configurable. */
typedef enum {
    BASE_RC_NO = 0,
    BASE_RC_YES,
} base_rc_t;

/* Indices which are used for indexing during call of parser function. */
typedef enum {
    BASE_SUCCEEDED            = 0,
    BASE_FAILED               = 1,
    BASE_VALIDATION_FAILED    = 2,
    BASE_VALIDATION_SUCCEEDED = 4,
    BASE_PARSING_FAILED       = 8,
    BASE_PARSING_SUCCEEDED    = 16,
} base_status_t;

/* Indices which are used for indexing during call of parser function. */
/* ietf-interfaces */
typedef enum {
    IF_NAMES = 0,
    IF_ENABLED,
    ETH_IF_ENABLE,
    ETH_DUPLEX,
    ETH_SPEED,
    ETH_MAX_FRAME_LEN,
    ETH_ADVERTISE,
    IF_TX_STATISTICS,
    IF_RX_STATISTICS,
    ETH_SUPPORTED_IF_MODE,
    ETH_LINK_DETECTED,
    ETH_STORE_ADVERTISED_MODES,
    ETH_SUPPORTED_MODES,
    ETH_STARTUP_ADVERTISED_MODES,
    Q_BRIDGE_PRIO_REG,
    Q_BRIDGE_TRAFFIC_CLASS,
    IF_MAX_OBJ_NUMBER,
} if_obj_index_t;

/* mstp */
typedef enum {
    MSTPD = 0,
    MSTP_MODULE,
    MSTP_BRIDGE_NAMES,
    // MSTP_BRIDGE_ID,// not active
    MSTP_PORT_NAMES,
    MSTP_BRIDGE_PORT_NAME_BY_ID, // helper
    MSTP_PORT_NUM,
    MSTP_CIST_TABLE,
    MSTP_CIST_BRIDGE_ID,
    MSTP_CIST_TOP_CHANGE,
    MSTP_CIST_REG_ROOT_ID,
    MSTP_CIST_PATH_COST,
    MSTP_CIST_BRIDGE_PRIO,
    MSTP_ENTRY_DEL,
    MSTP_ENTRY_ADD,
    MSTP_DEL_FIDS,
    MSTP_SET_FID_2_MSTID,
    MSTP_SET_VID_2_FID,
    MSTP_CIST_MAX_HOPS,
    MSTP_ID,
    MSTP_BRIDGE_ID,
    MSTP_SINCE_TOP_CHANGE,
    MSTP_TOP_CHANGES,
    MSTP_DSGN_ROOT,
    MSTP_ROOT_PATH_COST,
    MSTP_ROOT_PORT,
    MSTP_TABLE,
    MSTP_BRIDGE_PRIORITY,
    MSTP_CIST_PORT_TABLE,
    MSTP_CIST_PORT_DSGN_ROOT,
    MSTP_CIST_PORT_TOP_CHANGE_ACK,
    MSTP_CIST_PORT_HELLO_TIME,
    MSTP_CIST_PORT_OPER_EDGE_PORT,
    MSTP_CIST_PORT_PORT_ROLE,
    MSTP_CIST_PORT_DISPUTED,
    MSTP_CIST_PORT_REG_ROOT_ID,
    MSTP_CIST_PORT_PATH_COST,
    MSTP_CIST_PORT_ADMIN_PATH_COST,
    MSTP_CIST_PORT_ADMIN_EDGE_PORT,
    MSTP_CIST_PORT_MAC_ENABLED,
    MSTP_CIST_PORT_RESTR_ROLE,
    MSTP_CIST_PORT_RESTR_TCN,
    MSTP_PORT_TABLE,
    MSTP_PORT_STATE,
    MSTP_PORT_DSGN_ROOT,
    MSTP_PORT_DSGN_COST,
    MSTP_PORT_DSGN_BRIDGE,
    MSTP_PORT_DSGN_PORT,
    MSTP_PORT_ROLE,
    MSTP_PORT_DISPUTED,
    MSTP_PORT_PRIORITY,
    MSTP_PORT_PATH_COST,
    MSTP_CONF_ID_TABLE,
    MSTP_CONF_ID_SELECTOR,
    MSTP_CONF_ID_CONF_NAME,
    MSTP_CONF_ID_REVISION,
    MSTP_CONF_DIGEST,
    MSTP_PORT_ADMIN_P2P,
    MSTP_MAX_OBJ_NUMBER,
} mstp_obj_index_t;

/* q-bridge */
typedef enum {
    Q_BRIDGE_PORT_NAMES = 0,
    Q_BRIDGE_NAMES,
    Q_BRIDGE_ADDRESS,
    Q_BRIDGE_UPTIME,
    Q_BRIDGE_PORT_NUMBER,
    Q_BRIDGE_PORT_ADDRESS,
    Q_BRIDGE_PORT_COUNT,
    Q_BRIDGE_VIDS_BY_STATIC_FDB_ADDR,
    Q_BRIDGE_DEFAULT_VIDS_BY_STATIC_FDB_ADDR,
    Q_BRIDGE_STATIC_ENTRIES,
    Q_BRIDGE_DYNAMIC_ENTRIES,
    Q_BRIDGE_STATIC_VLAN_REG_ENTRY,
    Q_BRIDGE_DYNAMIC_VLAN_REG_ENTRY,
    Q_BRIDGE_MAC_ADDR_REG_ENTRY,
    Q_BRIDGE_COMP_DYN_ADDR,
    Q_BRIDGE_CONTROL_EL,
    Q_BRIDGE_CONNECT_ID,
    Q_BRIDGE_ADD_DEV_VID,
    Q_BRIDGE_DEL_DEV_VID,
    Q_BRIDGE_ADD_FDB_SELF_STATIC,
    Q_BRIDGE_ADD_FDB_SELF_STATIC_VLAN,
    Q_BRIDGE_DEL_FDB_SELF_STATIC,
    Q_BRIDGE_DEL_FDB_SELF_STATIC_VLAN,
    Q_BRIDGE_FIDS,
    Q_BRIDGE_MSTID_FID,
    Q_BRIDGE_GET_BRIDGE_VLAN,
    Q_BIDGE_UNTAGGED_PORTS,
    Q_BIDGE_EGRESS_PORTS,
    Q_BRIDGE_STATIC_VLAN_TRANSMITTED,
    Q_BRIDGE_STATIC_FDB_ADDRESSES,
    Q_BRIDGE_FDB_PORT_NAMES,
    Q_BRIDGE_DEFAULT_FDB_PORT_NAMES,
    Q_BRIDGE_FDB_EXISTS,
    Q_BRIDGE_NUM_OF_UNIQUE_FDB_ENTRIES,
    Q_BRIDGE_COMP_DYNAMIC_VLAN_ADDR,
    Q_BRIDGE_MST,
    Q_BRIDGE_FID2VID_ALLOC,
    Q_BRIDGE_VID,
    Q_BRIDGE_FID,
    Q_BRIDGE_PORT_PVID,
    Q_BRIDGE_PORT_REF,
    Q_BRIDGE_PORT_NAME_BY_ID,
    Q_BRIDGE_PORT_NAME_BY_VLAN,
    Q_BRIDGE_PORT_VLANS_BY_NAME,
    Q_BRIDGE_PORT_VLANS_WITHOUT_NAME,
    Q_BRIDGE_COMPRESSED_PORT_VLANS_BY_NAME,
    Q_BRIDGE_COMPRESSED_PORT_VLANS_WITHOUT_NAME,
    Q_BRIDGE_MAX_OBJ_NUMBER,
} q_bridge_obj_index_t;

// global maximums - it is needed that these maximums be smaller than greatest module maximums
#define MAX_COMBINATORS_NUM   7
#define MAX_CALLBACKS_NUM     4

// find max value
#define MAX_2(a,b)              (((int)a > (int)b) ? ((int)a) : ((int)b))
#define MAX_4(a,b,c,d)          MAX_2(MAX_2(a,b), MAX_2(c,d))
#define MAX_8(a,b,c,d,e,f,g,h)  MAX_2(MAX_4(a,b,c,d),MAX_4(e,f,g,h))
#define MAX_OBJ_NUMBER          MAX_8(IF_MAX_OBJ_NUMBER, MSTP_MAX_OBJ_NUMBER, Q_BRIDGE_MAX_OBJ_NUMBER, 0, 0, 0, 0, 0)

typedef base_status_t (*syscall_fn_t)(module_index_t, int, base_rc_t, void*,
                                      unsigned int, unsigned int, va_list);
typedef base_status_t (*combinator_fn_t)(module_index_t, int, int, FILE *,
                                         base_rc_t, void*, unsigned int, unsigned int);
typedef base_status_t (*parser_fn_t)(module_index_t, int, int, int,
                                     char*, base_rc_t, void*, unsigned int, unsigned int);

/* Parser structure which holds all data needed by parsing/validation function. */
typedef struct {
    parser_fn_t callback;        // pointer to the parser/validator
    char match_case[MAX_STR_LEN]; // command string which will be executed by syscall function
} parser_spec_t;

/* Parser structure which holds all data needed by parsing function. */
typedef struct {
    syscall_fn_t *sys_call;       // pointer to the syscall function
    char *sys_call_command;       // command string which will be executed by syscall function
    int num_of_combinators;
    combinator_fn_t *combinator;  // array of combinators
    int num_of_cb[MAX_COMBINATORS_NUM]; // array which holds number of callbacks (parsers and validators) for individual combinator
    parser_spec_t parse_cb[MAX_COMBINATORS_NUM][MAX_CALLBACKS_NUM];      // array of parser functions per combinator
} sys_call_spec_t;

#endif /* INCLUDE_BASE_PRIVATE_DEFINES_H_ */
