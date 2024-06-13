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

#include "bridge_port_delays.h"

int new_bridge_port_delays(struct lyd_node **parent, char *interface_name,
                           uint32_t port_speed)
{
    struct port_hdl *port = NULL;
    struct pt_delay_entry delay_entry;
    char path[MAX_STR_LEN] = "";
    char value[21] = "";

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    port = tsn_port_open_by_name(interface_name);
    if (NULL == port) {
        SRP_LOG_ERR(ERROR_MSG_MOD_FUNC_EL_MSG, BRIDGE_PORT_DELAYS_MODULE_NAME, __func__,
                    interface_name, ERR_PORT_NOT_OPEN_STR);
        return SR_ERR_OPERATION_FAILED;
    }

    if (0 == tsn_bridge_get_port_delays(port, port_speed, &delay_entry)) {
        /* list bridge-port-delays, leaf dependentRxDelayMin */
        fill_xpath(path, BRIDGE_PORT_DEP_RX_DELAY_MIN_XPATH, interface_name, port_speed);
        snprintf(value, 21, "%llu", delay_entry.dep_rx_delay_min_ps); // picoseconds
        if (NULL == lyd_new_path(*parent, NULL, path, value, 0, 0)) {
            SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                        path, ERR_NOT_CREATED_ELEMENT_STR);
        }
        /* list bridge-port-delays, leaf dependentRxDelayMax */
        fill_xpath(path, BRIDGE_PORT_DEP_RX_DELAY_MAX_XPATH, interface_name, port_speed);
        snprintf(value, 21, "%llu", delay_entry.dep_rx_delay_max_ps); // picoseconds
        if (NULL == lyd_new_path(*parent, NULL, path, value, 0, 0)) {
            SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                        path, ERR_NOT_CREATED_ELEMENT_STR);
        }
        /* list bridge-port-delays, leaf independentRxDelayMin */
        fill_xpath(path, BRIDGE_PORT_INDEP_RX_DELAY_MIN_XPATH, interface_name, port_speed);
        snprintf(value, 21, "%u", delay_entry.indep_rx_delay_min_ns); // nanoseconds
        if (NULL == lyd_new_path(*parent, NULL, path, value, 0, 0)) {
            SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                        path, ERR_NOT_CREATED_ELEMENT_STR);
        }
        /* list bridge-port-delays, leaf independentRxDelayMax */
        fill_xpath(path, BRIDGE_PORT_INDEP_RX_DELAY_MAX_XPATH, interface_name, port_speed);
        snprintf(value, 21, "%u", delay_entry.indep_rx_delay_max_ns); // nanoseconds
        if (NULL == lyd_new_path(*parent, NULL, path, value, 0, 0)) {
            SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                        path, ERR_NOT_CREATED_ELEMENT_STR);
        }
        /* list bridge-port-delays, leaf independentRlyDelayMin */
        fill_xpath(path, BRIDGE_PORT_INDEP_RLY_DELAY_MIN_XPATH, interface_name, port_speed);
        snprintf(value, 21, "%u", delay_entry.indep_rly_delay_min_ns); // nanoseconds
        if (NULL == lyd_new_path(*parent, NULL, path, value, 0, 0)) {
            SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                        path, ERR_NOT_CREATED_ELEMENT_STR);
        }
        /* list bridge-port-delays, leaf independentRlyDelayMax */
        fill_xpath(path, BRIDGE_PORT_INDEP_RLY_DELAY_MAX_XPATH, interface_name, port_speed);
        snprintf(value, 21, "%u", delay_entry.indep_rly_delay_max_ns); // nanoseconds
        if (NULL == lyd_new_path(*parent, NULL, path, value, 0, 0)) {
            SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                        path, ERR_NOT_CREATED_ELEMENT_STR);
        }
        /* list bridge-port-delays, leaf independentTxDelayMin */
        fill_xpath(path, BRIDGE_PORT_INDEP_TX_DELAY_MIN_XPATH, interface_name, port_speed);
        snprintf(value, 21, "%u", delay_entry.indep_tx_delay_min_ns); // nanoseconds
        if (NULL == lyd_new_path(*parent, NULL, path, value, 0, 0)) {
            SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                        path, ERR_NOT_CREATED_ELEMENT_STR);
        }
        /* list bridge-port-delays, leaf independentTxDelayMax */
        fill_xpath(path, BRIDGE_PORT_INDEP_TX_DELAY_MAX_XPATH, interface_name, port_speed);
        snprintf(value, 21, "%u", delay_entry.indep_tx_delay_max_ns); // nanoseconds
        if (NULL == lyd_new_path(*parent, NULL, path, value, 0, 0)) {
            SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                        path, ERR_NOT_CREATED_ELEMENT_STR);
        }
    }
    tsn_port_close(port);

    return SR_ERR_OK;
}
