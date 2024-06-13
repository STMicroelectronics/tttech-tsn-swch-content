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

#include "preemption.h"

/* global variable from ietf-interfaces.c */
extern int plugin_init;

struct lyd_node *new_frame_preemption_status_table(
        struct lyd_node *parent, const struct lys_module *module, char *if_name,
        int prio, int traffic_class)
{
    char tmp[MAX_STR_LEN] = "";
    struct lyd_node *preempt_status = NULL;
    struct port_hdl *port = NULL;
    preemp_stat preemp_status;

    /* new frame-preemption-status-table table */
    if (EXIT_FAILURE == new_container(parent, &preempt_status, module,
                                      PREEMPT_FRAME_STATUS_TABLE_STR)) {
        return NULL;
    }

    port = tsn_port_open_by_name(if_name);
    if (NULL == port) {
        SRP_LOG_ERR(ERROR_MSG_MOD_FUNC_EL_MSG, PREEMPT_MODULE_NAME_STR, __func__,
                    if_name, ERR_PORT_NOT_OPEN_STR);
        return NULL;
    }

    /* traffic-class, key leaf */
    snprintf(tmp, MAX_STR_LEN, "%d", traffic_class);
    if (EXIT_FAILURE == new_node(preempt_status, module,
                                 PREEMPT_TRAFFIC_CLASS_STR, tmp)) {
        tsn_port_close(port);
        return NULL;
    }

    /* frame-preemption-status */
    if (0 == tsn_preemp_get_status_table(port, prio, &preemp_status)) {
        if (preemp_status == UNKNOWN) {
            SRP_LOG_ERR(ERROR_MSG_MOD_FUNC_EL_MSG, PREEMPT_MODULE_NAME_STR, __func__,
                        PREEMPT_FRAME_STATUS_STR, ERR_NOT_CREATED_ELEMENT_STR);
            tsn_port_close(port);
            return NULL;
        }
        if (preemp_status == EXPRESS) {
            snprintf(tmp, MAX_STR_LEN, "%s", PREEMPT_STATUS_EXPRESS_STR);
        }
        if (preemp_status == PREEMPT) {
            snprintf(tmp, MAX_STR_LEN, "%s", PREEMPT_STATUS_PREEMPTABLE_STR);
        }
        if (EXIT_FAILURE == new_node(preempt_status, module,
                                     PREEMPT_FRAME_STATUS_STR, tmp)) {
            tsn_port_close(port);
            return NULL;
        }
    }
    tsn_port_close(port);

    return preempt_status;
}

struct lyd_node *new_frame_preemption_parameters_container(
        struct lyd_node *parent, struct ly_ctx *ctx, char *if_name)
{
    struct lyd_node *preempt_params = NULL;
    const struct lys_module *module = NULL;
    int prio = 0;
    int num_tc = 8;
    int i = 0;

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    module = ly_ctx_load_module(ctx, PREEMPT_MODULE_NAME_STR, NULL);

    if (NULL == module) {
        SRP_LOG_ERR(ERROR_MSG_FUN_AND_MSG, __func__, ERR_MSG_LOAD_MODULE_STR);
        return NULL;
    }
    /* new frame-preemption-parameters container */
    if (EXIT_FAILURE == new_container(parent, &preempt_params, module,
                                      PREEMPT_FRAME_PARAMS_STR)) {
        return NULL;
    }
    for (i = 0; i < num_tc; i++) {
        if (NULL == new_frame_preemption_status_table(preempt_params, module,
                                                      if_name, prio, i))
            return NULL;
    }

    return preempt_params;
}

int new_state_preemption_container(struct lyd_node **parent,
                                   char *interface_name)
{
    uint32_t tmp_int = 0u;
    char path[MAX_STR_LEN] = "";
    char tmp[MAX_STR_LEN] = "";
    char value[MAX_STR_LEN] = "";
    struct port_hdl *port = NULL;

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    port = tsn_port_open_by_name(interface_name);
    if (NULL == port) {
        SRP_LOG_ERR(ERROR_MSG_MOD_FUNC_EL_MSG, PREEMPT_MODULE_NAME_STR, __func__,
                    interface_name, ERR_PORT_NOT_OPEN_STR);
        return SR_ERR_OPERATION_FAILED;
    }

    /* leaf hold-advance */
    if (0 == tsn_preemp_get_hold_advance(port, &tmp_int)) {
        fill_xpath(path, INTERFACE_PREEMPTION_HOLD_ADVANCE_XPATH, interface_name);
        snprintf(value, MAX_STR_LEN, "%u", tmp_int);
        if (NULL == lyd_new_path(*parent, NULL, path, value, 0, 0)) {
            SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                        PREEMPT_HOLD_ADVANCE_STR, ERR_NOT_CREATED_ELEMENT_STR);
        }
    }

    /* leaf release-advance */
    if (0 == tsn_preemp_get_release_advance(port, &tmp_int)) {
        fill_xpath(path, INTERFACE_PREEMPTION_RELEASE_ADVANCE_XPATH, interface_name);
        snprintf(value, MAX_STR_LEN, "%u", tmp_int);
        if (NULL == lyd_new_path(*parent, NULL, path, value, 0, 0)) {
            SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                        PREEMPT_RELEASE_ADVANCE_STR, ERR_NOT_CREATED_ELEMENT_STR);
        }
    }

    /* leaf preemption-active */
    if (0 == tsn_preemp_get_preemption_active(port, &tmp_int)) {
        if (tmp_int == 1) {
            snprintf(tmp, MAX_STR_LEN, "%s", BASE_FALSE);
        } else if (tmp_int == 2) {
            snprintf(tmp, MAX_STR_LEN, "%s", BASE_TRUE);
        } else {
            SRP_LOG_ERR(ERROR_MSG_MOD_FUNC_EL_MSG, PREEMPT_MODULE_NAME_STR, __func__,
                        PREEMPT_ACTIVE_STR, ERR_NOT_CREATED_ELEMENT_STR);
            tsn_port_close(port);
            return SR_ERR_OPERATION_FAILED;
        }
        fill_xpath(path, INTERFACE_PREEMPTION_ACTIVE_XPATH, interface_name);
        if (NULL == lyd_new_path(*parent, NULL, path, tmp, 0, 0)) {
            SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                        PREEMPT_ACTIVE_STR, ERR_NOT_CREATED_ELEMENT_STR);
            tsn_port_close(port);
            return SR_ERR_OPERATION_FAILED;
        }
    }

    /* leaf hold-request */
    if (0 == tsn_preemp_get_hold_request(port, &tmp_int))
    {
        if (tmp_int == 1) {
            snprintf(tmp, MAX_STR_LEN, "%s", PREEMPT_HOLD_STR);
        } else if (tmp_int == 2) {
            snprintf(tmp, MAX_STR_LEN, "%s", PREEMPT_RELEASE_STR);
        } else {
            SRP_LOG_ERR(ERROR_MSG_MOD_FUNC_EL_MSG, PREEMPT_MODULE_NAME_STR, __func__,
                        PREEMPT_HOLD_REQUEST_STR, ERR_NOT_CREATED_ELEMENT_STR);
            tsn_port_close(port);
            return SR_ERR_OPERATION_FAILED;
        }
        fill_xpath(path, INTERFACE_PREEMPTION_HOLD_REQUEST_XPATH, interface_name);
        if (NULL == lyd_new_path(*parent, NULL, path, tmp, 0, 0)) {
            SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                        PREEMPT_HOLD_REQUEST_STR, ERR_NOT_CREATED_ELEMENT_STR);
            tsn_port_close(port);
            return SR_ERR_OPERATION_FAILED;
        }
    }

    tsn_port_close(port);

     return SR_ERR_OK;
}

/* CALLBACKS FOR PREEMPTION CONFIG AND STATE DATA */
int interface_frame_preemption_status_cb(sr_session_ctx_t *session,
                                         const char *module_name,
                                         const char *xpath, sr_event_t event,
                                         uint32_t request_id,
                                         void *private_data)
{
    (void)event;
    (void)module_name;
    (void)request_id;
    (void)private_data;

    int rc = SR_ERR_OK;
    tsn_return_value tsn_ret_val = E_TSN_LIB_NO_ERROR;
    sr_change_oper_t op = {0};
    sr_xpath_ctx_t st = {0};
    sr_change_iter_t *iter = NULL;
    sr_val_t *old_value = NULL;
    sr_val_t *new_value = NULL;
    sr_val_t *node = NULL;
    struct port_hdl *port = NULL;
    preemp_stat preemp_status = PREEMPT;
    char interface_name[MAX_STR_LEN] = "";
    char traffic_class[MAX_STR_LEN] = "";
    int prio = 0;

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    if ((SR_EV_DONE == event) || (0 == plugin_init)) {
        SRP_LOG_DBG(DEBUG_MSG_WITH_TWO_PARAM, DBG_APPLYING_CHANGES_MSG, __func__);
        return SR_ERR_OK;
    }

    rc = sr_get_changes_iter(session, xpath, &iter);
    if (SR_ERR_OK != rc) {
        return rc;
    }

    while (SR_ERR_OK == sr_get_change_next(session, iter, &op,
                                           &old_value, &new_value)) {
        node = (op == SR_OP_DELETED) ? old_value : new_value;

        /* get interface name from xpath */
        if (EXIT_SUCCESS != get_key_value(session, node->xpath, IF_INTERFACE_LIST,
                                          IF_NAME_STR, &st, interface_name)) {
            SRP_LOG_ERR("%s: %s (%s)", __func__,
                        ERR_MISSING_ELEMENT_STR, IF_NAME_STR);
            return SR_ERR_OPERATION_FAILED;
        }

        if ((op == SR_OP_MODIFIED) || (op == SR_OP_CREATED)) {
            /* find traffic class value */
            if (EXIT_SUCCESS != get_key_value(session, node->xpath,
                                              PREEMPT_FRAME_STATUS_TABLE_STR,
                                              PREEMPT_TRAFFIC_CLASS_STR, &st,
                                              traffic_class)) {
                SRP_LOG_ERR("%s: %s (%s)", __func__,
                            ERR_MISSING_ELEMENT_STR, PREEMPT_TRAFFIC_CLASS_STR);
                return SR_ERR_OPERATION_FAILED;
            }

            prio = strtol(traffic_class, NULL, 10);

            /* open port by name and check is feature supported on the port */
            port = tsn_port_open_by_name(interface_name);
            if (NULL == port) {
                sr_set_error(session, node->xpath, ERR_PORT_NOT_OPEN_XPATH_STR,
                             node->xpath);
                return SR_ERR_OPERATION_FAILED;
            }
            if (!tsn_is_feat_supported(port, FEAT_PREE)) {
                sr_set_error(session, node->xpath,
                             ERR_FEATURE_NOT_SUPPORTED_XPATH_STR,
                             "preemtp", node->xpath);
                tsn_port_close(port);
                return SR_ERR_OPERATION_FAILED;
            }
            /* check is element value 'express' or 'preemptable'.
             * Default value is 'preemptable'.
             */
            if (0 == strncmp(PREEMPT_STATUS_EXPRESS_STR, node->data.enum_val,
                             MAX_STR_LEN)) {
                preemp_status = EXPRESS;
            }
            if (E_TSN_LIB_NO_ERROR != (tsn_ret_val = tsn_preemp_set_status_table(port, prio, preemp_status))) {
                sr_set_error(session, node->xpath, ERR_SETTER_EXT_FMT_STR,
                             PREEMPT_FRAME_STATUS_TABLE_STR,
                             LIT_ENTRY_STR, LIT_XPATH_STR, node->xpath,
                             tsn_lib_get_known_ernno_msg(tsn_ret_val));
                tsn_port_close(port);
                return SR_ERR_OPERATION_FAILED;
            }
            tsn_port_close(port);
        }
        if (op == SR_OP_DELETED) {
            sr_set_error(session, node->xpath,
                         ERR_REM_OP_NOT_SUPPORTED_XPATH_STR,
                         node->xpath);
            return SR_ERR_OPERATION_FAILED;
        }
    }

    sr_free_val(old_value);
    sr_free_val(new_value);
    node = NULL;
    sr_free_change_iter(iter);

    return SR_ERR_OK;
}
