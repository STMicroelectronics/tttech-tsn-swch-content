/*
 * TTTech ieee802-dot1q-fqtss-yang-module
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
/* module specific includes */
#include "fqtss_defines.h"
/* sysrepo includes */
#include <sysrepo.h>
#include <sysrepo/values.h>
#include <sysrepo/xpath.h>
/* libbase includes */
#include "libbase/base_prints.h"
#include "libbase/base_linux_tool_helpers.h"
#include "libbase/base_q_bridge_linux_tools.h"
#include "libbase/base_defines.h"
/* libtsn includes */
#include "libtsn/tsn.h"
#include "libtsn/tsn_fqtss.h"
#include "libtsn/tsn_bridge.h" // for tsn_bridge_get_num_traffic_classes
#include "libtsn/tsn_error.h"

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
 * @param[in]   session         Automatically-created session that can be used
 *                              for obtaining changed data (e.g. by
 *                              sr_get_changes_iter call ot sr_get_item -like
 *                              calls). Do not stop this session.
 * @param[in]   module_name     Name of the module where the change has
 *                              occurred.
 * @param[in]   event           Type of the notification event that has
 *                              occurred.
 * @param[in]   private_ctx     Private context opaque to sysrepo, as passed to
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
}

/**
 * @brief Callback to be called by the event of changing content of node with
 * "/ieee802-dot1q-fqtss:ieee802-dot1q-fqtss/ieee8021FqtssBapTable/ieee8021FqtssBapEntry/ieee8021FqtssAdminIdleSlopeLs"
 * xpath.
 * "/ieee802-dot1q-fqtss:ieee802-dot1q-fqtss/ieee8021FqtssTxSelectionAlgorithmTable/ieee8021FqtssTxSelectionAlgorithmEntry/ieee8021FqtssTxSelectionAlgorithmID"
 * xpath.
 *
 * Subscribe to it by sr_module_change_subscribe call.
 *
 * @param[in]   session         Automatically-created session that can be used
 *                              for obtaining changed data (e.g. by
 *                              sr_get_changes_iter call or sr_get_item -like
 *                              calls). Do not stop this session.
 * @param[in]   xpath           Data Path of the subtree where the change has
 *                              occurred.
 * @param[in]   event           Type of the notification event that has
 *                              occurred.
 * @param[in]   private_ctx     Private context opaque to sysrepo, as passed to
 *                              sr_module_change_subscribe call.
 * @return      Error code (SR_ERR_OK on success).
 */
static int fqtss_element_change_cb(sr_session_ctx_t *session,
                                   const char *module_name, const char *xpath,
                                   sr_event_t event, uint32_t request_id,
                                   void *private_data)
{
    (void)module_name;
    (void)event;
    (void)private_data;
    (void)request_id;

    int rc = SR_ERR_OK;
    tsn_return_value tsn_ret_val = E_TSN_LIB_NO_ERROR;
    sr_val_t *old_value = NULL;
    sr_val_t *new_value = NULL;
    sr_change_oper_t op;
    sr_change_iter_t *iter = NULL;
    sr_xpath_ctx_t state = {0};
    char port_name[MAX_STR_LEN] = "";
    char tc_str[MAX_STR_LEN] = "";
    uint8_t tc_val = 0;
    struct port_hdl *port = NULL;
    bool slope_ms = true;
    uint64_t idle_slope = 0u;
    uint8_t tis = 0;
    uint64_t tmp = 0u;
    /* table_id=0 will be used for ieee8021FqtssBapTable,
     * table_id=1 will be used for ieee8021FqtssTxSelectionAlgorithmTable
     */
    int table_id = 0;
    char entry_name[MAX_STR_LEN] = FQTSS_BAP_ENTRY_STR;
    char key_name[MAX_STR_LEN] = FQTSS_BAP_TRAFFIC_CLASS_STR;

    SRP_LOG_DBG(DBG_FUNCTION_CALL_MSG_STR, __func__, xpath);

    if ((SR_EV_DONE == event) || (0 == plugin_init)) {
        SRP_LOG_DBG(DEBUG_MSG_WITH_TWO_PARAM, DBG_APPLYING_CHANGES_MSG, __func__);
        return SR_ERR_OK;
    }

    /* check object xpath.
     * if object is ieee8021FqtssTxSelectionAlgorithmID, set table_id to '1',
     * else it is table_id=0 which is defined for ieee8021FqtssBapTable objects */
    if (sr_xpath_node_name_eq(xpath, FQTSS_TX_SELECTION_ALGORITHM_ID_STR)) {
        table_id = 1;
        snprintf(entry_name, MAX_STR_LEN, "%s", FQTSS_TX_SELECTION_ALGORITHM_ENTRY_STR);
        snprintf(key_name, MAX_STR_LEN, "%s", FQTSS_TRAFFIC_CLASS_STR);

    }

    rc = sr_get_changes_iter(session, (char*)xpath, &iter);
    if (SR_ERR_OK != rc) {
        SRP_LOG_ERR(ERR_SR_ERROR_MSG_STR, xpath, sr_strerror(rc));
        sr_set_error(session, xpath, ERR_FORMING_ITERATOR_FAILED_XPATH_STR, xpath);
        return rc;
    }

    while (SR_ERR_NOT_FOUND != sr_get_change_next(session, iter, &op,
                                                  &old_value, &new_value)) {
        if (op == SR_OP_DELETED) {
            sr_set_error(session, xpath, ERR_REM_OP_NOT_SUPPORTED_XPATH_STR, xpath);
            return SR_ERR_UNSUPPORTED;
        }

        if ((op == SR_OP_CREATED) || (op == SR_OP_MODIFIED)) {
            /* find port name and bap traffic class to open port and get oper
             * idle slope value for current port and traffic class
             */
            if (EXIT_SUCCESS != get_key_value(session, new_value->xpath,
                                              entry_name, FQTSS_PORT_NAME_STR,
                                              &state, port_name)) {
                return SR_ERR_OPERATION_FAILED;
            }
            if (0 == strncmp(port_name, "", MAX_STR_LEN)) {
                sr_set_error(session, new_value->xpath,
                             ERR_KEY_VALUE_FAILED_XPATH_STR, new_value->xpath);
                return SR_ERR_NOT_FOUND;
            }

            port = tsn_port_open_by_name(port_name);
            if (NULL == port) {
                sr_set_error(session, xpath, ERR_PORT_NOT_OPEN_XPATH_STR, xpath);
                return SR_ERR_INTERNAL;
            }

            if (EXIT_SUCCESS != get_key_value(session, new_value->xpath,
                                              entry_name, key_name,
                                              &state, tc_str)) {
                return SR_ERR_OPERATION_FAILED;
            }

            tc_val = (uint8_t)strtoul(tc_str, NULL, 10);

            switch (table_id) {
            case 0: // ieee8021FqtssBapTable
                /* if slope_ms is 'true' that means the current node is
                 * "ieee8021FqtssAdminIdleSlopeMs", else the node is
                 * "ieee8021FqtssAdminIdleSlopeLs"
                 */
                slope_ms = sr_xpath_node_name_eq(new_value->xpath, FQTSS_ADMIN_SLOPE_MS_STR);

                /* If sysrepo-plugind is booting up (applying startup
                 * datastore), and if changed value is SlopeMs and it has 0
                 * value, then idle_slope will be 1599975585 and set function
                 * tsn_fqtss_set_admin_slope_table will fail. So we need this
                 * check, which is the same as in filling up startup datastore,
                 * to set idle_slope to 0, so set function for SlopeMs can
                 * pass.
                 */
                if (E_TSN_LIB_NO_ERROR != (tsn_ret_val = tsn_fqtss_get_tx_selection_table(port, tc_val, &tis))) {
                    sr_set_error(session, new_value->xpath,
                                 ERR_GETTER_EXT_FMT_STR, FQTSS_BAP_TABLE_STR,
                                 LIT_ENTRY_STR, LIT_XPATH_STR, new_value->xpath,
                                 tsn_lib_get_known_ernno_msg(tsn_ret_val));
                    tsn_port_close(port);
                    return SR_ERR_INTERNAL;
                }

                if (E_TSN_LIB_NO_ERROR != (tsn_ret_val = tsn_fqtss_get_admin_slope_table(port, tc_val, &idle_slope))) {
                    sr_set_error(session, new_value->xpath,
                                 ERR_GETTER_EXT_FMT_STR,
                                 FQTSS_TX_SELECTION_ALGORITHM_TABLE_STR,
                                 LIT_ENTRY_STR, LIT_XPATH_STR,
                                 new_value->xpath,
                                 tsn_lib_get_known_ernno_msg(tsn_ret_val));
                    tsn_port_close(port);
                    return SR_ERR_INTERNAL;
                }

                if (0 == tis) {
                    /* this means that oper slope is n/a, so set idle_slope to
                     * zero to prevent tsn_fqtss_set_admin_slope_table function
                     * to fail
                     */
                    idle_slope = 0;
                }

                if (slope_ms) {
                    idle_slope = idle_slope & 0x00000000FFFFFFFF;
                    tmp = new_value->data.uint32_val;
                    tmp = tmp << 32;
                    idle_slope = idle_slope | tmp;
                } else {
                    idle_slope = idle_slope & 0xFFFFFFFF00000000;
                    tmp = new_value->data.uint32_val;
                    idle_slope = idle_slope | tmp;
                }

                if (E_TSN_LIB_NO_ERROR != (tsn_ret_val = tsn_fqtss_set_admin_slope_table(port, tc_val, idle_slope))) {
                    sr_set_error(session, new_value->xpath,
                                 ERR_SETTER_EXT_FMT_STR, FQTSS_BAP_TABLE_STR,
                                 LIT_ENTRY_STR, LIT_XPATH_STR, new_value->xpath,
                                 tsn_lib_get_known_ernno_msg(tsn_ret_val));
                    tsn_port_close(port);
                    return SR_ERR_INTERNAL;
                }
                break;
            case 1: // ieee8021FqtssTxSelectionAlgorithmTable
                if (E_TSN_LIB_NO_ERROR != (tsn_ret_val = tsn_fqtss_set_tx_selection_table(port, tc_val, (uint8_t)new_value->data.uint32_val))) {
                    sr_set_error(session, new_value->xpath,
                                 ERR_SETTER_EXT_FMT_STR,
                                 FQTSS_TX_SELECTION_ALGORITHM_TABLE_STR,
                                 LIT_ENTRY_STR, LIT_XPATH_STR, new_value->xpath,
                                 tsn_lib_get_known_ernno_msg(tsn_ret_val));
                    tsn_port_close(port);
                    return SR_ERR_INTERNAL;
                }
                break;
            }
            tsn_port_close(port);
        }

    }
    sr_free_change_iter(iter);

    return rc;
}

/**
 * @brief Callback to be called when operational data of list
 * ieee8021FqtssBapEntry is requested, more precisely when leaf
 * ieee8021FqtssOperIdleSlopeLs is requested.
 *
 * Subscribe to it by sr_dp_get_items_subscribe call.
 *
 * @param[in]   xpath           Data Path identifying the level under which the
 *                              nodes are requested.
 * @param[out]  values          Array of values at the selected level
 *                              (allocated by the provider).
 * @param[out]  values_cnt      Number of values returned.
 * @param[in]   request_id      An ID identifying the originating request.
 * @param[in]   original_xpath  The xpath that was asked for in the originating
 *                              request.
 * @param[in]   private_ctx     Private context opaque to sysrepo, as passed to
 *                              sr_module_change_subscribe call.
 * @return      Error code (SR_ERR_OK on success).
 */
static int fqtss_state_cb(sr_session_ctx_t *session, const char *module_name,
                          const char *path, const char *request_xpath,
                          uint32_t request_id, struct lyd_node **parent,
                          void *private_data)
{
    (void)session;
    (void)module_name;
    (void)path;
    (void)request_xpath;
    (void)request_id;
    (void)private_data;

    char bridge_names[MAX_NUM_OF_INTERFACES][MAX_STR_LEN] = {0};
    char port_names[MAX_NUM_OF_INTERFACES][MAX_STR_LEN] = {0};
    char tmp_xpath[LONG_STR_LEN] = "";
    char tmp_value[MAX_STR_LEN] = "";
    struct port_hdl *port = NULL;
    uint8_t tis = 0;
    uint32_t tc_num = 0;
    uint32_t idle_slope_ms = 0u;
    uint32_t idle_slope_ls = 0u;
    uint64_t idle_slope = 0u;
    unsigned int i = 0;
    int j = 0;

    SRP_LOG_DBG(DBG_FUNCTION_CALL_MSG_STR, __func__, path);

    /* get bridge names */
    if (EXIT_FAILURE == get_bridge_names(bridge_names, MAX_NUM_OF_INTERFACES,
                                         MAX_STR_LEN)) {
        SRP_LOG_ERR(ERROR_MSG_MOD_FUNC_MSG, FQTSS_NAME_STR, __func__,
                    BRIDGE_NAMES_FAILED_STR);
        return SR_ERR_OPERATION_FAILED;
    }
    // if system supports more than one bridge print out error message
    if ((0 != strncmp(bridge_names[0], "", MAX_STR_LEN)) &&
        (0 != strncmp(bridge_names[1], "", MAX_STR_LEN))) {
        SRP_LOG_ERR(ERROR_MSG_FUN_AND_MSG, __func__, ERR_MSG_MORE_BRIDGES);
        return SR_ERR_OPERATION_FAILED;
    }

    if (EXIT_FAILURE == get_bridge_port_names(port_names, MAX_NUM_OF_INTERFACES,
                                              MAX_STR_LEN)) {
        SRP_LOG_ERR(ERROR_MSG_MOD_FUNC_MSG, FQTSS_NAME_STR, __func__,
                    BRIDGE_PORT_NAMES_FAILED_STR);
        return EXIT_FAILURE;
    }

    while ((j < MAX_NUM_OF_INTERFACES) &&
           (0 != strncmp(port_names[j], "", MAX_STR_LEN))) {
        port = tsn_port_open_by_name(port_names[j]);
        if (NULL == port) {
            SRP_LOG_ERR(ERROR_MSG_MOD_FUNC_EL_MSG, FQTSS_NAME_STR, __func__,
                        port_names[j], ERR_PORT_NOT_OPEN_STR);
            return EXIT_FAILURE;
        }

        if (0 != tsn_bridge_get_num_traffic_classes(port, &tc_num)) {
             tsn_port_close(port);
             SRP_LOG_ERR(ERROR_MSG_FUN_AND_MSG, __func__,
                         "number of traffic classes is unreachable");
             return EXIT_FAILURE;
        }

        for (i = 0; i < tc_num; i++) {
            if (0 == tsn_fqtss_get_oper_slope_table(port, i, &idle_slope)) {
                if (tsn_fqtss_get_tx_selection_table(port, i, &tis)) {
                     tsn_port_close(port);
                     SRP_LOG_ERR(ERROR_MSG_FUN_AND_MSG, __func__,
                                 "tis number is unreachable");
                     return EXIT_FAILURE;
                }

                if (0 == tis) {
                    /* this means that oper slope is n/a, so set idle_slope to
                     * zero to prevent some trash value
                     */
                    idle_slope = 0u;
                }

                /* idle_slope needs to be converted in proper values for idle slope MS and LS */
                idle_slope_ls = idle_slope & 0xFFFFFFFF;
                idle_slope_ms = (idle_slope & 0xFFFFFFFF00000000) >> 32;

                /* list ieee8021FqtssBapEntry, ieee8021FqtssOperIdleSlopeMs */
                fill_xpath(tmp_xpath, FQTSS_BAP_TABLE_ENTRY_XPATH, BASE_ONE_STR,
                           port_names[j], i, FQTSS_OPER_SLOPE_MS_STR);
                snprintf(tmp_value, MAX_STR_LEN, "%u", idle_slope_ms);
                if (NULL == lyd_new_path(*parent, NULL, tmp_xpath, tmp_value, 0, 0)) {
                    SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                                FQTSS_OPER_SLOPE_MS_STR, ERR_NOT_CREATED_ELEMENT_STR);
                }

                /* list ieee8021FqtssBapEntry, ieee8021FqtssOperIdleSlopeLs */
                fill_xpath(tmp_xpath, FQTSS_BAP_TABLE_ENTRY_XPATH, BASE_ONE_STR,
                           port_names[j], i, FQTSS_OPER_SLOPE_LS_STR);
                snprintf(tmp_value, MAX_STR_LEN, "%u", idle_slope_ls);
                if (NULL == lyd_new_path(*parent, NULL, tmp_xpath, tmp_value, 0, 0)) {
                    SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                                FQTSS_OPER_SLOPE_LS_STR, ERR_NOT_CREATED_ELEMENT_STR);
                }
            }
        }
        tsn_port_close(port);
        j++;
    }

    return SR_ERR_OK;
}

/**
 * @brief This function creates the XML elements related to idle slopes from
 * ieee802-dot1q-fqtss yang module.
 *
 * @param[out]  parent          Pointer to struct lyd_node which is going to be
 *                              parent for created element.
 * @param[in]   module          Main schema node structure representing YANG
 *                              module.
 * @param[in]   node_name       Contains the element name.
 * @param[in]   value           Contains the value of the element.
 * @param[in]   slope_ms        Contains data which represents is it master or
 *                              slave idle slope.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
static int create_idle_slope_node(struct lyd_node *parent,
                                  const struct lys_module *module,
                                  char *node_name, uint64_t value, int slope_ms)
{
    uint32_t idle_slope_tmp = 0u;
    char tmp[MAX_STR_LEN] = "";

    if (!slope_ms) {
        idle_slope_tmp = (value & 0xFFFFFFFF00000000) >> 32;
    } else {
        idle_slope_tmp = value & 0xFFFFFFFF;
    }

    snprintf(tmp, MAX_STR_LEN, "%u", idle_slope_tmp);
    if (EXIT_FAILURE == new_node(parent, module, node_name, tmp)) {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

/**
 * @brief Creates all entries for list ieee8021FqtssBapEntry and
 * ieee8021FqtssTxSelectionAlgorithmEntry from ieee802-dot1q-fqtss yang module.
 *
 * @param[out]  parent          Pointer to struct lyd_node which is going to be
 *                              parent to created list.
 * @param[in]   module          Main schema node structure representing YANG
 *                              module.
 * @param[in]   port_name       Name of the port.
 * @param[in]   index           Traffic class or priority index.
 * @param[in]   table_id        Table identifier. '0' for
 *                              ieee8021FqtssBapTable, '1' for
 *                              ieee8021FqtssTxSelectionAlgorithmTable
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
static int new_fqtss_entry(struct lyd_node *parent,
                           const struct lys_module *module, char *port_name,
                           int index, int table_id)
{
    static struct lyd_node *entry = NULL;
    char tmp_value[MAX_STR_LEN] = "";
    struct port_hdl *port = NULL;
    unsigned int tmp_index = 0;
    uint64_t idle_slope = 0;
    uint8_t tis = 0;
    char idle_slope_names[BASE_TWO][MAX_STR_LEN] = {0};
    char entry_name[MAX_STR_LEN] = "";
    char key_name[MAX_STR_LEN] = "";

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    switch (table_id) {
    case 0: // 0
        snprintf(entry_name, MAX_STR_LEN, "%s", FQTSS_BAP_ENTRY_STR);
        snprintf(key_name, MAX_STR_LEN, "%s", FQTSS_BAP_TRAFFIC_CLASS_STR);
        break;
    case 1: // ieee8021FqtssTxSelectionAlgorithmTable
        snprintf(entry_name, MAX_STR_LEN, "%s", FQTSS_TX_SELECTION_ALGORITHM_ENTRY_STR);
        snprintf(key_name, MAX_STR_LEN, "%s", FQTSS_TRAFFIC_CLASS_STR);
        break;
    }

    if (EXIT_FAILURE == new_container(parent, &entry, module, entry_name)) {
        return EXIT_FAILURE;
    }

     /* list ieee8021FqtssBapEntry, create all keys */
    if (EXIT_FAILURE == new_node(entry, module, FQTSS_BRIDGE_ID_STR, BASE_ONE_STR)) {
        return EXIT_FAILURE;
    }

    if (EXIT_FAILURE == new_node(entry, module, FQTSS_PORT_NAME_STR, port_name)) {
        return EXIT_FAILURE;
    }

    snprintf(tmp_value, MAX_STR_LEN, "%d", index);
    if (EXIT_FAILURE == new_node(entry, module, key_name, tmp_value)) {
        return EXIT_FAILURE;
    }

     /* tsn_port_open_by_name */
    port = tsn_port_open_by_name(port_name);
    if (NULL == port) {
        SRP_LOG_ERR(ERROR_MSG_MOD_FUNC_EL_MSG, FQTSS_NAME_STR, __func__,
                    port_name, ERR_PORT_NOT_OPEN_STR);
        return EXIT_FAILURE;
    }

    tmp_index = (unsigned int)index;

    switch (table_id) {
        case 0: // ieee8021FqtssBapTable
            snprintf(idle_slope_names[0], MAX_STR_LEN, "%s", FQTSS_ADMIN_SLOPE_MS_STR);
            snprintf(idle_slope_names[1], MAX_STR_LEN, "%s", FQTSS_ADMIN_SLOPE_LS_STR);
            if ((0 != tsn_fqtss_get_tx_selection_table(port, tmp_index, &tis)) ||
                    (0 != tsn_fqtss_get_admin_slope_table(port, tmp_index, &idle_slope)))
            {
                SRP_LOG_ERR(ERROR_MSG_FUN_XML_TWO_EL_AND_MSG, __func__,
                            FQTSS_ADMIN_SLOPE_MS_STR, FQTSS_ADMIN_SLOPE_LS_STR, ERR_NOT_CREATED_ELEMENT_STR);
                tsn_port_close(port);
                return EXIT_FAILURE;
            }
            if (0 == tis) {
                // this means that oper slope is n/a, so set idle_slope to zero to prevent some trash value
                idle_slope = 0;
            }
            for (int i = 0; i < BASE_TWO; i++) {
                if (EXIT_FAILURE == create_idle_slope_node(entry, module,
                                                           idle_slope_names[i],
                                                           idle_slope, i)) {
                    tsn_port_close(port);
                    return EXIT_FAILURE;
                }
            }
            break;
        case 1: // ieee8021FqtssTxSelectionAlgorithmTable
            if (0 != tsn_fqtss_get_tx_selection_table(port, tmp_index, &tis)) {
                SRP_LOG_ERR(ERROR_MSG_FUN_XML_EL_AND_MSG, __func__,
                            FQTSS_TX_SELECTION_ALGORITHM_ID_STR,
                            "tsn_fqtss_get_tx_selection_table is failed");
                tsn_port_close(port);
                return EXIT_FAILURE;
            }

            snprintf(tmp_value, MAX_STR_LEN, "%u", tis);
            if (EXIT_FAILURE == new_node(entry, module,
                                         FQTSS_TX_SELECTION_ALGORITHM_ID_STR,
                                         tmp_value)) {
                tsn_port_close(port);
                return EXIT_FAILURE;
            }
            break;
    }

    tsn_port_close(port);
    return EXIT_SUCCESS;
}

/**
 * @brief Creates all entries for list ieee8021FqtssBapEntry and
 * ieee8021FqtssTxSelectionAlgorithmEntry from ieee802-dot1q-fqtss yang module.
 *
 * @param[out]  root            Pointer to struct lyd_node which is going to be
 *                              parent to created list.
 * @param[in]   module          Main schema node structure representing YANG
 *                              module.
 * @param[in]   container_name  Name of the container that is going to be
 *                              created.
 * @param[in]   bridge_name     Bridge name.
 * @param[in]   table_id        Table identifier. '0' for
 *                              ieee8021FqtssBapTable, '1' for
 *                              ieee8021FqtssTxSelectionAlgorithmTable
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
static int new_fqtss_table(struct lyd_node *root,
                           const struct lys_module *module,
                           char *container_name, char *bridge_name,
                           int table_id)
{
    (void)bridge_name;

    struct lyd_node *table = NULL;
    char port_names[MAX_NUM_OF_INTERFACES][MAX_STR_LEN] = {0};
    uint32_t tc_num = 0;
    int i = 0;
    int j = 0;
    struct port_hdl *port = NULL;

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    if (EXIT_FAILURE == new_container(root, &table, module, container_name)) {
        return EXIT_FAILURE;
    }

    if (EXIT_FAILURE == get_bridge_port_names(port_names, MAX_NUM_OF_INTERFACES,
                                              MAX_STR_LEN)) {
        SRP_LOG_ERR(ERROR_MSG_MOD_FUNC_MSG, FQTSS_NAME_STR, __func__,
                    BRIDGE_PORT_NAMES_FAILED_STR);
        return EXIT_FAILURE;
    }

    while ((j < MAX_NUM_OF_INTERFACES) &&
           (0 != strncmp(port_names[j], "", MAX_STR_LEN))) {
        port = tsn_port_open_by_name(port_names[j]);
        if (NULL == port) {
            SRP_LOG_ERR(ERROR_MSG_MOD_FUNC_EL_MSG, FQTSS_NAME_STR, __func__,
                        port_names[j], ERR_PORT_NOT_OPEN_STR);
            return EXIT_FAILURE;
        }
        if (0 != tsn_bridge_get_num_traffic_classes(port, &tc_num)) {
             tsn_port_close(port);
             SRP_LOG_ERR(ERROR_MSG_FUN_AND_MSG, __func__,
                         "number of traffic classes is unreachable");
             return EXIT_FAILURE;
        }
        tsn_port_close(port);

        for (i = 0; i < (int)tc_num; i++) {
            if (EXIT_FAILURE == new_fqtss_entry(table, module,
                                                port_names[j], i, table_id)) {
                SRP_LOG_ERR(ERROR_MSG_MOD_FUNC_MSG, FQTSS_NAME_STR, __func__,
                            container_name);
                return EXIT_FAILURE;
            }
        }
        j++;
    }

    return EXIT_SUCCESS;
}

/**
 * @brief Creates a startup datastore for ieee802-dotq-fqtss module.
 *
 * Function is going to be called once, due to sysrepo-plugind startup.
 *
 * @return EXIT_SUCCESS or EXIT_FAILURE.
 */
static int fqtss_fill_datastore(sr_session_ctx_t *session)
{
    const struct ly_ctx *ctx = NULL;
    struct lyd_node *root = NULL;
    const struct lys_module *module = NULL;
    char bridge_names[MAX_NUM_OF_INTERFACES][MAX_STR_LEN] = {0};
    int i = 0;
    /* table_id=0 will be used for ieee8021FqtssBapTable,
     * table_id=1 will be used for ieee8021FqtssTxSelectionAlgorithmTable
     */
    int table_id = 0;
    int fd = -1;
    char *path;

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    /* this means that startup is empty */
    if (0 == sr_path_check_startup_done_file(FQTSS_MODULE_NAME, &path)) {
        ctx = sr_get_context(sr_session_get_connection(session));
        if (NULL == ctx) {
            SRP_LOG_ERR(ERROR_MSG_FUN_AND_MSG, __func__,
                        ERR_SESSION_CTX_FAILED_STR);
            free(path);
            return EXIT_FAILURE;
        }

        if (EXIT_FAILURE == get_bridge_names(bridge_names, MAX_NUM_OF_INTERFACES,
                                             MAX_STR_LEN)) {
            SRP_LOG_ERR(ERROR_MSG_MOD_FUNC_MSG, FQTSS_NAME_STR, __func__,
                        BRIDGE_NAMES_FAILED_STR);
            free(path);
            return EXIT_FAILURE;
        }

        /* create ieee802-dot1q-fqtss root element (container ieee802-dot1q-fqtss) */
        root = lyd_new_path(NULL, ctx, "/ieee802-dot1q-fqtss:ieee802-dot1q-fqtss", NULL, 0, 0);
        if (NULL == root) {
            SRP_LOG_ERR(ERROR_MSG_FUN_AND_MSG, __func__,
                        ERR_NOT_CREATED_ROOT_ELEMENT_STR);
            free(path);
            return EXIT_FAILURE;
        }

        /* container ieee8021FqtssBapTable from ieee802-dot1q-fqtss */
        while ((i < MAX_NUM_OF_INTERFACES) &&
               (0 != strncmp(bridge_names[i], "", MAX_STR_LEN))) {
            if (EXIT_FAILURE == new_fqtss_table(root, module,
                                                FQTSS_BAP_TABLE_STR,
                                                bridge_names[i], table_id)) {
                free(path);
                return EXIT_FAILURE;
            }
            i++;
        }
        i = 0;

        /* container ieee8021FqtssTxSelectionAlgorithmTable from ieee802-dot1q-fqtss */
        table_id = 1;
        while ((i < MAX_NUM_OF_INTERFACES) &&
               (0 != strncmp(bridge_names[i], "", MAX_STR_LEN))) {
            if (EXIT_FAILURE == new_fqtss_table(root, module,
                                                FQTSS_TX_SELECTION_ALGORITHM_TABLE_STR,
                                                bridge_names[i], table_id)) {
                free(path);
                return EXIT_FAILURE;
            }
            i++;
        }

        /******** NOTE *******/
        /* The startup to running datastore copy is done before the plugin is
         * started. So, here we will replace the current startup and running
         * datastores with the subtree called 'root' in this function.
         */
        /* Switch session to statup datastore */
        if (SR_ERR_OK != sr_session_switch_ds(session, SR_DS_STARTUP)) {
            SRP_LOG_ERR(ERROR_MSG_FUN_AND_MSG, __func__,
                        ERR_SWITCH_DATASTORE_FAILED_STR);
            free(path);
            return EXIT_FAILURE;
        }

        /* Replace current startup configuration with created 'root' subtree */
        if (SR_ERR_OK != sr_replace_config(session, FQTSS_NAME_STR, root, 0, 0)) {
            SRP_LOG_ERR(ERROR_MSG_FUN_AND_MSG, __func__,
                        ERR_REPLACE_CONFIG_FAILED_STR);
            free(path);
            return EXIT_FAILURE;
        }

        /* Switch session to running datastore */
        if (SR_ERR_OK != sr_session_switch_ds(session, SR_DS_RUNNING)) {
            SRP_LOG_ERR(ERROR_MSG_FUN_AND_MSG, __func__,
                        ERR_SWITCH_DATASTORE_FAILED_STR);
            free(path);
            return EXIT_FAILURE;
        }

        /* Copy startup to running datastore */
        if (SR_ERR_OK != sr_copy_config(session, FQTSS_NAME_STR,
                                        SR_DS_STARTUP, 0, 0)) {
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
 * @brief Sysrepo plugin initialization callback.
 *
 * @param[in]   session         Sysrepo session that can be used for any API
 *                              calls needed for plugin initialization (mainly
 *                              for reading of startup configuration and
 *                              subscribing for notifications).
 * @param[in]   private_ctx     Private context (opaque to sysrepo) that will
 *                              be passed to sr_plugin_cleanup_cb when plugin
 *                              cleanup is requested.
 * @return      Error code (SR_ERR_OK on success). If an error is returned, plugin will be considered as uninitialized.
 */
int sr_plugin_init_cb(sr_session_ctx_t *session, void **private_ctx)
{
    (void)private_ctx;

    int rc = SR_ERR_OK;
    char *path = NULL;

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    /* Set plugin_init to "1" so callbacks are triggered when startup configuration is copied to running. */
    if (0 != sr_path_check_startup_done_file(FQTSS_MODULE_NAME, &path)) {
        plugin_init = 1;
    }

    do {
        rc = sr_module_change_subscribe(session, FQTSS_NAME_STR, NULL, module_change_cb,
                NULL, 0, SR_SUBSCR_ENABLED, &subscription);
        if (SR_ERR_OK != rc) break;

        /* subscribe for ieee8021FqtssAdminIdleSlopeMs node changes */
        rc = sr_module_change_subscribe(session, FQTSS_NAME_STR,
                "/ieee802-dot1q-fqtss:ieee802-dot1q-fqtss/ieee8021FqtssBapTable/ieee8021FqtssBapEntry/ieee8021FqtssAdminIdleSlopeMs",
                fqtss_element_change_cb, NULL, 1, SR_SUBSCR_ENABLED, &subscription);
        if (SR_ERR_OK != rc) break;

        /* subscribe for ieee8021FqtssAdminIdleSlopeLs node changes */
        rc = sr_module_change_subscribe(session, FQTSS_NAME_STR,
                "/ieee802-dot1q-fqtss:ieee802-dot1q-fqtss/ieee8021FqtssBapTable/ieee8021FqtssBapEntry/ieee8021FqtssAdminIdleSlopeLs",
                fqtss_element_change_cb, NULL, 1, SR_SUBSCR_ENABLED , &subscription);
        if (SR_ERR_OK != rc) break;

        /* subscribe for ieee8021FqtssTxSelectionAlgorithmID node changes */
        rc = sr_module_change_subscribe(session, FQTSS_NAME_STR,
                "/ieee802-dot1q-fqtss:ieee802-dot1q-fqtss/ieee8021FqtssTxSelectionAlgorithmTable/ieee8021FqtssTxSelectionAlgorithmEntry/ieee8021FqtssTxSelectionAlgorithmID",
                fqtss_element_change_cb, NULL, 1, SR_SUBSCR_ENABLED, &subscription);
        if (SR_ERR_OK != rc) break;

        /* subscribe for state data */
        rc = sr_oper_get_items_subscribe(session, FQTSS_NAME_STR,
                "/ieee802-dot1q-fqtss:ieee802-dot1q-fqtss/ieee8021FqtssBapTable/ieee8021FqtssBapEntry",
                fqtss_state_cb, NULL, SR_SUBSCR_DEFAULT, &subscription);
        if (SR_ERR_OK != rc) break;
    } while (0);

    if (SR_ERR_OK != rc) {
        SRP_LOG_ERR(ERR_MODULE_INIT_FAILED_STR, FQTSS_NAME_STR, sr_strerror(rc));
        sr_unsubscribe(subscription);
        return rc;
    }

    if (EXIT_FAILURE == fqtss_fill_datastore(session)) {
        SRP_LOG_ERR(ERROR_MSG_MOD_FUNC_MSG, FQTSS_NAME_STR, __func__,
                    ERR_MSG_EMPTY_STARTUP_DS_STR);
        return SR_ERR_OPERATION_FAILED;
    }

    plugin_init = 1;

    SRP_LOG_INF(INF_MODULE_INIT_SUCCESS_STR, FQTSS_NAME_STR);

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
    (void)session;
    (void)private_data;

    /* nothing to cleanup except freeing the subscriptions */
    sr_unsubscribe(subscription);
    SRP_LOG_INF(INF_MODULE_CLEANUP_STR, FQTSS_NAME_STR);
}
