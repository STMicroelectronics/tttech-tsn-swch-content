/*
 * TTTech libbase
 * Copyright(c) 2018 TTTech Computertechnik AG.
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
 * Contact: https://tttech.com * support@tttech.com
 * TTTech Computertechnik AG, Schoenbrunnerstrasse 7, 1040 Vienna, Austria
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <regex.h>

#include "base_prints.h"
#include "base_interfaces_linux_tools.h"
#include "base_mstp_linux_tools.h"
#include "base_q_bridge_linux_tools.h"
#include "base_linux_tools.h"
#include "base_linux_tool_helpers.h"

static void mstp_demonstration()
{
	char value[MAX_NUM_OF_INTERFACES][MAX_STR_LEN];
	char sub_value[MAX_NUM_OF_INTERFACES][MAX_STR_LEN];
	char sub_sub_value[MAX_NUM_OF_INTERFACES][MAX_STR_LEN];
	int i = 0;
	int j = 0;

	base_print_debug(DEBUG_MSG_FUN_CALLED, __func__);
	//*********************************** are module supported *****************************************
	if(EXIT_SUCCESS == mstp_daemon_check_status(value, MAX_NUM_OF_INTERFACES, MAX_STR_LEN))
	{
		base_print_debug("%s(): mstp_daemon_check_status is PASSED!\n", __func__);
	}
	else
	{
		base_print_debug("%s(): mstp_daemon_check_status is FAILED!\n", __func__);
	}
	//***********************************   MSTP  ******************************************************
	if(EXIT_SUCCESS == get_mstp_bridge_names(value, MAX_NUM_OF_INTERFACES, MAX_STR_LEN))
	{
		i = 0;
		while((i < MAX_NUM_OF_INTERFACES) && (0 != strcmp("", value[i])))
		{
			base_print_debug("%s(): %d. bridge name is: (%s)\n", __func__, i, value[i]);
			if(EXIT_SUCCESS == get_mstp_bridge_port_names(sub_value, MAX_NUM_OF_INTERFACES, MAX_STR_LEN, value[i]))
			{
				j = 0;
				while((j < MAX_NUM_OF_INTERFACES) && (0 != strcmp("", sub_value[j])))
				{
					base_print_debug("%s(): %d. port name is: (%s)\n", __func__, j, sub_value[j]);
					if(EXIT_SUCCESS == get_mstp_port_id(sub_sub_value, MAX_NUM_OF_INTERFACES, MAX_STR_LEN, value[i], sub_value[j]))
					{
						base_print_debug("%s(): %d. port id is: (%s)\n", __func__, j, sub_sub_value[0]);//get_mstp_port_id returns only one string
					}
					else
					{
						base_print_debug("%s(): get_mstp_port_id is FAILED!\n", __func__);
					}
					if(EXIT_SUCCESS == get_cist_port_designated_root(sub_sub_value, MAX_NUM_OF_INTERFACES, MAX_STR_LEN, value[i], sub_value[j]))
					{
						base_print_debug("%s(): %d. get_cist_port_designated_root is: (%s)\n", __func__, j, sub_sub_value[0]);//get_cist_port_designated_root returns only one string
					}
					else
					{
						base_print_debug("%s(): get_cist_port_designated_root is FAILED!\n", __func__);
					}
					if(EXIT_SUCCESS == get_cist_port_top_change_ack(sub_sub_value, MAX_NUM_OF_INTERFACES, MAX_STR_LEN, value[i], sub_value[j]))
					{
						base_print_debug("%s(): %d. get_cist_port_top_change_ack is: (%s)\n", __func__, j, sub_sub_value[0]);//get_cist_port_top_change_ack returns only one string
					}
					else
					{
						base_print_debug("%s(): get_cist_port_top_change_ack is FAILED!\n", __func__);
					}
					if(EXIT_SUCCESS == get_cist_port_hello_time(sub_sub_value, MAX_NUM_OF_INTERFACES, MAX_STR_LEN, value[i], sub_value[j]))
					{
						base_print_debug("%s(): %d. get_cist_port_hello_time is: (%s)\n", __func__, j, sub_sub_value[0]);//get_cist_port_hello_time returns only one string
					}
					else
					{
						base_print_debug("%s(): get_cist_port_hello_time is FAILED!\n", __func__);
					}
					if(EXIT_SUCCESS == get_cist_port_oper_edge_port(sub_sub_value, MAX_NUM_OF_INTERFACES, MAX_STR_LEN, value[i], sub_value[j]))
					{
						base_print_debug("%s(): %d. get_cist_port_oper_edge_port is: (%s)\n", __func__, j, sub_sub_value[0]);//get_cist_port_oper_edge_port returns only one string
					}
					else
					{
						base_print_debug("%s(): get_cist_port_oper_edge_port is FAILED!\n", __func__);
					}
					if(EXIT_SUCCESS == get_cist_port_role(sub_sub_value, MAX_NUM_OF_INTERFACES, MAX_STR_LEN, value[i], sub_value[j]))
					{
						base_print_debug("%s(): %d. get_cist_port_role is: (%s)\n", __func__, j, sub_sub_value[0]);//get_cist_port_role returns only one string
					}
					else
					{
						base_print_debug("%s(): get_cist_port_role is FAILED!\n", __func__);
					}
					if(EXIT_SUCCESS == get_cist_port_disputed(sub_sub_value, MAX_NUM_OF_INTERFACES, MAX_STR_LEN, value[i], sub_value[j]))
					{
						base_print_debug("%s(): %d. get_cist_port_disputed is: (%s)\n", __func__, j, sub_sub_value[0]);//get_cist_port_disputed returns only one string
					}
					else
					{
						base_print_debug("%s(): get_cist_port_disputed is FAILED!\n", __func__);
					}
					if(EXIT_SUCCESS == get_cist_port_reg_root_id(sub_sub_value, MAX_NUM_OF_INTERFACES, MAX_STR_LEN, value[i], sub_value[j]))
					{
						base_print_debug("%s(): %d. get_cist_port_reg_root_id is: (%s)\n", __func__, j, sub_sub_value[0]);//get_cist_port_reg_root_id returns only one string
					}
					else
					{
						base_print_debug("%s(): get_cist_port_reg_root_id is FAILED!\n", __func__);
					}
					if(EXIT_SUCCESS == get_cist_port_path_cost(sub_sub_value, MAX_NUM_OF_INTERFACES, MAX_STR_LEN, value[i], sub_value[j]))
					{
						base_print_debug("%s(): %d. get_cist_port_path_cost is: (%s)\n", __func__, j, sub_sub_value[0]);//get_cist_port_path_cost returns only one string
					}
					else
					{
						base_print_debug("%s(): get_cist_port_path_cost is FAILED!\n", __func__);
					}
					if(EXIT_SUCCESS == get_cist_port_admin_path_cost(sub_sub_value, MAX_NUM_OF_INTERFACES, MAX_STR_LEN, value[i], sub_value[j]))
					{
						base_print_debug("%s(): %d. get_cist_port_admin_path_cost is: (%s)\n", __func__, j, sub_sub_value[0]);//get_cist_port_admin_path_cost returns only one string
					}
					else
					{
						base_print_debug("%s(): get_cist_port_admin_path_cost is FAILED!\n", __func__);
					}
					if(EXIT_SUCCESS == get_cist_port_admin_edge_port(sub_sub_value, MAX_NUM_OF_INTERFACES, MAX_STR_LEN, value[i], sub_value[j]))
					{
						base_print_debug("%s(): %d. get_cist_port_admin_edge_port is: (%s)\n", __func__, j, sub_sub_value[0]);//get_cist_port_admin_edge_port returns only one string
					}
					else
					{
						base_print_debug("%s(): get_cist_port_admin_edge_port is FAILED!\n", __func__);
					}
					if(EXIT_SUCCESS == get_cist_port_mac_enabled(sub_sub_value, MAX_NUM_OF_INTERFACES, MAX_STR_LEN, value[i], sub_value[j]))
					{
						base_print_debug("%s(): %d. get_cist_port_mac_enabled is: (%s)\n", __func__, j, sub_sub_value[0]);//get_cist_port_mac_enabled returns only one string
					}
					if(EXIT_SUCCESS == get_cist_port_admin_point_to_point(sub_sub_value, MAX_NUM_OF_INTERFACES, MAX_STR_LEN, value[i], sub_value[j]))
					{
						base_print_debug("%s(): %d. get_cist_port_admin_point_to_point is: (%s)\n", __func__, j, sub_sub_value[0]);//get_cist_port_mac_enabled returns only one string
					}
					else
					{
						base_print_debug("%s(): get_cist_port_mac_enabled is FAILED!\n", __func__);
					}
					if(EXIT_SUCCESS == get_cist_port_restricted_role(sub_sub_value, MAX_NUM_OF_INTERFACES, MAX_STR_LEN, value[i], sub_value[j]))
					{
						base_print_debug("%s(): %d. get_cist_port_restricted_role is: (%s)\n", __func__, j, sub_sub_value[0]);//get_cist_port_restricted_role returns only one string
					}
					else
					{
						base_print_debug("%s(): get_cist_port_restricted_role is FAILED!\n", __func__);
					}
					if(EXIT_SUCCESS == get_cist_port_restricted_tcn(sub_sub_value, MAX_NUM_OF_INTERFACES, MAX_STR_LEN, value[i], sub_value[j]))
					{
						base_print_debug("%s(): %d. get_cist_port_restricted_tcn is: (%s)\n", __func__, j, sub_sub_value[0]);//get_cist_port_restricted_tcn returns only one string
					}
					else
					{
						base_print_debug("%s(): get_cist_port_restricted_tcn is FAILED!\n", __func__);
					}
					if(EXIT_SUCCESS == get_mstp_port_state(sub_sub_value, MAX_NUM_OF_INTERFACES, MAX_STR_LEN, value[i], sub_value[j], "0"))
					{
						base_print_debug("%s(): %d. get_mstp_port_state is: (%s)\n", __func__, j, sub_sub_value[0]);//get_mstp_port_state returns only one string
					}
					else
					{
						base_print_debug("%s(): get_mstp_port_state is FAILED!\n", __func__);
					}
					if(EXIT_SUCCESS == get_mstp_port_dsgn_root(sub_sub_value, MAX_NUM_OF_INTERFACES, MAX_STR_LEN, value[i], sub_value[j], "0"))
					{
						base_print_debug("%s(): %d. get_mstp_port_dsgn_root is: (%s)\n", __func__, j, sub_sub_value[0]);//get_mstp_port_dsgn_root returns only one string
					}
					else
					{
						base_print_debug("%s(): get_mstp_port_dsgn_root is FAILED!\n", __func__);
					}
					if(EXIT_SUCCESS == get_mstp_port_dsgn_cost(sub_sub_value, MAX_NUM_OF_INTERFACES, MAX_STR_LEN, value[i], sub_value[j], "0"))
					{
						base_print_debug("%s(): %d. get_mstp_port_dsgn_cost is: (%s)\n", __func__, j, sub_sub_value[0]);//get_mstp_port_dsgn_cost returns only one string
					}
					else
					{
						base_print_debug("%s(): get_mstp_port_dsgn_cost is FAILED!\n", __func__);
					}
					if(EXIT_SUCCESS == get_mstp_port_dsgn_bridge(sub_sub_value, MAX_NUM_OF_INTERFACES, MAX_STR_LEN, value[i], sub_value[j], "0"))
					{
						base_print_debug("%s(): %d. get_mstp_port_dsgn_bridge is: (%s)\n", __func__, j, sub_sub_value[0]);//get_mstp_port_dsgn_bridge returns only one string
					}
					else
					{
						base_print_debug("%s(): get_mstp_port_dsgn_bridge is FAILED!\n", __func__);
					}
					if(EXIT_SUCCESS == get_mstp_port_dsgn_port(sub_sub_value, MAX_NUM_OF_INTERFACES, MAX_STR_LEN, value[i], sub_value[j], "0"))
					{
						base_print_debug("%s(): %d. get_mstp_port_dsgn_port is: (%s)\n", __func__, j, sub_sub_value[0]);//get_mstp_port_dsgn_port returns only one string
					}
					else
					{
						base_print_debug("%s(): get_mstp_port_dsgn_port is FAILED!\n", __func__);
					}
					if(EXIT_SUCCESS == get_mstp_port_role(sub_sub_value, MAX_NUM_OF_INTERFACES, MAX_STR_LEN, value[i], sub_value[j], "0"))
					{
						base_print_debug("%s(): %d. get_mstp_port_role is: (%s)\n", __func__, j, sub_sub_value[0]);//get_mstp_port_role returns only one string
					}
					else
					{
						base_print_debug("%s(): get_mstp_port_role is FAILED!\n", __func__);
					}
					if(EXIT_SUCCESS == get_mstp_port_disputed(sub_sub_value, MAX_NUM_OF_INTERFACES, MAX_STR_LEN, value[i], sub_value[j], "0"))
					{
						base_print_debug("%s(): %d. get_mstp_port_disputed is: (%s)\n", __func__, j, sub_sub_value[0]);//get_mstp_port_disputed returns only one string
					}
					else
					{
						base_print_debug("%s(): get_mstp_port_disputed is FAILED!\n", __func__);
					}
					if(EXIT_SUCCESS == get_mstp_port_prioriy(sub_sub_value, MAX_NUM_OF_INTERFACES, MAX_STR_LEN, value[i], sub_value[j], "0"))
					{
						base_print_debug("%s(): %d. get_mstp_port_prioriy is: (%s)\n", __func__, j, sub_sub_value[0]);//get_mstp_port_prioriy returns only one string
					}
					else
					{
						base_print_debug("%s(): get_mstp_port_prioriy is FAILED!\n", __func__);
					}
					if(EXIT_SUCCESS == get_mstp_port_path_cost(sub_sub_value, MAX_NUM_OF_INTERFACES, MAX_STR_LEN, value[i], sub_value[j], "0"))
					{
						base_print_debug("%s(): %d. get_mstp_port_path_cost is: (%s)\n", __func__, j, sub_sub_value[0]);//get_mstp_port_path_cost returns only one string
					}
					else
					{
						base_print_debug("%s(): get_mstp_port_path_cost is FAILED!\n", __func__);
					}
					j++;
				}
			}
			else
			{
				base_print_debug("%s(): get_mstp_bridge_port_names is FAILED!\n", __func__);
			}
			if(EXIT_SUCCESS == get_cist_bridge_comp_id(sub_value, MAX_NUM_OF_INTERFACES, MAX_STR_LEN, value[i]))
			{
				base_print_debug("%s(): %d. bridge id is: (%s)\n", __func__, i, sub_value[0]);//returns only one string
			}
			else
			{
				base_print_debug("%s(): get_cist_bridge_comp_id is FAILED!\n", __func__);
			}
			if(EXIT_SUCCESS == get_cist_bridge_topology_change(sub_value, MAX_NUM_OF_INTERFACES, MAX_STR_LEN, value[i]))
			{
				base_print_debug("%s(): %d. get_cist_bridge_topology_change is: (%s)\n", __func__, i, sub_value[0]);//get_cist_bridge_topology_change returns only one string
			}
			else
			{
				base_print_debug("%s(): get_cist_bridge_topology_change is FAILED!\n", __func__);
			}
			if(EXIT_SUCCESS == get_cist_bridge_reg_root_identifier(sub_value, MAX_NUM_OF_INTERFACES, MAX_STR_LEN, value[i]))
			{
				base_print_debug("%s(): %d. get_cist_bridge_reg_root_identifier is: (%s)\n", __func__, i, sub_value[0]);//returns only one string
			}
			else
			{
				base_print_debug("%s(): get_cist_bridge_reg_root_identifier is FAILED!\n", __func__);
			}
			if(EXIT_SUCCESS == get_cist_bridge_path_cost(sub_value, MAX_NUM_OF_INTERFACES, MAX_STR_LEN, value[i]))
			{
				base_print_debug("%s(): %d. get_cist_bridge_path_cost is: (%s)\n", __func__, i, sub_value[0]);//get_cist_bridge_path_cost returns only one string
			}
			else
			{
				base_print_debug("%s(): get_cist_bridge_path_cost is FAILED!\n", __func__);
			}
			if(EXIT_SUCCESS == get_cist_bridge_priority(sub_value, MAX_NUM_OF_INTERFACES, MAX_STR_LEN, value[i]))
			{
				base_print_debug("%s(): %d. get_cist_bridge_priority is: (%s)\n", __func__, i, sub_value[0]);//get_cist_bridge_path_cost returns only one string
			}
			else
			{
				base_print_debug("%s(): get_cist_bridge_priority is FAILED!\n", __func__);
			}
			if(EXIT_SUCCESS == get_cist_bridge_max_hops(sub_value, MAX_NUM_OF_INTERFACES, MAX_STR_LEN, value[i]))
			{
				base_print_debug("%s(): %d. get_cist_bridge_max_hops is: (%s)\n", __func__, i, sub_value[0]);//get_cist_bridge_max_hops returns only one string
			}
			else
			{
				base_print_debug("%s(): get_cist_bridge_max_hops is FAILED!\n", __func__);
			}
			if(EXIT_SUCCESS == get_mstp_bridge_id(sub_value, MAX_NUM_OF_INTERFACES, MAX_STR_LEN, value[i], "0"))
			{
				base_print_debug("%s(): %d. get_mstp_bridge_id is: (%s)\n", __func__, i, sub_value[0]);//get_mstp_bridge_id returns only one string
			}
			else
			{
				base_print_debug("%s(): get_mstp_bridge_id is FAILED!\n", __func__);
			}
			if(EXIT_SUCCESS == get_mstp_time_since_top_change(sub_value, MAX_NUM_OF_INTERFACES, MAX_STR_LEN, value[i], "0"))
			{
				base_print_debug("%s(): %d. get_mstp_time_since_top_change is: (%s)\n", __func__, i, sub_value[0]);//get_mstp_time_since_top_change returns only one string
			}
			else
			{
				base_print_debug("%s(): get_mstp_time_since_top_change is FAILED!\n", __func__);
			}
			if(EXIT_SUCCESS == get_mstp_top_changes(sub_value, MAX_NUM_OF_INTERFACES, MAX_STR_LEN, value[i], "0"))
			{
				base_print_debug("%s(): %d. get_mstp_top_changes is: (%s)\n", __func__, i, sub_value[0]);//get_mstp_top_changes returns only one string
			}
			else
			{
				base_print_debug("%s(): get_mstp_top_changes is FAILED!\n", __func__);
			}
			if(EXIT_SUCCESS == get_mstp_designated_root(sub_value, MAX_NUM_OF_INTERFACES, MAX_STR_LEN, value[i]))
			{
				base_print_debug("%s(): %d. get_mstp_designated_root is: (%s)\n", __func__, i, sub_value[0]);//get_mstp_designated_root returns only one string
			}
			else
			{
				base_print_debug("%s(): get_mstp_designated_root is FAILED!\n", __func__);
			}
			if(EXIT_SUCCESS == get_mstp_root_path_cost(sub_value, MAX_NUM_OF_INTERFACES, MAX_STR_LEN, value[i], "0"))
			{
				base_print_debug("%s(): %d. get_mstp_root_path_cost is: (%s)\n", __func__, i, sub_value[0]);//get_mstp_root_path_cost returns only one string
			}
			else
			{
				base_print_debug("%s(): get_mstp_root_path_cost is FAILED!\n", __func__);
			}
			if(EXIT_SUCCESS == get_mstp_root_port(sub_value, MAX_NUM_OF_INTERFACES, MAX_STR_LEN, value[i], "0"))
			{
				base_print_debug("%s(): %d. get_mstp_root_port is: (%s)\n", __func__, i, sub_value[0]);//get_mstp_root_port returns only one string
			}
			else
			{
				base_print_debug("%s(): get_mstp_root_port is FAILED!\n", __func__);
			}
			if(EXIT_SUCCESS == get_mstp_config_id_format_selector(sub_value, MAX_NUM_OF_INTERFACES, MAX_STR_LEN, value[i]))
			{
				base_print_debug("%s(): %d. get_mstp_config_id_format_selector is: (%s)\n", __func__, i, sub_value[0]);//get_mstp_config_id_format_selector returns only one string
			}
			else
			{
				base_print_debug("%s(): get_mstp_config_id_format_selector is FAILED!\n", __func__);
			}
			if(EXIT_SUCCESS == get_mstp_configuration_name(sub_value, MAX_NUM_OF_INTERFACES, MAX_STR_LEN, value[i]))
			{
				base_print_debug("%s(): %d. get_mstp_configuration_name is: (%s)\n", __func__, i, sub_value[0]);//get_mstp_configuration_name returns only one string
			}
			else
			{
				base_print_debug("%s(): get_mstp_configuration_name is FAILED!\n", __func__);
			}
			if(EXIT_SUCCESS == get_mstp_revision_level(sub_value, MAX_NUM_OF_INTERFACES, MAX_STR_LEN, value[i]))
			{
				base_print_debug("%s(): %d. get_mstp_revision_level is: (%s)\n", __func__, i, sub_value[0]);//get_mstp_revision_level returns only one string
			}
			else
			{
				base_print_debug("%s(): get_mstp_revision_level is FAILED!\n", __func__);
			}
			if(EXIT_SUCCESS == get_mstp_bridge_priority(sub_value, MAX_NUM_OF_INTERFACES, MAX_STR_LEN, value[i], "0"))
			{
				base_print_debug("%s(): %d. get_mstp_bridge_priority is: (%s)\n", __func__, i, sub_value[0]);//get_mstp_bridge_priority returns only one string
			}
			else
			{
				base_print_debug("%s(): get_mstp_bridge_priority is FAILED!\n", __func__);
			}
			if(EXIT_SUCCESS == get_bridge_mstpi(sub_value, MAX_NUM_OF_INTERFACES, MAX_STR_LEN, value[i]))
			{
				base_print_debug("%s(): %d. get_bridge_mstpi is: (%s)\n", __func__, i, sub_value[0]);//get_bridge_mstpi returns only one string
			}
			else
			{
				base_print_debug("%s(): get_bridge_mstpi is FAILED!\n", __func__);
			}
			i++;
		}
	}
	else
	{
		base_print_debug("%s(): get_mstp_bridge_names is FAILED!\n", __func__);
	}
}

static void interfaces_demonstration()
{
	char value[MAX_NUM_OF_INTERFACES][MAX_STR_LEN];
	int i = 0;
	uint32_t ret_val = 0x00000000;

	base_print_debug(DEBUG_MSG_FUN_CALLED, __func__);

	//***********************************   IETF-INTERFACES   ******************************************
	if(EXIT_SUCCESS == get_interface_names(value, MAX_NUM_OF_INTERFACES, MAX_STR_LEN))
	{
		while((i < MAX_NUM_OF_INTERFACES) && (0 != strcmp("", value[i])))
		{
			base_print_debug("%s(): %d. port name is: (%s)\n", __func__, i, value[i]);
			i++;
		}
	}
	else
	{
		base_print_debug("%s(): get_interface_names is FAILED!\n", __func__);
	}
	if(EXIT_SUCCESS == set_interface_enabled("sw0p0", "down"))
	{
		base_print_debug("%s(): set_interface_enabled is PASSED!\n", __func__);
	}
	else
	{
		base_print_debug("%s(): set_interface_enabled is FAILED!\n", __func__);
	}
	if(EXIT_SUCCESS == get_interface_enabled(value, MAX_NUM_OF_INTERFACES, MAX_STR_LEN, "sw0p0"))
	{
		base_print_debug("%s(): %d. (sw0p0) enabled is: (%s)\n", __func__, i, value[i]);
	}
	else
	{
		base_print_debug("%s(): get_interface_enabled is FAILED!\n", __func__);
	}

	if(EXIT_SUCCESS == set_interface_enabled("sw0p0", "up"))
	{
		base_print_debug("%s(): set_interface_enabled is PASSED!\n", __func__);
	}
	else
	{
		base_print_debug("%s(): set_interface_enabled is FAILED!\n", __func__);
	}
	if(EXIT_SUCCESS == get_interface_enabled(value, MAX_NUM_OF_INTERFACES, MAX_STR_LEN, "sw0p0"))
	{
		base_print_debug("%s(): %d. (sw0p0) enabled is: (%s)\n", __func__, i, value[i]);
	}
	else
	{
		base_print_debug("%s(): get_interface_enabled is FAILED!\n", __func__);
	}

	if(EXIT_SUCCESS == get_eth_if_auto_neg_enable(value, MAX_NUM_OF_INTERFACES, MAX_STR_LEN, "sw0p0"))
	{
		i = 0;
		while((i < MAX_NUM_OF_INTERFACES) && (0 != strcmp("", value[i])))
		{
			base_print_debug("%s(): %d. (sw0p0) enabled is: (%s)\n", __func__, i, value[i]);
			i++;
		}
	}
	else
	{
		base_print_debug("%s(): get_eth_if_auto_neg_enable is FAILED!\n", __func__);
	}
	if(EXIT_SUCCESS == get_eth_if_duplex(value, MAX_NUM_OF_INTERFACES, MAX_STR_LEN, "sw0p0"))
	{
		i = 0;
		while((i < MAX_NUM_OF_INTERFACES) && (0 != strcmp("", value[i])))
		{
			base_print_debug("%s(): %d. (sw0p0) duplex is: (%s)\n", __func__, i, value[i]);
			i++;
		}
	}
	else
	{
		base_print_debug("%s(): get_eth_if_duplex is FAILED!\n", __func__);
	}
	if(EXIT_SUCCESS == get_eth_if_speed(value, MAX_NUM_OF_INTERFACES, MAX_STR_LEN, "ens33"))
	{
		i = 0;
		while((i < MAX_NUM_OF_INTERFACES) && (0 != strcmp("", value[i])))
		{
			base_print_debug("%s(): %d. (ens33) SPEED is: (%s)\n", __func__, i, value[i]);
			i++;
		}
	}
	else
	{
		base_print_debug("%s(): get_eth_if_speed is FAILED!\n", __func__);
	}
	if(EXIT_SUCCESS == get_ethernet_max_frame_length(value, MAX_NUM_OF_INTERFACES, MAX_STR_LEN, "ens33"))
	{
		i = 0;
		while((i < MAX_NUM_OF_INTERFACES) && (0 != strcmp("", value[i])))
		{
			base_print_debug("%s(): %d. (ens33) max frame length is: (%s)\n", __func__, i, value[i]);
			i++;
		}
	}
	else
	{
		base_print_debug("%s(): get_ethernet_max_frame_length is FAILED!\n", __func__);
	}
	
	if(EXIT_SUCCESS == get_interface_tx_statistics(value, MAX_NUM_OF_INTERFACES, MAX_STR_LEN, "ens33"))
	{
		i = 0;
		while((i < MAX_NUM_OF_INTERFACES) && (0 != strcmp("", value[i])))
		{
			base_print_debug("%s(): %d. (ens33) interface TX statistics is: (%s)\n", __func__, i, value[i]);
			i++;
		}
	}
	else
	{
		base_print_debug("%s(): get_interface_tx_statistics is FAILED!\n", __func__);
	}
	
	if(EXIT_SUCCESS == get_interface_rx_statistics(value, MAX_NUM_OF_INTERFACES, MAX_STR_LEN, "sw0p0"))
	{
		i = 0;
		while((i < MAX_NUM_OF_INTERFACES) && (0 != strcmp("", value[i])))
		{
			base_print_debug("%s(): %d. (ens33) interface RX statistics is: (%s)\n", __func__, i, value[i]);
			i++;
		}
	}
	else
	{
		base_print_debug("%s(): get_interface_Rx_statistics is FAILED!\n", __func__);
	}
	if(EXIT_SUCCESS == get_eth_link_detected(value, MAX_NUM_OF_INTERFACES, MAX_STR_LEN, "ens33"))
	{
		i = 0;
		while((i < MAX_NUM_OF_INTERFACES) && (0 != strcmp("", value[i])))
		{
			base_print_debug("%s(): %d. (ens33) Link detected is: (%s)\n", __func__, i, value[i]);
			i++;
		}
	}
	else
	{
		base_print_debug("%s(): get_eth_link_detected is FAILED!\n", __func__);
	}

	if(EXIT_SUCCESS == store_advertised_link_modes(value, MAX_NUM_OF_INTERFACES, MAX_STR_LEN, "ens33"))
	{
		base_print_debug("%s(): get_advertised_link_modes is SUCCEEDED!\n", __func__);
		i = 0;
		while((i < MAX_NUM_OF_INTERFACES) && (0 != strcmp("", value[i])))
		{
			base_print_debug("%s(): %d. (ens33) Advertised link mode is: (%s)\n", __func__, i, value[i]);
			i++;
		}
	}
	else
	{
		base_print_debug("%s(): store_advertised_link_modes is FAILED!\n", __func__);
	}

	if(EXIT_SUCCESS == get_supported_link_modes(value, MAX_NUM_OF_INTERFACES, MAX_STR_LEN, "ens33"))
	{
		base_print_debug("%s(): get_supported_link_modes is SUCCEEDED!\n", __func__);
		i = 0;
		while((i < MAX_NUM_OF_INTERFACES) && (0 != strcmp("", value[i])))
		{
			base_print_debug("%s(): %d. (ens33) Supported link mode is: (%s)\n", __func__, i, value[i]);
			i++;
		}
	}
	else
	{
		base_print_debug("%s(): get_supported_link_modes is FAILED!\n", __func__);
	}

	ret_val = set_interface_link_modes("ens33", 1);
	if(0 != ret_val)
	{
		base_print_debug("%s(): link_modes for ens33 is '%x'!\n", __func__, ret_val);
	}
	else
	{
		base_print_debug("%s(): check_link_modes is FAILED!\n", __func__);
	}

#if 0
	if(EXIT_SUCCESS == get_q_bridge_port_pvid(value, MAX_NUM_OF_INTERFACES, MAX_STR_LEN, "sw0p0"))
	{
		base_print_debug("%s(): get_q_bridge_port_pvid is SUCCEEDED - value (%s)!\n", __func__, value[0]);
	}
	else
	{
		base_print_debug("%s(): get_q_bridge_port_pvid is FAILED!\n", __func__);
	}
	if(EXIT_SUCCESS == set_q_bridge_port_pvid("sw0p0", "2"))
	{
		base_print_debug("%s(): set_q_bridge_port_pvid is SUCCEEDED!\n", __func__);
	}
	else
	{
		base_print_debug("%s(): set_q_bridge_port_pvid is FAILED!\n", __func__);
	}
#endif
}

static void q_bridge_demonstration()
{
	char value[MAX_NUM_OF_INTERFACES][MAX_STR_LEN];
	char vlan_value[BASE_MAX_VID][BASE_TEN];
	int i = 0;

	base_print_debug(DEBUG_MSG_FUN_CALLED, __func__);

	if(EXIT_SUCCESS == get_bridge_port_names(value, MAX_NUM_OF_INTERFACES, MAX_STR_LEN))
	{
		i = 0;
		while((i < MAX_NUM_OF_INTERFACES) && (0 != strcmp("", value[i])))
		{
			base_print_debug("%s(): %d. bridge port name is: (%s)\n", __func__, i, value[i]);
			i++;
		}
	}
	else
	{
		base_print_debug("%s(): get_bridge_port_names is FAILED!\n", __func__);
	}
	if(EXIT_SUCCESS == get_bridge_names(value, MAX_NUM_OF_INTERFACES, MAX_STR_LEN))
	{
		i = 0;
		while((i < MAX_NUM_OF_INTERFACES) && (0 != strcmp("", value[i])))
		{
			base_print_debug("%s(): %d. bridge name is: (%s)\n", __func__, i, value[i]);
			i++;
		}
	}
	else
	{
		base_print_debug("%s(): get_bridge_names is FAILED!\n", __func__);
	}
	if(EXIT_SUCCESS == get_bridge_uptime(value, MAX_NUM_OF_INTERFACES, MAX_STR_LEN))
	{
		i = 0;
		while((i < MAX_NUM_OF_INTERFACES) && (0 != strcmp("", value[i])))
		{
			base_print_debug("%s(): %d. bridge up-time is: (%s)\n", __func__, i, value[i]);
			i++;
		}
	}
	else
	{
		base_print_debug("%s(): get_bridge_uptime is FAILED!\n", __func__);
	}
	if(EXIT_SUCCESS == get_interface_if_index(value, MAX_NUM_OF_INTERFACES, MAX_STR_LEN, "sw0p0"))
	{
		i = 0;
		while((i < MAX_NUM_OF_INTERFACES) && (0 != strcmp("", value[i])))
		{
			base_print_debug("%s(): %d. interface index is: (%s)\n", __func__, i, value[i]);
			i++;
		}
	}
	else
	{
		base_print_debug("%s(): get_interface_if_index is FAILED!\n", __func__);
	}


	if(EXIT_SUCCESS == get_interface_address(value, MAX_NUM_OF_INTERFACES, MAX_STR_LEN, "sw0p3"))
	{
		i = 0;
		while((i < MAX_NUM_OF_INTERFACES) && (0 != strcmp("", value[i])))
		{
			base_print_debug("%s(): %d. interface address is: (%s)\n", __func__, i, value[i]);
			i++;
		}
	}
	else
	{
		base_print_debug("%s(): get_interface_address is FAILED!\n", __func__);
	}
	if(EXIT_SUCCESS == get_state_comp_dyn_address(value, MAX_NUM_OF_INTERFACES, MAX_STR_LEN, "br0"))
	{
		i = 0;
		while((i < MAX_NUM_OF_INTERFACES) && (0 != strcmp("", value[i])))
		{
			base_print_debug("%s(): %d. component dynamic vlan address is: (%s)\n", __func__, i, value[i]);
			i++;
		}
	}
	else
	{
		base_print_debug("%s(): get_state_comp_dyn_address is FAILED!\n", __func__);
	}

	if(EXIT_SUCCESS == get_control_element(value, MAX_NUM_OF_INTERFACES, MAX_STR_LEN, "sw0p2"))
	{
		i = 0;
		while((i < MAX_NUM_OF_INTERFACES) && (0 != strcmp("", value[i])))
		{
			base_print_debug("%s(): %d. control element is: (%s)\n", __func__, i, value[i]);
			i++;
		}
	}
	else
	{
		base_print_debug("%s(): get_control_element is FAILED!\n", __func__);
	}

	if(EXIT_SUCCESS == get_connection_identifier(value, MAX_NUM_OF_INTERFACES, MAX_STR_LEN, "sw0p2"))
	{
		i = 0;
		while((i < MAX_NUM_OF_INTERFACES) && (0 != strcmp("", value[i])))
		{
			base_print_debug("%s(): %d. connection id is: (%s)\n", __func__, i, value[i]);
			i++;
		}
	}
	else
	{
		base_print_debug("%s(): get_connection_identifier is FAILED!\n", __func__);
	}

	if(EXIT_SUCCESS == get_bridge_mstpi(value, MAX_NUM_OF_INTERFACES, MAX_STR_LEN, "br0"))
	{
		i = 0;
		while((i < MAX_NUM_OF_INTERFACES) && (0 != strcmp("", value[i])))
		{
			base_print_debug("%s(): %d. brdige msti id is: (%s)\n", __func__, i, value[i]);
			i++;
		}
	}
	else
	{
		base_print_debug("%s(): get_bridge_mstpi is FAILED!\n", __func__);
	}

	if(EXIT_SUCCESS == bridge_vlan_mod_dev_vid("sw0p0", "5,6-10,18,20-25", Q_BRIDGE_ADD_DEV_VID_STR, "untagged"))
	{
		base_print_debug("%s(): bridge_vlan_mod_dev_vid is PASSED!\n", __func__);
	}
	else
	{
		base_print_debug("%s(): bridge_vlan_mod_dev_vid is FAILED!\n", __func__);
	}

	if(EXIT_SUCCESS == bridge_vlan_mod_dev_vid("sw0p0", "6-10,20-22", Q_BRIDGE_DEL_DEV_VID_STR, "untagged"))
	{
		base_print_debug("%s(): bridge_vlan_mod_dev_vid is PASSED!\n", __func__);
	}
	else
	{
		base_print_debug("%s(): bridge_vlan_mod_dev_vid is FAILED!\n", __func__);
	}


	if(EXIT_SUCCESS == bridge_fdb_rem_dev_filt_entry("sw0p0", "da:f8:ba:2e:a1:00", "2-5"))
	{
		base_print_debug("%s(): bridge_fdb_rem_dev_filt_entry is PASSED!\n", __func__);
	}
	else
	{
		base_print_debug("%s(): bridge_fdb_rem_dev_filt_entry is FAILED!\n", __func__);
	}

	if(EXIT_SUCCESS == get_bridge_fids(value, MAX_NUM_OF_INTERFACES, MAX_STR_LEN, "br0", "1"))
	{
		i = 0;
		while((i < MAX_NUM_OF_INTERFACES) && (0 != strcmp("", value[i])))
		{
			base_print_debug("%s(): %d. brdige fids is: (%s)\n", __func__, i, value[i]);
			i++;
		}
	}
	else
	{
		base_print_debug("%s(): get_bridge_fids is FAILED!\n", __func__);
	}
	if(EXIT_SUCCESS == get_vlan_transmitted(value, MAX_NUM_OF_INTERFACES, MAX_STR_LEN, "sw0p3", "1"))
	{
		i = 0;
		while((i < MAX_NUM_OF_INTERFACES) && (0 != strcmp("", value[i])))
		{
			base_print_debug("%s(): %d. sw0p3 transmitted is: (%s)\n", __func__, i, value[i]);
			i++;
		}
	}
	else
	{
		base_print_debug("%s(): get_vlan_transmitted is FAILED!\n", __func__);
	}
	if(EXIT_SUCCESS == get_bridge_fids(value, MAX_NUM_OF_INTERFACES, MAX_STR_LEN, "br0", "4"))
	{
		i = 0;
		while((i < MAX_NUM_OF_INTERFACES) && (0 != strcmp("", value[i])))
		{
			base_print_debug("%s(): %d. brdige fid is: (%s)\n", __func__, i, value[i]);
			i++;
		}
	}
	else
	{
		base_print_debug("%s(): get_bridge_fids is FAILED!\n", __func__);
	}
	if(EXIT_SUCCESS == get_bridge_fid_by_mstid(value, MAX_NUM_OF_INTERFACES, MAX_STR_LEN, "br0", "4"))
	{
		i = 0;
		while((i < MAX_NUM_OF_INTERFACES) && (0 != strcmp("", value[i])))
		{
			base_print_debug("%s(): %d. brdige fid by MSTID is: (%s)\n", __func__, i, value[i]);
			i++;
		}
	}
	else
	{
		base_print_debug("%s(): get_bridge_fid_by_mstid is FAILED!\n", __func__);
	}
	if(EXIT_SUCCESS == get_mac_address_entries(value, MAX_NUM_OF_INTERFACES, MAX_STR_LEN, "br0"))
	{
		i = 0;
		while((i < MAX_NUM_OF_INTERFACES) && (0 != strcmp("", value[i])))
		{
			base_print_debug("%s(): %d. mac address is: (%s)\n", __func__, i, value[i]);
			i++;
		}
	}
	else
	{
		base_print_debug("%s(): get_mac_address_entries is FAILED!\n", __func__);
	}
	if(EXIT_SUCCESS == get_untagged_port_names(value, MAX_NUM_OF_INTERFACES, MAX_STR_LEN, "sw0p0", "1"))
	{
		i = 0;
		while((i < MAX_NUM_OF_INTERFACES) && (0 != strcmp("", value[i])))
		{
			base_print_debug("%s(): %d. untagged port is: (%s)\n", __func__, i, value[i]);
			i++;
		}
	}
	else
	{
		base_print_debug("%s(): get_untagged_port_names is FAILED!\n", __func__);
	}

	if(EXIT_SUCCESS == get_egress_port_names(value, MAX_NUM_OF_INTERFACES, MAX_STR_LEN, "sw0p0", "1"))
	{
		base_print_debug("%s(): get_egress_port_names is PASSED!\n", __func__);
	}
	else
	{
		base_print_debug("%s(): get_egress_port_names is FAILED!\n", __func__);
	}
	char vids_array[BASE_MAX_VID][BASE_TEN];
	assemble_vids_as_array(vids_array, BASE_MAX_VID, BASE_TEN, "1,10,100-120");
	i = 0;
	while((i < BASE_MAX_VID) && (0 != strcmp("", vids_array[i])))
	{
		base_print_debug("%s(): %d. port is: (%s)\n", __func__, i, vids_array[i]);
		i++;
	}

	if(EXIT_SUCCESS == get_interface_name_by_id(value, MAX_NUM_OF_INTERFACES, MAX_STR_LEN, "7"))
	{
		i = 0;
		while((i < MAX_NUM_OF_INTERFACES) && (0 != strcmp("", value[i])))
		{
			base_print_debug("%s(): %d. interface name for port id[7] is: (%s)\n", __func__, i, value[i]);
			i++;
		}
	}
	else
	{
		base_print_debug("%s(): get_interface_name_by_id is FAILED!\n", __func__);
	}

	if(EXIT_SUCCESS == get_bridge_ports_name_by_vlan(value, MAX_NUM_OF_INTERFACES, MAX_STR_LEN, "1", "br0"))
	{
		i = 0;
		while((i < MAX_NUM_OF_INTERFACES) && (0 != strcmp("", value[i])))
		{
			base_print_debug("%s(): %d. bridge port name by VLAN is: (%s)\n", __func__, i, value[i]);
			i++;
		}
	}
	else
	{
		base_print_debug("%s(): get_bridge_ports_name_by_vlan is FAILED!\n", __func__);
	}

	if(EXIT_SUCCESS == get_port_vlans_by_name(vlan_value, BASE_MAX_VID, BASE_TEN, 0, "sw0p1"))
	{
		i = 0;
		while((i < BASE_MAX_VID) && (0 != strcmp("", vlan_value[i])))
		{
			base_print_debug("%s(): %d. bridge port vlans by name is: (%s)\n", __func__, i, vlan_value[i]);
			i++;
		}
	}
	else
	{
		base_print_debug("%s(): get_port_vlans_by_name is FAILED!\n", __func__);
	}

	if(EXIT_SUCCESS == get_bridge_vlans(vlan_value, BASE_MAX_VID, BASE_TEN, 1))
	{
		i = 0;
		while((i < BASE_MAX_VID) && (0 != strcmp("", vlan_value[i])))
		{
			base_print_debug("%s(): %d. bridge vlans is: (%s)\n", __func__, i, vlan_value[i]);
			i++;
		}
	}
	else
	{
		base_print_debug("%s(): get_bridge_vlans is FAILED!\n", __func__);
	}

	if(EXIT_SUCCESS == get_vlan_transmitted(value, MAX_NUM_OF_INTERFACES, MAX_STR_LEN, "sw0p0", "30"))
	{
		i = 0;
		while((i < MAX_NUM_OF_INTERFACES) && (0 != strcmp("", value[i])))
		{
			base_print_debug("%s(): %d. sw0p0, vlan '30', transmitted is: (%s)\n", __func__, i, value[i]);
			i++;
		}
	}
	else
	{
		base_print_debug("%s(): get_vlan_transmitted is FAILED!\n", __func__);
	}

	if(EXIT_SUCCESS == get_vlan_transmitted(value, MAX_NUM_OF_INTERFACES, MAX_STR_LEN, "sw0p0", "40-43"))
	{
		i = 0;
		while((i < MAX_NUM_OF_INTERFACES) && (0 != strcmp("", value[i])))
		{
			base_print_debug("%s(): %d. sw0p0, vlan '40-43', transmitted is: (%s)\n", __func__, i, value[i]);
			i++;
		}
	}
	else
	{
		base_print_debug("%s(): get_vlan_transmitted is FAILED!\n", __func__);
	}

}

int main(void)
{
	base_print_debug(DEBUG_MSG_FUN_CALLED, __func__);

	mstp_demonstration();
	interfaces_demonstration();
	q_bridge_demonstration();
	
    return 0;
}
