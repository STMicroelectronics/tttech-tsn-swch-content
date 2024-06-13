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
#include "ieee8021QBridgeVlanStaticTable.h"
/* libbase includes */
#include "base_prints.h"
#include "base_linux_tool_helpers.h"
#include "base_q_bridge_linux_tools.h"

/** Initializes the ieee8021QBridgeVlanStaticTable module */
void
init_ieee8021QBridgeVlanStaticTable(void)
{
  /* here we initialize all the tables we're planning on supporting */
    initialize_table_ieee8021QBridgeVlanStaticTable();
}


/** Initialize the ieee8021QBridgeVlanStaticTable table by defining its contents and how it's structured */
void
initialize_table_ieee8021QBridgeVlanStaticTable(void)
{
    const oid ieee8021QBridgeVlanStaticTable_oid[] = {1,3,111,2,802,1,1,4,1,4,3};
    const size_t ieee8021QBridgeVlanStaticTable_oid_len   = OID_LENGTH(ieee8021QBridgeVlanStaticTable_oid);
    netsnmp_handler_registration    *reg;
    netsnmp_tdata                   *table_data;
    netsnmp_table_registration_info *table_info;
    netsnmp_cache                   *cache;

    //DEBUGMSGTL(("ieee8021QBridgeVlanStaticTable:init", "initializing table ieee8021QBridgeVlanStaticTable\n"));

    reg = netsnmp_create_handler_registration(
              "ieee8021QBridgeVlanStaticTable",     ieee8021QBridgeVlanStaticTable_handler,
              ieee8021QBridgeVlanStaticTable_oid, ieee8021QBridgeVlanStaticTable_oid_len,
              HANDLER_CAN_RWRITE
              );

    table_data = netsnmp_tdata_create_table( "ieee8021QBridgeVlanStaticTable", 0 );
    if (NULL == table_data) {
        snmp_log(LOG_ERR,"error creating tdata table for ieee8021QBridgeVlanStaticTable\n");
        return;
    }
    cache = netsnmp_cache_create(IEEE8021QBRIDGEVLANSTATICTABLE_TIMEOUT,
                                  ieee8021QBridgeVlanStaticTable_load, ieee8021QBridgeVlanStaticTable_free,
                                  ieee8021QBridgeVlanStaticTable_oid, ieee8021QBridgeVlanStaticTable_oid_len);
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

    table_info = SNMP_MALLOC_TYPEDEF( netsnmp_table_registration_info );
    if (NULL == table_info) {
        snmp_log(LOG_ERR,"error creating table info for ieee8021QBridgeVlanStaticTable\n");
        return;
    }
    netsnmp_table_helper_add_indexes(table_info,
                           ASN_UNSIGNED,  /* index: ieee8021QBridgeVlanStaticComponentId */
                           ASN_UNSIGNED,  /* index: ieee8021QBridgeVlanStaticVlanIndex */
                           0);

    table_info->min_column = COLUMN_IEEE8021QBRIDGEVLANSTATICNAME;
    table_info->max_column = COLUMN_IEEE8021QBRIDGEVLANSTATICROWSTATUS;
    
    netsnmp_tdata_register( reg, table_data, table_info );

    /* Initialise the contents of the table here */
    if(0 != init_ieee8021QBridgeVlanStaticTable_with_sys_data(table_data))
    {
        snmp_log(LOG_ERR, "ERROR: ieee8021QBridgeVlanStaticTable initialization was unsuccessful.\n");
        free(table_info);
        return;
    }
    else
    {
        snmp_log(LOG_INFO,"INFO: ieee8021QBridgeVlanStaticTable initialization was successful.\n");
        netsnmp_cache_timer_start(cache);
    }

}

/* init table */
int init_ieee8021QBridgeVlanStaticTable_with_sys_data(netsnmp_tdata *table_data)
{
    netsnmp_tdata_row *row = NULL;
    char bridge_names[MAX_NUM_OF_INTERFACES][MAX_STR_LEN];
    char result_array[BASE_MAX_VID][BASE_TEN];
    u_long component_id = 1;
    int i = 0;

    snmp_log(LOG_INFO, "INFO: init_ieee8021QBridgeVlanStaticTable_with_sys_data called.\n");

    if(EXIT_FAILURE == get_bridge_names(bridge_names, MAX_NUM_OF_INTERFACES, MAX_STR_LEN))
    {
    	snmp_log(LOG_ERR, "ERROR: get_bridge_names failed.\n");
        return 1;
    }

    //0 is for uncompressed vlan ids
    if (EXIT_FAILURE == get_bridge_vlans(result_array, BASE_MAX_VID, BASE_TEN, 0))
    {
        return 1;
    }

    while((i < BASE_MAX_VID) && (0 != strncmp("", result_array[i], BASE_TEN)))
    {
		row = ieee8021QBridgeVlanStaticTable_createEntry(table_data, component_id, atoi(result_array[i]));
		if(NULL != row)
		{
			ieee8021QBridgeVlanStaticTable_fillEntry(bridge_names[0], (ieee8021QBridgeVlanStaticTable_entry_t*)row->data);
		}
		else
		{
			snmp_log(LOG_ERR, "ERROR: New row is not created.\n");
			/* error */
			return 1;
		}
		row = NULL;
    	i++;
    }

    /* success */
    return 0;
}

/* create a new row in the table */
netsnmp_tdata_row *
ieee8021QBridgeVlanStaticTable_createEntry(netsnmp_tdata *table_data
                 , u_long  ieee8021QBridgeVlanStaticComponentId
                 , u_long  ieee8021QBridgeVlanStaticVlanIndex
                ) {
    struct ieee8021QBridgeVlanStaticTable_entry *entry;
    netsnmp_tdata_row *row;

    entry = SNMP_MALLOC_TYPEDEF(struct ieee8021QBridgeVlanStaticTable_entry);
    if (!entry)
        return NULL;

    row = netsnmp_tdata_create_row();
    if (!row) {
        SNMP_FREE(entry);
        return NULL;
    }
    row->data = entry;

    //DEBUGMSGT(("ieee8021QBridgeVlanStaticTable:entry:create", "row 0x%x\n", (uintptr_t)row));
    snmp_log(LOG_INFO,"ieee8021BridgeBasePortTable_createEntry - (%ld) - (%ld)\n",
    		ieee8021QBridgeVlanStaticComponentId, ieee8021QBridgeVlanStaticVlanIndex);

    entry->ieee8021QBridgeVlanStaticComponentId = ieee8021QBridgeVlanStaticComponentId;
    netsnmp_tdata_row_add_index( row, ASN_UNSIGNED,
                                 &(entry->ieee8021QBridgeVlanStaticComponentId),
                                 sizeof(entry->ieee8021QBridgeVlanStaticComponentId));
    entry->ieee8021QBridgeVlanStaticVlanIndex = ieee8021QBridgeVlanStaticVlanIndex;
    netsnmp_tdata_row_add_index( row, ASN_UNSIGNED,
                                 &(entry->ieee8021QBridgeVlanStaticVlanIndex),
                                 sizeof(entry->ieee8021QBridgeVlanStaticVlanIndex));
    if (table_data)
        netsnmp_tdata_add_row( table_data, row );
    return row;
}

/* fill a new row data in the table */
int ieee8021QBridgeVlanStaticTable_fillEntry(char bridge_name[MAX_STR_LEN], ieee8021QBridgeVlanStaticTable_entry_t *entry)
{
	char tmp_vid[MAX_STR_LEN] = {0};
	char result_array[MAX_NUM_OF_INTERFACES][MAX_STR_LEN];
	char ports_name[MAX_NUM_OF_INTERFACES][MAX_STR_LEN];
	int i = 0;

    snmp_log(LOG_DEBUG, "called: ieee8021QBridgeVlanStaticTable_fillEntry\n");

    if (!entry)
    {
        snmp_log(LOG_ERR, "ieee8021QBridgeVlanStaticTable_fillEntry:entry == NULL\n");
        /* error */
        return 1;
    }

    snprintf(tmp_vid, MAX_STR_LEN, "%lu", entry->ieee8021QBridgeVlanStaticVlanIndex);

    strncpy(entry->ieee8021QBridgeVlanStaticName, bridge_name, MAX_STR_LEN);
    entry->ieee8021QBridgeVlanStaticName_len = strlen(entry->ieee8021QBridgeVlanStaticName);

    if(EXIT_FAILURE == get_bridge_port_names(ports_name, MAX_NUM_OF_INTERFACES, MAX_STR_LEN))
    {
    	snmp_log(LOG_ERR, "ERROR: get_bridge_port_names failed.\n");
        return 1;
    }

    while((i < MAX_NUM_OF_INTERFACES) && (0 != strncmp(ports_name[i], "", MAX_STR_LEN)))
    {
		if(EXIT_SUCCESS == get_egress_port_names(result_array, MAX_NUM_OF_INTERFACES, MAX_STR_LEN,
				ports_name[i], tmp_vid))
		{
		    strncpy(entry->ieee8021QBridgeVlanStaticEgressPorts, BASE_ONE_STR, sizeof(entry->ieee8021QBridgeVlanStaticEgressPorts));
		    entry->ieee8021QBridgeVlanStaticEgressPorts_len = sizeof(entry->ieee8021QBridgeVlanStaticEgressPorts);
		}
		else
		{
			strncpy(entry->ieee8021QBridgeVlanStaticEgressPorts, BASE_ZERO_STR, sizeof(entry->ieee8021QBridgeVlanStaticEgressPorts));
			entry->ieee8021QBridgeVlanStaticEgressPorts_len = sizeof(entry->ieee8021QBridgeVlanStaticEgressPorts);
		}
    	i++;
    }
    i = 0;

    while((i < MAX_NUM_OF_INTERFACES) && (0 != strncmp(ports_name[i], "", MAX_STR_LEN)))
    {
		if(EXIT_SUCCESS == get_untagged_port_names(result_array, MAX_NUM_OF_INTERFACES, MAX_STR_LEN,
				ports_name[i], tmp_vid))
		{
		    strncpy(entry->ieee8021QBridgeVlanStaticUntaggedPorts, BASE_ONE_STR, sizeof(entry->ieee8021QBridgeVlanStaticUntaggedPorts));
		    entry->ieee8021QBridgeVlanStaticUntaggedPorts_len = sizeof(entry->ieee8021QBridgeVlanStaticUntaggedPorts);
		}
		else
		{
			strncpy(entry->ieee8021QBridgeVlanStaticUntaggedPorts, BASE_ZERO_STR, sizeof(entry->ieee8021QBridgeVlanStaticUntaggedPorts));
			entry->ieee8021QBridgeVlanStaticUntaggedPorts_len = sizeof(entry->ieee8021QBridgeVlanStaticUntaggedPorts);
		}
    	i++;
    }


    return 0;
}

/* remove a row from the table */
void
ieee8021QBridgeVlanStaticTable_removeEntry(netsnmp_tdata     *table_data, 
                 netsnmp_tdata_row *row) {
    struct ieee8021QBridgeVlanStaticTable_entry *entry;

    if (!row)
        return;    /* Nothing to remove */

    //DEBUGMSGT(("ieee8021QBridgeVlanStaticTable:entry:remove", "row 0x%x\n", (uintptr_t)row));

    entry = (struct ieee8021QBridgeVlanStaticTable_entry *)row->data;
    SNMP_FREE( entry );   /* XXX - release any other internal resources */

    if (table_data)
        netsnmp_tdata_remove_and_delete_row( table_data, row );
    else
        netsnmp_tdata_delete_row( row );    
}

/* Example cache handling - set up table_data list from a suitable file */
int
ieee8021QBridgeVlanStaticTable_load( netsnmp_cache *cache, void *vmagic ) {
    /* will be used to iterate trough ieee8021BridgeBaseTable_head */
	netsnmp_tdata_row *agent_row = NULL;
	/* helper to keep next agent row */
	netsnmp_tdata_row *next_agent_row = NULL;
	/* will be used to check existence of the bridge */
	netsnmp_tdata *table = (netsnmp_tdata *)vmagic;

	ieee8021QBridgeVlanStaticTable_entry_t *entry = NULL;
	int vid_found = 0;
	char bridge_names[MAX_NUM_OF_INTERFACES][MAX_STR_LEN];
	char bridge_vids[BASE_MAX_VID][BASE_FIVE];
	u_long component_id = 1;
	int i = 0;

	snmp_log(LOG_DEBUG, "called: ieee8021QBridgeVlanStaticTable_load\n");

	/* gather fresh system data */
    if(EXIT_FAILURE == get_bridge_names(bridge_names, MAX_NUM_OF_INTERFACES, MAX_STR_LEN))
    {
    	snmp_log(LOG_ERR, "ERROR: get_bridge_names failed.\n");
        return 1;
    }
    //0 is for uncompressed vlan ids
    if (EXIT_FAILURE == get_bridge_vlans(bridge_vids, BASE_MAX_VID, BASE_FIVE, 0))
    {
        return 1;
    }

	/* returns the first row in the table */
	agent_row = netsnmp_tdata_row_first (table);

	while(NULL != agent_row)
	{
		entry = (ieee8021QBridgeVlanStaticTable_entry_t *)agent_row->data;

		if(NULL != entry)
		{
			/* there is no more agent_row, erase it from agent */
			if(!entry->ieee8021QBridgeVlanStaticComponentId || !entry->ieee8021QBridgeVlanStaticVlanIndex)
			{
				snmp_log(LOG_DEBUG, "there is no more agent_row, erase it from agent\n");
				/* returns next row */
				next_agent_row = netsnmp_tdata_row_next (table, agent_row);
				ieee8021QBridgeVlanStaticTable_removeEntry(table, agent_row);
				agent_row = next_agent_row;
				next_agent_row = NULL;
			}
			/* agent_row exist on system, skip it (leave it on agent) */
			else
			{
				snmp_log(LOG_DEBUG, "agent_row exist on system, skip it\n");
				/* refresh data in this existing row on agent */
				ieee8021QBridgeVlanStaticTable_fillEntry(bridge_names[0],
						(ieee8021QBridgeVlanStaticTable_entry_t *)agent_row->data);
				i++;
				/* take next */
				agent_row = netsnmp_tdata_row_next (table, agent_row);
			}
		}
		else
		{
			snmp_log(LOG_ERR, "error: agent_row->data is NULL\n");
		}
	}

	i = 0;
	while((i < MAX_NUM_OF_INTERFACES) && (0 != strncmp(bridge_vids[i], "", MAX_STR_LEN)))
	{
		snmp_log(LOG_DEBUG, "load: looking for vid: %s\n", bridge_vids[i]);
		/* returns the first row in the table */
		agent_row = netsnmp_tdata_row_first (table);

		while((NULL != agent_row) && (vid_found == 0))
		{
			entry = (ieee8021QBridgeVlanStaticTable_entry_t *)agent_row->data;

			if( (entry->ieee8021QBridgeVlanStaticComponentId == component_id) &&
					(entry->ieee8021QBridgeVlanStaticVlanIndex == atoi(bridge_vids[i])) )
			{
				vid_found = 1;
			}

			agent_row = netsnmp_tdata_row_next (table, agent_row);
		}

		if(vid_found == 0)
		{
			snmp_log(LOG_DEBUG, "load: Adding new port: %s\n", bridge_vids[i]);
			agent_row = ieee8021QBridgeVlanStaticTable_createEntry(table, component_id, atoi(bridge_vids[i]));
			ieee8021QBridgeVlanStaticTable_fillEntry(bridge_names[0], agent_row->data);
		}
		i++;
		vid_found = 0;
	}


    return 0;  /* OK */
}

void
ieee8021QBridgeVlanStaticTable_free( netsnmp_cache *cache, void *vmagic ) {
    netsnmp_tdata     *table = (netsnmp_tdata *)vmagic;
    netsnmp_tdata_row *this;

    while ((this = netsnmp_tdata_row_first(table))) {
        netsnmp_tdata_remove_and_delete_row(table, this);
    }
}

/** handles requests for the ieee8021QBridgeVlanStaticTable table */
int
ieee8021QBridgeVlanStaticTable_handler(
    netsnmp_mib_handler               *handler,
    netsnmp_handler_registration      *reginfo,
    netsnmp_agent_request_info        *reqinfo,
    netsnmp_request_info              *requests) {

    netsnmp_request_info       *request;
    netsnmp_table_request_info *table_info;
    netsnmp_tdata              *table_data;
    netsnmp_tdata_row          *table_row;
    struct ieee8021QBridgeVlanStaticTable_entry          *table_entry;
    int                         ret;

    //DEBUGMSGTL(("ieee8021QBridgeVlanStaticTable:handler", "Processing request (%d)\n", reqinfo->mode));

    switch (reqinfo->mode) {
        /*
         * Read-support (also covers GetNext requests)
         */
    case MODE_GET:
        for (request=requests; request; request=request->next) {
            if (request->processed)
                continue;

            table_entry = (struct ieee8021QBridgeVlanStaticTable_entry *)
                              netsnmp_tdata_extract_entry(request);
            table_info  =     netsnmp_extract_table_info( request);
    
            switch (table_info->colnum) {
            case COLUMN_IEEE8021QBRIDGEVLANSTATICNAME:
                if ( !table_entry ) {
                    netsnmp_set_request_error(reqinfo, request,
                                              SNMP_NOSUCHINSTANCE);
                    continue;
                }
                snmp_set_var_typed_value( request->requestvb, ASN_OCTET_STR,
                                          table_entry->ieee8021QBridgeVlanStaticName,
                                          table_entry->ieee8021QBridgeVlanStaticName_len);
                break;
            case COLUMN_IEEE8021QBRIDGEVLANSTATICEGRESSPORTS:
                if ( !table_entry ) {
                    netsnmp_set_request_error(reqinfo, request,
                                              SNMP_NOSUCHINSTANCE);
                    continue;
                }
                snmp_set_var_typed_value( request->requestvb, ASN_OCTET_STR,
                                          table_entry->ieee8021QBridgeVlanStaticEgressPorts,
                                          table_entry->ieee8021QBridgeVlanStaticEgressPorts_len);
                break;
#if 0
            case COLUMN_IEEE8021QBRIDGEVLANFORBIDDENEGRESSPORTS:
                if ( !table_entry ) {
                    netsnmp_set_request_error(reqinfo, request,
                                              SNMP_NOSUCHINSTANCE);
                    continue;
                }
                snmp_set_var_typed_value( request->requestvb, ASN_OCTET_STR,
                                          table_entry->ieee8021QBridgeVlanForbiddenEgressPorts,
                                          table_entry->ieee8021QBridgeVlanForbiddenEgressPorts_len);
                break;
#endif
            case COLUMN_IEEE8021QBRIDGEVLANSTATICUNTAGGEDPORTS:
                if ( !table_entry ) {
                    netsnmp_set_request_error(reqinfo, request,
                                              SNMP_NOSUCHINSTANCE);
                    continue;
                }
                snmp_set_var_typed_value( request->requestvb, ASN_OCTET_STR,
                                          table_entry->ieee8021QBridgeVlanStaticUntaggedPorts,
                                          table_entry->ieee8021QBridgeVlanStaticUntaggedPorts_len);
                break;
#if 0
            case COLUMN_IEEE8021QBRIDGEVLANSTATICROWSTATUS:
                if ( !table_entry ) {
                    netsnmp_set_request_error(reqinfo, request,
                                              SNMP_NOSUCHINSTANCE);
                    continue;
                }
                snmp_set_var_typed_integer( request->requestvb, ASN_INTEGER,
                                            table_entry->ieee8021QBridgeVlanStaticRowStatus);
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
            table_entry = (struct ieee8021QBridgeVlanStaticTable_entry *)
                              netsnmp_tdata_extract_entry(request);
#endif
            table_info  =     netsnmp_extract_table_info( request);
    
            switch (table_info->colnum) {
#if 0
            case COLUMN_IEEE8021QBRIDGEVLANSTATICNAME:
	        /* or possibly 'netsnmp_check_vb_type_and_size' */
                ret = netsnmp_check_vb_type_and_max_size(
                          request->requestvb, ASN_OCTET_STR, sizeof(table_entry->ieee8021QBridgeVlanStaticName));
                if ( ret != SNMP_ERR_NOERROR ) {
                    netsnmp_set_request_error( reqinfo, request, ret );
                    return SNMP_ERR_NOERROR;
                }
                break;
            case COLUMN_IEEE8021QBRIDGEVLANSTATICEGRESSPORTS:
	        /* or possibly 'netsnmp_check_vb_type_and_size' */
                ret = netsnmp_check_vb_type_and_max_size(
                          request->requestvb, ASN_OCTET_STR, sizeof(table_entry->ieee8021QBridgeVlanStaticEgressPorts));
                if ( ret != SNMP_ERR_NOERROR ) {
                    netsnmp_set_request_error( reqinfo, request, ret );
                    return SNMP_ERR_NOERROR;
                }
                break;
            case COLUMN_IEEE8021QBRIDGEVLANFORBIDDENEGRESSPORTS:
	        /* or possibly 'netsnmp_check_vb_type_and_size' */
                ret = netsnmp_check_vb_type_and_max_size(
                          request->requestvb, ASN_OCTET_STR, sizeof(table_entry->ieee8021QBridgeVlanForbiddenEgressPorts));
                if ( ret != SNMP_ERR_NOERROR ) {
                    netsnmp_set_request_error( reqinfo, request, ret );
                    return SNMP_ERR_NOERROR;
                }
                break;
            case COLUMN_IEEE8021QBRIDGEVLANSTATICUNTAGGEDPORTS:
	        /* or possibly 'netsnmp_check_vb_type_and_size' */
                ret = netsnmp_check_vb_type_and_max_size(
                          request->requestvb, ASN_OCTET_STR, sizeof(table_entry->ieee8021QBridgeVlanStaticUntaggedPorts));
                if ( ret != SNMP_ERR_NOERROR ) {
                    netsnmp_set_request_error( reqinfo, request, ret );
                    return SNMP_ERR_NOERROR;
                }
                break;
            case COLUMN_IEEE8021QBRIDGEVLANSTATICROWSTATUS:
                ret = netsnmp_check_vb_rowstatus(request->requestvb,
                         (table_entry ? RS_ACTIVE : RS_NONEXISTENT ));
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
#if 0
        for (request=requests; request; request=request->next) {
            if (request->processed)
                continue;

            table_row  = netsnmp_tdata_extract_row(  request);
            table_data = netsnmp_tdata_extract_table(request);
            table_info = netsnmp_extract_table_info( request);
    
            switch (table_info->colnum) {
            case COLUMN_IEEE8021QBRIDGEVLANSTATICROWSTATUS:
                switch (*request->requestvb->val.integer) {
                case RS_CREATEANDGO:
                case RS_CREATEANDWAIT:
                    table_row = ieee8021QBridgeVlanStaticTable_createEntry(table_data
                        , *table_info->indexes->val.integer
                        , *table_info->indexes->val.integer
                        );
                    if (table_row) {
                        netsnmp_insert_tdata_row( request, table_row );
                    } else {
                        netsnmp_set_request_error( reqinfo, request,
                                                   SNMP_ERR_RESOURCEUNAVAILABLE );
                        return SNMP_ERR_NOERROR;
                    }
                }
            }
        }
#endif
        break;

    case MODE_SET_FREE:
        for (request=requests; request; request=request->next) {
            if (request->processed)
                continue;

            table_entry = (struct ieee8021QBridgeVlanStaticTable_entry *)
                              netsnmp_tdata_extract_entry(request);
            table_row   =     netsnmp_tdata_extract_row(  request);
            table_data  =     netsnmp_tdata_extract_table(request);
            table_info  =     netsnmp_extract_table_info( request);
    
            switch (table_info->colnum) {
            case COLUMN_IEEE8021QBRIDGEVLANSTATICROWSTATUS:
                switch (*request->requestvb->val.integer) {
                case RS_CREATEANDGO:
                case RS_CREATEANDWAIT:
                    if (table_entry && !table_entry->valid) {
                        ieee8021QBridgeVlanStaticTable_removeEntry(table_data, table_row );
                    }
                }
            }
        }
        break;

    case MODE_SET_ACTION:
#if 0
        for (request=requests; request; request=request->next) {
            if (request->processed)
                continue;
            table_entry = (struct ieee8021QBridgeVlanStaticTable_entry *)
                              netsnmp_tdata_extract_entry(request);
            table_info  =     netsnmp_extract_table_info( request);
            switch (table_info->colnum) {
            case COLUMN_IEEE8021QBRIDGEVLANSTATICNAME:
                memcpy( table_entry->old_ieee8021QBridgeVlanStaticName,
                        table_entry->ieee8021QBridgeVlanStaticName,
                        sizeof(table_entry->ieee8021QBridgeVlanStaticName));
                table_entry->old_ieee8021QBridgeVlanStaticName_len =
                        table_entry->ieee8021QBridgeVlanStaticName_len;
                memset( table_entry->ieee8021QBridgeVlanStaticName, 0,
                        sizeof(table_entry->ieee8021QBridgeVlanStaticName));
                memcpy( table_entry->ieee8021QBridgeVlanStaticName,
                        request->requestvb->val.string,
                        request->requestvb->val_len);
                table_entry->ieee8021QBridgeVlanStaticName_len =
                        request->requestvb->val_len;
                break;
            case COLUMN_IEEE8021QBRIDGEVLANSTATICEGRESSPORTS:
                memcpy( table_entry->old_ieee8021QBridgeVlanStaticEgressPorts,
                        table_entry->ieee8021QBridgeVlanStaticEgressPorts,
                        sizeof(table_entry->ieee8021QBridgeVlanStaticEgressPorts));
                table_entry->old_ieee8021QBridgeVlanStaticEgressPorts_len =
                        table_entry->ieee8021QBridgeVlanStaticEgressPorts_len;
                memset( table_entry->ieee8021QBridgeVlanStaticEgressPorts, 0,
                        sizeof(table_entry->ieee8021QBridgeVlanStaticEgressPorts));
                memcpy( table_entry->ieee8021QBridgeVlanStaticEgressPorts,
                        request->requestvb->val.string,
                        request->requestvb->val_len);
                table_entry->ieee8021QBridgeVlanStaticEgressPorts_len =
                        request->requestvb->val_len;
                break;
            case COLUMN_IEEE8021QBRIDGEVLANFORBIDDENEGRESSPORTS:
                memcpy( table_entry->old_ieee8021QBridgeVlanForbiddenEgressPorts,
                        table_entry->ieee8021QBridgeVlanForbiddenEgressPorts,
                        sizeof(table_entry->ieee8021QBridgeVlanForbiddenEgressPorts));
                table_entry->old_ieee8021QBridgeVlanForbiddenEgressPorts_len =
                        table_entry->ieee8021QBridgeVlanForbiddenEgressPorts_len;
                memset( table_entry->ieee8021QBridgeVlanForbiddenEgressPorts, 0,
                        sizeof(table_entry->ieee8021QBridgeVlanForbiddenEgressPorts));
                memcpy( table_entry->ieee8021QBridgeVlanForbiddenEgressPorts,
                        request->requestvb->val.string,
                        request->requestvb->val_len);
                table_entry->ieee8021QBridgeVlanForbiddenEgressPorts_len =
                        request->requestvb->val_len;
                break;
            case COLUMN_IEEE8021QBRIDGEVLANSTATICUNTAGGEDPORTS:
                memcpy( table_entry->old_ieee8021QBridgeVlanStaticUntaggedPorts,
                        table_entry->ieee8021QBridgeVlanStaticUntaggedPorts,
                        sizeof(table_entry->ieee8021QBridgeVlanStaticUntaggedPorts));
                table_entry->old_ieee8021QBridgeVlanStaticUntaggedPorts_len =
                        table_entry->ieee8021QBridgeVlanStaticUntaggedPorts_len;
                memset( table_entry->ieee8021QBridgeVlanStaticUntaggedPorts, 0,
                        sizeof(table_entry->ieee8021QBridgeVlanStaticUntaggedPorts));
                memcpy( table_entry->ieee8021QBridgeVlanStaticUntaggedPorts,
                        request->requestvb->val.string,
                        request->requestvb->val_len);
                table_entry->ieee8021QBridgeVlanStaticUntaggedPorts_len =
                        request->requestvb->val_len;
                break;
            }
        }

        /* Check the internal consistency of an active row */
        for (request=requests; request; request=request->next) {
            table_entry = (struct ieee8021QBridgeVlanStaticTable_entry *)
                              netsnmp_tdata_extract_entry(request);
            table_info  =     netsnmp_extract_table_info( request);
    
            switch (table_info->colnum) {
            case COLUMN_IEEE8021QBRIDGEVLANSTATICROWSTATUS:
                switch (*request->requestvb->val.integer) {
                case RS_ACTIVE:
                case RS_CREATEANDGO:
                    if (/* XXX */) {
                        netsnmp_set_request_error( reqinfo, request,
                                                   SNMP_ERR_INCONSISTENTVALUE );
                        return SNMP_ERR_NOERROR;
                    }
                }
            }
        }
#endif
        break;

    case MODE_SET_UNDO:
#if 0
        for (request=requests; request; request=request->next) {
            if (request->processed)
                continue;

            table_entry = (struct ieee8021QBridgeVlanStaticTable_entry *)
                              netsnmp_tdata_extract_entry(request);
            table_row   =     netsnmp_tdata_extract_row(  request);
            table_data  =     netsnmp_tdata_extract_table(request);
            table_info  =     netsnmp_extract_table_info( request);
    
            switch (table_info->colnum) {
            case COLUMN_IEEE8021QBRIDGEVLANSTATICNAME:
                memcpy( table_entry->ieee8021QBridgeVlanStaticName,
                        table_entry->old_ieee8021QBridgeVlanStaticName,
                        sizeof(table_entry->ieee8021QBridgeVlanStaticName));
                memset( table_entry->old_ieee8021QBridgeVlanStaticName, 0,
                        sizeof(table_entry->ieee8021QBridgeVlanStaticName));
                table_entry->ieee8021QBridgeVlanStaticName_len =
                        table_entry->old_ieee8021QBridgeVlanStaticName_len;
                break;
            case COLUMN_IEEE8021QBRIDGEVLANSTATICEGRESSPORTS:
                memcpy( table_entry->ieee8021QBridgeVlanStaticEgressPorts,
                        table_entry->old_ieee8021QBridgeVlanStaticEgressPorts,
                        sizeof(table_entry->ieee8021QBridgeVlanStaticEgressPorts));
                memset( table_entry->old_ieee8021QBridgeVlanStaticEgressPorts, 0,
                        sizeof(table_entry->ieee8021QBridgeVlanStaticEgressPorts));
                table_entry->ieee8021QBridgeVlanStaticEgressPorts_len =
                        table_entry->old_ieee8021QBridgeVlanStaticEgressPorts_len;
                break;
            case COLUMN_IEEE8021QBRIDGEVLANFORBIDDENEGRESSPORTS:
                memcpy( table_entry->ieee8021QBridgeVlanForbiddenEgressPorts,
                        table_entry->old_ieee8021QBridgeVlanForbiddenEgressPorts,
                        sizeof(table_entry->ieee8021QBridgeVlanForbiddenEgressPorts));
                memset( table_entry->old_ieee8021QBridgeVlanForbiddenEgressPorts, 0,
                        sizeof(table_entry->ieee8021QBridgeVlanForbiddenEgressPorts));
                table_entry->ieee8021QBridgeVlanForbiddenEgressPorts_len =
                        table_entry->old_ieee8021QBridgeVlanForbiddenEgressPorts_len;
                break;
            case COLUMN_IEEE8021QBRIDGEVLANSTATICUNTAGGEDPORTS:
                memcpy( table_entry->ieee8021QBridgeVlanStaticUntaggedPorts,
                        table_entry->old_ieee8021QBridgeVlanStaticUntaggedPorts,
                        sizeof(table_entry->ieee8021QBridgeVlanStaticUntaggedPorts));
                memset( table_entry->old_ieee8021QBridgeVlanStaticUntaggedPorts, 0,
                        sizeof(table_entry->ieee8021QBridgeVlanStaticUntaggedPorts));
                table_entry->ieee8021QBridgeVlanStaticUntaggedPorts_len =
                        table_entry->old_ieee8021QBridgeVlanStaticUntaggedPorts_len;
                break;
            case COLUMN_IEEE8021QBRIDGEVLANSTATICROWSTATUS:
                switch (*request->requestvb->val.integer) {
                case RS_CREATEANDGO:
                case RS_CREATEANDWAIT:
                    if (table_entry && !table_entry->valid) {
                        ieee8021QBridgeVlanStaticTable_removeEntry(table_data, table_row );
                    }
                }
                break;
            }
        }
#endif
        break;

    case MODE_SET_COMMIT:
#if 0
        for (request=requests; request; request=request->next) {
            if (request->processed)
                continue;

            table_entry = (struct ieee8021QBridgeVlanStaticTable_entry *)
                              netsnmp_tdata_extract_entry(request);
            table_row   =     netsnmp_tdata_extract_row(  request);
            table_data  =     netsnmp_tdata_extract_table(request);
            table_info  =     netsnmp_extract_table_info(    request);
    
            switch (table_info->colnum) {
            case COLUMN_IEEE8021QBRIDGEVLANSTATICROWSTATUS:
                switch (*request->requestvb->val.integer) {
                case RS_CREATEANDGO:
                    table_entry->valid = 1;
                    /* Fall-through */
                case RS_ACTIVE:
                    table_entry->ieee8021QBridgeVlanStaticRowStatus = RS_ACTIVE;
                    break;

                case RS_CREATEANDWAIT:
                    table_entry->valid = 1;
                    /* Fall-through */
                case RS_NOTINSERVICE:
                    table_entry->ieee8021QBridgeVlanStaticRowStatus = RS_NOTINSERVICE;
                    break;

                case RS_DESTROY:
                    ieee8021QBridgeVlanStaticTable_removeEntry(table_data, table_row );
                }
            }
        }
#endif
        break;
    }
    return SNMP_ERR_NOERROR;
}