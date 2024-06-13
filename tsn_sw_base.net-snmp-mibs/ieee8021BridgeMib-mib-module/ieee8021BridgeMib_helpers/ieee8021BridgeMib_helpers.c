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

#include "ieee8021BridgeMib_helpers.h"
#include "../ieee8021BridgeTrafficClassTable/ieee8021BridgeTrafficClassTable.h"

int get_port_name_by_tsn_port_id(uint32_t port_id, char port_name[MAX_STR_LEN])
{
	char ports_name[MAX_NUM_OF_INTERFACES][MAX_STR_LEN];
	struct port_hdl *port = NULL;
	uint32_t tmp_port_id = 0u;
	int i = 0;

	if(EXIT_FAILURE == get_bridge_port_names(ports_name, MAX_NUM_OF_INTERFACES, MAX_STR_LEN))
	{
		snmp_log(LOG_ERR, "ERROR: get_bridge_port_names failed.\n");
		return 1;
	}

	while((i < MAX_NUM_OF_INTERFACES) && (0 != strncmp(ports_name[i], "", MAX_STR_LEN)))
	{
    	port = tsn_port_open_by_name(ports_name[i]);
    	if(NULL != port)
    	{
    	    if(0 == tsn_bridge_port_id(port, &tmp_port_id))
    	    {
    	    	if(tmp_port_id == port_id)
    	    	{
    	    		snprintf(port_name, MAX_STR_LEN, "%s", ports_name[i]);
    	        	tsn_port_close(port);
    	    		break;
    	    	}
    	    }
    	    else
    	    {
    			snmp_log(LOG_ERR, "ERROR: tsn_bridge_port_id failed.\n");
    	    	tsn_port_close(port);
    	    	break;
    	    }
    	}
    	else
    	{
            snmp_log(LOG_ERR, "port could not open\n");
            /* error */
            return EXIT_FAILURE;
    	}
    	tsn_port_close(port);
    	i++;
	}

	return EXIT_SUCCESS;
}

int find_row_in_tc_table(netsnmp_tdata* table, u_long component_id, uint32_t port_id, u_long priority)
{
	netsnmp_tdata_row* agent_row = NULL;
	ieee8021BridgeTrafficClassTable_entry_t* entry = NULL;

	/* returns the first row in the table */
	agent_row = netsnmp_tdata_row_first(table);
	while(NULL != agent_row)
	{
		entry = (ieee8021BridgeTrafficClassTable_entry_t*)agent_row->data;
		if(NULL != entry)
		{
			if((component_id == entry->ieee8021BridgeBasePortComponentId) &&
				(port_id == entry->ieee8021BridgeBasePort) &&
				(priority == entry->ieee8021BridgeTrafficClassPriority))
			{
					return 1;
			}
		}
		agent_row = netsnmp_tdata_row_next(table, agent_row);
	}
	return 0;
}
