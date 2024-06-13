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

#include <stdarg.h>
#include "base_prints.h"

/* global pointers which hold addresses of client printing functions */
print_fn_t base_print_error_fn_ptr; // for errors
print_fn_t base_print_warning_fn_ptr; // for warnings

void base_execute_print_error_fn(const char *error_message)
{
    if (NULL != base_print_error_fn_ptr) {
        // "" is to fulfill declaration of client printing function
        (*base_print_error_fn_ptr)(error_message);
    }
}

void base_execute_print_warning_fn(const char *error_message)
{
    if (NULL != base_print_warning_fn_ptr) {
        // "" is to fulfill declaration of client printing function
        (*base_print_warning_fn_ptr)(error_message);
    }
}

void base_print_error(const char *format, ...)
{
    char msg_bufer[2*MAX_STR_LEN] = {'\0'};

    va_list args;
    va_start(args, format);
    // write formatted message to msg_bufer
    vsnprintf(msg_bufer, sizeof(msg_bufer), format, args);
    msg_bufer[sizeof(msg_bufer) - 1] = '\0';
    base_execute_print_error_fn(msg_bufer); // call external handler
    va_end (args);
}

void base_print_warning(const char *format, ...)
{
    char msg_bufer[2*MAX_STR_LEN] = {'\0'};

    va_list args;
    va_start (args, format);
    // write formatted message to msg_bufer
    vsnprintf(msg_bufer, sizeof(msg_bufer), format, args);
    msg_bufer[sizeof(msg_bufer) - 1] = '\0';
    base_execute_print_warning_fn(msg_bufer); // call external handler
    va_end (args);
}

void set_base_print_error_fn_ptr(print_fn_t custom_print_fn_ptr)
{
    base_print_error_fn_ptr = custom_print_fn_ptr;
}

void set_base_print_warning_fn_ptr(print_fn_t custom_print_fn_ptr)
{
    base_print_warning_fn_ptr = custom_print_fn_ptr;
}
