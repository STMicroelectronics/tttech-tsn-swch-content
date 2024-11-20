/*
 * TTTech ieee8021QBridgeMib-mib-module
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
#ifndef ieee8021QBridgeTable_H
#define ieee8021QBridgeTable_H
/* module specific */
#include "base_defines.h"


typedef struct ieee8021QBridgeTable_entry ieee8021QBridgeTable_entry_t;

/* function declarations */
void init_ieee8021QBridgeTable(void);
void initialize_table_ieee8021QBridgeTable(void);
int init_ieee8021QBridgeTable_with_sys_data(netsnmp_tdata *table_data);
netsnmp_tdata_row *ieee8021QBridgeTable_createEntry(netsnmp_tdata *table_data, u_long  ieee8021BridgeBaseComponentId);
int ieee8021QBridgeTable_fillEntry(char bridge_name[MAX_STR_LEN], ieee8021QBridgeTable_entry_t *entry);
Netsnmp_Node_Handler ieee8021QBridgeTable_handler;
NetsnmpCacheLoad ieee8021QBridgeTable_load;
NetsnmpCacheFree ieee8021QBridgeTable_free;

#define IEEE8021QBRIDGETABLE_TIMEOUT  60

/* column number definitions for table ieee8021QBridgeTable */
#define COLUMN_IEEE8021QBRIDGECOMPONENTID			1
#define COLUMN_IEEE8021QBRIDGEVLANVERSIONNUMBER		2
#define COLUMN_IEEE8021QBRIDGEMAXVLANID				3
#define COLUMN_IEEE8021QBRIDGEMAXSUPPORTEDVLANS		4
#define COLUMN_IEEE8021QBRIDGENUMVLANS				5
#define COLUMN_IEEE8021QBRIDGEMVRPENABLEDSTATUS		6

/* Typical data structure for a row entry */
struct ieee8021QBridgeTable_entry {
	/* Index values */
	u_long ieee8021QBridgeComponentId;

	/* Column values */
	long ieee8021QBridgeVlanVersionNumber;
	long ieee8021QBridgeMaxVlanId;
	u_long ieee8021QBridgeMaxSupportedVlans;
	u_long ieee8021QBridgeNumVlans;
	long ieee8021QBridgeMvrpEnabledStatus;
	long old_ieee8021QBridgeMvrpEnabledStatus;

	int   valid;
};

#endif /* ieee8021QBridgeTable_H */
