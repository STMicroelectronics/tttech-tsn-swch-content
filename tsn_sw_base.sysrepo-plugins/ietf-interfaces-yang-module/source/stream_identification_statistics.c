/*
 * TTTech ietf-interfaces-yang-module
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

#include "stream_identification_statistics.h"

int sid_per_port_per_stream_counters(struct lyd_node **parent,
                                     char *interface_name)
{
    char path[MAX_STR_LEN] = "";
    char value[MAX_STR_LEN] = "";
    char err_msg[MAX_STR_LEN] = "";
    char bridge_names[MAX_NUM_OF_INTERFACES][MAX_STR_LEN] = {0};
    char direction[MAX_STR_LEN] = "";
    char ord_str[MAX_STR_LEN] = "";
    char stream_handles[MAX_STR_LEN][MAX_STR_LEN] = {0};
    tsn_return_value tsn_ret_val = E_TSN_LIB_NO_ERROR;
    struct port_hdl *brhdl = NULL;
    struct port_hdl *port_hdl = NULL;
    struct ident_params sid_ident = {0};
    uint32_t port_id = 0;
    uint32_t num_of_stream_handles = 0;
    uint16_t *ord = NULL;
    uint16_t port_map = 0;
    uint32_t ret = 0u;
    uint32_t cnt_in = 0;
    uint32_t cnt_out = 0;
    uint32_t sum_of_cnt_in = 0;
    uint32_t sum_of_cnt_out = 0;
    uint32_t i = 0;
    uint32_t str_hdl_id = 0;
    uint32_t str_hdl_tmp = 0;

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    if (EXIT_FAILURE == get_bridge_names(bridge_names,
                                         MAX_NUM_OF_INTERFACES, MAX_STR_LEN)) {
        SRP_LOG_ERR(ERROR_MSG_FUN_AND_MSG, __func__, ERR_BRIDE_NAMES_FAILED_STR);
        return SR_ERR_OPERATION_FAILED;
    }

    /* open bridge to get list of stream-handles */
    brhdl = tsn_port_open_by_name(bridge_names[0]);
    if (NULL == brhdl) {
        SRP_LOG_ERR(ERROR_MSG_MOD_FUNC_EL_MSG, IF_MODULE_NAME_STR, __func__,
                    bridge_names[0], ERR_PORT_NOT_OPEN_STR);
        return SR_ERR_OPERATION_FAILED;
    }

    if (UINT32_MAX == (port_id = tsn_get_id_by_name(brhdl, interface_name))) {
        SRP_LOG_ERR(ERR_MSG_EXT_FMT_STR, LIT_GET_STR, LIT_ID_STR, LIT_VAL_STR,
                    BASE_PORT_STR, interface_name);
    }

    /* get number of stream-handles */
    if (0 != tsn_sid_get_current_cnt(brhdl, &num_of_stream_handles))
    {
        SRP_LOG_ERR(ERROR_MSG_FUN_AND_MSG, __func__,
                    ERR_FAILED_TO_GET_NUMBER_OF_BRIDGE_HANDLES_STR);
        return SR_ERR_OPERATION_FAILED;
    }

    ord = malloc(sizeof(*ord) * (uint32_t)num_of_stream_handles);

    if (0 == tsn_sid_get_entries_list(brhdl, num_of_stream_handles, ord)) {
        for (i = 0; i < num_of_stream_handles; i++) {
            /* get ids of ports for which stream handle function is instantiated */
            if (E_TSN_LIB_NO_ERROR != (tsn_ret_val = tsn_sid_get_outfacin(brhdl, ord[i], &port_map))) {
                snprintf(ord_str, MAX_STR_LEN - 1, "%u", ord[i]);
                SRP_LOG_ERR(ERR_MSG_AND_EXPL_EXT_FMT_STR,
                            LIT_GET_STR, STREAM_ID_OUTFACIN_LIST_STR,
                            LIT_LIST_STR, STREAM_ID_INDEX_STR, ord_str,
                            tsn_lib_get_known_ernno_msg(tsn_ret_val));
            }

            /* update SID state data if the current stream handle function is instantiated onto this port */
            if (port_map & (((uint16_t)1)<< port_id)) {
                /* leaf direction-out-facing key of the list per-port-per-stream-counters */
                snprintf(direction, MAX_STR_LEN, "%s", "true");

                /* leaf handle key of the list per-port-per-stream-counters */
                /* Get current SID ident params */
                if (E_TSN_LIB_NO_ERROR != (tsn_ret_val = tsn_sid_get_ident_params(brhdl, ord[i], &sid_ident))) {
                    snprintf(ord_str, MAX_STR_LEN - 1, "%u", ord[i]);
                    SRP_LOG_ERR(ERR_MSG_AND_EXPL_EXT_FMT_STR,
                                LIT_GET_STR, STREAM_ID_IDENT_STR,
                                LIT_PARAMS_STR, STREAM_ID_INDEX_STR, ord_str,
                                tsn_lib_get_known_ernno_msg(tsn_ret_val));
                }

                /* collect all stream handles */
                snprintf(stream_handles[str_hdl_id], MAX_STR_LEN, "%u",
                         sid_ident.str_hdl);
                str_hdl_id++;
            }
        }
    } else {
        SRP_LOG_ERR(ERROR_MSG_FUN_AND_MSG, __func__,
                    ERR_FAILED_TO_GET_BRIDGE_HANDLES_STR);
        tsn_port_close(brhdl);
        brhdl = NULL;
        return SR_ERR_OPERATION_FAILED;

    }

    /* create an array of unique stream handles for current port */
    str_hdl_id = 0;
    array_uniq_elements(0, 0, 0, 0, NULL, BASE_RC_NO, stream_handles,
                        MAX_STR_LEN, MAX_STR_LEN);

    /* go through all stream handles */
    while ((str_hdl_id < MAX_STR_LEN) &&
           (0 != strncmp(stream_handles[str_hdl_id], "", MAX_STR_LEN))) {
        str_hdl_tmp = (uint32_t)strtoul(stream_handles[str_hdl_id], NULL, 10);
        /* open port */
        port_hdl = tsn_port_open_by_name(interface_name);
        if (NULL != port_hdl) {
            /* get input counter value */
            ret = tsn_sid_get_input_cps_cnt(port_hdl, str_hdl_tmp, &cnt_in);
            if (!ret) {
                /* get output counter value */
                ret = tsn_sid_get_output_cps_cnt(port_hdl, str_hdl_tmp, &cnt_out);
            }
            if (ret) {
                snprintf(err_msg, MAX_STR_LEN,
                         ERR_STREAM_ID_COULD_NOT_FIND_STREAM_HANDLE_STR,
                         interface_name);
                SRP_LOG_INF(ERROR_MSG_FUN_AND_MSG, __func__, err_msg);
            }
            tsn_port_close(port_hdl);
        }

        sum_of_cnt_in = sum_of_cnt_in + cnt_in;
        sum_of_cnt_out = sum_of_cnt_out + cnt_out;

        /* leaf input-pkts, list per-port-per-stream-counters */
        fill_xpath(path, STREAM_ID_PER_PORT_PER_STREAM_INPUT_PACKETS_XPATH,
                   interface_name, direction, stream_handles[str_hdl_id]);
        snprintf(value, MAX_STR_LEN, "%u", cnt_in);
        if (NULL == lyd_new_path(*parent, NULL, path, value, 0, 0)) {
            SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                        STREAM_ID_INPUT_PACKETS_STR, ERR_NOT_CREATED_ELEMENT_STR);
        }

        /* leaf output-pkts, list per-port-per-stream-counters */
        fill_xpath(path, STREAM_ID_PER_PORT_PER_STREAM_OUTPUT_PACKETS_XPATH,
                   interface_name, direction, stream_handles[str_hdl_id]);
        snprintf(value, MAX_STR_LEN, "%u", cnt_out);
        if (NULL == lyd_new_path(*parent, NULL, path, value, 0, 0)) {
            SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                        STREAM_ID_OUTPUT_PACKETS_STR, ERR_NOT_CREATED_ELEMENT_STR);
        }
        str_hdl_id++;
    }

    /* Close bridge handle */
    tsn_port_close(brhdl);
    brhdl = NULL;

    /* leaf input-packets, container stream-identification-per-port-counters */
    fill_xpath(path, STREAM_ID_PER_PORT_INPUT_PACKETS_XPATH, interface_name);
    snprintf(value, MAX_STR_LEN, "%u", sum_of_cnt_in);
    if (NULL == lyd_new_path(*parent, NULL, path, value, 0, 0)) {
        SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                    STREAM_ID_INPUT_PACKETS_STR, ERR_NOT_CREATED_ELEMENT_STR);
    }

    /* leaf output-packets, container stream-identification-per-port-counters */
    fill_xpath(path, STREAM_ID_PER_PORT_OUTPUT_PACKETS_XPATH, interface_name);
    snprintf(value, MAX_STR_LEN, "%u", sum_of_cnt_out);
    if (NULL == lyd_new_path(*parent, NULL, path, value, 0, 0)) {
        SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                    STREAM_ID_OUTPUT_PACKETS_STR, ERR_NOT_CREATED_ELEMENT_STR);
    }

    return SR_ERR_OK;
}
