/*
 * TTTech ieee8021MstpMib-mib-module
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
 /*
 * Note: this file originally auto-generated by mib2c using
 *  $
 */

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
/* module specific */
#include "ieee8021MstpCistTable/ieee8021MstpCistTable.h"
#include "ieee8021MstpCistPortTable/ieee8021MstpCistPortTable.h"
#include "ieee8021MstpConfigIdTable/ieee8021MstpConfigIdTable.h"
#include "ieee8021MstpPortTable/ieee8021MstpPortTable.h"
#include "ieee8021MstpTable/ieee8021MstpTable.h"

/* global variable */
u_long component_id = 1;// only one bridge is supported, hard coded

/** Initializes the ieee8021MstpMib module */
void init_ieee8021MstpMib(void)
{
  /* here we initialize all the tables we're planning to support */
    initialize_table_ieee8021MstpCistTable();
    initialize_table_ieee8021MstpCistPortTable();
    initialize_table_ieee8021MstpTable();
    initialize_table_ieee8021MstpPortTable();
    initialize_table_ieee8021MstpConfigIdTable();
}
