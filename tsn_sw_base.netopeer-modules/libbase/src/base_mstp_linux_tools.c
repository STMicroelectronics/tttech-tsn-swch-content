/*
 * TTTech libbase
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* libbase includes */
#include "base_defines.h"
#include "base_prints.h"
#include "base_linux_tools.h"
#include "base_linux_tool_helpers.h"
#include "base_mstp_linux_tools.h"
#include "base_private_defines.h"

int mstp_daemon_check_status(void *result_array, unsigned int row_num, unsigned int col_num)
{
    return get_object(MSTP, MSTPD_STR, BASE_RC_NO, result_array, row_num, col_num, "");
}

int get_mstp_bridge_names(void *result_array, unsigned int row_num, unsigned int col_num)
{
    return get_object(MSTP, MSTP_BRIDGE_NAMES_STR, BASE_RC_NO,
                      result_array, row_num, col_num, "");
}

int get_cist_bridge_comp_id(void *result_array, unsigned int row_num, unsigned int col_num,
                            char *bridge_name)
{
    return get_object(MSTP, MSTP_CIST_BRIDGE_ID_STR, BASE_RC_NO,
                      result_array, row_num, col_num,
                      bridge_name);
}

int get_cist_bridge_topology_change(void *result_array, unsigned int row_num, unsigned int col_num,
                                    char *bridge_name)
{
    return get_object(MSTP, MSTP_CIST_TOP_CHANGE_STR, BASE_RC_NO,
                      result_array, row_num, col_num,
                      bridge_name);
}

int get_cist_bridge_reg_root_identifier(void *result_array, unsigned int row_num, unsigned int col_num,
                                        char *bridge_name)
{
    return get_object(MSTP, MSTP_CIST_REG_ROOT_ID_STR, BASE_RC_NO,
                      result_array, row_num, col_num,
                      bridge_name);
}

int get_cist_bridge_path_cost(void *result_array, unsigned int row_num, unsigned int col_num,
                              char *bridge_name)
{
    return get_object(MSTP, MSTP_CIST_PATH_COST_STR, BASE_RC_NO,
                      result_array, row_num, col_num,
                      bridge_name);
}

int get_cist_bridge_priority(void *result_array, unsigned int row_num, unsigned int col_num,
                             char *bridge_name)
{
    return get_object(MSTP, MSTP_CIST_BRIDGE_PRIO_STR, BASE_RC_NO,
                      result_array, row_num, col_num,
                      bridge_name);
}

int get_cist_bridge_max_hops(void *result_array, unsigned int row_num, unsigned int col_num,
                             char *bridge_name)
{
    return get_object(MSTP, MSTP_CIST_MAX_HOPS_STR, BASE_RC_NO,
                      result_array, row_num, col_num,
                      bridge_name);
}

int get_mstp_bridge_port_names(void *result_array, unsigned int row_num, unsigned int col_num,
                               char *bridge_name)
{
    return get_object(MSTP, MSTP_PORT_NAMES_STR, BASE_RC_NO,
                      result_array, row_num, col_num,
                      bridge_name);
}

int get_mstp_port_id(void *result_array, unsigned int row_num, unsigned int col_num,
                     char *bridge_name, char *port_name)
{
    return get_object(MSTP, MSTP_PORT_NUM_STR, BASE_RC_NO,
                      result_array, row_num, col_num,
                      bridge_name, port_name);
}

int get_cist_port_admin_edge_port(void *result_array, unsigned int row_num, unsigned int col_num,
                                  char *bridge_name, char *port_name)
{
    return get_object(MSTP, MSTP_CIST_PORT_ADMIN_EDGE_PORT_STR, BASE_RC_NO,
                      result_array, row_num, col_num,
                      bridge_name, port_name);
}

int get_cist_port_admin_path_cost(void *result_array, unsigned int row_num, unsigned int col_num,
                                  char *bridge_name, char *port_name)
{
    return get_object(MSTP, MSTP_CIST_PORT_ADMIN_PATH_COST_STR, BASE_RC_NO,
                      result_array, row_num, col_num,
                      bridge_name, port_name);
}

int get_cist_port_mac_enabled(void *result_array, unsigned int row_num, unsigned int col_num,
                              char *bridge_name, char *port_name)
{
    return get_object(MSTP, MSTP_CIST_PORT_MAC_ENABLED_STR, BASE_RC_NO,
                      result_array, row_num, col_num,
                      bridge_name, port_name);
}

int get_cist_port_restricted_role(void *result_array, unsigned int row_num, unsigned int col_num,
                                  char *bridge_name, char *port_name)
{
    return get_object(MSTP, MSTP_CIST_PORT_RESTR_ROLE_STR, BASE_RC_NO,
                      result_array, row_num, col_num,
                      bridge_name, port_name);
}

int get_cist_port_restricted_tcn(void *result_array, unsigned int row_num, unsigned int col_num,
                                 char *bridge_name, char *port_name)
{
    return get_object(MSTP, MSTP_CIST_PORT_RESTR_TCN_STR, BASE_RC_NO,
                      result_array, row_num, col_num,
                      bridge_name, port_name);
}

int get_cist_port_designated_root(void *result_array, unsigned int row_num, unsigned int col_num,
                                  char *bridge_name, char *port_name)
{
    return get_object(MSTP, MSTP_CIST_PORT_DSGN_ROOT_STR, BASE_RC_NO,
                      result_array, row_num, col_num,
                      bridge_name, port_name);
}

int get_cist_port_top_change_ack(void *result_array, unsigned int row_num, unsigned int col_num,
                                 char *bridge_name, char *port_name)
{
    return get_object(MSTP, MSTP_CIST_PORT_TOP_CHANGE_ACK_STR, BASE_RC_NO,
                      result_array, row_num, col_num,
                      bridge_name, port_name);
}

int get_cist_port_hello_time(void *result_array, unsigned int row_num, unsigned int col_num,
                             char *bridge_name, char *port_name)
{
    return get_object(MSTP, MSTP_CIST_PORT_HELLO_TIME_STR, BASE_RC_NO,
                      result_array, row_num, col_num,
                      bridge_name, port_name);
}

int get_cist_port_oper_edge_port(void *result_array, unsigned int row_num, unsigned int col_num,
                                 char *bridge_name, char *port_name)
{
    return get_object(MSTP, MSTP_CIST_PORT_OPER_EDGE_PORT_STR, BASE_RC_NO,
                      result_array, row_num, col_num,
                      bridge_name, port_name);
}

int get_cist_port_role(void *result_array, unsigned int row_num, unsigned int col_num,
                       char *bridge_name, char *port_name)
{
    return get_object(MSTP, MSTP_CIST_PORT_PORT_ROLE_STR, BASE_RC_NO,
                      result_array, row_num, col_num,
                      bridge_name, port_name);
}

int get_cist_port_disputed(void *result_array, unsigned int row_num, unsigned int col_num,
                           char *bridge_name, char *port_name)
{
    return get_object(MSTP, MSTP_CIST_PORT_DISPUTED_STR, BASE_RC_NO,
                      result_array, row_num, col_num,
                      bridge_name, port_name);
}

int get_cist_port_reg_root_id(void *result_array, unsigned int row_num, unsigned int col_num,
                              char *bridge_name, char *port_name)
{
    return get_object(MSTP, MSTP_CIST_PORT_REG_ROOT_ID_STR, BASE_RC_NO,
                      result_array, row_num, col_num,
                      bridge_name, port_name);
}

int get_cist_port_path_cost(void *result_array, unsigned int row_num, unsigned int col_num,
                            char *bridge_name, char *port_name)
{
    return get_object(MSTP, MSTP_CIST_PORT_PATH_COST_STR, BASE_RC_NO,
                      result_array, row_num, col_num,
                      bridge_name, port_name);
}

int get_bridge_mstpi(void *result_array, unsigned int row_num, unsigned int col_num,
                     char *bridge_name)
{
    return get_object(MSTP, MSTP_ID_STR, BASE_RC_NO,
                      result_array, row_num, col_num,
                      bridge_name);
}
/* ieee8021MstpCistBridgeIdentifier */
int get_mstp_bridge_priority(void *result_array, unsigned int row_num, unsigned int col_num,
                             char *bridge_name, char *mstpi)
{
    return get_object(MSTP, MSTP_BRIDGE_PRIORITY_STR, BASE_RC_NO,
                      result_array, row_num, col_num,
                      bridge_name, mstpi);
}

int get_mstp_bridge_id(void *result_array, unsigned int row_num, unsigned int col_num,
                       char *bridge_name, char *mstpi)
{
    return get_object(MSTP, MSTP_BRIDGE_ID_STR, BASE_RC_NO,
                      result_array, row_num, col_num,
                      bridge_name, mstpi);
}

int get_mstp_time_since_top_change(void *result_array, unsigned int row_num, unsigned int col_num,
                                   char *bridge_name, char *mstpi)
{
    return get_object(MSTP, MSTP_SINCE_TOP_CHANGE_STR, BASE_RC_NO,
                      result_array, row_num, col_num,
                      bridge_name, mstpi);
}

int get_mstp_top_changes(void *result_array, unsigned int row_num, unsigned int col_num,
                         char *bridge_name, char *mstpi)
{
    return get_object(MSTP, MSTP_TOP_CHANGES_STR, BASE_RC_NO,
                      result_array, row_num, col_num,
                      bridge_name, mstpi);
}

int get_mstp_designated_root(void *result_array, unsigned int row_num, unsigned int col_num,
                             char *bridge_name)
{
    return get_object(MSTP, MSTP_DSGN_ROOT_STR, BASE_RC_NO,
                      result_array, row_num, col_num,
                      bridge_name);
}

int get_mstp_root_path_cost(void *result_array, unsigned int row_num, unsigned int col_num,
                            char *bridge_name, char *mstpi)
{
    return get_object(MSTP, MSTP_ROOT_PATH_COST_STR, BASE_RC_NO,
                      result_array, row_num, col_num,
                      bridge_name, mstpi);
}

int get_mstp_root_port(void *result_array, unsigned int row_num, unsigned int col_num,
                       char *bridge_name, char *mstpi)
{
    return get_object(MSTP, MSTP_ROOT_PORT_STR, BASE_RC_NO,
                      result_array, row_num, col_num,
                      bridge_name, mstpi);
}

int get_mstp_port_prioriy(void *result_array, unsigned int row_num, unsigned int col_num,
                          char *bridge_name, char *port_name, char *mstpi)
{
    return get_object(MSTP, MSTP_PORT_PRIORITY_STR, BASE_RC_NO,
                      result_array, row_num, col_num,
                      bridge_name, port_name, mstpi);
}

int get_mstp_port_path_cost(void *result_array, unsigned int row_num, unsigned int col_num,
                            char *bridge_name, char *port_name, char *mstpi)
{
    return get_object(MSTP, MSTP_PORT_PATH_COST_STR, BASE_RC_NO,
                      result_array, row_num, col_num,
                      bridge_name, port_name, mstpi);
}

int get_mstp_port_state(void *result_array, unsigned int row_num, unsigned int col_num,
                        char *bridge_name, char *port_name, char *mstpi)
{
    return get_object(MSTP, MSTP_PORT_STATE_STR, BASE_RC_NO,
                      result_array, row_num, col_num,
                      bridge_name, port_name, mstpi);
}

int get_mstp_port_dsgn_root(void *result_array, unsigned int row_num, unsigned int col_num,
                            char *bridge_name, char *port_name, char *mstpi)
{
    return get_object(MSTP, MSTP_PORT_DSGN_ROOT_STR, BASE_RC_NO,
                      result_array, row_num, col_num,
                      bridge_name, port_name, mstpi);
}

int get_mstp_port_dsgn_cost(void *result_array, unsigned int row_num, unsigned int col_num,
                            char *bridge_name, char *port_name, char *mstpi)
{
    return get_object(MSTP, MSTP_PORT_DSGN_COST_STR, BASE_RC_NO,
                      result_array, row_num, col_num,
                      bridge_name, port_name, mstpi);
}

int get_mstp_port_dsgn_bridge(void *result_array, unsigned int row_num, unsigned int col_num,
                              char *bridge_name, char *port_name, char *mstpi)
{
    return get_object(MSTP, MSTP_PORT_DSGN_BRIDGE_STR, BASE_RC_NO,
                      result_array, row_num, col_num,
                      bridge_name, port_name, mstpi);
}

int get_mstp_port_dsgn_port(void *result_array, unsigned int row_num, unsigned int col_num,
                            char *bridge_name, char *port_name, char *mstpi)
{
    return get_object(MSTP, MSTP_PORT_DSGN_PORT_STR, BASE_RC_NO,
                      result_array, row_num, col_num,
                      bridge_name, port_name, mstpi);
}

int get_mstp_port_role(void *result_array, unsigned int row_num, unsigned int col_num,
                       char *bridge_name, char *port_name, char *mstpi)
{
    return get_object(MSTP, MSTP_PORT_ROLE_STR, BASE_RC_NO,
                      result_array, row_num, col_num,
                      bridge_name, port_name, mstpi);
}

int get_mstp_port_disputed(void *result_array, unsigned int row_num, unsigned int col_num,
                           char *bridge_name, char *port_name, char *mstpi)
{
    return get_object(MSTP, MSTP_PORT_DISPUTED_STR, BASE_RC_NO,
                      result_array, row_num, col_num,
                      bridge_name, port_name, mstpi);
}

int get_mstp_config_id_format_selector(void *result_array, unsigned int row_num, unsigned int col_num,
                                       char *bridge_name)
{
    return get_object(MSTP, MSTP_CONF_ID_SELECTOR_STR, BASE_RC_NO,
                      result_array, row_num, col_num,
                      bridge_name);
}

int get_mstp_configuration_name(void *result_array, unsigned int row_num, unsigned int col_num,
                                char *bridge_name)
{
    return get_object(MSTP, MSTP_CONF_ID_CONF_NAME_STR, BASE_RC_NO,
                      result_array, row_num, col_num,
                      bridge_name);
}

int get_mstp_revision_level(void *result_array, unsigned int row_num, unsigned int col_num,
                            char *bridge_name)
{
    return get_object(MSTP, MSTP_CONF_ID_REVISION_STR, BASE_RC_NO,
                      result_array, row_num, col_num,
                      bridge_name);
}

int get_mstp_configuration_digest(void *result_array, unsigned int row_num, unsigned int col_num,
                                  char *bridge_name)
{
    return get_object(MSTP, MSTP_CONF_DIGEST_STR, BASE_RC_NO,
                      result_array, row_num, col_num,
                      bridge_name);
}

int remove_mstp_module()
{
    return set_object(MSTP, MSTP_NAME, MSTP_PREFIX, "");
}

int set_mstp_cist_max_hops(char *bridge_id, char *value)
{
    char bridge_name[MAX_STR_LEN] = "";

    if (EXIT_FAILURE == find_bridge_name_by_id(bridge_id, bridge_name)) {
        return EXIT_FAILURE;
    }
    return set_object(MSTP, MSTP_CIST_MAX_HOPS_STR, bridge_name, value);
}

int set_mstp_bridge_priority(char *bridge_id, char *mstpi, char *value)
{
    char bridge_name[MAX_STR_LEN] = "";

    if (EXIT_FAILURE == find_bridge_name_by_id(bridge_id, bridge_name)) {
        return EXIT_FAILURE;
    }
    return set_object(MSTP, MSTP_BRIDGE_PRIORITY_STR, bridge_name, mstpi, value);
}

int set_mstp_cist_port_admin_path_cost(char *bridge_id, char *port_id, char *value)
{
    char bridge_name[MAX_STR_LEN] = "";
    char port_name[MAX_STR_LEN] = "";

    if (EXIT_FAILURE == find_bridge_name_by_id(bridge_id, bridge_name)) {
        return EXIT_FAILURE;
    }
    if (EXIT_FAILURE == find_port_name_by_id(bridge_name, port_id, port_name)) {
        return EXIT_FAILURE;
    }
    return set_object(MSTP, MSTP_CIST_PORT_ADMIN_PATH_COST_STR, bridge_name, port_name, value);
}

int set_mstp_cist_port_admin_edge_port(char *bridge_id, char *port_id, char *value)
{
    char bridge_name[MAX_STR_LEN] = "";
    char port_name[MAX_STR_LEN] = "";

    if (EXIT_FAILURE == find_bridge_name_by_id(bridge_id, bridge_name)) {
        return EXIT_FAILURE;
    }

    if (EXIT_FAILURE == find_port_name_by_id(bridge_name, port_id, port_name)) {
        return EXIT_FAILURE;
    }

    return set_object(MSTP, MSTP_CIST_PORT_ADMIN_EDGE_PORT_STR, bridge_name, port_name, value);
}

int set_mstp_cist_port_mac_enabled(char *bridge_id, char *port_id, char *value)
{
    char bridge_name[MAX_STR_LEN] = "";
    char port_name[MAX_STR_LEN] = "";

    if (EXIT_FAILURE == find_bridge_name_by_id(bridge_id, bridge_name)) {
        return EXIT_FAILURE;
    }

    if (EXIT_FAILURE == find_port_name_by_id(bridge_name, port_id, port_name)) {
        return EXIT_FAILURE;
    }

    return set_object(MSTP, MSTP_CIST_PORT_MAC_ENABLED_STR, bridge_name, port_name, value);
}

int set_mstp_cist_port_restricted_role(char *bridge_id, char *port_id, char *value)
{
    char bridge_name[MAX_STR_LEN] = "";
    char port_name[MAX_STR_LEN] = "";

    if (EXIT_FAILURE == find_bridge_name_by_id(bridge_id, bridge_name)) {
        return EXIT_FAILURE;
    }

    if (EXIT_FAILURE == find_port_name_by_id(bridge_name, port_id, port_name)) {
        return EXIT_FAILURE;
    }

    return set_object(MSTP, MSTP_CIST_PORT_RESTR_ROLE_STR, bridge_name, port_name, value);
}

int set_mstp_cist_port_restricted_tcn(char *bridge_id, char *port_id, char *value)
{
    char bridge_name[MAX_STR_LEN] = "";
    char port_name[MAX_STR_LEN] = "";

    if (EXIT_FAILURE == find_bridge_name_by_id(bridge_id, bridge_name)) {
        return EXIT_FAILURE;
    }

    if (EXIT_FAILURE == find_port_name_by_id(bridge_name, port_id, port_name)) {
        return EXIT_FAILURE;
    }

    return set_object(MSTP, MSTP_CIST_PORT_RESTR_TCN_STR, bridge_name, port_name, value);
}

int set_mstp_port_priority(char *bridge_id, char *port_id, char *mstpi, char *value)
{
    char bridge_name[MAX_STR_LEN] = "";
    char port_name[MAX_STR_LEN] = "";

    if (EXIT_FAILURE == find_bridge_name_by_id(bridge_id, bridge_name)) {
        return EXIT_FAILURE;
    }
    if (EXIT_FAILURE == find_port_name_by_id(bridge_name, port_id, port_name)) {
        return EXIT_FAILURE;
    }

    return set_object(MSTP, MSTP_PORT_PRIORITY_STR, bridge_name, port_name, mstpi, value);
}

int set_mstp_port_path_cost(char *bridge_id, char *port_id, char *mstpi, char *value)
{
    char bridge_name[MAX_STR_LEN] = "";
    char port_name[MAX_STR_LEN] = "";

    if (EXIT_FAILURE == find_bridge_name_by_id(bridge_id, bridge_name)) {
        return EXIT_FAILURE;
    }
    if (EXIT_FAILURE == find_port_name_by_id(bridge_name, port_id, port_name)) {
        return EXIT_FAILURE;
    }

    return set_object(MSTP, MSTP_PORT_PATH_COST_STR, bridge_name, port_name, mstpi, value);
}

int set_mstp_conf_name(char *bridge_id, char *value)
{
    char bridge_name[MAX_STR_LEN] = "";
    char revision_level[BASE_TEN][MAX_STR_LEN];

    if (EXIT_FAILURE == find_bridge_name_by_id(bridge_id, bridge_name)) {
        return EXIT_FAILURE;
    }
    /* get_mstp_revision_level, needed */
    if (EXIT_FAILURE == get_mstp_revision_level(revision_level, BASE_TEN,
                                                MAX_STR_LEN, bridge_name)) {
        return EXIT_FAILURE;
    }

    return set_object(MSTP, MSTP_CONF_ID_CONF_NAME_STR, bridge_name,
                      revision_level, value);
}

int set_mstp_revision_level(char *bridge_id, char *value)
{
    char bridge_name[MAX_STR_LEN] = "";
    char config_name[MAX_NUM_OF_INTERFACES][MAX_STR_LEN];

    base_print_debug(DEBUG_MSG_FUN_CALLED_WITH_TWO_PARAM, __func__, bridge_id, value);

    if (EXIT_FAILURE == find_bridge_name_by_id(bridge_id, bridge_name)) {
        return EXIT_FAILURE;
    }
    /* get_mstp_revision_level, needed */
    if (EXIT_FAILURE == get_mstp_configuration_name(config_name, MAX_NUM_OF_INTERFACES,
                                                    MAX_STR_LEN, bridge_name)) {
        return EXIT_FAILURE;
    }

    return set_object(MSTP, MSTP_CONF_ID_REVISION_STR, bridge_name, value, config_name);
}

int remove_mstp_entry(char *bridge_id, char *msti)
{
    char bridge_name[MAX_STR_LEN] = "";

    if (EXIT_FAILURE == find_bridge_name_by_id(bridge_id, bridge_name)) {
        return EXIT_FAILURE;
    }
    return set_object(MSTP, MSTP_ENTRY_DEL_STR, bridge_name, msti);
}

int add_mstp_entry(char *bridge_id, char *msti)
{
    char bridge_name[MAX_STR_LEN] = "";

    if (EXIT_FAILURE == find_bridge_name_by_id(bridge_id, bridge_name)) {
        return EXIT_FAILURE;
    }
    return set_object(MSTP, MSTP_ENTRY_ADD_STR, bridge_name, msti);
}

int find_bridge_name_by_id(char *bridge_id, char *result)
{
    char bridge_names[MAX_NUM_OF_INTERFACES][MAX_STR_LEN];
    int module_bridge_index = 0;
    int array_bridge_index = 0;
    int ret_value = EXIT_SUCCESS;

    base_print_debug(DEBUG_MSG_FUN_CALLED_WITH_ONE_PARAM, __func__, bridge_id);

    if (EXIT_FAILURE == get_mstp_bridge_names(bridge_names, MAX_NUM_OF_INTERFACES, MAX_STR_LEN)) {
        return EXIT_FAILURE;
    }
    module_bridge_index = (int)strtol(bridge_id, NULL, 10);
    // bridge ids starting from 1, array indices starting from 0
    array_bridge_index = module_bridge_index - 1;

    if ((0 > array_bridge_index) && (array_bridge_index > MAX_NUM_OF_INTERFACES)) {
        base_print_warning("%s: non-existent bridge index (%s)!",
                           __func__, bridge_id);
        return EXIT_FAILURE;
    }
    if ((0 != strncmp(bridge_names[array_bridge_index], "", MAX_STR_LEN))) {
        strncpy(result, bridge_names[array_bridge_index], MAX_STR_LEN);
    } else {
        base_print_warning("%s: non-existent bridge at index (%s)!",
                           __func__, bridge_id);
        return EXIT_FAILURE;
    }

    return ret_value;
}

int find_port_name_by_id(char *bridge_name, char *port_id, char *result)
{
    char port_names[MAX_NUM_OF_INTERFACES][MAX_STR_LEN];
    char port_ids[MAX_NUM_OF_INTERFACES][MAX_STR_LEN];
    int i = 0;
    int ret_value = EXIT_SUCCESS;

    if (EXIT_FAILURE == get_mstp_bridge_port_names(port_names, MAX_NUM_OF_INTERFACES,
                                                   MAX_STR_LEN, bridge_name)) {
        return EXIT_FAILURE;
    }
    while ((i < MAX_NUM_OF_INTERFACES) &&
           (NULL != port_names[i]) &&
           (0 != strncmp("", port_names[i], MAX_STR_LEN))) {
        if (EXIT_FAILURE == get_mstp_port_id(port_ids, MAX_NUM_OF_INTERFACES,
                                             MAX_STR_LEN, bridge_name, port_names[i])) {
            base_print_warning("%s(): none-xistent port id!", __func__);
            return EXIT_FAILURE;
        }
        if (0 == strncmp(port_ids[0], port_id, MAX_STR_LEN)) {
            // get_mstp_port_id returns only one string
            strncpy(result, port_names[i], MAX_STR_LEN);
            break;
        }
        i++;
    }
    if (0 == strncmp("", result, MAX_STR_LEN)) {
        base_print_warning("%s(): none-xistent port!", __func__);
        return EXIT_FAILURE;
    }

    return ret_value;
}

int get_cist_port_admin_point_to_point(void *result_array, unsigned int row_num, unsigned int col_num,
                                       char *bridge_name, char *port_name)
{
    return get_object(MSTP, MSTP_PORT_ADMIN_P2P_STR, BASE_RC_NO,
                      result_array, row_num, col_num,
                      bridge_name, port_name);
}

int set_cist_port_admin_point_to_point(char *bridge_name, char *port_name, char *value)
{
    return set_object(MSTP, MSTP_PORT_ADMIN_P2P_STR, bridge_name, port_name, value);
}

int mstpctl_delete_fids(char *bridge_name, char *value)
{
    return set_object(MSTP, MSTP_DEL_FIDS_STR, bridge_name, value);
}

int mstpctl_setfid2mstid(char *bridge_name, char *mstid, char *fids)
{
    return set_object(MSTP, MSTP_SET_FID_2_MSTID_STR, bridge_name, mstid, fids);
}

int mstpctl_setvid2fid(char *bridge_name, char *fid, char *vid)
{
    return set_object(MSTP, MSTP_SET_VID_2_FID_STR, bridge_name, fid, vid);
}
