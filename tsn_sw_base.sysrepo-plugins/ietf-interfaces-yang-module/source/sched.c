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

#include "sched.h"
#include "bmp.h"
#include "config_change.h"
#include "libtsn/tsn_bridge.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <inttypes.h>

static uint32_t admin_control_list_length = 0;

static char sched_gcl_map[3][2][MAX_STR_LEN] = {
    // 3 pairs
    { SCHED_SET_GATE_STATES_IDENTITY_STR,   SCHED_SGS_PARAMS_STR },
    { SCHED_SET_HOLD_MAC_IDENTITY_STR,      SCHED_SHM_PARAMS_STR },
    { SCHED_SET_RELEASE_MAC_IDENTITY_STR,   SCHED_SRM_PARAMS_STR },
};

static char sched_op_name_map[3][2][MAX_STR_LEN] = {
    // 3 pairs
    { SCHED_SET_GATE_STATES_IDENTITY_STR,   BASE_ZERO_STR },
    { SCHED_SET_HOLD_MAC_IDENTITY_STR,      BASE_ONE_STR },
    { SCHED_SET_RELEASE_MAC_IDENTITY_STR,   BASE_TWO_STR },
};

struct pair {
    char key[MAX_STR_LEN];
    uint32_t val;
};

char sched_map[SCHED_MAX_OBJ_NUMBER][MAX_STR_LEN] =
{
    // map between object index and object name for sched module
    [SCHED_MAX_SDU_TABLE]               = SCHED_MAX_SDU_TABLE_STR,
    [SCHED_TRAFFIC_CLASS]               = SCHED_TRAFFIC_CLASS_STR,
    [SCHED_QUEUE_MAX_SDU]               = SCHED_QUEUE_MAX_SDU_STR,
    [SCHED_TRANSMISSION_OVERRUN]        = SCHED_TRANSMISSION_OVERRUN_STR,
    [SCHED_GATE_PARAMETERS]             = SCHED_GATE_PARAMETERS_STR,
    [SCHED_GATE_ENABLED]                = SCHED_GATE_ENABLED_STR,
    [SCHED_ADMIN_GATE_STATES]           = SCHED_ADMIN_GATE_STATES_STR,
    [SCHED_ADMIN_CTRL_LIST_LEN]         = SCHED_ADMIN_CTRL_LIST_LEN_STR,
    [SCHED_ADMIN_CTRL_LIST]             = SCHED_ADMIN_CTRL_LIST_STR,
    [SCHED_OPER_GATE_STATES]            = SCHED_OPER_GATE_STATES_STR,
    [SCHED_OPER_CTRL_LIST_LEN]          = SCHED_OPER_CTRL_LIST_LEN_STR,
    [SCHED_OPER_CTRL_LIST]              = SCHED_OPER_CTRL_LIST_STR,
    [SCHED_INDEX]                       = SCHED_INDEX_STR,
    [SCHED_OP_NAME]                     = SCHED_OP_NAME_STR,
    [SCHED_SGS_PARAMS]                  = SCHED_SGS_PARAMS_STR,
    [SCHED_GATE_STATES_VALUE]           = SCHED_GATE_STATES_VALUE_STR,
    [SCHED_TIME_INTERVAL_VALUE]         = SCHED_TIME_INTERVAL_VALUE_STR,
    [SCHED_ADMIN_CYCLE_TIME]            = SCHED_ADMIN_CYCLE_TIME_STR,
    [SCHED_OPER_CYCLE_TIME]             = SCHED_OPER_CYCLE_TIME_STR,
    [SCHED_ACT_NUMERATOR]               = SCHED_ACT_NUMERATOR_STR,
    [SCHED_ACT_DENOMINATOR]             = SCHED_ACT_DENOMINATOR_STR,
    [SCHED_ADMIN_CYCLE_TIME_EXT]        = SCHED_ADMIN_CYCLE_TIME_EXT_STR,
    [SCHED_OPER_CYCLE_TIME_EXT]         = SCHED_OPER_CYCLE_TIME_STR,
    [SCHED_ADMIN_BASE_TIME]             = SCHED_ADMIN_BASE_TIME_STR,
    [SCHED_OPER_BASE_TIME]              = SCHED_OPER_BASE_TIME_STR,
    [SCHED_CONFIG_CHANGE_TIME]          = SCHED_CONFIG_CHANGE_STR_TIME_STR,
    [SCHED_ABT_SECONDS]                 = SCHED_ABT_SECONDS_STR,
    [SCHED_ABT_FRACT_SECONDS]           = SCHED_ABT_FRACT_SECONDS_STR,
    [SCHED_TICK_GRANULARITY]            = SCHED_TICK_GRANULARITY_STR,
    [SCHED_CURRENT_TIME]                = SCHED_CURRENT_TIME_STR,
    [SCHED_CONFIG_PENDING]              = SCHED_CONFIG_PENDING_STR,
    [SCHED_CONFIG_CHANGE_ERROR]         = SCHED_CONFIG_CHANGE_STR_ERROR_STR,
    [SCHED_SUPPORTED_LIST_MAX]          = SCHED_SUPPORTED_LIST_MAX_STR,
    [SCHED_CONFIG_CHANGE]               = SCHED_CONFIG_CHANGE_STR,
};

/* global variable from ietf-interfaces.c */
extern int plugin_init;

struct lyd_node *new_max_sdu_entry(struct lyd_node *parent, struct ly_ctx *ctx,
                                   char *if_name, int prio, int traffic_class)
{
    struct lyd_node *max_sdu_table = NULL;
    const struct lys_module *module = NULL;
    struct port_hdl *port = NULL;
    char tmp[MAX_STR_LEN] = "";
    uint32_t tmp_uint32 = 0;

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    module = ly_ctx_load_module(ctx, SCHED_MODULE_NAME_STR, NULL);
    if (NULL == module) {
        SRP_LOG_ERR(ERROR_MSG_FUN_AND_MSG, __func__, ERR_MSG_LOAD_MODULE_STR);
        return NULL;
    }

    /* new max-sdu-table container */
    if (EXIT_FAILURE == new_container(parent, &max_sdu_table, module, SCHED_MAX_SDU_TABLE_STR)) {
        return NULL;
    }

    /* open port */
    port = tsn_port_open_by_name(if_name);
    if (NULL == port) {
        SRP_LOG_ERR(ERROR_MSG_MOD_FUNC_EL_MSG, SCHED_MODULE_NAME_STR, __func__,
                    if_name, ERR_PORT_NOT_OPEN_STR);
        return NULL;
    }

    /* traffic-class, key leaf */
    snprintf(tmp, MAX_STR_LEN, "%d", traffic_class);
    if (EXIT_FAILURE == new_node(max_sdu_table, module, SCHED_TRAFFIC_CLASS_STR, tmp)) {
        tsn_port_close(port);
        return NULL;
    }

    /* queue-max-sdu */
    if (0 == tsn_st_get_max_sdu(port, prio, &tmp_uint32)) {
        snprintf(tmp, MAX_STR_LEN, "%u", tmp_uint32);
        if (EXIT_FAILURE == new_node(max_sdu_table, module, SCHED_QUEUE_MAX_SDU_STR, tmp)) {
            tsn_port_close(port);
            return NULL;
        }
    }

    tsn_port_close(port);
    return max_sdu_table;
}

struct lyd_node *new_gate_parameters_container(struct lyd_node *parent,
                                               struct ly_ctx *ctx,
                                               char *if_name)
{
    const struct lys_module *module = NULL;
    struct lyd_node *gate_parameters = NULL;
    struct lyd_node *admin_control_list = NULL;
    struct lyd_node *params = NULL;
    struct lyd_node *cycle_time = NULL;
    struct lyd_node *base_time = NULL;
    struct port_hdl *port = NULL;
    struct tsn_st_rational_number st_rational = {0u,0u};
    struct tsn_st_ptp_time st_time = {0u, 0u};
    struct st_gate_control_entry *gcl = NULL;
    char tmp_str[MAX_STR_LEN] = "";
    char tmp_params_str[MAX_STR_LEN] = "";
    bool bvalue = false;
    uint32_t index = 0;
    uint32_t gcl_len = 1;
    uint32_t tmp_uint32 = 0;
    uint8_t admin_gate_state = 0;

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    module = ly_ctx_load_module(ctx, SCHED_MODULE_NAME_STR, NULL);
    if (NULL == module) {
        SRP_LOG_ERR(ERROR_MSG_FUN_AND_MSG, __func__, ERR_MSG_LOAD_MODULE_STR);
        return NULL;
    }

    /* new max-sdu-table container */
    if (EXIT_FAILURE == new_container(parent, &gate_parameters, module,
                                      SCHED_GATE_PARAMETERS_STR)) {
        return NULL;
    }

    /* open port */
    port = tsn_port_open_by_name(if_name);
    if (NULL == port) {
        SRP_LOG_ERR(ERROR_MSG_MOD_FUNC_EL_MSG, SCHED_MODULE_NAME_STR, __func__,
                    if_name, ERR_PORT_NOT_OPEN_STR);
        return NULL;
    }

    /* gate-enabled */
    if (0 == tsn_st_get_gate_enabled(port, &bvalue)) {
        snprintf(tmp_str, MAX_STR_LEN, "%s", btoa(bvalue));
        if (EXIT_FAILURE == new_node(gate_parameters, module,
                                     SCHED_GATE_ENABLED_STR, tmp_str)) {
            tsn_port_close(port);
            return NULL;
        }
    }

    /* admin-gate-states */
    if (0 == tsn_st_get_admin_gate_states(port, &admin_gate_state)) {
        snprintf(tmp_str, MAX_STR_LEN, "%u", admin_gate_state);
        if (EXIT_FAILURE == new_node(gate_parameters, module,
                                     SCHED_ADMIN_GATE_STATES_STR, tmp_str)) {
            tsn_port_close(port);
            return NULL;
        }
    }

    /* admin-control-list-length */
    if (0 == tsn_st_get_admin_control_list_length(port, &gcl_len)) {
        snprintf(tmp_str, MAX_STR_LEN, "%u", gcl_len);
        if (EXIT_FAILURE == new_node(gate_parameters, module,
                                     SCHED_ADMIN_CTRL_LIST_LEN_STR, tmp_str)) {
            tsn_port_close(port);
            return NULL;
        }
    }

    /* list admin_control_list */
    if ((gcl_len > 0) && (gcl_len <= MAX_NUMBER_OF_ACL)) {
        gcl = malloc(sizeof(*gcl) * gcl_len);
    } else {
        SRP_LOG_INF(ERROR_MSG_MOD_FUNC_EL_MSG, SCHED_MODULE_NAME_STR, __func__,
                    SCHED_ADMIN_CTRL_LIST_STR, ERR_NOT_CREATED_ELEMENT_STR);
    }
    if (NULL != gcl) {
        if (0 == tsn_st_get_admin_control_list(port, gcl_len, gcl)) {
            for (index = 0; index < gcl_len; index++) {
                if (EXIT_FAILURE == new_container(gate_parameters,
                                                  &admin_control_list, module,
                                                  SCHED_ADMIN_CTRL_LIST_STR)) {
                    tsn_port_close(port);
                    return NULL;
                }
                /* list admin_control_list, leaf index */
                snprintf(tmp_str, MAX_STR_LEN, "%u", index);
                if (EXIT_FAILURE == new_node(admin_control_list, module,
                                             SCHED_INDEX_STR, tmp_str)) {
                    tsn_port_close(port);
                    return NULL;
                }
                /* list admin_control_list, leaf operation-name and container [sgs|srm|shm]-params */
                switch (gcl[index].op) {
                case 0:
                    snprintf(tmp_str, MAX_STR_LEN, "%s", SCHED_SET_GATE_STATES_STR);
                    snprintf(tmp_params_str, MAX_STR_LEN, "%s", SCHED_SGS_PARAMS_STR);
                    break;
                case 1:
                    snprintf(tmp_str, MAX_STR_LEN, "%s", SCHED_SET_HOLD_MAC_STR);
                    snprintf(tmp_params_str, MAX_STR_LEN, "%s", SCHED_SHM_PARAMS_STR);
                    break;
                case 2:
                    snprintf(tmp_str, MAX_STR_LEN, "%s", SCHED_SET_RELEASE_MAC_STR);
                    snprintf(tmp_params_str, MAX_STR_LEN, "%s", SCHED_SRM_PARAMS_STR);
                    break;
                default:
                    tsn_port_close(port);
                    return NULL;
                }
                if (EXIT_FAILURE == new_node(admin_control_list, module,
                                             SCHED_OP_NAME_STR, tmp_str)) {
                    tsn_port_close(port);
                    return NULL;
                }
                if (EXIT_FAILURE == new_container(admin_control_list,
                                                  &params, module,
                                                  tmp_params_str)) {
                    tsn_port_close(port);
                    return NULL;
                }
                /* container [sgs|srm|shm]-params, leaf gate-states-value */
                snprintf(tmp_str, MAX_STR_LEN, "%u", gcl[index].gsv);
                if (EXIT_FAILURE == new_node(params, module,
                                             SCHED_GATE_STATES_VALUE_STR, tmp_str)) {
                    tsn_port_close(port);
                    return NULL;
                }
                /* container [sgs|srm|shm]-params, leaf time-interval-value */
                snprintf(tmp_str, MAX_STR_LEN, "%u", gcl[index].interval);
                if (EXIT_FAILURE == new_node(params, module,
                                             SCHED_TIME_INTERVAL_VALUE_STR, tmp_str)) {
                    tsn_port_close(port);
                    return NULL;
                }
            }
        } else {
            SRP_LOG_INF(ERROR_MSG_MOD_FUNC_EL_MSG, SCHED_MODULE_NAME_STR, __func__,
                        SCHED_ADMIN_CTRL_LIST_STR, ERR_NOT_CREATED_ELEMENT_STR);
        }
        free(gcl);
    }

    /* container admin-cycle-time */
    if (0 == tsn_st_get_admin_cycle_time(port, &st_rational)) {
        if (EXIT_FAILURE == new_container(gate_parameters, &cycle_time, module,
                                          SCHED_ADMIN_CYCLE_TIME_STR)) {
            tsn_port_close(port);
            return NULL;
        }
        /* leaf numerator */
        snprintf(tmp_str, MAX_STR_LEN, "%u", st_rational.numerator);
        if (EXIT_FAILURE == new_node(cycle_time, module,
                                     SCHED_ACT_NUMERATOR_STR, tmp_str)) {
            tsn_port_close(port);
            return NULL;
        }
        /* leaf denominator */
        snprintf(tmp_str, MAX_STR_LEN, "%u", st_rational.denominator);
        if (EXIT_FAILURE == new_node(cycle_time, module,
                                     SCHED_ACT_DENOMINATOR_STR, tmp_str)) {
            tsn_port_close(port);
            return NULL;
        }
    }

    /* leaf admin-cycle-time-extension */
    if (0 == tsn_st_get_admin_cycle_time_extension(port, &tmp_uint32)) {
        snprintf(tmp_str, MAX_STR_LEN, "%u", tmp_uint32);
        if (EXIT_FAILURE == new_node(gate_parameters, module,
                                     SCHED_ADMIN_CYCLE_TIME_EXT_STR, tmp_str)) {
            tsn_port_close(port);
            return NULL;
        }
    }

    /* container admin-base-time */
    if (0 == tsn_st_get_admin_base_time(port, &st_time)) {
        if (EXIT_FAILURE == new_container(gate_parameters, &base_time, module,
                                          SCHED_ADMIN_BASE_TIME_STR)) {
            tsn_port_close(port);
            return NULL;
        }
        /* leaf seconds */
        snprintf(tmp_str, MAX_STR_LEN, "%llu", (unsigned long long int)st_time.sec);
        if (EXIT_FAILURE == new_node(base_time, module,
                                     SCHED_ABT_SECONDS_STR, tmp_str)) {
            tsn_port_close(port);
            return NULL;
        }
        /* leaf denominator */
        snprintf(tmp_str, MAX_STR_LEN, "%u", st_time.nsec);
        if (EXIT_FAILURE == new_node(base_time, module,
                                     SCHED_ABT_FRACT_SECONDS_STR, tmp_str)) {
            tsn_port_close(port);
            return NULL;
        }
    }

    /* config-change */
    if (0 == tsn_st_get_config_change(port, &bvalue)) {
        snprintf(tmp_str, MAX_STR_LEN, "%s", btoa(bvalue));
        if (EXIT_FAILURE == new_node(gate_parameters, module,
                                     SCHED_CONFIG_CHANGE_STR, tmp_str)) {
            tsn_port_close(port);
            return NULL;
        }
    }

    tsn_port_close(port);
    return gate_parameters;
}

int validate_sched_configuration(sr_session_ctx_t *session, sr_val_t *node,
                                 struct port_hdl *port, char *if_name)
{
    uint32_t gcl_len = 0u;

    SRP_LOG_DBG(DEBUG_MSG_FUN_CALLED_WITH_ONE_PARAM, __func__, if_name);

    /* admin-control-list can be added even though link is not detected.
     * oper-control-list can be added to the port that is not UP also, only
     * advertised link modes on board has to have one value. */
    /* Check does admin-control-list exists. If admin-control-list does not
     * exist on system, configuration is not complete, so it is not possible to
     * set config-change to 'true'.
    */
    if (EXIT_FAILURE == tsn_st_get_admin_control_list_length(port, &gcl_len)) {
        SRP_LOG_ERR(ERROR_MSG_FUN_GETTER_FAILED, node->xpath);
        sr_set_error(session, node->xpath,
                     ERROR_MSG_FUN_GETTER_FAILED, BASE_ACL_LENGTH);
        return EXIT_FAILURE;
    }
    if (0 == gcl_len) {
        SRP_LOG_ERR(ERROR_MSG_ACL_MISSING, if_name);
        sr_set_error(session, node->xpath, ERROR_MSG_ACL_MISSING, if_name);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}


int new_max_sdu_state_entry(struct lyd_node **parent, char *interface_name,
                            int prio, int traffic_class)
{
    char path[MAX_STR_LEN] = "";
    char tmp[MAX_STR_LEN] = "";
    uint64_t tmp_uint64 = 0;
    struct port_hdl *port = NULL;

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    port = tsn_port_open_by_name(interface_name);
    if (NULL == port) {
        SRP_LOG_ERR(ERROR_MSG_MOD_FUNC_EL_MSG, SCHED_MODULE_NAME_STR, __func__,
                    interface_name, ERR_PORT_NOT_OPEN_STR);
        return SR_ERR_OPERATION_FAILED;
    }

    /* list max-sdu-table, leaf transmission-overrun */
    if (0 == tsn_st_get_overrun(port, prio, &tmp_uint64)) {
        /* traffic-class, key leaf */
        snprintf(tmp, MAX_STR_LEN, "%d", traffic_class);
        fill_xpath(path, SCHED_MAX_SDU_TRANSMISSION_OVERRUN_XPATH, interface_name, tmp);
        snprintf(tmp, MAX_STR_LEN, "%llu", tmp_uint64);
        if (NULL == lyd_new_path(*parent, NULL, path, tmp, 0, 0)) {
            SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                        SCHED_TRANSMISSION_OVERRUN_STR, ERR_NOT_CREATED_ELEMENT_STR);
        }

    }
    tsn_port_close(port);

    return SR_ERR_OK;
}

int new_oper_control_list_state_entry(struct lyd_node **parent,
                                      char *interface_name, int i,
                                      struct st_gate_control_entry *gcl)
{
    char path[MAX_STR_LEN] = "";
    char tmp_str[MAX_STR_LEN] = "";
    char tmp_index[MAX_STR_LEN] = "";
    char tmp_params_str[MAX_STR_LEN] = "";

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    /* list oper-control-list, leaf index */
    fill_xpath(path, SCHED_GATE_PARAMS_OCL_INDEX_XPATH, interface_name);
    snprintf(tmp_str, MAX_STR_LEN, "%d", i);
    if (NULL == lyd_new_path(*parent, NULL, path, tmp_str, 0, 0)) {
        SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                    SCHED_INDEX_STR, ERR_NOT_CREATED_ELEMENT_STR);
    }

    /* list oper-control-list, leaf operation-name */
    switch (gcl[i].op) {
    case 0:
        snprintf(tmp_str, MAX_STR_LEN, "%s", SCHED_SET_GATE_STATES_STR);
        snprintf(tmp_params_str, MAX_STR_LEN, "%s", SCHED_SGS_PARAMS_STR);
        break;
    case 1:
        snprintf(tmp_str, MAX_STR_LEN, "%s", SCHED_SET_HOLD_MAC_STR);
        snprintf(tmp_params_str, MAX_STR_LEN, "%s", SCHED_SHM_PARAMS_STR);
        break;
    case 2:
        snprintf(tmp_str, MAX_STR_LEN, "%s", SCHED_SET_RELEASE_MAC_STR);
        snprintf(tmp_params_str, MAX_STR_LEN, "%s", SCHED_SRM_PARAMS_STR);
        break;
    default:
        return SR_ERR_OPERATION_FAILED;
    }

    /* list oper-control-list, leaf operation-name */
    snprintf(tmp_index, MAX_STR_LEN, "%u", i);
    fill_xpath(path, SCHED_GATE_PARAMS_OCL_OP_NAME_XPATH, interface_name, tmp_index);
    if (NULL == lyd_new_path(*parent, NULL, path, tmp_str, 0, 0)) {
        SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                    SCHED_OP_NAME_STR, ERR_NOT_CREATED_ELEMENT_STR);
    }

    /* list oper-control-list, container [sgs|srm|shm]-params, leaf gate-states-value */
    fill_xpath(path, SCHED_GATE_PARAMS_OCL_PARAMS_GSV_XPATH, interface_name, tmp_index, tmp_params_str);
    snprintf(tmp_str, MAX_STR_LEN, "%u", gcl[i].gsv);
    if (NULL == lyd_new_path(*parent, NULL, path, tmp_str, 0, 0)) {
        SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                    SCHED_GATE_STATES_VALUE_STR, ERR_NOT_CREATED_ELEMENT_STR);
    }

    /* list oper_control_list, container [sgs|srm|shm]-params, leaf time-interval-value */
    fill_xpath(path, SCHED_GATE_PARAMS_OCL_PARAMS_TIV_XPATH, interface_name, tmp_index, tmp_params_str);
    snprintf(tmp_str, MAX_STR_LEN, "%u", gcl[i].interval);
    if (NULL == lyd_new_path(*parent, NULL, path, tmp_str, 0, 0)) {
        SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                    SCHED_TIME_INTERVAL_VALUE_STR, ERR_NOT_CREATED_ELEMENT_STR);
    }

    return SR_ERR_OK;
}

int new_state_oper_cycle_time(struct lyd_node **parent, char *interface_name,
                              struct tsn_st_rational_number *st_rational)
{
    char path[MAX_STR_LEN] = "";
    char tmp[MAX_STR_LEN] = "";

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    /* container oper-cycle-time, leaf numerator */
    fill_xpath(path, SCHED_GATE_PARAMS_OCT_NUMERATOR_XPATH, interface_name);
    snprintf(tmp, MAX_STR_LEN, "%u", st_rational->numerator);
    if (NULL == lyd_new_path(*parent, NULL, path, tmp, 0, 0)) {
        SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                    SCHED_ACT_NUMERATOR_STR, ERR_NOT_CREATED_ELEMENT_STR);
    }

    /* container oper-cycle-time, leaf denominator */
    fill_xpath(path, SCHED_GATE_PARAMS_OCT_DENOMINATOR_XPATH, interface_name);
    snprintf(tmp, MAX_STR_LEN, "%u", st_rational->denominator);
    if (NULL == lyd_new_path(*parent, NULL, path, tmp, 0, 0)) {
        SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                    SCHED_ACT_DENOMINATOR_STR, ERR_NOT_CREATED_ELEMENT_STR);
    }

    return SR_ERR_OK;
}

int new_state_oper_base_time(struct lyd_node **parent, char *interface_name,
                             struct tsn_st_ptp_time *st_time,
                             char *container_name)
{
    char path[MAX_STR_LEN] = "";
    char tmp[MAX_STR_LEN] = "";

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    /* container oper-base-time, leaf seconds */
    fill_xpath(path, SCHED_GATE_PARAMS_SECONDS_XPATH, interface_name, container_name);
    snprintf(tmp, MAX_STR_LEN, "%llu", (unsigned long long int)st_time->sec);
    if (NULL == lyd_new_path(*parent, NULL, path, tmp, 0, 0)) {
        SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                    SCHED_ABT_SECONDS_STR, ERR_NOT_CREATED_ELEMENT_STR);
    }

    /* container oper-base-time, leaf fractional-seconds */
    fill_xpath(path, SCHED_GATE_PARAMS_FRACTIONAL_SECONDS_XPATH, interface_name, container_name);
    snprintf(tmp, MAX_STR_LEN, "%u", st_time->nsec);
    if (NULL == lyd_new_path(*parent, NULL, path, tmp, 0, 0)) {
        SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                    SCHED_ABT_FRACT_SECONDS_STR, ERR_NOT_CREATED_ELEMENT_STR);
    }

    return SR_ERR_OK;
}

int new_gate_parameters_state_container(struct lyd_node **parent,
                                        char *interface_name)
{
    char path[MAX_STR_LEN] = "";
    char tmp[MAX_STR_LEN] = "";
    uint8_t usivalue = 0u;
    uint32_t tmp_ui32 = 0u;
    struct st_gate_control_entry *gcl = NULL;
    struct port_hdl *port = NULL;
    struct tsn_st_rational_number st_rational = {0u,0u};
    struct tsn_st_rational_number sup_cycle_max = {0u,0u};
    struct tsn_st_ptp_time st_time = {0u, 0u};
    bool bvalue = false;
    int i = 0;
    int tmp_int = 0;

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    port = tsn_port_open_by_name(interface_name);
    if (NULL == port) {
        SRP_LOG_ERR(ERROR_MSG_MOD_FUNC_EL_MSG, SCHED_MODULE_NAME_STR, __func__,
                    interface_name, ERR_PORT_NOT_OPEN_STR);
        return SR_ERR_OPERATION_FAILED;
    }

    /* container gate-parameters , leaf oper-gate-states */
    if (0 == tsn_st_get_oper_gate_states(port, &usivalue)) {
        fill_xpath(path, SCHED_GATE_PARAMS_OPER_GATE_STATES_XPATH, interface_name);
        snprintf(tmp, MAX_STR_LEN, "%u", usivalue);
        if (NULL == lyd_new_path(*parent, NULL, path, tmp, 0, 0)) {
            SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                        SCHED_OPER_GATE_STATES_STR, ERR_NOT_CREATED_ELEMENT_STR);
        }
    }

    /* container gate-parameters , leaf oper-control-list-length */
    if (0 == tsn_st_get_oper_control_list_length(port, &tmp_ui32)) {
        fill_xpath(path, SCHED_GATE_PARAMS_OPER_CONTROL_LIST_LENGTH_XPATH, interface_name);
        snprintf(tmp, MAX_STR_LEN, "%u", tmp_ui32);
        if (NULL == lyd_new_path(*parent, NULL, path, tmp, 0, 0)) {
            SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                        SCHED_OPER_CTRL_LIST_LEN_STR, ERR_NOT_CREATED_ELEMENT_STR);
        }
    }

    /* container gate-parameters, list oper-control-list */
    if ((tmp_ui32 > 0) && (tmp_ui32 <= MAX_NUMBER_OF_ACL)) {
        gcl = (struct st_gate_control_entry*)malloc(sizeof(*gcl) * tmp_ui32);

        if (NULL == gcl) {
            SRP_LOG_INFMSG(ERR_MEMORY_ALLOC_FAILED_STR);
            tsn_port_close(port);
            return SR_ERR_OPERATION_FAILED;
        }
    } else {
        SRP_LOG_INF(ERROR_MSG_MOD_FUNC_EL_MSG, SCHED_MODULE_NAME_STR, __func__,
                    SCHED_OPER_CTRL_LIST_STR, ERR_NOT_CREATED_ELEMENT_STR);
    }

    if (NULL != gcl) {
        if (0 == tsn_st_get_oper_control_list(port, tmp_ui32, gcl))
        {
            for (i = 0; i < (int)tmp_ui32; i++) {
                if (SR_ERR_OK != new_oper_control_list_state_entry(parent, interface_name, i, gcl)) {
                    SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                                SCHED_OPER_CTRL_LIST_STR, ERR_NOT_CREATED_ELEMENT_STR);
                }
            }
        } else {
            SRP_LOG_INF(ERROR_MSG_MOD_FUNC_EL_MSG, SCHED_MODULE_NAME_STR, __func__,
                        SCHED_OPER_CTRL_LIST_STR, ERR_NOT_CREATED_ELEMENT_STR);
        }
        free(gcl);
    }

    /* container oper-cycle-time */
    if (0 == tsn_st_get_oper_cycle_time(port, &st_rational)) {
        if (SR_ERR_OK != new_state_oper_cycle_time(parent, interface_name, &st_rational)) {
            SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                        SCHED_OPER_CTRL_LIST_STR, ERR_NOT_CREATED_ELEMENT_STR);
        }
    }


    /* container gate parameters, leaf oper-cycle-time-extension */
    if (0 == tsn_st_get_oper_cycle_time_extension(port, &tmp_ui32)) {
        fill_xpath(path, SCHED_GATE_PARAMS_OPER_CYCLE_TIME_EXTEN_XPATH, interface_name);
        snprintf(tmp, MAX_STR_LEN, "%u", tmp_ui32);
        if (NULL == lyd_new_path(*parent, NULL, path, tmp, 0, 0)) {
            SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                        SCHED_OPER_CYCLE_TIME_EXT_STR, ERR_NOT_CREATED_ELEMENT_STR);
        }
    }

    /* container gate parameters, container oper-base-time */
    if (0 == tsn_st_get_oper_base_time(port, &st_time)) {
        if (SR_ERR_OK != new_state_oper_base_time(parent, interface_name, &st_time, SCHED_OPER_BASE_TIME_STR)) {
            SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                        SCHED_OPER_BASE_TIME_STR, ERR_NOT_CREATED_ELEMENT_STR);
        }
    }

    /* container gate parameters, container config-change-time */
    if (0 == tsn_st_get_config_change_time(port, &st_time)) {
        if (SR_ERR_OK != new_state_oper_base_time(parent, interface_name, &st_time, SCHED_CONFIG_CHANGE_STR_TIME_STR)) {
            SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                        SCHED_CONFIG_CHANGE_STR_TIME_STR, ERR_NOT_CREATED_ELEMENT_STR);
        }
    }

    /* container gate parameters, leaf tick-granularity */
    if (0 == tsn_st_get_tick_granularity(port, &tmp_ui32)) {
        fill_xpath(path, SCHED_GATE_PARAMS_TICK_GRANULARITY_XPATH, interface_name);
        snprintf(tmp, MAX_STR_LEN, "%u", tmp_ui32);
        if (NULL == lyd_new_path(*parent, NULL, path, tmp, 0, 0)) {
            SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                        SCHED_TICK_GRANULARITY_STR, ERR_NOT_CREATED_ELEMENT_STR);
        }
    }

    /* container gate parameters, container current-time */
    if (0 == tsn_st_get_current_time(port, &st_time))
    {
        if (SR_ERR_OK != new_state_oper_base_time(parent, interface_name, &st_time, SCHED_CURRENT_TIME_STR))
        {
            SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                        SCHED_CURRENT_TIME_STR, ERR_NOT_CREATED_ELEMENT_STR);
        }
    }


    /* container gate parameters, leaf config-pending */
    if (0 == tsn_st_get_config_pending(port, &bvalue)) {
        fill_xpath(path, SCHED_GATE_PARAMS_CONFIG_PENDING_XPATH, interface_name);
        snprintf(tmp, MAX_STR_LEN, "%s", btoa(bvalue));
        if (NULL == lyd_new_path(*parent, NULL, path, tmp, 0, 0)) {
            SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                        SCHED_CONFIG_PENDING_STR, ERR_NOT_CREATED_ELEMENT_STR);
        }
    }

    /* container gate parameters, leaf config-change-error */
    if (0 == tsn_st_get_config_change_error(port, &tmp_int)) {
        fill_xpath(path, SCHED_GATE_PARAMS_CONFIG_CHANGE_ERROR_XPATH, interface_name);
        snprintf(tmp, MAX_STR_LEN, "%u", tmp_int);
        if (NULL == lyd_new_path(*parent, NULL, path, tmp, 0, 0)) {
            SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                        SCHED_CONFIG_CHANGE_STR_ERROR_STR, ERR_NOT_CREATED_ELEMENT_STR);
        }
    }

    /* container gate parameters, leaf supported-list-max */
    if (0 == tsn_st_get_supported_list_max(port, &tmp_ui32)) {
        fill_xpath(path, SCHED_GATE_PARAMS_SUPPORTED_LIST_MAX_XPATH, interface_name);
        snprintf(tmp, MAX_STR_LEN, "%u", tmp_ui32);
        if (NULL == lyd_new_path(*parent, NULL, path, tmp, 0, 0)) {
            SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                        SCHED_SUPPORTED_LIST_MAX_STR, ERR_NOT_CREATED_ELEMENT_STR);
        }
    }

    /* container supported-cycle-max */
    if (0 == tsn_st_get_supported_cycle_max(port, &sup_cycle_max)) {
        /* leaf numerator */
        fill_xpath(path, SCHED_GATE_PARAMS_SCM_NUMERATOR_XPATH, interface_name);
        snprintf(tmp, MAX_STR_LEN, "%u", sup_cycle_max.numerator);
        if (NULL == lyd_new_path(*parent, NULL, path, tmp, 0, 0)) {
            SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                        path, ERR_NOT_CREATED_ELEMENT_STR);
        }
        /* leaf denominator */
        fill_xpath(path, SCHED_GATE_PARAMS_SCM_DENOMINATOR_XPATH, interface_name);
        snprintf(tmp, MAX_STR_LEN, "%u", sup_cycle_max.denominator);
        if (NULL == lyd_new_path(*parent, NULL, path, tmp, 0, 0)) {
            SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                        path, ERR_NOT_CREATED_ELEMENT_STR);
        }
    }

    /* leaf supported-interval-max */
    if (0 == tsn_st_get_supported_interval_max(port, &tmp_ui32)) {
        fill_xpath(path, SCHED_GATE_PARAMS_SUP_INTRVL_MAX_XPATH, interface_name);
        snprintf(tmp, MAX_STR_LEN, "%u", tmp_ui32);
        if (NULL == lyd_new_path(*parent, NULL, path, tmp, 0, 0)) {
            SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                        path, ERR_NOT_CREATED_ELEMENT_STR);
        }
    }

    tsn_port_close(port);

    return SR_ERR_OK;
}

int interface_sched_acl_length_cb(sr_session_ctx_t *session,
                                  const char *module_name, const char *xpath,
                                  sr_event_t event, uint32_t request_id,
                                  void *private_data)
{
    (void)private_data;
    (void)module_name;
    (void)request_id;

    int rc = SR_ERR_OK;
    sr_change_oper_t op = {0};
    sr_xpath_ctx_t st = {0};
    sr_change_iter_t *iter = NULL;
    sr_val_t *old_value = NULL;
    sr_val_t *new_value = NULL;
    sr_val_t *node = NULL;
    sr_val_t *gate_param_values = NULL;
    sr_val_t *acl_values = NULL;
    struct port_hdl *port = NULL;
    char interface_name[MAX_STR_LEN] = "";
    char tmp_path[MAX_STR_LEN] = "";
    size_t acl_count = 0;
    size_t gate_param_count = 0;
    uint32_t gcl_len = 0u;
    uint32_t old_sched_index = 0u;
    uint32_t valid_gcl_len = 0u;// 0 is valid value
    uint32_t num_acl = 0;
    int i = 0;
    int ret = SR_ERR_OK;

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

        if ((op == SR_OP_MODIFIED) || (op == SR_OP_CREATED)) {
            gcl_len = node->data.uint32_val;
            /* check number of admin-control-lists */
            /* reset acl_count for next change */
            acl_count = 0;
            fill_xpath(tmp_path, SCHED_GATE_PARAMS_CHILD_XPATH, interface_name);
            ret = sr_get_items(session, tmp_path, 0, 0, &gate_param_values, &gate_param_count);
            if ((SR_ERR_OK != ret) && (SR_ERR_NOT_FOUND != ret)) {
                // if item is not found the count is 0
                SRP_LOG_ERR(ERR_GATE_PARAMETERS_GET_STR, interface_name);
                sr_set_error(session, node->xpath, ERR_GATE_PARAMETERS_GET_STR,
                             interface_name);
                return SR_ERR_OPERATION_FAILED;
            }

            i = 0;
            for (i = 0; i < (int)gate_param_count; i++) {
                if (NULL != strstr(gate_param_values[i].xpath, "admin-control-list[index")) {
                    num_acl++;
                }
            }
            sr_free_values(gate_param_values, gate_param_count);

            if (SR_EV_ABORT == event) {
                num_acl = gcl_len;
            }

            /* if number of admin-control-list is different from
             * admin-control-list length, return FAILTURE
             */
            if (gcl_len != num_acl) {
                SRP_LOG_ERR(ERR_ACL_WRONG_LEGTH_STR, interface_name);
                sr_set_error(session, node->xpath, ERR_ACL_WRONG_LEGTH_STR,
                             interface_name);
                return SR_ERR_OPERATION_FAILED;
            }

            num_acl = 0;

            /* check number of admin-control-lists */
            fill_xpath(tmp_path, SCHED_GATE_PARAMS_ACL_CHILD_XPATH, interface_name);
            ret = sr_get_items(session, tmp_path, 0, 0, &acl_values, &acl_count);
            if ((SR_ERR_OK != ret) && (SR_ERR_NOT_FOUND != ret)) {
                // if item is not found the count is 0
                SRP_LOG_ERR(ERR_ACL_GET_STR, interface_name);
                sr_set_error(session, node->xpath, ERR_ACL_GET_STR,
                             interface_name);
                return SR_ERR_OPERATION_FAILED;
            }

            i = 0;
            if (SR_ERR_NOT_FOUND != ret) {
                old_sched_index = 0;
                for (i = 0; i < (int)acl_count; i++) {
                    if (sr_xpath_node_name_eq(acl_values[i].xpath, SCHED_INDEX_STR)) {
                        if ((acl_values[i].data.uint32_val - old_sched_index) > 1) {
                            SRP_LOG_ERR(ERR_ACL_WRONG_INDEX_STR, interface_name);
                            sr_set_error(session, acl_values[i].xpath,
                                         ERR_ACL_WRONG_INDEX_STR, interface_name);
                            return SR_ERR_OPERATION_FAILED;
                        }
                        old_sched_index = acl_values[i].data.uint32_val;
                        if (gcl_len == (acl_values[i].data.uint32_val + 1)) {
                            valid_gcl_len++;
                            break;
                        }
                    }
                }
            }

            sr_free_values(acl_values, acl_count);

            /* open port by name and check is feature supported on the port */
            port = tsn_port_open_by_name(interface_name);
            if (NULL == port) {
                SRP_LOG_ERR(ERR_PORT_NOT_OPEN_XPATH_STR, interface_name);
                sr_set_error(session, node->xpath, ERR_PORT_NOT_OPEN_XPATH_STR,
                             node->xpath);
                return SR_ERR_OPERATION_FAILED;
            }

            if (!tsn_is_feat_supported(port, FEAT_ST)) {
                SRP_LOG_ERR(ERR_FEATURE_NOT_SUPPORTED_XPATH_STR, "sched", interface_name);
                sr_set_error(session, node->xpath,
                             ERR_FEATURE_NOT_SUPPORTED_XPATH_STR,
                             "sched", node->xpath);
                tsn_port_close(port);
                return SR_ERR_OPERATION_FAILED;
            }

            if (!valid_gcl_len) {
                if (0 == gcl_len) {
                    // when gcl_len=0, and there is no acl list in config, valid situation
                    /* set new admin control list length */
                    if (tsn_st_set_admin_control_list_length(port, gcl_len)) {
                        SRP_LOG_ERR(ERR_SET_FUNC_FAILED_XPATH_STR, node->xpath);
                        sr_set_error(session, node->xpath,
                                     ERR_SET_FUNC_FAILED_XPATH_STR, node->xpath);
                        tsn_port_close(port);
                        return SR_ERR_OPERATION_FAILED;
                    }
                    admin_control_list_length = gcl_len;
                    tsn_port_close(port);
                } else {
                    SRP_LOG_ERR(ERR_ACL_WRONG_LEGTH_STR, interface_name);
                    sr_set_error(session, node->xpath, ERR_ACL_WRONG_LEGTH_STR,
                                 interface_name);
                    return SR_ERR_OPERATION_FAILED;
                }
            } else {
                /* set new admin control list length */
                if (tsn_st_set_admin_control_list_length(port, gcl_len)) {
                    SRP_LOG_ERR(ERR_SET_FUNC_FAILED_XPATH_STR, node->xpath);
                    sr_set_error(session, node->xpath,
                                 ERR_SET_FUNC_FAILED_XPATH_STR, node->xpath);
                    tsn_port_close(port);
                    return SR_ERR_OPERATION_FAILED;
                }
                admin_control_list_length = gcl_len;
                tsn_port_close(port);
            }
        }
        if (op == SR_OP_DELETED) {
            SRP_LOG_ERR(ERR_REM_OP_NOT_SUPPORTED_XPATH_STR, node->xpath);
            sr_set_error(session, node->xpath,
                         ERR_REM_OP_NOT_SUPPORTED_XPATH_STR, node->xpath);
            return SR_ERR_OPERATION_FAILED;
        }
    }

    sr_free_val(old_value);
    sr_free_val(new_value);
    sr_free_change_iter(iter);
    node = NULL;

    return rc;
}

int interface_sched_acl_cb(sr_session_ctx_t *session, const char *module_name,
                           const char *xpath, sr_event_t event,
                           uint32_t request_id, void *private_data)
{
    (void)event;
    (void)module_name;
    (void)request_id;
    (void)private_data;

    int rc = SR_ERR_OK;
    sr_change_oper_t op = {0};
    sr_xpath_ctx_t st = {0};
    sr_change_iter_t *iter = NULL;
    sr_val_t *old_node = NULL;
    sr_val_t *new_node = NULL;
    sr_val_t *node = NULL;
    sr_val_t *acl_values = NULL;
    sr_val_t *tmp_value = NULL;
    struct port_hdl *port = NULL;
    struct st_gate_control_entry *gcl = NULL;
    char interface_name[MAX_STR_LEN] = "";
    char tmp_path[MAX_STR_LEN] = "";
    char tmp_gcl_params[MAX_STR_LEN] = "";
    char tmp_op_name[MAX_STR_LEN] = "";
    char tmp_sched_index[MAX_STR_LEN] = "";
    int ret = 0;
    int i = 0;
    size_t acl_count = 0;
    uint32_t number_of_acl = 0;
    uint32_t gcl_len = 0u;
    uint32_t sched_index = 0u;
    uint32_t time_interval_value = 0u;
    uint32_t granularity = 0;
    uint8_t gate_state_value = 0u;
    sr_val_t *config_change = NULL;

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

        if (NULL == node) {
            break;
        }

        /* get interface name from xpath */
        if (EXIT_SUCCESS != get_key_value(session, node->xpath, IF_INTERFACE_LIST,
                                          IF_NAME_STR, &st, interface_name))
        {
            SRP_LOG_ERR("%s: %s (%s)", __func__,
                        ERR_MISSING_ELEMENT_STR, IF_NAME_STR);
            return SR_ERR_OPERATION_FAILED;
        }

        /* Find admin-control-list-length */
        ret = get_item_value_cb(session, &tmp_value,
                                SCHED_GATE_PARAMS_ACL_LENGTH_XPATH,
                                interface_name);
        if (EXIT_FAILURE == ret)
            break;

        admin_control_list_length = tmp_value->data.uint32_val;
        sr_free_val(tmp_value);

        if ((op == SR_OP_CREATED) &&
            (sr_xpath_node_name_eq(node->xpath, SCHED_INDEX_STR))) {
            sched_index = node->data.uint32_val;
            /* open port by name and check is feature supported on the port */
            port = tsn_port_open_by_name(interface_name);
            if (NULL == port) {
                SRP_LOG_ERR(ERR_PORT_NOT_OPEN_XPATH_STR, interface_name);
                sr_set_error(session, node->xpath, ERR_PORT_NOT_OPEN_XPATH_STR,
                             interface_name);
                return SR_ERR_OPERATION_FAILED;
            }

            do {
                if (!tsn_is_feat_supported(port, FEAT_ST)) {
                    SRP_LOG_ERR(ERR_ACL_MAX_VALUE_STR, "sched", interface_name);
                    sr_set_error(session, node->xpath,
                                 ERR_FEATURE_NOT_SUPPORTED_XPATH_STR,
                                 "sched", interface_name);
                    ret = 1;
                    break;
                }

                /* check admin-control-list-length is equal to number of
                 * admin-control-list entries
                 */
                ret = get_item_value_cb(session, &tmp_value,
                                        SCHED_GATE_PARAMS_ACL_LENGTH_XPATH, interface_name);
                if (ret)
                    break;

                gcl_len = tmp_value->data.uint32_val;

                sr_free_val(tmp_value);
                if (gcl_len > MAX_NUMBER_OF_ACL) {
                    SRP_LOG_ERR(ERR_ACL_MAX_VALUE_STR, interface_name);
                    sr_set_error(session, node->xpath, ERR_ACL_MAX_VALUE_STR,
                                 interface_name);
                    ret = 1;
                    break;
                }

                /* check sched index from admin control list */
                if (sched_index >= gcl_len) {
                    SRP_LOG_ERR(ERR_GCL_INDEX_OUT_OF_BONDS_STR, interface_name);
                    sr_set_error(session, node->xpath,
                                 ERR_GCL_INDEX_OUT_OF_BONDS_STR, interface_name);
                    ret = 1;
                    break;
                }
                snprintf(tmp_sched_index, MAX_STR_LEN, "%u", sched_index);
                /* set new admin control list length */
                ret = tsn_st_set_admin_control_list_length(port, gcl_len);
                if (ret) {
                    SRP_LOG_ERR(ERR_SET_FUNC_FAILED_XPATH_STR, node->xpath);
                    sr_set_error(session, node->xpath,
                                 ERR_SET_FUNC_FAILED_XPATH_STR, node->xpath);
                    break;
                }
                admin_control_list_length = gcl_len;

                ret = get_item_value_cb(session, &config_change,
                                        SCHED_GATE_PARAMS_CONFIG_CHANGE_XPATH,
                                        interface_name);
                if (EXIT_FAILURE == ret) break;

                /* validate sched configuration */
                if (config_change->data.bool_val) {
                    ret = validate_sched_configuration(session, node, port, interface_name);
                    if (EXIT_FAILURE == ret)
                        break;
                }
                sr_free_val(config_change);

                /* find sched operation-name from admin control list */
                ret = get_item_value_cb(session, &tmp_value,
                                        SCHED_GATE_PARAMS_ACL_OP_NAME_XPATH,
                                        interface_name, tmp_sched_index);
                if (ret)
                    break;

                snprintf(tmp_gcl_params, MAX_STR_LEN, "%s", tmp_value->data.identityref_val);

                /* map the operation name to integer value for gcl.op */
                snprintf(tmp_op_name, MAX_STR_LEN, "%s", tmp_value->data.identityref_val);
                ret = translate_single_string_using_map(tmp_op_name, sched_op_name_map);
                if (ret) {
                    SRP_LOG_ERRMSG(ERR_SCHED_MAP_FAILED_STR);
                    sr_set_error(session, node->xpath, ERR_SCHED_MAP_FAILED_STR);
                    break;
                }
                sr_free_val(tmp_value);

                /* map the proper container [sgs|srm|shm]-params by operation name value */
                ret = translate_single_string_using_map(tmp_gcl_params, sched_gcl_map);
                if (ret) {
                    SRP_LOG_ERRMSG(ERR_SCHED_MAP_FAILED_STR);
                    sr_set_error(session, node->xpath, ERR_SCHED_MAP_FAILED_STR);
                    break;
                }

                /* find sched gate-state-value from [sgs|srm|shm]-params */
                ret = get_item_value_cb(session, &tmp_value,
                                        SCHED_GATE_PARAMS_ACL_PARAMS_GSV_XPATH,
                                        interface_name, tmp_sched_index,
                                        tmp_gcl_params);
                if (ret)
                    break;

                gate_state_value = tmp_value->data.uint8_val;
                sr_free_val(tmp_value);

                /* find sched time-interval-value from [sgs|srm|shm]-params */
                ret = get_item_value_cb(session, &tmp_value,
                                        SCHED_GATE_PARAMS_ACL_PARAMS_TIV_XPATH,
                                        interface_name, tmp_sched_index,
                                        tmp_gcl_params);
                if (ret)
                    break;

                time_interval_value = tmp_value->data.uint32_val;
                /* Check is time interval value aligned with tick granularity.
                 * time-interval-value needs to be an integer multiple of
                 * tick-granularity.
                 */
                ret = tsn_st_get_tick_granularity(port, &granularity);
                if (ret) {
                    SRP_LOG_ERR(ERR_GET_FUNC_FAILED_XPATH_STR, tmp_value->xpath);
                    sr_set_error(session, node->xpath, ERR_GET_FUNC_FAILED_XPATH_STR,
                                 tmp_value->xpath);
                    break;
                }
                if (0 != (time_interval_value % (granularity/10)))
                {
                    SRP_LOG_ERR(ERR_GRANULARITY_INTERVALS_STR, granularity/10, tmp_value->xpath);
                    sr_set_error(session, node->xpath,
                                 ERR_GRANULARITY_INTERVALS_STR,
                                 granularity/10, tmp_value->xpath);
                    ret = 1;
                    break;
                }
                sr_free_val(tmp_value);

                /* set new admin control list length */
                ret = tsn_st_get_admin_control_list_length(port, &gcl_len);
                if (ret) {
                    SRP_LOG_ERR(ERR_GETTER_EXT_FMT_STR, SCHED_ADMIN_CTRL_LIST_LEN_STR,
                                LIT_VAL_STR, LIT_XPATH_STR,
                                node->xpath, tsn_lib_get_known_ernno_msg(ret));
                    sr_set_error(session, node->xpath, ERR_GETTER_EXT_FMT_STR,
                                 SCHED_ADMIN_CTRL_LIST_LEN_STR,
                                 LIT_VAL_STR, LIT_XPATH_STR, node->xpath,
                                 tsn_lib_get_known_ernno_msg(ret));
                    break;
                }

                if ((op & SR_OP_CREATED) &&
                    ((0 == gcl_len) || (MAX_NUMBER_OF_ACL < gcl_len))) {
                    SRP_LOG_ERR(ERR_GCL_LEN_MSG_STR, node->xpath);
                    sr_set_error(session, node->xpath, ERR_GCL_LEN_MSG_STR,
                                 interface_name);
                    break;
                }

                gcl = (struct st_gate_control_entry*)malloc(sizeof(*gcl) * (sched_index + 1));
                if (NULL != gcl) {
                    /* if index is 0 for ADD operation that means current admin
                     * control list is empty, so we do not need to get current
                     * values
                     */
                    if (0 != sched_index) {
                        /* get current values */
                        ret = tsn_st_get_admin_control_list(port, sched_index, gcl);
                        if (ret) {
                            SRP_LOG_ERR(ERR_GETTER_EXT_FMT_STR,
                                        SCHED_ADMIN_CTRL_LIST_STR,
                                        LIT_ENTRIES_STR, LIT_XPATH_STR,
                                        node->xpath,
                                        tsn_lib_get_known_ernno_msg(ret));
                            sr_set_error(session, node->xpath,
                                         ERR_GETTER_EXT_FMT_STR,
                                         SCHED_ADMIN_CTRL_LIST_STR,
                                         LIT_ENTRIES_STR,
                                         LIT_XPATH_STR,
                                         node->xpath,
                                         tsn_lib_get_known_ernno_msg(ret));
                            free(gcl);
                            break;
                        }
                    }

                    gcl[sched_index].op = (uint8_t)strtoul(tmp_op_name, NULL, 10);
                    gcl[sched_index].gsv = gate_state_value;
                    gcl[sched_index].interval = time_interval_value;

                    ret = tsn_st_set_admin_control_list(port, (sched_index + 1), gcl);
                    if (ret) {
                        SRP_LOG_ERR(ERR_SET_FUNC_FAILED_XPATH_STR, node->xpath);
                        sr_set_error(session, node->xpath,
                                     ERR_SET_FUNC_FAILED_XPATH_STR, node->xpath);
                        free(gcl);
                        break;
                    }
                } else {
                    SRP_LOG_ERR(ERR_MEMORY_ALLOC_FAILED_FOR_STR, node->xpath);
                    sr_set_error(session, node->xpath,
                                 ERR_MEMORY_ALLOC_FAILED_FOR_STR, node->xpath);
                    ret = 1;
                    break;
                }

            } while (0);

            if (ret) {
                tsn_port_close(port);
                return SR_ERR_OPERATION_FAILED;
            }

            tsn_port_close(port);
            /* free resources */
            if (NULL != gcl) {
                free(gcl);
            }
        }
        if (op == SR_OP_DELETED) {
            fill_xpath(tmp_path, SCHED_GATE_PARAMS_CHILD_XPATH, interface_name);
            ret = sr_get_items(session, tmp_path, 0, 0, &acl_values, &acl_count);
            if ((SR_ERR_OK != ret) && (SR_ERR_NOT_FOUND != ret)) {
                // if item is not found the count is 0
                SRP_LOG_ERR(ERR_GATE_PARAMETERS_GET_STR, tmp_path);
                sr_set_error(session, node->xpath, ERR_GATE_PARAMETERS_GET_STR,
                             interface_name);
                return SR_ERR_OPERATION_FAILED;
            }

            i = 0;
            for (i = 0; i < (int)acl_count; i++) {
                if (NULL != strstr(acl_values[i].xpath, "admin-control-list[index")) {
                    number_of_acl++;
                }
            }
            sr_free_values(acl_values, acl_count);

            if (admin_control_list_length != number_of_acl) {
                SRP_LOG_ERR(ERR_ACL_WRONG_LEGTH_STR, node->xpath);
                sr_set_error(session, node->xpath, ERR_ACL_WRONG_LEGTH_STR,
                             interface_name);
                return SR_ERR_OPERATION_FAILED;
            }
            number_of_acl = 0;
        }
    }

    sr_free_val(old_node);
    sr_free_val(new_node);
    node = NULL;
    sr_free_change_iter(iter);

    return rc;
}

int interface_sched_acl_mod_cb(sr_session_ctx_t *session,
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
    sr_val_t *old_node = NULL;
    sr_val_t *new_node = NULL;
    sr_val_t *node = NULL;
    sr_val_t *tmp_value = NULL;
    struct port_hdl *port = NULL;
    struct st_gate_control_entry *gcl = NULL;
    char interface_name[MAX_STR_LEN] = "";
    char tmp_op_name[MAX_STR_LEN] = "";
    char tmp_sched_index[MAX_STR_LEN] = "";
    int ret = 0;
    uint32_t gcl_len = 0u;
    uint32_t acl_len = 0u;
    uint32_t sched_index = 0u;
    uint32_t granularity = 0;
    struct pair acl_pair[BASE_EIGHT];
    int i = 0;
    int obj_index = 0;
    bool length_changed = false;
    bool tmp_length_changed = true;
    int clean_flag = 0;

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    if ((SR_EV_DONE == event) || (0 == plugin_init)) {
        SRP_LOG_DBG(DEBUG_MSG_WITH_TWO_PARAM, DBG_APPLYING_CHANGES_MSG, __func__);
        return SR_ERR_OK;
    }
    /* find admin-control-list-length old node value */
    rc = sr_get_changes_iter(session,
                             "/ietf-interfaces:interfaces/interface/ieee802-dot1q-sched:gate-parameters/ieee802-dot1q-sched:admin-control-list-length",
                             &iter);
    if (SR_ERR_OK != rc) {
        return rc;
    }

    if (iter) {
        while (SR_ERR_OK == sr_get_change_next(session, iter, &op,
                                               &old_node, &new_node))
        {
            if (NULL == old_node) {
                break;
            }
            length_changed = true;

            /* get interface name from xpath */
            if (EXIT_SUCCESS != get_key_value(session, old_node->xpath, IF_INTERFACE_LIST,
                                              IF_NAME_STR, &st, interface_name)) {
                SRP_LOG_ERR("%s: %s (%s)", __func__,
                            ERR_MISSING_ELEMENT_STR, IF_NAME_STR);
                return SR_ERR_OPERATION_FAILED;
            }
            if (sr_xpath_node_name_eq(old_node->xpath, SCHED_ADMIN_CTRL_LIST_LEN_STR)) {
                snprintf(acl_pair[i].key, MAX_STR_LEN, "%s", interface_name);
                acl_pair[i].val = old_node->data.uint32_val;
                i++;
            }
        }
        sr_free_val(old_node);
        sr_free_val(new_node);
        sr_free_change_iter(iter);
    }

    rc = sr_get_changes_iter(session, xpath, &iter);
    if (SR_ERR_OK != rc) {
        return rc;
    }

    while (SR_ERR_OK == sr_get_change_next(session, iter, &op,
                                           &old_node, &new_node)) {
        clean_flag = 1;
        node = (op == SR_OP_DELETED) ? old_node : new_node;

        /* get interface name from xpath */
        if (EXIT_SUCCESS != get_key_value(session, node->xpath, IF_INTERFACE_LIST,
                                          IF_NAME_STR, &st, interface_name)) {
            SRP_LOG_ERR("%s: %s (%s)", __func__, ERR_MISSING_ELEMENT_STR, IF_NAME_STR);
            return SR_ERR_OPERATION_FAILED;
        }
        if ((op == SR_OP_MODIFIED)) {
            /* open port by name and check is feature supported on the port */
            port = tsn_port_open_by_name(interface_name);
            if (NULL == port) {
                SRP_LOG_ERR(ERR_PORT_NOT_OPEN_XPATH_STR, interface_name);
                sr_set_error(session, node->xpath,
                             ERR_PORT_NOT_OPEN_XPATH_STR, interface_name);
                return SR_ERR_OPERATION_FAILED;
            }

            do {
                if (!tsn_is_feat_supported(port, FEAT_ST)) {
                    SRP_LOG_ERR(ERR_FEATURE_NOT_SUPPORTED_XPATH_STR,
                                "sched", interface_name);
                    sr_set_error(session, node->xpath,
                                 ERR_FEATURE_NOT_SUPPORTED_XPATH_STR,
                                 "sched", interface_name);
                    ret = 1;
                    break;
                }

                /* find sched index from admin control list */
                if (EXIT_SUCCESS != get_key_value(session, node->xpath,
                                                  SCHED_ADMIN_CTRL_LIST_STR,
                                                  SCHED_INDEX_STR, &st,
                                                  tmp_sched_index)) {
                    SRP_LOG_ERR("%s: %s (%s)", __func__,
                                ERR_MISSING_ELEMENT_STR, SCHED_INDEX_STR);
                    return SR_ERR_OPERATION_FAILED;
                }
                sched_index = strtol(tmp_sched_index, NULL, 10);

                /* get admin control list length */
                ret = tsn_st_get_admin_control_list_length(port, &gcl_len);
                if (ret) {
                    SRP_LOG_ERR(ERR_GETTER_EXT_FMT_STR,
                                SCHED_ADMIN_CTRL_LIST_LEN_STR,
                                LIT_VAL_STR, LIT_XPATH_STR, node->xpath,
                                tsn_lib_get_known_ernno_msg(ret));
                    sr_set_error(session, node->xpath,
                                 ERR_GETTER_EXT_FMT_STR,
                                 SCHED_ADMIN_CTRL_LIST_LEN_STR,
                                 LIT_VAL_STR, LIT_XPATH_STR, node->xpath,
                                 tsn_lib_get_known_ernno_msg(ret));
                    break;
                }
                /* if length is change find port and value of acl length,
                 * else use the current acl-length
                 */
                if (length_changed) {
                    for (i = 0; i < BASE_EIGHT; i++) {
                        if (0 == strncmp(interface_name, acl_pair[i].key,
                                         MAX_STR_LEN)) {
                            acl_len = acl_pair[i].val;
                            break;
                        }
                        tmp_length_changed = false;
                    }
                }
                if (false == tmp_length_changed) {
                    /* Find admin-control-list-length */
                    ret = get_item_value_cb(session, &tmp_value,
                                            SCHED_GATE_PARAMS_ACL_LENGTH_XPATH,
                                            interface_name);
                    if (EXIT_FAILURE == ret)
                        break;

                    acl_len = tmp_value->data.uint32_val;
                    sr_free_val(tmp_value);
                }
                tmp_length_changed = true;
                /* acl_len or gcl_len? Which value to use for malloc and
                 * tsn_st_get_admin_control_list? This depends if
                 * callback_sched_admin_control_list is called with REM or ADD
                 * operation. This callback is just for MOD operations. ADD and
                 * REM operations of these objects are covered by
                 * callback_sched_admin_control_list. We will always use a
                 * lesser value. That will be enough for MOD operation. This is
                 * important in situations when we ADD/REM admin-control-list
                 * entry and MOD some of the entry in the same edit-config
                 * operation.
                 */
                if (acl_len > gcl_len) {
                    gcl_len = acl_len;
                }
                if ((op == SR_OP_MODIFIED) && (0 == gcl_len)) {
                    SRP_LOG_ERR(ERR_GCL_LEN_MSG_STR, interface_name);
                    sr_set_error(session, node->xpath, ERR_GCL_LEN_MSG_STR,
                                 interface_name);
                    break;
                }

                gcl = (struct st_gate_control_entry*)malloc(sizeof(*gcl)*gcl_len);
                if (NULL != gcl) {
                    /* get current values */
                    ret = tsn_st_get_admin_control_list(port, gcl_len, gcl);
                    if (ret) {
                        SRP_LOG_ERR(ERR_GCL_LEN_MSG_STR, interface_name);
                        sr_set_error(session, node->xpath, ERR_GCL_LEN_MSG_STR,
                                     interface_name);
                        break;
                    }

                    if (sched_index >= gcl_len) {
                        SRP_LOG_ERR(ERR_GCL_INDEX_OUT_OF_BONDS_STR,
                                    interface_name);
                        sr_set_error(session, node->xpath,
                                     ERR_GCL_INDEX_OUT_OF_BONDS_STR,
                                     interface_name);
                        break;
                    }

                    /* modify the proper object of the st_gate_control_entry structure */
                    obj_index = find_obj_index_by_name(sr_xpath_node_name(node->xpath),
                                                       sched_map, SCHED_MAX_OBJ_NUMBER);
                    switch (obj_index) {
                    case SCHED_GATE_STATES_VALUE:
                        gcl[sched_index].gsv = node->data.uint8_val;
                        break;
                    case SCHED_TIME_INTERVAL_VALUE:
                        gcl[sched_index].interval = node->data.uint32_val;
                        /* Check is time interval value aligned with tick
                         * granularity. time-interval-value needs to be an
                         * integer multiple of tick-granularity.
                         */
                        ret = tsn_st_get_tick_granularity(port, &granularity);
                        if (ret) {
                            SRP_LOG_ERR(ERR_GET_FUNC_FAILED_XPATH_STR, node->xpath);
                            sr_set_error(session, node->xpath,
                                         ERR_GET_FUNC_FAILED_XPATH_STR,
                                         node->xpath);
                            break;
                        }
                        if (0 != (gcl[sched_index].interval % (granularity/10))) {
                            SRP_LOG_ERR(ERR_GRANULARITY_INTERVALS_STR,
                                        granularity/10, node->xpath);
                            sr_set_error(session, node->xpath,
                                         ERR_GRANULARITY_INTERVALS_STR,
                                         granularity/10, node->xpath);
                            ret = 1;
                            break;
                        }
                        break;
                    case SCHED_OP_NAME:
                        /* map the operation name to integer value for gcl.op */
                        snprintf(tmp_op_name, MAX_STR_LEN, "%s", node->data.string_val);
                        if (EXIT_FAILURE == translate_single_string_using_map(tmp_op_name,
                                                                              sched_op_name_map)) {
                            SRP_LOG_ERRMSG(ERR_SCHED_MAP_FAILED_STR);
                            sr_set_error(session, node->xpath, ERR_SCHED_MAP_FAILED_STR);
                            ret = 1;
                            break;
                        }
                        gcl[sched_index].op = strtol(tmp_op_name, NULL, 10);
                        break;
                    }
                    if (ret) {
                        free(gcl);
                        break;
                    }

                    ret = tsn_st_set_admin_control_list(port, gcl_len, gcl);
                    if (ret) {
                        SRP_LOG_ERR(ERR_SET_FUNC_FAILED_XPATH_STR, node->xpath);
                        sr_set_error(session, node->xpath,
                                     ERR_SET_FUNC_FAILED_XPATH_STR,
                                     node->xpath);
                        free(gcl);
                        break;
                    }
                } else {
                    SRP_LOG_ERR(ERR_MEMORY_ALLOC_FAILED_FOR_STR, node->xpath);
                    sr_set_error(session, node->xpath,
                                 ERR_MEMORY_ALLOC_FAILED_FOR_STR, node->xpath);
                    ret = 1;
                    break;
                }

            } while (0);

            if (ret) {
                tsn_port_close(port);
                return SR_ERR_OPERATION_FAILED;
            }
            if (NULL != port) {
                tsn_port_close(port);
            }

            /* free resources */
            if (NULL != gcl) {
                free(gcl);
            }
        }
    }
    if (1 == clean_flag) {
        sr_free_val(old_node);
        sr_free_val(new_node);
        node = NULL;
        sr_free_change_iter(iter);
    }

    return rc;
}

int interface_sched_elements_cb(sr_session_ctx_t *session,
                                const char *module_name, const char *xpath,
                                sr_event_t event, uint32_t request_id,
                                void *private_data)
{
    (void)event;
    (void)module_name;
    (void)request_id;
    (void)private_data;

    tsn_return_value tsn_ret_val = E_TSN_LIB_NO_ERROR;
    int ret = SR_ERR_OK;
    sr_change_oper_t op = {0};
    sr_xpath_ctx_t st = {0};
    sr_change_iter_t *iter = NULL;
    sr_val_t *old_value = NULL;
    sr_val_t *new_value = NULL;
    sr_val_t *node = NULL;
    sr_val_t *tmp_value = NULL;
    struct port_hdl *port = NULL;
    struct tsn_st_ptp_time st_time = {0u, 0u};
    struct tsn_st_rational_number st_rational = {0u, 0u};
    char interface_name[MAX_STR_LEN] = "";
    char tmp_traffic_class[MAX_STR_LEN] = "";
    char err_msg[LONG_STR_LEN] = "";
    uint32_t act_ext = 0u;
    uint8_t traffic_class = 0;
    int obj_index = 0;
    uint32_t port_id;

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    if ((SR_EV_DONE == event) || (0 == plugin_init)) {
        SRP_LOG_DBG(DEBUG_MSG_WITH_TWO_PARAM, DBG_APPLYING_CHANGES_MSG, __func__);
        return SR_ERR_OK;
    }

    ret = sr_get_changes_iter(session, xpath, &iter);

    if (SR_ERR_OK != ret) {
        return ret;
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
            /* open port by name and check is feature suppported on the port */
            port = tsn_port_open_by_name(interface_name);
            if (NULL == port) {
                SRP_LOG_ERR(ERR_PORT_NOT_OPEN_XPATH_STR, interface_name);
                sr_set_error(session, node->xpath, ERR_PORT_NOT_OPEN_XPATH_STR,
                             interface_name);
                ret = SR_ERR_OPERATION_FAILED;
                goto chg_free_change_iter;
            }

            /* get port id */
            if (E_TSN_LIB_NO_ERROR != (ret = tsn_bridge_port_id(port, &port_id))) {
                sr_set_error(session, node->xpath,
                             ERR_MSG_AND_EXPL_SHT_FMT_STR,
                             LIT_GET_STR, interface_name, LIT_ID_STR,
                             tsn_lib_get_known_ernno_msg(ret));
                ret = SR_ERR_OPERATION_FAILED;
                goto chg_free_port_hdl;
            }

            if (!tsn_is_feat_supported(port, FEAT_ST)) {
                SRP_LOG_ERR(ERR_FEATURE_NOT_SUPPORTED_XPATH_STR, "sched",
                            interface_name);
                sr_set_error(session, node->xpath,
                             ERR_FEATURE_NOT_SUPPORTED_XPATH_STR,
                             "sched", interface_name);
                tsn_port_close(port);
                ret = SR_ERR_OPERATION_FAILED;
                goto chg_free_port_hdl;
            }

            /* identify the current element and call proper setter function */
            obj_index = find_obj_index_by_name(sr_xpath_node_name(node->xpath),
                                               sched_map, SCHED_MAX_OBJ_NUMBER);

            switch (obj_index) {
            case SCHED_QUEUE_MAX_SDU:
                /* find traffic needed for set max sdu */
                if (EXIT_SUCCESS != get_key_value(session, node->xpath,
                                                  SCHED_MAX_SDU_TABLE_STR,
                                                  SCHED_TRAFFIC_CLASS_STR, &st,
                                                  tmp_traffic_class)) {
                    SRP_LOG_ERR("%s: %s (%s)", __func__,
                                ERR_MISSING_ELEMENT_STR, SCHED_TRAFFIC_CLASS_STR);
                    return SR_ERR_OPERATION_FAILED;
                }

                traffic_class = (uint8_t)strtoul(tmp_traffic_class, NULL, 10);
                if (E_TSN_LIB_NO_ERROR != (tsn_ret_val = tsn_st_set_max_sdu(port, traffic_class, node->data.uint32_val))) {
                    SRP_LOG_ERR(ERR_SETTER_EXT_FMT_STR,
                                SCHED_MAX_SDU_TABLE_STR, LIT_ENTRY_STR,
                                LIT_XPATH_STR, node->xpath,
                                tsn_lib_get_known_ernno_msg(tsn_ret_val));
                    sr_set_error(session, node->xpath,
                                 ERR_SETTER_EXT_FMT_STR,
                                 SCHED_MAX_SDU_TABLE_STR, LIT_XPATH_STR,
                                 node->xpath,
                                 tsn_lib_get_known_ernno_msg(tsn_ret_val));
                    tsn_port_close(port);
                    return SR_ERR_OPERATION_FAILED;
                }

                tsn_port_close(port);
                return SR_ERR_OK;
            case SCHED_ADMIN_GATE_STATES:
                if (E_TSN_LIB_NO_ERROR != (tsn_ret_val = tsn_st_set_admin_gate_states(port, node->data.uint8_val))) {
                    SRP_LOG_ERR(ERR_SETTER_EXT_FMT_STR,
                                SCHED_ADMIN_GATE_STATES_STR,
                                LIT_VAL_STR, LIT_XPATH_STR, node->xpath,
                                tsn_lib_get_known_ernno_msg(tsn_ret_val));
                    sr_set_error(session, node->xpath,
                                 ERR_SETTER_EXT_FMT_STR,
                                 SCHED_ADMIN_GATE_STATES_STR,
                                 LIT_VAL_STR, LIT_XPATH_STR, node->xpath,
                                 tsn_lib_get_known_ernno_msg(tsn_ret_val));
                    tsn_port_close(port);
                    return SR_ERR_OPERATION_FAILED;
                }

                tsn_port_close(port);
                return SR_ERR_OK;
            default:
                break;
            }

            /* check and get item values */
            do {
                /* Find seconds from admin-base-time */
                ret = get_item_value_cb(session, &tmp_value, SCHED_GATE_PARAMS_ABT_SECONDS_XPATH, interface_name);
                if (EXIT_FAILURE == ret)
                    break;
                st_time.sec = tmp_value->data.uint64_val;
                sr_free_val(tmp_value);

                /* Find fractional-seconds from admin-base-time */
                ret = get_item_value_cb(session, &tmp_value, SCHED_GATE_PARAMS_ABT_NANO_SECONDS_XPATH, interface_name);
                if (EXIT_FAILURE == ret)
                    break;
                st_time.nsec = (uint32_t)tmp_value->data.uint64_val;
                sr_free_val(tmp_value);

                /* Find numerator from admin-cycle-time */
                ret = get_item_value_cb(session, &tmp_value, SCHED_GATE_PARAMS_ACT_NUMERATOR_XPATH, interface_name);
                if (EXIT_FAILURE == ret)
                    break;
                st_rational.numerator = tmp_value->data.uint32_val;
                sr_free_val(tmp_value);

                /* Find denominator from admin-cycle-time */
                ret = get_item_value_cb(session, &tmp_value, SCHED_GATE_PARAMS_ACT_DENOMINATOR_XPATH, interface_name);
                if (EXIT_FAILURE == ret)
                    break;
                st_rational.denominator = tmp_value->data.uint32_val;
                sr_free_val(tmp_value);

                /* Find admin-cycle-time-extension from gate-parameters */
                ret = get_item_value_cb(session, &tmp_value, SCHED_GATE_PARAMS_ACT_EXTENSION_XPATH, interface_name);
                if (EXIT_FAILURE == ret)
                    break;
                act_ext = tmp_value->data.uint32_val;
                sr_free_val(tmp_value);

            } while (0);

            if (EXIT_FAILURE == ret) {
                ret = SR_ERR_CALLBACK_FAILED;
                goto chg_free_port_hdl;
            }

            /* set time element values */
            do {
                /* set admin-base-time */
                if (E_TSN_LIB_NO_ERROR != (tsn_ret_val = tsn_st_set_admin_base_time(port, &st_time))) {
                    snprintf(err_msg, LONG_STR_LEN,
                             ERR_SCHED_CL_ENTRY_SET,
                             SCHED_ADMIN_BASE_TIME_STR, interface_name,
                             tsn_lib_get_known_ernno_msg(tsn_ret_val));
                    ret = EXIT_FAILURE;
                    break;
                }

                /* set admin-cycle-time */
                if (E_TSN_LIB_NO_ERROR != (tsn_ret_val = tsn_st_set_admin_cycle_time(port, &st_rational))) {
                    snprintf(err_msg, LONG_STR_LEN, ERR_SCHED_CL_ENTRY_SET,
                             SCHED_ADMIN_CYCLE_TIME_STR,
                             interface_name,
                             tsn_lib_get_known_ernno_msg(tsn_ret_val));
                    ret = EXIT_FAILURE;
                    break;
                }

                /* set admin-cycle-time-extension */
                if (E_TSN_LIB_NO_ERROR != (tsn_ret_val = tsn_st_set_admin_cycle_time_extension(port, act_ext))) {
                    snprintf(err_msg, LONG_STR_LEN,
                             ERR_SCHED_CL_ENTRY_SET,
                             SCHED_ADMIN_CYCLE_TIME_EXT_STR, interface_name,
                             tsn_lib_get_known_ernno_msg(tsn_ret_val));
                    ret = EXIT_FAILURE;
                    break;
                }

            } while (0);

            if (EXIT_FAILURE == ret) {
                SRP_LOG_ERRMSG(err_msg);
                sr_set_error(session, node->xpath, err_msg);
                ret = SR_ERR_OPERATION_FAILED;
                goto chg_free_port_hdl;

            }

            // close port handle
            tsn_port_close(port);
            port = NULL;
        }
        if (op & SR_OP_DELETED) {
            SRP_LOG_ERR(ERR_REM_OP_NOT_SUPPORTED_XPATH_STR, node->xpath);
            sr_set_error(session, node->xpath, ERR_REM_OP_NOT_SUPPORTED_XPATH_STR, node->xpath);
            ret = SR_ERR_OPERATION_FAILED;
            goto chg_free_change_iter;
        }
    }

chg_free_port_hdl:
    if (NULL != port) {
        tsn_port_close(port);
    }

chg_free_change_iter:
    if (NULL != iter) {
        sr_free_change_iter(iter);
    }

    return ret;
}

/**
 * @brief Write config-change to system.
 *
 * @param[in]   session         Session on which changes were detected.
 * @param[in]   node            Node on which change was detected.
 * @param[in]   port            Port handle for making the changes.
 * @param[in]   interface_name  Name of the interface.
 * @param[in]   config_change   Value to write.
 * @return      Error code (SR_ERR_OK on success).
 */
static int interface_sched_set_config_change(sr_session_ctx_t *session,
                                             sr_val_t *node,
                                             struct port_hdl *port,
                                             const char *interface_name,
                                             bool config_change)
{
    SRP_LOG_DBG("%s() set interface %s config-change %s",
                __func__, interface_name, btoa(config_change));
    tsn_return_value tsn_ret_val = tsn_st_set_config_change(port,
                                                            config_change);

    if (E_TSN_LIB_NO_ERROR != tsn_ret_val) {
        SRP_LOG_ERR(ERR_SCHED_CL_ENTRY_SET, SCHED_CONFIG_CHANGE_STR,
                    interface_name, tsn_lib_get_known_ernno_msg(tsn_ret_val));
        sr_set_error(session, node->xpath,
                     ERR_SCHED_CL_ENTRY_SET, SCHED_CONFIG_CHANGE_STR,
                     interface_name, tsn_lib_get_known_ernno_msg(tsn_ret_val));
        return SR_ERR_OPERATION_FAILED;
    }

    return SR_ERR_OK;
}

/**
 * @brief Write gate-enabled to system.
 *
 * @param[in]   session         Session on which changes were detected.
 * @param[in]   node            Node on which change was detected.
 * @param[in]   port            Port handle for making the changes.
 * @param[in]   interface_name  Name of the interface.
 * @param[in]   gate_enabled    Value to write.
 * @return      Error code (SR_ERR_OK on success).
 */
static int interface_sched_set_gate_enabled(sr_session_ctx_t *session,
                                            sr_val_t *node,
                                            struct port_hdl *port,
                                            const char *interface_name,
                                            bool gate_enabled)
{
    SRP_LOG_DBG("%s() set interface %s gate-enabled %s",
                __func__, interface_name, btoa(gate_enabled));
    tsn_return_value tsn_ret_val = tsn_st_set_gate_enabled(port, gate_enabled);

    if (E_TSN_LIB_NO_ERROR != tsn_ret_val) {
        SRP_LOG_ERR(ERR_SCHED_CL_ENTRY_SET, SCHED_GATE_ENABLED_STR,
                    interface_name, tsn_lib_get_known_ernno_msg(tsn_ret_val));
        sr_set_error(session, node->xpath,
                     ERR_SCHED_CL_ENTRY_SET, SCHED_GATE_ENABLED_STR,
                    interface_name, tsn_lib_get_known_ernno_msg(tsn_ret_val));

        return SR_ERR_OPERATION_FAILED;
    }

    return SR_ERR_OK;
}

/**
 * @brief Apply gate-enabled and config-change settings to system.
 *
 * @param[in]   session         Session on which changes were detected.
 * @param[in]   node            Node on which change was detected.
 * @param[in]   port            Port handle for making the changes.
 * @param[in]   interface_name  Name of the interface.
 * @param[in]   gate_enabled    Gate-enabled value to write.
 * @param[in]   config_change   Config-change value to write.
 * @return      Error code (SR_ERR_OK on success).
 */
static int interface_sched_apply_config_change(sr_session_ctx_t *session,
                                               sr_val_t *node,
                                               struct port_hdl *port,
                                               char *interface_name,
                                               bool gate_enabled,
                                               bool config_change)
{
    /* Detect mutual order of gate-enabled and config-change values
     * in which they are written to system.
     *
     * If gate-enabled is true, make sure to write config-change first, to
     * avoid triggering actual List Config state machine twice in case
     * gate-enabled was false but config-change was already true.
     *
     * Otherwise write gate-enabled first, to avoid triggering actual
     * List Config state machine briefly before stopping it by gate-enabled
     * in case gate-enabled was true and new config-change is true.
     */
    bool gate_enabled_first = !gate_enabled;
    int ret = SR_ERR_OK;

    if (gate_enabled && config_change) {
        /* Check is admin-control-list-length is 0 and is config-change is
         * "true" in the new configuration. If admin-control-list-length is 0
         * and config-change is "true", it could not be possible to apply
         * changes of admin-base-time and admin-cycle-time with config change
         * True, because admin-control-list is missing in the configuration. If
         * config-change is False it is valid situation that admin-control-list
         * is missing. Also, check is "ethtool Link detected". If link is not
         * detected, it is not possible to set config-change to 'true'. If
         * config-change is False it is valid situation that link is not
         * detected.
         */
        ret = validate_sched_configuration(session, node, port, interface_name);
        if (ret == EXIT_FAILURE)
            return SR_ERR_CALLBACK_FAILED;
    }

    if (SR_ERR_OK == ret && gate_enabled_first)
        ret = interface_sched_set_gate_enabled(session, node, port,
                                               interface_name, gate_enabled);
    if (SR_ERR_OK == ret)
        ret = interface_sched_set_config_change(session, node, port,
                                                interface_name, config_change);
    if (SR_ERR_OK == ret && !gate_enabled_first)
        ret = interface_sched_set_gate_enabled(session, node, port,
                                               interface_name, gate_enabled);

    return ret;
}

/**
 * @brief Handle changes to gate-enabled and config-change for an interface.
 *
 * This is used from callback which is subscribed for both changes,
 * as well as from initialization to fix config-change values in STARTUP
 * datastore.
 *
 * @param[in]   session         Session on which changes were detected.
 * @param[in]   reset_list      Callback private data to keep track of changes.
 * @param[in]   node            Node on which change was detected.
 * @param[in]   port            Port handle for making the changes to system,
 *                              or NULL to only handle config-change reset for
 *                              datastore.
 * @param[in]   interface_name  Name of the interface.
 * @return      Error code (SR_ERR_OK on success).
 */
static int interface_sched_handle_config_change(sr_session_ctx_t *session,
                                                struct config_change_list *reset_list,
                                                sr_val_t *node,
                                                struct port_hdl *port,
                                                char *interface_name)
{
    sr_val_t *config_change = NULL;
    sr_val_t *gate_enabled = NULL;
    int ret = SR_ERR_OK;

    SRP_LOG_DBG("%s() handle change %s", __func__, node->xpath);

    ret = get_item_value_cb(session, &config_change,
                            SCHED_GATE_PARAMS_CONFIG_CHANGE_XPATH,
                            interface_name);
    if (SR_ERR_OK != ret) {
        sr_set_error(session, node->xpath,
                     ERR_FAILED_TO_GET_ITEM_STR " (%s)",
                     SCHED_CONFIG_CHANGE_STR);
        return SR_ERR_OPERATION_FAILED;
    }
    SRP_LOG_DBG("%s() config-change %s", __func__,
                btoa(config_change->data.bool_val));

    if (config_change_list_exists(reset_list, config_change->xpath)) {
        /* This config-change has already been handled.
         * This could happen when both gate-enabled and config-change
         * have been changed to true, because this function is called
         * separately for both changes.
         */
        goto out_free_config_change;
    }

    ret = get_item_value_cb(session, &gate_enabled,
                            SCHED_GATE_PARAMS_GATE_ENABLED_XPATH,
                            interface_name);
    if (SR_ERR_OK != ret) {
        sr_set_error(session, node->xpath,
                     ERR_FAILED_TO_GET_ITEM_STR " (%s)",
                     SCHED_GATE_ENABLED_STR);
        goto out_free_config_change;
    }
    SRP_LOG_DBG("%s() gate-enabled %s", __func__,
                btoa(gate_enabled->data.bool_val));

    /* This is also used to only fix startup datastore at initialization,
     * in which case port is NULL.
     */
    if (port) {
        ret = interface_sched_apply_config_change(session, node, port,
                                                  interface_name,
                                                  gate_enabled->data.bool_val,
                                                  config_change->data.bool_val);
    }

    if (SR_ERR_OK == ret) {
        /* Record this gate-enabled and config-change pair, regardless of
         * actual trigger value, to avoid duplicate calls to system. Collected
         * actual triggers will be reset in datastore from DONE event later.
         */
        ret = config_change_list_push(reset_list, config_change->xpath,
                                      gate_enabled->data.bool_val &&
                                      config_change->data.bool_val);
        if (SR_ERR_OK != ret) {
            SRP_LOG_WRN(ERROR_MSG_MOD_FUNC_EL_MSG, SCHED_MODULE_NAME_STR,
                        __func__, interface_name,
                        "Failed to record this change");
        }
    }

    sr_free_val(gate_enabled);

out_free_config_change:
    sr_free_val(config_change);
    return ret;
}

int interface_sched_config_change_cb(sr_session_ctx_t *session,
                                     const char *module_name, const char *xpath,
                                     sr_event_t event, uint32_t request_id,
                                     void *private_data)
{
    (void)event;
    (void)module_name;
    (void)request_id;

    // Note that this callback is used for both gate-enabled and config-change.
    struct config_change_list *reset_list = private_data;
    int ret = SR_ERR_OK;
    sr_change_oper_t op = {0};
    sr_xpath_ctx_t st = {0};
    sr_change_iter_t *iter = NULL;
    sr_val_t *old_value = NULL;
    sr_val_t *new_value = NULL;
    sr_val_t *node = NULL;
    struct port_hdl *port = NULL;
    char interface_name[MAX_STR_LEN] = "";

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    if (SR_EV_ABORT == event) {
        /* Failed to apply some (other) changes, so datastore will not be be
         * written, so there is no need to reset any config-changes.
         * Abandon collected changes.
         */
        SRP_LOG_DBG("%s() abort, skip config-change resets", __func__);
        config_change_list_clear(reset_list);
        return SR_ERR_OK;
    }

    if ((SR_EV_DONE == event) || (0 == plugin_init)) {
        SRP_LOG_DBG(DEBUG_MSG_WITH_TWO_PARAM, DBG_APPLYING_CHANGES_MSG, __func__);
        if (SR_EV_DONE == event) {
            /* Changes applied successfully to datastore, ask config-change
             * helper to reset config-change triggers back to false
             * in datastore.
             */
            config_change_list_enqueue(reset_list);
        }
        return SR_ERR_OK;
    }

    ret = sr_get_changes_iter(session, xpath, &iter);
    if (SR_ERR_OK != ret) {
        return ret;
    }

    while (SR_ERR_OK == sr_get_change_next(session, iter, &op,
                                           &old_value, &new_value)) {
        node = (op == SR_OP_DELETED) ? old_value : new_value;

        /* get interface name from xpath */
        if (EXIT_SUCCESS != get_key_value(session, node->xpath,
                                          IF_INTERFACE_LIST, IF_NAME_STR,
                                          &st, interface_name)) {
            SRP_LOG_ERR("%s: %s (%s)", __func__,
                        ERR_MISSING_ELEMENT_STR, IF_NAME_STR);
            return SR_ERR_OPERATION_FAILED;
        }
        if ((op == SR_OP_MODIFIED) || (op == SR_OP_CREATED)) {
            /* open port by name and check is feature suppported on the port */
            port = tsn_port_open_by_name(interface_name);
            if (NULL == port) {
                SRP_LOG_ERR(ERR_PORT_NOT_OPEN_XPATH_STR, interface_name);
                sr_set_error(session, node->xpath,
                             ERR_PORT_NOT_OPEN_XPATH_STR, interface_name);
                ret = SR_ERR_OPERATION_FAILED;
                goto chg_free_change_iter;
            }

            if (!tsn_is_feat_supported(port, FEAT_ST)) {
                SRP_LOG_ERR(ERR_FEATURE_NOT_SUPPORTED_XPATH_STR, "sched",
                            interface_name);
                sr_set_error(session, node->xpath,
                             ERR_FEATURE_NOT_SUPPORTED_XPATH_STR,
                             "sched", interface_name);
                tsn_port_close(port);
                ret = SR_ERR_OPERATION_FAILED;
                goto chg_free_port_hdl;
            }

            /* Handle and record both gate-enabled and config-change for this
             * interface.
             */
            ret = interface_sched_handle_config_change(session, reset_list,
                                                       node, port,
                                                       interface_name);
            if (SR_ERR_OK != ret) {
                ret = SR_ERR_OPERATION_FAILED;
                goto chg_free_port_hdl;
            }

            // close port handle
            tsn_port_close(port);
            port = NULL;
        }
        if (op & SR_OP_DELETED) {
            SRP_LOG_ERR(ERR_REM_OP_NOT_SUPPORTED_XPATH_STR, node->xpath);
            sr_set_error(session, node->xpath,
                         ERR_REM_OP_NOT_SUPPORTED_XPATH_STR, node->xpath);
            ret = SR_ERR_OPERATION_FAILED;
            goto chg_free_change_iter;
        }
    }

chg_free_port_hdl:
    if (NULL != port) {
        tsn_port_close(port);
    }

chg_free_change_iter:
    if (NULL != iter) {
        sr_free_change_iter(iter);
    }

    if (SR_ERR_OK != ret) {
        /* Datastore will not be written in case of errors, so abandon
         * collected list of changes. Also ABORT event will not be called for
         * failing callback, so this needs to be done here.
         */
        SRP_LOG_DBG("%s() error, skip config-change resets", __func__);
        config_change_list_clear(reset_list);
    }

    return ret;
}

int interface_sched_fix_datastore(sr_session_ctx_t *session,
                                  sr_datastore_t datastore)
{
    sr_datastore_t orig_datastore = sr_session_get_ds(session);
    struct config_change_list *reset_list = NULL;
    sr_xpath_ctx_t st = {0};
    sr_val_t *nodes = NULL;
    size_t count = 0;
    size_t i = 0;
    char interface_name[MAX_STR_LEN] = "";
    int ret;

    SRP_LOG_DBG("%s() current datastore ", __func__, orig_datastore);

    if (datastore != orig_datastore) {
        ret = sr_session_switch_ds(session, datastore);
        if (SR_ERR_OK != ret) {
            SRP_LOG_ERR("%s() failed to switch datastore", __func__);
            return SR_ERR_OK;
        }
    }

    SRP_LOG_DBG("%s() switch datastore %d -> %d", __func__,
                orig_datastore, datastore);

    reset_list = config_change_list_new();
    if (!reset_list)
        return SR_ERR_NOMEM;

    ret = sr_get_items(session,
                       SCHED_GATE_PARAMS_CONFIG_CHANGE_SUB_XPATH "|"
                       SCHED_GATE_PARAMS_GATE_ENABLED_SUB_XPATH,
                       0, 0,
                       &nodes, &count);
    SRP_LOG_DBG("%s() sr_get_items %zu result %d", __func__, count, ret);
    if (SR_ERR_OK != ret)
        goto error;

    for (i = 0; i < count; i++) {
        SRP_LOG_DBG("%s() item %zu/%zu", __func__, i + 1, count);
        sr_val_t *node = &nodes[i];

        ret = get_key_value(session, node->xpath,
                            IF_INTERFACE_LIST, IF_NAME_STR,
                            &st, interface_name);
        if (EXIT_SUCCESS != ret) {
            SRP_LOG_WRN("%s: %s (%s)", __func__,
                        ERR_MISSING_ELEMENT_STR, IF_NAME_STR);
            // Not failing plugin startup because of this.
            ret = SR_ERR_OK;
            continue;
        }

        ret = interface_sched_handle_config_change(session, reset_list,
                                                   node, NULL,
                                                   interface_name);
        if (SR_ERR_OK != ret) {
            SRP_LOG_WRN("%s() failed to handle interface %s XPath %s", __func__,
                        interface_name, node->xpath);
            // Not failing plugin startup because of this.
            ret = SR_ERR_OK;
            continue;
        }
    }

    config_change_list_process(session, reset_list);

error:
    free(reset_list);
    sr_free_values(nodes, count);
    if (datastore != orig_datastore) {
        SRP_LOG_DBG("%s() switch to original datastore %d -> %d", __func__,
                    datastore, orig_datastore);
        ret = sr_session_switch_ds(session, orig_datastore);
        if (SR_ERR_OK)
            SRP_LOG_ERR("%s() failed to switch back to original datastore",
                        __func__);
    }

    SRP_LOG_DBG("%s() finished", __func__);
    return ret;
}
