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

#include "ethernet.h"

static char full_half_map[2][2][MAX_STR_LEN] = {
    // 2 pairs
    { ETH_DUPLEX_HALF_STR,  BASE_ETH_DUPLEX_HALF },
    { ETH_DUPLEX_FULL_STR,  BASE_ETH_DUPLEX_FULL },
};

static char eth_speed_map[4][2][MAX_STR_LEN] = {
    // 4 pairs
    { BASE_SPEED_TEN_STR,           ETH_SPEED_TEN_STR },
    { BASE_SPEED_ONE_HUNDRED_STR,   ETH_SPEED_ONE_HUNDRED_STR },
    { BASE_SPEED_ONE_THOUSAND_STR,  ETH_SPEED_ONE_THOUSAND_STR },
    { BASE_SPEED_TEN_THOUSAND_STR,  ETH_SPEED_ONE_THOUSAND_STR },
};

static char on_off_map[2][2][MAX_STR_LEN] = {
    // 2 pairs
    { BASE_ON,  BASE_TRUE },
    { BASE_OFF, BASE_FALSE },
};


static char eth_speed_map_cb[4][2][MAX_STR_LEN] = {
    // 4 pairs
    { ETH_SPEED_TEN,                BASE_TEN_STR },
    { ETH_SPEED_ONE_HUNDRED,        BASE_ONE_HUNDRED_STR },
    { ETH_SPEED_ONE_THOUSAND,       BASE_ONE_THOUSAND_STR },
    { ETH_SPEED_TEN_THOUSAND,       BASE_TEN_THOUSAND_STR },
};

/* global variable from ietf-interfaces.c */
extern int plugin_init;

int multiple_advertised_link_modes(char objects[][MAX_STR_LEN],
                                   char *advertised_mode)
{
    int i = 0;

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    /* go trough all advertised link modes */
    while ((i < MAX_NUM_OF_INTERFACES) &&
           (0 != strncmp(objects[i], "", MAX_STR_LEN))) {
        /* If advertised link mode is different from created one from speed and
         * duplex (tmp), or there is advertised link mode same as tmp, but
         * there is more of them that are different than tmp, then return 1.
         * Speed and duplex then should not be created.
         */
        if (NULL == strstr(objects[i], advertised_mode)) {
            return EXIT_FAILURE;
        }
        i++;
    }
    return EXIT_SUCCESS;
}

struct lyd_node *new_ethernet_container(struct lyd_node *parent,
                                        struct ly_ctx *ctx, char *if_name)
{
    struct lyd_node *ethernet_container = NULL;
    const struct lys_module *module = NULL;

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    module = ly_ctx_load_module(ctx, ETH_MODULE_NAME_STR, NULL);

    if (NULL == module) {
        SRP_LOG_ERR(ERROR_MSG_FUN_AND_MSG, __func__, ERR_MSG_LOAD_MODULE_STR);
        return NULL;
    }
    /* new ethernet container */
    if (EXIT_FAILURE == new_container(parent, &ethernet_container, module,
                                      ETH_ETHERNET_CONTAINER_STR)) {
        return NULL;
    }
    if (NULL == new_auto_negotiation(ethernet_container, module, if_name)) {
        return NULL;
    }

    return ethernet_container;
}

struct lyd_node *new_auto_negotiation(struct lyd_node *parent,
                                      const struct lys_module *module,
                                      char *if_name)
{
    struct lyd_node *auto_negotiation = NULL;
    char objects[MAX_NUM_OF_INTERFACES][MAX_STR_LEN] = {0};
    char speed[MAX_NUM_OF_INTERFACES][MAX_STR_LEN] = {0};
    char duplex[MAX_NUM_OF_INTERFACES][MAX_STR_LEN] = {0};
    char mode[MAX_STR_LEN] = ETH_BASE_T; /* for now we support only baseT link mode */
    long int tmp_speed = 0;
    char advertised_mode[3*MAX_STR_LEN] = {0};

    SRP_LOG_DBG(DEBUG_MSG_FUN_CALLED_WITH_ONE_PARAM, __func__, if_name);

    /* container auto-negotiation, mandatory */
    if (EXIT_FAILURE == new_container(parent, &auto_negotiation, module,
                                      ETH_AUTO_NEGOTIATION_STR)) {
        return NULL;
    }
    /* leaf enable from container auto-negotiation */
    if (EXIT_SUCCESS == get_eth_if_auto_neg_enable(objects, MAX_NUM_OF_INTERFACES,
                                                   MAX_STR_LEN, if_name)) {
        if (EXIT_FAILURE == translate_strings_using_map(objects, on_off_map)) {
            return NULL;
        }
        if (EXIT_FAILURE == new_node(auto_negotiation, module,
                                     ETH_IF_ENABLE_STR, (char*)objects)) {
            return NULL;
        }
    }

    get_eth_if_duplex(duplex, MAX_NUM_OF_INTERFACES, MAX_STR_LEN, if_name);
    get_eth_if_speed(speed, MAX_NUM_OF_INTERFACES, MAX_STR_LEN, if_name);
    tmp_speed = strtol(speed[0], NULL, 10); // take string value as long int
    snprintf(speed[0], MAX_STR_LEN, "%ld", tmp_speed);

    /* take current advertised link modes and write them to file
     * /usr/share/applications/[port_name]_advertise_link_modes.txt
     */
    if (EXIT_SUCCESS != store_advertised_link_modes(objects, MAX_NUM_OF_INTERFACES,
                                                    MAX_STR_LEN, if_name)) {
        SRP_LOG_ERR(ERROR_MSG_FUN_XML_EL_AND_MSG, __func__,
                    ETH_ADVERTISED_IF_MODE_STR, ERROR_MSG_STORED_LINK_MODES);
        return NULL;
    }

    /* create advertised link mode using speed and duplex */
    snprintf(advertised_mode, 3*MAX_STR_LEN, "%sbaseT/%s ", speed[0], duplex[0]);

    /* if link is unplugged (state DOWN), we need to prevent invalid values in datastore */
    if ((EXIT_SUCCESS == check_link_modes(BASE_RC_NO, if_name, speed[0], duplex[0],
                                          mode, ETH_SUPPORTED_IF_MODE_STR)) &&
        (EXIT_SUCCESS == multiple_advertised_link_modes(objects, advertised_mode)))
    {
        /* leaf duplex */
        if (EXIT_SUCCESS == get_eth_if_duplex(objects, MAX_NUM_OF_INTERFACES,
                                              MAX_STR_LEN, if_name)) {
            if (EXIT_FAILURE == translate_strings_using_map(objects, full_half_map)) {
                return NULL;
            }
            if (EXIT_FAILURE == new_node(parent, module, ETH_DUPLEX_STR, (char*)objects)) {
                return NULL;
            }
        }
        /* leaf speed */
        if (EXIT_SUCCESS == get_eth_if_speed(objects, MAX_NUM_OF_INTERFACES,
                                             MAX_STR_LEN, if_name)) {
            if (EXIT_FAILURE == translate_strings_using_map(objects, eth_speed_map)) {
                return NULL;
            }
            if (EXIT_FAILURE == new_node(parent, module, ETH_SPEED_STR,
                                         (char*)objects)) {
                return NULL;
            }
        }
    }

    return auto_negotiation;
}

int new_state_ethernet_statistics_container(struct lyd_node **parent,
                                            char *interface_name)
{
    u_long packets, mcast;
    char path[MAX_STR_LEN] = "";
    char tmp[MAX_STR_LEN] = "";
    char tx_objects[MAX_NUM_OF_INTERFACES][MAX_STR_LEN] = {0};
    char rx_objects[MAX_NUM_OF_INTERFACES][MAX_STR_LEN] = {0};

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    /* fill frame container, not mandatory */
    if (EXIT_FAILURE == get_interface_rx_statistics(rx_objects, MAX_NUM_OF_INTERFACES,
                                                    MAX_STR_LEN, interface_name) ||
            EXIT_FAILURE == get_interface_tx_statistics(tx_objects, MAX_NUM_OF_INTERFACES,
                                                        MAX_STR_LEN, interface_name)) {
        return SR_ERR_OPERATION_FAILED;
    }

    /* leaf in-total-frames */
    fill_xpath(path, INTERFACE_IN_TOTAL_FRAMES_XPATH, interface_name);
    if (NULL == lyd_new_path(*parent, NULL, path, rx_objects[1], 0, 0)) {
        SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                    ETH_IN_TOTAL_FRAMES, ERR_NOT_CREATED_ELEMENT_STR);
    }

    /* leaf in-total-octets */
    fill_xpath(path, INTERFACE_IN_TOTAL_OCTETS_XPATH, interface_name);
    if (NULL == lyd_new_path(*parent, NULL, path, rx_objects[0], 0, 0)) {
        SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                    ETH_IN_TOTAL_OCTETS, ERR_NOT_CREATED_ELEMENT_STR);
    }

    packets = atol(rx_objects[1]);
    mcast = atol(rx_objects[5]);
    packets = packets + mcast;
    snprintf(tmp, MAX_STR_LEN, "%lu", packets);

    /* leaf in-frames */
    fill_xpath(path, INTERFACE_IN_FRAMES_XPATH, interface_name);
    if (NULL == lyd_new_path(*parent, NULL, path, tmp, 0, 0)) {
        SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                    ETH_IN_FRAMES, ERR_NOT_CREATED_ELEMENT_STR);
    }

    /* leaf in-multicast-frames */
    fill_xpath(path, INTERFACE_IN_MULTICAST_FRAMES_XPATH, interface_name);
    if (NULL == lyd_new_path(*parent, NULL, path, rx_objects[5], 0, 0)) {
        SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                    ETH_IN_MULTICAST_FRAMES, ERR_NOT_CREATED_ELEMENT_STR);
    }

    packets = atol(tx_objects[1]);
    mcast = atol(tx_objects[5]);
    packets = packets + mcast;
    snprintf(tmp, MAX_STR_LEN, "%lu", packets);

    /* leaf out-frames */
    fill_xpath(path, INTERFACE_OUT_FRAMES_XPATH, interface_name);
    if (NULL == lyd_new_path(*parent, NULL, path, tmp, 0, 0)) {
        SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                    ETH_OUT_FRAMES, ERR_NOT_CREATED_ELEMENT_STR);
    }

    return SR_ERR_OK;

}

int new_state_ethernet_container(struct lyd_node **parent, char *interface_name)
{
    char path[MAX_STR_LEN] = "";
    char objects[MAX_NUM_OF_INTERFACES][MAX_STR_LEN] = {0};

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    /* leaf max-frame-length, not mandatory */
    if (EXIT_SUCCESS == get_ethernet_max_frame_length(objects, MAX_NUM_OF_INTERFACES,
                                                      MAX_STR_LEN, interface_name)) {
        if (strtoul(objects[0], NULL, 10) > UINT16_MAX) {
            snprintf(objects[0], MAX_STR_LEN, "%d", UINT16_MAX);
        }
        fill_xpath(path, INTERFACE_MAX_FRAME_LENGHT_XPATH, interface_name);
        if (NULL == lyd_new_path(*parent, NULL, path, objects[0], 0, 0)) {
            SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                        ETH_MAX_FRAME_LEN_STR, ERR_NOT_CREATED_ELEMENT_STR);
        }
    }

    /* container statistics, mandatory */
    if (SR_ERR_OK != new_state_ethernet_statistics_container(parent, interface_name)) {
        SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                    IF_STATISTICS_STR, ERR_NOT_CREATED_ELEMENT_STR);
    }

    return SR_ERR_OK;

}

int interface_ethernet_duplex_speed_cb(sr_session_ctx_t *session,
                                       const char *module_name,
                                       const char *xpath, sr_event_t event,
                                       uint32_t request_id, void *private_data)
{
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
    sr_val_t *speed = NULL;
    sr_val_t *duplex = NULL;
    char interface_name[MAX_STR_LEN] = "";
    char path[MAX_STR_LEN] = "";
    char tmp_speed[MAX_STR_LEN] = "";
    char mode[MAX_STR_LEN] = ETH_BASE_T; /* for now, only baseT link mode is supported */

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    if ((SR_EV_DONE == event) || (0 == plugin_init)) {
        SRP_LOG_DBG(DEBUG_MSG_WITH_TWO_PARAM, DBG_APPLYING_CHANGES_MSG, __func__);
        return SR_ERR_OK;
    }

    if (SR_EV_UPDATE == event) {
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
                rc = SR_ERR_OPERATION_FAILED;
                goto up_free_chg_iter;
            }

            /* If user changed values of leaf then node->dflt is "0". If
             * sysrepo changes value of leaf, because it has default value,
             * that node->dflt will be "1". This condition is added because of
             * booting, i.e. applying startup configuration. Every leaf that
             * has default value will be triggered with ADD operation if he
             * does not exist inside startup configuration. Its node->dflt flag
             * will be "1" then.
            */
            if ((0 == node->dflt) && (op == SR_OP_DELETED)) {
                /* If deleted node is speed than duplex should be deleted from
                 * running configuration
                 */
                if (sr_xpath_node_name_eq(node->xpath, ETH_SPEED_STR)) {
                    fill_xpath(path, INTERFACE_DUPLEX_XPATH, interface_name);
                    if (SR_ERR_OK != sr_delete_item(session, path, SR_EDIT_DEFAULT)) {
                        SRP_LOG_ERR(ERR_DELETING_VALUE_STR,
                                    ETH_ETHERNET_CONTAINER_STR,
                                    ETH_DUPLEX_STR, interface_name, __func__);
                        rc = SR_ERR_OPERATION_FAILED;
                        goto up_free_chg_iter;
                    }
                }

                /* If deleted node is duplex than speed should be deleted from
                 * running configuration
                 */
                if (sr_xpath_node_name_eq(node->xpath, ETH_DUPLEX_STR)) {
                    fill_xpath(path, INTERFACE_SPEED_XPATH, interface_name);
                    if (SR_ERR_OK != sr_delete_item(session, path, SR_EDIT_DEFAULT)) {
                        SRP_LOG_ERR(ERR_DELETING_VALUE_STR,
                                    ETH_ETHERNET_CONTAINER_STR,
                                    ETH_SPEED_STR, interface_name, __func__);
                        rc = SR_ERR_OPERATION_FAILED;
                        goto up_free_chg_iter;
                    }
                }
            }
        }

up_free_chg_iter:
        if (NULL != iter) {
            sr_free_change_iter(iter);
            iter = NULL;
        }

    }

    if (SR_EV_CHANGE == event) {
        rc = sr_get_changes_iter(session, xpath, &iter);

        if (SR_ERR_OK != rc) {
            return rc;
        }

        while (SR_ERR_OK == sr_get_change_next(session, iter, &op,
                                               &old_value, &new_value)) {
            node = (op == SR_OP_DELETED) ? old_value : new_value;

            /* If user changed values of leaf then node->dflt is "0". If
             * sysrepo changes value of leaf, because it has default value,
             * that node->dflt will be "1". This condition is added because of
             * booting, i.e. applying startup configuration. Every leaf that
             * has default value will be triggered with ADD operation if he
             * does not exist inside startup configuration. Its node->dflt flag
             * will be "1" then.
             */
            if ((0 == node->dflt) &&
                ((op == SR_OP_MODIFIED) ||
                 (op == SR_OP_CREATED) ||
                 (op == SR_OP_DELETED))) {
                /* get interface name from xpath */
                if (EXIT_SUCCESS != get_key_value(session, node->xpath, IF_INTERFACE_LIST,
                                                  IF_NAME_STR, &st, interface_name)) {
                    SRP_LOG_ERR("%s: %s (%s)", __func__,
                                ERR_MISSING_ELEMENT_STR, IF_NAME_STR);
                    rc =  SR_ERR_OPERATION_FAILED;
                    goto chg_free_chg_iter;
                }

                if ((op == SR_OP_MODIFIED) || (op == SR_OP_CREATED)) {
                    /* get speed */
                    fill_xpath(path, INTERFACE_SPEED_XPATH, interface_name);
                    rc = sr_get_item(session, path, 0, &speed);
                    if (SR_ERR_OK != rc) {
                        rc =  SR_ERR_OPERATION_FAILED;
                        goto chg_free_chg_iter;
                    }

                    /* get duplex */
                    fill_xpath(path, INTERFACE_DUPLEX_XPATH, interface_name);
                    rc = sr_get_item(session, path, 0, &duplex);
                    if (SR_ERR_OK != rc) {
                        rc =  SR_ERR_OPERATION_FAILED;
                        goto chg_free_srval_speed;
                    }

                    snprintf(tmp_speed, MAX_STR_LEN, "%.3lf", speed->data.decimal64_val);
                    if (EXIT_FAILURE == translate_single_string_using_map(tmp_speed, eth_speed_map_cb)) {
                        sr_set_error(session, speed->xpath,
                                     ERR_INVALID_VALUE_XPATH_STR, speed->xpath);
                        rc =  SR_ERR_OPERATION_FAILED;
                        goto chg_free_srval_duplex;
                    } else {
                        /* if link is unplugged (state DOWN), we need to prevent invalid values in datastore */
                        /* first letter in duplex is small, so the BASE_RC_YES is used */
                        if (EXIT_SUCCESS == check_link_modes(BASE_RC_YES, interface_name,
                                                             tmp_speed,
                                                             duplex->data.enum_val,
                                                             mode,
                                                             ETH_SUPPORTED_IF_MODE_STR)) {
                            if (EXIT_SUCCESS != set_eth_if_duplex_and_speed(sr_xpath_node_name(new_value->xpath),
                                                                            interface_name,
                                                                            duplex->data.enum_val,
                                                                            tmp_speed,
                                                                            0)) {
                                sr_set_error(session, new_value->xpath,
                                             ERR_SET_FUNC_FAILED_XPATH_STR,
                                             new_value->xpath);
                                rc =  SR_ERR_OPERATION_FAILED;
                                goto chg_free_srval_duplex;
                            }
                        } else {
                            SRP_LOG_ERR(DEBUG_MSG_FUN_NAME_AND_MSG, __func__,
                                        "unsupported link mode (speed and/or duplex)");
                            sr_set_error(session, new_value->xpath,
                                         "unsupported link mode (speed and/or duplex) for '%s'",
                                         new_value->xpath);
                            rc =  SR_ERR_OPERATION_FAILED;
                            goto chg_free_srval_duplex;
                        }

                    }

                    /* free duplex srval */
                    sr_free_val(duplex);
                    duplex = NULL;

                    /* free speed srval */
                    sr_free_val(speed);
                    speed = NULL;
                } else {
                    /* op == SR_OP_DELETED : operation REM using advertising - advertising will force maximum speed */
                    /* advertise will force maximum speed */
                    if (EXIT_SUCCESS != set_eth_if_duplex_and_speed(sr_xpath_node_name(old_value->xpath),
                                                                    interface_name,
                                                                    NULL, NULL, 1)) {
                        sr_set_error(session, old_value->xpath,
                                     ERR_SET_FUNC_FAILED_XPATH_STR,
                                     old_value->xpath);
                        rc =  SR_ERR_OPERATION_FAILED;
                        goto chg_free_chg_iter;
                    }
                }
            }
        }

chg_free_srval_duplex:
        if (NULL != duplex) {
            sr_free_val(duplex);
        }

chg_free_srval_speed:
        if (NULL != speed) {
            sr_free_val(speed);
        }

chg_free_chg_iter:
        if (NULL != iter) {
            sr_free_change_iter(iter);
        }

    }

    return rc;
}
