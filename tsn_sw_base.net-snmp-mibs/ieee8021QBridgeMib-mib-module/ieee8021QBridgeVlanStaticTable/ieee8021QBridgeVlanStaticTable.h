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
#ifndef IEEE8021QBRIDGEVLANSTATICTABLE_H
#define IEEE8021QBRIDGEVLANSTATICTABLE_H

#include "base_defines.h"

typedef struct ieee8021QBridgeVlanStaticTable_entry ieee8021QBridgeVlanStaticTable_entry_t;

/* function declarations */
void init_ieee8021QBridgeVlanStaticTable(void);
void initialize_table_ieee8021QBridgeVlanStaticTable(void);
int init_ieee8021QBridgeVlanStaticTable_with_sys_data(netsnmp_tdata *table_data);
netsnmp_tdata_row *
ieee8021QBridgeVlanStaticTable_createEntry(netsnmp_tdata *table_data
                 , u_long  ieee8021QBridgeVlanStaticComponentId
                 , u_long  ieee8021QBridgeVlanStaticVlanIndex
                );
int ieee8021QBridgeVlanStaticTable_fillEntry(char bridge_name[MAX_STR_LEN], ieee8021QBridgeVlanStaticTable_entry_t *entry);
Netsnmp_Node_Handler ieee8021QBridgeVlanStaticTable_handler;
NetsnmpCacheLoad ieee8021QBridgeVlanStaticTable_load;
NetsnmpCacheFree ieee8021QBridgeVlanStaticTable_free;
#define IEEE8021QBRIDGEVLANSTATICTABLE_TIMEOUT  60

/* column number definitions for table ieee8021QBridgeVlanStaticTable */
       #define COLUMN_IEEE8021QBRIDGEVLANSTATICCOMPONENTID		1
       #define COLUMN_IEEE8021QBRIDGEVLANSTATICVLANINDEX		2
       #define COLUMN_IEEE8021QBRIDGEVLANSTATICNAME		3
       #define COLUMN_IEEE8021QBRIDGEVLANSTATICEGRESSPORTS		4
       #define COLUMN_IEEE8021QBRIDGEVLANFORBIDDENEGRESSPORTS		5
       #define COLUMN_IEEE8021QBRIDGEVLANSTATICUNTAGGEDPORTS		6
       #define COLUMN_IEEE8021QBRIDGEVLANSTATICROWSTATUS		7


/* Typical data structure for a row entry */
struct ieee8021QBridgeVlanStaticTable_entry {
/* Index values */
u_long ieee8021QBridgeVlanStaticComponentId;
u_long ieee8021QBridgeVlanStaticVlanIndex;

/* Column values */
char ieee8021QBridgeVlanStaticName[MAX_STR_LEN];
size_t ieee8021QBridgeVlanStaticName_len;
char old_ieee8021QBridgeVlanStaticName[MAX_STR_LEN];
size_t old_ieee8021QBridgeVlanStaticName_len;
char ieee8021QBridgeVlanStaticEgressPorts[8];
size_t ieee8021QBridgeVlanStaticEgressPorts_len;
char old_ieee8021QBridgeVlanStaticEgressPorts[8];
size_t old_ieee8021QBridgeVlanStaticEgressPorts_len;
char ieee8021QBridgeVlanForbiddenEgressPorts[MAX_STR_LEN];
size_t ieee8021QBridgeVlanForbiddenEgressPorts_len;
char old_ieee8021QBridgeVlanForbiddenEgressPorts[MAX_STR_LEN];
size_t old_ieee8021QBridgeVlanForbiddenEgressPorts_len;
char ieee8021QBridgeVlanStaticUntaggedPorts[8];
size_t ieee8021QBridgeVlanStaticUntaggedPorts_len;
char old_ieee8021QBridgeVlanStaticUntaggedPorts[8];
size_t old_ieee8021QBridgeVlanStaticUntaggedPorts_len;
long ieee8021QBridgeVlanStaticRowStatus;

int   valid;
};

#endif /* IEEE8021QBRIDGEVLANSTATICTABLE_H */
