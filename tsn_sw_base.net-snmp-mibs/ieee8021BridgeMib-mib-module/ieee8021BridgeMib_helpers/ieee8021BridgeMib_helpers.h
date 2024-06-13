/*
 * TTTech ieee8021BridgeMib-mib-module
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
#ifndef IEEE8021BRIDGEMIBHELPERS_H
#define IEEE8021BRIDGEMIBHELPERS_H

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
/* libbase includes */
#include "base_prints.h"
#include "base_linux_tool_helpers.h"
#include "base_q_bridge_linux_tools.h"
/* libtsn includes */
#include "tsn.h"
#include "tsn_bridge.h"

/**
 * @brief This is helper function for get port name by ID which is given by tsn_bridge_port_id function from TSN libary.
 * The reason: There is a function tsn_port_open_by_id which opens port by interface index(which is different from tsn port id.)
 * The tsn port id is used for ieee8021BridgeBasePortNumber or similar objects.
 * @param[in] port_id Contains the ID of bridge port.
 * @param[out] port_name Place holder for port name.
 * @return EXIT_SUCCESS or EXIT_FAILURE.
 */
int get_port_name_by_tsn_port_id(uint32_t port_id, char port_name[MAX_STR_LEN]);

/**
 * @brief This is helper function which check if row is in table.
 * @param[in] table Targeted table.
 * @param[in] component_id The ieee8021BridgeBasePortComponentId value.
 * @param[in] port_id The ieee8021BridgeBasePort value.
 * @param[in] priority The ieee8021BridgeTrafficClassPriority value.
 * @return 0 or 1 - 1 is confirmation that row exists in table.
 */
int find_row_in_tc_table(netsnmp_tdata* table, u_long component_id, uint32_t port_id, u_long priority);
#endif
