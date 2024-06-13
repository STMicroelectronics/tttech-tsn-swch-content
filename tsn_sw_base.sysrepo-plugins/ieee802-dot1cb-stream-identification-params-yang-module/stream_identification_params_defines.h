/*
 * TTTech ieee802-dot1cb-stream-identification-params-yang-module
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

#ifndef SID_PARAMS_DEFINES_H_
#define SID_PARAMS_DEFINES_H_

/* common includes */
#include "common_defines.h"
#include "common.h"
/* sysrepo includes */
#include <sysrepo.h>
#include <sysrepo/values.h>
#include <sysrepo/xpath.h>
/* libbase includes */
#include "libbase/base_q_bridge_linux_tools.h"
/* libtsn includes */
#include "libtsn/tsn.h"
#include "libtsn/tsn_def.h"
#include "libtsn/tsn_sid.h"

#define SID_PARAMS_MODULE_NAME_STR          "ieee802-dot1cb-stream-identification-params"

#define SID_PARAMS_XPATH                    "/ieee802-dot1cb-stream-identification-params:stream-identification-params"

#define SID_PARAMS_TABLE_LENGTH_XPATH       SID_PARAMS_XPATH "/table-length"
#define SID_PARAMS_MAX_SUP_STRHDL_XPATH     SID_PARAMS_XPATH "/max-supported-stream-handle"
#define SID_PARAMS_METHOD_NULL_XPATH        SID_PARAMS_XPATH "/sid-methods/null-stream"
#define SID_PARAMS_METHOD_SOURCE_XPATH      SID_PARAMS_XPATH "/sid-methods/source-stream"
#define SID_PARAMS_METHOD_ACTIVE_XPATH      SID_PARAMS_XPATH "/sid-methods/active-stream"
#define SID_PARAMS_METHOD_IP_XPATH          SID_PARAMS_XPATH "/sid-methods/ip-stream"
#define SID_PARAMS_ENABLED_PORTS_XPATH      SID_PARAMS_XPATH "/enabled-ports"

#endif
