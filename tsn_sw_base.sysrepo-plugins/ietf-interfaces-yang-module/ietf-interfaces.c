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

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

/* common includes */
#include "common_defines.h"
#include "common.h"
#include "config_change.h"
/* sysrepo includes */
#include <sysrepo.h>
#include <sysrepo/values.h>
#include <sysrepo/xpath.h>
/* libbase includes */
#include "libbase/base_defines.h"
#include "libbase/base_prints.h"
#include "libbase/base_linux_tool_helpers.h"
#include "libbase/base_interfaces_linux_tools.h"
#include "libbase/base_private_translators.h"
#include "libbase/base_defines.h"
#include "libbase/base_private_defines.h"
#include "libbase/base_q_bridge_linux_tools.h"
/* libtsn includes */
#include "libtsn/tsn.h"
#include "libtsn/tsn_preemp.h"
/* module specific */
#include "ietf_interfaces_defines.h"
//#include "sched.h"
#include "preempt.h"
#include "preemption.h"
#include "ethernet.h"
#include "bridge_port.h"
#include "stream_identification_statistics.h"
#include "frer_statistics.h"

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

    rc = sr_get_items(session, "/ietf-interfaces:ietf-interfaces/interfaces", 0, 0, &values, &count);
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
 * @brief Callback to be called by the event of changing content of node with
 * "/ietf-interfaces:interfaces/interface/enabled" xpath.
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
static int interface_enabled_cb(sr_session_ctx_t *session,
                                const char *module_name, const char *xpath,
                                sr_event_t event, uint32_t request_id,
                                void *private_data)
{
    (void)event;
    (void)module_name;
    (void)request_id;
    (void)private_data;

    int rc = SR_ERR_OK;
    sr_change_oper_t op = {0};
    sr_xpath_ctx_t st = {0};
    sr_change_iter_t *iter = NULL;
    sr_val_t *old_value = NULL;
    sr_val_t *new_value = NULL;
    sr_val_t *node = NULL;
    char tmp[2*MAX_STR_LEN] = "";
    char interface_name[MAX_STR_LEN] = "";

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

        /* If user changed values of leaf then node->dflt is "0". If sysrepo changes value of leaf, because it has
         * default value, that node->dflt will be "1".
         * This condition is added because of booting, i.e. applying startup configuration.
         * Every leaf that has default value will be triggered with ADD operation if he does not exist inside startup configuration.
         * Its node->dflt flag will be "1" then. */
        if (((op == SR_OP_MODIFIED) || (op == SR_OP_CREATED)) &&
            (0 == node->dflt)) {
            /* mapping: true = up, false = down */
            snprintf(tmp, MAX_STR_LEN, "%s", bool_map[node->data.bool_val]);
            if (EXIT_FAILURE == translate_single_string_using_map(tmp, if_enable_map)) {
                sr_set_error(session, node->xpath, ERR_INVALID_VALUE_XPATH_STR,
                             node->xpath);
                return SR_ERR_OPERATION_FAILED;
            }
            if (EXIT_SUCCESS != set_interface_enabled(interface_name, tmp)) {
                sr_set_error(session, node->xpath,
                             ERR_SET_FUNC_FAILED_XPATH_STR,
                             node->xpath);
                return SR_ERR_OPERATION_FAILED;
            }
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
    sr_free_change_iter(iter);
    node = NULL;

    return SR_ERR_OK;
}

/**
 * @brief This function creates a new entry for list interface (which is config true).
 *
 * @param[in]   ctx             libyang context handler.
 * @param[out]  root            Pointer to struct lyd_node which is going to be
 *                              parent to created entry.
 * @param[in]   module          Main schema node structure representing YANG
 *                              module.
 * @param[in]   if_name         Contains the name of the interface.
 * @return      struct lyd_node* or NULL if element not created.
 */
struct lyd_node *if_create_list_interface_entry(struct ly_ctx *ctx,
                                                struct lyd_node *root,
                                                const struct lys_module *module,
                                                char *if_name)
{
    struct lyd_node *entry;
    struct port_hdl *port = NULL;
    char objects[MAX_NUM_OF_INTERFACES][MAX_STR_LEN] = {0};
    char result_array[MAX_NUM_OF_INTERFACES][MAX_STR_LEN] = {0};
    int num_tc = 8;
    int prio = 0;
    int i = 0;

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    /* new interface in interfaces list */
    if (EXIT_FAILURE == new_container(root, &entry, module, IF_INTERFACE_LIST)) {
        return NULL;
    }

    /* list interface, leaf name */
    if (EXIT_FAILURE == new_node(entry, module, IF_NAME_STR, if_name)) {
        return NULL;
    }

    /* list interface, leaf type */
    if (EXIT_FAILURE == new_node(entry, module, IF_TYPE_STR, IF_ETH_CSMACD_STR)) {
        return NULL;
    }

    /* list interface, leaf enabled */
    if (EXIT_SUCCESS == get_interface_enabled(objects, MAX_NUM_OF_INTERFACES,
                                              MAX_STR_LEN, if_name)) {
        if (EXIT_FAILURE == translate_single_string_using_map(objects[0], if_xml_enable_map)) {
            return NULL;
        }
        if (EXIT_FAILURE == new_node(entry, module, IF_ENABLE_STR, objects[0])) {
            return NULL;
        }
    }

    /* bridge-port, augment from ieee802-dot1q-bridge yang module */
    if (EXIT_SUCCESS != get_bridge_port_names(result_array, MAX_NUM_OF_INTERFACES, MAX_STR_LEN)) {
        SRP_LOG_WRN(ERROR_MSG_MOD_FUNC_EL_MSG, Q_BRIDGE_MODULE_NAME, __func__,
                    Q_BRIDGE_BRIDGE_PORTS, ERR_NOT_CREATED_ELEMENT_STR);
        return NULL;
    }

    i = 0;
    while ((i < MAX_NUM_OF_INTERFACES) &&
           (0 != strncmp(result_array[i], "", MAX_STR_LEN))) {
        if ((NULL != if_name) &&
            (0 == strncmp(if_name, result_array[i], MAX_STR_LEN))) {
            if (NULL == new_bridge_port_node(entry, ctx, if_name)) {
                return NULL;
            }
        }
        i++;
    }

    /* ieee802-ethernet-interface module implementation */
    /* First check if ethernet auto-negotiation is supported.
     * If it is not, do not create ethernet container and objects.
     */
    if (EXIT_SUCCESS == get_eth_if_auto_neg_enable(objects, MAX_NUM_OF_INTERFACES,
                                                   MAX_STR_LEN, if_name)) {
        if (NULL == new_ethernet_container(entry, ctx, if_name)) {
            return NULL;
        }
    }

    /* ieee802-dot1q-sched module */
    port = tsn_port_open_by_name(if_name);
    if (NULL == port) {
        SRP_LOG_ERR(ERROR_MSG_MOD_FUNC_EL_MSG, PREE_MACMERGE_MODULE_NAME_STR, __func__,
                    if_name, ERR_PORT_NOT_OPEN_STR);
        return NULL;
    }
    if (!tsn_is_feat_supported(port, FEAT_ST)) {
        SRP_LOG_INF(ERROR_MSG_MOD_FUNC_EL_MSG, SCHED_MODULE_NAME_STR, __func__,
                    ERR_SCHED_NOT_SUPPORTED_STR, if_name);
        tsn_port_close(port);
    } else {
        tsn_port_close(port);
        /*  list max-sdu-table, augment from ieee802-dot1q-sched yang module.
         *  with only prio = 0
         */
        for (i = 0; i < num_tc; i++) {
            if (NULL == new_max_sdu_entry(entry, ctx, if_name, prio, i)) {
                return NULL;
            }
        }
        /* gate_parameters, augment from ieee802-dot1q-sched yang module */
        if (NULL == new_gate_parameters_container(entry, ctx, if_name)) {
            return NULL;
        }
    }

    /* ieee802-dot3br-preempt module implementation */
    port = tsn_port_open_by_name(if_name);
    if (NULL == port) {
        SRP_LOG_ERR(ERROR_MSG_MOD_FUNC_EL_MSG, PREE_MACMERGE_MODULE_NAME_STR, __func__,
                    if_name, ERR_PORT_NOT_OPEN_STR);
        return NULL;
    }
    if (!tsn_is_feat_supported(port, FEAT_MACMERGE)) {
        SRP_LOG_INF(ERROR_MSG_MOD_FUNC_EL_MSG, PREE_MACMERGE_MODULE_NAME_STR, __func__,
                    ERROR_PREE_MAC_MERGE_NOT_SUPPORTED_STR, if_name);
        tsn_port_close(port);
    } else {
        tsn_port_close(port);
        if (NULL == new_mac_merge_container(entry, ctx, if_name)) {
            return NULL;
        }
    }

    /* ieee802-dot1q-preemption module implementation */
    port = tsn_port_open_by_name(if_name);
    if (NULL == port) {
        SRP_LOG_ERR(ERROR_MSG_MOD_FUNC_EL_MSG, PREEMPT_MODULE_NAME_STR, __func__,
                    if_name, ERR_PORT_NOT_OPEN_STR);
        return NULL;
    }
    if (!tsn_is_feat_supported(port, FEAT_PREE)) {
        SRP_LOG_INF(ERROR_MSG_MOD_FUNC_EL_MSG, PREEMPT_MODULE_NAME_STR, __func__,
                    ERROR_PREEMPT_NOT_SUPPORTED_STR, if_name);
        tsn_port_close(port);
    } else {
        tsn_port_close(port);
        if (NULL == new_frame_preemption_parameters_container(entry, ctx, if_name)) {
            return NULL;
        }
    }

    return entry;
}


/**
 * @brief Creates a startup datastore for ietf-interfaces module. Function is
 * going to be called once, due to sysrepo-plugind startup.
 *
 * @param[in]   session         Sysrepo session that can be used for any API
 *                              calls needed for plugin cleanup (mainly for
 *                              unsubscribing of subscriptions initialized in
 *                              sr_plugin_init_cb).
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
static int interfaces_fill_startup_datastore(sr_session_ctx_t *session)
{
    const struct ly_ctx *ctx = NULL;
    const struct lys_module *module = NULL;
    struct lyd_node *root = NULL;
    struct lyd_node *entry = NULL;
    char interface_names[MAX_NUM_OF_INTERFACES][MAX_STR_LEN] = {0};
    int i = 0;
    int fd = -1;
    char *path = NULL;

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    if (0 == sr_path_check_startup_done_file(INTERFACES_MODULE_NAME, &path)) {
        ctx = sr_get_context(sr_session_get_connection(session));
        if (NULL == ctx) {
            SRP_LOG_ERR(ERROR_MSG_FUN_AND_MSG, __func__, ERR_SESSION_CTX_FAILED_STR);
            free(path);
            return EXIT_FAILURE;
        }

        /* create ietf-interfaces root element (container interfaces) */
        root = lyd_new_path(NULL, ctx, "/ietf-interfaces:interfaces", NULL, 0, 0);
        if (NULL == root) {
            SRP_LOG_ERR(ERROR_MSG_FUN_AND_MSG, __func__,
                        ERR_NOT_CREATED_ROOT_ELEMENT_STR);
            free(path);
            return EXIT_FAILURE;
        }

        /* load "iana-if-type" module to use necessary 'identity' types */
        if (NULL == ly_ctx_load_module((struct ly_ctx*)ctx, "iana-if-type", NULL)) {
            SRP_LOG_ERR(ERROR_MSG_FUN_AND_MSG, __func__, ERR_MSG_LOAD_MODULE_STR);
            free(path);
            return EXIT_FAILURE;
        }

        /* get all interfaces and create interface list entries */
        if (EXIT_FAILURE == get_interface_names(interface_names,
                                                MAX_NUM_OF_INTERFACES,
                                                MAX_STR_LEN)) {
            free(path);
            return EXIT_FAILURE;
        }

        while ((i < MAX_NUM_OF_INTERFACES) &&
               (0 != strncmp(interface_names[i], "", MAX_STR_LEN))) {
            entry = if_create_list_interface_entry((struct ly_ctx*)ctx, root,
                                                   module, interface_names[i]);
            if (NULL == entry) {
                SRP_LOG_WRN(ERROR_MSG_MOD_FUNC_MSG, IF_MODULE_NAME_STR, __func__,
                            ERR_MSG_EMPTY_STARTUP_DS_STR);
            }
            i++;
        }

        /******** NOTE *******/
        /* The startup to running datastore copy is done before the plugin is started.
         * So, here we will replace the current startup and running datastores with the
         * subtree called 'root' in this function.
         */

        /* switch to startup datastore */
        if (SR_ERR_OK != sr_session_switch_ds(session, SR_DS_STARTUP)) {
            SRP_LOG_ERR(ERROR_MSG_FUN_AND_MSG, __func__,
                        ERR_SWITCH_DATASTORE_FAILED_STR);
            free(path);
            return EXIT_FAILURE;
        }

        /* Replace current running configuration with created 'root' subtree */
        if (SR_ERR_OK != sr_replace_config(session, IF_MODULE_NAME_STR, root, 0, 0)) {
            SRP_LOG_ERR(ERROR_MSG_FUN_AND_MSG, __func__,
                        ERR_REPLACE_CONFIG_FAILED_STR);
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

        /* copy config from startup to running datastore */
        if (SR_ERR_OK != sr_copy_config(session, IF_MODULE_NAME_STR, SR_DS_STARTUP, 0, 0)) {
            SRP_LOG_ERR(ERROR_MSG_FUN_AND_MSG, __func__,
                        ERR_COPY_DATASTORE_FAILED_STR);
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

/**
 * @brief Creates a new container statistics for list 'interface' inside
 * ietf-interfaces yang module.
 *
 * This function is for state data from list 'interface'.
 *
 * @param[in]   parent          Pointer to an existing parent of the requested
 *                              nodes. Is NULL for top-level nodes. Caller is
 *                              supposed to append the requested nodes to this
 *                              data subtree and return either the original
 *                              parent or a top-level node.
 * @param[in]   interface_name  Name of the interface.
 * @return      Error code (SR_ERR_OK on success).
 */
static int new_statistics_container(struct lyd_node **parent,
                                    char *interface_name)
{
    char path[MAX_STR_LEN] = "";
    char tx_objects[MAX_NUM_OF_INTERFACES][MAX_STR_LEN] = {0};
    char rx_objects[MAX_NUM_OF_INTERFACES][MAX_STR_LEN] = {0};

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    if (EXIT_FAILURE == get_interface_rx_statistics(rx_objects, MAX_NUM_OF_INTERFACES,
                                                    MAX_STR_LEN, interface_name) ||
        EXIT_FAILURE == get_interface_tx_statistics(tx_objects, MAX_NUM_OF_INTERFACES,
                                                    MAX_STR_LEN, interface_name)) {
        SRP_LOG_ERR(ERROR_MSG_FUN_AND_MSG, __func__,
                    ERR_FAILED_TO_GET_STATISTICS);
        return SR_ERR_OPERATION_FAILED;
    }

    /* leaf in-octets */
    if (strncmp(rx_objects[0], "", MAX_STR_LEN)) {
        fill_xpath(path, INTERFACE_STATISTICS_IN_OCTETS_XPATH, interface_name);
        if (NULL == lyd_new_path(*parent, NULL, path, rx_objects[0], 0, 0)) {
            SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                        IF_IN_OCTETS_STR, ERR_NOT_CREATED_ELEMENT_STR);
        }
    }

    /* leaf in-unicast-kts */
    if (strncmp(rx_objects[1], "", MAX_STR_LEN)) {
        fill_xpath(path, INTERFACE_STATISTICS_IN_UNICAST_PKTS_XPATH, interface_name);
        if (NULL == lyd_new_path(*parent, NULL, path, rx_objects[1], 0, 0)) {
            SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                        IF_IN_UNICAST_PKTS_STR, ERR_NOT_CREATED_ELEMENT_STR);
        }
    }

    /* leaf in-multicast-pkts */
    if (strncmp(rx_objects[5], "", MAX_STR_LEN)) {
        fill_xpath(path, INTERFACE_STATISTICS_IN_MULTICAST_PKTS_XPATH, interface_name);
        if (NULL == lyd_new_path(*parent, NULL, path, rx_objects[5], 0, 0)) {
            SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                        IF_IN_MULTICAST_PKTS_STR, ERR_NOT_CREATED_ELEMENT_STR);
        }
    }

    /* leaf in-discards */
    if (strncmp(rx_objects[3], "", MAX_STR_LEN)) {
        fill_xpath(path, INTERFACE_STATISTICS_IN_DISCARDS_XPATH, interface_name);
        if (NULL == lyd_new_path(*parent, NULL, path, rx_objects[3], 0, 0)) {
            SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                        IF_IN_DISCARDS_STR, ERR_NOT_CREATED_ELEMENT_STR);
        }
    }

    /* leaf in-errors */
    if (strncmp(rx_objects[2], "", MAX_STR_LEN)) {
        fill_xpath(path, INTERFACE_STATISTICS_IN_ERRORS_XPATH, interface_name);
        if (NULL == lyd_new_path(*parent, NULL, path, rx_objects[2], 0, 0)) {
            SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                        IF_IN_ERRORS_STR, ERR_NOT_CREATED_ELEMENT_STR);
        }
    }

    /* leaf out-octets */
    if (strncmp(tx_objects[0], "", MAX_STR_LEN)) {
        fill_xpath(path, INTERFACE_STATISTICS_OUT_OCTETS_XPATH, interface_name);
        if (NULL == lyd_new_path(*parent, NULL, path, tx_objects[0], 0, 0)) {
            SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                        IF_OUT_OCTETS_STR, ERR_NOT_CREATED_ELEMENT_STR);
        }
    }

    /* leaf out-unicast-pkts */
    if (strncmp(tx_objects[1], "", MAX_STR_LEN)) {
        fill_xpath(path, INTERFACE_STATISTICS_OUT_UNICAST_PKTS_XPATH, interface_name);
        if (NULL == lyd_new_path(*parent, NULL, path, tx_objects[1], 0, 0)) {
            SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                        IF_OUT_UNICAST_PKTS_STR, ERR_NOT_CREATED_ELEMENT_STR);
        }
    }

    /* leaf out-multicast-pkts */
    if (strncmp(tx_objects[5], "", MAX_STR_LEN)) {
        fill_xpath(path, INTERFACE_STATISTICS_OUT_MULTICAST_PKTS_XPATH, interface_name);
        if (NULL == lyd_new_path(*parent, NULL, path, tx_objects[5], 0, 0)) {
            SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                        IF_OUT_MULTICAST_PKTS_STR, ERR_NOT_CREATED_ELEMENT_STR);
        }
    }

    /* leaf out-discards */
    if (strncmp(tx_objects[3], "", MAX_STR_LEN)) {
        fill_xpath(path, INTERFACE_STATISTICS_OUT_DISCARDS_XPATH, interface_name);
        if (NULL == lyd_new_path(*parent, NULL, path, tx_objects[3], 0, 0)) {
            SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                        IF_OUT_DISCARDS_STR, ERR_NOT_CREATED_ELEMENT_STR);
        }
    }

    /* leaf out-errors */
    if (strncmp(tx_objects[2], "", MAX_STR_LEN)) {
        fill_xpath(path, INTERFACE_STATISTICS_OUT_ERRORS_XPATH, interface_name);
        if (NULL == lyd_new_path(*parent, NULL, path, tx_objects[2], 0, 0)) {
            SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                        IF_OUT_ERRORS_STR, ERR_NOT_CREATED_ELEMENT_STR);
        }
    }

    return SR_ERR_OK;
}

/**
 * @brief Creates a new list entry for list 'interface' ietf-interfaces yang
 * module.
 *
 * This function is for state data from list 'interface'.
 *
 * @param[in]   parent          Pointer to an existing parent of the
 *                              requested nodes. Is NULL for top-level nodes.
 *                              Caller is supposed to append the requested
 *                              nodes to this data subtree and return either
 *                              the original parent or a top-level node.
 * @param[in]   interface_name  Name of the interface.
 * @return      Error code (SR_ERR_OK on success).
 */
static int new_interface_node(struct lyd_node **parent, char *interface_name)
{
    char path[MAX_STR_LEN] = "";
    char result_array[MAX_NUM_OF_INTERFACES][MAX_STR_LEN] = {0};
    char objects[MAX_NUM_OF_INTERFACES][MAX_STR_LEN] = {0};
    struct port_hdl *port = NULL;
    int num_tc = 8;
    int prio = 0;
    int i = 0;;

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    port = tsn_port_open_by_name(interface_name);
    if (NULL == port) {
        SRP_LOG_ERR(ERROR_MSG_MOD_FUNC_EL_MSG, IF_MODULE_NAME_STR, __func__, interface_name, ERR_PORT_NOT_OPEN_STR);
        return SR_ERR_OPERATION_FAILED;
    }

    /* leaf if-index */
    if (EXIT_SUCCESS == get_interface_if_index(result_array, MAX_NUM_OF_INTERFACES, MAX_STR_LEN, interface_name)) {
        if (strncmp(result_array[0], "", MAX_STR_LEN)) {
            fill_xpath(path, INTERFACE_IF_INDEX_XPATH, interface_name);
            if (NULL == lyd_new_path(*parent, NULL, path, result_array[0], 0, 0)) {
                SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                            IF_INDEX_STR, ERR_NOT_CREATED_ELEMENT_STR);
            }
        }
    }

    /* leaf phys-address */
    if (EXIT_SUCCESS == get_interface_address(result_array, MAX_NUM_OF_INTERFACES, MAX_STR_LEN, interface_name)) {
        if (strncmp(result_array[0], "", MAX_STR_LEN))
        {
            fill_xpath(path, INTERFACE_PHYS_ADDRESS_XPATH, interface_name);
            if (NULL == lyd_new_path(*parent, NULL, path, result_array[0], 0, 0)) {
                SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                            IF_PHYS_ADDRESS_STR, ERR_NOT_CREATED_ELEMENT_STR);
            }
        }
    }

    /* leaf speed */
    if (EXIT_SUCCESS == get_eth_if_speed(result_array, MAX_NUM_OF_INTERFACES, MAX_STR_LEN, interface_name)) {
        if (strncmp(result_array[0], "", MAX_STR_LEN)) {
            fill_xpath(path, INTERFACE_STATISTICS_SPEED_XPATH, interface_name);
            if (EXIT_FAILURE != translate_single_string_using_map(result_array[0], interface_speed_map)) {
                if (NULL == lyd_new_path(*parent, NULL, path, result_array[0], 0, 0)) {
                    SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                                IF_SPEED_STR, ERR_NOT_CREATED_ELEMENT_STR);
                }
            }
        }
    }

    /* container statistics */
    if (SR_ERR_OK != new_statistics_container(parent, interface_name)) {
        SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                    IF_STATISTICS_STR, ERR_NOT_CREATED_ELEMENT_STR);
    }


    /* bridge-port, augment from ieee802-dot1q-bridge yang module */
    if (EXIT_SUCCESS != get_bridge_port_names(result_array, MAX_NUM_OF_INTERFACES, MAX_STR_LEN)) {
        SRP_LOG_WRN(ERROR_MSG_MOD_FUNC_EL_MSG, Q_BRIDGE_MODULE_NAME, __func__,
                    Q_BRIDGE_BRIDGE_PORTS, ERR_NOT_CREATED_ELEMENT_STR);
        return SR_ERR_OPERATION_FAILED;
    }

    i = 0;
    while ((i < MAX_NUM_OF_INTERFACES) &&
           (0 != strncmp(result_array[i], "", MAX_STR_LEN)))
    {
        if ((NULL != interface_name) && (0 == strncmp(interface_name, result_array[i], MAX_STR_LEN))) {
            if (SR_ERR_OK != new_bridge_port_state_node(parent, interface_name)) {
                SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                            Q_BRIDGE_BRIDGE_PORTS, ERR_NOT_CREATED_ELEMENT_STR);
            }
        }
        i++;
    }

    /* ieee802-ethernet-interface module implementation */
    /* First check if ethernet auto-negotiation is supported.
     * If it is not, do not create ethernet container and objects. */
    if (EXIT_SUCCESS == get_eth_if_auto_neg_enable(objects, MAX_NUM_OF_INTERFACES, MAX_STR_LEN, interface_name)) {
        if (SR_ERR_OK != new_state_ethernet_container(parent, interface_name)) {
            SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                        ETH_ETHERNET_CONTAINER, ERR_NOT_CREATED_ELEMENT_STR);
        }
    }

    i = 0;
    /* ieee802-dot1q-sched module implementation */
    port = tsn_port_open_by_name(interface_name);
    if (NULL == port) {
        SRP_LOG_ERR(ERROR_MSG_MOD_FUNC_EL_MSG, PREE_MACMERGE_MODULE_NAME_STR, __func__,
                    interface_name, ERR_PORT_NOT_OPEN_STR);
        return SR_ERR_OPERATION_FAILED;
    }
    if (!tsn_is_feat_supported(port, FEAT_ST)) {
        SRP_LOG_INF(ERROR_MSG_MOD_FUNC_EL_MSG, SCHED_MODULE_NAME_STR, __func__,
                    ERR_SCHED_NOT_SUPPORTED_STR, interface_name);
        tsn_port_close(port);
    } else {
        tsn_port_close(port);
        /*  list max-sdu-table, augment from ieee802-dot1q-sched yang module.
         *  with only prio = 0
         */
        for (i = 0; i < num_tc; i++) {
            if (SR_ERR_OK != new_max_sdu_state_entry(parent, interface_name, prio, i)) {
                SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                            SCHED_MAX_SDU_TABLE_STR, ERR_NOT_CREATED_ELEMENT_STR);
            }
        }
        /* gate_parameters, augment from ieee802-dot1q-sched yang module */
        if (SR_ERR_OK != new_gate_parameters_state_container(parent, interface_name)) {
            SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                        SCHED_GATE_PARAMETERS_STR, ERR_NOT_CREATED_ELEMENT_STR);
        }
    }

    /* ieee802-dot1q-preemption module implementation */
    port = tsn_port_open_by_name(interface_name);
    if (NULL == port) {
        SRP_LOG_ERR(ERROR_MSG_MOD_FUNC_EL_MSG, PREEMPT_MODULE_NAME_STR, __func__,
                    interface_name, ERR_PORT_NOT_OPEN_STR);
        return SR_ERR_OPERATION_FAILED;
    }
    if (!tsn_is_feat_supported(port, FEAT_PREE)) {
        SRP_LOG_INF(ERROR_MSG_MOD_FUNC_EL_MSG, PREEMPT_MODULE_NAME_STR, __func__,
                    ERROR_PREEMPT_NOT_SUPPORTED_STR, interface_name);
        tsn_port_close(port);
    } else {
        tsn_port_close(port);
        if (SR_ERR_OK != new_state_preemption_container(parent, interface_name)) {
            SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                        PREEMPT_FRAME_PARAMS_STR, ERR_NOT_CREATED_ELEMENT_STR);
        }
    }

    /* ieee802-dot3br-preempt module implementation */
    port = tsn_port_open_by_name(interface_name);
    if (NULL == port) {
        SRP_LOG_ERR(ERROR_MSG_MOD_FUNC_EL_MSG, PREE_MACMERGE_MODULE_NAME_STR, __func__,
                    interface_name, ERR_PORT_NOT_OPEN_STR);
        return SR_ERR_OPERATION_FAILED;
    }
    if (!tsn_is_feat_supported(port, FEAT_MACMERGE)) {
        SRP_LOG_INF(ERROR_MSG_MOD_FUNC_EL_MSG, PREE_MACMERGE_MODULE_NAME_STR, __func__,
                    ERROR_PREE_MAC_MERGE_NOT_SUPPORTED_STR, interface_name);
        tsn_port_close(port);
    } else {
        tsn_port_close(port);
        if (SR_ERR_OK != new_state_mac_merge_container(parent, interface_name)) {
            SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                        PREE_MACMERGE_MAC_MERGE_STR, ERR_NOT_CREATED_ELEMENT_STR);
        }
    }

    /* augment ieee802-dot1cb-stream-identification, list
     * stream-identification-per-port-per-stream-counters and container
     * stream-identification-per-port-counters
     */
    if (SR_ERR_OK != sid_per_port_per_stream_counters(parent, interface_name)) {
        SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                    STREAM_ID_STREAM_IDENT_COUNTERS_STR, ERR_NOT_CREATED_ELEMENT_STR);
    }

    /* augment ieee802-dot1cb-frer, container
     * frame-replication-and-elimination-per-port-per-stream-counters
     */
    if (SR_ERR_OK != frame_replication_and_elimination_per_port_per_stream_counters(parent, interface_name)) {
        SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                    FRER_FRAME_REP_AND_EL_STREAM_COUNT_STR, ERR_NOT_CREATED_ELEMENT_STR);
    }

    return SR_ERR_OK;
}

/**
 * @brief Callback to be called when operational data of container frame
 * preemption parameters is requested.
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
 * @param[in]   private_data    Private context opaque to sysrepo, as passed
 *                              to sr_oper_get_items_subscribe call.
 * @return      Error code (SR_ERR_OK on success).
 */
static int interface_state_cb(sr_session_ctx_t *session,
                              const char *module_name, const char *path,
                              const char *request_xpath, uint32_t request_id,
                              struct lyd_node **parent, void *private_data)
{
    (void)session;
    (void)request_id;
    (void)module_name;
    (void)request_xpath;
    (void)request_id;
    (void)private_data;
    (void)path;

    char interface_names[MAX_NUM_OF_INTERFACES][MAX_STR_LEN] = {0};
    int i = 0;

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    if (EXIT_SUCCESS != get_interface_names(interface_names,
                                            MAX_NUM_OF_INTERFACES,
                                            MAX_STR_LEN)) {
        SRP_LOG_ERR(ERR_INTERFACE_NAMES_FAILED_STR, __func__);
        return SR_ERR_OPERATION_FAILED;
    }
    while ((i < MAX_NUM_OF_INTERFACES) &&
           (0 != strncmp(interface_names[i], "", MAX_STR_LEN))) {
        if (SR_ERR_OK != new_interface_node(parent, interface_names[i])) {
            SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                        IF_INTERFACE_LIST, ERR_NOT_CREATED_ELEMENT_STR);
        }
        i++;
    }

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
    char *path = NULL;

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    rc = config_change_helper_start();
    if (rc) {
        SRP_LOG_ERR(ERR_MODULE_INIT_FAILED_STR, IF_MODULE_NAME_STR,
                    sr_strerror(rc));
        return rc;
    }

    struct config_change_list *reset_list = config_change_list_new();
    if (!reset_list) {
        SRP_LOG_ERR(ERR_MODULE_INIT_FAILED_STR, IF_MODULE_NAME_STR,
                    sr_strerror(SR_ERR_NOMEM));
        config_change_helper_stop();
        return SR_ERR_NOMEM;
    }

    /* Set plugin_init to "1" so callbacks are triggered when startup
     * configuration is copied to running.
     */
    if (0 != sr_path_check_startup_done_file(INTERFACES_MODULE_NAME, &path)) {
        /* Startup datastore has already been copied to running datastore.
         * Apply config-change reset to both datastores
         * before subscriptions apply configuration to system.
         */
        rc = interface_sched_fix_datastore(session, SR_DS_STARTUP);
        if (SR_ERR_OK == rc)
            rc = interface_sched_fix_datastore(session, SR_DS_RUNNING);
        if (SR_ERR_OK != rc) {
            free(reset_list);
            config_change_helper_stop();
            return rc;
        }

        plugin_init = 1;
    }

    do {
        /* subscribe for /ietf-interfaces module changes */
        rc = sr_module_change_subscribe(session, IF_MODULE_NAME_STR, NULL,
                module_change_cb, NULL, 1, SR_SUBSCR_ENABLED, &subscription);
        if (SR_ERR_OK != rc) break;

        /* subscribe for /ietf-interfaces:interfaces/interface/enabled changes */
        rc = sr_module_change_subscribe(
                session, IF_MODULE_NAME_STR,
                "/ietf-interfaces:interfaces/interface/enabled",
                interface_enabled_cb, NULL, 2, SR_SUBSCR_ENABLED, &subscription);
        if (SR_ERR_OK != rc) break;

        /* subscribe as state data provider for the /ietf-interfaces:interfaces/interface  */
        rc = sr_oper_get_items_subscribe(
                session, IF_MODULE_NAME_STR,
                "/ietf-interfaces:interfaces/interface",
                interface_state_cb, NULL, SR_SUBSCR_DEFAULT , &subscription);
        if (SR_ERR_OK != rc) break;

        /* subscribe for /ietf-interfaces:interfaces/interface/ieee802-ethernet-interface:ethernet/speed changes */
        rc = sr_module_change_subscribe(
                session, IF_MODULE_NAME_STR,
                "/ietf-interfaces:interfaces/interface/ieee802-ethernet-interface:ethernet/speed",
                interface_ethernet_duplex_speed_cb, NULL, 7, SR_SUBSCR_UPDATE, &subscription);
        if (SR_ERR_OK != rc) break;

        /* subscribe for /ietf-interfaces:interfaces/interface/ieee802-ethernet-interface:ethernet/duplex changes */
        rc = sr_module_change_subscribe(
                session, IF_MODULE_NAME_STR,
                "/ietf-interfaces:interfaces/interface/ieee802-ethernet-interface:ethernet/duplex",
                interface_ethernet_duplex_speed_cb, NULL, 6, SR_SUBSCR_UPDATE, &subscription);
        if (SR_ERR_OK != rc) break;

        /* subscribe for /ietf-interfaces:interfaces/interface/ieee802-dot1q-preemption:frame-preemption-parameters/ieee802-dot1q-preemption:frame-preemption-status-table/ieee802-dot1q-preemption:frame-preemption-status changes */
        rc = sr_module_change_subscribe(
                session, IF_MODULE_NAME_STR,
                "/ietf-interfaces:interfaces/interface/ieee802-dot1q-preemption:frame-preemption-parameters/ieee802-dot1q-preemption:frame-preemption-status-table/ieee802-dot1q-preemption:frame-preemption-status",
                interface_frame_preemption_status_cb, NULL, 2, SR_SUBSCR_ENABLED, &subscription);
        if (SR_ERR_OK != rc) break;

        /* subscribe for /ietf-interfaces:interfaces/interface/ieee802-dot3br-preempt:mac-merge/ieee802-dot3br-preempt:admin-control/merge-enable-tx changes */
        rc = sr_module_change_subscribe(
                session, IF_MODULE_NAME_STR,
                "/ietf-interfaces:interfaces/interface/ieee802-dot3br-preempt:mac-merge/ieee802-dot3br-preempt:admin-control/merge-enable-tx",
                interface_admin_control_cb, NULL, 2, SR_SUBSCR_ENABLED, &subscription);
        if (SR_ERR_OK != rc) break;

        /* subscribe for /ietf-interfaces:interfaces/interface/ieee802-dot3br-preempt:mac-merge/ieee802-dot3br-preempt:admin-control/verify-disable-tx changes */
        rc = sr_module_change_subscribe(
                session, IF_MODULE_NAME_STR,
                "/ietf-interfaces:interfaces/interface/ieee802-dot3br-preempt:mac-merge/ieee802-dot3br-preempt:admin-control/verify-disable-tx",
                interface_admin_control_cb, NULL, 2, SR_SUBSCR_ENABLED, &subscription);
        if (SR_ERR_OK != rc) break;

        /* subscribe for /ietf-interfaces:interfaces/interface/ieee802-dot3br-preempt:mac-merge/ieee802-dot3br-preempt:admin-control/verify-time changes */
        rc = sr_module_change_subscribe(
                session, IF_MODULE_NAME_STR,
                "/ietf-interfaces:interfaces/interface/ieee802-dot3br-preempt:mac-merge/ieee802-dot3br-preempt:admin-control/verify-time",
                interface_admin_control_cb, NULL, 2, SR_SUBSCR_ENABLED, &subscription);
        if (SR_ERR_OK != rc) break;

        /* subscribe for /ietf-interfaces:interfaces/interface/ieee802-dot3br-preempt:mac-merge/ieee802-dot3br-preempt:admin-control/frag-size changes */
        rc = sr_module_change_subscribe(
                session, IF_MODULE_NAME_STR,
                "/ietf-interfaces:interfaces/interface/ieee802-dot3br-preempt:mac-merge/ieee802-dot3br-preempt:admin-control/frag-size",
                interface_admin_control_cb, NULL, 2, SR_SUBSCR_ENABLED, &subscription);
        if (SR_ERR_OK != rc) break;

        /* subscribe for /ietf-interfaces:interfaces/interface/ieee802-dot1q-sched:max-sdu-table/ieee802-dot1q-sched:queue-max-sdu changes */
        rc = sr_module_change_subscribe(
                session, IF_MODULE_NAME_STR,
                "/ietf-interfaces:interfaces/interface/ieee802-dot1q-sched:max-sdu-table/queue-max-sdu",
                interface_sched_elements_cb, NULL, 2, SR_SUBSCR_ENABLED, &subscription);
        if (SR_ERR_OK != rc) break;

        /* subscribe for /ietf-interfaces:interfaces/interface/ieee802-dot1q-sched:gate-parameters/ieee802-dot1q-sched:admin-control-list-length changes */
        rc = sr_module_change_subscribe(
                session, IF_MODULE_NAME_STR,
                "/ietf-interfaces:interfaces/interface/ieee802-dot1q-sched:gate-parameters/ieee802-dot1q-sched:admin-control-list-length",
                interface_sched_acl_length_cb, NULL, 7, SR_SUBSCR_ENABLED, &subscription);
        if (SR_ERR_OK != rc) break;

        /* subscribe for /ietf-interfaces:interfaces/interface/ieee802-dot1q-sched:gate-parameters/ieee802-dot1q-sched:admin-control-list/ieee802-dot1q-sched:operation-name changes */
        rc = sr_module_change_subscribe(
                session, IF_MODULE_NAME_STR,
                "/ietf-interfaces:interfaces/interface/ieee802-dot1q-sched:gate-parameters/ieee802-dot1q-sched:admin-control-list/ieee802-dot1q-sched:operation-name",
                interface_sched_acl_mod_cb, NULL, 5, SR_SUBSCR_ENABLED, &subscription);
        if (SR_ERR_OK != rc) break;

        /* subscribe for /ietf-interfaces:interfaces/interface/ieee802-dot1q-sched:gate-parameters/ieee802-dot1q-sched:admin-control-list/ieee802-dot1q-sched:sgs-params/gate-states-value */
        rc = sr_module_change_subscribe(
                session, IF_MODULE_NAME_STR,
                "/ietf-interfaces:interfaces/interface/ieee802-dot1q-sched:gate-parameters/ieee802-dot1q-sched:admin-control-list/ieee802-dot1q-sched:sgs-params/gate-states-value",
                interface_sched_acl_mod_cb, NULL, 5, SR_SUBSCR_ENABLED, &subscription);
        if (SR_ERR_OK != rc) break;

        /* subscribe for /ietf-interfaces:interfaces/interface/ieee802-dot1q-sched:gate-parameters/ieee802-dot1q-sched:admin-control-list/ieee802-dot1q-sched:sgs-params/gate-states-value */
        rc = sr_module_change_subscribe(
                session, IF_MODULE_NAME_STR,
                "/ietf-interfaces:interfaces/interface/ieee802-dot1q-sched:gate-parameters/ieee802-dot1q-sched:admin-control-list/ieee802-dot1q-sched:sgs-params/time-interval-value",
                interface_sched_acl_mod_cb, NULL, 5, SR_SUBSCR_ENABLED, &subscription);
        if (SR_ERR_OK != rc) break;

        /* subscribe for /ietf-interfaces:interfaces/interface/ieee802-dot1q-sched:gate-parameters/ieee802-dot1q-sched:admin-control-list/ieee802-dot1q-sched:shm-params changes */
        rc = sr_module_change_subscribe(
                session, IF_MODULE_NAME_STR,
                "/ietf-interfaces:interfaces/interface/ieee802-dot1q-sched:gate-parameters/ieee802-dot1q-sched:admin-control-list/ieee802-dot1q-sched:shm-params/gate-states-value",
                interface_sched_acl_mod_cb, NULL, 5, SR_SUBSCR_ENABLED, &subscription);
        if (SR_ERR_OK != rc) break;

        /* subscribe for /ietf-interfaces:interfaces/interface/ieee802-dot1q-sched:gate-parameters/ieee802-dot1q-sched:admin-control-list/ieee802-dot1q-sched:shm-params changes */
        rc = sr_module_change_subscribe(
                session, IF_MODULE_NAME_STR,
                "/ietf-interfaces:interfaces/interface/ieee802-dot1q-sched:gate-parameters/ieee802-dot1q-sched:admin-control-list/ieee802-dot1q-sched:shm-params/time-interval-value",
                interface_sched_acl_mod_cb, NULL, 5, SR_SUBSCR_ENABLED, &subscription);
        if (SR_ERR_OK != rc) break;

        /* subscribe for /ietf-interfaces:interfaces/interface/ieee802-dot1q-sched:gate-parameters/ieee802-dot1q-sched:admin-control-list/ieee802-dot1q-sched:srm-params changes */
        rc = sr_module_change_subscribe(
                session, IF_MODULE_NAME_STR,
                "/ietf-interfaces:interfaces/interface/ieee802-dot1q-sched:gate-parameters/ieee802-dot1q-sched:admin-control-list/ieee802-dot1q-sched:srm-params/gate-states-value",
                interface_sched_acl_mod_cb, NULL, 5, SR_SUBSCR_ENABLED, &subscription);
        if (SR_ERR_OK != rc) break;

        /* subscribe for /ietf-interfaces:interfaces/interface/ieee802-dot1q-sched:gate-parameters/ieee802-dot1q-sched:admin-control-list/ieee802-dot1q-sched:srm-params changes */
        rc = sr_module_change_subscribe(
                session, IF_MODULE_NAME_STR,
                "/ietf-interfaces:interfaces/interface/ieee802-dot1q-sched:gate-parameters/ieee802-dot1q-sched:admin-control-list/ieee802-dot1q-sched:srm-params/time-interval-value",
                interface_sched_acl_mod_cb, NULL, 5, SR_SUBSCR_ENABLED, &subscription);
        if (SR_ERR_OK != rc) break;

        /* subscribe for /ietf-interfaces:interfaces/interface/ieee802-dot1q-sched:gate-parameters/ieee802-dot1q-sched:admin-control-list changes */
        rc = sr_module_change_subscribe(
                session, IF_MODULE_NAME_STR,
                "/ietf-interfaces:interfaces/interface/ieee802-dot1q-sched:gate-parameters/ieee802-dot1q-sched:admin-control-list/index",
                interface_sched_acl_cb, NULL, 6, SR_SUBSCR_ENABLED, &subscription);
        if (SR_ERR_OK != rc) break;

        /* subscribe for /ietf-interfaces:interfaces/interface/ieee802-dot1q-sched:gate-parameters/ieee802-dot1q-sched:admin-base-time/ieee802-dot1q-sched:seconds changes */
        rc = sr_module_change_subscribe(
                session, IF_MODULE_NAME_STR,
                "/ietf-interfaces:interfaces/interface/ieee802-dot1q-sched:gate-parameters/admin-base-time/seconds",
                interface_sched_elements_cb, NULL, 3, SR_SUBSCR_ENABLED, &subscription);
        if (SR_ERR_OK != rc) break;

        /* subscribe for /ietf-interfaces:interfaces/interface/ieee802-dot1q-sched:gate-parameters/ieee802-dot1q-sched:admin-base-time/ieee802-dot1q-sched:fractional-seconds changes */
        rc = sr_module_change_subscribe(
                session, IF_MODULE_NAME_STR,
                "/ietf-interfaces:interfaces/interface/ieee802-dot1q-sched:gate-parameters/admin-base-time/fractional-seconds",
                interface_sched_elements_cb, NULL, 3, SR_SUBSCR_ENABLED, &subscription);
        if (SR_ERR_OK != rc) break;

        /* subscribe for /ietf-interfaces:interfaces/interface/ieee802-dot1q-sched:gate-parameters/ieee802-dot1q-sched:admin-cycle-time/ieee802-dot1q-sched:numerator changes */
        rc = sr_module_change_subscribe(
                session, IF_MODULE_NAME_STR,
                "/ietf-interfaces:interfaces/interface/ieee802-dot1q-sched:gate-parameters/admin-cycle-time/numerator",
                interface_sched_elements_cb, NULL, 3, SR_SUBSCR_ENABLED, &subscription);
        if (SR_ERR_OK != rc) break;

        /* subscribe for /ietf-interfaces:interfaces/interface/ieee802-dot1q-sched:gate-parameters/ieee802-dot1q-sched:admin-cycle-time/ieee802-dot1q-sched:denominator changes */
        rc = sr_module_change_subscribe(
                session, IF_MODULE_NAME_STR,
                "/ietf-interfaces:interfaces/interface/ieee802-dot1q-sched:gate-parameters/admin-cycle-time/denominator",
                interface_sched_elements_cb, NULL, 3, SR_SUBSCR_ENABLED, &subscription);
        if (SR_ERR_OK != rc) break;

        /* subscribe for /ietf-interfaces:interfaces/interface/ieee802-dot1q-sched:gate-parameters/ieee802-dot1q-sched:admin-cycle-time-extension changes */
        rc = sr_module_change_subscribe(
                session, IF_MODULE_NAME_STR,
                "/ietf-interfaces:interfaces/interface/ieee802-dot1q-sched:gate-parameters/admin-cycle-time-extension",
                interface_sched_elements_cb, NULL, 3, SR_SUBSCR_ENABLED, &subscription);
        if (SR_ERR_OK != rc) break;

        /* subscribe for /ietf-interfaces:interfaces/interface/ieee802-dot1q-sched:gate-parameters/ieee802-dot1q-sched:admin-gate-states changes */
        rc = sr_module_change_subscribe(
                session, IF_MODULE_NAME_STR,
                "/ietf-interfaces:interfaces/interface/ieee802-dot1q-sched:gate-parameters/admin-gate-states",
                interface_sched_elements_cb, NULL, 3, SR_SUBSCR_ENABLED, &subscription);
        if (SR_ERR_OK != rc) break;

        /* subscribe for /ietf-interfaces:interfaces/interface/ieee802-dot1q-sched:gate-parameters/ieee802-dot1q-sched:config-change changes */
        rc = sr_module_change_subscribe(
                session, IF_MODULE_NAME_STR,
                "/ietf-interfaces:interfaces/interface/ieee802-dot1q-sched:gate-parameters/config-change" "|"
                "/ietf-interfaces:interfaces/interface/ieee802-dot1q-sched:gate-parameters/gate-enabled",
                interface_sched_config_change_cb, reset_list, 2, SR_SUBSCR_ENABLED, &subscription);
        if (SR_ERR_OK != rc) break;

        /* subscribe for "/ietf-interfaces:interfaces/interface/ieee802-dot1q-bridge:bridge-port/ieee802-dot1q-bridge:port-type" changes */
        rc = sr_module_change_subscribe(
                session, IF_MODULE_NAME_STR,
                "/ietf-interfaces:interfaces/interface/ieee802-dot1q-bridge:bridge-port/ieee802-dot1q-bridge:port-type",
                bridge_port_port_type_cb, NULL, 2, SR_SUBSCR_ENABLED, &subscription);
        if (SR_ERR_OK != rc) break;

        /* subscribe for "/ietf-interfaces:interfaces/interface/ieee802-dot1q-bridge:bridge-port/ieee802-dot1q-bridge:component-name" changes */
        rc = sr_module_change_subscribe(
                session, IF_MODULE_NAME_STR,
                "/ietf-interfaces:interfaces/interface/ieee802-dot1q-bridge:bridge-port/ieee802-dot1q-bridge:component-name",
                mod_not_supported_cb, NULL, 2, SR_SUBSCR_ENABLED, &subscription);
        if (SR_ERR_OK != rc) break;

        /* subscribe for "/ietf-interfaces:interfaces/interface/ieee802-dot1q-bridge:bridge-port/ieee802-dot1q-bridge:acceptable-frame" changes */
        rc = sr_module_change_subscribe(
                session, IF_MODULE_NAME_STR,
                "/ietf-interfaces:interfaces/interface/ieee802-dot1q-bridge:bridge-port/ieee802-dot1q-bridge:acceptable-frame",
                mod_not_supported_cb, NULL, 2, SR_SUBSCR_ENABLED, &subscription);
        if (SR_ERR_OK != rc) break;

        /* subscribe for "/ietf-interfaces:interfaces/interface/ieee802-dot1q-bridge:bridge-port/ieee802-dot1q-bridge:enable-ingress-filtering" changes */
        rc = sr_module_change_subscribe(
                session, IF_MODULE_NAME_STR,
                "/ietf-interfaces:interfaces/interface/ieee802-dot1q-bridge:bridge-port/ieee802-dot1q-bridge:enable-ingress-filtering",
                mod_not_supported_cb, NULL, 2, SR_SUBSCR_ENABLED, &subscription);
        if (SR_ERR_OK != rc) break;

        /* subscribe for "/ietf-interfaces:interfaces/interface/ieee802-dot1q-bridge:bridge-port/ieee802-dot1q-bridge:pvid" changes */
        rc = sr_module_change_subscribe(
                session, IF_MODULE_NAME_STR,
                "/ietf-interfaces:interfaces/interface/ieee802-dot1q-bridge:bridge-port/ieee802-dot1q-bridge:pvid",
                bridge_port_change_cb, NULL, 2, SR_SUBSCR_ENABLED, &subscription);
        if (SR_ERR_OK != rc) break;

        /* subscribe for "/ietf-interfaces:interfaces/interface/ieee802-dot1q-bridge:bridge-port/ieee802-dot1q-bridge:default-priority" changes */
        rc = sr_module_change_subscribe(
                session, IF_MODULE_NAME_STR,
                "/ietf-interfaces:interfaces/interface/ieee802-dot1q-bridge:bridge-port/ieee802-dot1q-bridge:default-priority",
                bridge_port_change_cb, NULL, 2, SR_SUBSCR_ENABLED, &subscription);
        if (SR_ERR_OK != rc) break;
        /* container priority-regeneration */

        /* subscribe for "/ietf-interfaces:interfaces/interface/ieee802-dot1q-bridge:bridge-port/ieee802-dot1q-bridge:priority-regeneration/priority0" changes */
        rc = sr_module_change_subscribe(
                session, IF_MODULE_NAME_STR,
                "/ietf-interfaces:interfaces/interface/ieee802-dot1q-bridge:bridge-port/ieee802-dot1q-bridge:priority-regeneration/priority0",
                bridge_port_change_cb, NULL, 2, SR_SUBSCR_ENABLED, &subscription);
        if (SR_ERR_OK != rc) break;

        /* subscribe for "/ietf-interfaces:interfaces/interface/ieee802-dot1q-bridge:bridge-port/ieee802-dot1q-bridge:priority-regeneration/priority1" changes */
        rc = sr_module_change_subscribe(
                session, IF_MODULE_NAME_STR,
                "/ietf-interfaces:interfaces/interface/ieee802-dot1q-bridge:bridge-port/ieee802-dot1q-bridge:priority-regeneration/priority1",
                bridge_port_change_cb, NULL, 2, SR_SUBSCR_ENABLED, &subscription);
        if (SR_ERR_OK != rc) break;

        /* subscribe for "/ietf-interfaces:interfaces/interface/ieee802-dot1q-bridge:bridge-port/ieee802-dot1q-bridge:priority-regeneration/priority2" changes */
        rc = sr_module_change_subscribe(
                session, IF_MODULE_NAME_STR,
                "/ietf-interfaces:interfaces/interface/ieee802-dot1q-bridge:bridge-port/ieee802-dot1q-bridge:priority-regeneration/priority2",
                bridge_port_change_cb, NULL, 2, SR_SUBSCR_ENABLED, &subscription);
        if (SR_ERR_OK != rc) break;

        /* subscribe for "/ietf-interfaces:interfaces/interface/ieee802-dot1q-bridge:bridge-port/ieee802-dot1q-bridge:priority-regeneration/priority3" changes */
        rc = sr_module_change_subscribe(
                session, IF_MODULE_NAME_STR,
                "/ietf-interfaces:interfaces/interface/ieee802-dot1q-bridge:bridge-port/ieee802-dot1q-bridge:priority-regeneration/priority3",
                bridge_port_change_cb, NULL, 2, SR_SUBSCR_ENABLED, &subscription);
        if (SR_ERR_OK != rc) break;

        /* subscribe for "/ietf-interfaces:interfaces/interface/ieee802-dot1q-bridge:bridge-port/ieee802-dot1q-bridge:priority-regeneration/priority4" changes */
        rc = sr_module_change_subscribe(
                session, IF_MODULE_NAME_STR,
                "/ietf-interfaces:interfaces/interface/ieee802-dot1q-bridge:bridge-port/ieee802-dot1q-bridge:priority-regeneration/priority4",
                bridge_port_change_cb, NULL, 2, SR_SUBSCR_ENABLED, &subscription);
        if (SR_ERR_OK != rc) break;

        /* subscribe for "/ietf-interfaces:interfaces/interface/ieee802-dot1q-bridge:bridge-port/ieee802-dot1q-bridge:priority-regeneration/priorit5" changes */
        rc = sr_module_change_subscribe(
                session, IF_MODULE_NAME_STR,
                "/ietf-interfaces:interfaces/interface/ieee802-dot1q-bridge:bridge-port/ieee802-dot1q-bridge:priority-regeneration/priority5",
                bridge_port_change_cb, NULL, 2, SR_SUBSCR_ENABLED, &subscription);
        if (SR_ERR_OK != rc) break;

        /* subscribe for "/ietf-interfaces:interfaces/interface/ieee802-dot1q-bridge:bridge-port/ieee802-dot1q-bridge:priority-regeneration/priority6" changes */
        rc = sr_module_change_subscribe(
                session, IF_MODULE_NAME_STR,
                "/ietf-interfaces:interfaces/interface/ieee802-dot1q-bridge:bridge-port/ieee802-dot1q-bridge:priority-regeneration/priority6",
                bridge_port_change_cb, NULL, 2, SR_SUBSCR_ENABLED, &subscription);
        if (SR_ERR_OK != rc) break;

        /* subscribe for "/ietf-interfaces:interfaces/interface/ieee802-dot1q-bridge:bridge-port/ieee802-dot1q-bridge:priority-regeneration/priority7" changes */
        rc = sr_module_change_subscribe(
                session, IF_MODULE_NAME_STR,
                "/ietf-interfaces:interfaces/interface/ieee802-dot1q-bridge:bridge-port/ieee802-dot1q-bridge:priority-regeneration/priority7",
                bridge_port_change_cb, NULL, 2, SR_SUBSCR_ENABLED, &subscription);
        if (SR_ERR_OK != rc) break;

        /* container traffic-class */

        /* subscribe for "/ietf-interfaces:interfaces/interface/ieee802-dot1q-bridge:bridge-port/ieee802-dot1q-bridge:traffic-class/priority0" changes */
        rc = sr_module_change_subscribe(
                session, IF_MODULE_NAME_STR,
                "/ietf-interfaces:interfaces/interface/ieee802-dot1q-bridge:bridge-port/ieee802-dot1q-bridge:traffic-class/priority0",
                bridge_port_change_cb, NULL, 2, SR_SUBSCR_ENABLED, &subscription);
        if (SR_ERR_OK != rc) break;

        /* subscribe for "/ietf-interfaces:interfaces/interface/ieee802-dot1q-bridge:bridge-port/ieee802-dot1q-bridge:traffic-class/priority1" changes */
        rc = sr_module_change_subscribe(
                session, IF_MODULE_NAME_STR,
                "/ietf-interfaces:interfaces/interface/ieee802-dot1q-bridge:bridge-port/ieee802-dot1q-bridge:traffic-class/priority1",
                bridge_port_change_cb, NULL, 2, SR_SUBSCR_ENABLED, &subscription);
        if (SR_ERR_OK != rc) break;

        /* subscribe for "/ietf-interfaces:interfaces/interface/ieee802-dot1q-bridge:bridge-port/ieee802-dot1q-bridge:traffic-class/priority2" changes */
        rc = sr_module_change_subscribe(
                session, IF_MODULE_NAME_STR,
                "/ietf-interfaces:interfaces/interface/ieee802-dot1q-bridge:bridge-port/ieee802-dot1q-bridge:traffic-class/priority2",
                bridge_port_change_cb, NULL, 2, SR_SUBSCR_ENABLED, &subscription);
        if (SR_ERR_OK != rc) break;

        /* subscribe for "/ietf-interfaces:interfaces/interface/ieee802-dot1q-bridge:bridge-port/ieee802-dot1q-bridge:traffic-class/priority3" changes */
        rc = sr_module_change_subscribe(
                session, IF_MODULE_NAME_STR,
                "/ietf-interfaces:interfaces/interface/ieee802-dot1q-bridge:bridge-port/ieee802-dot1q-bridge:traffic-class/priority3",
                bridge_port_change_cb, NULL, 2, SR_SUBSCR_ENABLED, &subscription);
        if (SR_ERR_OK != rc) break;

        /* subscribe for "/ietf-interfaces:interfaces/interface/ieee802-dot1q-bridge:bridge-port/ieee802-dot1q-bridge:traffic-class/priority4" changes */
        rc = sr_module_change_subscribe(
                session, IF_MODULE_NAME_STR,
                "/ietf-interfaces:interfaces/interface/ieee802-dot1q-bridge:bridge-port/ieee802-dot1q-bridge:traffic-class/priority4",
                bridge_port_change_cb, NULL, 2, SR_SUBSCR_ENABLED, &subscription);
        if (SR_ERR_OK != rc) break;

        /* subscribe for "/ietf-interfaces:interfaces/interface/ieee802-dot1q-bridge:bridge-port/ieee802-dot1q-bridge:traffic-class/priority5" changes */
        rc = sr_module_change_subscribe(
                session, IF_MODULE_NAME_STR,
                "/ietf-interfaces:interfaces/interface/ieee802-dot1q-bridge:bridge-port/ieee802-dot1q-bridge:traffic-class/priority5",
                bridge_port_change_cb, NULL, 2, SR_SUBSCR_ENABLED, &subscription);
        if (SR_ERR_OK != rc) break;

        /* subscribe for "/ietf-interfaces:interfaces/interface/ieee802-dot1q-bridge:bridge-port/ieee802-dot1q-bridge:traffic-class/priority6" changes */
        rc = sr_module_change_subscribe(
                session, IF_MODULE_NAME_STR,
                "/ietf-interfaces:interfaces/interface/ieee802-dot1q-bridge:bridge-port/ieee802-dot1q-bridge:traffic-class/priority6",
                bridge_port_change_cb, NULL, 2, SR_SUBSCR_ENABLED, &subscription);
        if (SR_ERR_OK != rc) break;

        /* subscribe for "/ietf-interfaces:interfaces/interface/ieee802-dot1q-bridge:bridge-port/ieee802-dot1q-bridge:traffic-class/priority7" changes */
        rc = sr_module_change_subscribe(
                session, IF_MODULE_NAME_STR,
                "/ietf-interfaces:interfaces/interface/ieee802-dot1q-bridge:bridge-port/ieee802-dot1q-bridge:traffic-class/priority7",
                bridge_port_change_cb, NULL, 2, SR_SUBSCR_ENABLED, &subscription);
        if (SR_ERR_OK != rc) break;

    } while (0);

    if (SR_ERR_OK != rc) {
        SRP_LOG_ERR(ERR_MODULE_INIT_FAILED_STR, IF_MODULE_NAME_STR,
                    sr_strerror(rc));
        sr_unsubscribe(subscription);
        config_change_helper_stop();
        free(reset_list);
        return rc;
    }

    if (EXIT_FAILURE == interfaces_fill_startup_datastore(session)) {
        SRP_LOG_ERR(ERROR_MSG_MOD_FUNC_MSG, IF_MODULE_NAME_STR, __func__,
                    ERR_MSG_EMPTY_STARTUP_DS_STR);
        config_change_helper_stop();
        free(reset_list);
        return SR_ERR_OPERATION_FAILED;
    }

    *private_data = reset_list;

    plugin_init = 1;

    SRP_LOG_INF(INF_MODULE_INIT_SUCCESS_STR, IF_MODULE_NAME_STR);

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

    sr_unsubscribe(subscription);
    config_change_helper_stop();
    free(private_data);
    SRP_LOG_INF(INF_MODULE_CLEANUP_STR, IF_MODULE_NAME_STR);
}
