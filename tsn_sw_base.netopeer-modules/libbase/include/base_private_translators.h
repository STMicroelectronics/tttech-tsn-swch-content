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

#ifndef LIBBASE_INCLUDE_BASE_PRIVATE_TRANSLATORS_H_
#define LIBBASE_INCLUDE_BASE_PRIVATE_TRANSLATORS_H_

#include <string.h>
#include <stdlib.h>
#include "base_prints.h"

/**
 * @brief Translates string from 'value' into new string from 'map' if first
 * string from 'map' pair is same as string from 'values'.
 *
 * @param[out]  value   String for manipulation (char [MAX_STR_LEN]).
 * @param[in]   map     Collection of string pairs key and new value.
 * @return EXIT_FAILURE or EXIT_SUCCESS
 */
int translate_single_string_using_map(char *value, char map[][2][MAX_STR_LEN]);

#endif /* LIBBASE_INCLUDE_BASE_PRIVATE_TRANSLATORS_H_ */
