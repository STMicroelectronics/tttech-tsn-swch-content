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

#include "base_private_translators.h"

int translate_single_string_using_map(char *value, char map[][2][MAX_STR_LEN])
{
    int ret_value = EXIT_SUCCESS;
    int map_pair = 0;
    int flag = 1;

    base_print_debug(DEBUG_MSG_FUN_CALLED, __func__);

    if ((NULL != value) && (0 != strncmp(value, "", MAX_STR_LEN))) {
        while ((NULL != map[map_pair]) &&
               (0 != strncmp(map[map_pair][0], "", MAX_STR_LEN)) &&
               (0 != strncmp(map[map_pair][1], "", MAX_STR_LEN))) {
            if (0 == strncmp(value, map[map_pair][0], MAX_STR_LEN)) {
                // first string from the map is key
                // second string is new mapped value
                strncpy(value, map[map_pair][1], MAX_STR_LEN);
                flag = 0;
            }

            /* if flag=0 the map pair matches, break the loop */
            if (0 == flag) break;

            map_pair++;
        }
        if (1 == flag) {
            // last values element did not match any of map keys
            ret_value = EXIT_FAILURE;
            base_print_warning("%s(): (%s) element did not match any of map keys.",
                               __func__, value);
        }
    }

    return ret_value;
}
