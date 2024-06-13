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
#include "ieee8021QBridgeTable.h"
/* libbase includes */
#include "base_prints.h"
#include "base_linux_tool_helpers.h"
#include "base_q_bridge_linux_tools.h"
/* libtsn includes */
#include "tsn.h"
#include "tsn_qbridge.h"

/** Initializes the ieee8021BridgeBaseTable module */
void init_ieee8021QBridgeTable(void)
{
  /* here we initialize all the tables we're planning on supporting */
    initialize_table_ieee8021QBridgeTable();
}

void initialize_table_ieee8021QBridgeTable(void)
{
    const oid ieee8021QBridgeTable_oid[] 		= {1,3,111,2,802,1,1,4,1,1,1};
    const size_t ieee8021QBridgeTable_oid_len   = OID_LENGTH(ieee8021QBridgeTable_oid);
    netsnmp_handler_registration    			*reg;
    netsnmp_tdata                   			*table_data;
    netsnmp_table_registration_info 			*table_info;
    netsnmp_cache                   			*cache;

    DEBUGMSGTL(("ieee8021QBridgeTable:init", "initializing table ieee8021QBridgeTable\n"));

    reg = netsnmp_create_handler_registration("ieee8021QBridgeTable", ieee8021QBridgeTable_handler,
    		ieee8021QBridgeTable_oid, ieee8021QBridgeTable_oid_len, HANDLER_CAN_RWRITE);

    table_data = netsnmp_tdata_create_table("ieee8021QBridgeTable", 0 );
    if (NULL == table_data)
    {
        snmp_log(LOG_ERR, "error creating data table for ieee8021QBridgeTable\n");
        return;
    }
    cache = netsnmp_cache_create(IEEE8021QBRIDGETABLE_TIMEOUT,
    		ieee8021QBridgeTable_load, ieee8021QBridgeTable_free,
			ieee8021QBridgeTable_oid, ieee8021QBridgeTable_oid_len);
    if (NULL == cache)
    {
        snmp_log(LOG_ERR, "error creating cache for ieee8021QBridgeTable\n");
    }
    else
    {
        cache->magic = (void *)table_data;
        cache->flags =  NETSNMP_CACHE_DONT_FREE_EXPIRED     | \
                        NETSNMP_CACHE_DONT_AUTO_RELEASE     | \
                        NETSNMP_CACHE_AUTO_RELOAD           | \
                        NETSNMP_CACHE_DONT_FREE_BEFORE_LOAD | \
                        NETSNMP_CACHE_DONT_INVALIDATE_ON_SET ;
    }

    table_info = SNMP_MALLOC_TYPEDEF(netsnmp_table_registration_info);
    if (NULL == table_info)
    {
        snmp_log(LOG_ERR, "error creating table info for ieee8021QBridgeTable\n");
        return;
    }
    /* index: ieee8021BridgeBaseComponentId */
    netsnmp_table_helper_add_indexes(table_info, ASN_UNSIGNED, 0);

    table_info->min_column = COLUMN_IEEE8021QBRIDGEVLANVERSIONNUMBER;
    table_info->max_column = COLUMN_IEEE8021QBRIDGEMVRPENABLEDSTATUS;
    
    netsnmp_tdata_register(reg, table_data, table_info);

    /* Initialise the contents of the table here */
    if(0 != init_ieee8021QBridgeTable_with_sys_data(table_data))
    {
        snmp_log(LOG_ERR, "ERROR: ieee8021QBridgeTable initialization was unsuccessful.\n");
        free(table_info);
        return;
    }
    else
    {
        snmp_log(LOG_INFO,"INFO: ieee8021QBridgeTable initialization was successful.\n");
        netsnmp_cache_timer_start(cache);
    }
}

/* init table */
int init_ieee8021QBridgeTable_with_sys_data(netsnmp_tdata *table_data)
{
    netsnmp_tdata_row *row = NULL;
    char bridge_names[MAX_NUM_OF_INTERFACES][MAX_STR_LEN];
    u_long component_id = 1;
    int i = 0;

    snmp_log(LOG_INFO, "INFO: init_ieee8021QBridgeTable_with_sys_data called.\n");

    if(EXIT_FAILURE == get_bridge_names(bridge_names, MAX_NUM_OF_INTERFACES, MAX_STR_LEN))
    {
    	snmp_log(LOG_ERR, "ERROR: get_bridge_names failed.\n");
        return 1;
    }

    while((i < MAX_NUM_OF_INTERFACES) && (0 != strcmp(bridge_names[i], "")))
    {
		row = ieee8021QBridgeTable_createEntry(table_data, component_id);
		if(NULL != row)
		{
			ieee8021QBridgeTable_fillEntry(bridge_names[i], (ieee8021QBridgeTable_entry_t*)row->data);
		}
		else
		{
			snmp_log(LOG_ERR, "ERROR: New row is not created.\n");
			/* error */
			return 1;
		}
		row = NULL;
		component_id++;
    	i++;
    }

    /* success */
    return 0;
}
/* create a new row in the table */
netsnmp_tdata_row *ieee8021QBridgeTable_createEntry(netsnmp_tdata *table_data, u_long ieee8021QBridgeComponentId)
{
    struct ieee8021QBridgeTable_entry *entry = NULL;
    netsnmp_tdata_row *row = NULL;

    entry = SNMP_MALLOC_TYPEDEF(struct ieee8021QBridgeTable_entry);
    if (!entry)
    {
        return NULL;
    }

    row = netsnmp_tdata_create_row();
    if (!row)
    {
        SNMP_FREE(entry);
        return NULL;
    }
    row->data = entry;

    snmp_log(LOG_INFO, "ieee8021QBridgeTable_createEntry - (%ld) \n", ieee8021QBridgeComponentId);
    entry->ieee8021QBridgeComponentId = ieee8021QBridgeComponentId;
    netsnmp_tdata_row_add_index(row, ASN_UNSIGNED, &(entry->ieee8021QBridgeComponentId), sizeof(entry->ieee8021QBridgeComponentId));
    if (table_data)
    {
        netsnmp_tdata_add_row( table_data, row );
    }

	return row;
}

/* fill a new row data in the table */
int ieee8021QBridgeTable_fillEntry(char bridge_name[MAX_STR_LEN], ieee8021QBridgeTable_entry_t *entry)
{
	struct port_hdl *port = NULL;
	uint32_t tmp_ui32 = 0u;

    snmp_log(LOG_DEBUG, "called: ieee8021QBridgeTable_fillEntry\n");

    if (!entry)
    {
        snmp_log(LOG_ERR, "ieee8021QBridgeTable_fillEntry:entry == NULL\n");
        /* error */
        return 1;
    }

	port = tsn_port_open_by_name(bridge_name);
	if(NULL == port)
	{
        snmp_log(LOG_ERR, "port could not open\n");
        /* error */
        return 1;
	}

    if(0 == tsn_qbridge_get_max_vlan_id(port, &tmp_ui32))
    {
    	entry->ieee8021QBridgeMaxVlanId = tmp_ui32;
    }

    if(0 == tsn_qbridge_get_max_supported_vlans(port, &tmp_ui32))
    {
    	entry->ieee8021QBridgeMaxSupportedVlans = tmp_ui32;
    }

    if(0 == tsn_qbridge_get_num_vlans(port, &tmp_ui32))
    {
    	entry->ieee8021QBridgeNumVlans = tmp_ui32;
    }

    tsn_port_close(port);

    return 0;
}

/* remove a row from the table */
void ieee8021QBridgeTable_removeEntry(netsnmp_tdata *table_data, netsnmp_tdata_row *row)
{
    struct ieee8021QBridgeTable_entry *entry = NULL;

    if (!row)
    {
        return;    /* Nothing to remove */
    }

    snmp_log(LOG_INFO, "ieee8021QBridgeTable_removeEntry entry.\n");

    entry = (struct ieee8021QBridgeTable_entry *)row->data;
    SNMP_FREE(entry);   /* XXX - release any other internal resources */

    if (table_data)
    {
        netsnmp_tdata_remove_and_delete_row( table_data, row );
    }
    else
    {
        netsnmp_tdata_delete_row( row );    
    }
}

/* Example cache handling - set up table_data list from a suitable file */
int ieee8021QBridgeTable_load( netsnmp_cache *cache, void *vmagic )
{
    /* will be used to iterate trough ieee8021QBridgeTable_head */
    netsnmp_tdata_row *agent_row = NULL;
    /* helper to keep next agent row */
    netsnmp_tdata_row *next_agent_row = NULL;
    netsnmp_tdata *table = (netsnmp_tdata *)vmagic;
    ieee8021QBridgeTable_entry_t *entry = NULL;
    int bridge_found = 0;
    char bridge_names[MAX_NUM_OF_INTERFACES][MAX_STR_LEN];
    u_long component_id = 1;
    int i = 0;

    snmp_log(LOG_DEBUG, "called: ieee8021QBridgeTable_load\n");

    /* gather fresh system data */
    if(EXIT_FAILURE == get_bridge_names(bridge_names, MAX_NUM_OF_INTERFACES, MAX_STR_LEN))
    {
    	snmp_log(LOG_ERR, "ERROR: get_bridge_names failed.\n");
        return 1;
    }

    /* returns the first row in the table */
    agent_row = netsnmp_tdata_row_first (table);

    while(NULL != agent_row)
    {
        snmp_log(LOG_ERR, "Agent row exists.\n");
        entry = (ieee8021QBridgeTable_entry_t *)agent_row->data;

        if(NULL != entry)
        {
            if(!entry->ieee8021QBridgeComponentId)
            {
                snmp_log(LOG_DEBUG, "there is no more agent_row, erase it from agent\n");
                /* returns next row */
                next_agent_row = netsnmp_tdata_row_next (table, agent_row);
                ieee8021QBridgeTable_removeEntry(table, agent_row);
                agent_row = next_agent_row;
                next_agent_row = NULL;
            }
            /* agent_row exist on system, skip it (leave it on agent) */
            else
            {
                snmp_log(LOG_DEBUG, "agent_row exist on system, skip it\n");
                /* refresh data in this existing row on agent */
                ieee8021QBridgeTable_fillEntry(bridge_names[0], (ieee8021QBridgeTable_entry_t *)agent_row->data);
                /* take next */
                agent_row = netsnmp_tdata_row_next (table, agent_row);
            }
        }
        else
        {
            snmp_log(LOG_ERR, "error: agent_row->data is NULL\n");
        }
    }

    while((i < MAX_NUM_OF_INTERFACES) && (0 != strcmp(bridge_names[i], "")))
    {
        snmp_log(LOG_DEBUG, "load: looking for bridge: %s\n", bridge_names[i]);
        /* returns the first row in the table */
        agent_row = netsnmp_tdata_row_first (table);

        while((NULL != agent_row) && (bridge_found == 0))
        {
            entry = (ieee8021QBridgeTable_entry_t *)agent_row->data;

            if((entry->ieee8021QBridgeComponentId == component_id))
            {
                bridge_found = 1;
            }

            agent_row = netsnmp_tdata_row_next (table, agent_row);
        }

        if(bridge_found == 0)
        {
            snmp_log(LOG_DEBUG, "load: Adding new bridge: %s\n", bridge_names[i]);
            agent_row = ieee8021QBridgeTable_createEntry(table, component_id);
            ieee8021QBridgeTable_fillEntry(bridge_names[i], agent_row->data);
        }
        component_id++;
        i++;
        bridge_found = 0;
    }

    return 0;
}

void ieee8021QBridgeTable_free( netsnmp_cache *cache, void *vmagic ) {
    netsnmp_tdata     *table = (netsnmp_tdata *)vmagic;
    netsnmp_tdata_row *this;

    while ((this = netsnmp_tdata_row_first(table)))
    {
        netsnmp_tdata_remove_and_delete_row(table, this);
    }
}

/** handles requests for the ieee8021QBridgeTable table */
int
ieee8021QBridgeTable_handler(
    netsnmp_mib_handler               *handler,
    netsnmp_handler_registration      *reginfo,
    netsnmp_agent_request_info        *reqinfo,
    netsnmp_request_info              *requests) {

    netsnmp_request_info       *request;
    netsnmp_table_request_info *table_info;
    netsnmp_tdata              *table_data;
    netsnmp_tdata_row          *table_row;
    struct ieee8021QBridgeTable_entry          *table_entry;
    int                         ret;

    DEBUGMSGTL(("ieee8021QBridgeTable:handler", "Processing request (%d)\n", reqinfo->mode));

    switch (reqinfo->mode) {
        /*
         * Read-support (also covers GetNext requests)
         */
    case MODE_GET:
        for (request=requests; request; request=request->next) {
            if (request->processed)
                continue;

            table_entry = (struct ieee8021QBridgeTable_entry *)
                              netsnmp_tdata_extract_entry(request);
            table_info  =     netsnmp_extract_table_info( request);
    
            switch (table_info->colnum) {
#if 0
            case COLUMN_IEEE8021QBRIDGEVLANVERSIONNUMBER:
                if ( !table_entry ) {
                    netsnmp_set_request_error(reqinfo, request,
                                              SNMP_NOSUCHINSTANCE);
                    continue;
                }
                snmp_set_var_typed_integer( request->requestvb, ASN_INTEGER,
                                            table_entry->ieee8021QBridgeVlanVersionNumber);
                break;
#endif
            case COLUMN_IEEE8021QBRIDGEMAXVLANID:
                if ( !table_entry ) {
                    netsnmp_set_request_error(reqinfo, request,
                                              SNMP_NOSUCHINSTANCE);
                    continue;
                }
                snmp_set_var_typed_integer( request->requestvb, ASN_INTEGER,
                                            table_entry->ieee8021QBridgeMaxVlanId);
                break;
            case COLUMN_IEEE8021QBRIDGEMAXSUPPORTEDVLANS:
                if ( !table_entry ) {
                    netsnmp_set_request_error(reqinfo, request,
                                              SNMP_NOSUCHINSTANCE);
                    continue;
                }
                snmp_set_var_typed_integer( request->requestvb, ASN_UNSIGNED,
                                            table_entry->ieee8021QBridgeMaxSupportedVlans);
                break;
            case COLUMN_IEEE8021QBRIDGENUMVLANS:
                if ( !table_entry ) {
                    netsnmp_set_request_error(reqinfo, request,
                                              SNMP_NOSUCHINSTANCE);
                    continue;
                }
                snmp_set_var_typed_integer( request->requestvb, ASN_GAUGE,
                                            table_entry->ieee8021QBridgeNumVlans);
                break;
#if 0
            case COLUMN_IEEE8021QBRIDGEMVRPENABLEDSTATUS:
                if ( !table_entry ) {
                    netsnmp_set_request_error(reqinfo, request,
                                              SNMP_NOSUCHINSTANCE);
                    continue;
                }
                snmp_set_var_typed_integer( request->requestvb, ASN_INTEGER,
                                            table_entry->ieee8021QBridgeMvrpEnabledStatus);
                break;
#endif
            default:
                netsnmp_set_request_error(reqinfo, request,
                                          SNMP_NOSUCHOBJECT);
                break;
            }
        }
        break;

        /*
         * Write-support
         */
    case MODE_SET_RESERVE1:
        for (request=requests; request; request=request->next) {
            if (request->processed)
                continue;
#if 0
            table_entry = (struct ieee8021QBridgeTable_entry *)
                              netsnmp_tdata_extract_entry(request);
#endif
            table_info  =     netsnmp_extract_table_info( request);
    
            switch (table_info->colnum) {
#if 0
            case COLUMN_IEEE8021QBRIDGEMVRPENABLEDSTATUS:
                /* or possibly 'netsnmp_check_vb_int_range' */
                ret = netsnmp_check_vb_int( request->requestvb );
                if ( ret != SNMP_ERR_NOERROR ) {
                    netsnmp_set_request_error( reqinfo, request, ret );
                    return SNMP_ERR_NOERROR;
                }
                break;
#endif
            default:
                netsnmp_set_request_error( reqinfo, request,
                                           SNMP_ERR_NOTWRITABLE );
                return SNMP_ERR_NOERROR;
            }
        }
        break;

    case MODE_SET_RESERVE2:
        break;

    case MODE_SET_FREE:
        break;

    case MODE_SET_ACTION:
#if 0
        for (request=requests; request; request=request->next) {
            if (request->processed)
                continue;

            table_entry = (struct ieee8021QBridgeTable_entry *)
                              netsnmp_tdata_extract_entry(request);
            table_info  =     netsnmp_extract_table_info( request);
    
            switch (table_info->colnum) {
            case COLUMN_IEEE8021QBRIDGEMVRPENABLEDSTATUS:
                table_entry->old_ieee8021QBridgeMvrpEnabledStatus = table_entry->ieee8021QBridgeMvrpEnabledStatus;
                table_entry->ieee8021QBridgeMvrpEnabledStatus     = *request->requestvb->val.integer;
                break;
            }
        }
#endif
        break;

    case MODE_SET_UNDO:
#if 0
        for (request=requests; request; request=request->next) {
            if (request->processed)
                continue;

            table_entry = (struct ieee8021QBridgeTable_entry *)
                              netsnmp_tdata_extract_entry(request);
            table_row   =     netsnmp_tdata_extract_row(  request);
            table_data  =     netsnmp_tdata_extract_table(request);
            table_info  =     netsnmp_extract_table_info( request);
    
            switch (table_info->colnum) {
            case COLUMN_IEEE8021QBRIDGEMVRPENABLEDSTATUS:
                table_entry->ieee8021QBridgeMvrpEnabledStatus     = table_entry->old_ieee8021QBridgeMvrpEnabledStatus;
                table_entry->old_ieee8021QBridgeMvrpEnabledStatus = 0;
                break;
            }
        }
#endif
        break;

    case MODE_SET_COMMIT:
        break;
    }
    return SNMP_ERR_NOERROR;
}