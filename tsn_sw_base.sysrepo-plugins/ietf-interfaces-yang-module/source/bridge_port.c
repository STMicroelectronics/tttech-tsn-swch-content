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

#include "bridge_port.h"
#include "bridge_port_delays.h"
#include "ethernet.h"

// map between object index and object name for bridge-port augment
static char bridge_port_map[BRIDGE_PORT_MAX_OBJ_NUMBER][MAX_STR_LEN] =
{
    [BRIDGE_PORT_PVID]          = Q_BRIDGE_PORT_PVID_STR,
    [BRIDGE_PORT_DEF_PRIORITY]  = Q_BRIDGE_DEF_PRIORITY,
    [BRIDGE_PORT_PRIO_REG]      = Q_BRIDGE_PRIO_REG_STR,
    [BRIDGE_PORT_TRAFFIC_CLASS] = Q_BRIDGE_TRAFFIC_CLASS_STR,
};

// source is IEEE8021-TC-MIB
static char q_bridge_port_type_map[7][2][MAX_STR_LEN] = {
    // 7 pairs
    { BASE_ONE_STR,     "none" },
    { BASE_TWO_STR,     "c-vlan-bridge-port" },
    { BASE_THREE_STR,   "provider-network-port" },
    { BASE_FOUR_STR,    "customer-network-port" },
    { BASE_FIVE_STR,    "customer-edge-port" },
    { BASE_EIGHT_STR,   "d-bridge-port" },
    { BASE_NINE_STR,    "remote-customer-access-port" }
};

static char q_bridge_acceptable_frame_map[3][2][MAX_STR_LEN] = {
    // 3 pairs
    { BASE_ONE_STR,     "admit-all-frames" },
    { BASE_TWO_STR,     "admit-only-untagged-and-priority-tagged" },
    { BASE_THREE_STR,   "admit-only-VLAN-tagged-frames" }
};

/* global variable from ietf-interfaces.c */
extern int plugin_init;

int new_bridge_port_statistic_state_node(struct lyd_node **parent,
                                         char *interface_name)
{
    char path[MAX_STR_LEN] = "";
    char tx_objects[MAX_NUM_OF_INTERFACES][MAX_STR_LEN] = {0};
    char rx_objects[MAX_NUM_OF_INTERFACES][MAX_STR_LEN] = {0};

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    if (EXIT_SUCCESS == get_interface_rx_statistics(rx_objects, MAX_NUM_OF_INTERFACES,
                                                    MAX_STR_LEN, interface_name) &&
        EXIT_SUCCESS == get_interface_tx_statistics(tx_objects, MAX_NUM_OF_INTERFACES,
                                                    MAX_STR_LEN, interface_name)) {

        /* container statistics, leaf frame-rx */
        fill_xpath(path, BRIDGE_PORT_STATISTICS_XPATH, interface_name, Q_BRIDGE_FRAME_RX);
        if (NULL == lyd_new_path(*parent, NULL, path, rx_objects[1], 0, 0)) {
            SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                        Q_BRIDGE_FRAME_RX, ERR_NOT_CREATED_ELEMENT_STR);
        }

        /* container statistics, leaf octets-rx */
        fill_xpath(path, BRIDGE_PORT_STATISTICS_XPATH, interface_name, Q_BRIDGE_OCTET_RX);
        if (NULL == lyd_new_path(*parent, NULL, path, rx_objects[0], 0, 0)) {
            SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                        Q_BRIDGE_OCTET_RX, ERR_NOT_CREATED_ELEMENT_STR);
        }

        /* container statistics, leaf frame-tx */
        fill_xpath(path, BRIDGE_PORT_STATISTICS_XPATH, interface_name, Q_BRIDGE_FRAME_TX);
        if (NULL == lyd_new_path(*parent, NULL, path, tx_objects[1], 0, 0)) {
            SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                        Q_BRIDGE_FRAME_TX, ERR_NOT_CREATED_ELEMENT_STR);
        }

        /* container statistics, leaf octets-tx */
        fill_xpath(path, BRIDGE_PORT_STATISTICS_XPATH, interface_name, Q_BRIDGE_OCTET_TX);
        if (NULL == lyd_new_path(*parent, NULL, path, tx_objects[0], 0, 0)) {
            SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                        Q_BRIDGE_OCTET_TX, ERR_NOT_CREATED_ELEMENT_STR);
        }

        /* container statistics, leaf discard-inbound */
        fill_xpath(path, BRIDGE_PORT_STATISTICS_XPATH, interface_name, Q_BRIDGE_DISCARD_INBOUND);
        if (NULL == lyd_new_path(*parent, NULL, path, tx_objects[3], 0, 0)) {
            SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                        Q_BRIDGE_DISCARD_INBOUND, ERR_NOT_CREATED_ELEMENT_STR);
        }
    }

    return SR_ERR_OK;
}

int new_bridge_port_state_node(struct lyd_node **parent, char *interface_name)
{
    char path[MAX_STR_LEN] = "";
    char result_array[MAX_NUM_OF_INTERFACES][MAX_STR_LEN] = {0};
    char type_capabilities_aray[Q_BRIDGE_TYPE_CAPAB_NUM][Q_BRIDGE_TYPE_LEN] = {0};
    char capabilities_aray[Q_BRIDGE_CAPAB_NUM][Q_BRIDGE_TYPE_LEN] = {0};
    char tmp_str[MAX_STR_LEN * 2] = "";
    int i = 0;
    struct port_hdl *port = NULL;
    struct brport_attr_and_caps caps = {0};
    char tmp[MAX_STR_LEN * 2] = "";
    uint32_t num_tcs = 0;
    uint32_t suporrted_port_speed[3] = {10, 100, 1000};
    char supported_modes[MAX_NUM_OF_INTERFACES][MAX_STR_LEN] = {0};
    int j = 0;

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    memset(type_capabilities_aray, 0, Q_BRIDGE_TYPE_CAPAB_NUM * Q_BRIDGE_TYPE_LEN);
    memset(capabilities_aray, 0, Q_BRIDGE_CAPAB_NUM * Q_BRIDGE_TYPE_LEN);

    port = tsn_port_open_by_name(interface_name);
    if (NULL == port) {
        SRP_LOG_ERR(ERROR_MSG_MOD_FUNC_EL_MSG, Q_BRIDGE_COMP_NAME_STR, __func__,
                    interface_name, ERR_PORT_NOT_OPEN_STR);
        return SR_ERR_OPERATION_FAILED;
    }

    /* container bridge-port, leaf num-traffic-class */
    if (0 == tsn_bridge_get_num_traffic_classes(port, &num_tcs)) {
        snprintf(tmp, MAX_STR_LEN, "%u", num_tcs);
        fill_xpath(path, BRIDGE_PORT_NUM_TC_XPATH, interface_name);
        if (NULL == lyd_new_path(*parent, NULL, path, tmp, 0, 0))
        {
            SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                        path, ERR_NOT_CREATED_ELEMENT_STR);
        }
    }

    /* container bridge-port, leaf port-number */
    if (EXIT_SUCCESS == get_interface_if_index(result_array, MAX_NUM_OF_INTERFACES,
                                               MAX_STR_LEN, interface_name)) {
        fill_xpath(path, BRIDGE_PORT_PORT_NUM_XPATH, interface_name);
        if (NULL == lyd_new_path(*parent, NULL, path, result_array[0], 0, 0)) {
            SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                        Q_BRIDGE_PORT_NUMBER_STR, ERR_NOT_CREATED_ELEMENT_STR);
        }
    }

    /* container bridge-port, leaf address */
    if (EXIT_SUCCESS == get_interface_address(result_array, MAX_NUM_OF_INTERFACES,
                                              MAX_STR_LEN, interface_name)) {
        fill_xpath(path, BRIDGE_PORT_ADDRESS_XPATH, interface_name);
        change_addr_colons_to_dashes(result_array[0]);
        if (NULL == lyd_new_path(*parent, NULL, path, result_array[0], 0, 0)) {
            SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                        Q_BRIDGE_ADDRESS_STR, ERR_NOT_CREATED_ELEMENT_STR);
        }
    }

    /* get bridge port capabilities */
    if (0 == tsn_bridge_port_attr(port, &caps)) {
        /* leaf capabilities, augment from ieee802-dot1q-bridge yang module */
        /* why is it implemented like this (paragraph 9.7) - https://tools.ietf.org/html/rfc6020 */
        if (caps.portTaggingCapable) {
            strncpy(capabilities_aray[i], Q_BRIDGE_TAGGING_STR,
                    Q_BRIDGE_TYPE_LEN);
            i++;
        }
        if (caps.portConfigurableAcceptableFrameTypes) {
            strncpy(capabilities_aray[i], Q_BRIDGE_CONFIG_ACC_FRAME_TYPE_STR,
                    Q_BRIDGE_TYPE_LEN);
            i++;
        }
        if (caps.portIngressFilteringCapable) {
            strncpy(capabilities_aray[i], Q_BRIDGE_INGRESS_FILTERING_STR,
                    Q_BRIDGE_TYPE_LEN);
        }
        i = 0;
        while ((i < Q_BRIDGE_CAPAB_NUM) &&
               (0 != strncmp(capabilities_aray[i], "", Q_BRIDGE_TYPE_LEN))) {
            if (0 == i) {
                // 11*40=440 is lower than 510 (MAX_STR_LEN*2)
                memset(tmp_str, 0, MAX_STR_LEN * 2);
                strncat(tmp_str, capabilities_aray[i], Q_BRIDGE_TYPE_LEN);
            } else {
                // 11*40=440 is lower than 510 (MAX_STR_LEN*2)
                strncat(tmp_str, " ", 2);
                strncat(tmp_str, capabilities_aray[i], Q_BRIDGE_TYPE_LEN);
            }
            i++;
        }

        fill_xpath(path, BRIDGE_PORT_CAPABILITIES_XPATH, interface_name);
        if (NULL == lyd_new_path(*parent, NULL, path, tmp_str, 0, 0)) {
            SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                        Q_BRIDGE_CAPABILITIES_STR, ERR_NOT_CREATED_ELEMENT_STR);
        }
        memset(tmp_str, 0, MAX_STR_LEN * 2);

        /* leaf type-capabilities, augment from ieee802-dot1q-bridge yang module */
        /* why is it implemented like this (paragraph 9.7) - https://tools.ietf.org/html/rfc6020 */
        i = 0;
        if (caps.portTypeCapCustomerVlan) {
            strncpy(type_capabilities_aray[i], Q_BRIDGE_CUSTOMER_VLAN_PORT_STR, Q_BRIDGE_TYPE_LEN);
            i++;
        }
        if (caps.portTypeCapProviderNetwork) {
            strncpy(type_capabilities_aray[i], Q_BRIDGE_PROVIDER_NETWORK_PORT_STR, Q_BRIDGE_TYPE_LEN);
            i++;
        }
        if (caps.portTypeCapCustomerNetwork) {
            strncpy(type_capabilities_aray[i], Q_BRIDGE_CUSTOMER_NETWORK_PORT_STR, Q_BRIDGE_TYPE_LEN);
            i++;
        }
        if (caps.portTypeCapCustomerEdge) {
            strncpy(type_capabilities_aray[i], Q_BRIDGE_CUSTOMER_EDGE_PORT_STR, Q_BRIDGE_TYPE_LEN);
            i++;
        }
        if (caps.portTypeCapCustomerBackbone) {
            strncpy(type_capabilities_aray[i], Q_BRIDGE_CUSTOMER_BACKBONE_PORT_STR, Q_BRIDGE_TYPE_LEN);
            i++;
        }
        if (caps.portTypeCapVirtualInstance) {
            strncpy(type_capabilities_aray[i], Q_BRIDGE_VIRTUAL_INSTANCE_PORT_STR, Q_BRIDGE_TYPE_LEN);
            i++;
        }
        if (caps.portTypeCapDBridge) {
            strncpy(type_capabilities_aray[i], Q_BRIDGE_D_BRIDGE_PORT_STR, Q_BRIDGE_TYPE_LEN);
            i++;
        }
        if (caps.portTypeCapRemoteCustomerAccess) {
            strncpy(type_capabilities_aray[i], Q_BRIDGE_REMOTE_CUSTOMER_ACC_PORT_STR, Q_BRIDGE_TYPE_LEN);
            i++;
        }
        if (caps.portTypeCapStationFacing) {
            strncpy(type_capabilities_aray[i], Q_BRIDGE_STAT_FACING_BRIDGE_PORT_STR, Q_BRIDGE_TYPE_LEN);
            i++;
        }
        if (caps.portTypeCapUplinkAccess) {
            strncpy(type_capabilities_aray[i], Q_BRIDGE_UPLINK_ACCESS_PORT_STR, Q_BRIDGE_TYPE_LEN);
            i++;
        }
        if (caps.portTypeCapUplinkRelay) {
            strncpy(type_capabilities_aray[i], Q_BRIDGE_UPLINK_RELAY_PORT_STR, Q_BRIDGE_TYPE_LEN);
        }
        i = 0;
        while ((i < Q_BRIDGE_TYPE_CAPAB_NUM) &&
               (0 != strncmp(type_capabilities_aray[i], "", Q_BRIDGE_TYPE_LEN))) {
            if (0 == i) {
                // 11*40 = 440 is lower than 510 (MAX_STR_LEN*2)
                strncat(tmp_str, type_capabilities_aray[i], Q_BRIDGE_TYPE_LEN);
            } else {
                // 11*40 = 440 is lower than 510 (MAX_STR_LEN*2)
                strncat(tmp_str, " ", 2);
                strncat(tmp_str, type_capabilities_aray[i], Q_BRIDGE_TYPE_LEN);
            }
            i++;
        }
        fill_xpath(path, BRIDGE_PORT_TYPE_CAPABILITIES_XPATH, interface_name);
        if (NULL == lyd_new_path(*parent, NULL, path, tmp_str, 0, 0)) {
            SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                        Q_BRIDGE_TYPE_CAPABILITIES_STR, ERR_NOT_CREATED_ELEMENT_STR);
        }
        memset(tmp_str, 0, MAX_STR_LEN * 2);

        /* container bridge-port, leaf external */
        fill_xpath(path, BRIDGE_PORT_EXTERNAL_XPATH, interface_name);
        snprintf(tmp, MAX_STR_LEN * 2, "%s", (caps.portExternal == 0) ? BASE_FALSE : BASE_TRUE);
        if (NULL == lyd_new_path(*parent, NULL, path, tmp, 0, 0)) {
            SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                        Q_BRIDGE_EXTERNAL_STR, ERR_NOT_CREATED_ELEMENT_STR);
        }
    }

    /* container statistics */
    if (SR_ERR_OK != new_bridge_port_statistic_state_node(parent, interface_name)) {
        SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                    Q_BRIDGE_STATISTICS, ERR_NOT_CREATED_ELEMENT_STR);
    }

    /* bridge-port-delays, augment from ieee802-dot1q-bridge-delays yang module */
    /* get_supported_link_modes() returns the supported link modes as they are
     * shown in the ethtool, like this:
     *
     * Supported link modes:   100baseT/Full
                               1000baseT/Full

     * So, we need to check is our link mode shown in the supported modes.
     */
    if (EXIT_SUCCESS == get_supported_link_modes(supported_modes, MAX_NUM_OF_INTERFACES,
                                                 MAX_STR_LEN, interface_name)) {
        /* go through all possible supported speeds 10, 100, 1000 Megabits per second */
        i = 0;
        while (i < 3) {
            /* Form the link mode based on speed. Only duplex Full is supported */
            snprintf(tmp, MAX_STR_LEN * 2, "%ubaseT/Full", suporrted_port_speed[i]);
            j = 0;
            while ((j < MAX_NUM_OF_INTERFACES) &&
                   (0 != strncmp(supported_modes[j], "", MAX_STR_LEN))) {
                if (NULL != strstr(supported_modes[j], tmp)) {
                    if (SR_ERR_OK != new_bridge_port_delays(parent,
                                                            interface_name,
                                                            suporrted_port_speed[i])) {
                        SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                                    Q_BRIDGE_BRIDGE_PORTS, ERR_NOT_CREATED_ELEMENT_STR);
                    }
                    break;
                }
                j++;
            }
            i++;
        }
    }

    tsn_port_close(port);

    return SR_ERR_OK;
}

struct lyd_node *new_bridge_port_node(struct lyd_node *parent,
                                      struct ly_ctx *ctx, char *if_name)
{
    struct lyd_node *bridge_port = NULL;
    struct lyd_node *priority_reg = NULL;
    struct lyd_node *traffic_class = NULL;
    const struct lys_module *module = NULL;
    struct port_hdl *port = NULL;
    struct brport_attr_and_caps caps = {0};
    char tmp_value[MAX_STR_LEN] = "";
    char bridge_names[MAX_NUM_OF_INTERFACES][MAX_STR_LEN] = {0};
    char priority_name[MAX_STR_LEN] = "";
    char result_array[MAX_NUM_OF_INTERFACES][MAX_STR_LEN] = {0};
    uint8_t tmp_uint8 = 0;
    uint32_t tmp_uint32 = 0;
    uint32_t num_tcs = 0;
    uint8_t i = 0;

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    module = ly_ctx_load_module(ctx, Q_BRIDGE_MODULE_NAME, NULL);
    if (NULL == module) {
        SRP_LOG_ERR(ERROR_MSG_FUN_AND_MSG, __func__, ERR_MSG_LOAD_MODULE_STR);
        return NULL;
    }

    /* new bridge-port container */
    if (EXIT_FAILURE == new_container(parent, &bridge_port, module, Q_BRIDGE_BRIDGE_PORTS)) {
        return NULL;
    }

    /* open port */
    port = tsn_port_open_by_name(if_name);
    if (NULL == port) {
        SRP_LOG_ERR(ERROR_MSG_MOD_FUNC_EL_MSG, Q_BRIDGE_MODULE_NAME, __func__,
                    if_name, ERR_PORT_NOT_OPEN_STR);
        return NULL;
    }

    /* get bridge port capabilities */
    if (tsn_bridge_port_attr(port, &caps)) {
        SRP_LOG_ERR(ERR_GET_FUNC_FAILED_STR, Q_BRIDGE_COMP_NAME_STR, if_name);
        tsn_port_close(port);
        return NULL;
    }

    if (EXIT_FAILURE == get_bridge_names(bridge_names, MAX_NUM_OF_INTERFACES, MAX_STR_LEN)) {
        SRP_LOG_ERR(ERR_BRIDE_NAMES_FAILED_STR, __func__);
        return NULL;
    }

    /* leaf component-name from container bridge-port */
    if (EXIT_FAILURE == new_node(bridge_port, module, Q_BRIDGE_COMP_NAME_STR, bridge_names[0])) {
        tsn_port_close(port);
        return NULL;
    }

    /* leaf port-type from container bridge-port */
    snprintf(tmp_value, MAX_STR_LEN, "%d", caps.portType);
    if (EXIT_FAILURE == translate_single_string_using_map(tmp_value, q_bridge_port_type_map)) {
        tsn_port_close(port);
        return NULL;
    }
    if (0 != strncmp(tmp_value, "none", MAX_STR_LEN)) {
        if (EXIT_FAILURE == new_node(bridge_port, module, Q_BRIDGE_PORT_TYPE_STR, tmp_value)) {
            tsn_port_close(port);
            return NULL;
        }
    }

    /* leaf pvid from container bridge-port */
    if (EXIT_SUCCESS == get_q_bridge_port_pvid(result_array, 1, MAX_STR_LEN, if_name)) {
        if (EXIT_FAILURE == new_node(bridge_port, module, Q_BRIDGE_PORT_PVID_STR, result_array[0])) {
            tsn_port_close(port);
            return NULL;
        }
    }

    /* leaf pvid from container bridge-port */
    if (0 == tsn_bridge_get_default_prio(port, &tmp_uint32)) {
        snprintf(tmp_value, MAX_STR_LEN, "%u", tmp_uint32);
        if (EXIT_FAILURE == new_node(bridge_port, module, Q_BRIDGE_DEF_PRIORITY, tmp_value)) {
            tsn_port_close(port);
            return NULL;
        }
    }

    /* container priority-regeneration, augment from ieee802-dot1q-bridge yang module. */
    if (EXIT_FAILURE == new_container(bridge_port, &priority_reg, module, Q_BRIDGE_PRIO_REG_STR)) {
        tsn_port_close(port);
        return NULL;
    }
    /* create leaf priority[0-9], from container priority-regeneration */
    for (i = 0 ; i < BASE_EIGHT ; i++) {
        snprintf(priority_name, MAX_STR_LEN, "priority%d", i);
        if (0 == tsn_fqtss_get_prio_regen_table(port, i, &tmp_uint8)) {
            snprintf(tmp_value, MAX_STR_LEN, "%u", tmp_uint8);
            if (EXIT_FAILURE == new_node(priority_reg, module, priority_name, tmp_value)) {
                tsn_port_close(port);
                return NULL;
            }
        }
    }

    /* container traffic-class, augment from ieee802-dot1q-bridge yang module. */
    if (EXIT_FAILURE == new_container(bridge_port, &traffic_class, module, Q_BRIDGE_TRAFFIC_CLASS_STR)) {
        tsn_port_close(port);
        return NULL;
    }
    /* get number of traffic classes */
    if (0 != tsn_bridge_get_num_traffic_classes(port, &num_tcs)) {
        SRP_LOG_ERR(ERR_GET_FUNC_FAILED_STR, Q_BRIDGE_COMP_NAME_STR, if_name);
        tsn_port_close(port);
        return NULL;
    }
    for (i = 0 ; i < num_tcs ; i++) {
        snprintf(priority_name, MAX_STR_LEN, "priority%d", i);
        if (0 == tsn_fqtss_get_tc_table(port, i, &tmp_uint8)) {
            snprintf(tmp_value, MAX_STR_LEN, "%u", tmp_uint8);
            if (EXIT_FAILURE == new_node(traffic_class, module, priority_name, tmp_value)) {
                tsn_port_close(port);
                return NULL;
            }
        }
    }

    /* leaf acceptable-frame, augment from ieee802-dot1q-bridge yang module */
    snprintf(tmp_value, MAX_STR_LEN, "%d", caps.portAcceptableFrameTypes);
    if (EXIT_FAILURE == translate_single_string_using_map(tmp_value, q_bridge_acceptable_frame_map)) {
        tsn_port_close(port);
        return NULL;
    }
    if (EXIT_FAILURE == new_node(bridge_port, module, Q_BRIDGE_ACCEPTABLE_FRAME_STR, tmp_value)) {
        tsn_port_close(port);
        return NULL;
    }
    memset(tmp_value, 0, MAX_STR_LEN);

    /* leaf enable-ingress-filtering, augment from ieee802-dot1q-bridge yang module */
    snprintf(tmp_value, MAX_STR_LEN, "%s", (caps.portEnableIngressFiltering) ? BASE_TRUE : BASE_FALSE);
    if (EXIT_FAILURE == new_node(bridge_port, module, Q_BRIDGE_ENABLE_INGRESS_FILTERING_STR, tmp_value)) {
        tsn_port_close(port);
        return NULL;
    }
    memset(tmp_value, 0, MAX_STR_LEN);

    tsn_port_close(port);
    return bridge_port;
}

int bridge_port_port_type_cb(sr_session_ctx_t *session, const char *module_name,
                             const char *xpath, sr_event_t event,
                             uint32_t request_id, void *private_data)
{
    (void)module_name;
    (void)event;
    (void)request_id;
    (void)private_data;

    int rc = SR_ERR_OK;
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

        if ((op == SR_OP_MODIFIED) || (op == SR_OP_CREATED)) {
            if (0 != strncmp(node->data.identityref_val,
                             "ieee802-dot1q-bridge:c-vlan-bridge-port",
                             MAX_STR_LEN)) {
                sr_set_error(session, node->xpath,
                             ERR_PORT_TYPE_SPPORTED_XPATH_STR, node->xpath);
                return SR_ERR_OPERATION_FAILED;
            }
        }
        /* delete operation is not suppoerted */
        if (op == SR_OP_DELETED) {
            sr_set_error(session, node->xpath,
                         ERR_REM_OP_NOT_SUPPORTED_XPATH_STR, node->xpath);
            return SR_ERR_OPERATION_FAILED;
        }
    }

    sr_free_val(old_value);
    sr_free_val(new_value);
    sr_free_change_iter(iter);
    node = NULL;

    return SR_ERR_OK;
}

int bridge_port_change_cb(sr_session_ctx_t *session, const char *module_name,
                          const char *xpath, sr_event_t event,
                          uint32_t request_id, void *private_data)
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
    sr_val_t *old_node = NULL;
    sr_val_t *new_node = NULL;
    sr_val_t *node = NULL;
    struct port_hdl *port = NULL;
    char tmp_str[MAX_STR_LEN] = "";
    char interface_name[MAX_STR_LEN] = "";
    char node_name[MAX_STR_LEN] = "";
    char *tmp_node_name = NULL;
    uint32_t tmp_uint32 = 0;
    unsigned int pcp = 0;
    int obj_index = 0;

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
                                           &old_node, &new_node)) {
        node = (op == SR_OP_DELETED) ? old_node : new_node;

        /* get interface name from xpath */
        if (EXIT_SUCCESS != get_key_value(session, node->xpath, IF_INTERFACE_LIST,
                                          IF_NAME_STR, &st, interface_name)) {
            SRP_LOG_ERR("%s: %s (%s)", __func__, ERR_MISSING_ELEMENT_STR, IF_NAME_STR);
            return SR_ERR_OPERATION_FAILED;
        }

        if (op == SR_OP_DELETED) {
            sr_set_error(session, node->xpath,
                         ERR_REM_OP_NOT_SUPPORTED_XPATH_STR, node->xpath);
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
            /* open port */
            port = tsn_port_open_by_name(interface_name);
            if (NULL == port) {
                sr_set_error(session, node->xpath,
                             ERR_PORT_NOT_OPEN_XPATH_STR, node->xpath);
                return SR_ERR_INTERNAL;
            }

            /* This callback is for all nodes from bridge-port augment. If it
             * is triggered for some priority node, for example for
             * priority-regeneration/priority1, it is necessary to take
             * priority-regeneration as node name. The same is for
             * traffic-class container and his priority values. If the callback
             * is triggered for pvid or default-priority, then the node will be
             * gathered with sr_xpath_node_name(node->xpath) as usual.
             */
            if (sr_xpath_node(node->xpath, Q_BRIDGE_PRIO_REG_STR, &st)) {
                snprintf(node_name, MAX_STR_LEN, "%s", Q_BRIDGE_PRIO_REG_STR);
            } else if (sr_xpath_node(node->xpath, Q_BRIDGE_TRAFFIC_CLASS_STR, &st)) {
                snprintf(node_name, MAX_STR_LEN, "%s", Q_BRIDGE_TRAFFIC_CLASS_STR);
            } else {
                snprintf(node_name, MAX_STR_LEN, "%s", sr_xpath_node_name(node->xpath));
            }
            sr_xpath_recover(&st);

            obj_index = find_obj_index_by_name(node_name, bridge_port_map, BRIDGE_PORT_MAX_OBJ_NUMBER);
            switch (obj_index) {
            case BRIDGE_PORT_PVID:
                snprintf(tmp_str, MAX_STR_LEN, "%u", node->data.uint32_val);
                rc = set_q_bridge_port_pvid(interface_name, tmp_str);
                if (rc) {
                    sr_set_error(session, node->xpath,
                                 ERR_SET_FUNC_FAILED_XPATH_STR, node->xpath);
                    break;
                }
                break;
            case BRIDGE_PORT_DEF_PRIORITY:
                /* set new admin control list length */
                tmp_uint32 = (uint32_t)node->data.uint8_val;
                tsn_ret_val = tsn_bridge_set_default_prio(port, tmp_uint32);
                if (E_TSN_LIB_NO_ERROR != tsn_ret_val) {
                    sr_set_error(session, node->xpath, ERR_SETTER_EXT_FMT_STR,
                                 Q_BRIDGE_DEF_PRIORITY, LIT_VAL_STR,
                                 LIT_XPATH_STR, node->xpath,
                                 tsn_lib_get_known_ernno_msg(tsn_ret_val));
                    break;
                }
                break;
            case BRIDGE_PORT_PRIO_REG:
                /* get the priority[X] leaf name and make the pcp value of
                 * that. For example, if the node is 'priority4' the pcp value
                 * needs to be 4.
                 */
                tmp_node_name = sr_xpath_node_name(node->xpath);
                pcp = tmp_node_name[8] - '0';
                tsn_ret_val = tsn_fqtss_set_prio_regen_table(port, pcp, node->data.uint8_val);
                if (E_TSN_LIB_NO_ERROR != tsn_ret_val) {
                    sr_set_error(session, node->xpath, ERR_SETTER_EXT_FMT_STR,
                                 Q_BRIDGE_PRIO_REG_STR, LIT_ENTRY_STR,
                                 LIT_XPATH_STR, node->xpath,
                                 tsn_lib_get_known_ernno_msg(tsn_ret_val));
                    break;
                }
                break;
            case BRIDGE_PORT_TRAFFIC_CLASS:
                tmp_node_name = sr_xpath_node_name(node->xpath);
                pcp = tmp_node_name[8] - '0';
                rc = tsn_fqtss_set_tc_table(port, pcp, node->data.uint8_val);
                if (rc) {
                    sr_set_error(session, node->xpath,
                                 ERR_SET_FUNC_FAILED_XPATH_STR, node->xpath);
                    break;
                }
                break;
            }
            if (rc) {
                tsn_port_close(port);
                return SR_ERR_OPERATION_FAILED;
            }
            tsn_port_close(port);
        }
    }

    sr_free_val(old_node);
    sr_free_val(new_node);
    node = NULL;
    sr_free_change_iter(iter);

    return SR_ERR_OK;
}
