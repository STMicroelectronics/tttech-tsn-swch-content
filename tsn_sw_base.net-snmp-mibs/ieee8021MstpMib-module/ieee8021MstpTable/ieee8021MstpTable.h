/*
 * TTTech ieee8021MstpMib-mib-module
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
#ifndef IEEE8021MSTPTABLE_H
#define IEEE8021MSTPTABLE_H

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include "base_defines.h"

typedef struct ieee8021MstpTable_entry ieee8021MstpTable_entry_t;

/* function declarations */
void init_ieee8021MstpTable(void);
void initialize_table_ieee8021MstpTable(void);
netsnmp_tdata_row* ieee8021MstpTable_createEntry( netsnmp_tdata* table_data
                                                , u_long  ieee8021MstpComponentId
                                                , u_long  ieee8021MstpId
                                                );
int ieee8021MstpTable_fillEntry( char bridge_name[MAX_STR_LEN]
		                       , u_long bridge_id
							   , u_long mstp_id
							   , ieee8021MstpTable_entry_t *entry
							   );
int init_ieee8021MstpTable_with_sys_data(netsnmp_tdata *table_data);
Netsnmp_Node_Handler ieee8021MstpTable_handler;
NetsnmpCacheLoad ieee8021MstpTable_load;
NetsnmpCacheFree ieee8021MstpTable_free;
#define IEEE8021MSTPTABLE_TIMEOUT  60

/* column number definitions for table ieee8021MstpTable */
       #define COLUMN_IEEE8021MSTPCOMPONENTID		1
       #define COLUMN_IEEE8021MSTPID		2
       #define COLUMN_IEEE8021MSTPBRIDGEID		3
       #define COLUMN_IEEE8021MSTPTIMESINCETOPOLOGYCHANGE		4
       #define COLUMN_IEEE8021MSTPTOPOLOGYCHANGES		5
       #define COLUMN_IEEE8021MSTPTOPOLOGYCHANGE		6
       #define COLUMN_IEEE8021MSTPDESIGNATEDROOT		7
       #define COLUMN_IEEE8021MSTPROOTPATHCOST		8
       #define COLUMN_IEEE8021MSTPROOTPORT		9
       #define COLUMN_IEEE8021MSTPBRIDGEPRIORITY		10
       #define COLUMN_IEEE8021MSTPVIDS0		11
       #define COLUMN_IEEE8021MSTPVIDS1		12
       #define COLUMN_IEEE8021MSTPVIDS2		13
       #define COLUMN_IEEE8021MSTPVIDS3		14
       #define COLUMN_IEEE8021MSTPROWSTATUS		15

/* Typical data structure for a row entry */
struct ieee8021MstpTable_entry {
	/* Index values */
	u_long ieee8021MstpComponentId;
	u_long ieee8021MstpId;

	/* Column values */
	char ieee8021MstpBridgeId[MAX_STR_LEN];
	size_t ieee8021MstpBridgeId_len;
	u_long ieee8021MstpTimeSinceTopologyChange;
	u_long ieee8021MstpTopologyChanges;
	long ieee8021MstpTopologyChange;
	char ieee8021MstpDesignatedRoot[MAX_STR_LEN];
	size_t ieee8021MstpDesignatedRoot_len;
	long ieee8021MstpRootPathCost;
	u_long ieee8021MstpRootPort;
	long ieee8021MstpBridgePriority;
	long old_ieee8021MstpBridgePriority;
	char ieee8021MstpVids0[MAX_STR_LEN];
	size_t ieee8021MstpVids0_len;
	char ieee8021MstpVids1[MAX_STR_LEN];
	size_t ieee8021MstpVids1_len;
	char ieee8021MstpVids2[MAX_STR_LEN];
	size_t ieee8021MstpVids2_len;
	char ieee8021MstpVids3[MAX_STR_LEN];
	size_t ieee8021MstpVids3_len;
	long ieee8021MstpRowStatus;

	int   valid;
};
#endif /* IEEE8021MSTPTABLE_H */