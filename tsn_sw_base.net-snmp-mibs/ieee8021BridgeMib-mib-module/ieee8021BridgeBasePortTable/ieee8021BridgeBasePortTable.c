/*
 * TTTech ieee8021BridgeMib-mib-module
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
#include "ieee8021BridgeBasePortTable.h"
#include "../ieee8021BridgeMib_helpers/ieee8021BridgeMib_helpers.h"
/* libbase includes */
#include "base_prints.h"
#include "base_linux_tool_helpers.h"
#include "base_q_bridge_linux_tools.h"
#include "base_mstp_linux_tools.h"
#include "base_private_translators.h"
/* libtsn includes */
#include "tsn.h"
#include "tsn_bridge.h"

static char p2p_map[3][2][MAX_STR_LEN] = {//3 pairs
		{BASE_YES, BASE_ONE_STR},
		{BASE_NO, BASE_TWO_STR},
		{BASE_AUTO, BASE_THREE_STR},
};

static char p2p_map_inverse[3][2][MAX_STR_LEN] = {//3 pairs
		{BASE_ONE_STR, BASE_YES},
		{BASE_TWO_STR, BASE_NO},
		{BASE_THREE_STR, BASE_AUTO},
};

/** Initializes the ieee8021BridgeBasePortTable module */
void init_ieee8021BridgeBasePortTable(void)
{
  /* here we initialize all the tables we're planning on supporting */
    initialize_table_ieee8021BridgeBasePortTable();
}


/** Initialize the ieee8021BridgeBasePortTable table by defining its contents and how it's structured */
void initialize_table_ieee8021BridgeBasePortTable(void)
{
    const oid ieee8021BridgeBasePortTable_oid[] = {1,3,111,2,802,1,1,2,1,1,4};
    const size_t ieee8021BridgeBasePortTable_oid_len   = 
            OID_LENGTH(ieee8021BridgeBasePortTable_oid);
    netsnmp_handler_registration    *reg;
    netsnmp_tdata                   *table_data;
    netsnmp_table_registration_info *table_info;
    netsnmp_cache                   *cache;

    DEBUGMSGTL(("ieee8021BridgeBasePortTable:init", "initializing table ieee8021BridgeBasePortTable\n"));

    reg = netsnmp_create_handler_registration(
              "ieee8021BridgeBasePortTable",     ieee8021BridgeBasePortTable_handler,
              ieee8021BridgeBasePortTable_oid, ieee8021BridgeBasePortTable_oid_len,
              HANDLER_CAN_RWRITE
              );

    table_data = netsnmp_tdata_create_table( "ieee8021BridgeBasePortTable", 0 );
    if (NULL == table_data) {
        snmp_log(LOG_ERR,"error creating tdata table for ieee8021BridgeBasePortTable\n");
        return;
    }
    cache = netsnmp_cache_create(IEEE8021BRIDGEBASEPORTTABLE_TIMEOUT,
                                  ieee8021BridgeBasePortTable_load, ieee8021BridgeBasePortTable_free,
                                  ieee8021BridgeBasePortTable_oid, ieee8021BridgeBasePortTable_oid_len);
    if (NULL == cache) {
        snmp_log(LOG_ERR,"error creating cache for ieee8021BridgeBasePortTable\n");
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
        snmp_log(LOG_ERR,"error creating table info for ieee8021BridgeBasePortTable\n");
        return;
    }
    netsnmp_table_helper_add_indexes(table_info,
                           ASN_UNSIGNED,  /* index: ieee8021BridgeBasePortComponentId */
                           ASN_UNSIGNED,  /* index: ieee8021BridgeBasePort */
                           0);

    table_info->min_column = COLUMN_IEEE8021BRIDGEBASEPORTIFINDEX;
    table_info->max_column = COLUMN_IEEE8021BRIDGEBASEPORTNAME;
    
    netsnmp_tdata_register( reg, table_data, table_info );

    /* Initialise the contents of the table here */
    if(0 != init_ieee8021BridgeBasePortTable_with_sys_data(table_data))
    {
        snmp_log(LOG_ERR,"ERROR: ieee8021BridgeBasePortTable initialization was unsuccessful.\n");
        free(table_info);
        return;
    }
    else
    {
        snmp_log(LOG_INFO,"INFO: ieee8021BridgeBasePortTable initialization was successful.\n");
        netsnmp_cache_timer_start(cache);
    }
}

/* init table */
int init_ieee8021BridgeBasePortTable_with_sys_data(netsnmp_tdata *table_data)
{
    netsnmp_tdata_row *row = NULL;
    char bridges_name[MAX_NUM_OF_INTERFACES][MAX_STR_LEN];
    char ports_name[MAX_NUM_OF_INTERFACES][MAX_STR_LEN];
    int i = 0;
	struct port_hdl *port = NULL;
	uint32_t port_id = 0u;

    snmp_log(LOG_INFO,"called: init_ieee8021BridgeBasePortTable_with_sys_data.\n");

    /* gather system data */
	if(EXIT_FAILURE == get_bridge_names(bridges_name, MAX_NUM_OF_INTERFACES, MAX_STR_LEN))
	{
		snmp_log(LOG_ERR, "ERROR: get_bridge_names failed.\n");
		return 1;
	}
    if(EXIT_FAILURE == get_bridge_port_names(ports_name, MAX_NUM_OF_INTERFACES, MAX_STR_LEN))
    {
    	snmp_log(LOG_ERR, "ERROR: get_bridge_port_names failed.\n");
        return 1;
    }

    while((i < MAX_NUM_OF_INTERFACES) && (0 != strncmp(ports_name[i], "", MAX_STR_LEN)))
    {
    	port = tsn_port_open_by_name(ports_name[i]);//open port with port_name
    	if(NULL != port)
    	{
    	    if(0 != tsn_bridge_port_id(port, &port_id))//read port_id
    	    {
				snmp_log(LOG_ERR, "ERROR: Failed to get port id: '%s'.\n", ports_name[i]);
				tsn_port_close(port);
				break;
			}
    	    tsn_port_close(port);
    	}
    	else//port could not be open
    	{
            snmp_log(LOG_ERR, "%s(): port could not be open: (%s)\n", __func__, ports_name[i]);
            break;
    	}

		row = ieee8021BridgeBasePortTable_createEntry(table_data, 1, port_id + 1);
		if(NULL != row)
		{
			ieee8021BridgeBasePortTable_fillEntry(
					bridges_name[0], ports_name[i], (ieee8021BridgeBasePortTable_entry_t*)row->data);
		}
		else
		{
			snmp_log(LOG_ERR, "ERROR: New row is not created.\n");
			break;
		}
    	row = NULL;
    	i++;
    }
    /* success */
    return 0;
}

/* create a new row in the table */
netsnmp_tdata_row *ieee8021BridgeBasePortTable_createEntry(netsnmp_tdata *table_data,
		u_long ieee8021BridgeBasePortComponentId, u_long ieee8021BridgeBasePort)
{
    struct ieee8021BridgeBasePortTable_entry *entry = NULL;
    netsnmp_tdata_row *row = NULL;

    entry = SNMP_MALLOC_TYPEDEF(struct ieee8021BridgeBasePortTable_entry);
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

    snmp_log(LOG_INFO,"ieee8021BridgeBasePortTable_createEntry - (%ld) - (%ld)\n",
            ieee8021BridgeBasePortComponentId, ieee8021BridgeBasePort);

    entry->ieee8021BridgeBasePortComponentId = ieee8021BridgeBasePortComponentId;
    netsnmp_tdata_row_add_index( row, ASN_UNSIGNED,
		 &(entry->ieee8021BridgeBasePortComponentId),
		 sizeof(entry->ieee8021BridgeBasePortComponentId));

    entry->ieee8021BridgeBasePort = ieee8021BridgeBasePort;
    netsnmp_tdata_row_add_index( row, ASN_UNSIGNED,
		 &(entry->ieee8021BridgeBasePort),
		 sizeof(entry->ieee8021BridgeBasePort));

    if (table_data)
    {
        netsnmp_tdata_add_row( table_data, row );
    }

    return row;
}

/* fill a new row data in the table */
int ieee8021BridgeBasePortTable_fillEntry(char bridge_name[MAX_STR_LEN], char port_name[MAX_STR_LEN],
        ieee8021BridgeBasePortTable_entry_t *entry)
{
    char result_array[MAX_NUM_OF_INTERFACES][MAX_STR_LEN];
	struct port_hdl *port = NULL;
	struct brport_attr_and_caps caps = {0};

    if (!entry)
    {
        snmp_log(LOG_ERR, "ieee8021BridgeBasePortTable_fillEntry:entry == NULL\n");
        /* error */
        return 1;
    }

    /* ieee8021BridgeBasePortIfIndex */
    if(EXIT_SUCCESS == get_interface_if_index(result_array, MAX_NUM_OF_INTERFACES, MAX_STR_LEN, port_name))
    {
    	entry->ieee8021BridgeBasePortIfIndex = strtol(result_array[0], NULL, 10);
    }

    /* ieee8021BridgeBasePortName */
    strncpy(entry->ieee8021BridgeBasePortName, port_name, MAX_STR_LEN);
    entry->ieee8021BridgeBasePortName_len = strlen(entry->ieee8021BridgeBasePortName);

    /* ieee8021BridgeBasePortAdminPointToPoint */
    if(EXIT_SUCCESS == get_cist_port_admin_point_to_point(result_array, MAX_NUM_OF_INTERFACES, MAX_STR_LEN, bridge_name, port_name))
    {
    	if(EXIT_SUCCESS == translate_single_string_using_map(result_array[0], p2p_map))
    	{
    		entry->ieee8021BridgeBasePortAdminPointToPoint = strtol(result_array[0], NULL, 10);
    	}
    }

    /* ieee8021BridgeBasePortOperPointToPoint */
    if(EXIT_SUCCESS == get_cist_port_mac_enabled(result_array, MAX_NUM_OF_INTERFACES, MAX_STR_LEN, bridge_name, port_name))
    {
    	if(EXIT_SUCCESS == translate_single_string_using_map(result_array[0], p2p_map))
    	{
    		entry->ieee8021BridgeBasePortOperPointToPoint = strtol(result_array[0], NULL, 10);
    	}
    }

    /* ieee8021BridgeBasePortType */
	port = tsn_port_open_by_name(port_name);/* open port */
	if(NULL != port)
	{
		if(tsn_bridge_port_attr(port, &caps))
		{
			snmp_log(LOG_ERR, "ieee8021BridgeBasePortTable_fillEntry: port's attributes are unreachable\n");
			tsn_port_close(port);
			return 1; /* error */
		}
		entry->ieee8021BridgeBasePortType = caps.portType;
		tsn_port_close(port);
	}
	else
	{
		snmp_log(LOG_ERR, "ieee8021BridgeBasePortTable_fillEntry: port handler is invalid\n");
		return 1; /* error */
	}

    return 0;
}

/* remove a row from the table */
void ieee8021BridgeBasePortTable_removeEntry(netsnmp_tdata     *table_data,
                 netsnmp_tdata_row *row) {
    struct ieee8021BridgeBasePortTable_entry *entry = NULL;

    if (!row)
        return;    /* Nothing to remove */

    entry = (struct ieee8021BridgeBasePortTable_entry *)row->data;
    SNMP_FREE( entry );   /* XXX - release any other internal resources */

    if (table_data)
        netsnmp_tdata_remove_and_delete_row( table_data, row );
    else
        netsnmp_tdata_delete_row( row );    
}

/* Example cache handling - set up table_data list from a suitable file */
int ieee8021BridgeBasePortTable_load( netsnmp_cache *cache, void *vmagic ) {
	/* will be used to iterate trough ieee8021BridgeBaseTable_head */
	netsnmp_tdata_row *agent_row = NULL;
	/* helper to keep next agent row */
	netsnmp_tdata_row *next_agent_row = NULL;
	/* will be used to check existence of the bridge */
	netsnmp_tdata *table = (netsnmp_tdata *)vmagic;
	ieee8021BridgeBasePortTable_entry_t *entry = NULL;
	int port_found = 0;
	char bridges_name[MAX_NUM_OF_INTERFACES][MAX_STR_LEN];
	char ports_name[MAX_NUM_OF_INTERFACES][MAX_STR_LEN];
	u_long component_id = 1;
	struct port_hdl *port = NULL;
	uint32_t port_id = 0u;
	int i = 0;
	char port_name[MAX_STR_LEN];

	/* gather fresh system data */
	if(EXIT_FAILURE == get_bridge_names(bridges_name, MAX_NUM_OF_INTERFACES, MAX_STR_LEN))
	{
		snmp_log(LOG_ERR, "ERROR: get_bridge_names failed.\n");
		return 1;
	}
	if(EXIT_FAILURE == get_bridge_port_names(ports_name, MAX_NUM_OF_INTERFACES, MAX_STR_LEN))
	{
		snmp_log(LOG_ERR, "ERROR: get_bridge_port_names failed.\n");
		return 1;
	}

	/* returns the first row in the table */
	agent_row = netsnmp_tdata_row_first (table);

	while(NULL != agent_row)
	{
		entry = (ieee8021BridgeBasePortTable_entry_t *)agent_row->data;

		if(NULL != entry)
		{
			/* there is no more agent_row, erase it from agent */
			if((!entry->ieee8021BridgeBasePortComponentId) || (!entry->ieee8021BridgeBasePort))
			{
				snmp_log(LOG_DEBUG, "there is no more agent_row, erase it from agent\n");
				/* returns next row */
				next_agent_row = netsnmp_tdata_row_next (table, agent_row);
				ieee8021BridgeBasePortTable_removeEntry(table, agent_row);
				agent_row = next_agent_row;
				next_agent_row = NULL;
			}
			/* agent_row exist on system, skip it (leave it on agent) */
			else
			{
				snmp_log(LOG_DEBUG, "agent_row exist on system, skip it\n");
				/* refresh data in this existing row on agent */
				port = tsn_port_open_by_index(entry->ieee8021BridgeBasePortIfIndex);
				if(NULL != port)
				{
					snprintf(port_name, MAX_STR_LEN, "%s", tsn_get_name_by_hdl(port));
					ieee8021BridgeBasePortTable_fillEntry(bridges_name[0], port_name,
							(ieee8021BridgeBasePortTable_entry_t *)agent_row->data);
					tsn_port_close(port);
				}
				/* take next */
				agent_row = netsnmp_tdata_row_next (table, agent_row);
			}
		}
		else
		{
			snmp_log(LOG_ERR, "error: agent_row->data is NULL\n");
		}
	}

	while((i < MAX_NUM_OF_INTERFACES) && (0 != strncmp(ports_name[i], "", MAX_STR_LEN)))
	{
		port = tsn_port_open_by_name(ports_name[i]);
		if(NULL == port)
		{
			snmp_log(LOG_ERR, "port could not open: '%s'\n", ports_name[i]);
			/* error */
			return 1;
		}
		if(0 != tsn_bridge_port_id(port, &port_id))
		{
			snmp_log(LOG_ERR, "could not get port id: '%s'\n", ports_name[i]);
			tsn_port_close(port);
			/* error */
			return 1;
		}
		tsn_port_close(port);
		/* returns the first row in the table */
		agent_row = netsnmp_tdata_row_first(table);

		while((NULL != agent_row) && (port_found == 0))
		{
			entry = (ieee8021BridgeBasePortTable_entry_t *)agent_row->data;

			if( (entry->ieee8021BridgeBasePortComponentId == component_id) &&
					(entry->ieee8021BridgeBasePort == (port_id + 1)) )
			{
				port_found = 1;
			}
			agent_row = netsnmp_tdata_row_next(table, agent_row);
		}

		if(port_found == 0)
		{
			snmp_log(LOG_DEBUG, "load: Adding new port: '%s'\n", ports_name[i]);
			agent_row = ieee8021BridgeBasePortTable_createEntry(table, component_id, port_id + 1);
			ieee8021BridgeBasePortTable_fillEntry(bridges_name[0], ports_name[i], agent_row->data);
		}
		i++;
		port_found = 0;
	}
	/* OK */
	return 0;
}

void ieee8021BridgeBasePortTable_free( netsnmp_cache *cache, void *vmagic ) {
    netsnmp_tdata     *table = (netsnmp_tdata *)vmagic;
    netsnmp_tdata_row *this;

    while ((this = netsnmp_tdata_row_first(table))) {
        netsnmp_tdata_remove_and_delete_row(table, this);
    }
}

/** handles requests for the ieee8021BridgeBasePortTable table */
int ieee8021BridgeBasePortTable_handler(
    netsnmp_mib_handler               *handler,
    netsnmp_handler_registration      *reginfo,
    netsnmp_agent_request_info        *reqinfo,
    netsnmp_request_info              *requests) {

    netsnmp_request_info       *request;
    netsnmp_table_request_info *table_info;
    netsnmp_tdata              *table_data;
    netsnmp_tdata_row          *table_row;
    struct ieee8021BridgeBasePortTable_entry          *table_entry;
    int                         ret;
    char bridges_name[MAX_NUM_OF_INTERFACES][MAX_STR_LEN];
    char port_name[MAX_STR_LEN];
    char tmp_p2p[MAX_STR_LEN];

    DEBUGMSGTL(("ieee8021BridgeBasePortTable:handler", "Processing request (%d)\n", reqinfo->mode));

    switch (reqinfo->mode) {
        /*
         * Read-support (also covers GetNext requests)
         */
    case MODE_GET:
        for (request=requests; request; request=request->next) {
            if (request->processed)
                continue;

            table_entry = (struct ieee8021BridgeBasePortTable_entry *)
                              netsnmp_tdata_extract_entry(request);
            table_info  =     netsnmp_extract_table_info( request);

            switch (table_info->colnum) {

            case COLUMN_IEEE8021BRIDGEBASEPORTIFINDEX:
                if ( !table_entry ) {
                    netsnmp_set_request_error(reqinfo, request,
                                              SNMP_NOSUCHINSTANCE);
                    continue;
                }
                snmp_set_var_typed_integer( request->requestvb, ASN_INTEGER,
                                            table_entry->ieee8021BridgeBasePortIfIndex);
                break;
#if 0
            case COLUMN_IEEE8021BRIDGEBASEPORTDELAYEXCEEDEDDISCARDS:
                if ( !table_entry ) {
                    netsnmp_set_request_error(reqinfo, request,
                                              SNMP_NOSUCHINSTANCE);
                    continue;
                }
                snmp_set_var_typed_integer( request->requestvb, ASN_COUNTER64,
                                            table_entry->ieee8021BridgeBasePortDelayExceededDiscards);
                break;
            case COLUMN_IEEE8021BRIDGEBASEPORTMTUEXCEEDEDDISCARDS:
                if ( !table_entry ) {
                    netsnmp_set_request_error(reqinfo, request,
                                              SNMP_NOSUCHINSTANCE);
                    continue;
                }
                snmp_set_var_typed_integer( request->requestvb, ASN_COUNTER64,
                                            table_entry->ieee8021BridgeBasePortMtuExceededDiscards);
                break;
            case COLUMN_IEEE8021BRIDGEBASEPORTCAPABILITIES:
                if ( !table_entry ) {
                    netsnmp_set_request_error(reqinfo, request,
                                              SNMP_NOSUCHINSTANCE);
                    continue;
                }
                snmp_set_var_typed_value( request->requestvb, ASN_OCTET_STR,
                                          table_entry->ieee8021BridgeBasePortCapabilities,
                                          table_entry->ieee8021BridgeBasePortCapabilities_len);
                break;
            case COLUMN_IEEE8021BRIDGEBASEPORTTYPECAPABILITIES:
                if ( !table_entry ) {
                    netsnmp_set_request_error(reqinfo, request,
                                              SNMP_NOSUCHINSTANCE);
                    continue;
                }
                snmp_set_var_typed_value( request->requestvb, ASN_OCTET_STR,
                                          table_entry->ieee8021BridgeBasePortTypeCapabilities,
                                          table_entry->ieee8021BridgeBasePortTypeCapabilities_len);
                break;
#endif
            case COLUMN_IEEE8021BRIDGEBASEPORTTYPE:
                if ( !table_entry ) {
                    netsnmp_set_request_error(reqinfo, request,
                                              SNMP_NOSUCHINSTANCE);
                    continue;
                }
                snmp_set_var_typed_integer( request->requestvb, ASN_INTEGER,
                                            table_entry->ieee8021BridgeBasePortType);
                break;
#if 0
            case COLUMN_IEEE8021BRIDGEBASEPORTEXTERNAL:
                if ( !table_entry ) {
                    netsnmp_set_request_error(reqinfo, request,
                                              SNMP_NOSUCHINSTANCE);
                    continue;
                }
                snmp_set_var_typed_integer( request->requestvb, ASN_INTEGER,
                                            table_entry->ieee8021BridgeBasePortExternal);
                break;
#endif
            case COLUMN_IEEE8021BRIDGEBASEPORTADMINPOINTTOPOINT:
                if ( !table_entry ) {
                    netsnmp_set_request_error(reqinfo, request, SNMP_NOSUCHINSTANCE);
                    continue;
                }
                snmp_set_var_typed_integer( request->requestvb, ASN_INTEGER,
                                            table_entry->ieee8021BridgeBasePortAdminPointToPoint);
                break;
            case COLUMN_IEEE8021BRIDGEBASEPORTOPERPOINTTOPOINT:
                if ( !table_entry ) {
                    netsnmp_set_request_error(reqinfo, request,
                                              SNMP_NOSUCHINSTANCE);
                    continue;
                }
                snmp_set_var_typed_integer( request->requestvb, ASN_INTEGER,
                                            table_entry->ieee8021BridgeBasePortOperPointToPoint);
                break;
            case COLUMN_IEEE8021BRIDGEBASEPORTNAME:
                if ( !table_entry ) {
                    netsnmp_set_request_error(reqinfo, request,
                                              SNMP_NOSUCHINSTANCE);
                    continue;
                }
                snmp_set_var_typed_value( request->requestvb, ASN_OCTET_STR,
                                          table_entry->ieee8021BridgeBasePortName,
                                          table_entry->ieee8021BridgeBasePortName_len);
                break;
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
            table_entry = (struct ieee8021BridgeBasePortTable_entry *)
                              netsnmp_tdata_extract_entry(request);
#endif
            table_info  =     netsnmp_extract_table_info( request);
    
            switch (table_info->colnum) {
#if 0
            case COLUMN_IEEE8021BRIDGEBASEPORTIFINDEX:
                /* or possibly 'netsnmp_check_vb_int_range' */
                ret = netsnmp_check_vb_int( request->requestvb );
                if ( ret != SNMP_ERR_NOERROR ) {
                    netsnmp_set_request_error( reqinfo, request, ret );
                    return SNMP_ERR_NOERROR;
                }
                break;
#endif
            case COLUMN_IEEE8021BRIDGEBASEPORTADMINPOINTTOPOINT:
                /* or possibly 'netsnmp_check_vb_int_range' */
                ret = netsnmp_check_vb_int( request->requestvb );
                if ( ret != SNMP_ERR_NOERROR ) {
                    netsnmp_set_request_error( reqinfo, request, ret );
                    return SNMP_ERR_NOERROR;
                }
                break;
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
        
        for (request=requests; request; request=request->next) {
            if (request->processed)
                continue;

            table_entry = (struct ieee8021BridgeBasePortTable_entry *)
                              netsnmp_tdata_extract_entry(request);
            table_info  =     netsnmp_extract_table_info( request);
    
            switch (table_info->colnum) {
#if 0
            case COLUMN_IEEE8021BRIDGEBASEPORTIFINDEX:
                table_entry->old_ieee8021BridgeBasePortIfIndex = table_entry->ieee8021BridgeBasePortIfIndex;
                table_entry->ieee8021BridgeBasePortIfIndex     = *request->requestvb->val.integer;
                break;
#endif
            case COLUMN_IEEE8021BRIDGEBASEPORTADMINPOINTTOPOINT:
                table_entry->old_ieee8021BridgeBasePortAdminPointToPoint = table_entry->ieee8021BridgeBasePortAdminPointToPoint;
                table_entry->ieee8021BridgeBasePortAdminPointToPoint     = *request->requestvb->val.integer;

                if(EXIT_FAILURE == get_bridge_names(bridges_name, MAX_NUM_OF_INTERFACES, MAX_STR_LEN))
                {
                	snmp_log(LOG_ERR, "ERROR: get_bridge_names failed.\n");
                    break;
                }

				if(EXIT_FAILURE == get_port_name_by_tsn_port_id(table_entry->ieee8021BridgeBasePort, port_name))
				{
					snmp_log(LOG_ERR, "error: get_port_name_by_tsn_port_id returns EXIT_FAILURE\n");
					break;
				}

				snprintf(tmp_p2p, MAX_STR_LEN, "%ld", table_entry->ieee8021BridgeBasePortAdminPointToPoint);

		    	if(EXIT_SUCCESS == translate_single_string_using_map(tmp_p2p, p2p_map_inverse))
		    	{
					if(EXIT_FAILURE == set_cist_port_admin_point_to_point(bridges_name[0], port_name, tmp_p2p))
					{
						snmp_log(LOG_ERR, "error: get_port_name_by_tsn_port_id returns EXIT_FAILURE\n");
						break;
					}		    	}
            }
        }
        break;

    case MODE_SET_UNDO:
#if 0
        for (request=requests; request; request=request->next) {
            if (request->processed)
                continue;

            table_entry = (struct ieee8021BridgeBasePortTable_entry *)
                              netsnmp_tdata_extract_entry(request);
            table_row   =     netsnmp_tdata_extract_row(  request);
            table_data  =     netsnmp_tdata_extract_table(request);
            table_info  =     netsnmp_extract_table_info( request);
    
            switch (table_info->colnum) {
            case COLUMN_IEEE8021BRIDGEBASEPORTIFINDEX:
                table_entry->ieee8021BridgeBasePortIfIndex     = table_entry->old_ieee8021BridgeBasePortIfIndex;
                table_entry->old_ieee8021BridgeBasePortIfIndex = 0;
                break;
            case COLUMN_IEEE8021BRIDGEBASEPORTADMINPOINTTOPOINT:
                table_entry->ieee8021BridgeBasePortAdminPointToPoint     = table_entry->old_ieee8021BridgeBasePortAdminPointToPoint;
                set_admin_point_to_point(table_entry->ieee8021BridgeBasePortComponentId,
                        table_entry->ieee8021BridgeBasePort, 
                        table_entry->old_ieee8021BridgeBasePortAdminPointToPoint);
                table_entry->old_ieee8021BridgeBasePortAdminPointToPoint = 0;
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
