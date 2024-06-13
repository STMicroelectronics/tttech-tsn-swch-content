/*
 * TTTech ieee802-dot1cb-stream-identification-yang-module
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
/* common includes */
#include "common_defines.h"
#include "common.h"
/* sysrepo includes */
#include <sysrepo.h>
#include <sysrepo/values.h>
#include <sysrepo/xpath.h>
/* libbase includes */
#include "libbase/base_defines.h"
#include "libbase/base_prints.h"
#include "libbase/base_private_translators.h"
#include "libbase/base_q_bridge_linux_tools.h"
/* libtsn includes */
#include "libtsn/tsn.h"
#include "libtsn/tsn_sid.h"
#include "libtsn/tsn_frer.h"
#include "libtsn/tsn_def.h"
/* module specific includes */
#include "stream_identification_defines.h"

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

static char cb_sid_type_map[4][2][MAX_STR_LEN] = {
    // 4 pairs
    { BASE_ONE_STR,      STREAM_ID_NULL_STREAM_STR },
    { BASE_TWO_STR,      STREAM_ID_SOURCE_MAC_STR },
    { BASE_THREE_STR,    STREAM_ID_ACTIVE_ID_STR },
    { BASE_FOUR_STR,     STREAM_ID_IP_IDENTIFICATION_STR }
};

static char stream_tagged_map[3][2][MAX_STR_LEN] = {
    // 3 pairs
    { BASE_ONE_STR,      STREAM_ID_STREAM_TAGGED_TAGGED_STR },
    { BASE_TWO_STR,      STREAM_ID_STREAM_TAGGED_PRIORITY_STR },
    { BASE_THREE_STR,    STREAM_ID_STREAM_TAGGED_ALL_STR }
};

static char stream_tagged_reverse_map[3][2][MAX_STR_LEN] = {
    // 3 pairs
    { STREAM_ID_STREAM_TAGGED_TAGGED_STR,        BASE_ONE_STR },
    { STREAM_ID_STREAM_TAGGED_PRIORITY_STR,      BASE_TWO_STR },
    { STREAM_ID_STREAM_TAGGED_ALL_STR,           BASE_THREE_STR }
};

#define TSN_ERRNO_ERROR_CODE(return_value)      ((return_value) & 0xFF)

static sid_pos sid_get_position(char *str)
{
    if (!strncmp("out-fac-in", str, MAX_STR_LEN))
        return OUT_FAC_IN;
    else if (!strncmp("in-fac-in", str, MAX_STR_LEN))
        return IN_FAC_IN;
    else if (!strncmp("out-fac-out", str, MAX_STR_LEN))
        return OUT_FAC_OUT;
    else if (!strncmp("in-fac-out", str, MAX_STR_LEN))
        return IN_FAC_OUT;

    SRP_LOG_ERR(ERR_STREAM_ID_POSITION_PARAMETER_WRING_STR , str);
    return EPERM;
}

/**
 * @brief Callback to be called by the event of changing any running datastore
 * content within the module.
 *
 * @param[in]   session         Implicit session (do not stop) with information
 *                              about the changed data (retrieved by
 *                              sr_get_changes_iter) the event originator
 *                              session IDs.
 * @param[in]   module_name     Name of the module where the change has
 *                              occurred.
 * @param[in]   xpath           XPath used when subscribing, NULL if the whole
 *                              module was subscribed to.
 * @param[in]   event           Type of the notification event that has
 *                              occurred.
 * @param[in]   request_id      Request ID unique for the specific module_name.
 *                              Connected events for one request (SR_EV_CHANGE
 *                              and SR_EV_DONE, for example) have the same
 *                              request ID.
 * @param[in]   private_data    Private context opaque to sysrepo, as passed to
 *                              sr_module_change_subscribe call.
 * @return      Error code (SR_ERR_OK on success).
 */
static int module_change_cb(sr_session_ctx_t *session, const char *module_name,
                            const char *xpath, sr_event_t event,
                            uint32_t request_id, void *private_data)
{
    (void)session;
    (void)module_name;
    (void)xpath;
    (void)event;
    (void)request_id;
    (void)private_data;

    return SR_ERR_OK;
#if 0
    sr_val_t *values = NULL;
    size_t count = 0;
    int rc = SR_ERR_OK;

    SRP_LOG_DBGMSG("Retrieve current configuration.\n");

    rc = sr_get_items(session, "/ieee802-dot1cb-stream-identification:ieee802-dot1cb-stream-identification//*",
                      0, &values, &count);
    if (SR_ERR_OK != rc) {
        SRP_LOG_ERR("Error by sr_get_items: %s", sr_strerror(rc));
        return rc;
    }
    for (size_t i = 0; i < count; i++){
        sr_print_val(&values[i]);
    }
    sr_free_values(values, count);
    return rc;
#endif
}

/**
 * @brief Creates a new out-facing container from
 * ieee802-dot1cb-stream-identification yang module.
 *
 * It is used for config data.
 *
 * @param[out]  parent          Pointer to struct lyd_node which is going to be
 *                              parent to created entry.
 * @param[in]   module          Main schema node structure representing YANG
 *                              module.
 * @param[in]   port_mask       Port mask (uint16_t).
 * @param[in]   pos             Enumeration value.
 * @return      struct lyd_node *or NULL if element not created.
 */
struct lyd_node *new_out_fac_container(struct lyd_node *parent,
                                       const struct lys_module *module,
                                       uint16_t port_mask, sid_pos pos)
{
    int  i = 0;
    char tmp[MAX_STR_LEN] = "";
    struct lyd_node *out_facing = NULL;

    SRP_LOG_DBG(DEBUG_MSG_FUN_CALLED, __func__);

    /* new out-facing container */
    if (EXIT_FAILURE == new_container(parent, &out_facing, module,
                                      STREAM_ID_OUT_FACING_STR)) {
        return NULL;
    }

    if (0 == port_mask) {
        SRP_LOG_INF(ERROR_MSG_FUN_AND_MSG, __func__,
                    ERR_STREAM_ID_UNKNOWN_POR_POSITION_LIST_STR);
    } else if (OUT_FAC_IN == pos) {
         for (i = 0; i < 16; i++) {
             if ((port_mask & (1 << i))) {
                 snprintf(tmp, MAX_STR_LEN, "sw0p%d", i);
                 if (EXIT_FAILURE == new_node(out_facing, module,
                                              STREAM_ID_INPUT_PORT_LIST_STR, tmp)) {
                     return NULL;
                 }
             }
         }
    }

    return out_facing;
}

/**
 * @brief Creates a new entry for the "stream-identity" list from
 * ieee802-dot1cb-stream-identification yang module.
 *
 * It is used for config data.
 *
 * @param[out]  parent          Pointer to struct lyd_node which is going to be
 *                              parent to created entry.
 * @param[in]   module          Main schema node structure representing YANG
 *                              module.
 * @param[in]   port            Poiner to struct port_hdl struct, that
 *                              represents opened port.
 * @param[in]   index           int.
 * @param[in]   str_hdl         uint16_t.
 * @return      struct lyd_node *or NULL if element not created.
 */
struct lyd_node *new_stream_identity_list(struct lyd_node *parent,
                                          const struct lys_module *module,
                                          struct port_hdl *port, uint32_t ord)
{
    struct lyd_node *parameters = NULL;
    struct ident_params ident;
    uint16_t port_mask;
    char tmp[MAX_STR_LEN] = "";
    char tmp_address[MAX_STR_LEN] = "";

    SRP_LOG_DBG(DEBUG_MSG_FUN_CALLED, __func__);

    /* list stream-identity, leaf 'index' */
    snprintf(tmp, MAX_STR_LEN, "%d", ord);
    if (EXIT_FAILURE == new_node(parent, module, STREAM_ID_INDEX_STR, tmp)) {
        return NULL;
    }

    if (0 == tsn_sid_get_ident_params(port, ord, &ident)) {
        /* list stream-identity, leaf 'handle' */
        snprintf(tmp, MAX_STR_LEN, "%u", ident.str_hdl);
        if (EXIT_FAILURE == new_node(parent, module, STREAM_ID_HANDLE_STR, tmp)) {
            return NULL;
        }

        if (0 == tsn_sid_get_outfacin(port, ord, &port_mask)) {
            snprintf(tmp, MAX_STR_LEN, "%d", ident.id_type);

            /* container stream-identity, leaf identification-type */
            if (EXIT_SUCCESS == translate_single_string_using_map(tmp, cb_sid_type_map)) {
                if (NULL == new_out_fac_container(parent, module, port_mask, OUT_FAC_IN)) {
                    return NULL;
                }
            }

            /* fill tmp_address */
            snprintf(tmp_address, MAX_STR_LEN, "%02x-%02x-%02x-%02x-%02x-%02x",
                     ident.addr[0], ident.addr[1], ident.addr[2],
                     ident.addr[3], ident.addr[4], ident.addr[5]);

            /* list stream-identity, choice parameters */

            /* Check if identification-type is NULL (null-stream) or SRC
             * (source-stream), and depending on it create
             * null-stream-identification or source-mac-identification
             * container.
             */
            switch (ident.id_type) {
            case SID_NULL:
                /* list stream-identity, container null-stream-identification */
                if (EXIT_FAILURE == new_container(parent, &parameters, module,
                                                  STREAM_ID_NULL_ST_ID_STR)) {
                    return NULL;
                }
                /* list stream-identity, container null-stream-identification,
                 * leaf destination-mac
                 */
                if (EXIT_FAILURE == new_node(parameters, module,
                                             STREAM_ID_DEST_MAC_STR, tmp_address)) {
                    return NULL;
                }
                snprintf(tmp, MAX_STR_LEN, "%d", ident.tag);

                if (EXIT_SUCCESS == translate_single_string_using_map(tmp, stream_tagged_map)) {
                    /* list stream-identity, container
                     * null-stream-identification, leaf tagged
                     */
                    if (EXIT_FAILURE == new_node(parameters, module,
                                                 STREAM_ID_TAGGED_STR, tmp)) {
                        return NULL;
                    }
                }
                snprintf(tmp, MAX_STR_LEN, "%d", ident.vid);
                /* list stream-identity, container null-stream-identification,
                 * leaf vlan
                 */
                if (EXIT_FAILURE == new_node(parameters, module,
                                             STREAM_ID_VLAN_STR, tmp)) {
                    return NULL;
                }
                break;

            case SID_SOURCE:
                /* list stream-identity, container source-mac-identification */
                if (EXIT_FAILURE == new_container(parent, &parameters, module,
                                                  STREAM_ID_SMAC_VLAN_ST_ID_STR)) {
                    return NULL;
                }
                /* list stream-identity, container null-stream-identification,
                 * leaf source-mac
                 */
                if (EXIT_FAILURE == new_node(parameters, module,
                                             STREAM_ID_SRC_MAC_STR, tmp_address)) {
                    return NULL;
                }
                snprintf(tmp, MAX_STR_LEN, "%d", ident.tag);

                if (EXIT_SUCCESS == translate_single_string_using_map(tmp, stream_tagged_map)) {
                    /* list stream-identity, container
                     * smac-stream-identification, leaf tagged
                     */
                    if (EXIT_FAILURE == new_node(parameters, module,
                                                 STREAM_ID_TAGGED_STR, tmp)) {
                        return NULL;
                    }
                }
                snprintf(tmp, MAX_STR_LEN, "%d", ident.vid);
                /* list stream-identity, container smac-stream-identification,
                 * leaf vlan
                 */
                if (EXIT_FAILURE == new_node(parameters, module,
                                             STREAM_ID_VLAN_STR, tmp)) {
                    return NULL;
                }
                break;
            case SID_UNKNOWN:
            case SID_ACTIVE:
            case SID_IPSTR:
            case SID_IDENT_MAX:
                SRP_LOG_ERR(ERROR_MSG_FUN_AND_MSG, __func__,
                            ERR_STREAM_ID_UNKNOWN_SID_TYPE_STR);
                break;
            }
        }
    }

    return parent;
}

/**
 * @brief Creates a new container stream-identification inside
 * ieee802-dot1cb-stream-identification yang module.
 *
 * This function is for configuration data from list component.
 *
 * @param[out]  parent          Pointer to struct lyd_node which is going to be
 *                              parent to created entry.
 * @param[in]   module          Main schema node structure representing YANG
 *                              module.
 * @param[in]   bridge_name     Contains the name of the bridge.
 * @return      struct lyd_node *or NULL if element not created.
 */
struct lyd_node *stream_identification(struct lyd_node *parent,
                                       const struct lys_module *module,
                                       char *bridge_name)
{
    struct port_hdl *port = NULL;
    uint32_t cnt = 0;
    uint32_t i = 0;
    uint16_t *ord = NULL;

    SRP_LOG_DBG(DEBUG_MSG_FUN_CALLED, __func__);

    port = tsn_port_open_by_name(bridge_name);
    if (NULL == port) {
        SRP_LOG_ERR(ERROR_MSG_MOD_FUNC_EL_MSG, STREAM_ID_MODULE_NAME_STR, __func__,
                    bridge_name, ERR_PORT_NOT_OPEN_STR);
        return NULL;
    }

    /* get number of stream handles and malloc memory for stream-handle list */
    if (0 == tsn_sid_get_current_cnt(port, &cnt)) {
        ord = malloc(sizeof(*ord) * (uint32_t)cnt);

        if (NULL != ord) {
            if (0 == tsn_sid_get_entries_list(port, cnt, ord)) {
                for (i = 0; i < cnt; i++) {
                    if (NULL == new_stream_identity_list(parent, module,
                                                         port, ord[i])) {
                        return NULL;
                    }
                }
            } else {
                SRP_LOG_ERR(ERROR_MSG_MOD_FUNC_EL_MSG, STREAM_ID_MODULE_NAME_STR, __func__,
                            STREAM_ID_STREAM_IDENT_LIST_STR, ERR_NOT_CREATED_ELEMENT_STR);
            }
            free(ord);
        } else {
            SRP_LOG_ERR(ERROR_MSG_FUN_AND_MSG, __func__,
                        ERR_STREAM_ID_STREAM_IDENT_LIST_ALLOCATE_STR);
        }
    }

    tsn_port_close(port);

    return parent;
}

/**
 * @brief Creates a startup datastore for ieee802-dot1cb-stream-identification
 * module. Function is going to be called once, due to sysrepo-plugind startup.
 *
 * @param[in]   session         Sysrepo session that can be used for any API
 *                              calls needed for plugin cleanup (mainly for
 *                              unsubscribing of subscriptions initialized in
 *                              sr_plugin_init_cb).
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
static int stream_identification_fill_startup_datastore(sr_session_ctx_t *session)
{
    const struct ly_ctx *ctx = NULL;
    const struct lys_module *module = NULL;
    struct lyd_node *first_entry = NULL;
    struct lyd_node **entries = NULL;
    char bridge_names[MAX_NUM_OF_INTERFACES][MAX_STR_LEN]= {0};
    int fd = -1;
    char *path = NULL;
    struct port_hdl *port = NULL;
    uint32_t cnt = 0;
    uint32_t i = 0;
    uint16_t *ord = NULL;

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    /* this means that startup is empty */
    if (0 == sr_path_check_startup_done_file(STREAM_ID_MODULE_NAME_STR, &path)) {
        ctx = sr_get_context(sr_session_get_connection(session));
        if (NULL == ctx) {
            SRP_LOG_ERR(ERROR_MSG_FUN_AND_MSG, __func__,
                        ERR_SESSION_CTX_FAILED_STR);
            free(path);
            return EXIT_FAILURE;
        }

        if (EXIT_FAILURE == get_bridge_names(bridge_names, MAX_NUM_OF_INTERFACES,
                                             MAX_STR_LEN)) {
            SRP_LOG_ERR(ERR_BRIDE_NAMES_FAILED_STR, __func__);
            return EXIT_SUCCESS;
        }

        port = tsn_port_open_by_name(bridge_names[0]);
        if (NULL == port) {
            SRP_LOG_ERR(ERROR_MSG_MOD_FUNC_EL_MSG, STREAM_ID_MODULE_NAME_STR, __func__,
                        bridge_names[0], ERR_PORT_NOT_OPEN_STR);
            return EXIT_FAILURE;
        }

        /* get number of stream handles and malloc memory for stream-handle list */
        if (0 == tsn_sid_get_current_cnt(port, &cnt)) {
            ord = malloc(sizeof(*ord) * (uint32_t)cnt);

            if (NULL != ord) {
                if (0 == tsn_sid_get_entries_list(port, cnt, ord)) {
                    /* This condition is here because entries can only be
                     * created if counter is different from zero, meaning that
                     * there is at least one stream-identity entry.
                     */
                    if (cnt > 0) {
                        /* create first entry of the stream-identity list */
                        first_entry = lyd_new_path(NULL, ctx, "/ieee802-dot1cb-stream-identification:stream-identity",
                                                   NULL, 0, 0);
                        if (NULL == first_entry) {
                            SRP_LOG_ERR(ERROR_MSG_FUN_AND_MSG, __func__,
                                        ERR_NOT_CREATED_ROOT_ELEMENT_STR);
                            free(path);
                            return EXIT_FAILURE;
                        }
                        /* fill up first entry of the stream-identity list */
                        if (NULL == new_stream_identity_list(first_entry, module, port, ord[0])) {
                            return EXIT_FAILURE;
                        }

                        /* if there is more than one entry */
                        if ((cnt-1) != 0) {
                            /* Allocate memory for other entries. Create array of entries. */
                            entries = malloc(sizeof(struct lyd_node *)*((int)cnt-1));

                            for (i = 1; i < cnt; i++) {
                                /* allocate each entry */
                                entries[i] = malloc(sizeof(struct lyd_node));
                                entries[i] = lyd_new_path(NULL, ctx, "/ieee802-dot1cb-stream-identification:stream-identity",
                                                          NULL, 0, 0);
                                if (NULL == entries[i]) {
                                    SRP_LOG_ERR(ERROR_MSG_FUN_AND_MSG, __func__,
                                                "Entry of list is not created.");
                                    free(path);
                                    return EXIT_FAILURE;
                                }

                                if (NULL == new_stream_identity_list(entries[i], module, port, ord[i])) {
                                    return EXIT_FAILURE;
                                }
                            }
                        }
                    }
                } else {
                    SRP_LOG_ERR(ERROR_MSG_MOD_FUNC_EL_MSG, STREAM_ID_MODULE_NAME_STR, __func__,
                                STREAM_ID_STREAM_IDENT_LIST_STR, ERR_NOT_CREATED_ELEMENT_STR);
                }
                free(ord);
            } else {
                SRP_LOG_ERR(ERROR_MSG_FUN_AND_MSG, __func__,
                            ERR_STREAM_ID_STREAM_IDENT_LIST_ALLOCATE_STR);
            }
        } else {
            SRP_LOG_WRN(ERROR_MSG_MOD_FUNC_MSG, STREAM_ID_MODULE_NAME_STR, __func__,
                        ERR_MSG_EMPTY_STARTUP_DS_STR);
            tsn_port_close(port);
            return EXIT_FAILURE;
        }

        tsn_port_close(port);

        /* attach all entries in one configuration */
        for (i = 1; i < cnt; i++) {
            lyd_insert_sibling(&first_entry, entries[i]);
        }

        /******** NOTE *******/
        /* The startup to running datastore copy is done before the plugin is
         * started. So, here we will replace the current startup and running
         * datastores with the subtree called 'root' in this function.
         */

        /* switch to running datastore */
        if (SR_ERR_OK != sr_session_switch_ds(session, SR_DS_RUNNING)) {
            SRP_LOG_ERR(ERROR_MSG_FUN_AND_MSG, __func__,
                        ERR_SWITCH_DATASTORE_FAILED_STR);
            free(path);
            return EXIT_FAILURE;
        }

        /* Replace current running configuration with created 'root' subtree */
        if (SR_ERR_OK != sr_replace_config(session, STREAM_ID_MODULE_NAME_STR,
                                           first_entry, 0, 0)) {
            SRP_LOG_ERR(ERROR_MSG_FUN_AND_MSG, __func__,
                        ERR_REPLACE_CONFIG_FAILED_STR);
            free(path);
            return EXIT_FAILURE;
        }

        /* switch back to startup datastore */
        if (SR_ERR_OK != sr_session_switch_ds(session, SR_DS_STARTUP)) {
            SRP_LOG_ERR(ERROR_MSG_FUN_AND_MSG, __func__,
                        ERR_SWITCH_DATASTORE_FAILED_STR);
            free(path);
            return EXIT_FAILURE;
        }

        /* copy config from running to startup datastore */
        if (SR_ERR_OK != sr_copy_config(session, STREAM_ID_MODULE_NAME_STR,
                                        SR_DS_RUNNING, 0, 0)) {
            SRP_LOG_ERR(ERROR_MSG_FUN_AND_MSG, __func__,
                        ERR_COPY_DATASTORE_FAILED_STR);
            free(path);
            return EXIT_FAILURE;
        }

        /* switch back to running datastore */
        if (SR_ERR_OK != sr_session_switch_ds(session, SR_DS_RUNNING)) {
            SRP_LOG_ERR(ERROR_MSG_FUN_AND_MSG, __func__,
                        ERR_SWITCH_DATASTORE_FAILED_STR);
            free(path);
            return EXIT_FAILURE;
        }

        fd = open(path, O_WRONLY | O_APPEND | O_CREAT, 0644);
        if (-1 == fd) {
            SRP_LOG_ERR("Unable to create file %s.", path);
            free(path);
            close(fd);
            return EXIT_FAILURE;
        }
        close(fd);
    }
    if (path) {
        free(path);
        path = NULL;
    }

    return EXIT_SUCCESS;
}

#if 0
/**
 * @brief Callback to be called by the event of changing content of leaf
 * identification-type inside list stream-identity.
 *
 * Subscribe to it by sr_module_change_subscribe call.
 *
 * @param[in]   session         Implicit session (do not stop) with information
 *                              about the changed data (retrieved by
 *                              sr_get_changes_iter) the event originator
 *                              session IDs.
 * @param[in]   module_name     Name of the module where the change has
 *                              occurred.
 * @param[in]   xpath           XPath used when subscribing, NULL if the whole
 *                              module was subscribed to.
 * @param[in]   event           Type of the notification event that has
 *                              occurred.
 * @param[in]   request_id      Request ID unique for the specific module_name.
 *                              Connected events for one request (SR_EV_CHANGE
 *                              and SR_EV_DONE, for example) have the same
 *                              request ID.
 * @param[in]   private_data   Private context opaque to sysrepo, as passed to
 *                              sr_module_change_subscribe call.
 * @return      Error code (SR_ERR_OK on success).
 */
int identication_type_cb(sr_session_ctx_t *session, const char *module_name,
                         const char *xpath, sr_event_t event,
                         uint32_t request_id, void *private_data)
{
    (void)module_name;
    (void)event;
    (void)request_id;
    (void)private_data;

    int rc;
    sr_change_oper_t op = {0};
    sr_change_iter_t *iter = NULL;
    sr_val_t *old_value = NULL;
    sr_val_t *new_value = NULL;
    sr_val_t *node = NULL;

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
        if (op == SR_OP_MODIFIED) {
            SRP_LOG_ERR(ERROR_MSG_FUN_XML_EL_AND_MSG, __func__,
                        sr_xpath_node_name(xpath), ERR_MOD_OP_NOT_SUPPORTED_STR);
            sr_set_error(session, node->xpath,
                         ERR_MOD_OP_NOT_SUPPORTED_XPATH_STR, node->xpath);
            return SR_ERR_OPERATION_FAILED;
        }
        /* SR_OP_CREATED operation is supported but only if object has
         * "null-stream-identification" value
         */
        /* Leaf has identity-ref type, his value will be
         * "ieee802-dot1cb-stream-identification:null-stream-identification"
         * because of that strstr function is used.
         */
        if (op == SR_OP_CREATED) {
            if (NULL == strstr(node->data.identityref_val, "null-stream")) {
                sr_set_error(session, node->xpath,
                             ERR_NULL_STREAM_SUPPORTED_STR,
                             "null-stream-identification", node->xpath);
                return SR_ERR_OPERATION_FAILED;
            }
        }
        /* SR_OP_DELETED operation is supported. When leaf is deleted there is
         * nothing to do.
         */
    }

    sr_free_val(old_value);
    sr_free_val(new_value);
    sr_free_change_iter(iter);
    node = NULL;

    return SR_ERR_OK;
}
#endif

/**
 * @brief Callback to be called by the event of changing content of list
 * stream-handles inside container stream-identification inside ieee802-dot1CB
 * module.
 *
 * Subscribe to it by sr_module_change_subscribe call.
 *
 * @param[in]   session         Implicit session (do not stop) with information
 *                              about the changed data (retrieved by
 *                              sr_get_changes_iter) the event originator
 *                              session IDs.
 * @param[in]   module_name     Name of the module where the change has
 *                              occurred.
 * @param[in]   xpath           XPath used when subscribing, NULL if the whole
 *                              module was subscribed to.
 * @param[in]   event           Type of the notification event that has
 *                              occurred.
 * @param[in]   request_id      Request ID unique for the specific module_name.
 *                              Connected events for one request (SR_EV_CHANGE
 *                              and SR_EV_DONE, for example) have the same
 *                              request ID.
 * @param[in]   private_data    Private context opaque to sysrepo, as passed to
 *                              sr_module_change_subscribe call.
 * @return      Error code (SR_ERR_OK on success).
 */
static int stream_identity_list_cb(sr_session_ctx_t *session,
                                   const char *module_name, const char *xpath,
                                   sr_event_t event, uint32_t request_id,
                                   void *private_data)
{
    (void)module_name;
    (void)request_id;
    (void)private_data;

    int rc = SR_ERR_OK;
    sr_change_oper_t op = {0};
    sr_change_iter_t *iter = NULL;
    sr_val_t *old_value = NULL;
    sr_val_t *new_value = NULL;
    sr_val_t *node = NULL;
    sr_val_t *tsn_stream_id_type = NULL;
    sr_val_t *mac_address = NULL;
    sr_val_t *tagged = NULL;
    sr_val_t *handle = NULL;
    sr_val_t *vlan = NULL;
    char path[2*MAX_STR_LEN] = "";
    char index_str[MAX_STR_LEN] = "";
    char tmp[LONG_STR_LEN] = "";
    char err_msg[MAX_STR_LEN] = "";
    char br_names[MAX_NUM_OF_INTERFACES][MAX_STR_LEN] = {0};
    struct port_hdl *port = NULL;
    uint16_t strhdl = 0;
    uint16_t index = 0;
    struct ident_params sid;
    uint32_t ret = 0u;

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    if ((SR_EV_DONE == event) || (0 == plugin_init)) {
        SRP_LOG_DBG(DEBUG_MSG_WITH_TWO_PARAM, DBG_APPLYING_CHANGES_MSG, __func__);
        return SR_ERR_OK;
    }

    /* Get bridge name */
    if (EXIT_FAILURE == get_bridge_names(br_names, MAX_NUM_OF_INTERFACES,
                                         MAX_STR_LEN)) {
        sr_set_error(session, xpath, ERR_BRIDGE_NAMES_STR);
        return SR_ERR_OPERATION_FAILED;
    }

    rc = sr_get_changes_iter(session, xpath, &iter);
    if (SR_ERR_OK != rc) {
        return rc;
    }

    while (SR_ERR_OK == sr_get_change_next(session, iter, &op,
                                           &old_value, &new_value)) {
        node = (op == SR_OP_DELETED) ? old_value : new_value;

        /* if stream-identity entry is added or deleted */
        if ((true == sr_xpath_node_name_eq(node->xpath, STREAM_ID_INDEX_STR)) &&
            ((SR_OP_CREATED == op) ||
             (SR_OP_DELETED == op))) {
            /* Get index value */
            index = node->data.uint16_val;
            /* Convert index val to str */
            snprintf(index_str, MAX_STR_LEN, "%u", index);

            /* open bridge */
            port = tsn_port_open_by_name(br_names[0]);
            if (NULL == port) {
                sr_set_error(session, node->xpath, ERR_PORT_NOT_OPEN_XPATH_STR,
                             br_names[0]);
                return SR_ERR_OPERATION_FAILED;
            }

            if (SR_OP_CREATED == op) {
                /* find handle value */
                fill_xpath(path, STREAM_ID_HANDLE_XPATH, index_str);
                if (SR_ERR_OK != sr_get_item(session, path, 0, &handle)) {
                    sr_set_error(session, path, ERR_FAILED_GET_OBJ_STR, path);
                    return SR_ERR_OPERATION_FAILED;
                }

                /* set handle value */
                strhdl = handle->data.uint32_val;
                sid.str_hdl = strhdl;
                sr_free_val(handle);

                /* find destination-mac */
                fill_xpath(path, STREAM_ID_DESTIONATION_MAC_XPATH, index_str);
                if (SR_ERR_OK != sr_get_item(session, path, 0, &mac_address)) {
                    sr_set_error(session, path, ERR_FAILED_GET_OBJ_STR, path);
                    return SR_ERR_OPERATION_FAILED;
                }

                /* If destination-mac is found at null-stream-identification
                 * container that means that stream identification type is
                 * null-stream. Also, is only supported stream identification
                 * type.
                 */
                sid.id_type = SID_NULL;

                /* find tagged */
                fill_xpath(path, STREAM_ID_NULL_STREAM_TAGGED_XPATH, index_str);
                if (SR_ERR_OK != sr_get_item(session, path, 0, &tagged)) {
                    sr_set_error(session, path, ERR_FAILED_GET_OBJ_STR, path);
                    return SR_ERR_OPERATION_FAILED;
                }

                strncpy(tmp, tagged->data.enum_val, (MAX_STR_LEN-1));
                if (EXIT_FAILURE == translate_single_string_using_map(tmp, stream_tagged_reverse_map)) {
                    return SR_ERR_OPERATION_FAILED;
                }

                sid.tag = strtoul(tmp, NULL, 10);

                /* find vlan */
                fill_xpath(path, STREAM_ID_NULL_STREAM_VLAN_XPATH, index_str);
                if (SR_ERR_OK != sr_get_item(session, path, 0, &vlan)) {
                    sr_set_error(session, path, ERR_FAILED_GET_OBJ_STR, path);
                    return SR_ERR_OPERATION_FAILED;
                }
                sid.vid = vlan->data.uint16_val;

                /* only null-stream is supported for now */
#if 0
                if (0 == strncmp(tsn_stream_id_type->data.enum_val,
                                 "smac-vlan-stream-identification",
                                 MAX_STR_LEN)) {
                    sid.id_type = SID_SOURCE;

                    /* find source-mac */
                    fill_xpath(path, STREAM_ID_SOURCE_MAC_XPATH, index, handle);
                    if (SR_ERR_OK != sr_get_item(session, path, 0, &mac_address)) {
                        sr_set_error(session, path, ERR_FAILED_GET_OBJ_STR, path);
                        return SR_ERR_OPERATION_FAILED;
                    }

                    /* find tagged */
                    fill_xpath(path, STREAM_ID_SMAC_TAGGED_XPATH, index, handle);
                    if (SR_ERR_OK != sr_get_item(session, path, 0, &tagged)) {
                        sr_set_error(session, path, ERR_FAILED_GET_OBJ_STR, path);
                        return SR_ERR_OPERATION_FAILED;
                    }

                    strncpy(tmp, tagged->data.enum_val, (MAX_STR_LEN-1));

                    if (EXIT_FAILURE == translate_single_string_using_map(tmp, stream_tagged_reverse_map)) {
                        return SR_ERR_OPERATION_FAILED;
                    }

                    sid.tag = strtoul(tmp, NULL, 10);

                    /* find vlan */
                    fill_xpath(path, STREAM_ID_SMAC_VLAN_XPATH, index, handle);
                    if (SR_ERR_OK != sr_get_item(session, path, 0, &vlan)) {
                        sr_set_error(session, path, ERR_FAILED_GET_OBJ_STR, path);
                        return SR_ERR_OPERATION_FAILED;
                    }

                    sid.vid = vlan->data.uint16_val;
                }
#endif
                sscanf(mac_address->data.string_val,
                       "%hhx-%hhx-%hhx-%hhx-%hhx-%hhx",
                       &sid.addr[0], &sid.addr[1], &sid.addr[2],
                       &sid.addr[3], &sid.addr[4], &sid.addr[5]);

                if (E_TSN_LIB_NO_ERROR != (ret = tsn_sid_add_ident(port, index, &sid))) {
                    if (0 == strncmp(ERR_NO_SPACE_LEFT_STR, tsn_lib_get_known_ernno_msg(ret),
                                     strlen(ERR_NO_SPACE_LEFT_STR))) {
                        snprintf(err_msg, MAX_STR_LEN, ERR_HASH_COLLISION_STR);
                    } else {
                        strncpy(err_msg, tsn_lib_get_known_ernno_msg(ret), MAX_STR_LEN-1);
                    }
                    sr_set_error(session, node->xpath,
                                 ERR_MSG_AND_EXPL_EXT_FMT_STR, LIT_CREATE_STR,
                                 STREAM_ID_STREAM_IDENT_LIST_STR, LIT_ENTRY_STR,
                                 LIT_XPATH_STR, node->xpath, err_msg);
                    tsn_port_close(port);
                    return SR_ERR_OPERATION_FAILED;
                }

                if (E_TSN_LIB_NO_ERROR != (ret = tsn_sid_set_strhdl(port, index, strhdl))) {
                    sr_set_error(session, node->xpath, ERR_SETTER_EXT_FMT_STR,
                                 STREAM_ID_STREAM_IDENT_LIST_STR,
                                 STREAM_ID_HANDLE_STR, LIT_XPATH_STR,
                                 node->xpath,
                                 tsn_lib_get_known_ernno_msg(ret));
                    tsn_port_close(port);
                    return SR_ERR_OPERATION_FAILED;
                }
            }
            if (SR_OP_DELETED == op) {
                if (E_TSN_LIB_NO_ERROR != (ret = tsn_sid_delete_entry(port, index))) {
                    sr_set_error(session, node->xpath,
                                 ERR_MSG_AND_EXPL_EXT_FMT_STR, LIT_DELETE_STR,
                                 STREAM_ID_STREAM_IDENT_LIST_STR, LIT_ENTRY_STR,
                                 LIT_XPATH_STR, node->xpath,
                                 tsn_lib_get_known_ernno_msg(ret));
                    tsn_port_close(port);
                    return SR_ERR_OPERATION_FAILED;
                }
            }
            tsn_port_close(port);
        }
    }

    sr_free_val(new_value);
    sr_free_val(old_value);
    sr_free_change_iter(iter);
    sr_free_val(tsn_stream_id_type);
    sr_free_val(mac_address);
    sr_free_val(tagged);
    sr_free_val(vlan);

    node = NULL;
    tsn_stream_id_type = NULL;
    mac_address = NULL;
    tagged = NULL;
    vlan = NULL;

    return SR_ERR_OK;
}

#if 0
/**
 * @brief This function checks if port with deleted input-port-list exists on
 * board inside stream-identity list.
 *
 * @param[in]   node            Pointer to sr_val_t structure representing
 *                              handle that is checked on board.
 * @return      Error code (EXIT_SUCCESS on success).
 */
int check_stream_handle_list(sr_val_t *node, const char *br_name)
{
    struct port_hdl *brdev = NULL;
    uint16_t cnt = 0;
    uint16_t *strhdl = NULL;
    uint16_t handle = 0;
    int  i = 0;

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    handle = node->data.uint32_val;

    brdev = tsn_port_open_by_name(br_name);
    if (NULL == brdev) {
        SRP_LOG_ERR(ERROR_MSG_MOD_FUNC_EL_MSG, STREAM_ID_MODULE_NAME_STR, __func__,
                    br_name, ERR_PORT_NOT_OPEN_STR);
        return EXIT_FAILURE;
    }

    /* list sequence-recovery-list */
    cnt = 0;
    tsn_sid_get_current_cnt(brdev, &cnt);
    if (0 == cnt) {
        SRP_LOG_INF(ERROR_MSG_FUN_AND_MSG, __func__,
                    ERR_STREAM_ID_STREAM_LIST_EMPTY_STR);
        tsn_port_close(brdev);
        return EXIT_FAILURE;
    }

    /* get number of stream handles and malloc memory for stream-handle list */
    if (0 != cnt) {
        strhdl = malloc(sizeof(*strhdl) * (uint32_t)cnt);
        if (NULL != strhdl) {
            if (0 == tsn_sid_get_entries_list(brdev, cnt, strhdl)) {
                for (i = 0; i < cnt; i++) {
                    /* check if entry with fnidx exist on board */
                    if (strhdl[i] == handle) {
                        tsn_port_close(brdev);
                        free(strhdl);
                        return EXIT_SUCCESS;
                    }
                }
            } else {
                SRP_LOG_ERR(ERROR_MSG_MOD_FUNC_EL_MSG, STREAM_ID_MODULE_NAME_STR, __func__,
                            STREAM_ID_STREAM_IDENT_LIST_STR, "failed to get list");
            }
            free(strhdl);
        } else {
            SRP_LOG_ERR(ERROR_MSG_FUN_AND_MSG, __func__,
                        ERR_STREAM_ID_STREAM_IDENT_LIST_ALLOCATE_STR);
        }
    }

    tsn_port_close(brdev);

    return EXIT_FAILURE;
}
#endif

/**
 * @brief Callback to be called by the event of changing content of leaf-list
 * out-facing-input-port-list inside list stream-handles.
 *
 * Subscribe to it by sr_module_change_subscribe call.
 *
 * @param[in]   session         Implicit session (do not stop) with information
 *                              about the changed data (retrieved by
 *                              sr_get_changes_iter) the event originator
 *                              session IDs.
 * @param[in]   module_name     Name of the module where the change has
 *                              occurred.
 * @param[in]   xpath           XPath used when subscribing, NULL if the whole
 *                              module was subscribed to.
 * @param[in]   event           Type of the notification event that has
 *                              occurred.
 * @param[in]   request_id      Request ID unique for the specific module_name.
 *                              Connected events for one request (SR_EV_CHANGE
 *                              and SR_EV_DONE, for example) have the same
 *                              request ID.
 * @param[in]   private_data    Private context opaque to sysrepo, as passed to
 *                              sr_module_change_subscribe call.
 * @return      Error code (SR_ERR_OK on success).
 */
static int out_facing_input_port_list_cb(sr_session_ctx_t *session,
                                         const char *module_name,
                                         const char *xpath, sr_event_t event,
                                         uint32_t request_id,
                                         void *private_data)
{
    (void)module_name;
    (void)request_id;
    (void)private_data;

    int rc = SR_ERR_OK;
    sr_change_oper_t op = {0};
    sr_change_iter_t *iter = NULL;
    sr_val_t *tmp_srval = NULL;
    sr_val_t *old_value = NULL;
    sr_val_t *new_value = NULL;
    sr_val_t *node = NULL;
    char *out_fac_in = "out-fac-in";
    char br_names[MAX_NUM_OF_INTERFACES][MAX_STR_LEN] = {0};
    char path[MAX_STR_LEN] = "";
    char index_str[MAX_STR_LEN] = "";
    sr_xpath_ctx_t st = {0};
    struct port_hdl *brhdl = NULL;
    struct port_pos sid_pos = {0};
    tsn_return_value tsn_ret_val = E_TSN_LIB_NO_ERROR;
    uint32_t ret = 0u;
    uint16_t port_map = 0;
    uint16_t index = 0;

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    if ((SR_EV_DONE == event) || (0 == plugin_init)) {
        SRP_LOG_DBG(DEBUG_MSG_WITH_TWO_PARAM, DBG_APPLYING_CHANGES_MSG, __func__);
        return SR_ERR_OK;
    }

    rc = sr_get_changes_iter(session, xpath, &iter);
    if (SR_ERR_OK != rc) {
        return rc;
    }

    /* Get bridge name */
    if (EXIT_FAILURE == get_bridge_names(br_names, MAX_NUM_OF_INTERFACES,
                                         MAX_STR_LEN)) {
        sr_set_error(session, xpath, ERR_BRIDGE_NAMES_STR);
        return SR_ERR_OPERATION_FAILED;
    }

    while (SR_ERR_OK == sr_get_change_next(session, iter, &op,
                                           &old_value, &new_value)) {
        node = (op == SR_OP_DELETED) ? old_value : new_value;

        /* if out-facing-input-port-list entry is added or deleted */
        if ((SR_OP_CREATED == op) || (SR_OP_DELETED == op)) {
            /* Get leaf index, key of the stream-identity list */
            if (EXIT_SUCCESS != get_key_value(session, node->xpath,
                                              STREAM_ID_STREAM_IDENT_LIST_STR,
                                              STREAM_ID_INDEX_STR, &st, index_str)) {
                return SR_ERR_OPERATION_FAILED;
            }
            index = (uint16_t)strtoul(index_str, NULL, 10);

            /* open bridge */
            brhdl = tsn_port_open_by_name(br_names[0]);
            if (NULL == brhdl) {
                sr_set_error(session, node->xpath,
                             ERR_PORT_NOT_OPEN_XPATH_STR, br_names[0]);
                return SR_ERR_OPERATION_FAILED;
            }

            if (SR_OP_CREATED == op) {
                sid_pos.port = tsn_get_id_by_name(brhdl, node->data.string_val);
                sid_pos.pos = sid_get_position(out_fac_in);

                if (sid_pos.pos >= POS_UNKNOWN)
                    ret = -EINVAL;
                else
                    ret = tsn_sid_add_port_pos(brhdl, index, &sid_pos);

                if (ret) {
                    sr_set_error(session, node->xpath,
                                 ERR_STREAM_ID_ENTRY_NOT_CREATED_STR, node->xpath);
                    tsn_port_close(brhdl);
                    return SR_ERR_OPERATION_FAILED;
                }
            }

            if (SR_OP_DELETED == op) {
                /* get handle value */
                fill_xpath(path, STREAM_ID_INDEX_XPATH, index_str);
                /* If stream handle is found inside configuration when
                 * operation is DELETE for input-port-list, that means that
                 * only leaf-list entry is deleted. If leaf handle is not found
                 * inside configuration when operation for input-port-list is
                 * delete, that means that entire stream-identity list entry is
                 * deleted, so there is nothing to do with input-port-list.
                 */

                /* NOTE: If stream-identity list entry is deleted, that means
                 * that sr_get_item function will not find leaf handle inside
                 * configuration, meanining we will get ERROR message on
                 * sysrepo log (ERR leaf handle not found...).
                 */
                if (SR_ERR_OK == sr_get_item(session, path, 0, &tmp_srval)) {
                    sr_free_val(tmp_srval);
                    tmp_srval = NULL;

                    sid_pos.port = tsn_get_id_by_name(brhdl, node->data.string_val);
                    sid_pos.pos = sid_get_position(out_fac_in);

                    tsn_sid_get_port_pos(brhdl, index, sid_pos.pos, &port_map);

                    if (E_TSN_LIB_NO_ERROR != (tsn_ret_val = tsn_sid_del_port_pos(brhdl, index, &sid_pos))) {
                        sr_set_error(session, node->xpath,
                                     ERR_SETTER_EXT_FMT_STR, STREAM_ID_STREAM_IDENT_LIST_STR,
                                     LIT_ENTRY_STR, LIT_XPATH_STR, node->xpath,
                                     tsn_lib_get_known_ernno_msg(tsn_ret_val));
                        tsn_port_close(brhdl);
                        return SR_ERR_OPERATION_FAILED;
                    }

                }
            }
            tsn_port_close(brhdl);
        }
    }

    sr_free_val(new_value);
    sr_free_val(old_value);
    sr_free_change_iter(iter);
    node = NULL;

    return SR_ERR_OK;
}

/**
 * @brief Callback to be called by the event of changing content of container
 * 'null-stream-identification' inside 'stream-identity' list. Used for
 * modification operation.
 *
 * Subscribe to it by sr_module_change_subscribe call.
 *
 * @param[in]   session         Implicit session (do not stop) with information
 *                              about the changed data (retrieved by
 *                              sr_get_changes_iter) the event originator
 *                              session IDs.
 * @param[in]   module_name     Name of the module where the change has
 *                              occurred.
 * @param[in]   xpath           XPath used when subscribing, NULL if the whole
 *                              module was subscribed to.
 * @param[in]   event           Type of the notification event that has
 *                              occurred.
 * @param[in]   request_id      Request ID unique for the specific module_name.
 *                              Connected events for one request (SR_EV_CHANGE
 *                              and SR_EV_DONE, for example) have the same
 *                              request ID.
 * @param[in]   private_data    Private context opaque to sysrepo, as passed to
 *                              sr_module_change_subscribe call.
 * @return      Error code (SR_ERR_OK on success).
 */
static int mod_null_stream_id_cb(sr_session_ctx_t *session,
                                 const char *module_name, const char *xpath,
                                 sr_event_t event, uint32_t request_id,
                                 void *private_data)
{
    (void)module_name;
    (void)request_id;
    (void)private_data;

    sr_xpath_ctx_t st = {0};
    int sr_ret = SR_ERR_OK;
    tsn_return_value tsn_ret = E_TSN_LIB_NO_ERROR;
    sr_change_iter_t *iter = NULL;
    sr_change_oper_t op = {0};
    sr_val_t *old_value = NULL;
    sr_val_t *new_value = NULL;
    sr_val_t *node = NULL;
    sr_val_t *tmp_srval = NULL;
    char br_names[MAX_NUM_OF_INTERFACES][MAX_STR_LEN] = {0};
    char index_str[MAX_STR_LEN] = "";
    struct ident_params sid_ident = {0};
    struct port_pos sid_port = {0};
    struct port_hdl *br_hdl = NULL;
    uint16_t index = 0;
    uint16_t port_map = 0;
    uint16_t port_msk = 0;
    uint16_t i = 0;
    struct ident_params ident;

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    if ((SR_EV_DONE == event) || (0 == plugin_init)) {
        SRP_LOG_DBG(DEBUG_MSG_WITH_TWO_PARAM, DBG_APPLYING_CHANGES_MSG, __func__);
        return SR_ERR_OK;
    }

    /* Get changes iter */
    sr_ret = sr_get_changes_iter(session, xpath, &iter);
    if (SR_ERR_OK != sr_ret) {
        return SR_ERR_OPERATION_FAILED;
    }

    /* Get bridge name */
    if (EXIT_FAILURE == get_bridge_names(br_names, MAX_NUM_OF_INTERFACES, MAX_STR_LEN)) {
        sr_set_error(session, xpath, ERR_BRIDGE_NAMES_STR);
        return SR_ERR_OPERATION_FAILED;
    }

    /* Get bridge handle */
    if (NULL == (br_hdl = tsn_port_open_by_name(br_names[0]))) {
        sr_set_error(session, xpath, ERR_PORT_NOT_OPEN_XPATH_STR, br_names[0]);
        sr_ret = SR_ERR_OPERATION_FAILED;
        goto free_mem;
    }

    while (SR_ERR_OK == sr_get_change_next(session, iter, &op, &old_value, &new_value)) {
        node = (SR_OP_DELETED == op) ? old_value : new_value;

        /* Get leaf index value */
        if (EXIT_SUCCESS != get_key_value(session, node->xpath,
                                          STREAM_ID_STREAM_IDENT_LIST_STR,
                                          STREAM_ID_INDEX_STR, &st, index_str)) {
            sr_set_error(session, node->xpath, ERR_KEY_VALUE_FAILED_XPATH_STR,
                         node->xpath);
            sr_ret = SR_ERR_OPERATION_FAILED;
            goto free_mem;
        }
        index = (uint16_t)strtoul(index_str, NULL, 10);

        if (SR_OP_MODIFIED == op) {
            /* Get current SID ident params */
            if (E_TSN_LIB_NO_ERROR != (tsn_ret = tsn_sid_get_ident_params(br_hdl, index, &sid_ident))) {
                sr_set_error(session, node->xpath, ERR_MSG_AND_EXPL_EXT_FMT_STR,
                             LIT_GET_STR, STREAM_ID_NULL_ST_ID_STR, LIT_PARAMS_STR,
                             STREAM_ID_INDEX_STR, index_str,
                             tsn_lib_get_known_ernno_msg(tsn_ret));
                sr_ret = SR_ERR_OPERATION_FAILED;
                goto free_mem;
            }

            /*
             * Apply new changes
             */
            if (NULL != strstr(node->xpath, STREAM_ID_DEST_MAC_STR)) {
                // if destination-mac is changed
                if (SR_ERR_OK != sr_get_item(session, node->xpath, 0, &tmp_srval)) {
                    sr_set_error(session, node->xpath, ERR_FAILED_GET_OBJ_STR,
                                 node->xpath);
                    sr_ret = SR_ERR_OPERATION_FAILED;
                    goto free_mem;
                }

                sscanf(tmp_srval->data.string_val,
                       "%hhx-%hhx-%hhx-%hhx-%hhx-%hhx",
                       &sid_ident.addr[0], &sid_ident.addr[1],
                       &sid_ident.addr[2], &sid_ident.addr[3],
                       &sid_ident.addr[4], &sid_ident.addr[5]);

                sr_free_val(tmp_srval);
                tmp_srval = NULL;
            } else if (NULL != strstr(node->xpath, STREAM_ID_TAGGED_STR)) {
                // currently tagged leaf value change isn't supported
                sr_set_error(session, node->xpath, ERR_MSG_AND_EXPL_EXT_FMT_STR,
                             LIT_SET_STR, STREAM_ID_TAGGED_STR, LIT_VAL_STR,
                             LIT_XPATH_STR, node->xpath,
                             ERR_MOD_OP_NOT_SUPPORTED_STR);
                sr_ret = SR_ERR_OPERATION_FAILED;
                goto free_mem;
            } else if (NULL != strstr(node->xpath, STREAM_ID_VLAN_STR)) {
                // if vlan is changed
                if (SR_ERR_OK != sr_get_item(session, node->xpath, 0, &tmp_srval)) {
                    sr_set_error(session, node->xpath, ERR_FAILED_GET_OBJ_STR,
                                 node->xpath);
                    sr_ret = SR_ERR_OPERATION_FAILED;
                    goto free_mem;
                }

                sid_ident.vid = tmp_srval->data.uint16_val;

                sr_free_val(tmp_srval);
                tmp_srval = NULL;
            } else {
                    sr_set_error(session, node->xpath,
                                 ERR_FAILED_TO_GET_ITEM_XPATH_STR, node->xpath);
                    sr_ret = SR_ERR_OPERATION_FAILED;
                    goto free_mem;
            }

            // get out-fac-in port list
            if (E_TSN_LIB_NO_ERROR != (tsn_ret = tsn_sid_get_outfacin(br_hdl, index, &port_map))) {
                sr_set_error(session, node->xpath, ERR_MSG_AND_EXPL_EXT_FMT_STR,
                             LIT_GET_STR, STREAM_ID_OUTFACIN_LIST_STR,
                             LIT_LIST_STR, STREAM_ID_INDEX_STR, index_str,
                             tsn_lib_get_known_ernno_msg(tsn_ret));
                sr_ret = SR_ERR_OPERATION_FAILED;
                goto free_mem;
            }

            // delete existing SID entry with given handle value
            if (E_TSN_LIB_NO_ERROR != (tsn_ret = tsn_sid_delete_entry(br_hdl, index))) {
                sr_set_error(session, node->xpath, ERR_MSG_AND_EXPL_EXT_FMT_STR,
                             LIT_DELETE_STR, STREAM_ID_STREAM_IDENT_LIST_STR,
                             LIT_ENTRY_STR, STREAM_ID_INDEX_STR, index_str,
                             tsn_lib_get_known_ernno_msg(tsn_ret));
                sr_ret = SR_ERR_OPERATION_FAILED;
                goto free_mem;
            }

            // add new SID entry with updated value
            if (E_TSN_LIB_NO_ERROR != (tsn_ret = tsn_sid_add_ident(br_hdl, index, &sid_ident))) {
                sr_set_error(session, node->xpath, ERR_MSG_AND_EXPL_EXT_FMT_STR,
                             LIT_CREATE_STR, STREAM_ID_STREAM_IDENT_LIST_STR,
                             LIT_ENTRY_STR, STREAM_ID_INDEX_STR, index_str,
                             tsn_lib_get_known_ernno_msg(tsn_ret));
                sr_ret = SR_ERR_OPERATION_FAILED;
                goto free_mem;
            }

            // set handle value
            if (E_TSN_LIB_NO_ERROR != (tsn_ret = tsn_sid_set_strhdl(br_hdl, index, sid_ident.str_hdl))) {
                sr_set_error(session, node->xpath, ERR_SETTER_EXT_FMT_STR,
                             STREAM_ID_STREAM_IDENT_LIST_STR,
                             STREAM_ID_HANDLE_STR, LIT_XPATH_STR, node->xpath,
                             tsn_lib_get_known_ernno_msg(tsn_ret));
                sr_ret = SR_ERR_OPERATION_FAILED;
                goto free_mem;
            }

            // set port list
            sid_port.pos = OUT_FAC_IN;

            port_msk = 1;
            for (i = 0; i < sizeof(uint16_t) * CHAR_BIT; i++) {
                if (port_map & (port_msk << i)) {
                    sid_port.port = i;

                    if (E_TSN_LIB_NO_ERROR != (tsn_ret = tsn_sid_add_port_pos(br_hdl, index, &sid_port))) {
                        sr_set_error(session, node->xpath,
                                     ERR_MSG_AND_EXPL_EXT_FMT_STR, LIT_SET_STR,
                                     STREAM_ID_OUTFACIN_LIST_STR, LIT_LIST_STR,
                                     STREAM_ID_INDEX_STR, index_str,
                                     tsn_lib_get_known_ernno_msg(tsn_ret));
                        sr_ret = SR_ERR_OPERATION_FAILED;
                        goto free_mem;
                    }
                }
            }
        }
        /* event abort, operation deleted. Used for delete the entries added to
         * the devices before configuration fails. So, if config fails at some
         * point, this part of the code will delete all enties added by this
         * config previously.
         */
        if (SR_EV_ABORT==event && SR_OP_DELETED==op) {
            /* get current entry, if exist delete that enry */
            if (0 == tsn_sid_get_ident_params(br_hdl, index, &ident)) {
                // delete existing SID entry with given handle value
                if (E_TSN_LIB_NO_ERROR != (tsn_ret = tsn_sid_delete_entry(br_hdl, index))) {
                    sr_set_error(session, node->xpath,
                                 ERR_MSG_AND_EXPL_EXT_FMT_STR, LIT_DELETE_STR,
                                 STREAM_ID_STREAM_IDENT_LIST_STR, LIT_ENTRY_STR,
                                 STREAM_ID_INDEX_STR, index_str,
                                 tsn_lib_get_known_ernno_msg(tsn_ret));
                    sr_ret = SR_ERR_OPERATION_FAILED;
                    goto free_mem;
                }
            }
        }
    }

free_mem:
    if (NULL != br_hdl) {
        tsn_port_close(br_hdl);
        br_hdl = NULL;
    }
    if (NULL != iter) {
        sr_free_change_iter(iter);
        iter = NULL;
    }

    return sr_ret;
}

/**
 * @brief Callback to be called by the event of changing content of container
 * 'null-stream-identification' inside 'stream-identity' list. Used for
 * deletion operation.
 *
 * Subscribe to it by sr_module_change_subscribe call.
 *
 * @param[in]   session         Implicit session (do not stop) with information
 *                              about the changed data (retrieved by
 *                              sr_get_changes_iter) the event originator
 *                              session IDs.
 * @param[in]   module_name     Name of the module where the change has
 *                              occurred.
 * @param[in]   xpath           XPath used when subscribing, NULL if the whole
 *                              module was subscribed to.
 * @param[in]   event           Type of the notification event that has
 *                              occurred.
 * @param[in]   request_id      Request ID unique for the specific module_name.
 *                              Connected events for one request (SR_EV_CHANGE
 *                              and SR_EV_DONE, for example) have the same
 *                              request ID.
 * @param[in]   private_data    Private context opaque to sysrepo, as passed to
 *                              sr_module_change_subscribe call.
 * @return      Error code (SR_ERR_OK on success).
 */
static int del_null_stream_id_cb(sr_session_ctx_t *session,
                                 const char *module_name, const char *xpath,
                                 sr_event_t event, uint32_t request_id,
                                 void *private_data)
{
    (void)module_name;
    (void)request_id;
    (void)private_data;

    sr_xpath_ctx_t st = {0};
    int sr_ret = SR_ERR_OK;
    sr_change_iter_t *iter = NULL;
    sr_change_oper_t op = {0};
    sr_val_t *old_value = NULL;
    sr_val_t *new_value = NULL;
    sr_val_t *node = NULL;
    sr_val_t *tmp_srval = NULL;
    char path[MAX_STR_LEN] = "";
    char index[MAX_STR_LEN] = "";
    char *node_name = NULL;

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    if ((SR_EV_DONE == event) || (0 == plugin_init)) {
        SRP_LOG_DBG(DEBUG_MSG_WITH_TWO_PARAM, DBG_APPLYING_CHANGES_MSG, __func__);
        return SR_ERR_OK;
    }

    /* Get changes iter */
    sr_ret = sr_get_changes_iter(session, xpath, &iter);
    if (SR_ERR_OK != sr_ret) {
        return SR_ERR_OPERATION_FAILED;
    }

    while (SR_ERR_OK == sr_get_change_next(session, iter, &op,
                                           &old_value, &new_value)) {
        node = (SR_OP_DELETED == op) ? old_value : new_value;

        if (SR_OP_DELETED == op) {
            /* Get leaf index value */
            if (EXIT_SUCCESS != get_key_value(session, node->xpath,
                                              STREAM_ID_STREAM_IDENT_LIST_STR,
                                              STREAM_ID_INDEX_STR, &st, index)) {
                sr_set_error(session, node->xpath,
                             ERR_KEY_VALUE_FAILED_XPATH_STR, node->xpath);
                sr_ret = SR_ERR_OPERATION_FAILED;
                goto free_mem;
            }

            fill_xpath(path, STREAM_ID_INDEX_XPATH, index);
            if (SR_ERR_OK == sr_get_item(session, path, 0, &tmp_srval)) {
                sr_free_val(tmp_srval);

                if (NULL != (node_name = sr_xpath_node_name(node->xpath))) {
                    sr_set_error(session, node->xpath,
                                 ERR_MSG_AND_EXPL_EXT_FMT_STR, LIT_DELETE_STR,
                                 node_name, LIT_LEAF_STR, LIT_XPATH_STR,
                                 node->xpath, ERR_DEL_OP_NOT_SUPPORTED_STR);
                } else {
                    SRP_LOG_ERR(ERR_FUNCTION_FAILED_STR, "sr_xpath_node_name");
                }

                sr_ret = SR_ERR_OPERATION_FAILED;
                goto free_mem;
            }
        }
    }

free_mem:
    if (NULL != iter) {
        sr_free_change_iter(iter);
        iter = NULL;
    }

    return sr_ret;
}

/**
 * @brief Helper function for 'chk_handles_cb' callback. Used to check if
 * handle value is in the allowed range.
 *
 * @param[in]   session         Implicit session (do not stop) with information
 *                              about the changed data (retrieved by
 *                              sr_get_changes_iter) the event originator
 *                              session IDs.
 * @param[in]   xpath           XPath used when subscribing, NULL if the whole
 *                              module was subscribed to.
 * @param[in]   max_handle      Maximum allowed handle value.
 * @return      Error code (SR_ERR_OK on success).
 */
static int chk_handle_value(sr_session_ctx_t *session, const char *xpath,
                            uint32_t max_handle)
{
    int sr_ret = SR_ERR_OK;
    sr_change_iter_t *iter = NULL;
    sr_change_oper_t op = {0};
    sr_val_t *old_value = NULL;
    sr_val_t *new_value = NULL;
    sr_val_t *node = NULL;
    uint32_t handle = 0;

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    /* Get changes iter */
    sr_ret = sr_get_changes_iter(session, xpath, &iter);
    if (SR_ERR_OK != sr_ret) {
        return SR_ERR_OPERATION_FAILED;
    }

    while (SR_ERR_OK == sr_get_change_next(session, iter, &op,
                                           &old_value, &new_value)) {
        node = (SR_OP_DELETED == op) ? old_value : new_value;

        if (SR_OP_CREATED == op || SR_OP_MODIFIED == op) {
            /* Get current handle value */
            handle = node->data.uint32_val;

            /* Check if the handle value is between 0 and MAX_SUPPORTED_STREAMS
             * value
             */
            if (handle > max_handle) {
                sr_set_error(session, xpath, ERR_STREAM_ID_MAX_HANDLE_VALUE,
                             handle, max_handle);
                sr_ret = SR_ERR_OPERATION_FAILED;
                goto free_chg_iter;
            }
        }
    }

free_chg_iter:
    if (NULL != iter) {
        sr_free_change_iter(iter);
        iter = NULL;
    }

    return sr_ret;

}

#if 0
/**
 * @brief Helper function for 'chk_handles_cb' callback. Used to check if there
 * is an overlapping of handles.
 *
 * @param[in]   session         Implicit session (do not stop) with information
 *                              about the changed data (retrieved by
 *                              sr_get_changes_iter) the event originator
 *                              session IDs.
 * @param[in]   xpath           XPath used when subscribing, NULL if the whole
 *                              module was subscribed to.
 * @param[in]   max_handle      Maximum allowed handle value.
 * @return      Error code (SR_ERR_OK on success).
 */
static int chk_handles_overlap(sr_session_ctx_t *session, const char *xpath,
                               uint32_t max_handle)
{
    int sr_ret = SR_ERR_OK;
    bmp_word_t *bmp = NULL;
    sr_val_t *handles_sr = NULL;
    uint32_t cnt = 0;
    uint32_t i = 0;

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    // Reserve bitmap as underlying structure for overlap check
    if (NULL == (bmp = (bmp_word_t *)calloc(max_handle/sizeof(bmp_word_t) + 1,
                                            sizeof(bmp_word_t)))) {
        sr_set_error(session, xpath, ERR_MEMORY_ALLOC_FAILED_STR);
        return SR_ERR_OPERATION_FAILED;
    }

    if (SR_ERR_OK != sr_get_items(session, xpath, 0, 0, &handles_sr, &cnt)) {
        sr_set_error(session, xpath, ERR_FAILED_TO_GET_ITEMS_XPATH_STR, xpath);
        sr_ret = SR_ERR_OPERATION_FAILED;
        goto free_bmp;
    }

    for (i = 0; i < cnt; i++) {
        if (1U == bmp_get_bit(bmp, handles_sr[i].data.uint32_val)) {
            sr_set_error(session, xpath, ERR_STREAM_ID_HANDLE_VALUE,
                         handles_sr[i].data.uint32_val);
            sr_ret = SR_ERR_OPERATION_FAILED;
            goto free_items;
        } else {
            bmp_set_bit(bmp, handles_sr[i].data.uint32_val);
        }
    }

free_items:
    if (NULL != handles_sr) {
        sr_free_values(handles_sr, cnt);
        handles_sr = NULL;
    }

free_bmp:
    if (NULL != bmp) {
        free(bmp);
        bmp = NULL;
    }

    return sr_ret;
}
#endif

/**
 * @brief Callback to be called by the event of changing content of leaf
 * 'handle' inside 'stream-identity' list. Used for value check.
 *
 * Subscribe to it by sr_module_change_subscribe call.
 *
 * @param[in]   session         Implicit session (do not stop) with information
 *                              about the changed data (retrieved by
 *                              sr_get_changes_iter) the event originator
 *                              session IDs.
 * @param[in]   module_name     Name of the module where the change has
 *                              occurred.
 * @param[in]   xpath           XPath used when subscribing, NULL if the whole
 *                              module was subscribed to.
 * @param[in]   event           Type of the notification event that has
 *                              occurred.
 * @param[in]   request_id      Request ID unique for the specific module_name.
 *                              Connected events for one request (SR_EV_CHANGE
 *                              and SR_EV_DONE, for example) have the same
 *                              request ID.
 * @param[in]   private_data    Private context opaque to sysrepo, as passed to
 *                              sr_module_change_subscribe call.
 * @return      Error code (SR_ERR_OK on success).
 */
static int chk_handles_cb(sr_session_ctx_t *session, const char *module_name,
                          const char *xpath, sr_event_t event,
                          uint32_t request_id, void *private_data)
{
    (void)module_name;
    (void)request_id;
    (void)private_data;

    char br_names[MAX_NUM_OF_INTERFACES][MAX_STR_LEN] = {0};
    tsn_return_value tsn_ret = E_TSN_LIB_NO_ERROR;
    struct port_hdl *br_hdl = NULL;
    uint32_t max_handle = 0;

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    if ((SR_EV_DONE == event) || (0 == plugin_init)) {
        SRP_LOG_DBG(DEBUG_MSG_WITH_TWO_PARAM, DBG_APPLYING_CHANGES_MSG, __func__);
        return SR_ERR_OK;
    }

    /* Get bridge name */
    if (EXIT_FAILURE == get_bridge_names(br_names, MAX_NUM_OF_INTERFACES, MAX_STR_LEN)) {
        sr_set_error(session, xpath, ERR_BRIDGE_NAMES_STR);
        return SR_ERR_OPERATION_FAILED;
    }

    /* Get bridge handle */
    if (NULL == (br_hdl = tsn_port_open_by_name(br_names[0]))) {
        sr_set_error(session, xpath, ERR_PORT_NOT_OPEN_XPATH_STR, br_names[0]);
        return SR_ERR_OPERATION_FAILED;
    }

    /* Get num of max supported stream handles */
    if (E_TSN_LIB_NO_ERROR != (tsn_ret = tsn_sid_get_max_sup(br_hdl, &max_handle))) {
        tsn_port_close(br_hdl);
        br_hdl = NULL;
        sr_set_error(session, xpath, ERR_STREAM_ID_MAX_SUPPORTED);
        return SR_ERR_OPERATION_FAILED;
    }

    tsn_port_close(br_hdl);
    br_hdl = NULL;

    /* Check if handle value is out of allowed range */
    if (SR_ERR_OK != chk_handle_value(session, xpath, max_handle)) {
        return SR_ERR_OPERATION_FAILED;
    }

#if 0
    /* Check for handles overlapping */
    if (SR_ERR_OK != chk_handles_overlap(session, xpath, max_handle)) {
        return SR_ERR_OPERATION_FAILED;
    }
#endif

    return SR_ERR_OK;
}

/**
 * @brief Callback to be called by the event of changing content of leaf
 * 'handle' inside 'stream-identity' list. Used to update existing entry.
 *
 * Subscribe to it by sr_module_change_subscribe call.
 *
 * @param[in]   session         Implicit session (do not stop) with information
 *                              about the changed data (retrieved by
 *                              sr_get_changes_iter) the event originator
 *                              session IDs.
 * @param[in]   module_name     Name of the module where the change has
 *                              occurred.
 * @param[in]   xpath           XPath used when subscribing, NULL if the whole
 *                              module was subscribed to.
 * @param[in]   event           Type of the notification event that has
 *                              occurred.
 * @param[in]   request_id      Request ID unique for the specific module_name.
 *                              Connected events for one request (SR_EV_CHANGE
 *                              and SR_EV_DONE, for example) have the same
 *                              request ID.
 * @param[in]   private_data    Private context opaque to sysrepo, as passed to
 *                              sr_module_change_subscribe call.
 * @return      Error code (SR_ERR_OK on success).
 */
static int mod_handle_cb(sr_session_ctx_t *session, const char *module_name,
                         const char *xpath, sr_event_t event,
                         uint32_t request_id, void *private_data)
{
    (void)module_name;
    (void)request_id;
    (void)private_data;

    int sr_ret = SR_ERR_OK;
    tsn_return_value tsn_ret = E_TSN_LIB_NO_ERROR;
    sr_xpath_ctx_t st = {0};
    sr_change_oper_t op = {0};
    sr_change_iter_t *iter = NULL;
    sr_val_t *node = NULL;
    sr_val_t *old_value = NULL;
    sr_val_t *new_value = NULL;
    struct port_hdl *br_hdl = NULL;
    uint16_t handle = 0;
    uint16_t index = 0;
    char index_str[MAX_STR_LEN] = "";
    char br_names[MAX_NUM_OF_INTERFACES][MAX_STR_LEN]= {0};

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    if ((SR_EV_DONE == event) || (0 == plugin_init)) {
        SRP_LOG_DBG(DEBUG_MSG_WITH_TWO_PARAM, DBG_APPLYING_CHANGES_MSG, __func__);
        return SR_ERR_OK;
    }

    /* Get bridge name */
    if (EXIT_FAILURE == get_bridge_names(br_names, MAX_NUM_OF_INTERFACES,
                                         MAX_STR_LEN)) {
        sr_set_error(session, xpath, ERR_BRIDGE_NAMES_STR);
        return SR_ERR_OPERATION_FAILED;
    }

    /* Get bridge handle */
    if (NULL == (br_hdl = tsn_port_open_by_name(br_names[0]))) {
        sr_set_error(session, xpath, ERR_PORT_NOT_OPEN_XPATH_STR, br_names[0]);
        return SR_ERR_OPERATION_FAILED;
    }

    /* Get changes iter */
    if (SR_ERR_OK != sr_get_changes_iter(session, xpath, &iter)) {
        sr_set_error(session, xpath, ERR_FORMING_ITERATOR_FAILED_STR);
        sr_ret = SR_ERR_OPERATION_FAILED;
        goto free_br_hdl;
    }

    while (SR_ERR_OK == sr_get_change_next(session, iter, &op,
                                           &old_value, &new_value)) {
        node = (SR_OP_DELETED == op) ? old_value : new_value;

        if (SR_OP_MODIFIED == op) {
            handle = node->data.uint16_val;

            /* Get INDEX value */
            if (EXIT_SUCCESS != get_key_value(session, node->xpath,
                                              STREAM_ID_STREAM_IDENT_LIST_STR,
                                              STREAM_ID_INDEX_STR,
                                              &st, index_str)) {
                return SR_ERR_OPERATION_FAILED;
            }

            index = (uint16_t)strtoul(index_str, NULL, 10);

            /* Set handle value */
            if (E_TSN_LIB_NO_ERROR != (tsn_ret = tsn_sid_set_strhdl(br_hdl, index, handle))) {
                sr_set_error(session, node->xpath, ERR_SETTER_EXT_FMT_STR,
                             STREAM_ID_STREAM_IDENT_LIST_STR,
                             STREAM_ID_HANDLE_STR, LIT_XPATH_STR, node->xpath,
                             tsn_lib_get_known_ernno_msg(tsn_ret));
                sr_ret = SR_ERR_OPERATION_FAILED;
                goto free_chg_iter;
            }
        }
    }

free_chg_iter:
    if (NULL != iter) {
        sr_free_change_iter(iter);
        iter = NULL;
    }
free_br_hdl:
    if (NULL != br_hdl) {
        tsn_port_close(br_hdl);
        br_hdl = NULL;
    }

    return sr_ret;

}

/**
 * @brief Callback to be called by the event of changing content of leaf
 * 'tagged' inside 'null-stream-identification' container. Used for value
 * check.
 *
 * Subscribe to it by sr_module_change_subscribe call.
 *
 * @param[in]   session         Implicit session (do not stop) with information
 *                              about the changed data (retrieved by
 *                              sr_get_changes_iter) the event originator
 *                              session IDs.
 * @param[in]   module_name     Name of the module where the change has
 *                              occurred.
 * @param[in]   xpath           XPath used when subscribing, NULL if the whole
 *                              module was subscribed to.
 * @param[in]   event           Type of the notification event that has
 *                              occurred.
 * @param[in]   request_id      Request ID unique for the specific module_name.
 *                              Connected events for one request (SR_EV_CHANGE
 *                              and SR_EV_DONE, for example) have the same
 *                              request ID.
 * @param[in]   private_data    Private context opaque to sysrepo, as passed to
 *                              sr_module_change_subscribe call.
 * @return      Error code (SR_ERR_OK on success).
 */
static int chk_tagged_cb(sr_session_ctx_t *session, const char *module_name,
                         const char *xpath, sr_event_t event,
                         uint32_t request_id, void *private_data)
{
    (void)module_name;
    (void)request_id;
    (void)private_data;

    int sr_ret = SR_ERR_OK;
    sr_change_iter_t *iter = NULL;
    sr_change_oper_t op = {0};
    sr_val_t *node = NULL;
    sr_val_t *old_value = NULL;
    sr_val_t *new_value = NULL;
    char tag_str[MAX_STR_LEN] = "";
    sid_tag tag = SID_TAGGED;
    char path[2*MAX_STR_LEN] = "";
    char index_str[MAX_STR_LEN] = "";
    sr_xpath_ctx_t st = {0};
    sr_val_t *vlan = NULL;

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    if ((SR_EV_DONE == event) || (0 == plugin_init)) {
        SRP_LOG_DBG(DEBUG_MSG_WITH_TWO_PARAM, DBG_APPLYING_CHANGES_MSG, __func__);
        return SR_ERR_OK;
    }

    /* Get changes iter */
    sr_ret = sr_get_changes_iter(session, xpath, &iter);
    if (SR_ERR_OK != sr_ret) {
        return SR_ERR_OPERATION_FAILED;
    }

    while (SR_ERR_OK == sr_get_change_next(session, iter, &op,
                                           &old_value, &new_value)) {
        node = (op == SR_OP_DELETED) ? old_value : new_value;

        if (SR_OP_CREATED == op || SR_OP_MODIFIED == op) {
            strncpy(tag_str, new_value->data.enum_val, (MAX_STR_LEN-1));
            if (EXIT_FAILURE == translate_single_string_using_map(tag_str, stream_tagged_reverse_map)) {
                sr_set_error(session, new_value->xpath, ERR_MSG_EXT_FMT_STR,
                             LIT_GET_STR, STREAM_ID_TAGGED_STR, LIT_VAL_STR,
                             LIT_XPATH_STR, new_value->xpath);
                sr_ret = SR_ERR_OPERATION_FAILED;
                goto free_chg_iter;
            }

            tag = strtol(tag_str, NULL, 10);

            if (SID_ALL == tag) {
                // currently only tagged value is supported
                sr_set_error(session, new_value->xpath,
                             ERR_MSG_AND_EXPL_EXT_FMT_STR, LIT_SET_STR,
                             STREAM_ID_TAGGED_STR, LIT_VAL_STR, LIT_XPATH_STR,
                             new_value->xpath, ERR_STREAM_ID_TAGGED_ALL);
                sr_ret = SR_ERR_OPERATION_FAILED;
                goto free_chg_iter;
            }

            /* find vlan object value */
            /* Get leaf index, key of the stream-identity list */
            if (EXIT_SUCCESS != get_key_value(session, node->xpath,
                                              STREAM_ID_STREAM_IDENT_LIST_STR,
                                              STREAM_ID_INDEX_STR,
                                              &st, index_str)) {
                return SR_ERR_OPERATION_FAILED;
            }
            fill_xpath(path, STREAM_ID_NULL_STREAM_VLAN_XPATH, index_str);
            if (SR_ERR_OK != sr_get_item(session, path, 0, &vlan)) {
                sr_set_error(session, path, ERR_FAILED_GET_OBJ_STR, path);
                return SR_ERR_OPERATION_FAILED;
            }
            if (SID_PRIORITY == tag && 0 != vlan->data.uint16_val) {
                // The tagged=priority is supported only for vlan=0
                sr_set_error(session, new_value->xpath,
                             ERR_MSG_AND_EXPL_EXT_FMT_STR, LIT_SET_STR,
                             STREAM_ID_TAGGED_STR, LIT_VAL_STR, LIT_XPATH_STR,
                             new_value->xpath, ERR_STREAM_ID_TAGGED_PRIORITY);
                sr_ret = SR_ERR_OPERATION_FAILED;
                goto free_chg_iter;
            }
        }
    }

free_chg_iter:
    if (NULL != iter) {
        sr_free_change_iter(iter);
        iter = NULL;
    }

    return sr_ret;

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
    char *path = NULL;

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    /* Set plugin_init to "1" so callbacks are triggered when startup
     * configuration is copied to running.
     */
    if (0 != sr_path_check_startup_done_file(STREAM_ID_MODULE_NAME_STR, &path)) {
        plugin_init = 1;
    }

    do {
        /* subscribe for /ieee802-dot1cb-stream-identification module changes */
        rc = sr_module_change_subscribe(session, STREAM_ID_MODULE_NAME_STR, NULL,
                module_change_cb, NULL, 0, SR_SUBSCR_ENABLED, &subscription);
         if (SR_ERR_OK != rc) break;

        /* subscribe for /ieee802-dot1cb-stream-identification:stream-identity/index changes */
        rc = sr_module_change_subscribe(session, STREAM_ID_MODULE_NAME_STR, "/ieee802-dot1cb-stream-identification:stream-identity/index",
                stream_identity_list_cb, NULL, 2, SR_SUBSCR_ENABLED, &subscription);
        if (rc != SR_ERR_OK) break;

        /* subscribe for /ieee802-dot1cb-stream-identification:stream-identity/smac-vlan-stream-identification changes */
        rc = sr_module_change_subscribe(session, STREAM_ID_MODULE_NAME_STR, "/ieee802-dot1cb-stream-identification:stream-identity/smac-vlan-stream-identification/*",
                add_rem_mod_not_supported_cb, NULL, 2, SR_SUBSCR_ENABLED, &subscription);
        if (rc != SR_ERR_OK) break;

        /* subscribe for /ieee802-dot1cb-stream-identification:stream-identity/dmac-vlan-stream-identification changes */
        rc = sr_module_change_subscribe(session, STREAM_ID_MODULE_NAME_STR, "/ieee802-dot1cb-stream-identification:stream-identity/dmac-vlan-stream-identification/*",
                add_rem_mod_not_supported_cb, NULL, 2, SR_SUBSCR_ENABLED, &subscription);
        if (rc != SR_ERR_OK) break;

        /* subscribe for /ieee802-dot1cb-stream-identification:stream-identity/ip-stream-identification changes */
        rc = sr_module_change_subscribe(session, STREAM_ID_MODULE_NAME_STR, "/ieee802-dot1cb-stream-identification:stream-identity/ip-stream-identification/*",
                add_rem_mod_not_supported_cb, NULL, 2, SR_SUBSCR_ENABLED, &subscription);
        if (rc != SR_ERR_OK) break;

        /* subscribe for /ieee802-dot1cb-stream-identification:stream-identity/handle changes */
        rc = sr_module_change_subscribe(session, STREAM_ID_MODULE_NAME_STR, "/ieee802-dot1cb-stream-identification:stream-identity/handle",
                chk_handles_cb, NULL, 4, SR_SUBSCR_ENABLED, &subscription);
        if (rc != SR_ERR_OK) break;

        /* subscribe for /ieee802-dot1cb-stream-identification:stream-identity/tagged changes */
        rc = sr_module_change_subscribe(session, STREAM_ID_MODULE_NAME_STR, "/ieee802-dot1cb-stream-identification:stream-identity/null-stream-identification/tagged",
                chk_tagged_cb, NULL, 4, SR_SUBSCR_ENABLED, &subscription);
        if (rc != SR_ERR_OK) break;

        /* subscribe for /ieee802-dot1cb-stream-identification:stream-identity/handle changes */
        rc = sr_module_change_subscribe(session, STREAM_ID_MODULE_NAME_STR, "/ieee802-dot1cb-stream-identification:stream-identity/handle",
                mod_handle_cb, NULL, 3, SR_SUBSCR_ENABLED, &subscription);
        if (rc != SR_ERR_OK) break;

        /* subscribe for /ieee802-dot1cb-stream-identification:stream-identity/null-stream-identification/destination-mac changes */
        rc = sr_module_change_subscribe(session, STREAM_ID_MODULE_NAME_STR, "/ieee802-dot1cb-stream-identification:stream-identity/null-stream-identification/destination-mac",
                mod_null_stream_id_cb, NULL, 3, SR_SUBSCR_ENABLED, &subscription);
        if (rc != SR_ERR_OK) break;

        /* subscribe for /ieee802-dot1cb-stream-identification:stream-identity/tagged changes */
        rc = sr_module_change_subscribe(session, STREAM_ID_MODULE_NAME_STR, "/ieee802-dot1cb-stream-identification:stream-identity/null-stream-identification/tagged",
                mod_null_stream_id_cb, NULL, 3, SR_SUBSCR_ENABLED, &subscription);
        if (rc != SR_ERR_OK) break;

        /* subscribe for /ieee802-dot1cb-stream-identification:stream-identity/null-stream-identification/vlan changes */
        rc = sr_module_change_subscribe(session, STREAM_ID_MODULE_NAME_STR, "/ieee802-dot1cb-stream-identification:stream-identity/null-stream-identification/vlan",
                mod_null_stream_id_cb, NULL, 3, SR_SUBSCR_ENABLED, &subscription);
        if (rc != SR_ERR_OK) break;

        /* subscribe for /ieee802-dot1cb-stream-identification:stream-identity/null-stream-identification changes */
        rc = sr_module_change_subscribe(session, STREAM_ID_MODULE_NAME_STR, "/ieee802-dot1cb-stream-identification:stream-identity/null-stream-identification/*",
                del_null_stream_id_cb, NULL, 1, SR_SUBSCR_ENABLED, &subscription);
        if (rc != SR_ERR_OK) break;

        /* subscribe for /ieee802-dot1cb-stream-identification:stream-identity/out-facing/input-port-list changes */
        rc = sr_module_change_subscribe(session, STREAM_ID_MODULE_NAME_STR, "/ieee802-dot1cb-stream-identification:stream-identity/out-facing/input-port",
                out_facing_input_port_list_cb, NULL, 1, SR_SUBSCR_ENABLED, &subscription);
        if (rc != SR_ERR_OK) break;

    } while (0);

    if (SR_ERR_OK != rc) {
        SRP_LOG_ERR(ERR_MODULE_INIT_FAILED_STR, STREAM_ID_MODULE_NAME_STR, sr_strerror(rc));
        sr_unsubscribe(subscription);
        return rc;
    }

    if (EXIT_FAILURE == stream_identification_fill_startup_datastore(session)) {
        SRP_LOG_ERR(ERROR_MSG_MOD_FUNC_MSG, STREAM_ID_MODULE_NAME_STR, __func__,
                    ERR_MSG_EMPTY_STARTUP_DS_STR);
        return SR_ERR_OPERATION_FAILED;
    }

    plugin_init = 1;

    SRP_LOG_INF(INF_MODULE_INIT_SUCCESS_STR, STREAM_ID_MODULE_NAME_STR);
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
    SRP_LOG_INF(INF_MODULE_CLEANUP_STR, STREAM_ID_MODULE_NAME_STR);
}
