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

#ifndef IEEE8021BRIDGEBASEPORTTABLE_H
#define IEEE8021BRIDGEBASEPORTTABLE_H

#include "base_prints.h"

typedef struct ieee8021BridgeBasePortTable_entry ieee8021BridgeBasePortTable_entry_t;

/* function declarations */
void init_ieee8021BridgeBasePortTable(void);
void initialize_table_ieee8021BridgeBasePortTable(void);
int init_ieee8021BridgeBasePortTable_with_sys_data(netsnmp_tdata *table_data);
netsnmp_tdata_row *
ieee8021BridgeBasePortTable_createEntry(
        netsnmp_tdata *table_data,
        u_long  ieee8021BridgeBasePortComponentId,
        u_long  ieee8021BridgeBasePort);
int ieee8021BridgeBasePortTable_fillEntry(char bridge_name[MAX_STR_LEN], char port_name[MAX_STR_LEN],
        ieee8021BridgeBasePortTable_entry_t *entry);
Netsnmp_Node_Handler ieee8021BridgeBasePortTable_handler;
NetsnmpCacheLoad ieee8021BridgeBasePortTable_load;
NetsnmpCacheFree ieee8021BridgeBasePortTable_free;

#define IEEE8021BRIDGEBASEPORTTABLE_TIMEOUT  60

/* column number definitions for table ieee8021BridgeBasePortTable */
#define COLUMN_IEEE8021BRIDGEBASEPORTCOMPONENTID        		1
#define COLUMN_IEEE8021BRIDGEBASEPORT        					2
#define COLUMN_IEEE8021BRIDGEBASEPORTIFINDEX        			3
#define COLUMN_IEEE8021BRIDGEBASEPORTDELAYEXCEEDEDDISCARDS      4
#define COLUMN_IEEE8021BRIDGEBASEPORTMTUEXCEEDEDDISCARDS        5
#define COLUMN_IEEE8021BRIDGEBASEPORTCAPABILITIES        		6
#define COLUMN_IEEE8021BRIDGEBASEPORTTYPECAPABILITIES        	7
#define COLUMN_IEEE8021BRIDGEBASEPORTTYPE        				8
#define COLUMN_IEEE8021BRIDGEBASEPORTEXTERNAL        			9
#define COLUMN_IEEE8021BRIDGEBASEPORTADMINPOINTTOPOINT        	10
#define COLUMN_IEEE8021BRIDGEBASEPORTOPERPOINTTOPOINT        	11
#define COLUMN_IEEE8021BRIDGEBASEPORTNAME        				12

    /* Typical data structure for a row entry */
struct ieee8021BridgeBasePortTable_entry {
    /* Index values */
    u_long ieee8021BridgeBasePortComponentId;
    u_long ieee8021BridgeBasePort;

    /* Column values */
    long ieee8021BridgeBasePortIfIndex;
    long old_ieee8021BridgeBasePortIfIndex;
    u_long ieee8021BridgeBasePortDelayExceededDiscards;
    u_long ieee8021BridgeBasePortMtuExceededDiscards;
    char ieee8021BridgeBasePortCapabilities[MAX_STR_LEN];
    size_t ieee8021BridgeBasePortCapabilities_len;
    char ieee8021BridgeBasePortTypeCapabilities[MAX_STR_LEN];
    size_t ieee8021BridgeBasePortTypeCapabilities_len;
    long ieee8021BridgeBasePortType;
    long ieee8021BridgeBasePortExternal;
    long ieee8021BridgeBasePortAdminPointToPoint;
    long old_ieee8021BridgeBasePortAdminPointToPoint;
    long ieee8021BridgeBasePortOperPointToPoint;
    char ieee8021BridgeBasePortName[MAX_STR_LEN];
    size_t ieee8021BridgeBasePortName_len;

    int   valid;
};

#endif /* IEEE8021BRIDGEBASEPORTTABLE_H */
