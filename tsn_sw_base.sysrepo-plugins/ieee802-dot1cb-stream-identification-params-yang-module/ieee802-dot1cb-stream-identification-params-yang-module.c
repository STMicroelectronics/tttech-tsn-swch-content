/*
 * TTTech ieee802-dot1cb-stream-identification-params-yang-module
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

/* module specific includes */
#include "stream_identification_params_defines.h"

/* structure holding all the subscriptions */
sr_subscription_ctx_t *subscription;

/* Variable to detect is callback triggered for the first time The startup
 * datastore is copied to the running before this plugin is initialized. So, at
 * first time plugin started we need just to copy data to startup and running.
 * It is not necessary to trigger setter function. "plugin_init" will be '0' if
 * plugin is not initialized. Set "plugin_init" to '1' after is initialized.
 * The will be after fill startup datastore function.
 */
int plugin_init = 0;

#define SID_MAX_PORT_CNT   (16u)

/**
 * @brief Callback to be called when operational data of module
 * ieee802-dot1cb-stream-identification-params is requested.
 *
 * Subscribe to it by sr_oper_get_items_subscribe call.
 *
 * @param[in]   session         Implicit session (do not stop) with information
 *                              about the event originator session IDs.
 * @param[out]  module_name     Name of the affected module.
 * @param[out]  path            Path identifying the subtree that is supposed
 *                              to be provided, same as the one used for the
 *                              subscription.
 * @param[in]   request_xpath   XPath as requested by a client. Can be NULL.
 * @param[in]   request_id      Request ID unique for the specific module_name.
 * @param[in]   parent          Pointer to an existing parent of the requested
 *                              nodes. Is NULL for top-level nodes. Caller is
 *                              supposed to append the requested nodes to this
 *                              data subtree and return either the original
 *                              parent or a top-level node.
 * @param[in]   private_data    Private context opaque to sysrepo, as passed to
 *                              sr_oper_get_items_subscribe call.
 * @return      Error code (SR_ERR_OK on success).
 */
static int sid_params_state_cb(sr_session_ctx_t *session,
                               const char *module_name, const char *path,
                               const char *request_xpath, uint32_t request_id,
                               struct lyd_node **parent, void *private_data)
{
    (void)session;
    (void)module_name;
    (void)request_xpath;
    (void)request_id;
    (void)private_data;
    (void)path;

    char bridge_names[MAX_NUM_OF_INTERFACES][MAX_STR_LEN] = {0};
    char tmp_value[MAX_STR_LEN] = "";
    struct port_hdl *br_hdl = NULL;
    uint32_t tmp_uint32 = 0;
    uint16_t port_mask = 0;
    unsigned i = 0;
    char *port_name = NULL;
    bool supported = false;
    sid_ident_type ident = 0;

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    if (EXIT_FAILURE == get_bridge_names(bridge_names, MAX_NUM_OF_INTERFACES,
                                         MAX_STR_LEN)) {
        return SR_ERR_OPERATION_FAILED;
    }

    br_hdl = tsn_port_open_by_name(bridge_names[0]);
    if (NULL == br_hdl) {
        SRP_LOG_ERR(ERROR_MSG_MOD_FUNC_EL_MSG, SID_PARAMS_MODULE_NAME_STR, __func__,
                    bridge_names[0], ERR_PORT_NOT_OPEN_STR);
        return SR_ERR_OPERATION_FAILED;
    }

    /* container stream-identification-params, leaf table-length */
    if (0 == tsn_sid_get_tab_length(br_hdl, &tmp_uint32)) {
        snprintf(tmp_value, MAX_STR_LEN, "%u", tmp_uint32);
        if (NULL == lyd_new_path(*parent, NULL, SID_PARAMS_TABLE_LENGTH_XPATH,
                                 tmp_value, 0, 0)) {
            SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                        SID_PARAMS_TABLE_LENGTH_XPATH, ERR_NOT_CREATED_ELEMENT_STR);
        }
    }

    /* container stream-identification-params, leaf max-supported-stream-handle */
    if (0 == tsn_sid_get_max_sup(br_hdl, &tmp_uint32)) {
        snprintf(tmp_value, MAX_STR_LEN, "%u", tmp_uint32);
        if (NULL == lyd_new_path(*parent, NULL, SID_PARAMS_MAX_SUP_STRHDL_XPATH,
                                 tmp_value, 0, 0)) {
            SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                        SID_PARAMS_MAX_SUP_STRHDL_XPATH, ERR_NOT_CREATED_ELEMENT_STR);
        }
    }

    /* container stream-identification-params/sid-methods */
    for (ident = SID_UNKNOWN + 1; ident < SID_IDENT_MAX; ident++) {
        if (0 == tsn_sid_get_ident_type(br_hdl, ident, &supported)) {
            snprintf(tmp_value, MAX_STR_LEN, "%s", bool_to_str(supported));
            switch (ident) {
            case SID_NULL:
                /* container stream-identification-params/sid-methods, leaf null-stream */
                if (NULL == lyd_new_path(*parent, NULL, SID_PARAMS_METHOD_NULL_XPATH,
                                         tmp_value, 0, 0)) {
                    SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                                SID_PARAMS_METHOD_NULL_XPATH, ERR_NOT_CREATED_ELEMENT_STR);
                }
                break;
            case SID_SOURCE:
                /* container stream-identification-params/sid-methods, leaf source-stream */
                if (NULL == lyd_new_path(*parent, NULL, SID_PARAMS_METHOD_SOURCE_XPATH,
                                         tmp_value, 0, 0)) {
                    SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                                SID_PARAMS_METHOD_SOURCE_XPATH, ERR_NOT_CREATED_ELEMENT_STR);
                }
                break;
            case SID_ACTIVE:
                /* container stream-identification-params/sid-methods, leaf active-stream */
                if (NULL == lyd_new_path(*parent, NULL, SID_PARAMS_METHOD_ACTIVE_XPATH,
                                         tmp_value, 0, 0)) {
                    SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                                SID_PARAMS_METHOD_ACTIVE_XPATH, ERR_NOT_CREATED_ELEMENT_STR);
                }
                break;
            case SID_IPSTR:
                /* container stream-identification-params/sid-methods, leaf ip-stream */
                if (NULL == lyd_new_path(*parent, NULL, SID_PARAMS_METHOD_IP_XPATH,
                                         tmp_value, 0, 0)) {
                    SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                                SID_PARAMS_METHOD_IP_XPATH, ERR_NOT_CREATED_ELEMENT_STR);
                }
                break;
            default:
                break;
            }
        }
    }

    /* container stream-identification-params, leaf enabled-ports */
    i = 0;
    if (0 == tsn_sid_get_supported_ports(br_hdl, &port_mask)) {
        for (i = 0; i < SID_MAX_PORT_CNT; i++) {
            if ((port_mask & (1 << i))) {
                port_name = tsn_get_name_by_id(br_hdl, i);
                if (NULL == lyd_new_path(*parent, NULL, SID_PARAMS_ENABLED_PORTS_XPATH,
                                         port_name, 0, 0)) {
                    SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                                SID_PARAMS_ENABLED_PORTS_XPATH, ERR_NOT_CREATED_ELEMENT_STR);
                }
            }
        }
    }

    tsn_port_close(br_hdl);

    return SR_ERR_OK;
}

/**
 * @brief Sysrepo plugin initialization callback.
 *
 * @param[in]   session         Sysrepo session that can be used for any API
 *                              calls needed for plugin initialization (mainly
 *                              for reading of startup configuration and
 *                              subscribing for notifications).
 * @param[in]   private_data    Private context as passed in sr_plugin_init_cb.
 * @return      Error code (SR_ERR_OK on success). If an error is returned,
 *              plugin will be considered as uninitialized.
 */
int sr_plugin_init_cb(sr_session_ctx_t *session, void **private_data)
{
    (void)private_data;

    int rc = SR_ERR_OK;

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    rc = sr_oper_get_items_subscribe(session, SID_PARAMS_MODULE_NAME_STR,
                                     "/ieee802-dot1cb-stream-identification-params:stream-identification-params/*",
                                     sid_params_state_cb, NULL, SR_SUBSCR_DEFAULT,
                                     &subscription);

    if (SR_ERR_OK != rc) {
        SRP_LOG_ERR(ERR_MODULE_INIT_FAILED_STR, SID_PARAMS_MODULE_NAME_STR,
                    sr_strerror(rc));
        sr_unsubscribe(subscription);
        return rc;
    }

    SRP_LOG_INF(INF_MODULE_INIT_SUCCESS_STR, SID_PARAMS_MODULE_NAME_STR);
    return rc;
}

/**
 * @brief Sysrepo plugin cleanup callback.
 *
 * @param[in]   session         Sysrepo session that can be used for any API
 *                              calls needed for plugin cleanup (mainly for
 *                              unsubscribing of subscriptions initialized in
 *                              sr_plugin_init_cb).
 * @param[in]   private_data    Private context as passed in sr_plugin_init_cb.
 */
void sr_plugin_cleanup_cb(sr_session_ctx_t *session, void *private_data)
{
    (void)private_data;
    (void)session;

    /* nothing to cleanup except freeing the subscriptions */
    sr_unsubscribe(subscription);
    SRP_LOG_INF(INF_MODULE_CLEANUP_STR, SID_PARAMS_MODULE_NAME_STR);
}
