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

#include "base_private_combinators.h"
#include "base_private_defines.h"
#include "base_defines.h"
#include "base_private_parsers.h"
#include "base_prints.h"
#include "base_linux_tool_helpers.h"

typedef enum {
    BASE_COMB_REPEAT_UNITL_NULL = 0,
    BASE_COMB_GET_NEXT_LINE,
    BASE_COMB_OTHER
} base_comb_flag;

struct combinator_data {
    int loop;
    base_status_t status;
    base_status_t callback_status;
    char line[MAX_STR_LEN];
    int len;
    sys_call_spec_t tmp_sys_call_spec;
    parser_fn_t tmp_calllback;
    base_comb_flag comb_flag;
};

static int base_set_callback(struct combinator_data *comb, int obj_index, int comb_index);

static int base_check_callback_status(struct combinator_data *comb);

static int base_set_and_check_callback(struct combinator_data *comb,
                                       module_index_t module_index, int obj_index,
                                       int comb_index, base_rc_t rc_flag,
                                       void *result_array, unsigned int row_num, unsigned int col_num);

static base_status_t base_check_index(module_index_t module_index, int obj_index,
                                      int comb_index, struct combinator_data *comb);

static base_status_t base_combinator_helper(struct combinator_data *comb,
                                            module_index_t module_index, int obj_index,
                                            FILE *output, int comb_index, base_rc_t rc_flag,
                                            void *result_array, unsigned int row_num, unsigned int col_num);

base_status_t combinator_comb(module_index_t module_index, int obj_index,
                              int comb_index, FILE *output, base_rc_t rc_flag,
                              void *result_array, unsigned int row_num, unsigned int col_num)
{
    int loop = 1; // start on next combinator
    base_status_t status = BASE_SUCCEEDED;
    sys_call_spec_t tmp_sys_call = assemble_sys_call_structure(module_index, obj_index, rc_flag);

    base_print_debug(DEBUG_MSG_FUN_CALLED, __func__);

    if (!are_object_indices_valid(module_index, obj_index, comb_index, 0)) {
        status = BASE_FAILED;
    } else {
        while (loop < tmp_sys_call.num_of_combinators) {
            base_print_debug("DEBUG: %s(): obj_index (%d), loop (%d), comb_index (%d)\n",
                             __func__, obj_index, loop, comb_index);
            if (NULL == tmp_sys_call.combinator[loop]) {
                base_print_warning("Combinator does not exist: obj_index (%d), loop (%d), comb_index (%d)",
                                   obj_index, loop, comb_index);
                status = BASE_FAILED;
                break;
            }
            status |= tmp_sys_call.combinator[loop](module_index, obj_index,
                                                    loop /* = combinator index */, output, rc_flag,
                                                    result_array, row_num, col_num);
            if (BASE_FAILED & status) {
                break; // sub-combinator failed - break this too
            }
            loop++;
        }
    }

    return status;
}

base_status_t repeat_until_null_with_check(module_index_t module_index, int obj_index,
                                           int comb_index, FILE *output, base_rc_t rc_flag,
                                           void *result_array, unsigned int row_num, unsigned int col_num)
{
    struct combinator_data comb = {
            0,
            BASE_SUCCEEDED,
            BASE_SUCCEEDED,
            {0},
            MAX_STR_LEN,
            assemble_sys_call_structure(module_index, obj_index, rc_flag),
            NULL,
            BASE_COMB_REPEAT_UNITL_NULL
    };
    memset(comb.line, 0, comb.len);
    if (!base_combinator_helper(&comb, module_index, obj_index,
                                output, comb_index, rc_flag,
                                result_array, row_num, col_num)) {
        return comb.status;
    }

    return comb.status;
}

base_status_t repeat_check_and_get_next_line(module_index_t module_index, int obj_index,
                                             int comb_index, FILE *output, base_rc_t rc_flag,
                                             void *result_array, unsigned int row_num, unsigned int col_num)
{
    struct combinator_data comb = {
            0,
            BASE_SUCCEEDED,
            BASE_SUCCEEDED,
            {0},
            MAX_STR_LEN,
            assemble_sys_call_structure(module_index, obj_index, rc_flag),
            NULL,
            BASE_COMB_GET_NEXT_LINE
    };
    memset(comb.line, 0, comb.len);

    base_print_debug(DEBUG_MSG_FUN_CALLED, __func__);

    if (!base_combinator_helper(&comb, module_index, obj_index,
                                output, comb_index, rc_flag,
                                result_array, row_num, col_num)) {
        return comb.status;
    }

    return comb.status;
}

base_status_t OR_combinator(module_index_t module_index, int obj_index,
                            int comb_index, FILE *output, base_rc_t rc_flag,
                            void *result_array, unsigned int row_num, unsigned int col_num)
{
    (void)output;

    struct combinator_data comb = {
            0,
            BASE_SUCCEEDED,
            BASE_SUCCEEDED,
            {0},
            MAX_STR_LEN,
            assemble_sys_call_structure(module_index, obj_index, rc_flag),
            NULL,
            BASE_COMB_OTHER
    };
    memset(comb.line, 0, comb.len);

    base_print_debug(DEBUG_MSG_FUN_CALLED, __func__);

    if (NULL == fgets(comb.line, comb.len, output)) {
        // only one line is taken
        comb.status = BASE_FAILED;
    }

    if (BASE_FAILED == base_check_index(module_index, obj_index, comb_index, &comb)) {
        return comb.status;
    }

    while ((comb.loop < comb.tmp_sys_call_spec.num_of_cb[comb_index]) &&
           !(BASE_FAILED & comb.status)) {
        if (!base_set_callback(&comb, obj_index, comb_index)) {
            break;
        }
        comb.status |= (*comb.tmp_calllback)(module_index, obj_index,
                                             comb_index, comb.loop, comb.line, rc_flag,
                                             result_array, row_num, col_num);
        if ((BASE_PARSING_SUCCEEDED & comb.status) ||
            (BASE_VALIDATION_SUCCEEDED & comb.status)) {
            // first with match breaks loop
            break;
        }
        comb.loop++;
    }
    return comb.status;
}

base_status_t AND_combinator(module_index_t module_index, int obj_index,
                             int comb_index, FILE *output, base_rc_t rc_flag,
                             void *result_array, unsigned int row_num, unsigned int col_num)
{
    struct combinator_data comb = {
        0,
        BASE_SUCCEEDED,
        BASE_SUCCEEDED,
        {0},
        MAX_STR_LEN,
        assemble_sys_call_structure(module_index, obj_index, rc_flag),
        NULL,
        BASE_COMB_OTHER
    };
    memset(comb.line, 0, comb.len);
    base_print_debug(DEBUG_MSG_FUN_CALLED, __func__);

    if (BASE_FAILED == base_check_index(module_index, obj_index, comb_index, &comb)) {
        return comb.status;
    }

    while (NULL != fgets(comb.line, comb.len, output)) {
        // line loop
        terminate_string(comb.line);
        while ((comb.loop < comb.tmp_sys_call_spec.num_of_cb[comb_index]) &&
               !(BASE_FAILED & comb.status)) {
            // callbacks loop
            if (!base_set_callback(&comb, obj_index, comb_index)) {
                break;
            }
            comb.callback_status = (*comb.tmp_calllback)(module_index, obj_index,
                                                         comb_index, comb.loop, comb.line, rc_flag,
                                                         result_array, row_num, col_num);
            if (BASE_PARSING_FAILED & comb.callback_status) {
                // go to next line
                break;
            }
            comb.status |= comb.callback_status;
            comb.callback_status = BASE_SUCCEEDED; // clear it for next callback
            comb.tmp_calllback = NULL; // clear it for next callback
            comb.loop++; // next callback
        }
        // end callbacks loop

        if ((comb.loop == comb.tmp_sys_call_spec.num_of_cb[comb_index]) &&
            !(BASE_PARSING_FAILED & comb.callback_status) &&
            !(BASE_VALIDATION_FAILED & comb.callback_status)) {
            // all callbacks and validators are passed
            break;
        }
        comb.loop = 0; // empty loop for next line
        comb.status = BASE_SUCCEEDED; // reset status for next line
        memset(comb.line, 0, comb.len);
    }
    // end line loop

    /* if any callback (parser or validator) finds any match case,
     * status will not be BASE_SUCCEEDED
     */
    if (BASE_SUCCEEDED == comb.status) {
        comb.status = BASE_FAILED;
        // in that case there is no match case on any line, so combinator failed
    }
    return comb.status;
}

base_status_t no_combinator(module_index_t module_index, int obj_index,
                            int comb_index, FILE *output, base_rc_t rc_flag,
                            void *result_array, unsigned int row_num, unsigned int col_num)
{
    (void)output;

    int status = BASE_SUCCEEDED;
    sys_call_spec_t tmp = assemble_sys_call_structure(module_index, obj_index, rc_flag);

    base_print_debug(DEBUG_MSG_FUN_CALLED, __func__);

    if (!are_object_indices_valid(module_index, obj_index, comb_index, 0)) {
        status = BASE_FAILED;
    } else {
        // always only one callback - first one
        if (NULL == tmp.parse_cb[comb_index][0].callback) {
            status |= BASE_FAILED;
            base_print_warning("Callback does not exist: obj_index (%d), comb_index (%d)",
                               obj_index, comb_index);
        } else {
            status |= tmp.parse_cb[comb_index][0].callback(module_index, obj_index,
                                                           comb_index, 0, result_array, rc_flag,
                                                           result_array, row_num, col_num);
        }
    }
    return status;
}

base_status_t repeat_until_eof_and_count(module_index_t module_index, int obj_index,
                                         int comb_index, FILE *output, base_rc_t rc_flag,
                                         void *result_array, unsigned int row_num, unsigned int col_num)
{
    struct combinator_data comb = {
        0,
        BASE_SUCCEEDED,
        BASE_SUCCEEDED,
        {0},
        MAX_STR_LEN,
        assemble_sys_call_structure(module_index, obj_index, rc_flag),
        NULL,
        BASE_COMB_OTHER
    };
    int cnt = 0;
    int cnt_flag = 0;
    memset(comb.line, 0, comb.len);

    base_print_debug(DEBUG_MSG_FUN_CALLED, __func__);

    if (!are_object_indices_valid(module_index, obj_index, comb_index, 0)) {
        comb.status = BASE_FAILED;
    } else {
        while (NULL != fgets(comb.line, comb.len, output)) {
            terminate_string(comb.line);
            while ((comb.loop < comb.tmp_sys_call_spec.num_of_cb[comb_index]) &&
                   !(BASE_FAILED & comb.status)) {
                if (!base_set_callback(&comb, obj_index, comb_index)) {
                    break;
                }
                comb.callback_status = (*comb.tmp_calllback)(module_index, obj_index,
                                                             comb_index, comb.loop, comb.line, rc_flag,
                                                             result_array, row_num, col_num);
                if (!base_check_callback_status(&comb)) {
                    cnt_flag = 0;
                    break;
                }
                cnt_flag = 1;
                comb.status |= comb.callback_status;
                comb.callback_status = BASE_SUCCEEDED; // clear it for next callback
                comb.tmp_calllback = NULL; // clear it for next callback
                comb.loop++; // next callback
            }
            if (0 != cnt_flag) cnt++;
            if ((BASE_PARSING_SUCCEEDED & comb.status) && (BASE_VALIDATION_FAILED & comb.status)) {
                base_print_warning(ERROR_MSG_FUN_AND_MSG, __func__,
                                   "validator returned bad status");
                break; // break combinator
            }
            comb.loop = 0; // empty loop for next line
            memset(comb.line, 0, comb.len);
        }
        if (BASE_PARSING_SUCCEEDED & comb.status) {
            // if parser found match case even once, global parsing passed
            comb.status = comb.status & (~BASE_PARSING_FAILED); // erase BASE_PARSING_FAILED
        }
        // combinator covers a lot of different scenarios - this step is crucial for streams with more than one line
    }
    if (0 == cnt) {
        comb.status = BASE_PARSING_SUCCEEDED;
    }
    snprintf(result_array, col_num, "%d", cnt);
    return comb.status;
}

static int base_set_callback(struct combinator_data *comb, int obj_index, int comb_index)
{
    int ret = 1;
    comb->tmp_calllback = comb->tmp_sys_call_spec.parse_cb[comb_index][comb->loop].callback;
    if (NULL == comb->tmp_calllback) {
        comb->status |= BASE_FAILED; // this status will break combinator
        base_print_error("Callback does not exist: obj_index (%d), loop (%d), comb_index (%d).",
                         obj_index, comb->loop, comb_index);
        ret = 0;
    }
    return ret;
}

static int base_check_callback_status(struct combinator_data *comb)
{
    int ret = 1;
    if (BASE_PARSING_FAILED & comb->callback_status) {
        base_print_debug(DEBUG_MSG_FUN_NAME_AND_MSG, __func__,
                         "parser returned bad status");
        comb->status |= comb->callback_status;
        return 0;
    }
    comb->status |= comb->callback_status;
    comb->callback_status = BASE_SUCCEEDED; // clear it for next callback
    return ret;
}

static int base_set_and_check_callback(struct combinator_data *comb, module_index_t module_index, int obj_index,
                                       int comb_index, base_rc_t rc_flag,
                                       void *result_array, unsigned int row_num, unsigned int col_num)
{
    int ret = 1;

    if (!base_set_callback(comb, obj_index, comb_index)) {
        return 0;
    }
    comb->callback_status = (*comb->tmp_calllback)(module_index, obj_index, comb_index,
                                                   comb->loop, comb->line, rc_flag,
                                                   result_array, row_num, col_num);
    if (!base_check_callback_status(comb)) {
        return 0;
    }
    comb->tmp_calllback = NULL; // clear it for next callback
    comb->loop++; // next callback

    return ret;
}

static base_status_t base_check_index(module_index_t module_index, int obj_index,
                                      int comb_index, struct combinator_data *comb)
{
    comb->status = BASE_SUCCEEDED;
    if (!are_object_indices_valid(module_index, obj_index, comb_index, 0)) {
        comb->callback_status = BASE_FAILED;
        return comb->callback_status;
    }
    return comb->callback_status;
}

static base_status_t base_combinator_helper(struct combinator_data *comb, module_index_t module_index, int obj_index,
                                            FILE *output, int comb_index, base_rc_t rc_flag,
                                            void *result_array, unsigned int row_num, unsigned int col_num)
{

    if (BASE_FAILED == base_check_index(module_index, obj_index, comb_index, comb)) {
        return comb->status;
    }

    while (fgets(comb->line, comb->len, output)) {
        terminate_string(comb->line);
        while ((comb->loop < comb->tmp_sys_call_spec.num_of_cb[comb_index]) &&
               !(BASE_FAILED & comb->status)) {
            if (!base_set_and_check_callback(comb, module_index, obj_index, comb_index,
                                             rc_flag, result_array, row_num, col_num)) {
                break;
            }
            switch (comb->comb_flag) {
            case BASE_COMB_REPEAT_UNITL_NULL:
                break;
            case BASE_COMB_GET_NEXT_LINE:
                memset(comb->line, 0, comb->len);
                if (NULL == fgets(comb->line, comb->len, output)) {
                    break;
                }
                if (!base_set_and_check_callback(comb, module_index, obj_index, comb_index,
                                                 rc_flag, result_array, row_num, col_num)) {
                    break;
                }
                break;
            default:
                base_print_warning(ERROR_MSG_FUN_AND_MSG, __func__,
                                   "The combinator flag is not correct.");
                return BASE_FAILED;
            }
        }
        switch (comb->comb_flag) {
        case BASE_COMB_REPEAT_UNITL_NULL:
            if ((BASE_PARSING_SUCCEEDED & comb->status) &&
                (BASE_VALIDATION_FAILED & comb->status)) {
                base_print_warning(ERROR_MSG_FUN_AND_MSG, __func__,
                                   "validator returned bad status");
                break; // break combinator
            }
            break;
        case BASE_COMB_GET_NEXT_LINE:
            break;
        default:
            base_print_warning(ERROR_MSG_FUN_AND_MSG, __func__,
                               "The combinator flag is not correct.");
            return BASE_FAILED;
        }
        comb->loop = 0; // empty loop for next line
        memset(comb->line, 0, comb->len);
    }
    if (BASE_PARSING_SUCCEEDED & comb->status) {
        // if parser found match case even once, global parsing passed
        comb->status = comb->status & (~BASE_PARSING_FAILED); // erase BASE_PARSING_FAILED
    }
    // combinator covers a lot of different scenarios - this step is crucial for streams with more than one line

    return comb->status;
}
