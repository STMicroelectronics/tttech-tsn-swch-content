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

#include "base_private_defines.h"
#include "base_interfaces_linux_tools.h"
#include "base_q_bridge_linux_tools.h"
#include "base_private_parsers.h"
#include "base_private_validators.h"
#include "base_private_combinators.h"

char if_map[IF_MAX_OBJ_NUMBER][MAX_STR_LEN] = {
    // map between object index and object name for ietf-interfaces module
    [IF_NAMES]                      = IF_NAME_STR,
    [IF_ENABLED]                    = IF_ENABLE_STR,
    [ETH_IF_ENABLE]                 = ETH_IF_ENABLE_STR,
    [ETH_DUPLEX]                    = ETH_DUPLEX_STR,
    [ETH_SPEED]                     = ETH_SPEED_STR,
    [ETH_ADVERTISE]                 = ETH_ADVERTISE_STR,
    [ETH_MAX_FRAME_LEN]             = ETH_MAX_FRAME_LEN_STR,
    [IF_TX_STATISTICS]              = IF_TX_STATISTICS_STR,
    [IF_RX_STATISTICS]              = IF_RX_STATISTICS_STR,
    [ETH_SUPPORTED_IF_MODE]         = ETH_SUPPORTED_IF_MODE_STR,
    [ETH_LINK_DETECTED]             = ETH_LINK_DETECTED_STR,
    [ETH_STORE_ADVERTISED_MODES]    = ETH_ADVERTISED_IF_MODE_STR,
    [ETH_SUPPORTED_MODES]           = ETH_SUPPORTED_MODES_STR,
    [ETH_STARTUP_ADVERTISED_MODES]  = ETH_STARTUP_ADVERTISED_MODES_STR,
    [Q_BRIDGE_PRIO_REG]             = Q_BRIDGE_PRIO_REG_STR,
    [Q_BRIDGE_TRAFFIC_CLASS]        = Q_BRIDGE_TRAFFIC_CLASS_STR,
};

// Commands used for get system calls
char if_get_sys_call_commands[IF_MAX_OBJ_NUMBER][MAX_STR_LEN] = {
    [IF_NAMES]                      = "ip -o link show",
    [IF_ENABLED]                    = "ip -o link show %s",
    [ETH_IF_ENABLE]                 = "ethtool %s",
    [ETH_DUPLEX]                    = "ethtool %s",
    [ETH_SPEED]                     = "ethtool %s",
    [ETH_MAX_FRAME_LEN]             = "ip -o link show %s",
    [IF_TX_STATISTICS]              = "ip -s link show %s",
    [IF_RX_STATISTICS]              = "ip -s link show %s",
    [ETH_SUPPORTED_IF_MODE]         = "ethtool %s",
    [ETH_LINK_DETECTED]             = "ethtool %s",
    [ETH_STORE_ADVERTISED_MODES]    = "ethtool %s",
    [ETH_SUPPORTED_MODES]   = "ethtool %s",
    [ETH_STARTUP_ADVERTISED_MODES]  = PORT_ADVERTISE_FULL_PATH,
};

syscall_fn_t if_syscalls[IF_MAX_OBJ_NUMBER] = {
    [IF_NAMES]                      = (syscall_fn_t)&get_str_sys_call,
    [IF_ENABLED]                    = (syscall_fn_t)&get_str_sys_call,
    [ETH_IF_ENABLE]                 = (syscall_fn_t)&get_str_sys_call,
    [ETH_DUPLEX]                    = (syscall_fn_t)&get_str_sys_call,
    [ETH_SPEED]                     = (syscall_fn_t)&get_str_sys_call,
    [ETH_MAX_FRAME_LEN]             = (syscall_fn_t)&get_str_sys_call,
    [IF_TX_STATISTICS]              = (syscall_fn_t)&get_str_sys_call,
    [IF_RX_STATISTICS]              = (syscall_fn_t)&get_str_sys_call,
    [ETH_SUPPORTED_IF_MODE]         = (syscall_fn_t)&get_str_sys_call,
    [ETH_LINK_DETECTED]             = (syscall_fn_t)&get_str_sys_call,
    [ETH_STORE_ADVERTISED_MODES]    = (syscall_fn_t)&get_str_sys_call,
    [ETH_SUPPORTED_MODES]   = (syscall_fn_t)&get_str_sys_call,
    [ETH_STARTUP_ADVERTISED_MODES]  = (syscall_fn_t)&read_file_sys_call,
};

// Number of combinators per system call (object index)
int if_num_of_combinators[IF_MAX_OBJ_NUMBER] = {
    [IF_NAMES]                      = 5,
    [IF_ENABLED]                    = 5,
    [ETH_IF_ENABLE]                 = 4,
    [ETH_DUPLEX]                    = 2,
    [ETH_SPEED]                     = 2,
    [ETH_MAX_FRAME_LEN]             = 2,
    [IF_TX_STATISTICS]              = 2,
    [IF_RX_STATISTICS]              = 2,
    [ETH_SUPPORTED_IF_MODE]         = 2,
    [ETH_LINK_DETECTED]             = 4,
    [ETH_STORE_ADVERTISED_MODES]    = 5,
    [ETH_SUPPORTED_MODES]           = 5,
    [ETH_STARTUP_ADVERTISED_MODES]  = 2,
};

combinator_fn_t if_combinators[IF_MAX_OBJ_NUMBER][MAX_COMBINATORS_NUM] = {
    [IF_NAMES]                      = {&combinator_comb, &repeat_until_null_with_check, &no_combinator, &no_combinator, &no_combinator},
    [IF_ENABLED]                    = {&combinator_comb, &OR_combinator, &no_combinator, &no_combinator, &no_combinator},
    [ETH_IF_ENABLE]                 = {&combinator_comb, &repeat_until_null_with_check, &no_combinator, &no_combinator},
    [ETH_DUPLEX]                    = {&combinator_comb, &repeat_until_null_with_check},
    [ETH_SPEED]                     = {&combinator_comb, &repeat_until_null_with_check},
    [ETH_MAX_FRAME_LEN]             = {&combinator_comb, &repeat_until_null_with_check},
    [IF_TX_STATISTICS]              = {&combinator_comb, &repeat_check_and_get_next_line},
    [IF_RX_STATISTICS]              = {&combinator_comb, &repeat_check_and_get_next_line},
    [ETH_SUPPORTED_IF_MODE]         = {&combinator_comb, &repeat_until_null_with_check},
    [ETH_LINK_DETECTED]             = {&combinator_comb, &repeat_until_null_with_check, &no_combinator, &no_combinator},
    [ETH_STORE_ADVERTISED_MODES]    = {&combinator_comb, &repeat_until_null_with_check, &no_combinator, &no_combinator, &no_combinator},
    [ETH_SUPPORTED_MODES]   = {&combinator_comb, &repeat_until_null_with_check, &no_combinator, &no_combinator, &no_combinator},
    [ETH_STARTUP_ADVERTISED_MODES]  = {&combinator_comb, &repeat_until_null_with_check},
};

// We need a number of callbacks per combinator if we are using more than one
int if_num_of_cb[IF_MAX_OBJ_NUMBER][MAX_COMBINATORS_NUM] = {
    // A combinator uses a predefined amount of callbacks, actually set here
    [IF_NAMES]                      = {0, 2, 1, 1, 1},
    [IF_ENABLED]                    = {0, 2, 1, 1, 1},
    [ETH_IF_ENABLE]                 = {0, 1, 1, 1},
    [ETH_DUPLEX]                    = {0, 1},
    [ETH_SPEED]                     = {0, 1},
    [ETH_MAX_FRAME_LEN]             = {0, 1},
    [IF_TX_STATISTICS]              = {0, 2},
    [IF_RX_STATISTICS]              = {0, 2},
    [ETH_SUPPORTED_IF_MODE]         = {0, 1},
    [ETH_LINK_DETECTED]             = {0, 1, 1, 1},
    [ETH_STORE_ADVERTISED_MODES]    = {0, 1, 1, 1, 1},
    [ETH_SUPPORTED_MODES]           = {0, 1, 1, 1, 1},
    [ETH_STARTUP_ADVERTISED_MODES]  = {0, 1},
};

// read it like: at object index, at combinator index
parser_spec_t if_parsing_cb[IF_MAX_OBJ_NUMBER][MAX_COMBINATORS_NUM][MAX_CALLBACKS_NUM] = {
    [IF_NAMES][1]                       = {{&find_on_line, LESS_THAN_STR}, {&find_and_get_next_word, COLON_AND_SPACE_STR}},
    [IF_NAMES][2]                       = {{&array_until_match_case, COLON_STR}},
    [IF_NAMES][3]                       = {{&array_delete_element, BASE_GENERIC_IF_NAME}},
    [IF_NAMES][4]                       = {{&array_uniq_elements, ""}},
    [IF_ENABLED][1]                     = {{&find_on_line_and_fill_result_array, BASE_LOWER_UP}, {&find_on_line_and_fill_result_array, BASE_NO_CARRIER}},
    [IF_ENABLED][2]                     = {{&validate_string_content, BASE_LOWER_UP}},
    [IF_ENABLED][3]                     = {{&validate_string_content, BASE_NO_CARRIER}},
    [IF_ENABLED][4]                     = {{&validate_string_content, ""}}, // it is allowed (interface is down)
    [ETH_IF_ENABLE][1]                  = {{&find_and_get_next_word, BASE_ETH_AUTO_NEG}},
    [ETH_IF_ENABLE][2]                  = {{&validate_string_content, BASE_ON}},
    [ETH_IF_ENABLE][3]                  = {{&validate_string_content, BASE_OFF}},
    [ETH_DUPLEX][1]                     = {{&find_and_get_next_word, BASE_ETH_DUPLEX}},
    [ETH_SPEED][1]                      = {{&find_and_get_next_word, BASE_ETH_SPEED}},
    [ETH_MAX_FRAME_LEN][1]              = {{&find_and_get_next_word, BASE_MTU}},
    [IF_TX_STATISTICS][1]               = {{&find_on_line, BASE_IF_TX}, {&parse_dec_values_separated_with_spaces, ""}},
    [IF_RX_STATISTICS][1]               = {{&find_on_line, BASE_IF_RX}, {&parse_dec_values_separated_with_spaces, ""}},
    [ETH_SUPPORTED_IF_MODE][1]          = {{&find_on_line, ""}}, // will be rewritten
    [ETH_LINK_DETECTED][1]              = {{&find_and_get_next_word, BASE_ETH_LINK_DETECTED}},
    [ETH_LINK_DETECTED][2]              = {{&validate_string_content, BASE_YES}},
    [ETH_LINK_DETECTED][3]              = {{&validate_string_content, BASE_NO}},
    [ETH_STORE_ADVERTISED_MODES][1]     = {{&get_all_lines,""}},
    [ETH_STORE_ADVERTISED_MODES][2]     = {{&array_take_values_after_match_case, ETH_SUPPORTS_AUTO_NEG}},
    [ETH_STORE_ADVERTISED_MODES][3]     = {{&array_take_values_before_match_case, ETH_ADVERTISE_PAUSE_FRAME}},
    [ETH_STORE_ADVERTISED_MODES][4]     = {{&array_write_to_file, ""}},
    [ETH_SUPPORTED_MODES][1]            = {{&get_all_lines,""}},
    [ETH_SUPPORTED_MODES][2]            = {{&array_take_values_after_match_case, ETH_SUPPORTED_PORTS}},
    [ETH_SUPPORTED_MODES][3]            = {{&array_take_values_before_match_case, ETH_SUPPORTED_PAUSE_FRAME}},
    [ETH_SUPPORTED_MODES][4]            = {{&array_write_to_file, ""}},
    [ETH_STARTUP_ADVERTISED_MODES][1]   = {{&find_on_line, ""}}, // will be rewritte
};

// Commands used for set system calls
char if_set_sys_call_commands[IF_MAX_OBJ_NUMBER][MAX_STR_LEN] = {
    [IF_ENABLED]        = "ip link set dev %s %s",
    [ETH_IF_ENABLE]     = "ethtool -s %s autoneg %s",
    [ETH_ADVERTISE]     = "ethtool -s %s advertise %s",
    [ETH_DUPLEX]        = "ethtool -s %s speed %s duplex %s",
    [ETH_SPEED]         = "ethtool -s %s speed %s duplex %s",
};
