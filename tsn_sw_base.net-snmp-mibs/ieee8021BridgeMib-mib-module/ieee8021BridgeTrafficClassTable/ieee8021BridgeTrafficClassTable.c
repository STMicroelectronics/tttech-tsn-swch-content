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
#include "ieee8021BridgeTrafficClassTable.h"
#include "../ieee8021BridgeMib_helpers/ieee8021BridgeMib_helpers.h"
/* libbase includes */
#include "base_prints.h"
#include "base_linux_tool_helpers.h"
#include "base_q_bridge_linux_tools.h"
#include "base_private_translators.h"
/* libtsn includes */
#include "tsn.h"
#include "tsn_bridge.h"

/** Initializes the ieee8021BridgeTrafficClassTable module */
void
init_ieee8021BridgeTrafficClassTable(void)
{
  /* here we initialize all the tables we're planning on supporting */
    initialize_table_ieee8021BridgeTrafficClassTable();
}


/** Initialize the ieee8021BridgeTrafficClassTable table by defining its contents and how it's structured */
void
initialize_table_ieee8021BridgeTrafficClassTable(void)
{
    const oid ieee8021BridgeTrafficClassTable_oid[] = {1,3,111,2,802,1,1,2,1,3,3};
    const size_t ieee8021BridgeTrafficClassTable_oid_len   = OID_LENGTH(ieee8021BridgeTrafficClassTable_oid);
    netsnmp_handler_registration    *reg;
    netsnmp_tdata                   *table_data;
    netsnmp_table_registration_info *table_info;
    netsnmp_cache                   *cache;

//    DEBUGMSGTL(("ieee8021BridgeTrafficClassTable:init", "initializing table ieee8021BridgeTrafficClassTable\n"));

    reg = netsnmp_create_handler_registration(
              "ieee8021BridgeTrafficClassTable",     ieee8021BridgeTrafficClassTable_handler,
              ieee8021BridgeTrafficClassTable_oid, ieee8021BridgeTrafficClassTable_oid_len,
              HANDLER_CAN_RWRITE
              );

    table_data = netsnmp_tdata_create_table( "ieee8021BridgeTrafficClassTable", 0 );
    if (NULL == table_data) {
        snmp_log(LOG_ERR,"error creating tdata table for ieee8021BridgeTrafficClassTable\n");
        return;
    }
    cache = netsnmp_cache_create(IEEE8021BRIDGETRAFFICCLASSTABLE_TIMEOUT,
                                  ieee8021BridgeTrafficClassTable_load, ieee8021BridgeTrafficClassTable_free,
                                  ieee8021BridgeTrafficClassTable_oid, ieee8021BridgeTrafficClassTable_oid_len);
    if (NULL == cache) {
        snmp_log(LOG_ERR,"error creating cache for ieee8021BridgeTrafficClassTable\n");
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
        snmp_log(LOG_ERR,"error creating table info for ieee8021BridgeTrafficClassTable\n");
        return;
    }
    netsnmp_table_helper_add_indexes(table_info,
                           ASN_UNSIGNED,  /* index: ieee8021BridgeBasePortComponentId */
                           ASN_UNSIGNED,  /* index: ieee8021BridgeBasePort */
                           ASN_UNSIGNED,  /* index: ieee8021BridgeTrafficClassPriority */
                           0);

    table_info->min_column = COLUMN_IEEE8021BRIDGETRAFFICCLASS;
    table_info->max_column = COLUMN_IEEE8021BRIDGETRAFFICCLASS;
    
    netsnmp_tdata_register( reg, table_data, table_info );

    /* Initialise the contents of the table here */
    if(0 != init_ieee8021BridgeTrafficClassTable_with_sys_data(table_data))
    {
        snmp_log(LOG_ERR,"ERROR: ieee8021BridgeTrafficClassTable initialization was unsuccessful.\n");
        free(table_info);
        return;
    }
    else
    {
        snmp_log(LOG_INFO,"INFO: ieee8021BridgeTrafficClassTable initialization was successful.\n");
        netsnmp_cache_timer_start(cache);
    }
}

/* init table */
int init_ieee8021BridgeTrafficClassTable_with_sys_data(netsnmp_tdata *table_data)
{
    netsnmp_tdata_row *row = NULL;
    char ports_name[MAX_NUM_OF_INTERFACES][MAX_STR_LEN];
    int i = 0;
	struct port_hdl *port = NULL;
	uint32_t port_id = 0u;
    unsigned int priority = 0;

    snmp_log(LOG_INFO,"called: init_ieee8021BridgeTrafficClassTable_with_sys_data.\n");

    /* gather system data */
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
				return 1;
			}
    	    tsn_port_close(port);
    	}
    	else//port could not be open
    	{
            snmp_log(LOG_ERR, "port could not open: (%s)\n", ports_name[i]);
            return 1;
    	}

		priority = 0;
		while(priority < 8)
		{
			row = ieee8021BridgeTrafficClassTable_createEntry(table_data, 1, port_id + 1, priority);
			if(NULL != row)
			{
				ieee8021BridgeTrafficClassTable_fillEntry(ports_name[i], priority, (ieee8021BridgeTrafficClassTable_entry_t*)row->data);
			}
			else
			{
				snmp_log(LOG_ERR, "ERROR: New row is not created.\n");
				return 1;
			}
			priority++;
		}

    	row = NULL;//for next entry
    	i++;
    }

    return 0;/* success */
}

/* create a new row in the table */
netsnmp_tdata_row* ieee8021BridgeTrafficClassTable_createEntry(netsnmp_tdata *table_data,
		u_long ieee8021BridgeBasePortComponentId, u_long ieee8021BridgeBasePort, u_long ieee8021BridgeTrafficClassPriority)
{
    struct ieee8021BridgeTrafficClassTable_entry *entry;
    netsnmp_tdata_row *row;

    entry = SNMP_MALLOC_TYPEDEF(struct ieee8021BridgeTrafficClassTable_entry);
    if (!entry)
        return NULL;

    row = netsnmp_tdata_create_row();
    if (!row) {
        SNMP_FREE(entry);
        return NULL;
    }
    row->data = entry;

//    DEBUGMSGT(("ieee8021BridgeTrafficClassTable:entry:create", "row 0x%x\n", (uintptr_t)row));
    snmp_log(LOG_INFO,"ieee8021BridgeTrafficClassTable_createEntry - (%ld) - (%ld) - (%ld)\n",
    		ieee8021BridgeBasePortComponentId, ieee8021BridgeBasePort, ieee8021BridgeTrafficClassPriority);

    entry->ieee8021BridgeBasePortComponentId = ieee8021BridgeBasePortComponentId;
    netsnmp_tdata_row_add_index( row, ASN_UNSIGNED,
                                 &(entry->ieee8021BridgeBasePortComponentId),
                                 sizeof(entry->ieee8021BridgeBasePortComponentId));
    entry->ieee8021BridgeBasePort = ieee8021BridgeBasePort;
    netsnmp_tdata_row_add_index( row, ASN_UNSIGNED,
                                 &(entry->ieee8021BridgeBasePort),
                                 sizeof(entry->ieee8021BridgeBasePort));
    entry->ieee8021BridgeTrafficClassPriority = ieee8021BridgeTrafficClassPriority;
    netsnmp_tdata_row_add_index( row, ASN_UNSIGNED,
                                 &(entry->ieee8021BridgeTrafficClassPriority),
                                 sizeof(entry->ieee8021BridgeTrafficClassPriority));
    if (table_data)
        netsnmp_tdata_add_row( table_data, row );
    return row;
}

int ieee8021BridgeTrafficClassTable_fillEntry(char port_name[MAX_STR_LEN], unsigned int priority,
		ieee8021BridgeTrafficClassTable_entry_t *entry)
{
	struct port_hdl *port = NULL;
	uint8_t traffic_class = 0u;

    if (!entry)
    {
        snmp_log(LOG_ERR, "ieee8021BridgeTrafficClassTable_fillEntry:entry == NULL\n");
        /* error */
        return 1;
    }

	port = tsn_port_open_by_name(port_name);
	if(NULL == port)
	{
        snmp_log(LOG_ERR, "%s(): port could not open\n", __func__);
        /* error */
        return 1;
	}

    if(0 == tsn_fqtss_get_tc_table(port, priority, &traffic_class))
    {
    	entry->ieee8021BridgeTrafficClass = (u_long)traffic_class;
    	tsn_port_close(port);
    }
    else
    {
        snmp_log(LOG_ERR, "%s(): tsn_fqtss_get_tc_table FAILED!\n", __func__);
        tsn_port_close(port);
        /* error */
        return 1;
    }

	return 0;
}

/* remove a row from the table */
void
ieee8021BridgeTrafficClassTable_removeEntry(
		netsnmp_tdata *table_data, netsnmp_tdata_row *row) {
    struct ieee8021BridgeTrafficClassTable_entry *entry;

    if (!row)
        return;    /* Nothing to remove */

//    DEBUGMSGT(("ieee8021BridgeTrafficClassTable:entry:remove", "row 0x%x\n", (uintptr_t)row));

    entry = (struct ieee8021BridgeTrafficClassTable_entry *)row->data;
    SNMP_FREE( entry );   /* XXX - release any other internal resources */

    if (table_data)
        netsnmp_tdata_remove_and_delete_row( table_data, row );
    else
        netsnmp_tdata_delete_row( row );    
}

/* Example cache handling - set up table_data list from a suitable file */
int
ieee8021BridgeTrafficClassTable_load( netsnmp_cache *cache, void *vmagic ) {
	/* will be used to iterate trough ieee8021BridgeBaseTable_head */
	netsnmp_tdata_row *agent_row = NULL;
	/* helper to keep next agent row */
	netsnmp_tdata_row *next_agent_row = NULL;
	/* will be used to check existence of the bridge */
	netsnmp_tdata *table = (netsnmp_tdata *)vmagic;
	ieee8021BridgeTrafficClassTable_entry_t *entry = NULL;
	char ports_name[MAX_NUM_OF_INTERFACES][MAX_STR_LEN];
	u_long component_id = 1;
	struct port_hdl *port = NULL;
	uint32_t port_id = 0u;
	unsigned int priority = 0;
	int i = 0;
	char port_name[MAX_STR_LEN];

	/* gather fresh system data */
	if(EXIT_FAILURE == get_bridge_port_names(ports_name, MAX_NUM_OF_INTERFACES, MAX_STR_LEN))
	{
		snmp_log(LOG_ERR, "ERROR: get_bridge_names failed.\n");
		return 1;
	}

	/* returns the first row in the table */
	agent_row = netsnmp_tdata_row_first (table);

	while(NULL != agent_row)
	{
		entry = (ieee8021BridgeTrafficClassTable_entry_t *)agent_row->data;

		if(NULL != entry)
		{
			/* there is no more agent_row, erase it from agent */
			if(1 != entry->ieee8021BridgeBasePortComponentId)// only 1 is supported
			{
				/* returns next row */
				next_agent_row = netsnmp_tdata_row_next (table, agent_row);
				ieee8021BridgeTrafficClassTable_removeEntry(table, agent_row);
				agent_row = next_agent_row;
				next_agent_row = NULL;
			}
			else /* agent_row exist on system, skip it (leave it on agent) */
			{
				/* refresh data in this existing row on agent */
				if(EXIT_FAILURE == get_port_name_by_tsn_port_id(entry->ieee8021BridgeBasePort - 1, port_name))
				{
					snmp_log(LOG_ERR, "error: get_port_name_by_tsn_port_id returns EXIT_FAILURE\n");
					return 1;
				}
				if(0 != strncmp(port_name, "", MAX_STR_LEN))
				{
					if(ieee8021BridgeTrafficClassTable_fillEntry(port_name, entry->ieee8021BridgeTrafficClassPriority, entry))//error is 1
					{
						snmp_log(LOG_ERR, "%s(): ieee8021BridgeTrafficClassTable_fillEntry (%s) (%lu) is failed!\n",
								__func__, port_name, entry->ieee8021BridgeTrafficClassPriority);
						return 1;
					}
				}
				else
				{
					snmp_log(LOG_ERR, "%s(): port name is empty\n", __func__);
					return 1;
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

	i = 0;//next loop checks data on system and updates data in memory
	while((i < MAX_NUM_OF_INTERFACES) && (0 != strncmp(ports_name[i], "", MAX_STR_LEN)))
	{
		port = tsn_port_open_by_name(ports_name[i]);
		if(NULL == port)
		{
			snmp_log(LOG_ERR, "port could not open: '%s'\n", ports_name[i]);
			/* error */
			break;
		}
		if(0 != tsn_bridge_port_id(port, &port_id))
		{
			snmp_log(LOG_ERR, "could not get port id: '%s'\n", ports_name[i]);
			tsn_port_close(port);
			/* error */
			break;
		}
		tsn_port_close(port);

		priority = 0;//reset it for every agent_row
		while(priority < 8)
		{
			//if row does not exist in existing table
			if(0 == find_row_in_tc_table(table, component_id, (port_id + 1), priority))
			{
				agent_row = ieee8021BridgeTrafficClassTable_createEntry(table, component_id, port_id + 1, priority);
				ieee8021BridgeTrafficClassTable_fillEntry(ports_name[i], priority, agent_row->data);
			}
			priority++;
		}
		i++;
	}

	return 0;/* OK */
}

void
ieee8021BridgeTrafficClassTable_free( netsnmp_cache *cache, void *vmagic ) {
    netsnmp_tdata     *table = (netsnmp_tdata *)vmagic;
    netsnmp_tdata_row *this;

    while ((this = netsnmp_tdata_row_first(table))) {
        netsnmp_tdata_remove_and_delete_row(table, this);
    }
}

/** handles requests for the ieee8021BridgeTrafficClassTable table */
int
ieee8021BridgeTrafficClassTable_handler(
    netsnmp_mib_handler               *handler,
    netsnmp_handler_registration      *reginfo,
    netsnmp_agent_request_info        *reqinfo,
    netsnmp_request_info              *requests) {

    netsnmp_request_info       *request;
    netsnmp_table_request_info *table_info;
    netsnmp_tdata              *table_data;
    netsnmp_tdata_row          *table_row;
    struct ieee8021BridgeTrafficClassTable_entry          *table_entry;
    int                         ret;
    char port_name[MAX_STR_LEN];
    struct port_hdl *port = NULL;

//    DEBUGMSGTL(("ieee8021BridgeTrafficClassTable:handler", "Processing request (%d)\n", reqinfo->mode));

    switch (reqinfo->mode) {
        /*
         * Read-support (also covers GetNext requests)
         */
    case MODE_GET:
        for (request=requests; request; request=request->next) {
            if (request->processed)
                continue;

            table_entry = (struct ieee8021BridgeTrafficClassTable_entry *)
                              netsnmp_tdata_extract_entry(request);
            table_info  =     netsnmp_extract_table_info( request);
    
            switch (table_info->colnum) {
            case COLUMN_IEEE8021BRIDGETRAFFICCLASS:
                if ( !table_entry ) {
                    netsnmp_set_request_error(reqinfo, request,
                                              SNMP_NOSUCHINSTANCE);
                    continue;
                }
                snmp_set_var_typed_integer( request->requestvb, ASN_INTEGER,
                                            table_entry->ieee8021BridgeTrafficClass);
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
            table_entry = (struct ieee8021BridgeTrafficClassTable_entry *)
                              netsnmp_tdata_extract_entry(request);
#endif
            table_info  =     netsnmp_extract_table_info( request);
    
            switch (table_info->colnum) {
            case COLUMN_IEEE8021BRIDGETRAFFICCLASS:
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

            table_entry = (struct ieee8021BridgeTrafficClassTable_entry *)
                              netsnmp_tdata_extract_entry(request);
            table_info  =     netsnmp_extract_table_info( request);
    
            switch (table_info->colnum) {
            case COLUMN_IEEE8021BRIDGETRAFFICCLASS:
                table_entry->old_ieee8021BridgeTrafficClass = table_entry->ieee8021BridgeTrafficClass;
                table_entry->ieee8021BridgeTrafficClass     = *request->requestvb->val.integer;

				if(EXIT_FAILURE == get_port_name_by_tsn_port_id(table_entry->ieee8021BridgeBasePort, port_name))
				{
					snmp_log(LOG_ERR, "error: get_port_name_by_tsn_port_id returns EXIT_FAILURE\n");
					break;
				}
				port = tsn_port_open_by_name(port_name);
				if(NULL == port)
				{
					snmp_log(LOG_ERR, "%s(): port could not open (%s)\n", __func__, port_name);
					/* error */
					break;
				}
			    if(0 != tsn_fqtss_set_tc_table(port, table_entry->ieee8021BridgeTrafficClassPriority,
						(uint8_t)table_entry->ieee8021BridgeTrafficClass))
			    {
					snmp_log(LOG_ERR, "%s(): tsn_fqtss_set_tc_table FAILED!\n", port_name);
					/* error */
					break;
			    }
			    tsn_port_close(port);

                break;
            }
        }
        break;

    case MODE_SET_UNDO:
        for (request=requests; request; request=request->next) {
            if (request->processed)
                continue;

            table_entry = (struct ieee8021BridgeTrafficClassTable_entry *)
                              netsnmp_tdata_extract_entry(request);
#if 0
            table_row   =     netsnmp_tdata_extract_row(  request);
            table_data  =     netsnmp_tdata_extract_table(request);
#endif
            table_info  =     netsnmp_extract_table_info( request);
    
            switch (table_info->colnum) {
            case COLUMN_IEEE8021BRIDGETRAFFICCLASS:
                table_entry->ieee8021BridgeTrafficClass     = table_entry->old_ieee8021BridgeTrafficClass;
                table_entry->old_ieee8021BridgeTrafficClass = 0;
                break;
            }
        }
        break;

    case MODE_SET_COMMIT:
        break;
    }
    return SNMP_ERR_NOERROR;
}
