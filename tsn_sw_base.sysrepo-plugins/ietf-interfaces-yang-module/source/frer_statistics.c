/*
 * TTTech ietf-interfaces-yang-module
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

#include "frer_statistics.h"

int frame_replication_and_elimination_per_port_per_stream_counters(
        struct lyd_node **parent, char *interface_name)
{
    char path[MAX_STR_LEN] = "";
    char value[MAX_STR_LEN] = "";
    char bridge_names[MAX_NUM_OF_INTERFACES][MAX_STR_LEN] = {0};
    char ord_str[MAX_STR_LEN] = "";
    char handle[MAX_STR_LEN] = "";
    char direction[MAX_STR_LEN] = "";
    struct port_hdl *brhdl = NULL;
    struct port_hdl *porthdl = NULL;
    tsn_return_value tsn_ret = E_TSN_LIB_NO_ERROR;
    uint32_t num_of_ords = 0;
    uint16_t *ord_arr = NULL;
    struct ident_params sid_ident = {0};
    uint16_t port_map = 0;
    uint32_t port_id = 0;
    uint32_t cnt = 0;
    uint64_t port_passed_cnt = 0;
    uint64_t port_discarded_cnt = 0;
    uint32_t i = 0;

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    if (EXIT_FAILURE == get_bridge_names(bridge_names, MAX_NUM_OF_INTERFACES, MAX_STR_LEN)) {
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

    /* get interface id */
    if (UINT32_MAX == (port_id = tsn_get_id_by_name(brhdl, interface_name))) {
        SRP_LOG_ERR(ERR_MSG_EXT_FMT_STR,
                    LIT_GET_STR, LIT_ID_STR, LIT_VAL_STR, BASE_PORT_STR,
                    interface_name);
    }

    /* get number of stream-handles */
    if (0 != tsn_sid_get_current_cnt(brhdl, &num_of_ords)) {
        SRP_LOG_ERR(ERROR_MSG_FUN_AND_MSG, __func__,
                    ERR_FAILED_TO_GET_NUMBER_OF_BRIDGE_HANDLES_STR);
        return SR_ERR_OPERATION_FAILED;
    }

    ord_arr = malloc(sizeof(uint16_t) * (uint32_t)num_of_ords);
    if (NULL == ord_arr) {
        SRP_LOG_ERR(ERR_MEMORY_ALLOC_FAILED_STR);
    }

    if (0 == tsn_sid_get_entries_list(brhdl, num_of_ords, ord_arr)) {
        porthdl = tsn_port_open_by_name(interface_name);
        if (NULL == porthdl) {
            SRP_LOG_ERR(ERROR_MSG_MOD_FUNC_EL_MSG, IF_MODULE_NAME_STR, __func__,
                        interface_name, ERR_PORT_NOT_OPEN_STR);
            return SR_ERR_OPERATION_FAILED;
        } else {
            for (i = 0; i < num_of_ords; i++) {
                /* get ids of ports for which stream handle function is instantiated */
                if (E_TSN_LIB_NO_ERROR != (tsn_ret = tsn_sid_get_outfacin(brhdl, ord_arr[i], &port_map))) {
                    snprintf(ord_str, MAX_STR_LEN - 1, "%u", ord_arr[i]);
                    SRP_LOG_ERR(ERR_MSG_AND_EXPL_EXT_FMT_STR,
                                LIT_GET_STR, STREAM_ID_OUTFACIN_LIST_STR,
                                LIT_LIST_STR, STREAM_ID_INDEX_STR, ord_str,
                                tsn_lib_get_known_ernno_msg(tsn_ret));
                }

                /* update state data if the current stream handle function is instantiated onto this port */
                if (port_map & (((uint16_t)1) << port_id)) {
                    /* leaf direction-out-facing key of the list per-port-per-stream-counters */
                    snprintf(direction, MAX_STR_LEN, "%s", "true");

                    /* leaf handle key of the list per-port-per-stream-counters */
                    if (E_TSN_LIB_NO_ERROR != (tsn_ret = tsn_sid_get_ident_params(brhdl, ord_arr[i], &sid_ident))) {
                        snprintf(ord_str, MAX_STR_LEN - 1, "%u", ord_arr[i]);
                        SRP_LOG_ERR(ERR_MSG_AND_EXPL_EXT_FMT_STR,
                                    LIT_GET_STR, STREAM_ID_IDENT_STR,
                                    LIT_PARAMS_STR, STREAM_ID_INDEX_STR, ord_str,
                                    tsn_lib_get_known_ernno_msg(tsn_ret));
                    }
                    snprintf(handle, MAX_STR_LEN, "%u", sid_ident.str_hdl);

                    /* leaf rx-out-of-order-pkts */
                    if (!tsn_frer_seqrec_cnt_outoforder(porthdl, (unsigned int)sid_ident.str_hdl, &cnt)) {
                        fill_xpath(path, FRER_PER_PORT_PER_STREAM_XPATH,
                                   interface_name, direction, handle,
                                   FRER_REC_OUT_ORDER_PCKTS_STR);
                        snprintf(value, MAX_STR_LEN, "%u", cnt);
                        if (NULL == lyd_new_path(*parent, NULL, path, value, 0, 0)) {
                            SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                                        FRER_REC_OUT_ORDER_PCKTS_STR,
                                        ERR_NOT_CREATED_ELEMENT_STR);
                        }
                    }
                    /* leaf rx-rogue-pkts */
                    if (!tsn_frer_seqrec_cnt_rogue(porthdl, (unsigned int)sid_ident.str_hdl, &cnt)) {
                        fill_xpath(path, FRER_PER_PORT_PER_STREAM_XPATH,
                                   interface_name, direction, handle,
                                   FRER_REC_ROUGE_PCKTS_STR);
                        snprintf(value, MAX_STR_LEN, "%u", cnt);
                        if (NULL == lyd_new_path(*parent, NULL, path, value, 0, 0)) {
                            SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                                        FRER_REC_ROUGE_PCKTS_STR, ERR_NOT_CREATED_ELEMENT_STR);
                        }
                    }
                    /* leaf rx-passed-pkts */
                    if (!tsn_frer_seqrec_cnt_passed(porthdl, (unsigned int)sid_ident.str_hdl, &cnt)) {
                        fill_xpath(path, FRER_PER_PORT_PER_STREAM_XPATH,
                                   interface_name, direction, handle,
                                   FRER_REC_PASSED_PCKTS_STR);
                        snprintf(value, MAX_STR_LEN, "%u", cnt);
                        if (NULL == lyd_new_path(*parent, NULL, path, value, 0, 0)) {
                            SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                                        FRER_REC_PASSED_PCKTS_STR, ERR_NOT_CREATED_ELEMENT_STR);
                        }
                        port_passed_cnt += cnt;
                    }
                    /* leaf rx-discarded-pkts */
                    if (!tsn_frer_seqrec_cnt_discarded(porthdl, (unsigned int)sid_ident.str_hdl, &cnt)) {
                        fill_xpath(path, FRER_PER_PORT_PER_STREAM_XPATH,
                                   interface_name, direction, handle,
                                   FRER_REC_DISCARDED_PCKTS_STR);
                        snprintf(value, MAX_STR_LEN, "%u", cnt);
                        if (NULL == lyd_new_path(*parent, NULL, path, value, 0, 0)) {
                            SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                                        FRER_REC_DISCARDED_PCKTS_STR, ERR_NOT_CREATED_ELEMENT_STR);
                        }
                        port_discarded_cnt += cnt;
                    }
                }
            }
            /* free mem */
            tsn_port_close(porthdl);
            porthdl = NULL;
            free(ord_arr);
            ord_arr = NULL;
            tsn_port_close(brhdl);
            brhdl = NULL;
        }
    } else {
        SRP_LOG_ERR(ERROR_MSG_FUN_AND_MSG, __func__,
                    ERR_FAILED_TO_GET_BRIDGE_HANDLES_STR);
        tsn_port_close(brhdl);
        return SR_ERR_OPERATION_FAILED;
    }

    /* container per-port-counters, leaf "rx-passed-pkts" */
    fill_xpath(path, FRER_PER_PORT_XPATH, interface_name, FRER_REC_PASSED_PCKTS_STR);
    snprintf(value, MAX_STR_LEN, "%llu", port_passed_cnt);
    if (NULL == lyd_new_path(*parent, NULL, path, value, 0, 0)) {
        SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                    FRER_REC_PASSED_PCKTS_STR, ERR_NOT_CREATED_ELEMENT_STR);
    }

    /* container per-port-counters, leaf "rx-discarded-pkts" */
    fill_xpath(path, FRER_PER_PORT_XPATH, interface_name, FRER_REC_DISCARDED_PCKTS_STR);
    snprintf(value, MAX_STR_LEN, "%llu", port_discarded_cnt);
    if (NULL == lyd_new_path(*parent, NULL, path, value, 0, 0)) {
        SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                    FRER_REC_DISCARDED_PCKTS_STR, ERR_NOT_CREATED_ELEMENT_STR);
    }

    return SR_ERR_OK;
}
