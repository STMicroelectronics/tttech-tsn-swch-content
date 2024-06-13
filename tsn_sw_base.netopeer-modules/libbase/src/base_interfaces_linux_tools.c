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
#include <stdint.h>

#include "base_prints.h"
#include "base_interfaces_linux_tools.h"
#include "base_linux_tool_helpers.h"
#include "base_linux_tools.h"
#include "base_private_defines.h"
#include "base_private_parsers.h"
#include "base_private_syscalls.h"

int get_interface_names(void *result_array, unsigned int row_num, unsigned int col_num)
{
    return get_object(IETF_INTERFACES, IF_NAME_STR, BASE_RC_NO,
                      result_array, row_num, col_num, "");
}

int get_interface_enabled(void *result_array, unsigned int row_num, unsigned int col_num, char *interface_name)
{
    return get_object(IETF_INTERFACES, IF_ENABLE_STR, BASE_RC_NO,
                      result_array, row_num, col_num, interface_name);
}

int set_interface_enabled(char *interface_name, char *value)
{
    return set_object(IETF_INTERFACES, IF_ENABLE_STR, interface_name, value);
}

int get_interface_tx_statistics(void *result_array, unsigned int row_num, unsigned int col_num,
                                char *interface_name)
{
    return get_object(IETF_INTERFACES, IF_TX_STATISTICS_STR, BASE_RC_NO,
                      result_array, row_num, col_num,
                      interface_name);
}

int get_interface_rx_statistics(void *result_array, unsigned int row_num, unsigned int col_num,
                                char *interface_name)
{
    return get_object(IETF_INTERFACES, IF_RX_STATISTICS_STR, BASE_RC_NO,
                      result_array, row_num, col_num,
                      interface_name);
}

int get_eth_if_auto_neg_enable(void *result_array, unsigned int row_num, unsigned int col_num,
                               char *interface_name)
{
    return get_object(IETF_INTERFACES, ETH_IF_ENABLE_STR, BASE_RC_NO,
                      result_array, row_num, col_num,
                      interface_name);
}

int get_eth_if_duplex(void *result_array, unsigned int row_num, unsigned int col_num,
                      char *interface_name)
{
    return get_object(IETF_INTERFACES, ETH_DUPLEX_STR, BASE_RC_NO,
                      result_array, row_num, col_num,
                      interface_name);
}

int get_eth_if_speed(void *result_array, unsigned int row_num, unsigned int col_num,
                     char *interface_name)
{
    return get_object(IETF_INTERFACES, ETH_SPEED_STR, BASE_RC_NO,
                      result_array, row_num, col_num,
                      interface_name);
}

int get_ethernet_max_frame_length(void *result_array, unsigned int row_num, unsigned int col_num,
                                  char *interface_name)
{
    return get_object(IETF_INTERFACES, ETH_MAX_FRAME_LEN_STR, BASE_RC_NO,
                      result_array, row_num, col_num,
                      interface_name);
}

int set_eth_if_auto_neg_enable(char *interface_name, char *value)
{
    return set_object(IETF_INTERFACES, ETH_IF_ENABLE_STR, interface_name, value);
}

int set_eth_if_duplex_and_speed(char *object_name, char *interface_name,
                                char *duplex, char *speed, int with_advertising)
{
    char mode[MAX_STR_LEN] = ETH_BASE_T; /* for now we support only baseT link mode */
    uint32_t supported_modes = 0;
    char advertise[MAX_STR_LEN] = "";
    base_print_debug(DEBUG_MSG_FUN_CALLED, __func__);

    /* advertise will force maximum speed */
    if (with_advertising) {
        supported_modes = set_interface_link_modes(interface_name, with_advertising);
        snprintf(advertise, MAX_STR_LEN, "%x", supported_modes);
        return set_object(IETF_INTERFACES, ETH_ADVERTISE_STR, interface_name, advertise);
    } else {
        if (EXIT_FAILURE == check_link_modes(BASE_RC_YES, interface_name,
                                             speed, duplex, mode, ETH_SUPPORTED_IF_MODE_STR)) {
            base_print_warning(ERROR_MSG_FUN_XML_EL_AND_MSG, __func__,
                               object_name, ERR_MSG_ETH_NOT_SUPPORTED);
            return EXIT_FAILURE;
        }
        return set_object(IETF_INTERFACES, object_name, interface_name, speed, duplex);
    }
}

int check_link_modes(base_rc_t rc_flag, char *interface_name,
                     char *speed, char *duplex, char *mode, char *obj_name)
{
    char result_array[MAX_NUM_OF_INTERFACES][MAX_STR_LEN];
    sys_call_spec_t tmp_sys_call_spec;
    int obj_index = 0;

    base_print_debug(DEBUG_MSG_FUN_CALLED_WITH_THREE_PARAM, __func__,
                     speed, duplex, mode);

    obj_index = find_obj_index_by_name(obj_name, if_map, IF_MAX_OBJ_NUMBER);

    tmp_sys_call_spec = assemble_sys_call_structure(IETF_INTERFACES, obj_index, BASE_RC_NO);
    // this function can receive two types of duplex values
    if (rc_flag) {
        if (0 == strncmp(BASE_ETH_DUPLEX_HALF, duplex, MAX_STR_LEN)) {
            snprintf(tmp_sys_call_spec.parse_cb[1][0].match_case, MAX_STR_LEN, "%s%s/%s",
                     speed, mode, ETH_DUPLEX_HALF); // 2nd combinator, 1st callback - from configuration
        }
        else if (0 == strncmp(BASE_ETH_DUPLEX_FULL, duplex, MAX_STR_LEN)) {
            snprintf(tmp_sys_call_spec.parse_cb[1][0].match_case, MAX_STR_LEN, "%s%s/%s",
                     speed, mode, ETH_DUPLEX_FULL); // 2nd combinator, 1st callback - from configuration
        } else {
            base_print_warning(ERROR_MSG_FUN_AND_MSG, __func__, "unsupported duplex");
            return EXIT_FAILURE;
        }
    } else {
        snprintf(tmp_sys_call_spec.parse_cb[1][0].match_case, MAX_STR_LEN, "%s%s/%s",
                 speed, mode, duplex); // 2nd combinator, 1st callback - from configuration
    }
    set_rc_sys_call_spec(tmp_sys_call_spec) ;// store customized system call structure
    // BASE_RC_YES means "use runtime configurable value"
    return get_object(IETF_INTERFACES, obj_name, BASE_RC_YES,
            &result_array, MAX_NUM_OF_INTERFACES, MAX_STR_LEN, interface_name);
}

/**
 * Next link modes can be supported:
 * SUPPORTED_10baseT_Half
 * SUPPORTED_10baseT_Full
 * SUPPORTED_100baseT_Half
 * SUPPORTED_100baseT_Full
 * SUPPORTED_1000baseT_Half
 * SUPPORTED_1000baseT_Full
 * SUPPORTED_10000baseT_Full
 *
 * @return mask of supported types
 */

uint32_t set_interface_link_modes(char *interface_name, int with_advertising)
{
    uint32_t ret_val = 0x00000000;
    char obj_name[MAX_STR_LEN] = "";
    int i = 0;
    link_mode_t link_mode_array[7] = {
        { ETH_BASE_T, ETH_DUPLEX_HALF, BASE_TEN_STR, SUPPORTED_10baseT_Half },
        { ETH_BASE_T, ETH_DUPLEX_HALF, BASE_ONE_HUNDRED_STR, SUPPORTED_100baseT_Half },
        { ETH_BASE_T, ETH_DUPLEX_HALF, BASE_ONE_THOUSAND_STR, SUPPORTED_1000baseT_Half },
        { ETH_BASE_T, ETH_DUPLEX_FULL, BASE_TEN_STR, SUPPORTED_10baseT_Full },
        { ETH_BASE_T, ETH_DUPLEX_FULL, BASE_ONE_HUNDRED_STR, SUPPORTED_100baseT_Full },
        { ETH_BASE_T, ETH_DUPLEX_FULL, BASE_ONE_THOUSAND_STR, SUPPORTED_1000baseT_Full },
        { ETH_BASE_T, ETH_DUPLEX_FULL, BASE_TEN_THOUSAND_STR, SUPPORTED_10000baseT_Full },
    };

    base_print_debug(DEBUG_MSG_FUN_CALLED_WITH_ONE_PARAM, __func__, interface_name);

    if (with_advertising) {
        snprintf(obj_name, MAX_STR_LEN, "%s", ETH_STARTUP_ADVERTISED_MODES_STR);
    } else {
        snprintf(obj_name, MAX_STR_LEN, "%s", ETH_SUPPORTED_IF_MODE_STR);
    }

    for (i = 0; i < 7; i++) {
        if (EXIT_SUCCESS == check_link_modes( BASE_RC_NO, interface_name,
            link_mode_array[i].speed, link_mode_array[i].duplex,
            link_mode_array[i].mode, obj_name))
        ret_val = ret_val | link_mode_array[i].mask;
    }

    return ret_val;
}

int get_eth_link_detected(void *result_array, unsigned int row_num, unsigned int col_num,
                          char *interface_name)
{
    return get_object(IETF_INTERFACES, ETH_LINK_DETECTED_STR, BASE_RC_NO,
                      result_array, row_num, col_num,
                      interface_name);
}

int store_advertised_link_modes(void *result_array, unsigned int row_num, unsigned int col_num,
                                char *interface_name)
{
    sys_call_spec_t tmp_sys_call_spec;

    base_print_debug(DEBUG_MSG_FUN_CALLED_WITH_ONE_PARAM, __func__, interface_name);

    tmp_sys_call_spec = assemble_sys_call_structure(IETF_INTERFACES, ETH_STORE_ADVERTISED_MODES, BASE_RC_NO);
    // this function can receive two types of duplex values
    // 4th combinator, 1st callback - from configuration
    snprintf(tmp_sys_call_spec.parse_cb[4][0].match_case, MAX_STR_LEN, "%s", interface_name);
    set_rc_sys_call_spec(tmp_sys_call_spec); // store customized system call structure
    // BASE_RC_YES means "use runtime configurable value"
    return get_object(IETF_INTERFACES, ETH_ADVERTISED_IF_MODE_STR, BASE_RC_YES,
                      result_array, row_num, col_num,
                      interface_name);
}

int get_supported_link_modes(void *result_array, unsigned int row_num, unsigned int col_num,
                             char *interface_name)
{
    sys_call_spec_t tmp_sys_call_spec;

    base_print_debug(DEBUG_MSG_FUN_CALLED_WITH_ONE_PARAM, __func__, interface_name);

    tmp_sys_call_spec = assemble_sys_call_structure(IETF_INTERFACES,
                                                    ETH_SUPPORTED_MODES,
                                                    BASE_RC_NO);
    // this function can receive two types of duplex values
    // 4th combinator, 1st callback - from configuration
    snprintf(tmp_sys_call_spec.parse_cb[4][0].match_case, MAX_STR_LEN, "%s", interface_name);
    set_rc_sys_call_spec(tmp_sys_call_spec); // store customized system call structure
    // BASE_RC_YES means "use runtime configurable value"
    return get_object(IETF_INTERFACES, ETH_SUPPORTED_MODES_STR, BASE_RC_YES,
                      result_array, row_num, col_num,
                      interface_name);
}
