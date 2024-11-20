/*
 * TTTech ieee802-dot1ab-lldp-yang-module
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

#ifndef MSTP_DEFINES_H_
#define MSTP_DEFINES_H_

#define MSTP_MODULE_NAME                    "ieee8021-mstp"

#define MSTP_DISABLED_STR                   "disabled"
#define MSTP_LISTENING_STR                  "listening"
#define MSTP_LEARNING_STR                   "learning"
#define MSTP_FORWARDING_STR                 "forwarding"
#define MSTP_BLOCKING_STR                   "blocking"
#define MSTP_DISCARDING_STR                 "discarding"

#define MSTP_PORT_ROLE_ROOT_STR             "root"
#define MSTP_PORT_ROLE_ALTERNATE_STR        "alternate"
#define MSTP_PORT_ROLE_DESIGNATED_STR       "designated"
#define MSTP_PORT_ROLE_BACKUP_STR           "backup"

#define MSTP_YES_STR                        "yes "
#define MSTP_NO_STR                         "no "

// mappings for objects - key and value matrix
static char yes_no_map[2][2][MAX_STR_LEN] = {
    // 2 pairs
    { BASE_YES, BASE_TRUE },
    { BASE_NO,  BASE_FALSE },
};

static char true_false_map[2][2][MAX_STR_LEN] = {
    // 2 pairs
    { BASE_ONE_STR,     MSTP_YES_STR },
    { BASE_ZERO_STR,    MSTP_NO_STR },
};

static char port_role_map[5][2][MAX_STR_LEN] = {
    // 4 pairs
    { BASE_ROLE_DISABLED_STR,    MSTP_DISABLED_STR },
    { BASE_ROOT_STR,             MSTP_PORT_ROLE_ROOT_STR },
    { BASE_ALTERNATE_STR,        MSTP_PORT_ROLE_ALTERNATE_STR },
    { BASE_DESIGNATED_STR,       MSTP_PORT_ROLE_DESIGNATED_STR },
    { BASE_BACKUP_STR,           MSTP_PORT_ROLE_BACKUP_STR },
};

#if 0
static char port_state_map[6][2][MAX_STR_LEN] = {
    // 5 pairs
    { BASE_DISABLED_STR,    MSTP_DISABLED_STR },
    { BASE_LISTENING_STR,   MSTP_LISTENING_STR },
    { BASE_LEARNING_STR,    MSTP_LEARNING_STR },
    { BASE_FORWARDING_STR,  MSTP_FORWARDING_STR },
    { BASE_BLOCKING_STR,    MSTP_BLOCKING_STR },
    { BASE_DISCARDING_STR,  MSTP_DISCARDING_STR },
};
#endif

/* MSTP xpath for state data callback */
#define MSTP_XPATH                      "/ieee8021-mstp:ieee8021-mstp"
#define MSTP_CIST_ENTRY_XPATH           MSTP_XPATH "/ieee8021MstpCistTable/ieee8021MstpCistEntry[ieee8021MstpCistComponentId='%s']/%s"
#define MSTP_ENTRY_XPATH                MSTP_XPATH "/ieee8021MstpTable/ieee8021MstpEntry[ieee8021MstpComponentId='%s'][ieee8021MstpId='%s']/%s"
#define MSTP_CIST_PORT_ENTRY_XPATH      MSTP_XPATH "/ieee8021MstpCistPortTable/ieee8021MstpCistPortEntry[ieee8021MstpCistPortComponentId='%s'][ieee8021MstpCistPortNum='%s']/%s"
#define MSTP_PORT_ENTRY_XPATH           MSTP_XPATH "/ieee8021MstpPortTable/ieee8021MstpPortEntry[ieee8021MstpPortComponentId='%s'][ieee8021MstpPortMstId='%s'][ieee8021MstpPortNum='%s']/%s"
#define MSTP_CONFIG_ID_ENTRY_XPATH      MSTP_XPATH "/ieee8021MstpConfigIdTable/ieee8021MstpConfigIdEntry[ieee8021MstpConfigIdComponentId='%s']/%s"

#endif /* MSTP_DEFINES_H_ */
