/*
 * TTTech libbase
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "base_prints.h"
#include "base_q_bridge_linux_tools.h"
#include "base_linux_tool_helpers.h"
#include "base_linux_tools.h"
#include "base_private_defines.h"
#include "base_private_parsers.h"

int bridge_vlan_mod_dev_vid(char *dev_name, char *vids, char *object_name, char *option)
{
    base_print_debug(DEBUG_MSG_FUN_CALLED_WITH_THREE_PARAM, __func__,
                     dev_name, vids, option);
    return set_object(Q_BRIDGE, object_name, dev_name, vids, option);
}

int bridge_fdb_add_dev_filt_entry(char *dev_name, char *dev_address, char *vids)
{
    char port_address[MAX_NUM_OF_INTERFACES][MAX_STR_LEN];
    char result_array[BASE_MAX_VID][BASE_FIVE];
    int i = 0;
    int ret_val = EXIT_SUCCESS;

    base_print_debug(DEBUG_MSG_FUN_CALLED_WITH_THREE_PARAM, __func__,
                     dev_name, dev_address, vids);

    /* get real port address, not filtering entry address */
    if (EXIT_FAILURE == get_interface_address(port_address, MAX_NUM_OF_INTERFACES,
                                              MAX_STR_LEN, dev_name)) {
        base_print_warning(ERR_MSG_REAL_MAC_ADDR_FAILED, dev_name);
        return EXIT_FAILURE;
    }

    assemble_vids_as_array(result_array, BASE_MAX_VID, BASE_FIVE, vids);
    change_addr_dashes_to_colons(dev_address);

    while ((i < BASE_MAX_VID) &&
           (0 != strncmp(result_array[i], "", BASE_FIVE)) &&
           (EXIT_SUCCESS == ret_val)) {
        if (0 == strncmp(DEFAULT_VID, result_array[i], BASE_FIVE)) {
            ret_val = set_object(Q_BRIDGE, Q_BRIDGE_ADD_FDB_SELF_STATIC_STR,
                                 dev_address, dev_name);
        } else {
            ret_val = set_object(Q_BRIDGE, Q_BRIDGE_ADD_FDB_SELF_STATIC_VLAN_STR,
                                 dev_address, dev_name, result_array[i]);
        }
        i++;
    }
    return ret_val;
}

int bridge_fdb_rem_dev_filt_entry(char *dev_name, char *dev_address, char *vids)
{
    char result_array[BASE_MAX_VID][BASE_FIVE];
    int i = 0;
    int ret_val = EXIT_SUCCESS;

    base_print_debug(DEBUG_MSG_FUN_CALLED_WITH_THREE_PARAM, __func__,
                     dev_name, dev_address, vids);

    assemble_vids_as_array(result_array, BASE_MAX_VID, BASE_FIVE, vids);
    change_addr_dashes_to_colons(dev_address);

    while ((i < BASE_MAX_VID) &&
           (0 != strncmp(result_array[i], "", BASE_FIVE)) &&
           (EXIT_SUCCESS == ret_val)) {
        if (0 == strncmp(DEFAULT_VID, result_array[i], BASE_FIVE)) {
            ret_val = set_object(Q_BRIDGE, Q_BRIDGE_DEL_FDB_SELF_STATIC_STR,
                                 dev_address, dev_name);
        } else {
            ret_val = set_object(Q_BRIDGE, Q_BRIDGE_DEL_FDB_SELF_STATIC_VLAN_STR,
                                 dev_address, dev_name, result_array[i]);
        }
        i++;
    }
    return ret_val;
}

int get_bridge_port_names(void *result_array, unsigned int row_num, unsigned int col_num)
{
    return get_object(Q_BRIDGE, Q_BRIDGE_PORTS_STR, BASE_RC_NO,
                      result_array, row_num, col_num,
                      "");
}

int get_q_bridge_port_pvid(void *result_array, unsigned int row_num, unsigned int col_num,
                           char *port_name)
{
    return get_object(Q_BRIDGE, Q_BRIDGE_PORT_PVID_STR, BASE_RC_NO,
                      result_array, row_num, col_num,
                      port_name);
}

int set_q_bridge_port_pvid(char *dev_name, char *pvid)
{
    return set_object(Q_BRIDGE, Q_BRIDGE_PORT_PVID_STR, pvid, dev_name);
}

int get_bridge_names(void *result_array, unsigned int row_num, unsigned int col_num)
{
    return get_object(Q_BRIDGE, Q_BRIDGE_NAME_STR, BASE_RC_NO,
                      result_array, row_num, col_num,
                      "");
}

int get_bridge_fids(void *result_array, unsigned int row_num, unsigned int col_num,
                    char *bridge_name, char *bridge_msti)
{
    sys_call_spec_t tmp_sys_call_spec;

    tmp_sys_call_spec = assemble_sys_call_structure(Q_BRIDGE, Q_BRIDGE_FIDS, BASE_RC_NO);
    // 2nd combinator, 1st callback - from configuration
    snprintf(tmp_sys_call_spec.parse_cb[1][0].match_case, MAX_STR_LEN, " %s:", bridge_msti);
    // store customized system call structure
    set_rc_sys_call_spec(tmp_sys_call_spec);
    // BASE_RC_YES means "use runtime configurable value"
    return get_object(Q_BRIDGE, Q_BRIDGE_FIDS_STR, BASE_RC_YES,
                      result_array, row_num, col_num,
                      bridge_name);
}

int get_bridge_fid_by_mstid(void *result_array, unsigned int row_num, unsigned int col_num,
                            char *bridge_name, char *bridge_msti)
{
    sys_call_spec_t tmp_sys_call_spec;

    tmp_sys_call_spec = assemble_sys_call_structure(Q_BRIDGE, Q_BRIDGE_MSTID_FID, BASE_RC_NO);
    // 2nd combinator, 1st callback - from configuration
    snprintf(tmp_sys_call_spec.parse_cb[1][0].match_case, MAX_STR_LEN, " %s:", bridge_msti);
    // store customized system call structure
    set_rc_sys_call_spec(tmp_sys_call_spec);
    // BASE_RC_YES means "use runtime configurable value"
    return get_object(Q_BRIDGE, Q_BRIDGE_MSTID_FID_STR, BASE_RC_YES,
                      result_array, row_num, col_num,
                      bridge_name);
}

int get_bridge_type(void *result_array, unsigned int row_num, unsigned int col_num,
                    char *bridge_name)
{
    (void)bridge_name;
    (void)row_num;
    (void)col_num;

    snprintf(result_array, MAX_STR_LEN, Q_BRIDGE_CUST_VLAN_BRIDGE);
    return EXIT_SUCCESS;
}

int get_bridge_uptime(void *result_array, unsigned int row_num, unsigned int col_num)
{
    return get_object(Q_BRIDGE, Q_BRIDGE_UPTIME_STR, BASE_RC_NO,
                      result_array, row_num, col_num,
                      "");
}

int get_bridge_vlans(void *bridge_vlans, unsigned int row_num, unsigned int col_num,
                     int compressed)
{
    char ports_name[MAX_NUM_OF_INTERFACES][MAX_STR_LEN];
    char port_vlans[BASE_MAX_VID][BASE_TEN];
    char *tmp_bridge_vlans = NULL; // placeholder
    int i = 0; // iterates trough ports name
    int j = 0; // iterates trough port's vlans
    int k = 0; // iterates trough finalized bridge's vlans
    int ret_value = EXIT_FAILURE;

    memset(port_vlans, 0, BASE_MAX_VID * BASE_TEN);

    if (EXIT_SUCCESS == get_bridge_port_names(ports_name, MAX_NUM_OF_INTERFACES, MAX_STR_LEN)) {
        while ((i < MAX_NUM_OF_INTERFACES) &&
               (0 != strncmp(ports_name[i], "", MAX_STR_LEN))) {
            if (EXIT_FAILURE == get_port_vlans_by_name(port_vlans, row_num, col_num, compressed, ports_name[i])) {
                // if process break somewhere, erase everything
                memset(bridge_vlans, 0, row_num * col_num);
                break;
            }
            j = 0;
            while ((j < BASE_MAX_VID) && (0 != strncmp("", port_vlans[j], BASE_TEN))) {
                tmp_bridge_vlans = (char*)bridge_vlans + (k * col_num);
                memcpy(tmp_bridge_vlans, port_vlans[j], col_num);
                j++;
                k++;
            }
            i++;
            memset(port_vlans, 0, BASE_MAX_VID * BASE_TEN);
        }
        // first 6 parameters are irrelevant
        array_uniq_elements(0, 0, 0, 0, NULL, BASE_RC_NO, bridge_vlans, row_num, col_num);
        ret_value = EXIT_SUCCESS;
    }

    return ret_value;
}

int get_interface_if_index(void *result_array, unsigned int row_num, unsigned int col_num,
                           char *interface_name)
{
    return get_object(Q_BRIDGE, Q_BRIDGE_PORT_NUMBER_STR, BASE_RC_NO,
                      result_array, row_num, col_num,
                      interface_name);
}

int get_interface_address(void *result_array, unsigned int row_num, unsigned int col_num,
                          char *interface_name)
{
    return get_object(Q_BRIDGE, Q_BRIDGE_ADDRESS_STR, BASE_RC_NO,
                      result_array, row_num, col_num,
                      interface_name);
}

int set_interface_address(char *interface_name, char *value)
{
    base_print_debug(DEBUG_MSG_FUN_CALLED_WITH_TWO_PARAM, __func__,
                     interface_name, value);

    /* turn off component */
    if (EXIT_FAILURE == ip_link_set_dev_down(interface_name)) {
        return EXIT_FAILURE;
    }
    /* change component's address */
    change_addr_dashes_to_colons(value);
    if (EXIT_FAILURE == ip_link_set_dev_address(interface_name, value)) {
        return EXIT_FAILURE;
    }
    /* turn on component */
    if (EXIT_FAILURE == ip_link_set_dev_up(interface_name)) {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

int get_vids_by_static_fdb_address(void *result_array, unsigned int row_num, unsigned int col_num,
                                   char *address)
{
    sys_call_spec_t tmp_sys_call_spec;
    int ret_value = EXIT_FAILURE;
    int i = 0;
    // max number of vids + max number of default vids per port
    char fdb_vids[BASE_MAX_VID + Q_BRIDGE_PORT_MAX_NUM][BASE_FIVE];
    char default_fdb_vid[Q_BRIDGE_PORT_MAX_NUM][BASE_FIVE];

    if (((BASE_MAX_VID + Q_BRIDGE_PORT_MAX_NUM) != row_num) || (BASE_FIVE != col_num)) {
        return ret_value;
    }
    tmp_sys_call_spec = assemble_sys_call_structure(Q_BRIDGE, Q_BRIDGE_VIDS_BY_STATIC_FDB_ADDR, BASE_RC_NO);
    // 2nd combinator, 1st callback - from configuration
    snprintf(tmp_sys_call_spec.parse_cb[1][0].match_case, MAX_STR_LEN, "%s", address);
    change_addr_dashes_to_colons(tmp_sys_call_spec.parse_cb[1][0].match_case);
    set_rc_sys_call_spec(tmp_sys_call_spec); // store customized system call structure
    // BASE_RC_YES means "use runtime configurable value"
    if (EXIT_SUCCESS == get_object(Q_BRIDGE, Q_BRIDGE_VIDS_BY_STATIC_FDB_ADDR_STR, BASE_RC_YES,
                                   fdb_vids, BASE_MAX_VID + Q_BRIDGE_PORT_MAX_NUM, BASE_FIVE)) {
        // next function just check is there entry with no vlan
        if (EXIT_SUCCESS == get_default_vids_by_static_fdb_address(default_fdb_vid, Q_BRIDGE_PORT_MAX_NUM,
                                                                   BASE_FIVE, address)) {
            while ((i < BASE_MAX_VID) && (0 != strncmp(fdb_vids[i], "", BASE_FIVE))) {
                // no need that loop goes to BASE_MAX_VID + Q_BRIDGE_PORT_MAX_NUM
                i++;
            }
            memcpy(fdb_vids[i], DEFAULT_VID, BASE_FIVE); // add default vid
            memcpy(result_array, fdb_vids, row_num * col_num);
        } else {
            // take regular vids
            memcpy(result_array, fdb_vids, row_num * col_num);
        }
        ret_value = EXIT_SUCCESS; // in both cases return value is ok
    }
    return ret_value;
}

int get_default_vids_by_static_fdb_address(void *result_array, unsigned int row_num, unsigned int col_num,
                                           char *address)
{
    sys_call_spec_t tmp_sys_call_spec;

    tmp_sys_call_spec = assemble_sys_call_structure(Q_BRIDGE, Q_BRIDGE_DEFAULT_VIDS_BY_STATIC_FDB_ADDR, BASE_RC_NO);
    // 2nd combinator, 1st callback - from configuration
    snprintf(tmp_sys_call_spec.parse_cb[1][0].match_case, MAX_STR_LEN, "%s", address);
    change_addr_dashes_to_colons(tmp_sys_call_spec.parse_cb[1][0].match_case);
    set_rc_sys_call_spec(tmp_sys_call_spec); // store customized system call structure
    // BASE_RC_YES means "use runtime configurable value"
    return get_object(Q_BRIDGE, Q_BRIDGE_DEFAULT_VIDS_BY_STATIC_FDB_ADDR_STR, BASE_RC_YES,
                      result_array, row_num, col_num);
}

int get_static_fdb_addresses(void *result_array, unsigned int row_num, unsigned int col_num)
{
    return get_object(Q_BRIDGE, Q_BRIDGE_STATIC_FDB_ADDRESSES_STR, BASE_RC_NO,
                      result_array, row_num, col_num);
}

int get_fdb_port_names_by_addr_and_vid(void *result_array, unsigned int row_num, unsigned int col_num,
                                       char *address, char *vid)
{
    sys_call_spec_t tmp_sys_call_spec;

    if (0 == strncmp(vid, DEFAULT_VID, BASE_FIVE)) {
        return get_fdb_port_names_by_addr_and_default_vid(result_array, row_num, col_num, address);
    } else {
        tmp_sys_call_spec = assemble_sys_call_structure(Q_BRIDGE, Q_BRIDGE_FDB_PORT_NAMES, BASE_RC_NO);
        // 2nd combinator, 1st callback - from configuration
        snprintf(tmp_sys_call_spec.parse_cb[1][0].match_case, MAX_STR_LEN, "%s", address);
        change_addr_dashes_to_colons(tmp_sys_call_spec.parse_cb[1][0].match_case);
        // 2nd combinator, 2nd callback - from configuration
        snprintf(tmp_sys_call_spec.parse_cb[1][1].match_case, BASE_FIVE, "%s", vid);
        // store customized system call structure
        set_rc_sys_call_spec(tmp_sys_call_spec);
        // BASE_RC_YES means "use runtime configurable value"
        return get_object(Q_BRIDGE, Q_BRIDGE_FDB_PORT_NAMES_STR, BASE_RC_YES,
                          result_array, row_num, col_num);
    }
}

int does_static_fdb_entry_exist(char *address, char *vid)
{
    sys_call_spec_t tmp_sys_call_spec;
    char dummy_result_array[BASE_TWO][BASE_TWO]; // it is not used, but must exists to fulfill declarations

    tmp_sys_call_spec = assemble_sys_call_structure(Q_BRIDGE, Q_BRIDGE_FDB_EXISTS, BASE_RC_NO);
    // 2nd combinator, 2nd callback - from configuration
    snprintf(tmp_sys_call_spec.parse_cb[1][1].match_case, MAX_STR_LEN, "%s", address);
    change_addr_dashes_to_colons(tmp_sys_call_spec.parse_cb[1][1].match_case);
    // 2nd combinator, 3rd callback - from configuration// 2nd combinator, 3rd callback - from configuration
    snprintf(tmp_sys_call_spec.parse_cb[1][2].match_case, BASE_FIVE, "%s", vid);
    // store customized system call structure
    set_rc_sys_call_spec(tmp_sys_call_spec);

    return get_object(Q_BRIDGE, Q_BRIDGE_FDB_EXISTS_STR, BASE_RC_YES,
                      dummy_result_array, BASE_TWO, BASE_TWO);
}

int get_number_of_unique_static_fdb_entries(void *result_array, unsigned int row_num, unsigned int col_num,
                                            char *bridge_name)
{
    return get_object(Q_BRIDGE, Q_BRIDGE_NUM_OF_UNIQUE_FDB_ENTRIES_STR, BASE_RC_NO,
                      result_array, row_num, col_num,
                      bridge_name);
}

int get_fdb_port_names_by_addr_and_default_vid(void *result_array, unsigned int row_num, unsigned int col_num,
                                               char *address)
{
    sys_call_spec_t tmp_sys_call_spec;

    tmp_sys_call_spec = assemble_sys_call_structure(Q_BRIDGE, Q_BRIDGE_DEFAULT_FDB_PORT_NAMES, BASE_RC_NO);
    // 2nd combinator, 1st callback - from configuration
    snprintf(tmp_sys_call_spec.parse_cb[1][0].match_case, MAX_STR_LEN, "%s", address);
    change_addr_dashes_to_colons(tmp_sys_call_spec.parse_cb[1][0].match_case);
    // store customized system call structure
    set_rc_sys_call_spec(tmp_sys_call_spec);
    // BASE_RC_YES means "use runtime configurable value"
    return get_object(Q_BRIDGE, Q_BRIDGE_DEFAULT_FDB_PORT_NAMES_STR, BASE_RC_YES,
                      result_array, row_num, col_num);
}

int get_dynamic_entries(void *result_array, unsigned int row_num, unsigned int col_num,
                        char *bridge_name)
{
    return get_object(Q_BRIDGE, Q_BRIDGE_DYNAMIC_ENTRIES_STR, BASE_RC_NO,
                      result_array, row_num, col_num,
                      bridge_name);
}

int get_dynamic_vlan_entries(void *result_array, unsigned int row_num, unsigned int col_num,
                             char *bridge_name)
{
    return get_object(Q_BRIDGE, Q_BRIDGE_DYNAMIC_VLAN_REG_ENTRY_STR, BASE_RC_NO,
                      result_array, row_num, col_num,
                      bridge_name);
}

int get_mac_address_entries(void *result_array, unsigned int row_num, unsigned int col_num,
                            char *bridge_name)
{
    return get_object(Q_BRIDGE, Q_BRIDGE_MAC_ADDR_REG_ENTRY_STR, BASE_RC_NO,
                      result_array, row_num, col_num,
                      bridge_name);
}

int get_state_comp_dyn_address(void *result_array, unsigned int row_num, unsigned int col_num,
                               char *bridge_name)
{
    return get_object(Q_BRIDGE, Q_BRIDGE_COMP_DYN_ADDR_STR, BASE_RC_NO,
                      result_array, row_num, col_num,
                      bridge_name);
}

int get_state_comp_dyn_vlan(void *result_array, unsigned int row_num, unsigned int col_num,
                            char *port_name, char *port_address)
{
    sys_call_spec_t tmp_sys_call_spec;

    tmp_sys_call_spec = assemble_sys_call_structure(Q_BRIDGE, Q_BRIDGE_COMP_DYNAMIC_VLAN_ADDR, BASE_RC_NO);
    // 2nd combinator, 2nd callback - from configuration
    snprintf(tmp_sys_call_spec.parse_cb[1][1].match_case, MAX_STR_LEN, "%s", port_address);
    // store customized system call structure
    set_rc_sys_call_spec(tmp_sys_call_spec);
    // BASE_RC_YES means "use runtime configurable value"
    return get_object(Q_BRIDGE, Q_BRIDGE_COMP_DYNAMIC_VLAN_ADDR_STR, BASE_RC_YES,
                      result_array, row_num, col_num,
                      port_name);
}

int get_untagged_port_names(void *result_array, unsigned int row_num, unsigned int col_num,
                            char *port_name, char *vid)
{
    sys_call_spec_t tmp_sys_call_spec;

    tmp_sys_call_spec = assemble_sys_call_structure(Q_BRIDGE, Q_BIDGE_UNTAGGED_PORTS, BASE_RC_NO);
    // 2nd combinator, 1st callback - from configuration
    snprintf(tmp_sys_call_spec.parse_cb[1][0].match_case, MAX_STR_LEN, " %s", vid);
    // store customized system call structure
    set_rc_sys_call_spec(tmp_sys_call_spec);
    // BASE_RC_YES means "use runtime configurable value"
    return get_object(Q_BRIDGE, Q_BIDGE_UNTAGGED_PORTS_STR, BASE_RC_YES,
                      result_array, row_num, col_num,
                      port_name);
}

int get_vlan_transmitted(void *result_array, unsigned int row_num, unsigned int col_num,
                         char *port_name, char *vid)
{
    int ret_value = EXIT_SUCCESS;
    sys_call_spec_t tmp_sys_call_spec;

    tmp_sys_call_spec = assemble_sys_call_structure(Q_BRIDGE, Q_BRIDGE_STATIC_VLAN_TRANSMITTED, BASE_RC_NO);
    // 2nd combinator, 1st callback - from configuration
    snprintf(tmp_sys_call_spec.parse_cb[1][0].match_case, MAX_STR_LEN, " %s", vid);
    // store customized system call structure// store customized system call structure
    set_rc_sys_call_spec(tmp_sys_call_spec);
    // BASE_RC_YES means "use runtime configurable value"
    ret_value = get_object(Q_BRIDGE, Q_BRIDGE_STATIC_VLAN_TRANSMITTED_STR, BASE_RC_YES,
                           result_array, row_num, col_num,
                           port_name);

    if (EXIT_FAILURE == ret_value) {
        // default value
        snprintf((char*)result_array, BASE_EIGHT, "%s", Q_BRIDGE_VLAN_TAGGED);
        ret_value = EXIT_SUCCESS;
    }

    return ret_value;
}

int get_control_element(void *result_array, unsigned int row_num, unsigned int col_num,
                        char *interface_name)
{
    return get_object(Q_BRIDGE, Q_BRIDGE_CONTROL_EL_STR, BASE_RC_NO,
                      result_array, row_num, col_num,
                      interface_name);
}

int get_connection_identifier(void *result_array, unsigned int row_num, unsigned int col_num,
                              char *interface_name)
{
    return get_object(Q_BRIDGE, Q_BRIDGE_CONNECT_ID_STR, BASE_RC_NO,
                      result_array, row_num, col_num,
                      interface_name);
}

int get_egress_port_names(void *result_array, unsigned int row_num, unsigned int col_num,
                          char *port_name, char *vid)
{
    sys_call_spec_t tmp_sys_call_spec;

    tmp_sys_call_spec = assemble_sys_call_structure(Q_BRIDGE, Q_BIDGE_EGRESS_PORTS, BASE_RC_NO);
    // 2nd combinator, 1st callback - from configuration
    snprintf(tmp_sys_call_spec.parse_cb[1][0].match_case, MAX_STR_LEN, " %s", vid);
    // store customized system call structure
    set_rc_sys_call_spec(tmp_sys_call_spec);
    memset(result_array, 0, row_num*col_num);
    // BASE_RC_YES means "use runtime configurable value"
    return get_object(Q_BRIDGE, Q_BIDGE_EGRESS_PORTS_STR, BASE_RC_YES,
                      result_array, row_num, col_num,
                      port_name);
}

int get_bridge_mstp_vids(void *result_array, unsigned int row_num, unsigned int col_num,
                         char *bridge_name)
{
    return get_object(Q_BRIDGE, Q_BRIDGE_MST_STR, BASE_RC_NO,
                      result_array, row_num, col_num,
                      bridge_name);
}

int get_bridge_mstp_fids(void *result_array, unsigned int row_num, unsigned int col_num,
                         char *bridge_name)
{
    return get_object(Q_BRIDGE, Q_BRIDGE_FID2VID_ALLOC_STR, BASE_RC_NO,
                      result_array, row_num, col_num,
                      bridge_name);
}

int get_mstp_fid_by_vids(void *result_array, unsigned int row_num, unsigned int col_num,
                         char *bridge_name, char *bridge_vids)
{
    sys_call_spec_t tmp_sys_call_spec;

    tmp_sys_call_spec = assemble_sys_call_structure(Q_BRIDGE, Q_BRIDGE_FID, BASE_RC_NO);
    // 2nd combinator, 1st callback - from configuration
    snprintf(tmp_sys_call_spec.parse_cb[1][0].match_case, MAX_STR_LEN, "%s", bridge_vids);
    // store customized system call structure
    set_rc_sys_call_spec(tmp_sys_call_spec);
    // BASE_RC_YES means "use runtime configurable value"
    return get_object(Q_BRIDGE, Q_BRIDGE_FID_STR, BASE_RC_YES,
                      result_array, row_num, col_num,
                      bridge_name);
}

int get_mstp_vids_by_fid(void *result_array, unsigned int row_num, unsigned int col_num,
                         char *bridge_name, char *bridge_fid)
{
    sys_call_spec_t tmp_sys_call_spec;

    tmp_sys_call_spec = assemble_sys_call_structure(Q_BRIDGE, Q_BRIDGE_VID, BASE_RC_NO);
    // 2nd combinator, 1st callback - from configuration
    snprintf(tmp_sys_call_spec.parse_cb[1][0].match_case, MAX_STR_LEN, " %s:", bridge_fid);
    // store customized system call structure
    set_rc_sys_call_spec(tmp_sys_call_spec);
    // BASE_RC_YES means "use runtime configurable value"
    return get_object(Q_BRIDGE, Q_BRIDGE_VID_STR, BASE_RC_YES,
                      result_array, row_num, col_num,
                      bridge_name);
}

int get_bridge_port_ref(void *result_array, unsigned int row_num, unsigned int col_num,
                        char *interface_name)
{
    return get_object(Q_BRIDGE, Q_BRIDGE_PORT_REF_STR, BASE_RC_NO,
                      result_array, row_num, col_num,
                      interface_name);
}

int get_interface_name_by_id(void *result_array, unsigned int row_num, unsigned int col_num,
                             char *port_id)
{
    sys_call_spec_t tmp_sys_call_spec;

    tmp_sys_call_spec = assemble_sys_call_structure(Q_BRIDGE, Q_BRIDGE_PORT_NAME_BY_ID, BASE_RC_NO);
    // 2nd combinator, 1st callback - from configuration
    snprintf(tmp_sys_call_spec.parse_cb[1][0].match_case, MAX_STR_LEN, "%s:", port_id);
    // store customized system call structure
    set_rc_sys_call_spec(tmp_sys_call_spec);
    // BASE_RC_YES means "use runtime configurable value"
    return get_object(Q_BRIDGE, Q_BRIDGE_PORT_NAME_BY_ID_STR, BASE_RC_YES,
                      result_array, row_num, col_num,
                      port_id);
}

int get_bridge_ports_name_by_vlan(void *result_array, unsigned int row_num, unsigned int col_num,
                                  char *interface_vlan, char *bridge_name)
{
    sys_call_spec_t tmp_sys_call_spec;

    tmp_sys_call_spec = assemble_sys_call_structure(Q_BRIDGE, Q_BRIDGE_PORT_NAME_BY_VLAN, BASE_RC_NO);
    // 2nd combinator, 1st callback - from configuration
    snprintf(tmp_sys_call_spec.parse_cb[1][0].match_case, MAX_STR_LEN, " %s", interface_vlan);
    // 2nd combinator, 2nd callback - from configuration
    snprintf(tmp_sys_call_spec.parse_cb[1][1].match_case, MAX_STR_LEN, "%s", bridge_name);
    // store customized system call structure
    set_rc_sys_call_spec(tmp_sys_call_spec);
    // BASE_RC_YES means "use runtime configurable value"
    return get_object(Q_BRIDGE, Q_BRIDGE_PORT_NAME_BY_VLAN_STR, BASE_RC_YES,
                      result_array, row_num, col_num,
                      interface_vlan);
}

int get_port_vlans_by_name(void *result_array, unsigned int row_num, unsigned int col_num,
                           int compressed, char *interface_name)
{
    sys_call_spec_t tmp_sys_call_spec;
    int ret_value = EXIT_FAILURE;
    char vlans_array[BASE_MAX_VID][BASE_TEN];
    char tmp_vlans_array[BASE_MAX_VID][BASE_TEN];
    char object_name_1[MAX_STR_LEN];
    char object_name_2[MAX_STR_LEN];
    unsigned int object_index_1 = 0;
    unsigned int object_index_2 = 0;

    base_print_debug(DEBUG_MSG_FUN_CALLED_WITH_ONE_PARAM, __func__,
                     interface_name);

    if ((row_num >= BASE_MAX_VID) && (col_num >= BASE_TEN)) {
        if (0 == compressed) {
            // not compressed vlan ids
            snprintf(object_name_1, MAX_STR_LEN, "%s", Q_BRIDGE_PORT_VLANS_BY_NAME_STR);
            snprintf(object_name_2, MAX_STR_LEN, "%s", Q_BRIDGE_PORT_VLANS_WITHOUT_NAME_STR);
            object_index_1 = Q_BRIDGE_PORT_VLANS_BY_NAME;
            object_index_2 = Q_BRIDGE_PORT_VLANS_WITHOUT_NAME;
        } else {
            snprintf(object_name_1, MAX_STR_LEN, "%s", Q_BRIDGE_COMPRESSED_PORT_VLANS_BY_NAME_STR);
            snprintf(object_name_2, MAX_STR_LEN, "%s", Q_BRIDGE_COMPRESSED_PORT_VLANS_WITHOUT_NAME_STR);
            object_index_1 = Q_BRIDGE_COMPRESSED_PORT_VLANS_BY_NAME;
            object_index_2 = Q_BRIDGE_COMPRESSED_PORT_VLANS_WITHOUT_NAME;
        }
        /* The output of bridge -c vlan command for port CE01 example:
         * root@de-eval-board:~# bridge -c vlan show dev CE01
         * port    vlan ids
         * CE01     1
         *          5-9
         *          11 Egress Untagged
         *          17 Egress Untagged
         */
        /*
         *  First, we need to get port vids from row with port name:
         *  CE01     1
         */
        tmp_sys_call_spec = assemble_sys_call_structure(Q_BRIDGE, object_index_1, BASE_RC_NO);
        // 2nd combinator, 1st callback - from configuration// 2nd combinator, 1st callback - from configuration
        snprintf(tmp_sys_call_spec.parse_cb[1][0].match_case, MAX_STR_LEN, "%s", interface_name);
        // store customized system call structure
        set_rc_sys_call_spec(tmp_sys_call_spec);
        // BASE_RC_YES means "use runtime configurable value"
        ret_value = get_object(Q_BRIDGE, object_name_1, BASE_RC_YES,
                               vlans_array, row_num, col_num, interface_name);

        if (EXIT_SUCCESS == ret_value) {
            /*
             *  Second, we need to get port vids from rows without port name and without "port    vlan ids".
             * port    vlan ids
             *          5-9
             *          11 Egress Untagged
             *          17 Egress Untagged
             */
            tmp_sys_call_spec = assemble_sys_call_structure(Q_BRIDGE, object_index_2, BASE_RC_NO);
            // 2nd combinator, 1st callback - from configuration
            snprintf(tmp_sys_call_spec.parse_cb[1][0].match_case, MAX_STR_LEN, "%s", interface_name);
            // store customized system call structure
            set_rc_sys_call_spec(tmp_sys_call_spec);
            // BASE_RC_YES means "use runtime configurable value"
            ret_value = get_object(Q_BRIDGE, object_name_2, BASE_RC_YES,
                                   tmp_vlans_array, row_num, col_num, interface_name);

            // save first - skip it
            memcpy(vlans_array[1], tmp_vlans_array, (BASE_MAX_VID - 1) * BASE_TEN);
            memcpy(result_array, vlans_array, BASE_MAX_VID * BASE_TEN);
        }
    }
    return ret_value;
}
