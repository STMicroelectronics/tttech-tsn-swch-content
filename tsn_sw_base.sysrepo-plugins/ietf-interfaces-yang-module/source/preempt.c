/*
 * TTTech ietf-interfaces-yang-module
 * Copyright(c) 2018TTTech Computertechnik AG.
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

#include "preempt.h"

char mac_merge_map[PREE_MACMERGE_MAX_OBJ_NUMBER][MAX_STR_LEN] = {
    // map between object index and object name for mac-merge module
    [PREE_MACMERGE_MERGE_ENABLE_TX]     = PREE_MACMERGE_MERGE_ENABLE_TX_STR,
    [PREE_MACMERGE_VERIFY_DISABLE_TX]   = PREE_MACMERGE_VERIFY_DISABLE_TX_STR,
    [PREE_MACMERGE_VERIFY_TIME]         = PREE_MACMERGE_VERIFY_TIME_STR,
    [PREE_MACMERGE_FRAG_SIZE]           = PREE_MACMERGE_FRAG_SIZE_STR,
};

char mac_merge_enable_map[2][2][MAX_STR_LEN] = {
    // 2 pairs
    { PREE_MACMERGE_ENABLED_STR,    BASE_TRUE },
    { PREE_MACMERGE_DISABLED_STR,   BASE_FALSE },
};

#if 0
char enabled_disabled_map[2][2][MAX_STR_LEN] = {
    // 2 pairs
    { BASE_ONE_STR,     PREE_MACMERGE_ENABLED_STR },
    { BASE_ZERO_STR,    PREE_MACMERGE_DISABLED_STR },
};
#endif

char merge_support_map[2][2][MAX_STR_LEN] = {
    // 2 pairs
    { BASE_ONE_STR,     PREE_MACMERGE_SUPPPORTED_STR },
    { BASE_ZERO_STR,    PREE_MACMERGE_NOT_SUPPORTED_STR },
};

char verify_status_map[7][2][MAX_STR_LEN] = {
    // 6 pairs
    { BASE_ONE_STR,     PREE_MACMERGE_STATUS_UNKNOWN_STR },
    { BASE_TWO_STR,     PREE_MACMERGE_STATUS_INITIAL_STR },
    { BASE_THREE_STR,   PREE_MACMERGE_STATUS_VERIFYING_STR },
    { BASE_FOUR_STR,    PREE_MACMERGE_STATUS_SUCCEEDED_STR },
    { BASE_FIVE_STR,    PREE_MACMERGE_STATUS_FAILED_STR },
    { BASE_SIX_STR,     PREE_MACMERGE_STATUS_DISABLED_STR },
};

char status_tx_map[3][2][MAX_STR_LEN] = {
    // 3 pairs
    { BASE_ONE_STR, PREE_MACMERGE_STATUS_UNKNOWN_STR },
    { BASE_TWO_STR, PREE_MACMERGE_STATUS_INACTIVE_STR },
    { BASE_THREE_STR, PREE_MACMERGE_STATUS_ACTIVE_STR },
};

/* global variable from ietf-interfaces.c */
extern int plugin_init;

struct lyd_node *new_mac_merge_container(struct lyd_node *parent,
                                         struct ly_ctx *ctx, char *if_name)
{
    struct lyd_node *mac_merge = NULL;
    const struct lys_module *module = NULL;

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    module = ly_ctx_load_module(ctx, PREE_MACMERGE_MODULE_NAME_STR, NULL);

    if (NULL == module) {
        SRP_LOG_ERR(ERROR_MSG_FUN_AND_MSG, __func__, ERR_MSG_LOAD_MODULE_STR);
        return NULL;
    }
    /* new mac-merge container */
    if (EXIT_FAILURE == new_container(parent, &mac_merge, module,
                                      PREE_MACMERGE_MAC_MERGE_STR)) {
        return NULL;
    }

    if (NULL == new_admin_control(mac_merge, module, if_name)) {
        return NULL;
    }

    return mac_merge;
}

struct lyd_node *new_admin_control(struct lyd_node *parent,
                                   const struct lys_module *module,
                                   char *if_name)
{
    struct lyd_node *admin_control = NULL;
    char tmp_value[MAX_STR_LEN] = "";
    struct port_hdl *port = NULL;
    uint8_t tmp_int = 0u;
    bool tmp_bool;
    preemp_fragsz frag_size;

    SRP_LOG_DBG(DEBUG_MSG_FUN_CALLED_WITH_ONE_PARAM, __func__, if_name);

    /* new admin-control container */
    if (EXIT_FAILURE == new_container(parent, &admin_control, module,
                                      PREE_MACMERGE_ADMIN_CONTROL_STR)) {
        return NULL;
    }

    port = tsn_port_open_by_name(if_name);
    if (NULL == port) {
        SRP_LOG_ERR(ERROR_MSG_MOD_FUNC_EL_MSG, PREE_MACMERGE_MODULE_NAME_STR, __func__,
                    if_name, ERR_PORT_NOT_OPEN_STR);
        return NULL;
    }

    /* container admin-control, leaf merge-enable-tx */
    if (0 == tsn_preemp_get_tx_enabled(port, &tmp_bool)) {
        /* Translate string using map is removed here because of mac_merge_enable_map.
         * Because of that map "1" is mapped to "Enabled" and than to "true" again. */
        if (tmp_bool == 1) {
            snprintf(tmp_value, MAX_STR_LEN, "%s", PREE_MACMERGE_ENABLED_STR);
        } else if (tmp_bool == 0) {
            snprintf(tmp_value, MAX_STR_LEN, "%s", PREE_MACMERGE_DISABLED_STR);
        } else {
            tsn_port_close(port);
            return NULL;
        }

        if (EXIT_FAILURE == new_node(admin_control, module,
                                     PREE_MACMERGE_MERGE_ENABLE_TX_STR, tmp_value)) {
            tsn_port_close(port);
            return NULL;
        }
    }

    /* container admin-control, leaf verify-disable-tx */
    if (0 == tsn_preemp_get_verify_tx_disabled(port, &tmp_bool)) {
        /* Translate string using map is removed here because of mac_merge_enable_map.
         * Because of that map "1" is mapped to "Enabled" and than to "true" again. */
        if (tmp_bool == 1) {
            snprintf(tmp_value, MAX_STR_LEN, "%s", PREE_MACMERGE_ENABLED_STR);
        } else if (tmp_bool == 0) {
            snprintf(tmp_value, MAX_STR_LEN, "%s", PREE_MACMERGE_DISABLED_STR);
        } else {
            tsn_port_close(port);
            return NULL;
        }
        if (EXIT_FAILURE == new_node(admin_control, module,
                                     PREE_MACMERGE_VERIFY_DISABLE_TX_STR, tmp_value)) {
            tsn_port_close(port);
            return NULL;
        }
    }

    /* container admin-control, leaf verify-time */
    if (0 == tsn_preemp_get_verify_time(port, &tmp_int)) {
        snprintf(tmp_value, MAX_STR_LEN, "%u", tmp_int);
        if (EXIT_FAILURE == new_node(admin_control, module,
                                     PREE_MACMERGE_VERIFY_TIME_STR, tmp_value)) {
            tsn_port_close(port);
            return NULL;
        }
    }

    /* container admin-control, leaf frag-size */
    if (0 == tsn_preemp_get_frag_size(port, &frag_size)) {
        snprintf(tmp_value, MAX_STR_LEN, "%u", frag_size);
        if (EXIT_FAILURE == new_node(admin_control, module,
                                     PREE_MACMERGE_FRAG_SIZE_STR, tmp_value)) {
            tsn_port_close(port);
            return NULL;
        }
    }

    tsn_port_close(port);

    return admin_control;
}

int new_admin_status(struct lyd_node **parent, char *interface_name)
{
    char path[MAX_STR_LEN] = "";
    char tmp[MAX_STR_LEN] = "";
    struct port_hdl *port = NULL;
    bool tmp_bool;
    preemp_vrfy_stat verify_status;
    preemp_tx_stat status_tx;

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    port = tsn_port_open_by_name(interface_name);
    if (NULL == port) {
        SRP_LOG_ERR(ERROR_MSG_MOD_FUNC_EL_MSG, PREE_MACMERGE_MODULE_NAME_STR, __func__,
                    interface_name, ERR_PORT_NOT_OPEN_STR);
        return SR_ERR_OPERATION_FAILED;
    }

    /* leaf merge-support */
    if (0 == tsn_preemp_get_support(port, &tmp_bool)) {
        snprintf(tmp, MAX_STR_LEN, "%d", tmp_bool);
        if (EXIT_FAILURE == translate_single_string_using_map(tmp, merge_support_map)) {
            tsn_port_close(port);
            return SR_ERR_OPERATION_FAILED;
        }
        fill_xpath(path, INTERFACE_PREEMPT_MERGE_SUPPORT_XPATH, interface_name);
        if (NULL == lyd_new_path(*parent, NULL, path, tmp, 0, 0)) {
            SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                        PREE_MACMERGE_MERGE_SUPPORT_STR, ERR_NOT_CREATED_ELEMENT_STR);
        }
    }

    /* leaf verify-status */
    if (0 == tsn_preemp_get_verify_status(port, &verify_status)) {
        snprintf(tmp, MAX_STR_LEN, "%d", verify_status);
        if (EXIT_FAILURE == translate_single_string_using_map(tmp, verify_status_map)) {
            tsn_port_close(port);
            return SR_ERR_OPERATION_FAILED;
        }
        fill_xpath(path, INTERFACE_PREEMPT_VERIFY_STATUS_XPATH, interface_name);
        if (NULL == lyd_new_path(*parent, NULL, path, tmp, 0, 0)) {
            SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                        PREE_MACMERGE_VERIFY_STATUS_STR, ERR_NOT_CREATED_ELEMENT_STR);
        }
    }

    /* leaf status-tx */
    if (0 == tsn_preemp_get_status_tx(port, &status_tx)) {
        snprintf(tmp, MAX_STR_LEN, "%d", status_tx);
        if (EXIT_FAILURE == translate_single_string_using_map(tmp, status_tx_map)) {
            tsn_port_close(port);
            return SR_ERR_OPERATION_FAILED;
        }
        fill_xpath(path, INTERFACE_PREEMPT_STATUS_TX_XPATH, interface_name);
        if (NULL == lyd_new_path(*parent, NULL, path, tmp, 0, 0)) {
            SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                        PREE_MACMERGE_STATUS_TX_STR, ERR_NOT_CREATED_ELEMENT_STR);
        }
    }
    tsn_port_close(port);

    return SR_ERR_OK;
}

int new_statistics(struct lyd_node **parent, char *interface_name)
{
    char tmp[MAX_STR_LEN] = "";
    char path[MAX_STR_LEN] = "";
    struct port_hdl *port = NULL;
    uint32_t tmp_int = 0u;

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    port = tsn_port_open_by_name(interface_name);
    if (NULL == port) {
        SRP_LOG_ERR(ERROR_MSG_MOD_FUNC_EL_MSG, PREE_MACMERGE_MODULE_NAME_STR, __func__,
                    interface_name, ERR_PORT_NOT_OPEN_STR);
        return SR_ERR_OPERATION_FAILED;
    }

    /* leaf assembly-error-count */
    if (0 == tsn_preemp_get_asserr_cnt(port, &tmp_int)) {
        snprintf(tmp, MAX_STR_LEN, "%d", tmp_int);
        fill_xpath(path, INTERFACE_PREEMPT_ASSEMBLY_ERR_CNT_XPATH, interface_name);
        if (NULL == lyd_new_path(*parent, NULL, path, tmp, 0, 0)) {
            SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                        PREE_MACMERGE_ASSEMBLY_ERR_CNT_STR,
                        ERR_NOT_CREATED_ELEMENT_STR);
        }
    }

    /* leaf smd-error-count */
    if (0 == tsn_preemp_get_smderr_cnt(port, &tmp_int)) {
        snprintf(tmp, MAX_STR_LEN, "%d", tmp_int);
        fill_xpath(path, INTERFACE_PREEMPT_SMD_ERR_CNT_XPATH, interface_name);
        if (NULL == lyd_new_path(*parent, NULL, path, tmp, 0, 0)) {
            SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                        PREE_MACMERGE_SMD_ERR_CNT_STR, ERR_NOT_CREATED_ELEMENT_STR);
        }
    }

    /* leaf assembly-ok-count */
    if (0 == tsn_preemp_get_assok_cnt(port, &tmp_int)) {
        snprintf(tmp, MAX_STR_LEN, "%d", tmp_int);
        fill_xpath(path, INTERFACE_PREEMPT_ASSEMBLY_OK_CNT_XPATH, interface_name);
        if (NULL == lyd_new_path(*parent, NULL, path, tmp, 0, 0)) {
            SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                        PREE_MACMERGE_ASSEMBLY_OK_CNT_STR,
                        ERR_NOT_CREATED_ELEMENT_STR);
        }
    }

    /* leaf fragment-count-rx */
    if (0 == tsn_preemp_get_fragrx_cnt(port, &tmp_int)) {
        snprintf(tmp, MAX_STR_LEN, "%d", tmp_int);
        fill_xpath(path, INTERFACE_PREEMPT_FRAGMENT_CNT_RX_XPATH, interface_name);
        if (NULL == lyd_new_path(*parent, NULL, path, tmp, 0, 0)) {
            SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                        PREE_MACMERGE_FRAGMENT_CNT_RX_STR,
                        ERR_NOT_CREATED_ELEMENT_STR);
        }
    }

    /* leaf fragment-count-tx */
    if (0 == tsn_preemp_get_fragtx_cnt(port, &tmp_int)) {
        snprintf(tmp, MAX_STR_LEN, "%d", tmp_int);
        fill_xpath(path, INTERFACE_PREEMPT_FRAGMENT_CNT_TX_XPATH, interface_name);
        if (NULL == lyd_new_path(*parent, NULL, path, tmp, 0, 0)) {
            SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                        PREE_MACMERGE_FRAGMENT_CNT_TX_STR, ERR_NOT_CREATED_ELEMENT_STR);
        }
    }

    /* leaf hold-count */
    if (0 == tsn_preemp_get_hold_cnt(port, &tmp_int)) {
        snprintf(tmp, MAX_STR_LEN, "%d", tmp_int);
        fill_xpath(path, INTERFACE_PREEMPT_HOLD_CNT_XPATH, interface_name);
        if (NULL == lyd_new_path(*parent, NULL, path, tmp, 0, 0)) {
            SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                        PREE_MACMERGE_HOLD_CNT_STR, ERR_NOT_CREATED_ELEMENT_STR);
        }
    }

    tsn_port_close(port);

     return SR_ERR_OK;
}

int new_state_mac_merge_container(struct lyd_node **parent,
                                  char *interface_name)
{
    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    if (SR_ERR_OK != new_admin_status(parent, interface_name)) {
        SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                    PREE_MACMERGE_ADMIN_STATUS_STR, ERR_NOT_CREATED_ELEMENT_STR);
    }

    if (SR_ERR_OK != new_statistics(parent, interface_name)) {
        SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                    PREE_MACMERGE_STATISTICS_STR, ERR_NOT_CREATED_ELEMENT_STR);
    }

     return SR_ERR_OK;
}

/* CALLBACKS FOR PREEMPT CONFIG AND STATE DATA */
int interface_admin_control_cb(sr_session_ctx_t *session,
                               const char *module_name, const char *xpath,
                               sr_event_t event, uint32_t request_id,
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
    char tmp[MAX_STR_LEN] = "";
    char interface_name[MAX_STR_LEN] = "";
    char *node_name = NULL;
    struct port_hdl *port = NULL;
    int obj_index = 0;
    bool tmp_bool = false;

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
            SRP_LOG_ERR("%s: %s (%s)", __func__, ERR_MISSING_ELEMENT_STR, IF_NAME_STR);
            return SR_ERR_OPERATION_FAILED;
        }

        /* If user changed values of leaf then node->dflt is "0". If sysrepo
         * changes value of leaf, because it has default value, that node->dflt
         * will be "1". This condition is added because of booting, i.e.
         * applying startup configuration. Every leaf that has default value
         * will be triggered with ADD operation if he does not exist inside
         * startup configuration. Its node->dflt flag will be "1" then.
         */
        if (((op == SR_OP_MODIFIED) || (op == SR_OP_CREATED)) &&
            (0 == node->dflt)) {
            /* open port by name and check is feature supported on the port */
            port = tsn_port_open_by_name(interface_name);
            if (NULL == port) {
                sr_set_error(session, node->xpath,
                             ERR_PORT_NOT_OPEN_XPATH_STR,
                             node->xpath);
                return SR_ERR_OPERATION_FAILED;
            }
            if (!tsn_is_feat_supported(port, FEAT_MACMERGE)) {

                sr_set_error(session, node->xpath,
                             ERR_FEATURE_NOT_SUPPORTED_XPATH_STR,
                             "mac-merge", node->xpath);
                tsn_port_close(port);
                return SR_ERR_OPERATION_FAILED;
            }

            /* get node name */
            node_name = sr_xpath_node_name(xpath);
            sr_xpath_recover(&st);
            obj_index = find_obj_index_by_name(node_name, mac_merge_map,
                                               PREE_MACMERGE_MAX_OBJ_NUMBER);

            switch (obj_index) {
            case PREE_MACMERGE_MERGE_ENABLE_TX:
                snprintf(tmp, MAX_STR_LEN, "%s", node->data.enum_val);
                if (EXIT_FAILURE == translate_string_to_bool(tmp, mac_merge_enable_map, &tmp_bool)) {
                    return SR_ERR_OPERATION_FAILED;
                }
                if (E_TSN_LIB_NO_ERROR != (tsn_ret_val = tsn_preemp_set_tx_enable(port, tmp_bool))) {
                    sr_set_error(session, node->xpath,
                                 ERR_SETTER_EXT_FMT_STR,
                                 PREE_MACMERGE_MERGE_ENABLE_TX_STR,
                                 LIT_VAL_STR, LIT_XPATH_STR, node->xpath,
                                 tsn_lib_get_known_ernno_msg(tsn_ret_val));
                    tsn_port_close(port);
                    return SR_ERR_OPERATION_FAILED;
                }
                break;

            case PREE_MACMERGE_VERIFY_DISABLE_TX:
                snprintf(tmp, MAX_STR_LEN, "%s", node->data.enum_val);
                if (EXIT_FAILURE == translate_string_to_bool(tmp, mac_merge_enable_map, &tmp_bool)) {
                    return SR_ERR_OPERATION_FAILED;
                }
                if (E_TSN_LIB_NO_ERROR != (tsn_ret_val = tsn_preemp_set_verify_tx_disable(port, tmp_bool))) {
                    sr_set_error(session, node->xpath,
                                 ERR_SETTER_EXT_FMT_STR,
                                 PREE_MACMERGE_VERIFY_DISABLE_TX_STR,
                                 LIT_VAL_STR, LIT_XPATH_STR, node->xpath,
                                 tsn_lib_get_known_ernno_msg(tsn_ret_val));
                    tsn_port_close(port);
                    return SR_ERR_OPERATION_FAILED;
                }
                break;
            case PREE_MACMERGE_VERIFY_TIME:
                if (E_TSN_LIB_NO_ERROR != (tsn_ret_val = tsn_preemp_set_verify_time(port, node->data.uint16_val))) {
                    sr_set_error(session, node->xpath,
                                 ERR_SETTER_EXT_FMT_STR,
                                 PREE_MACMERGE_VERIFY_TIME_STR, LIT_VAL_STR,
                                 LIT_XPATH_STR, node->xpath,
                                 tsn_lib_get_known_ernno_msg(tsn_ret_val));
                    tsn_port_close(port);
                    return SR_ERR_OPERATION_FAILED;
                }
                break;
            case PREE_MACMERGE_FRAG_SIZE:
                if (E_TSN_LIB_NO_ERROR != (tsn_ret_val = tsn_preemp_set_frag_size(port, node->data.uint16_val))) {
                    sr_set_error(session, node->xpath,
                                 ERR_SETTER_EXT_FMT_STR,
                                 PREE_MACMERGE_FRAG_SIZE_STR, LIT_VAL_STR,
                                 LIT_XPATH_STR, node->xpath,
                                 tsn_lib_get_known_ernno_msg(tsn_ret_val));
                    tsn_port_close(port);
                    return SR_ERR_OPERATION_FAILED;
                }
                break;
            }
            tsn_port_close(port);
        }
        if (op == SR_OP_DELETED) {
            sr_set_error(session, node->xpath,
                         ERR_REM_OP_NOT_SUPPORTED_XPATH_STR, node->xpath);
            return SR_ERR_OPERATION_FAILED;
        }
    }

    sr_free_val(old_value);
    sr_free_val(new_value);
    node = NULL;
    sr_free_change_iter(iter);

    return rc;
}
