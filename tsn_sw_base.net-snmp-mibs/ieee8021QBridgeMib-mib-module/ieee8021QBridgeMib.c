/*
 * TTTech ieee8021QBridgeMib-mib-module
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
 /*
 * Note: this file originally auto-generated by mib2c using
 *  $
 */

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
/* module cpecific */
#include "ieee8021QBridgeTable/ieee8021QBridgeTable.h"
#include "ieee8021QBridgeVlanStaticTable/ieee8021QBridgeVlanStaticTable.h"

/** Initializes the ieee8021QBridgeMib module */
void init_ieee8021QBridgeMib(void)
{
	/* here we initialize all the tables we're planning on supporting */
	initialize_table_ieee8021QBridgeTable();
	initialize_table_ieee8021QBridgeVlanStaticTable();
}