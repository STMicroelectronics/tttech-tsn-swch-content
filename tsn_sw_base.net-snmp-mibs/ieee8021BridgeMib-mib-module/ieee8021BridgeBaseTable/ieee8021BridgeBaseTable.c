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
/*
 * Note: this file originally auto-generated by mib2c using
 *  $
 */

#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include "ieee8021BridgeBaseTable.h"
#include "../ieee8021BridgeMib_helpers/ieee8021BridgeMib_helpers.h"
#include "base_private_translators.h"
/* libbase includes */
#include "base_prints.h"
#include "base_linux_tool_helpers.h"
#include "base_q_bridge_linux_tools.h"
/* libtsn includes */
#include "tsn.h"
#include "tsn_bridge.h"

/** Initializes the ieee8021BridgeBaseTable module */
void init_ieee8021BridgeBaseTable(void)
{
  /* here we initialize all the tables we're planning on supporting */
    initialize_table_ieee8021BridgeBaseTable();
}

/** Initialize the ieee8021BridgeBaseTable table by defining its contents and how it's structured */
void initialize_table_ieee8021BridgeBaseTable(void)
{
    const oid ieee8021BridgeBaseTable_oid[] = {1,3,111,2,802,1,1,2,1,1,1};
    const size_t ieee8021BridgeBaseTable_oid_len   = OID_LENGTH(ieee8021BridgeBaseTable_oid);
    netsnmp_handler_registration    *reg;
    netsnmp_tdata                   *table_data;
    netsnmp_table_registration_info *table_info;
    netsnmp_cache                   *cache;

    DEBUGMSGTL(("ieee8021BridgeBaseTable:init", "initializing table ieee8021BridgeBaseTable\n"));

    reg = netsnmp_create_handler_registration(
              "ieee8021BridgeBaseTable",     ieee8021BridgeBaseTable_handler,
              ieee8021BridgeBaseTable_oid, ieee8021BridgeBaseTable_oid_len,
              HANDLER_CAN_RWRITE
              );

    table_data = netsnmp_tdata_create_table( "ieee8021BridgeBaseTable", 0 );
    if (NULL == table_data) {
        snmp_log(LOG_ERR,"error creating tdata table for ieee8021BridgeBaseTable\n");
        return;
    }
    cache = netsnmp_cache_create(IEEE8021BRIDGEBASETABLE_TIMEOUT,
                                  ieee8021BridgeBaseTable_load, ieee8021BridgeBaseTable_free,
                                  ieee8021BridgeBaseTable_oid, ieee8021BridgeBaseTable_oid_len);
    if (NULL == cache) {
        snmp_log(LOG_ERR,"error creating cache for ieee8021BridgeBaseTable\n");
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
        snmp_log(LOG_ERR,"error creating table info for ieee8021BridgeBaseTable\n");
        return;
    }
    netsnmp_table_helper_add_indexes(table_info,
                           ASN_UNSIGNED,  /* index: ieee8021BridgeBaseComponentId */
                           0);

    table_info->min_column = COLUMN_IEEE8021BRIDGEBASEBRIDGEADDRESS;
    table_info->max_column = COLUMN_IEEE8021BRIDGEBASEROWSTATUS;
    
    netsnmp_tdata_register( reg, table_data, table_info );

    /* Initialise the contents of the table here */
    if(0 != init_ieee8021BridgeBaseTable_with_sys_data(table_data))
    {
        snmp_log(LOG_ERR,"ERROR: ieee8021BridgeBaseTable initialization was unsuccessful.\n");
        free(table_info);
        return;
    }
    else
    {
        snmp_log(LOG_INFO,"INFO: ieee8021BridgeBaseTable initialization was successful.\n");
        netsnmp_cache_timer_start(cache);
    }
}

/* init table */
int init_ieee8021BridgeBaseTable_with_sys_data(netsnmp_tdata *table_data)
{
    netsnmp_tdata_row *row = NULL;
    char bridge_names[MAX_NUM_OF_INTERFACES][MAX_STR_LEN];
    u_long component_id = 1;
    int i = 0;

    snmp_log(LOG_INFO, "INFO: init_ieee8021BridgeBaseTable_with_sys_data called.\n");

    if(EXIT_FAILURE == get_bridge_names(bridge_names, MAX_NUM_OF_INTERFACES, MAX_STR_LEN))
    {
    	snmp_log(LOG_ERR, "ERROR: get_bridge_names failed.\n");
        return 1;
    }

    while((i < MAX_NUM_OF_INTERFACES) && (0 != strncmp(bridge_names[i], "", MAX_STR_LEN)))
    {
		row = ieee8021BridgeBaseTable_createEntry(table_data, component_id);
		if(NULL != row)
		{
			ieee8021BridgeBaseTable_fillEntry(bridge_names[i], (ieee8021BridgeBaseTable_entry_t*)row->data);
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
netsnmp_tdata_row *ieee8021BridgeBaseTable_createEntry(netsnmp_tdata *table_data,
		u_long ieee8021BridgeBaseComponentId)
{
    struct ieee8021BridgeBaseTable_entry *entry = NULL;
    netsnmp_tdata_row *row = NULL;

    entry = SNMP_MALLOC_TYPEDEF(struct ieee8021BridgeBaseTable_entry);
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

    snmp_log(LOG_INFO,"ieee8021BridgeBaseTable_createEntry - (%ld) \n", ieee8021BridgeBaseComponentId);
    entry->ieee8021BridgeBaseComponentId = ieee8021BridgeBaseComponentId;
    netsnmp_tdata_row_add_index(row, ASN_UNSIGNED, &(entry->ieee8021BridgeBaseComponentId),
    		sizeof(entry->ieee8021BridgeBaseComponentId));
    if (table_data)
    {
        netsnmp_tdata_add_row( table_data, row );
    }

    return row;
}
/* fill a new row data in the table */
int ieee8021BridgeBaseTable_fillEntry(char bridge_name[MAX_STR_LEN], ieee8021BridgeBaseTable_entry_t *entry)
{
    char result_array[MAX_NUM_OF_INTERFACES][MAX_STR_LEN];
    char tmp_result_array[MAX_NUM_OF_INTERFACES][MAX_STR_LEN];
    long int pos = 0;
    int i = 0;
	struct port_hdl *port = NULL;
	struct bridge_attr_and_caps caps = {0};

    snmp_log(LOG_DEBUG, "called: ieee8021BridgeBaseTable_fillEntry\n");

    if (!entry)
    {
        snmp_log(LOG_ERR, "ieee8021BridgeBaseTable_fillEntry:entry == NULL\n");
        /* error */
        return 1;
    }

    /* ieee8021BridgeBaseNumPorts and ieee8021BridgeBaseTrafficClassesEnabled*/
	port = tsn_port_open_by_name(bridge_name);/* open port */
	if(NULL != port)
	{
		if(tsn_bridge_attr(port, &caps))
		{
			snmp_log(LOG_ERR, "ieee8021BridgeBaseTable_fillEntry: port's attributes are unreachable\n");
			tsn_port_close(port);
			return 1; /* error */
		}
		entry->ieee8021BridgeBaseNumPorts = caps.BridgeNumPorts;

		/* ieee8021BridgeBaseTrafficClassesEnabled default value is false(2) */
		entry->ieee8021BridgeBaseTrafficClassesEnabled = 2;
		/* if caps.BridgeTrafficClassesEnabled is true, the ieee8021BridgeBaseTrafficClassesEnabled should be true(1) */
		if(caps.BridgeTrafficClassesEnabled)
		{
			entry->ieee8021BridgeBaseTrafficClassesEnabled = 1;
		}
		/* ieee8021BridgeBaseComponentType */
		entry->ieee8021BridgeBaseComponentType = caps.BridgeType;
		tsn_port_close(port);
	}
	else
	{
		snmp_log(LOG_ERR, "ieee8021BridgeBasePortTable_fillEntry: port handler is invalid\n");
		return 1; /* error */
	}

    if(EXIT_SUCCESS == get_interface_address(result_array, MAX_NUM_OF_INTERFACES, MAX_STR_LEN, bridge_name))
    {
		memcpy(tmp_result_array[0], result_array[0], MAX_STR_LEN);
		while((i < 6) && (0 != strncmp(tmp_result_array[0], "", MAX_STR_LEN)))
		{
			pos = strcspn(tmp_result_array[0], ":");//return position of ":"
			*(tmp_result_array[0] + pos) = '\0';//break string on that position
			entry->ieee8021BridgeBaseBridgeAddress[i] = strtol(tmp_result_array[0], NULL, 16);//take first octet
			i++;//go to next octet
			memcpy(tmp_result_array[0], (result_array[0] + pos + 1), MAX_STR_LEN);//take string after identified ":"
		}
    	entry->ieee8021BridgeBaseBridgeAddress_len = sizeof(entry->ieee8021BridgeBaseBridgeAddress);
    }

    return 0;
}

/* remove a row from the table */
void ieee8021BridgeBaseTable_removeEntry(netsnmp_tdata *table_data, netsnmp_tdata_row *row)
{
    struct ieee8021BridgeBaseTable_entry *entry = NULL;

    if (!row)
        return;    /* Nothing to remove */

     snmp_log(LOG_INFO, "ieee8021BridgeBaseTable_removeEntry entry.\n");

    entry = (struct ieee8021BridgeBaseTable_entry *)row->data;
    SNMP_FREE(entry);   /* XXX - release any other internal resources */

    if (table_data)
        netsnmp_tdata_remove_and_delete_row( table_data, row );
    else
        netsnmp_tdata_delete_row( row );    
}
/* Example cache handling - set up table_data list from a suitable file */int
ieee8021BridgeBaseTable_load( netsnmp_cache *cache, void *vmagic ) {
    /* will be used to iterate trough ieee8021BridgeBaseTable_head */
    netsnmp_tdata_row *agent_row = NULL;
    /* helper to keep next agent row */
    netsnmp_tdata_row *next_agent_row = NULL;
    /* will be used to check existence of the bridge */
    netsnmp_tdata *table = (netsnmp_tdata *)vmagic;
    ieee8021BridgeBaseTable_entry_t *entry = NULL;
    int bridge_found = 0;
    char bridge_names[MAX_NUM_OF_INTERFACES][MAX_STR_LEN];
    u_long component_id = 1;
    int i = 0;

    snmp_log(LOG_DEBUG, "called: ieee8021BridgeBaseTable_load\n");

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
        entry = (ieee8021BridgeBaseTable_entry_t *)agent_row->data;

        if(NULL != entry)
        {
            /* check if this entry (bridge) exist on system*/
            /* there is no more agent_row, erase it from agent */
            if(!entry->ieee8021BridgeBaseComponentId)
            {
                snmp_log(LOG_DEBUG, "there is no more agent_row, erase it from agent\n");
                /* returns next row */
                next_agent_row = netsnmp_tdata_row_next (table, agent_row);
                ieee8021BridgeBaseTable_removeEntry(table, agent_row);
                agent_row = next_agent_row;
                next_agent_row = NULL;
            }
            /* agent_row exist on system, skip it (leave it on agent) */
            else
            {
                snmp_log(LOG_DEBUG, "agent_row exist on system, skip it\n");
                /* refresh data in this existing row on agent */
                ieee8021BridgeBaseTable_fillEntry(bridge_names[0], (ieee8021BridgeBaseTable_entry_t *)agent_row->data);
                /* take next */
                agent_row = netsnmp_tdata_row_next (table, agent_row);
            }
        }
        else
        {
            snmp_log(LOG_ERR, "error: agent_row->data is NULL\n");
        }
    }

    while((i < MAX_NUM_OF_INTERFACES) && (0 != strncmp(bridge_names[i], "", MAX_STR_LEN)))
    {
        snmp_log(LOG_DEBUG, "load: looking for bridge: %s\n", bridge_names[i]);
        /* returns the first row in the table */
        agent_row = netsnmp_tdata_row_first (table);

        while((NULL != agent_row) && (bridge_found == 0))
        {
            entry = (ieee8021BridgeBaseTable_entry_t *)agent_row->data;

            if((entry->ieee8021BridgeBaseComponentId == component_id))
            {
                bridge_found = 1;
            }

            agent_row = netsnmp_tdata_row_next (table, agent_row);
        }

        if(bridge_found == 0)
        {
            snmp_log(LOG_DEBUG, "load: Adding new bridge: %s\n", bridge_names[i]);
            agent_row = ieee8021BridgeBaseTable_createEntry(table, component_id);
            ieee8021BridgeBaseTable_fillEntry(bridge_names[i], agent_row->data);
        }
        component_id++;
        i++;
        bridge_found = 0;
    }
    /* OK */
    return 0;
}
void ieee8021BridgeBaseTable_free( netsnmp_cache *cache, void *vmagic ) {
    netsnmp_tdata     *table = (netsnmp_tdata *)vmagic;
    netsnmp_tdata_row *this;

    while ((this = netsnmp_tdata_row_first(table))) {
        netsnmp_tdata_remove_and_delete_row(table, this);
    }
}

/** handles requests for the ieee8021BridgeBaseTable table */
int ieee8021BridgeBaseTable_handler(
    netsnmp_mib_handler               *handler,
    netsnmp_handler_registration      *reginfo,
    netsnmp_agent_request_info        *reqinfo,
    netsnmp_request_info              *requests) {

    netsnmp_request_info       *request;
    netsnmp_table_request_info *table_info;
    netsnmp_tdata              *table_data;
    netsnmp_tdata_row          *table_row;
    struct ieee8021BridgeBaseTable_entry          *table_entry;
    int                         ret;
    char tmp[MAX_STR_LEN];
    char bridge_names[MAX_NUM_OF_INTERFACES][MAX_STR_LEN];

    DEBUGMSGTL(("ieee8021BridgeBaseTable:handler", "Processing request (%d)\n", reqinfo->mode));

    switch (reqinfo->mode) {
        /*
         * Read-support (also covers GetNext requests)
         */
    case MODE_GET:
        for (request=requests; request; request=request->next) {
            if (request->processed)
                continue;

            table_entry = (struct ieee8021BridgeBaseTable_entry *)
                              netsnmp_tdata_extract_entry(request);
            table_info  =     netsnmp_extract_table_info( request);
    
            switch (table_info->colnum) {
            case COLUMN_IEEE8021BRIDGEBASEBRIDGEADDRESS:
                if ( !table_entry ) {
                    netsnmp_set_request_error(reqinfo, request,
                                              SNMP_NOSUCHINSTANCE);
                    continue;
                }
                snmp_set_var_typed_value( request->requestvb, ASN_OCTET_STR,
                                          table_entry->ieee8021BridgeBaseBridgeAddress,
                                          table_entry->ieee8021BridgeBaseBridgeAddress_len);
                break;
            case COLUMN_IEEE8021BRIDGEBASENUMPORTS:
                if ( !table_entry ) {
                    netsnmp_set_request_error(reqinfo, request,
                                              SNMP_NOSUCHINSTANCE);
                    continue;
                }
                snmp_set_var_typed_integer( request->requestvb, ASN_INTEGER,
                                            table_entry->ieee8021BridgeBaseNumPorts);
                break;
            case COLUMN_IEEE8021BRIDGEBASECOMPONENTTYPE:
                if ( !table_entry ) {
                    netsnmp_set_request_error(reqinfo, request,
                                              SNMP_NOSUCHINSTANCE);
                    continue;
                }
                snmp_set_var_typed_integer( request->requestvb, ASN_INTEGER,
                                            table_entry->ieee8021BridgeBaseComponentType);
                break;
#if 0
            case COLUMN_IEEE8021BRIDGEBASEDEVICECAPABILITIES:
                if ( !table_entry ) {
                    netsnmp_set_request_error(reqinfo, request,
                                              SNMP_NOSUCHINSTANCE);
                    continue;
                }
                snmp_set_var_typed_value( request->requestvb, ASN_OCTET_STR,
                                          table_entry->ieee8021BridgeBaseDeviceCapabilities,
                                          table_entry->ieee8021BridgeBaseDeviceCapabilities_len);
                break;
#endif
            case COLUMN_IEEE8021BRIDGEBASETRAFFICCLASSESENABLED:
                if ( !table_entry ) {
                    netsnmp_set_request_error(reqinfo, request,
                                              SNMP_NOSUCHINSTANCE);
                    continue;
                }
                snmp_set_var_typed_integer( request->requestvb, ASN_INTEGER,
                                            table_entry->ieee8021BridgeBaseTrafficClassesEnabled);
                break;
#if 0
            case COLUMN_IEEE8021BRIDGEBASEMMRPENABLEDSTATUS:
                if ( !table_entry ) {
                    netsnmp_set_request_error(reqinfo, request,
                                              SNMP_NOSUCHINSTANCE);
                    continue;
                }
                snmp_set_var_typed_integer( request->requestvb, ASN_INTEGER,
                                            table_entry->ieee8021BridgeBaseMmrpEnabledStatus);
                break;
            case COLUMN_IEEE8021BRIDGEBASEROWSTATUS:
                if ( !table_entry ) {
                    netsnmp_set_request_error(reqinfo, request,
                                              SNMP_NOSUCHINSTANCE);
                    continue;
                }
                snmp_set_var_typed_integer( request->requestvb, ASN_INTEGER,
                                            table_entry->ieee8021BridgeBaseRowStatus);
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

            table_entry = (struct ieee8021BridgeBaseTable_entry *)
                              netsnmp_tdata_extract_entry(request);
            table_info  =     netsnmp_extract_table_info( request);
    
            switch (table_info->colnum) {
            case COLUMN_IEEE8021BRIDGEBASEBRIDGEADDRESS:
            /* or possibly 'netsnmp_check_vb_type_and_size' */
                ret = netsnmp_check_vb_type_and_max_size(
                          request->requestvb, ASN_OCTET_STR, sizeof(table_entry->ieee8021BridgeBaseBridgeAddress));
                if ( ret != SNMP_ERR_NOERROR ) {
                    netsnmp_set_request_error( reqinfo, request, ret );
                    return SNMP_ERR_NOERROR;
                }
                break;
#if 0
            case COLUMN_IEEE8021BRIDGEBASECOMPONENTTYPE:
                /* or possibly 'netsnmp_check_vb_int_range' */
                ret = netsnmp_check_vb_int( request->requestvb );
                if ( ret != SNMP_ERR_NOERROR ) {
                    netsnmp_set_request_error( reqinfo, request, ret );
                    return SNMP_ERR_NOERROR;
                }
                break;
            case COLUMN_IEEE8021BRIDGEBASEDEVICECAPABILITIES:
            /* or possibly 'netsnmp_check_vb_type_and_size' */
                ret = netsnmp_check_vb_type_and_max_size(
                          request->requestvb, ASN_OCTET_STR, sizeof(table_entry->ieee8021BridgeBaseDeviceCapabilities));
                if ( ret != SNMP_ERR_NOERROR ) {
                    netsnmp_set_request_error( reqinfo, request, ret );
                    return SNMP_ERR_NOERROR;
                }
                break;
            case COLUMN_IEEE8021BRIDGEBASETRAFFICCLASSESENABLED:
                /* or possibly 'netsnmp_check_vb_int_range' */
                ret = netsnmp_check_vb_int( request->requestvb );
                if ( ret != SNMP_ERR_NOERROR ) {
                    netsnmp_set_request_error( reqinfo, request, ret );
                    return SNMP_ERR_NOERROR;
                }
                break;
            case COLUMN_IEEE8021BRIDGEBASEMMRPENABLEDSTATUS:
                /* or possibly 'netsnmp_check_vb_int_range' */
                ret = netsnmp_check_vb_int( request->requestvb );
                if ( ret != SNMP_ERR_NOERROR ) {
                    netsnmp_set_request_error( reqinfo, request, ret );
                    return SNMP_ERR_NOERROR;
                }
                break;
            case COLUMN_IEEE8021BRIDGEBASEROWSTATUS:
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
            case COLUMN_IEEE8021BRIDGEBASEROWSTATUS:
                switch (*request->requestvb->val.integer) {
                case RS_CREATEANDGO:
                case RS_CREATEANDWAIT:
                    table_row = ieee8021BridgeBaseTable_createEntry(table_data
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
        break;
#endif
    case MODE_SET_FREE:
#if 0  
        for (request=requests; request; request=request->next) {
            if (request->processed)
                continue;

            table_entry = (struct ieee8021BridgeBaseTable_entry *)
                              netsnmp_tdata_extract_entry(request);
            table_row   =     netsnmp_tdata_extract_row(  request);
            table_data  =     netsnmp_tdata_extract_table(request);
            table_info  =     netsnmp_extract_table_info( request);
    
            switch (table_info->colnum) {
            case COLUMN_IEEE8021BRIDGEBASEROWSTATUS:
                switch (*request->requestvb->val.integer) {
                case RS_CREATEANDGO:
                case RS_CREATEANDWAIT:
                    if (table_entry && !table_entry->valid) {
                        ieee8021BridgeBaseTable_removeEntry(table_data, table_row );
                    }
                }
            }
        }
        break;
#endif
    case MODE_SET_ACTION:
        for (request=requests; request; request=request->next) {
            if (request->processed)
                continue;

            table_entry = (struct ieee8021BridgeBaseTable_entry*)netsnmp_tdata_extract_entry(request);
            table_info  = netsnmp_extract_table_info( request);
    
            switch (table_info->colnum) {
            case COLUMN_IEEE8021BRIDGEBASEBRIDGEADDRESS:
                memcpy( table_entry->old_ieee8021BridgeBaseBridgeAddress,
                        table_entry->ieee8021BridgeBaseBridgeAddress,
                        sizeof(table_entry->ieee8021BridgeBaseBridgeAddress));
                table_entry->old_ieee8021BridgeBaseBridgeAddress_len =
                        table_entry->ieee8021BridgeBaseBridgeAddress_len;
                memset( table_entry->ieee8021BridgeBaseBridgeAddress, 0,
                        sizeof(table_entry->ieee8021BridgeBaseBridgeAddress));
                memcpy( table_entry->ieee8021BridgeBaseBridgeAddress,
                        request->requestvb->val.string,
                        request->requestvb->val_len);
                table_entry->ieee8021BridgeBaseBridgeAddress_len =
                        request->requestvb->val_len;

                snprintf(tmp, MAX_STR_LEN, "%02x:%02x:%02x:%02x:%02x:%02x",
                		table_entry->ieee8021BridgeBaseBridgeAddress[0],
						table_entry->ieee8021BridgeBaseBridgeAddress[1],
						table_entry->ieee8021BridgeBaseBridgeAddress[2],
						table_entry->ieee8021BridgeBaseBridgeAddress[3],
						table_entry->ieee8021BridgeBaseBridgeAddress[4],
						table_entry->ieee8021BridgeBaseBridgeAddress[5]);

                if(EXIT_FAILURE != get_bridge_names(bridge_names, MAX_NUM_OF_INTERFACES, MAX_STR_LEN))
                {
                	netsnmp_set_request_error(reqinfo, request, SNMP_ERR_GENERR);
                    return SNMP_ERR_NOERROR;
                }

                /* because there is only on bridge on system we will use bridge_names[0] */
                if(EXIT_FAILURE == set_interface_address(bridge_names[0], tmp))
                {
                	netsnmp_set_request_error(reqinfo, request, SNMP_ERR_WRONGVALUE);
                	return SNMP_ERR_NOERROR;
                }

                break;
#if 0
            case COLUMN_IEEE8021BRIDGEBASECOMPONENTTYPE:
                table_entry->old_ieee8021BridgeBaseComponentType = table_entry->ieee8021BridgeBaseComponentType;
                table_entry->ieee8021BridgeBaseComponentType     = *request->requestvb->val.integer;
                break;
            case COLUMN_IEEE8021BRIDGEBASEDEVICECAPABILITIES:
                memcpy( table_entry->old_ieee8021BridgeBaseDeviceCapabilities,
                        table_entry->ieee8021BridgeBaseDeviceCapabilities,
                        sizeof(table_entry->ieee8021BridgeBaseDeviceCapabilities));
                table_entry->old_ieee8021BridgeBaseDeviceCapabilities_len =
                        table_entry->ieee8021BridgeBaseDeviceCapabilities_len;
                memset( table_entry->ieee8021BridgeBaseDeviceCapabilities, 0,
                        sizeof(table_entry->ieee8021BridgeBaseDeviceCapabilities));
                memcpy( table_entry->ieee8021BridgeBaseDeviceCapabilities,
                        request->requestvb->val.string,
                        request->requestvb->val_len);
                table_entry->ieee8021BridgeBaseDeviceCapabilities_len =
                        request->requestvb->val_len;
                break;
            case COLUMN_IEEE8021BRIDGEBASETRAFFICCLASSESENABLED:
                table_entry->old_ieee8021BridgeBaseTrafficClassesEnabled = table_entry->ieee8021BridgeBaseTrafficClassesEnabled;
                table_entry->ieee8021BridgeBaseTrafficClassesEnabled     = *request->requestvb->val.integer;
                break;
            case COLUMN_IEEE8021BRIDGEBASEMMRPENABLEDSTATUS:
                table_entry->old_ieee8021BridgeBaseMmrpEnabledStatus = table_entry->ieee8021BridgeBaseMmrpEnabledStatus;
                table_entry->ieee8021BridgeBaseMmrpEnabledStatus     = *request->requestvb->val.integer;
                break;
#endif
            }
        }
        /* Check the internal consistency of an active row */
#if 0
        for (request=requests; request; request=request->next) {
            table_entry = (struct ieee8021BridgeBaseTable_entry *)
                              netsnmp_tdata_extract_entry(request);
            table_info  =     netsnmp_extract_table_info( request);
    
            switch (table_info->colnum) {
            case COLUMN_IEEE8021BRIDGEBASEROWSTATUS:
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
        for (request=requests; request; request=request->next) {
            if (request->processed)
                continue;

            table_entry = (struct ieee8021BridgeBaseTable_entry *)
                              netsnmp_tdata_extract_entry(request);
#if 0
            table_row   =     netsnmp_tdata_extract_row(  request);
            table_data  =     netsnmp_tdata_extract_table(request);
#endif
            table_info  =     netsnmp_extract_table_info( request);
    
            switch (table_info->colnum) {
            case COLUMN_IEEE8021BRIDGEBASEBRIDGEADDRESS:
                memcpy( table_entry->ieee8021BridgeBaseBridgeAddress,
                        table_entry->old_ieee8021BridgeBaseBridgeAddress,
                        sizeof(table_entry->ieee8021BridgeBaseBridgeAddress));
                memset( table_entry->old_ieee8021BridgeBaseBridgeAddress, 0,
                        sizeof(table_entry->ieee8021BridgeBaseBridgeAddress));
                table_entry->ieee8021BridgeBaseBridgeAddress_len =
                        table_entry->old_ieee8021BridgeBaseBridgeAddress_len;
                break;
#if 0
            case COLUMN_IEEE8021BRIDGEBASECOMPONENTTYPE:
                table_entry->ieee8021BridgeBaseComponentType     = table_entry->old_ieee8021BridgeBaseComponentType;
                table_entry->old_ieee8021BridgeBaseComponentType = 0;
                break;
            case COLUMN_IEEE8021BRIDGEBASEDEVICECAPABILITIES:
                memcpy( table_entry->ieee8021BridgeBaseDeviceCapabilities,
                        table_entry->old_ieee8021BridgeBaseDeviceCapabilities,
                        sizeof(table_entry->ieee8021BridgeBaseDeviceCapabilities));
                memset( table_entry->old_ieee8021BridgeBaseDeviceCapabilities, 0,
                        sizeof(table_entry->ieee8021BridgeBaseDeviceCapabilities));
                table_entry->ieee8021BridgeBaseDeviceCapabilities_len =
                        table_entry->old_ieee8021BridgeBaseDeviceCapabilities_len;
                break;
            case COLUMN_IEEE8021BRIDGEBASETRAFFICCLASSESENABLED:
                table_entry->ieee8021BridgeBaseTrafficClassesEnabled     = table_entry->old_ieee8021BridgeBaseTrafficClassesEnabled;
                table_entry->old_ieee8021BridgeBaseTrafficClassesEnabled = 0;
                break;
            case COLUMN_IEEE8021BRIDGEBASEMMRPENABLEDSTATUS:
                table_entry->ieee8021BridgeBaseMmrpEnabledStatus     = table_entry->old_ieee8021BridgeBaseMmrpEnabledStatus;
                table_entry->old_ieee8021BridgeBaseMmrpEnabledStatus = 0;
                break;
            case COLUMN_IEEE8021BRIDGEBASEROWSTATUS:
                switch (*request->requestvb->val.integer) {
                case RS_CREATEANDGO:
                case RS_CREATEANDWAIT:
                    if (table_entry && !table_entry->valid) {
                        ieee8021BridgeBaseTable_removeEntry(table_data, table_row );
                    }
                }
                break;
#endif
            }
        }
        break;
    case MODE_SET_COMMIT:
#if 0  
        for (request=requests; request; request=request->next) {
            if (request->processed)
                continue;

            table_entry = (struct ieee8021BridgeBaseTable_entry *)
                              netsnmp_tdata_extract_entry(request);
            table_row   =     netsnmp_tdata_extract_row(  request);
            table_data  =     netsnmp_tdata_extract_table(request);
            table_info  =     netsnmp_extract_table_info(    request);
    
            switch (table_info->colnum) {
            case COLUMN_IEEE8021BRIDGEBASEROWSTATUS:
                switch (*request->requestvb->val.integer) {
                case RS_CREATEANDGO:
                    table_entry->valid = 1;
                    /* Fall-through */
                case RS_ACTIVE:
                    table_entry->ieee8021BridgeBaseRowStatus = RS_ACTIVE;
                    break;

                case RS_CREATEANDWAIT:
                    table_entry->valid = 1;
                    /* Fall-through */
                case RS_NOTINSERVICE:
                    table_entry->ieee8021BridgeBaseRowStatus = RS_NOTINSERVICE;
                    break;

                case RS_DESTROY:
                    ieee8021BridgeBaseTable_removeEntry(table_data, table_row );
                }
            }
        }
#endif
        break;

    }

    return SNMP_ERR_NOERROR;
}
