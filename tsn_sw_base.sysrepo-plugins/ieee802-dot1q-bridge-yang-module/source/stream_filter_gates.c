/*
 * TTTech ieee802-dot1q-bridge-yang-module
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

#include "stream_filter_gates.h"

/* global variable from ieee802-dot1q-bridge.c */
extern int plugin_init;

static char sfg_prio_spec_map[9][2][MAX_STR_LEN] = {
    // 8 pairs
    { "0",   "zero" },
    { "1",   "one" },
    { "2",   "two" },
    { "3",   "three" },
    { "4",   "four" },
    { "5",   "five" },
    { "6",   "six" },
    { "7",   "seven" },
    { "-1",  "wildcard" }
};

static char sfg_prio_spec_reverse_map[9][2][MAX_STR_LEN] = {
    // 8 pairs
    { "zero",    "0" },
    { "one",     "1" },
    { "two",     "2" },
    { "three",   "3" },
    { "four",    "4" },
    { "five",    "5" },
    { "six",     "6" },
    { "seven",   "7" },
    { "wildcard", "-1" }
};

static char sfg_ipv_spec_map[9][2][MAX_STR_LEN] = {
    // 8 pairs
    { "0",   "zero" },
    { "1",   "one" },
    { "2",   "two" },
    { "3",   "three" },
    { "4",   "four" },
    { "5",   "five" },
    { "6",   "six" },
    { "7",   "seven" },
    { "-1",  "null" },
};

static char sfg_ipv_spec_reverse_map[9][2][MAX_STR_LEN] = {
    // 8 pairs
    { "zero",    "0" },
    { "one",     "1" },
    { "two",     "2" },
    { "three",   "3" },
    { "four",    "4" },
    { "five",    "5" },
    { "six",     "6" },
    { "seven",   "7" },
    { "null",    "-1" }
};

static char psfp_ags_map[2][2][MAX_STR_LEN] = {
    // 8 pairs
    { "0",   "closed" },
    { "1",   "open" }
};

static char psfp_ags_reverse_map[2][2][MAX_STR_LEN] = {
    // 8 pairs
    { "closed",  "0" },
    { "open",    "1" }
};

char psfp_map[PSFP_MAX_OBJ_NUMBER][MAX_STR_LEN] = {
    // map between object index and object name for sched module
    [PSFP_OP_NAME]                  = PSFP_OP_NAME_STR,
    [PSFP_TIME_INTERVAL]            = PSFP_TIME_INTERVAL_VALUE_STR,
    [PSFP_GATE_STATE]               = PSFP_GATE_STATE_VALUE_STR,
    [PSFP_IPV_SPEC]                 = PSFP_IPV_SPEC_STR,
    [PSFP_GATE_ENABLED]             = SFG_GATE_ENABLE_STR,
    [PSFP_AGS]                      = SFG_ADMIN_GATE_STATES_STR,
    [PSFP_ADMIN_IPV]                = SFG_ADMIN_IPV_STR,
    [PSFP_STREAM_HDL]               = SFG_STREAM_HANDLE_STR,
    [PSFP_PRIO_SPEC]                = SFG_PRIORITY_SPEC_STR,
    [PSFP_MAX_SDU_SIZE]             = SFG_MAX_SDU_SIZE_STR,
    [PSFP_STR_BLCKD_FRAME_ENBLD]    = SFG_BLK_OVERZ_ENA_STR,
    [PSFP_STR_BLCKD_FRAME]          = SFG_BLK_OVERZ_STR,
    [PSFP_STR_GATE_REF]             = SFG_STREAM_GATE_REF_STR,
    [PSFP_INVALID_RX_ENBLD]         = "ieee802-dot1q-psfp:gate-closed-due-to-invalid-rx-enable",
    [PSFP_INVALID_RX]               = "ieee802-dot1q-psfp:gate-closed-due-to-invalid-rx",
};

struct lyd_node *new_stream_filter_entry(struct lyd_node *parent,
                                         const struct lys_module *module,
                                         struct port_hdl *port, uint32_t flt_id)
{
    struct lyd_node *stream_filter_entry = NULL;
    struct psfp_flt_params params;
    char tmp[MAX_STR_LEN] = "";

    SRP_LOG_DBG(DEBUG_MSG_FUN_CALLED, __func__);

    /* first get filter parameters */
    if (0 == tsn_psfp_flt_get_params(port, flt_id, &params)) {
        /* new stream-filter-instance-table entry */
        if (EXIT_FAILURE == new_container(parent, &stream_filter_entry, module,
                                          SFG_STREAM_FILTER_INSTANCE_STR)) {
            return NULL;
        }
        /* list stream-filter-instance-table, leaf 'stream-filter-instance-id' */
        snprintf(tmp, MAX_STR_LEN, "%u", flt_id);
        if (EXIT_FAILURE == new_node(stream_filter_entry, module,
                                     SFG_STREAM_FILTER_INSTANCE_ID_STR, tmp)) {
            return NULL;
        }
        /* list stream-filter-instance-table, leaf 'stream-handle' */
        snprintf(tmp, MAX_STR_LEN, "%d", params.str_hdl);
        if (EXIT_FAILURE == new_node(stream_filter_entry, module,
                                     SFG_STREAM_HANDLE_STR, tmp)) {
            return NULL;
        }
        /* list stream-filter-instance-table, leaf 'priority-spec' */
        snprintf(tmp, MAX_STR_LEN, "%d", params.prio);
        if (EXIT_SUCCESS == translate_single_string_using_map(tmp, sfg_prio_spec_map)) {
            /* list stream-filter-instance-table, leaf 'priority-spec' */
            if (EXIT_FAILURE == new_node(stream_filter_entry, module,
                                         SFG_PRIORITY_SPEC_STR, tmp)) {
                return NULL;
            }
        }
        /* list stream-filter-instance-table, leaf 'max-sdu-size' */
        snprintf(tmp, MAX_STR_LEN, "%u", params.max_sdu);
        if (EXIT_FAILURE == new_node(stream_filter_entry, module,
                                     SFG_MAX_SDU_SIZE_STR, tmp)) {
            return NULL;
        }
        /* list stream-filter-instance-table, leaf
         * 'stream-blocked-due-to-oversize-frame-enabled'
         */
        snprintf(tmp, MAX_STR_LEN, "%s", params.blk_oversz_ena ? BASE_TRUE : BASE_FALSE);
        if (EXIT_FAILURE == new_node(stream_filter_entry, module,
                                     SFG_BLK_OVERZ_ENA_STR, tmp)) {
            return NULL;
        }
        /* list stream-filter-instance-table, leaf
         * 'stream-blocked-due-to-oversize-frame'
         */
        snprintf(tmp, MAX_STR_LEN, "%s", params.blk_oversz_ena ? BASE_TRUE : BASE_FALSE);
        if (EXIT_FAILURE == new_node(stream_filter_entry, module,
                                     SFG_BLK_OVERZ_STR, tmp)) {
            return NULL;
        }
        /* list stream-filter-instance-table, leaf 'stream-gate-ref' */
        snprintf(tmp, MAX_STR_LEN, "%u", params.gate_id);
        if (EXIT_FAILURE == new_node(stream_filter_entry, module,
                                     SFG_STREAM_GATE_REF_STR, tmp)) {
            return NULL;
        }
    }

    return stream_filter_entry;
}

struct lyd_node *new_stream_filters_container(struct ly_ctx *ctx,
                                              struct lyd_node *parent,
                                              char *bridge_name)
{
    struct lyd_node *stream_filters = NULL;
    struct port_hdl *port = NULL;
    const struct lys_module *module = NULL;
    struct psfp_flt_params params;
    uint32_t max_filters = 0;
    uint32_t flt_id = 0;
    uint32_t ret = 0;

    SRP_LOG_DBG(DEBUG_MSG_FUN_CALLED_WITH_ONE_PARAM, __func__, bridge_name);

    module = ly_ctx_load_module(ctx, SFG_MODULE_NAME_STR, NULL);

    if (NULL == module) {
        SRP_LOG_ERR(ERROR_MSG_FUN_AND_MSG, __func__, ERR_MSG_LOAD_MODULE_STR);
        return NULL;
    }

    /* new stream-filters container */
    if (EXIT_FAILURE == new_container(parent, &stream_filters, module,
                                      SFG_STREAM_FILTERS_STR)) {
        return NULL;
    }

    port = tsn_port_open_by_name(bridge_name);
    if (NULL == port) {
        SRP_LOG_ERR(ERROR_MSG_MOD_FUNC_EL_MSG, SFG_MODULE_NAME_STR, __func__,
                    bridge_name, ERR_PORT_NOT_OPEN_STR);
        return NULL;
    }

    /* get number of max filters for stream-filter-instance-table list */
    if (0 == tsn_psfp_get_max_filters(port, &max_filters)) {
        for (flt_id = 0; flt_id < max_filters; flt_id++) {
            ret = tsn_psfp_flt_get_params(port, flt_id, &params);
            if (TSN_ERRNO_ERROR_CODE(ret) == ENOENT)
                continue;
            if (NULL == new_stream_filter_entry(stream_filters, module,
                                                port, flt_id)) {
                return NULL;
            }
        }
    }

    tsn_port_close(port);

    return stream_filters;
}

struct lyd_node *new_stream_gate_entry(struct ly_ctx *ctx,
                                       struct lyd_node *parent,
                                       const struct lys_module *module,
                                       struct port_hdl *port, uint32_t gate_id)
{
    struct lyd_node *stream_gate_entry = NULL;
    struct lyd_node *admin_control_list = NULL;
    struct lyd_node *gate_control_entry = NULL;
    struct lyd_node *cycle_time = NULL;
    struct lyd_node *base_time = NULL;
    struct tsn_st_rational_number st_rational = {0u,0u};
    struct tsn_st_ptp_time st_time = {0u, 0u};
    struct psfp_gate_control_entry *acl_entry = NULL;
    char tmp_str[MAX_STR_LEN] = "";
    bool tmp_bool = false;
    int8_t tmp_int8 = 0;
    uint8_t tmp_ui8 = 0;
    uint32_t index = 0;
    uint32_t acl_length = 1;
    uint32_t tmp_uint32 = 0;

    SRP_LOG_DBG(DEBUG_MSG_FUN_CALLED, __func__);

    /* new stream-gate-instance-table entry */
    if (EXIT_FAILURE == new_container(parent, &stream_gate_entry, module,
                                      SFG_STREAM_GATE_INSTANCE_STR)) {
        return NULL;
    }

    /* list stream-gate-instance-table, leaf 'stream-gate-instance-id' */
    snprintf(tmp_str, MAX_STR_LEN, "%u", gate_id);
    if (EXIT_FAILURE == new_node(stream_gate_entry, module,
                                 SFG_STREAM_GATE_INSTANCE_ID_STR, tmp_str)) {
        return NULL;
    }
    /* list stream-gate-instance-table, leaf 'gate-enable' */
    if (0 == tsn_psfp_gt_get_gate_enabled(port, gate_id, &tmp_bool)) {
        snprintf(tmp_str, MAX_STR_LEN, "%s", tmp_bool ? BASE_TRUE : BASE_FALSE);
        if (EXIT_FAILURE == new_node(stream_gate_entry, module,
                                     SFG_GATE_ENABLE_STR, tmp_str)) {
            return NULL;
        }
    }

    /* list stream-gate-instance-table, leaf 'admin-gate-states' */
    if (0 == tsn_psfp_gt_get_admin_gate_states(port, gate_id, &tmp_ui8)) {
        snprintf(tmp_str, MAX_STR_LEN, "%d", tmp_ui8);
        if (EXIT_SUCCESS == translate_single_string_using_map(tmp_str, psfp_ags_map)) {
            if (EXIT_FAILURE == new_node(stream_gate_entry, module,
                                         SFG_ADMIN_GATE_STATES_STR, tmp_str)) {
                return NULL;
            }
        }
    }

    /* list stream-gate-instance-table, leaf 'admin-ipv' */
    if (0 == tsn_psfp_gt_get_admin_ipv(port, gate_id, &tmp_int8)) {
        snprintf(tmp_str, MAX_STR_LEN, "%d", tmp_int8);
        if (EXIT_SUCCESS == translate_single_string_using_map(tmp_str, sfg_ipv_spec_map)) {
            if (EXIT_FAILURE == new_node(stream_gate_entry, module,
                                         SFG_ADMIN_IPV_STR, tmp_str)) {
                return NULL;
            }
        }
    }

    /* augment from ieee802-dot1q-psfp.yang module */
    module = NULL;
    module = ly_ctx_load_module(ctx, PSFP_MODULE_NAME_STR, NULL);
    if (NULL == module) {
        SRP_LOG_ERR(ERROR_MSG_FUN_AND_MSG, __func__, ERR_MSG_LOAD_MODULE_STR);
        return NULL;
    }
    /* container admin-cotrol-list */
    if (0 == tsn_psfp_gt_get_admin_control_list_length(port, gate_id, &acl_length)) {
        if (acl_length > 0) {
            /* new admin-control-list container */
            if (EXIT_FAILURE == new_container(stream_gate_entry, &admin_control_list,
                                              module, PSFP_ADMIN_CTRL_LIST_STR)) {
                return NULL;
            }

            /* list admin_control_list */
            if ((acl_length > 0) && (acl_length <= MAX_NUMBER_OF_ACL)) {
                acl_entry = malloc(sizeof(*acl_entry) * acl_length);
            } else {
                SRP_LOG_INF(ERROR_MSG_MOD_FUNC_EL_MSG, PSFP_MODULE_NAME_STR, __func__,
                            PSFP_ADMIN_CTRL_LIST_STR, ERR_NOT_CREATED_ELEMENT_STR);
            }
            if (NULL != acl_entry) {
                if (0 == tsn_psfp_gt_get_admin_control_list(port, gate_id, acl_length, acl_entry)) {
                    for (index = 0; index < acl_length; index++) {
                        /* container gate-control-entry */
                        if (EXIT_FAILURE == new_container(admin_control_list, &gate_control_entry,
                                                          module, PSFP_GCL_ENTRY_STR)) {
                            tsn_port_close(port);
                            return NULL;
                        }
                        /* list gate-control-entry, leaf index */
                        snprintf(tmp_str, MAX_STR_LEN, "%u", index);
                        if (EXIT_FAILURE == new_node(gate_control_entry, module,
                                                     PSFP_INDEX_STR, tmp_str)) {
                            tsn_port_close(port);
                            return NULL;
                        }
                        /* list gate-control-entry, leaf operation-name */
                        switch (acl_entry[index].op) {
                        case 0:
                            snprintf(tmp_str, MAX_STR_LEN, "%s", PSFP_SET_GATE_AND_IPV_STR);
                            break;
                        default:
                            tsn_port_close(port);
                            return NULL;
                        }
                        /* leaf operation-name */
                        if (EXIT_FAILURE == new_node(gate_control_entry, module,
                                                     PSFP_OP_NAME_STR, tmp_str)) {
                            tsn_port_close(port);
                            return NULL;
                        }
                        /* leaf time-interval-value */
                        snprintf(tmp_str, MAX_STR_LEN, "%u", acl_entry[index].interval);
                        if (EXIT_FAILURE == new_node(gate_control_entry, module,
                                                     PSFP_TIME_INTERVAL_VALUE_STR, tmp_str)) {
                            tsn_port_close(port);
                            return NULL;
                        }
                        /* leaf gate-state-value */
                        snprintf(tmp_str, MAX_STR_LEN, "%u", acl_entry[index].gsv);
                        if (EXIT_SUCCESS == translate_single_string_using_map(tmp_str, psfp_ags_map)) {
                            if (EXIT_FAILURE == new_node(gate_control_entry, module,
                                                         PSFP_GATE_STATE_VALUE_STR, tmp_str)) {
                                tsn_port_close(port);
                                return NULL;
                            }
                        }

                        /* leaf ipv-spec */
                        snprintf(tmp_str, MAX_STR_LEN, "%d", acl_entry[index].ipv);
                        if (EXIT_SUCCESS == translate_single_string_using_map(tmp_str, sfg_ipv_spec_map)) {
                            if (EXIT_FAILURE == new_node(gate_control_entry, module,
                                                         PSFP_IPV_SPEC_STR, tmp_str)) {
                                tsn_port_close(port);
                                return NULL;
                            }
                        }

#if 0
                        /* TODO: leaf interval-octet-max */
                        snprintf(tmp_str, MAX_STR_LEN, "%u", acl_entry[index].interval);
                        if (EXIT_FAILURE == new_node(gate_control_entry, module,
                                                     PSFP_INT_MAX_OCET_STR, tmp_str)) {
                            tsn_port_close(port);
                            return NULL;
                        }
#endif
                    }
                }
            } else {
                SRP_LOG_INF(ERROR_MSG_MOD_FUNC_EL_MSG, PSFP_MODULE_NAME_STR,
                        __func__, PSFP_ADMIN_CTRL_LIST_STR, ERR_NOT_CREATED_ELEMENT_STR);
            }
            free(acl_entry);
        } else {
            SRP_LOG_INF(ERROR_MSG_MOD_FUNC_EL_MSG, PSFP_MODULE_NAME_STR,
                    __func__, PSFP_ADMIN_CTRL_LIST_STR, ERR_NOT_CREATED_ELEMENT_STR);
        }
    }

    /* container admin-cycle-time */
    if (0 == tsn_psfp_gt_get_admin_cycle_time(port, gate_id, &st_rational)) {
        if (EXIT_FAILURE == new_container(stream_gate_entry, &cycle_time, module,
                                          PSFP_ADMIN_CYCLE_TIME_STR)) {
            tsn_port_close(port);
            return NULL;
        }
        /* leaf numerator */
        snprintf(tmp_str, MAX_STR_LEN, "%u", st_rational.numerator);
        if (EXIT_FAILURE == new_node(cycle_time, module,
                                     PSFP_NUMERATOR_STR, tmp_str)) {
            tsn_port_close(port);
            return NULL;
        }

        /* leaf denominator */
        snprintf(tmp_str, MAX_STR_LEN, "%u", st_rational.denominator);
        if (EXIT_FAILURE == new_node(cycle_time, module,
                                     PSFP_DENOMINATOR_STR, tmp_str)) {
            tsn_port_close(port);
            return NULL;
        }
    }

    /* leaf admin-cycle-time-extension */
    if (0 == tsn_psfp_gt_get_admin_cycle_time_extension(port, gate_id, &tmp_uint32)) {
        snprintf(tmp_str, MAX_STR_LEN, "%u", tmp_uint32);
        if (EXIT_FAILURE == new_node(stream_gate_entry, module,
                                     PSFP_ADMIN_CYCLE_TIME_EXT_STR, tmp_str)) {
            tsn_port_close(port);
            return NULL;
        }
    }

    /* container admin-base-time */
    if (0 == tsn_psfp_gt_get_admin_base_time(port, gate_id, &st_time)) {
        if (EXIT_FAILURE == new_container(stream_gate_entry, &base_time, module,
                                          PSFP_ADMIN_BASE_TIME_STR)) {
            tsn_port_close(port);
            return NULL;
        }
        /* leaf seconds */
        snprintf(tmp_str, MAX_STR_LEN, "%llu", (unsigned long long int)st_time.sec);
        if (EXIT_FAILURE == new_node(base_time, module,
                                     PSFP_ABT_SECONDS_STR, tmp_str)) {
            tsn_port_close(port);
            return NULL;
        }
        /* leaf nanoseconds */
        snprintf(tmp_str, MAX_STR_LEN, "%u", st_time.nsec);
        if (EXIT_FAILURE == new_node(base_time, module,
                                     PSFP_ABT_FRACT_SECONDS_STR, tmp_str)) {
            tsn_port_close(port);
            return NULL;
        }
    }
    /* config-change */
    if (0 == tsn_psfp_gt_get_config_change(port, gate_id, &tmp_bool)) {
        snprintf(tmp_str, MAX_STR_LEN, "%s", tmp_bool ? BASE_TRUE : BASE_FALSE);
        if (EXIT_FAILURE == new_node(stream_gate_entry, module,
                                     PSFP_CONFIG_CHANGE_STR, tmp_str)) {
            tsn_port_close(port);
            return NULL;
        }
    }
    /* gate-closed-due-to-invalid-rx-enable */
    if (0 == tsn_psfp_gt_get_closed_inv_rx_enable(port, gate_id, &tmp_bool)) {
        snprintf(tmp_str, MAX_STR_LEN, "%s", tmp_bool ? BASE_TRUE : BASE_FALSE);
        if (EXIT_FAILURE == new_node(stream_gate_entry, module,
                                     PSFP_GT_CLOSED_INVALID_RX_ENA_STR, tmp_str)) {
            tsn_port_close(port);
            return NULL;
        }
    }

    /* gate-closed-due-to-invalid-rx */
    if (0 == tsn_psfp_gt_get_closed_inv_rx(port, gate_id, &tmp_bool)) {
        snprintf(tmp_str, MAX_STR_LEN, "%s", tmp_bool ? BASE_TRUE : BASE_FALSE);
        if (EXIT_FAILURE == new_node(stream_gate_entry, module,
                                     PSFP_GT_CLOSED_INVALID_RX_STR, tmp_str)) {
            tsn_port_close(port);
            return NULL;
        }
    }

    return stream_gate_entry;
}

struct lyd_node *new_stream_gates_container(struct ly_ctx *ctx,
                                            struct lyd_node *parent,
                                            char *bridge_name)
{
    struct lyd_node *stream_gates = NULL;
    struct lyd_node *supported_cycle_max = NULL;
    const struct lys_module *module = NULL;
    struct port_hdl *port = NULL;
    struct tsn_st_rational_number st_rational = {0u,0u};
    char tmp_str[MAX_STR_LEN] = "";
    uint32_t tmp_uint32 = 0;
    uint32_t gate_id = 0;

    SRP_LOG_DBG(DEBUG_MSG_FUN_CALLED_WITH_ONE_PARAM, __func__, bridge_name);

    module = ly_ctx_load_module(ctx, SFG_MODULE_NAME_STR, NULL);

    if (NULL == module) {
        SRP_LOG_ERR(ERROR_MSG_FUN_AND_MSG, __func__, ERR_MSG_LOAD_MODULE_STR);
        return NULL;
    }

    /* new stream-gates container */
    if (EXIT_FAILURE == new_container(parent, &stream_gates, module,
                                      SFG_STREAM_GATES_STR)) {
        return NULL;
    }

    port = tsn_port_open_by_name(bridge_name);
    if (NULL == port) {
        SRP_LOG_ERR(ERROR_MSG_MOD_FUNC_EL_MSG, SFG_MODULE_NAME_STR, __func__,
                    bridge_name, ERR_PORT_NOT_OPEN_STR);
        return NULL;
    }

    module = NULL;
    /* augment from ieee802-dot1q-psfp.yang module */
    module = ly_ctx_load_module(ctx, PSFP_MODULE_NAME_STR, NULL);
    if (NULL == module) {
        SRP_LOG_ERR(ERROR_MSG_FUN_AND_MSG, __func__, ERR_MSG_LOAD_MODULE_STR);
        return NULL;
    }
    /* leaf supported-list-max */
    if (0 == tsn_psfp_get_supported_list_max(port, &tmp_uint32)) {
        snprintf(tmp_str, MAX_STR_LEN, "%u", tmp_uint32);
        if (EXIT_FAILURE == new_node(stream_gates, module,
                                     PSFP_SUP_LIST_MAX_STR, tmp_str)) {
            tsn_port_close(port);
            return NULL;
        }
    }

    /* container supported-cycle-max */
    if (0 == tsn_psfp_get_supported_cycle_max(port, &st_rational)) {
        if (EXIT_FAILURE == new_container(stream_gates, &supported_cycle_max,
                                          module, PSFP_SUP_CYCLE_MAX_STR)) {
            tsn_port_close(port);
            return NULL;
        }
        /* leaf numerator */
        snprintf(tmp_str, MAX_STR_LEN, "%u", st_rational.numerator);
        if (EXIT_FAILURE == new_node(supported_cycle_max, module,
                                     PSFP_NUMERATOR_STR, tmp_str)) {
            tsn_port_close(port);
            return NULL;
        }

        /* leaf denominator */
        snprintf(tmp_str, MAX_STR_LEN, "%u", st_rational.denominator);
        if (EXIT_FAILURE == new_node(supported_cycle_max, module,
                                     PSFP_DENOMINATOR_STR, tmp_str)) {
            tsn_port_close(port);
            return NULL;
        }
    }

    /* leaf supported-interval-max */
    if (0 == tsn_psfp_get_supported_interval_max(port, &tmp_uint32)) {
        snprintf(tmp_str, MAX_STR_LEN, "%u", tmp_uint32);
        if (EXIT_FAILURE == new_node(stream_gates, module,
                                     PSFP_SUP_INTRVL_MAX_STR, tmp_str)) {
            tsn_port_close(port);
            return NULL;
        }
    }

    /* get number of max gates for stream-gate-instance-table list */
    module = ly_ctx_load_module(ctx, SFG_MODULE_NAME_STR, NULL);

    if (NULL == module) {
        SRP_LOG_ERR(ERROR_MSG_FUN_AND_MSG, __func__, ERR_MSG_LOAD_MODULE_STR);
        return NULL;
    }
    if (0 == tsn_psfp_get_max_gates(port, &tmp_uint32)) {
        for (gate_id = 0; gate_id < tmp_uint32; gate_id++) {
            if (NULL == new_stream_gate_entry(ctx, stream_gates, module,
                                              port, gate_id)) {
                return NULL;
            }
        }
    }
    tsn_port_close(port);

    return stream_gates;
}

int sfg_stream_filters_cb(sr_session_ctx_t *session, const char *module_name,
                          const char *xpath, sr_event_t event,
                          uint32_t request_id, void *private_data)
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
    sr_val_t *tmp_value = NULL;
    char bridge_name[MAX_STR_LEN] = "";
    char filter_id[MAX_STR_LEN] = "";
    char tmp_str[MAX_STR_LEN] = "";
    sr_xpath_ctx_t st = {0};
    uint32_t flt_id = 0;
    struct psfp_flt_params params;
    struct port_hdl *port = NULL;
    int ret = EXIT_SUCCESS;
    tsn_return_value tsn_ret = E_TSN_LIB_NO_ERROR;

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    /* if event is SR_EV_DONE or SR_EV_ABORT return flag to false */
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

        /* if stream-filters/stream-filter-instance-table entry is added or deleted */
        if ((true == sr_xpath_node_name_eq(node->xpath, SFG_STREAM_FILTER_INSTANCE_ID_STR)) &&
            ((SR_OP_CREATED == op) ||
             (SR_OP_DELETED == op))) {
            /* get bridge name from xpath */
            if (EXIT_SUCCESS != get_key_value(session, (char*)node->xpath,
                                              Q_BRIDGE_BRIDGE_ENTRY, Q_BRIDGE_NAME_STR,
                                              &st, bridge_name)) {
                return SR_ERR_OPERATION_FAILED;
            }

            /* open bridge */
            port = tsn_port_open_by_name(bridge_name);
            if (NULL == port) {
                sr_set_error(session, node->xpath,
                             ERR_PORT_NOT_OPEN_XPATH_STR, bridge_name);
                return SR_ERR_OPERATION_FAILED;
            }

            /* set stream filter instance id value */
            flt_id = node->data.uint32_val;
            snprintf(filter_id, MAX_STR_LEN, "%u", node->data.uint32_val);

            if (SR_OP_CREATED == op) {
                do {
                    /* find stream-handle value */
                    ret = get_item_value_cb(session, &tmp_value, SFG_STREAM_HANDLE_XPATH,
                                            bridge_name, bridge_name, filter_id);
                    if (EXIT_FAILURE == ret) break;
                    params.str_hdl = tmp_value->data.int32_val;
                    sr_free_val(tmp_value);

                    /* find priority-spec value */
                    ret = get_item_value_cb(session, &tmp_value, SFG_PRIORITY_SPEC_XPATH,
                                            bridge_name, bridge_name, filter_id);
                    if (EXIT_FAILURE == ret) break;
                    strncpy(tmp_str, tmp_value->data.enum_val, MAX_STR_LEN - 1);
                    if (EXIT_FAILURE == translate_single_string_using_map(tmp_str, sfg_prio_spec_reverse_map)) break;
                    params.prio = strtoul(tmp_str, NULL, 10);
                    sr_free_val(tmp_value);

                    /* find max-sdu-size value */
                    ret = get_item_value_cb(session, &tmp_value, SFG_MAX_SDU_SIZE_XPATH,
                                            bridge_name, bridge_name, filter_id);
                    if (EXIT_FAILURE == ret) break;
                    params.max_sdu = tmp_value->data.uint32_val;
                    sr_free_val(tmp_value);

                    /* find stream-blocked-due-to-oversize-frame-enabled value */
                    ret = get_item_value_cb(session, &tmp_value, SFG_BLK_OVERZ_ENA_XPATH,
                                            bridge_name, bridge_name, filter_id);
                    if (EXIT_FAILURE == ret) break;
                    params.blk_oversz_ena = tmp_value->data.bool_val;
                    sr_free_val(tmp_value);

                    /* find stream-blocked-due-to-oversize-frame value */
                    ret = get_item_value_cb(session, &tmp_value, SFG_BLK_OVERZ_XPATH,
                                            bridge_name, bridge_name, filter_id);
                    if (EXIT_FAILURE == ret) break;
                    params.blk_oversz = tmp_value->data.bool_val;
                    sr_free_val(tmp_value);

                    /* find stream-gate-ref value */
                    ret = get_item_value_cb(session, &tmp_value, SFG_STREAM_GATE_REF_XPATH,
                                            bridge_name, bridge_name, filter_id);
                    if (EXIT_FAILURE == ret) break;
                    params.gate_id = tmp_value->data.uint32_val;
                    sr_free_val(tmp_value);

                    /* The Flow meter instance ID must be set, so it will be se to 'none'
                     * There are objects for flow meter inside the psfp yang module.
                     * Set it there. */
                    params.fmtr_id = PSFP_FLT_NO_FMTR;
                } while (0);
                if (EXIT_FAILURE == ret) {
                    tsn_port_close(port);
                    return SR_ERR_CALLBACK_FAILED;
                }

                /* TODO: ADD THIS ERROR HANDLING TO ACL CALLBACKS */
                tsn_ret = tsn_psfp_flt_create(port, flt_id, &params);
                if (TSN_ERRNO_ERROR_CODE(tsn_ret) == EEXIST) {
                    sr_set_error(session, node->xpath,
                                 ERR_SFG_ENTRY_EXISTS_STR, node->xpath);
                    tsn_port_close(port);
                    return SR_ERR_OPERATION_FAILED;
                }
                else if (tsn_ret) {
                    sr_set_error(session, node->xpath,
                                 ERR_MSG_AND_EXPL_EXT_FMT_STR, LIT_CREATE_STR,
                                 SFG_STREAM_FILTER_INSTANCE_STR, LIT_ENTRY_STR,
                                 SFG_STREAM_FILTER_INSTANCE_ID_STR, filter_id,
                                 tsn_lib_get_known_ernno_msg(tsn_ret));
                    tsn_port_close(port);
                    return SR_ERR_OPERATION_FAILED;
                }
            }
            if (SR_OP_DELETED == op) {
                if (0 != tsn_psfp_flt_delete(port, flt_id)) {
                    sr_set_error(session, node->xpath,
                                 ERR_SFG_ENTRY_NOT_DELETED_STR, node->xpath);
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
    node = NULL;

    return SR_ERR_OK;
}

int sfg_stream_filters_mod_cb(sr_session_ctx_t *session,
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
    char bridge_name[MAX_STR_LEN] = "";
    char filter_id[MAX_STR_LEN] = "";
    char tmp_str[MAX_STR_LEN] = "";
    sr_xpath_ctx_t st = {0};
    struct psfp_flt_params params;
    struct port_hdl *brdev = NULL;
    int ret = EXIT_SUCCESS;
    tsn_return_value tsn_ret = E_TSN_LIB_NO_ERROR;
    uint32_t flt_id = 0;
    int obj_index = 0;

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    /* if event is SR_EV_DONE or SR_EV_ABORT return flag to false */
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

        if (SR_OP_MODIFIED == op) {
            /* get bridge name from xpath */
            if (EXIT_SUCCESS != get_key_value(session, (char*)node->xpath,
                                              Q_BRIDGE_BRIDGE_ENTRY, Q_BRIDGE_NAME_STR,
                                              &st, bridge_name)) {
                return SR_ERR_OPERATION_FAILED;
            }

            /* get filter id from xpath */
            if (EXIT_SUCCESS != get_key_value(session, node->xpath,
                                              SFG_STREAM_FILTER_INSTANCE_STR,
                                              SFG_STREAM_FILTER_INSTANCE_ID_STR,
                                              &st, filter_id)) {
                return SR_ERR_OPERATION_FAILED;
            }
            flt_id = (uint32_t)strtoul(filter_id, NULL, 10);

            /* open bridge */
            brdev = tsn_port_open_by_name(bridge_name);
            if (NULL == brdev) {
                sr_set_error(session, node->xpath,
                             ERR_PORT_NOT_OPEN_XPATH_STR, bridge_name);
                return SR_ERR_OPERATION_FAILED;
            }
            /* First get the current stream filter parameters */
            tsn_ret = tsn_psfp_flt_get_params(brdev, flt_id, &params);
            if (tsn_ret) {
                sr_set_error(session, node->xpath, ERR_TSN_MSG_FAIL_XPATH_STR,
                             LIT_GET_STR, LIT_VAL_STR, node->xpath,
                             tsn_lib_get_known_ernno_msg(tsn_ret));
                tsn_port_close(brdev);
                return SR_ERR_OPERATION_FAILED;
            }

            /* Now delete the current stream filter parameters, the new entry
             * for same filter ID will created
             */
            if (0 != tsn_psfp_flt_delete(brdev, flt_id)) {
                sr_set_error(session, node->xpath,
                             ERR_SFG_ENTRY_NOT_DELETED_STR, node->xpath);
                tsn_port_close(brdev);
                return SR_ERR_OPERATION_FAILED;
            }

            /* identify the current element and call proper setter function */
            obj_index = find_obj_index_by_name(sr_xpath_node_name(node->xpath),
                                               psfp_map, PSFP_MAX_OBJ_NUMBER);
            switch (obj_index) {
            case PSFP_STREAM_HDL:
                params.str_hdl = node->data.int32_val;
                break;
            case PSFP_PRIO_SPEC:
                snprintf(tmp_str, MAX_STR_LEN, "%s", node->data.enum_val);
                ret = translate_single_string_using_map(tmp_str, sfg_prio_spec_reverse_map);
                params.prio = strtoul(tmp_str, NULL, 10);
                break;
            case PSFP_MAX_SDU_SIZE:
                params.max_sdu = node->data.uint32_val;
                break;
            case PSFP_STR_BLCKD_FRAME_ENBLD:
                params.blk_oversz_ena = node->data.bool_val;
                break;
            case PSFP_STR_BLCKD_FRAME:
                params.blk_oversz = node->data.bool_val;
                break;
            case PSFP_STR_GATE_REF:
                params.gate_id = node->data.uint32_val;
                break;
            default:
                tsn_port_close(brdev);
                return SR_ERR_OPERATION_FAILED;
            }
            /* If translate_single_string_using_map function fails report the
             * invalid value at xpath.
             */
            if (ret) {
                sr_set_error(session, node->xpath, ERR_INVALID_VALUE_XPATH_STR,
                             node->xpath);
                tsn_port_close(brdev);
                return SR_ERR_OPERATION_FAILED;
            }

            tsn_ret = tsn_psfp_flt_create(brdev, flt_id, &params);
            if (TSN_ERRNO_ERROR_CODE(tsn_ret) == EEXIST) {
                sr_set_error(session, node->xpath, ERR_SFG_ENTRY_EXISTS_STR,
                             node->xpath);
                tsn_port_close(brdev);
                return SR_ERR_OPERATION_FAILED;
            }
            else if (tsn_ret) {
                sr_set_error(session, node->xpath, ERR_MSG_AND_EXPL_EXT_FMT_STR,
                             LIT_CREATE_STR, SFG_STREAM_FILTER_INSTANCE_STR,
                             LIT_ENTRY_STR, SFG_STREAM_FILTER_INSTANCE_ID_STR,
                             filter_id, tsn_lib_get_known_ernno_msg(tsn_ret));
                tsn_port_close(brdev);
                return SR_ERR_OPERATION_FAILED;
            }
            tsn_port_close(brdev);
        }
    }

    sr_free_val(new_value);
    sr_free_val(old_value);
    sr_free_change_iter(iter);
    node = NULL;

    return SR_ERR_OK;
}

int sfg_stream_gates_mod_cb(sr_session_ctx_t *session, const char *module_name,
                            const char *xpath, sr_event_t event,
                            uint32_t request_id, void *private_data)
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
    char bridge_name[MAX_STR_LEN] = "";
    char gate_id[MAX_STR_LEN] = "";
    char tmp_str[MAX_STR_LEN] = "";
    sr_xpath_ctx_t st = {0};
    uint32_t gt_id = 0;
    struct port_hdl *brdev = NULL;
    int ret = EXIT_SUCCESS;
    tsn_return_value tsn_ret = E_TSN_LIB_NO_ERROR;
    int obj_index = 0;

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    /* if event is SR_EV_DONE or SR_EV_ABORT return flag to false */
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

        if (SR_OP_MODIFIED == op) {
            /* get bridge name from xpath */
            if (EXIT_SUCCESS != get_key_value(session, (char*)node->xpath,
                                              Q_BRIDGE_BRIDGE_ENTRY,
                                              Q_BRIDGE_NAME_STR,
                                              &st, bridge_name)) {
                return SR_ERR_OPERATION_FAILED;
            }

            /* get gate id from xpath */
            if (EXIT_SUCCESS != get_key_value(session, node->xpath,
                                              SFG_STREAM_GATE_INSTANCE_STR,
                                              SFG_STREAM_GATE_INSTANCE_ID_STR,
                                              &st, gate_id)) {
                return SR_ERR_OPERATION_FAILED;
            }
            gt_id = (uint32_t)strtoul(gate_id, NULL, 10);

            /* open bridge */
            brdev = tsn_port_open_by_name(bridge_name);
            if (NULL == brdev) {
                sr_set_error(session, node->xpath,
                             ERR_PORT_NOT_OPEN_XPATH_STR, bridge_name);
                return SR_ERR_OPERATION_FAILED;
            }

            /* identify the current element and call proper setter function */
            obj_index = find_obj_index_by_name(sr_xpath_node_name(node->xpath),
                                               psfp_map, PSFP_MAX_OBJ_NUMBER);
            switch (obj_index) {
            case PSFP_AGS:
                snprintf(tmp_str, MAX_STR_LEN, "%s", node->data.enum_val);
                ret = translate_single_string_using_map(tmp_str, psfp_ags_reverse_map);
                tsn_ret = tsn_psfp_gt_set_admin_gate_states(brdev, gt_id,
                                                            (uint8_t)strtoul(tmp_str, NULL, 10));
                break;
            case PSFP_ADMIN_IPV:
                snprintf(tmp_str, MAX_STR_LEN, "%s", node->data.enum_val);
                ret = translate_single_string_using_map(tmp_str, sfg_ipv_spec_reverse_map);
                tsn_ret = tsn_psfp_gt_set_admin_ipv(brdev, gt_id,
                                                    (uint8_t)strtoul(tmp_str, NULL, 10));
                break;
            case PSFP_INVALID_RX_ENBLD:
                tsn_ret = tsn_psfp_gt_set_closed_inv_rx_enable(brdev, gt_id, node->data.bool_val);
                break;
            case PSFP_INVALID_RX:
                tsn_ret = tsn_psfp_gt_set_closed_inv_rx(brdev, gt_id, node->data.bool_val);
                break;
            default:
                tsn_port_close(brdev);
                return SR_ERR_OPERATION_FAILED;
            }

            /* If translate_single_string_using_map function fails report the
             * invalid value at xpath.
             */
            if (ret) {
                sr_set_error(session, node->xpath,
                             ERR_INVALID_VALUE_XPATH_STR, node->xpath);
                tsn_port_close(brdev);
                return SR_ERR_OPERATION_FAILED;
            }
            /* If TSN function failed report that set value failed at xpath,
             * with libtsn error message.
             */
            if (tsn_ret) {
                sr_set_error(session, node->xpath, ERR_TSN_MSG_FAIL_XPATH_STR,
                             LIT_SET_STR, LIT_VAL_STR, node->xpath,
                             tsn_lib_get_known_ernno_msg(tsn_ret));
                tsn_port_close(brdev);
                return SR_ERR_OPERATION_FAILED;
            }
            tsn_port_close(brdev);
        }
    }

    sr_free_val(new_value);
    sr_free_val(old_value);
    sr_free_change_iter(iter);
    node = NULL;

    return SR_ERR_OK;
}

int psfp_gcl_validation(sr_session_ctx_t *session, sr_val_t *node,
                        struct port_hdl *brdev, char *bridge_name,
                        char *gate_id)
{
    uint32_t gcl_len = 0u;
    uint32_t gt_id = 0u;
    sr_val_t *config_change = NULL;

    SRP_LOG_DBG(DEBUG_MSG_FUN_CALLED_WITH_TWO_PARAM, __func__,
                bridge_name, gate_id);

    /* If current config-change in configuration is 'true' checks are needed.
     * If current config-change is "false" it not necessary to have complete
     * configuration and status of link detection is not important.
     */
    if (EXIT_FAILURE == get_item_value_cb(session, &config_change,
                                          PSFP_CONFIG_CHANGE_XPATH,
                                          bridge_name, bridge_name, gate_id)) {
        return EXIT_FAILURE;
    }

    gt_id = (uint32_t)strtoul(gate_id, NULL, 10);

    if (1 == config_change->data.bool_val) {
         /* admin-control-list can be added even though link is not detected.
          * oper-control-list can be added to the port that is not UP also,
          * only advertised link modes on board has to have one value.
          *
          * Check does admin-control-list exists. If admin-control-list does
          * not exist on system, configuration is not complete, so it is not
          * possible to set config-change to 'true'.
         */
        if (EXIT_FAILURE == tsn_psfp_gt_get_admin_control_list_length(brdev, gt_id, &gcl_len)) {
            SRP_LOG_ERR(ERROR_MSG_FUN_GETTER_FAILED, node->xpath);
            sr_set_error(session, node->xpath,
                         ERROR_MSG_FUN_GETTER_FAILED, BASE_ACL_LENGTH);
            return EXIT_FAILURE;
        }
        if (0 == gcl_len) {
            SRP_LOG_ERR(ERR_PSFP_ACL_MISSING_STR, bridge_name, gate_id);
            sr_set_error(session, node->xpath,
                         ERR_PSFP_ACL_MISSING_STR, bridge_name, gate_id);
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}
int psfp_ocl_list_entry(struct lyd_node **parent, char *bridge_name,
                        char *gate_id, int i,
                        struct psfp_gate_control_entry *ocl)
{
    char path[2*MAX_STR_LEN] = "";
    char tmp_str[MAX_STR_LEN] = "";
    char tmp_index[MAX_STR_LEN] = "";

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    /* list gate-control-entry, leaf operation-name */
    switch (ocl[i].op) {
    case 0:
        snprintf(tmp_str, MAX_STR_LEN, "%s", PSFP_SET_GATE_AND_IPV_STR);
        break;
    default:
        return SR_ERR_OPERATION_FAILED;
    }

    /* list gate-control-entry, leaf operation-name */
    snprintf(tmp_index, MAX_STR_LEN, "%d", i);
    fill_xpath(path, PSFP_OCL_OP_NAME_XPATH, bridge_name, bridge_name, gate_id, tmp_index);
    if (NULL == lyd_new_path(*parent, NULL, path, tmp_str, 0, 0)) {
        SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                    path, ERR_NOT_CREATED_ELEMENT_STR);
    }

    /* list gate-control-entry, leaf time-interval-value */
    fill_xpath(path, PSFP_OCL_TIME_INT_VAL_XPATH, bridge_name, bridge_name, gate_id, tmp_index);
    snprintf(tmp_str, MAX_STR_LEN, "%u", ocl[i].interval);
    if (NULL == lyd_new_path(*parent, NULL, path, tmp_str, 0, 0)) {
        SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                    path, ERR_NOT_CREATED_ELEMENT_STR);
    }

    /* list gate-control-entry, leaf gate-state-value */
    fill_xpath(path, PSFP_OCL_GATE_STATE_XPATH, bridge_name, bridge_name, gate_id, tmp_index);
    snprintf(tmp_str, MAX_STR_LEN, "%u", ocl[i].gsv);
    if (EXIT_SUCCESS == translate_single_string_using_map(tmp_str, psfp_ags_map)) {
        if (NULL == lyd_new_path(*parent, NULL, path, tmp_str, 0, 0)) {
            SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                        path, ERR_NOT_CREATED_ELEMENT_STR);
        }
    }

    /* list gate-control-entry, leaf ipv-spec */
    fill_xpath(path, PSFP_OCL_IPV_SPEC_XPATH, bridge_name, bridge_name, gate_id, tmp_index);
    snprintf(tmp_str, MAX_STR_LEN, "%d", ocl[i].ipv);
    if (EXIT_SUCCESS == translate_single_string_using_map(tmp_str, sfg_ipv_spec_map)) {
        if (NULL == lyd_new_path(*parent, NULL, path, tmp_str, 0, 0)) {
            SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                        path, ERR_NOT_CREATED_ELEMENT_STR);
        }
    }

    return SR_ERR_OK;
}

int psfp_ptp_time_entry(struct lyd_node **parent,
                        struct tsn_st_ptp_time *st_time, char *bridge_name,
                        char *gate_id, char *container_name)
{
    char path[2*MAX_STR_LEN] = "";
    char tmp[MAX_STR_LEN] = "";

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    /* leaf seconds */
    fill_xpath(path, PSFP_SECONDS_XPATH, bridge_name, bridge_name, gate_id, container_name);
    snprintf(tmp, MAX_STR_LEN, "%llu", (unsigned long long int)st_time->sec);
    if (NULL == lyd_new_path(*parent, NULL, path, tmp, 0, 0)) {
        SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                    path, ERR_NOT_CREATED_ELEMENT_STR);
    }

    /* leaf nanoseconds */
    fill_xpath(path, PSFP_NANOSECONDS_XPATH, bridge_name, bridge_name, gate_id, container_name);
    snprintf(tmp, MAX_STR_LEN, "%u", st_time->nsec);
    if (NULL == lyd_new_path(*parent, NULL, path, tmp, 0, 0)) {
        SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                    path, ERR_NOT_CREATED_ELEMENT_STR);
    }

    return SR_ERR_OK;
}

int psfp_acl_cb(sr_session_ctx_t *session, const char *module_name,
                const char *xpath, sr_event_t event, uint32_t request_id,
                void *private_data)
{
    (void)event;
    (void)module_name;
    (void)request_id;
    (void)private_data;

    int rc = SR_ERR_OK;
    sr_change_iter_t *iter = NULL;
    sr_change_oper_t op = {0};
    sr_xpath_ctx_t st = {0};
    sr_val_t *old_node = NULL;
    sr_val_t *new_node = NULL;
    sr_val_t *node = NULL;
    sr_val_t *acl_values = NULL;
    sr_val_t *tmp_value = NULL;
    sr_val_t *gcl_values = NULL;
    struct psfp_gate_control_entry *gcl = NULL;
    struct port_hdl *brdev = NULL;
    char bridge_name[MAX_STR_LEN] = "";
    char gate_id[MAX_STR_LEN] = "";
    char tmp_path[2*MAX_STR_LEN] = "";
    char tmp_op_name[MAX_STR_LEN] = "";
    char tmp_gate_state_val[MAX_STR_LEN] = "";
    char tmp_ipv_spec[MAX_STR_LEN] = "";
    char tmp_gcl_index[MAX_STR_LEN] = "";
    int ret = 0;
    int i = 0;
    uint32_t gcl_len = 0u;
    uint32_t gcl_index = 0u;
    uint32_t time_interval_value = 0u;
    uint32_t old_sched_index = 0u;
    uint32_t valid_gcl_len = 0u;// 0 is valid value
    uint32_t num_acl = 0;
    uint32_t gt_id = 0;
    uint32_t granularity = 0;
    size_t acl_count = 0;
    size_t gcl_count = 0;

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
        /* get bridge name from xpath */
        if (EXIT_SUCCESS != get_key_value(session, node->xpath,
                                          Q_BRIDGE_BRIDGE_ENTRY,
                                          Q_BRIDGE_NAME_STR,
                                          &st, bridge_name)) {
            return SR_ERR_OPERATION_FAILED;
        }

        /* get gate id from xpath */
        if (EXIT_SUCCESS != get_key_value(session, node->xpath,
                                          SFG_STREAM_GATE_INSTANCE_STR,
                                          SFG_STREAM_GATE_INSTANCE_ID_STR,
                                          &st, gate_id)) {
            return SR_ERR_OPERATION_FAILED;
        }
        gt_id = (uint32_t)strtoul(gate_id, NULL, 10);

        /* open port by name and check is feature supported on the port */
        brdev = tsn_port_open_by_name(bridge_name);
        if (NULL == brdev) {
            SRP_LOG_ERR(ERR_PORT_NOT_OPEN_XPATH_STR, bridge_name);
            sr_set_error(session, node->xpath,
                         ERR_PORT_NOT_OPEN_XPATH_STR, bridge_name);
            return SR_ERR_OPERATION_FAILED;
        }

        /* get entire admin-control-list container and count number of
         * gate-control-list entries inside
         */
        fill_xpath(tmp_path, PSFP_ACL_CONTAINER_XPATH, bridge_name, bridge_name, gate_id);
        ret = sr_get_items(session, tmp_path, 0, 0, &acl_values, &acl_count);
        if ((SR_ERR_OK != ret) && (SR_ERR_NOT_FOUND != ret)) {
            // if item is not found the count is 0
            SRP_LOG_ERR(PSFP_ACL_CONTAINER_XPATH, bridge_name);
            sr_set_error(session, node->xpath,
                         PSFP_ACL_CONTAINER_XPATH, bridge_name);
            return SR_ERR_OPERATION_FAILED;
        }

        i = 0;
        for (i = 0; i < (int)acl_count; i++) {
            if (NULL != strstr(acl_values[i].xpath, "gate-control-entry[index")) {
                num_acl++;
            }
        }
        sr_free_values(acl_values, acl_count);
        gcl_len = num_acl;

        /* get gate-control-list entries and check if they are in acending order */
        fill_xpath(tmp_path, PSFP_GCL_LIST_XPATH, bridge_name, bridge_name, gate_id);
        ret = sr_get_items(session, tmp_path, 0, 0, &gcl_values, &gcl_count);
        if ((SR_ERR_OK != ret) && (SR_ERR_NOT_FOUND != ret)) {
            // if item is not found the count is 0
            SRP_LOG_ERR(PSFP_GCL_LIST_XPATH, bridge_name, gate_id);
            sr_set_error(session, node->xpath,
                         PSFP_GCL_LIST_XPATH, bridge_name, gate_id);
            return SR_ERR_OPERATION_FAILED;
        }

        i = 0;
        if (SR_ERR_NOT_FOUND != ret) {
            old_sched_index = 0;
            for (i = 0; i < (int)gcl_count; i++) {
                if (sr_xpath_node_name_eq(gcl_values[i].xpath, PSFP_INDEX_STR)) {
                    if ((gcl_values[i].data.uint32_val - old_sched_index) > 1) {
                        SRP_LOG_ERR(ERR_GCL_WRONG_INDEX_STR, bridge_name, gate_id);
                        sr_set_error(session, gcl_values[i].xpath,
                                     ERR_GCL_WRONG_INDEX_STR, bridge_name, gate_id);
                        return SR_ERR_OPERATION_FAILED;
                    }
                    old_sched_index = gcl_values[i].data.uint32_val;
                    if (gcl_len == (gcl_values[i].data.uint32_val + 1)) {
                        valid_gcl_len++;
                        break;
                    }
                }
            }
        }
        sr_free_values(gcl_values, gcl_count);

        if (!valid_gcl_len) {
            if (0 == gcl_len) {
                // when gcl_len=0, and there is no acl list in config, valid situation
                /* set new admin control list length */
                if (tsn_psfp_gt_set_admin_control_list_length(brdev, gt_id, gcl_len)) {
                    SRP_LOG_ERR(ERR_SET_FUNC_FAILED_XPATH_STR, node->xpath);
                    sr_set_error(session, node->xpath,
                                 ERR_SET_FUNC_FAILED_XPATH_STR, node->xpath);
                    tsn_port_close(brdev);
                    return SR_ERR_OPERATION_FAILED;
                }
                tsn_port_close(brdev);
            } else {
                SRP_LOG_ERR(ERR_GCL_WRONG_LEGTH_STR, bridge_name);
                sr_set_error(session, node->xpath,
                             ERR_GCL_WRONG_LEGTH_STR, bridge_name);
                return SR_ERR_OPERATION_FAILED;
            }
        } else {
            /* set new admin control list length */
            if (tsn_psfp_gt_set_admin_control_list_length(brdev, gt_id, gcl_len)) {

                SRP_LOG_ERR(ERR_SET_FUNC_FAILED_XPATH_STR, node->xpath);
                sr_set_error(session, node->xpath,
                             ERR_SET_FUNC_FAILED_XPATH_STR, node->xpath);
                tsn_port_close(brdev);
                return SR_ERR_OPERATION_FAILED;
            }
        }

        num_acl = 0;
        valid_gcl_len = 0;

        if ((op == SR_OP_CREATED) &&
            (sr_xpath_node_name_eq(node->xpath, PSFP_INDEX_STR))) {
            gcl_index = node->data.uint32_val;

            do {
                if (gcl_len > MAX_NUMBER_OF_ACL) {
                    SRP_LOG_ERR(ERR_GCL_MAX_VALUE_STR, bridge_name, gate_id);
                    sr_set_error(session, node->xpath,
                                 ERR_GCL_MAX_VALUE_STR, bridge_name, gate_id);
                    ret = 1;
                    break;
                }

                /* check sched index from admin control list */
                if (gcl_index >= gcl_len) {
                    SRP_LOG_ERR(ERR_GCL_INDEX_BONDS_OUT_STR, bridge_name, gate_id);
                    sr_set_error(session, node->xpath,
                                 ERR_GCL_INDEX_BONDS_OUT_STR, bridge_name, gate_id);
                    ret = 1;
                    break;
                }
                snprintf(tmp_gcl_index, MAX_STR_LEN, "%u", gcl_index);

                /* set new admin control list length */
                ret = tsn_psfp_gt_set_admin_control_list_length(brdev, gt_id, gcl_len);
                if (ret) {
                    SRP_LOG_ERR(ERR_SET_FUNC_FAILED_XPATH_STR, node->xpath);
                    sr_set_error(session, node->xpath,
                                 ERR_SET_FUNC_FAILED_XPATH_STR, node->xpath);
                    break;
                }

                /* validate PSFP configuration */
                ret = psfp_gcl_validation(session, node, brdev, bridge_name, gate_id);
                if (EXIT_FAILURE == ret) break;

                /* find psfp operation-name from gate-control-entry */
                ret = get_item_value_cb(session, &tmp_value, PSFP_GCL_OP_NAME_XPATH,
                                        bridge_name, bridge_name, gate_id, tmp_gcl_index);
                if (ret) {
                    break;
                }
                snprintf(tmp_op_name, MAX_STR_LEN, "%s", tmp_value->data.identityref_val);
                if (0 != strncmp(tmp_op_name, "ieee802-dot1q-psfp:set-gate-and-ipv", 36)) {
                    break;
                }
                sr_free_val(tmp_value);

                /* find PSFP gate-state-value */
                ret = get_item_value_cb(session, &tmp_value, PSFP_GCL_GATE_STATE_XPATH,
                                        bridge_name, bridge_name, gate_id, tmp_gcl_index);
                if (ret) break;
                snprintf(tmp_gate_state_val, MAX_STR_LEN, "%s", tmp_value->data.enum_val);
                if (EXIT_FAILURE == translate_single_string_using_map(tmp_gate_state_val, psfp_ags_reverse_map)) {
                    break;
                }
                sr_free_val(tmp_value);

                /* find psfp time-interval-value */
                ret = get_item_value_cb(session, &tmp_value, PSFP_GCL_TIME_INTERVAL_XPATH,
                                        bridge_name, bridge_name, gate_id, tmp_gcl_index);
                if (ret) break;
                time_interval_value = tmp_value->data.uint32_val;
                /* Check is time interval value aligned with tick granularity.
                 * time-interval-value needs to be an integer multiple of
                 * tick-granularity.
                 */
                ret = tsn_psfp_gt_get_tick_granularity(brdev, gt_id, &granularity);
                if (ret) {
                    SRP_LOG_ERR(ERR_GET_FUNC_FAILED_XPATH_STR, tmp_value->xpath);
                    sr_set_error(session, node->xpath,
                                 ERR_GET_FUNC_FAILED_XPATH_STR, tmp_value->xpath);
                    break;
                }
                if (0 != (time_interval_value % (granularity/10))) {
                    SRP_LOG_ERR(ERR_GRANULARITY_INTERVALS_STR,
                                granularity/10, tmp_value->xpath);
                    sr_set_error(session, node->xpath,
                                 ERR_GRANULARITY_INTERVALS_STR,
                                 granularity/10, tmp_value->xpath);
                    ret = 1;
                    break;
                }
                sr_free_val(tmp_value);

                /* find PSFP ipv-spec */
                ret = get_item_value_cb(session, &tmp_value, PSFP_GCL_IPV_SPEC_XPATH,
                                        bridge_name, bridge_name, gate_id, tmp_gcl_index);
                if (ret) break;
                snprintf(tmp_ipv_spec, MAX_STR_LEN, "%s", tmp_value->data.enum_val);
                if (EXIT_FAILURE == translate_single_string_using_map(tmp_ipv_spec, sfg_ipv_spec_reverse_map)) {
                    break;
                }
                sr_free_val(tmp_value);

                /* set new admin control list length */
                ret = tsn_psfp_gt_get_admin_control_list_length(brdev, gt_id, &gcl_len);
                if (ret) {
                    SRP_LOG_ERR(ERR_GET_FUNC_FAILED_XPATH_STR, node->xpath);
                    sr_set_error(session, node->xpath,
                                 ERR_GET_FUNC_FAILED_XPATH_STR, node->xpath);
                    break;
                }

                if ((op & SR_OP_CREATED) &&
                    ((0 == gcl_len) || (MAX_NUMBER_OF_ACL < gcl_len))) {
                    SRP_LOG_ERR(ERR_PSFP_GCL_LEN_MSG_STR, node->xpath);
                    sr_set_error(session, node->xpath,
                                 ERR_PSFP_GCL_LEN_MSG_STR, bridge_name, gate_id);
                    break;
                }

                gcl = (struct psfp_gate_control_entry*)malloc(sizeof(*gcl) * (gcl_index + 1));

                if (NULL != gcl) {
                    /* if index is 0 for ADD operation that means current admin
                     * control list is empty, so we do not need to get current
                     * values
                     */
                    if (0 != gcl_index) {
                        /* get current values */
                        ret = tsn_psfp_gt_get_admin_control_list(brdev, gt_id, gcl_index, gcl);
                        if (ret) {
                            SRP_LOG_ERR(ERR_GET_FUNC_FAILED_XPATH_STR, node->xpath);
                            sr_set_error(session, node->xpath,
                                         ERR_GET_FUNC_FAILED_XPATH_STR, node->xpath);
                            free(gcl);
                            break;
                        }
                    }

                    gcl[gcl_index].op = TSN_PSFP_GT_OP_SET_GATE_AND_IPV;
                    gcl[gcl_index].gsv = (uint8_t)strtoul(tmp_gate_state_val, NULL, 10);
                    gcl[gcl_index].interval = time_interval_value;
                    gcl[gcl_index].ipv = (int8_t)strtoul(tmp_ipv_spec, NULL, 10);

                    ret = tsn_psfp_gt_set_admin_control_list(brdev, gt_id, (gcl_index + 1), gcl);
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
                tsn_port_close(brdev);
                return SR_ERR_OPERATION_FAILED;
            }

            tsn_port_close(brdev);
            /* free resources */
            if (NULL != gcl) {
                free(gcl);
            }
        }
    }

    sr_free_val(old_node);
    sr_free_val(new_node);
    node = NULL;
    sr_free_change_iter(iter);

    return rc;
}

int psfp_acl_mod_cb(sr_session_ctx_t *session, const char *module_name,
                    const char *xpath, sr_event_t event, uint32_t request_id,
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
    struct psfp_gate_control_entry *gcl = NULL;
    struct port_hdl *brdev = NULL;
    char bridge_name[MAX_STR_LEN] = "";
    char gate_id[MAX_STR_LEN] = "";
    char tmp_value[MAX_STR_LEN] = "";
    char tmp_gcl_index[MAX_STR_LEN] = "";
    int ret = 0;
    uint32_t gt_id = 0u;
    uint32_t gcl_len = 0u;
    uint32_t gcl_index = 0u;
    uint32_t granularity = 0;
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

        /* get bridge name from xpath */
        if (EXIT_SUCCESS != get_key_value(session, node->xpath,
                                          Q_BRIDGE_BRIDGE_ENTRY,
                                          Q_BRIDGE_NAME_STR,
                                          &st, bridge_name)) {
            return SR_ERR_OPERATION_FAILED;
        }

        /* get gate id from xpath */
        if (EXIT_SUCCESS != get_key_value(session, node->xpath,
                                          SFG_STREAM_GATE_INSTANCE_STR,
                                          SFG_STREAM_GATE_INSTANCE_ID_STR,
                                          &st, gate_id)) {
            return SR_ERR_OPERATION_FAILED;
        }
        gt_id = (uint32_t)strtoul(gate_id, NULL, 10);

        if ((op == SR_OP_MODIFIED)) {
            /* open port by name and check is feature supported on the port */
            brdev = tsn_port_open_by_name(bridge_name);
            if (NULL == brdev) {
                SRP_LOG_ERR(ERR_PORT_NOT_OPEN_XPATH_STR, bridge_name);
                sr_set_error(session, node->xpath,
                             ERR_PORT_NOT_OPEN_XPATH_STR, bridge_name);
                return SR_ERR_OPERATION_FAILED;
            }

            do {
                /* find index from gate-control-entry */
                if (EXIT_SUCCESS != get_key_value(session, node->xpath,
                                                  PSFP_GCL_ENTRY_STR,
                                                  PSFP_INDEX_STR,
                                                  &st, tmp_gcl_index)) {
                    return SR_ERR_OPERATION_FAILED;
                }
                gcl_index = strtol(tmp_gcl_index, NULL, 10);

                /* get number of gate-control-entry entries from board */
                ret = tsn_psfp_gt_get_admin_control_list_length(brdev, gt_id, &gcl_len);
                if (ret) {
                    SRP_LOG_ERR(ERR_GET_FUNC_FAILED_XPATH_STR, node->xpath);
                    sr_set_error(session, node->xpath,
                                 ERR_GET_FUNC_FAILED_XPATH_STR, node->xpath);
                    break;
                }

                /* If operation if modify, then entry of admin control list must exist */
                if ((op == SR_OP_MODIFIED) && (0 == gcl_len)) {
                    SRP_LOG_ERR(ERR_PSFP_GCL_LEN_MSG_STR, node->xpath);
                    sr_set_error(session, node->xpath,
                                 ERR_PSFP_GCL_LEN_MSG_STR, bridge_name, gate_id);
                    break;
                }

                gcl = (struct psfp_gate_control_entry*)malloc(sizeof(*gcl)*gcl_len);
                if (NULL != gcl) {
                    /* get current values */
                    ret = tsn_psfp_gt_get_admin_control_list(brdev, gt_id, gcl_len, gcl);
                    if (ret) {
                        SRP_LOG_ERR(ERR_GET_FUNC_FAILED_XPATH_STR, node->xpath);
                        sr_set_error(session, node->xpath,
                                     ERR_GET_FUNC_FAILED_XPATH_STR, node->xpath);
                        break;
                    }

                    if (gcl_index >= gcl_len) {
                        SRP_LOG_ERR(ERR_GCL_INDEX_BONDS_OUT_STR, bridge_name, gate_id);
                        sr_set_error(session, node->xpath,
                                     ERR_GCL_INDEX_BONDS_OUT_STR, bridge_name, gate_id);
                        break;
                    }

                    /* modify the proper object of the st_gate_control_entry structure */
                    obj_index = find_obj_index_by_name(sr_xpath_node_name(node->xpath),
                                                       psfp_map, PSFP_MAX_OBJ_NUMBER);
                    switch (obj_index) {
                    case PSFP_GATE_STATE:
                        snprintf(tmp_value, MAX_STR_LEN, "%s", node->data.enum_val);
                        if (EXIT_FAILURE == translate_single_string_using_map(tmp_value, psfp_ags_reverse_map)) {
                            sr_set_error(session, node->xpath,
                                         "Translator failed to map psfp gsv value.");
                            ret = 1;
                            break;
                        }
                        gcl[gcl_index].gsv = (uint8_t)strtoul(tmp_value, NULL, 10);
                        break;
                    case PSFP_TIME_INTERVAL:
                        gcl[gcl_index].interval = node->data.uint32_val;
                        /* Check is time interval value aligned with tick
                         * granularity. time-interval-value needs to be an
                         * integer multiple of tick-granularity.
                         */
                        ret = tsn_psfp_gt_get_tick_granularity(brdev, gt_id, &granularity);
                        if (ret) {
                            SRP_LOG_ERR(ERR_GET_FUNC_FAILED_XPATH_STR, node->xpath);
                            sr_set_error(session, node->xpath, ERR_GET_FUNC_FAILED_XPATH_STR, node->xpath);
                            break;
                        }
                        if (0 != (gcl[gcl_index].interval % (granularity/10))) {
                            SRP_LOG_ERR(ERR_GRANULARITY_INTERVALS_STR,
                                        granularity/10, node->xpath);
                            sr_set_error(session, node->xpath,
                                         ERR_GRANULARITY_INTERVALS_STR,
                                         granularity/10, node->xpath);
                            ret = 1;
                            break;
                        }
                        break;
                    case PSFP_OP_NAME:
                        /* map the operation name to integer value for gcl.op */
                        snprintf(tmp_value, MAX_STR_LEN, "%s", node->data.identityref_val);
                        if (0 != strncmp(tmp_value, "ieee802-dot1q-psfp:set-gate-and-ipv", 36)) {
                            sr_set_error(session, node->xpath,
                                         "Unknown operation-name. Only set-gate-and-ipv is supported!");
                            ret = 1;
                            break;
                        }
                        gcl[gcl_index].op = TSN_PSFP_GT_OP_SET_GATE_AND_IPV;
                        break;
                    case PSFP_IPV_SPEC:
                        /* map the operation name to integer value for gcl.ipv */
                        snprintf(tmp_value, MAX_STR_LEN, "%s", node->data.enum_val);
                        if (EXIT_FAILURE == translate_single_string_using_map(tmp_value, sfg_ipv_spec_reverse_map)) {
                            sr_set_error(session, node->xpath,
                                         "Translator failed to map psfp ipv-spec value.");
                            ret = 1;
                            break;
                        }
                        gcl[gcl_index].ipv = (int8_t)strtoul(tmp_value, NULL, 10);
                        break;
                    }

                    if (ret) {
                        free(gcl);
                        break;
                    }

                    ret = tsn_psfp_gt_set_admin_control_list(brdev, gt_id, gcl_len, gcl);
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
                tsn_port_close(brdev);
                return SR_ERR_OPERATION_FAILED;
            }
            tsn_port_close(brdev);

            /* free resources */
            if (NULL != gcl) {
                free(gcl);
            }
        }
    }

    sr_free_val(old_node);
    sr_free_val(new_node);
    node = NULL;
    sr_free_change_iter(iter);

    return rc;
}

int psfp_stream_gates_cb(sr_session_ctx_t *session, const char *module_name,
                         const char *xpath, sr_event_t event,
                         uint32_t request_id, void *private_data)
{
    (void)event;
    (void)module_name;
    (void)request_id;
    (void)private_data;

    tsn_return_value tsn_ret_val = E_TSN_LIB_NO_ERROR;
    int rc = SR_ERR_OK;
    sr_change_oper_t op = {0};
    sr_xpath_ctx_t st = {0};
    sr_change_iter_t *iter = NULL;
    sr_val_t *old_value = NULL;
    sr_val_t *new_value = NULL;
    sr_val_t *node = NULL;
    sr_val_t *tmp_value = NULL;
    struct port_hdl *brdev = NULL;
    struct tsn_st_ptp_time st_time = {0u, 0u};
    struct tsn_st_rational_number st_rational = {0u, 0u};
    char bridge_name[MAX_STR_LEN] = "";
    char gate_id[MAX_STR_LEN] = "";
    char err_msg[LONG_STR_LEN] = "";
    int ret = EXIT_SUCCESS;
    uint32_t gt_id = 0u;
    uint32_t act_ext = 0u;

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    if (0 == plugin_init) {
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

        /* get bridge name from xpath */
        if (EXIT_SUCCESS != get_key_value(session, node->xpath,
                                          Q_BRIDGE_BRIDGE_ENTRY,
                                          Q_BRIDGE_NAME_STR,
                                          &st, bridge_name)) {
            return SR_ERR_OPERATION_FAILED;
        }

        /* get gate id from xpath */
        if (EXIT_SUCCESS != get_key_value(session, node->xpath,
                                          SFG_STREAM_GATE_INSTANCE_STR,
                                          SFG_STREAM_GATE_INSTANCE_ID_STR,
                                          &st, gate_id)) {
            return SR_ERR_OPERATION_FAILED;
        }
        gt_id = (uint32_t)strtoul(gate_id, NULL, 10);

        if ((op == SR_OP_MODIFIED) || (op == SR_OP_CREATED)) {
            /* open port by name and check is feature suppported on the port */
            brdev = tsn_port_open_by_name(bridge_name);
            if (NULL == brdev) {
                SRP_LOG_ERR(ERR_PORT_NOT_OPEN_XPATH_STR, bridge_name);
                sr_set_error(session, node->xpath,
                             ERR_PORT_NOT_OPEN_XPATH_STR, bridge_name);
                return SR_ERR_OPERATION_FAILED;
            }

            /* check and get item values */
            do {
                /* Find seconds from admin-base-time */
                ret = get_item_value_cb(session, &tmp_value,
                                        PSFP_ABT_SECONDS_XPATH,
                                        bridge_name, bridge_name, gate_id);
                if (EXIT_FAILURE == ret) break;
                st_time.sec = tmp_value->data.uint64_val;
                sr_free_val(tmp_value);
                /* Find nanoseconds from admin-base-time */
                ret = get_item_value_cb(session, &tmp_value,
                                        PSFP_ABT_NANOSECONDS_XPATH,
                                        bridge_name, bridge_name, gate_id);
                if (EXIT_FAILURE == ret) break;
                st_time.nsec = (uint32_t)tmp_value->data.uint64_val;
                sr_free_val(tmp_value);

                /* Find numerator from admin-cycle-time */
                ret = get_item_value_cb(session, &tmp_value,
                                        PSFP_ACT_NUMERATOR_XPATH,
                                        bridge_name, bridge_name, gate_id);
                if (EXIT_FAILURE == ret) break;
                st_rational.numerator = tmp_value->data.uint32_val;
                sr_free_val(tmp_value);

                /* Find denominator from admin-cycle-time */
                ret = get_item_value_cb(session, &tmp_value,
                                        PSFP_ACT_DENOMINATOR_XPATH,
                                        bridge_name, bridge_name, gate_id);
                if (EXIT_FAILURE == ret) break;
                st_rational.denominator = tmp_value->data.uint32_val;
                sr_free_val(tmp_value);

                /* Find admin-cycle-time-extension */
                ret = get_item_value_cb(session, &tmp_value,
                                        PSFP_ACT_EXTENSION_XPATH,
                                        bridge_name, bridge_name, gate_id);
                if (EXIT_FAILURE == ret) break;
                act_ext = tmp_value->data.uint32_val;
                sr_free_val(tmp_value);

            } while (0);

            if (EXIT_FAILURE == ret) {
                tsn_port_close(brdev);
                return SR_ERR_CALLBACK_FAILED;
            }

            /* set time element values */
            do {
                /* set admin-base-time */
                if (E_TSN_LIB_NO_ERROR != (tsn_ret_val = tsn_psfp_gt_set_admin_base_time(brdev, gt_id, &st_time))) {
                    snprintf(err_msg, LONG_STR_LEN, ERR_TSN_MSG_FAIL_STR,
                             LIT_SET_STR, PSFP_ADMIN_BASE_TIME_STR, LIT_VAL_STR,
                             tsn_lib_get_known_ernno_msg(tsn_ret_val));
                    ret = EXIT_FAILURE;
                    break;
                }

                /* set admin-cycle-time */
                if (E_TSN_LIB_NO_ERROR != (tsn_ret_val = tsn_psfp_gt_set_admin_cycle_time(brdev, gt_id, &st_rational))) {
                    snprintf(err_msg, LONG_STR_LEN, ERR_TSN_MSG_FAIL_STR,
                             LIT_SET_STR, PSFP_ADMIN_CYCLE_TIME_STR, LIT_VAL_STR,
                             tsn_lib_get_known_ernno_msg(tsn_ret_val));
                    ret = EXIT_FAILURE;
                    break;
                }

                /* set admin-cycle-time-extension */
                if (E_TSN_LIB_NO_ERROR != (tsn_ret_val = tsn_psfp_gt_set_admin_cycle_time_extension(brdev, gt_id, act_ext))) {
                    snprintf(err_msg, LONG_STR_LEN, ERR_TSN_MSG_FAIL_STR, LIT_SET_STR,
                             PSFP_ADMIN_CYCLE_TIME_EXT_STR, LIT_VAL_STR,
                             tsn_lib_get_known_ernno_msg(tsn_ret_val));
                    ret = EXIT_FAILURE;
                    break;
                }

            } while (0);
            if (EXIT_FAILURE == ret) {
                SRP_LOG_ERRMSG(err_msg);
                sr_set_error(session, node->xpath, err_msg);
                tsn_port_close(brdev);
                return SR_ERR_OPERATION_FAILED;
            }
            tsn_port_close(brdev);
        }
        if (op & SR_OP_DELETED) {
            SRP_LOG_ERR(ERR_REM_OP_NOT_SUPPORTED_XPATH_STR, node->xpath);
            sr_set_error(session, node->xpath,
                         ERR_REM_OP_NOT_SUPPORTED_XPATH_STR, node->xpath);
            return SR_ERR_OPERATION_FAILED;
        }
    }

    sr_free_val(old_value);
    sr_free_val(new_value);
    node = NULL;
    sr_free_change_iter(iter);

    return SR_ERR_OK;
}

/**
 * @brief Write config-change to system.
 *
 * @param[in]   session         Session on which changes were detected.
 * @param[in]   node            Node on which change was detected.
 * @param[in]   brdev           Bridge handle for making the changes.
 * @param[in]   bridge_name     Name of the bridge interface.
 * @param[in]   gt_id           Gate ID.
 * @param[in]   config_change   Value to write.
 * @return      Error code (SR_ERR_OK on success).
 */
static int psfp_set_config_change(sr_session_ctx_t *session,
                                  sr_val_t *node,
                                  struct port_hdl *brdev,
                                  const char *bridge_name,
                                  uint32_t gt_id,
                                  bool config_change)
{
    SRP_LOG_DBG("%s() set bridge %s gate %u config-change %s",
                __func__, bridge_name, gt_id, btoa(config_change));
    tsn_return_value tsn_ret_val = tsn_psfp_gt_set_config_change(brdev, gt_id,
                                                                 config_change);

    if (E_TSN_LIB_NO_ERROR != tsn_ret_val) {
        SRP_LOG_ERR(ERR_TSN_MSG_FAIL_STR, LIT_SET_STR,
                    PSFP_CONFIG_CHANGE_STR, LIT_VAL_STR,
                    tsn_lib_get_known_ernno_msg(tsn_ret_val));
        sr_set_error(session, node->xpath,
                     ERR_TSN_MSG_FAIL_STR, LIT_SET_STR,
                     PSFP_CONFIG_CHANGE_STR, LIT_VAL_STR,
                     tsn_lib_get_known_ernno_msg(tsn_ret_val));
        return SR_ERR_OPERATION_FAILED;
    }

    return SR_ERR_OK;
}

/**
 * @brief Write gate-enabled to system.
 *
 * @param[in]   session         Session on which changes were detected.
 * @param[in]   node            Node on which change was detected.
 * @param[in]   brdev           Bridge handle for making the changes.
 * @param[in]   bridge_name     Name of the bridge interface.
 * @param[in]   gt_id           Gate ID.
 * @param[in]   gate_enabled    Value to write.
 * @return      Error code (SR_ERR_OK on success).
 */
static int psfp_set_gate_enabled(sr_session_ctx_t *session,
                                 sr_val_t *node,
                                 struct port_hdl *brdev,
                                 const char *bridge_name,
                                 uint32_t gt_id,
                                 bool gate_enabled)
{
    SRP_LOG_DBG("%s() set bridge %s gate %u gate-enabled %s",
                __func__, bridge_name, gt_id, btoa(gate_enabled));
    tsn_return_value tsn_ret_val = tsn_psfp_gt_set_gate_enabled(brdev, gt_id,
                                                                gate_enabled);

    if (E_TSN_LIB_NO_ERROR != tsn_ret_val) {
        SRP_LOG_ERR(ERR_TSN_MSG_FAIL_STR, LIT_SET_STR,
                    SFG_GATE_ENABLE_STR, LIT_VAL_STR,
                    tsn_lib_get_known_ernno_msg(tsn_ret_val));
        sr_set_error(session, node->xpath,
                     ERR_TSN_MSG_FAIL_STR, LIT_SET_STR,
                     SFG_GATE_ENABLE_STR, LIT_VAL_STR,
                     tsn_lib_get_known_ernno_msg(tsn_ret_val));

        return SR_ERR_OPERATION_FAILED;
    }

    return SR_ERR_OK;
}

/**
 * @brief Apply gate-enabled and config-change settings to system.
 *
 * @param[in]   session         Session on which changes were detected.
 * @param[in]   node            Node on which change was detected.
 * @param[in]   brdev           Bridge handle for making the changes.
 * @param[in]   bridge_name     Name of the bridge interface.
 * @param[in]   gt_id           Gate ID.
 * @param[in]   gate_enabled    Gate-enabled value to write.
 * @param[in]   config_change   Config-change value to write.
 * @return      Error code (SR_ERR_OK on success).
 */
static int psfp_apply_config_change(sr_session_ctx_t *session,
                                    sr_val_t *node,
                                    struct port_hdl *brdev,
                                    char *bridge_name,
                                    char *gate_id,
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
    uint32_t gt_id = (uint32_t)strtoul(gate_id, NULL, 10);
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
        ret = psfp_gcl_validation(session, node, brdev, bridge_name, gate_id);
        if (ret == EXIT_FAILURE)
            return SR_ERR_CALLBACK_FAILED;
    }

    if (SR_ERR_OK == ret && gate_enabled_first)
        ret = psfp_set_gate_enabled(session, node, brdev,
                                    bridge_name, gt_id, gate_enabled);
    if (SR_ERR_OK == ret)
        ret = psfp_set_config_change(session, node, brdev,
                                     bridge_name, gt_id, config_change);
    if (SR_ERR_OK == ret && !gate_enabled_first)
        ret = psfp_set_gate_enabled(session, node, brdev,
                                    bridge_name, gt_id, gate_enabled);

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
 * @param[in]   brdev           Bridge handle for making the changes to system,
 *                              or NULL to only handle config-change reset for
 *                              datastore.
 * @param[in]   bridge_name     Name of the bridge interface.
 * @param[in]   gate_id         Gate ID.
 * @return      Error code (SR_ERR_OK on success).
 */
static int psfp_handle_config_change(sr_session_ctx_t *session,
                                     struct config_change_list *reset_list,
                                     sr_val_t *node,
                                     struct port_hdl *brdev,
                                     char *bridge_name,
                                     char *gate_id)
{
    sr_val_t *config_change = NULL;
    sr_val_t *gate_enabled = NULL;
    int ret = SR_ERR_OK;

    SRP_LOG_DBG("%s() handle change %s", __func__, node->xpath);

    ret = get_item_value_cb(session, &config_change,
                            PSFP_CONFIG_CHANGE_XPATH,
                            bridge_name, bridge_name, gate_id);
    if (SR_ERR_OK != ret) {
        sr_set_error(session, node->xpath,
                     ERR_FAILED_TO_GET_ITEM_STR " (%s)",
                     PSFP_CONFIG_CHANGE_STR);
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
                            SFG_GATE_ENABLE_XPATH,
                            bridge_name, bridge_name, gate_id);
    if (SR_ERR_OK != ret) {
        sr_set_error(session, node->xpath,
                     ERR_FAILED_TO_GET_ITEM_STR " (%s)",
                     SFG_GATE_ENABLE_STR);
        goto out_free_config_change;
    }
    SRP_LOG_DBG("%s() gate-enabled %s", __func__,
                btoa(gate_enabled->data.bool_val));

    /* This is also used to only fix startup datastore at initialization,
     * in which case brdev is NULL.
     */
    if (brdev) {
        ret = psfp_apply_config_change(session, node, brdev, bridge_name,
                                       gate_id,
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
            SRP_LOG_WRN(ERROR_MSG_MOD_FUNC_EL_MSG, SFG_MODULE_NAME_STR,
                        __func__, bridge_name,
                        "Failed to record this change");
        }
    }

    sr_free_val(gate_enabled);

out_free_config_change:
    sr_free_val(config_change);
    return ret;
}

int psfp_config_change_cb(sr_session_ctx_t *session, const char *module_name,
                          const char *xpath, sr_event_t event,
                          uint32_t request_id, void *private_data)
{
    (void)event;
    (void)module_name;
    (void)request_id;

    // Note that this callback is used for both gate-enabled and config-change.
    struct config_change_list *reset_list = private_data;

    int rc = SR_ERR_OK;
    sr_change_oper_t op = {0};
    sr_xpath_ctx_t st = {0};
    sr_change_iter_t *iter = NULL;
    sr_val_t *old_value = NULL;
    sr_val_t *new_value = NULL;
    sr_val_t *node = NULL;
    struct port_hdl *brdev = NULL;
    char bridge_name[MAX_STR_LEN] = "";
    char gate_id[MAX_STR_LEN] = "";
    int ret = EXIT_SUCCESS;

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
        SRP_LOG_INF(DEBUG_MSG_WITH_TWO_PARAM, DBG_APPLYING_CHANGES_MSG, __func__);
        if (SR_EV_DONE == event) {
            /* Changes applied successfully to datastore, ask config-change
             * helper to reset config-change triggers back to false
             * in datastore.
             */
            config_change_list_enqueue(reset_list);
        }
        return SR_ERR_OK;
    }

    rc = sr_get_changes_iter(session, xpath, &iter);
    if (SR_ERR_OK != rc) {
        return rc;
    }
    while (SR_ERR_OK == sr_get_change_next(session, iter, &op,
                                           &old_value, &new_value)) {
        node = (op == SR_OP_DELETED) ? old_value : new_value;

        /* get bridge name from xpath */
        if (EXIT_SUCCESS != get_key_value(session, node->xpath,
                                          Q_BRIDGE_BRIDGE_ENTRY,
                                          Q_BRIDGE_NAME_STR,
                                          &st, bridge_name)) {
            return SR_ERR_OPERATION_FAILED;
        }

        /* get gate id from xpath */
        if (EXIT_SUCCESS != get_key_value(session, node->xpath,
                                          SFG_STREAM_GATE_INSTANCE_STR,
                                          SFG_STREAM_GATE_INSTANCE_ID_STR,
                                          &st, gate_id)) {
            return SR_ERR_OPERATION_FAILED;
        }

        if ((op == SR_OP_MODIFIED) || (op == SR_OP_CREATED)) {
            /* open port by name and check is feature suppported on the port */
            brdev = tsn_port_open_by_name(bridge_name);
            if (NULL == brdev) {
                SRP_LOG_ERR(ERR_PORT_NOT_OPEN_XPATH_STR, bridge_name);
                sr_set_error(session, node->xpath,
                             ERR_PORT_NOT_OPEN_XPATH_STR, bridge_name);
                return SR_ERR_OPERATION_FAILED;
            }

            /* Handle and record both gate-enabled and config-change for this
             * interface.
             */
            ret = psfp_handle_config_change(session, reset_list, node,
                                            brdev, bridge_name, gate_id);
            if (SR_ERR_OK != ret) {
                ret = SR_ERR_OPERATION_FAILED;
                goto chg_free_port_hdl;
            }

            // close port handle
            tsn_port_close(brdev);
            brdev = NULL;
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
    if (NULL != brdev) {
        tsn_port_close(brdev);
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

    return SR_ERR_OK;
}

int psfp_fix_datastore(sr_session_ctx_t *session, sr_datastore_t datastore)
{
    sr_datastore_t orig_datastore = sr_session_get_ds(session);
    struct config_change_list *reset_list = NULL;
    sr_xpath_ctx_t st = {0};
    sr_val_t *nodes = NULL;
    size_t count = 0;
    size_t i = 0;
    char bridge_name[MAX_STR_LEN] = "";
    char gate_id[MAX_STR_LEN] = "";
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
                       PSFP_CONFIG_CHANGE_SUB_XPATH "|"
                       SFG_GATE_ENABLE_SUB_XPATH,
                       0, 0,
                       &nodes, &count);
    SRP_LOG_DBG("%s() sr_get_items %zu result %d", __func__, count, ret);
    if (SR_ERR_OK != ret)
        goto error;

    for (i = 0; i < count; i++) {
        SRP_LOG_DBG("%s() item %zu/%zu", __func__, i + 1, count);
        sr_val_t *node = &nodes[i];

        ret = get_key_value(session, node->xpath,
                            Q_BRIDGE_BRIDGE_ENTRY, Q_BRIDGE_NAME_STR,
                            &st, bridge_name);
        if (EXIT_SUCCESS != ret) {
            SRP_LOG_WRN("%s: %s (%s)", __func__,
                        ERR_MISSING_ELEMENT_STR, Q_BRIDGE_NAME_STR);
            // Not failing plugin startup because of this.
            ret = SR_ERR_OK;
            continue;
        }

        ret = get_key_value(session, node->xpath,
                            SFG_STREAM_GATE_INSTANCE_STR,
                            SFG_STREAM_GATE_INSTANCE_ID_STR,
                            &st, gate_id);
        if (EXIT_SUCCESS != ret) {
            SRP_LOG_WRN("%s: %s (%s)", __func__,
                        ERR_MISSING_ELEMENT_STR, Q_BRIDGE_NAME_STR);
            // Not failing plugin startup because of this.
            ret = SR_ERR_OK;
            continue;
        }

        ret = psfp_handle_config_change(session, reset_list, node,
                                        NULL, bridge_name, gate_id);
        if (SR_ERR_OK != ret) {
            SRP_LOG_WRN("%s() failed to handle bridge %s XPath %s", __func__,
                        bridge_name, node->xpath);
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

int get_sfsg_state_data(struct lyd_node **parent, char *bridge_name)
{
    struct port_hdl *port = NULL;
    struct psfp_flt_params params;
    struct psfp_gate_control_entry *ocl_entry = NULL;
    struct tsn_st_rational_number st_rational = {0u,0u};
    struct tsn_st_ptp_time st_time = {0u, 0u};
    uint32_t max_sfg = 0;
    uint32_t sfg_id = 0;
    uint32_t ret = 0;
    char tmp_value[MAX_STR_LEN] = "";
    char path[2*MAX_STR_LEN] = "";
    char filter_id[MAX_STR_LEN] = "";
    char gate_id[MAX_STR_LEN] = "";
    uint64_t tmp_ui64 = 0;
    uint32_t tmp_ui32 = 0;
    uint8_t tmp_ui8 = 0;
    int i = 0;
    bool tmp_bool = false;

    SRP_LOG_DBG(DEBUG_MSG_FUN_CALLED_WITH_ONE_PARAM, __func__, bridge_name);

    port = tsn_port_open_by_name(bridge_name);
    if (NULL == port) {
        SRP_LOG_ERR(ERROR_MSG_MOD_FUNC_EL_MSG, SFG_MODULE_NAME_STR, __func__,
                    bridge_name, ERR_PORT_NOT_OPEN_STR);
        return SR_ERR_OPERATION_FAILED;
    }

    /* container stream-filters, leaf max-stream-filter-instances */
    if (0 == tsn_psfp_get_max_filters(port, &tmp_ui32)) {
        snprintf(tmp_value, MAX_STR_LEN, "%u", tmp_ui32);
        fill_xpath(path, SFG_MAX_FILTERS_XPATH, bridge_name, bridge_name);
        if (NULL == lyd_new_path(*parent, NULL, path, tmp_value, 0, 0)) {
            SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                        SFG_MAX_FILTERS_STR, ERR_NOT_CREATED_ELEMENT_STR);
        }
    }

    /* container stream-gates, leaf max-stream-gate-instances */
    if (0 == tsn_psfp_get_max_gates(port, &tmp_ui32)) {
        snprintf(tmp_value, MAX_STR_LEN, "%u", tmp_ui32);
        fill_xpath(path, SFG_MAX_GATES_XPATH, bridge_name, bridge_name);
        if (NULL == lyd_new_path(*parent, NULL, path, tmp_value, 0, 0)) {
            SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                        SFG_MAX_GATES_STR, ERR_NOT_CREATED_ELEMENT_STR);
        }
    }

    /* get number of max filters for stream-filter-instance-table list */
    if (0 == tsn_psfp_get_max_filters(port, &max_sfg)) {
        for (sfg_id = 0; sfg_id < max_sfg; sfg_id++) {
            ret = tsn_psfp_flt_get_params(port, sfg_id, &params);
            if (TSN_ERRNO_ERROR_CODE(ret) == ENOENT)
                continue;

            snprintf(filter_id, MAX_STR_LEN, "%u", sfg_id);
            /* module ieee802-dot1q-psfp, augment of
             * stream-filter-instance-table list, leaf matching-frames-count
             */
            if (0 == tsn_psfp_flt_get_matching_frames_cnt(port, sfg_id, &tmp_ui32)) {
                snprintf(tmp_value, MAX_STR_LEN, "%u", tmp_ui32);
                fill_xpath(path, PSFP_MATCH_FRAMES_CNT_XPATH, bridge_name, bridge_name, filter_id);
                if (NULL == lyd_new_path(*parent, NULL, path, tmp_value, 0, 0)) {
                    SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                                PSFP_MATCH_FRAMES_CNT_XPATH,
                                ERR_NOT_CREATED_ELEMENT_STR);
                }
            }
            /* module ieee802-dot1q-psfp, augment of
             * stream-filter-instance-table list, leaf passing-frames-count
             */
            if (0 == tsn_psfp_flt_get_passing_frames_cnt(port, sfg_id, &tmp_ui32)) {
                snprintf(tmp_value, MAX_STR_LEN, "%u", tmp_ui32);
                fill_xpath(path, PSFP_PASS_FRAMES_CNT_XPATH, bridge_name, bridge_name, filter_id);
                if (NULL == lyd_new_path(*parent, NULL, path, tmp_value, 0, 0)) {
                    SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                                PSFP_PASS_FRAMES_CNT_XPATH,
                                ERR_NOT_CREATED_ELEMENT_STR);
                }
            }
            /* module ieee802-dot1q-psfp, augment of
             * stream-filter-instance-table list, leaf not-passing-frames-count
             */
            if (0 == tsn_psfp_flt_get_not_passing_frames_cnt(port, sfg_id, &tmp_ui32)) {
                snprintf(tmp_value, MAX_STR_LEN, "%u", tmp_ui32);
                fill_xpath(path, PSFP_NOT_PASS_FRAMES_CNT_XPATH, bridge_name, bridge_name, filter_id);
                if (NULL == lyd_new_path(*parent, NULL, path, tmp_value, 0, 0)) {
                    SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                                PSFP_NOT_PASS_FRAMES_CNT_XPATH,
                                ERR_NOT_CREATED_ELEMENT_STR);
                }
            }
            /* module ieee802-dot1q-psfp, augment of
             * stream-filter-instance-table list, leaf red-frames-count
             */
            if (0 == tsn_psfp_flt_get_red_frames_cnt(port, sfg_id, &tmp_ui32)) {
                snprintf(tmp_value, MAX_STR_LEN, "%u", tmp_ui32);
                fill_xpath(path, PSFP_RED_FRAMES_CNT_XPATH, bridge_name, bridge_name, filter_id);
                if (NULL == lyd_new_path(*parent, NULL, path, tmp_value, 0, 0)) {
                    SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                                PSFP_RED_FRAMES_CNT_XPATH,
                                ERR_NOT_CREATED_ELEMENT_STR);
                }
            }
            /* module ieee802-dot1q-psfp, augment of
             * stream-filter-instance-table list, leaf passing-sdu-count
             */
            if (0 == tsn_psfp_flt_get_passing_sdu_cnt(port, sfg_id, &tmp_ui32)) {
                snprintf(tmp_value, MAX_STR_LEN, "%u", tmp_ui32);
                fill_xpath(path, PSFP_PASS_SDU_FRAMES_CNT_XPATH, bridge_name, bridge_name, filter_id);
                if (NULL == lyd_new_path(*parent, NULL, path, tmp_value, 0, 0)) {
                    SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                                PSFP_PASS_SDU_FRAMES_CNT_XPATH,
                                ERR_NOT_CREATED_ELEMENT_STR);
                }
            }
            /* module ieee802-dot1q-psfp, augment of
             * stream-filter-instance-table list, leaf not-passing-sdu-count
             */
            if (0 == tsn_psfp_flt_get_not_passing_sdu_cnt(port, sfg_id, &tmp_ui32)) {
                snprintf(tmp_value, MAX_STR_LEN, "%u", tmp_ui32);
                fill_xpath(path, PSFP_NOT_PASS_SDU_FRAMES_CNT_XPATH, bridge_name, bridge_name, filter_id);
                if (NULL == lyd_new_path(*parent, NULL, path, tmp_value, 0, 0)) {
                    SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                                PSFP_NOT_PASS_SDU_FRAMES_CNT_XPATH,
                                ERR_NOT_CREATED_ELEMENT_STR);
                }
            }
        }
    }

    /* get number of max filters for stream-gate-instance-table list */
    if (0 == tsn_psfp_get_max_gates(port, &max_sfg)) {
        for (sfg_id = 0; sfg_id < max_sfg; sfg_id++) {
            snprintf(gate_id, MAX_STR_LEN, "%u", sfg_id);
            /* module ieee802-dot1q-psfp, augment of stream-gate-instance-table
             * list, leaf oper-gate-states
             */
            if (0 == tsn_psfp_gt_get_oper_gate_states(port, sfg_id, &tmp_ui8)) {
                snprintf(tmp_value, MAX_STR_LEN, "%u", tmp_ui8);
                fill_xpath(path, PSFP_OPER_GATE_STATES_XPATH, bridge_name, bridge_name, gate_id);
                if (NULL == lyd_new_path(*parent, NULL, path, tmp_value, 0, 0)) {
                    SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                                PSFP_OPER_GATE_STATES_XPATH,
                                ERR_NOT_CREATED_ELEMENT_STR);
                }
            }
            /* module ieee802-dot1q-psfp, augment of stream-gate-instance-table
             * list, leaf oper-ipv
             */
            if (0 == tsn_psfp_gt_get_oper_ipv(port, sfg_id, &tmp_ui8)) {
                snprintf(tmp_value, MAX_STR_LEN, "%u", tmp_ui8);
                fill_xpath(path, PSFP_OPER_IPV_XPATH, bridge_name, bridge_name, gate_id);
                if (NULL == lyd_new_path(*parent, NULL, path, tmp_value, 0, 0)) {
                    SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                                PSFP_OPER_IPV_XPATH,
                                ERR_NOT_CREATED_ELEMENT_STR);
                }
            }
            /* module ieee802-dot1q-psfp, augment of stream-gate-instance-table
             * list, container oper-control-list
             */
            if (0 == tsn_psfp_gt_get_oper_control_list_length(port, sfg_id, &tmp_ui32)) {
                /* container gate-parameters, list oper-control-list */
                if ((tmp_ui32 > 0) &&
                    (tmp_ui32 <= MAX_NUMBER_OF_ACL)) {
                    ocl_entry = (struct psfp_gate_control_entry*)malloc(sizeof(*ocl_entry) * tmp_ui32);
                    if (NULL == ocl_entry) {
                        SRP_LOG_INFMSG(ERR_MEMORY_ALLOC_FAILED_STR);
                        tsn_port_close(port);
                        return SR_ERR_OPERATION_FAILED;
                    }
                } else {
                    SRP_LOG_INF(ERROR_MSG_MOD_FUNC_EL_MSG, PSFP_MODULE_NAME_STR, __func__,
                                PSFP_OPER_CTRL_LIST_STR, ERR_NOT_CREATED_ELEMENT_STR);
                }

                if (NULL != ocl_entry) {
                    if (0 == tsn_psfp_gt_get_oper_control_list(port, sfg_id, tmp_ui32, ocl_entry)) {
                        for (i = 0; i < (int)tmp_ui32; i++) {
                            if (SR_ERR_OK != psfp_ocl_list_entry(parent, bridge_name,
                                                                 gate_id, i, ocl_entry)) {
                                SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                                            PSFP_OPER_CTRL_LIST_STR, ERR_NOT_CREATED_ELEMENT_STR);
                            }
                        }
                    } else {
                        SRP_LOG_INF(ERROR_MSG_MOD_FUNC_EL_MSG, PSFP_MODULE_NAME_STR, __func__,
                                    PSFP_OPER_CTRL_LIST_STR, ERR_NOT_CREATED_ELEMENT_STR);
                    }
                    free(ocl_entry);
                    ocl_entry = NULL;
                }
            }
            /* container oper-cycle-time */
            if (0 == tsn_psfp_gt_get_oper_cycle_time(port, sfg_id, &st_rational)) {
                /* container oper-cycle-time, leaf numerator */
                fill_xpath(path, PSFP_OCT_NUMERATOR_XPATH, bridge_name, bridge_name, gate_id);
                snprintf(tmp_value, MAX_STR_LEN, "%u", st_rational.numerator);
                if (NULL == lyd_new_path(*parent, NULL, path, tmp_value, 0, 0)) {
                    SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                                path, ERR_NOT_CREATED_ELEMENT_STR);
                }
                /* container oper-cycle-time, leaf denominator */
                fill_xpath(path, PSFP_OCT_DENOMINATOR_XPATH, bridge_name, bridge_name, gate_id);
                snprintf(tmp_value, MAX_STR_LEN, "%u", st_rational.denominator);
                if (NULL == lyd_new_path(*parent, NULL, path, tmp_value, 0, 0)) {
                    SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                                path, ERR_NOT_CREATED_ELEMENT_STR);
                }
            }
            /* leaf oper-cycle-time-extension */
            if (0 == tsn_psfp_gt_get_oper_cycle_time_extension(port, sfg_id, &tmp_ui32)) {
                fill_xpath(path, PSFP_OCT_EXTENSION_XPATH, bridge_name, bridge_name, gate_id);
                snprintf(tmp_value, MAX_STR_LEN, "%u", tmp_ui32);
                if (NULL == lyd_new_path(*parent, NULL, path, tmp_value, 0, 0)) {
                    SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                                path, ERR_NOT_CREATED_ELEMENT_STR);
                }
            }
            /* container oper-base-time */
            if (0 == tsn_psfp_gt_get_oper_base_time(port, sfg_id, &st_time)) {
                if (SR_ERR_OK != psfp_ptp_time_entry(parent, &st_time, bridge_name,
                                                     gate_id, PSFP_OPER_BASE_TIME_STR)) {
                    SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                                PSFP_OPER_BASE_TIME_STR, ERR_NOT_CREATED_ELEMENT_STR);
                }
            }
            /* container config-change-time */
            if (0 == tsn_psfp_gt_get_config_change_time(port, sfg_id, &st_time)) {
                if (SR_ERR_OK != psfp_ptp_time_entry(parent, &st_time, bridge_name,
                                                     gate_id, PSFP_CONFIG_CHANGE_STR_TIME_STR)) {
                    SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                                PSFP_CONFIG_CHANGE_STR_TIME_STR, ERR_NOT_CREATED_ELEMENT_STR);
                }
            }
            /* leaf tick-granularity */
            if (0 == tsn_psfp_gt_get_tick_granularity(port, sfg_id, &tmp_ui32)) {
                fill_xpath(path, PSFP_TICK_GRANULARITY_XPATH, bridge_name, bridge_name, gate_id);
                snprintf(tmp_value, MAX_STR_LEN, "%u", tmp_ui32);
                if (NULL == lyd_new_path(*parent, NULL, path, tmp_value, 0, 0)) {
                    SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                                path, ERR_NOT_CREATED_ELEMENT_STR);
                }
            }
            /* container current-time */
            if (0 == tsn_psfp_gt_get_current_time(port, sfg_id, &st_time)) {
                if (SR_ERR_OK != psfp_ptp_time_entry(parent, &st_time, bridge_name,
                                                     gate_id, PSFP_CURRENT_TIME_STR)) {
                    SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                                PSFP_CURRENT_TIME_STR, ERR_NOT_CREATED_ELEMENT_STR);
                }
            }
            /* leaf config-pending */
            if (0 == tsn_psfp_gt_get_config_pending(port, sfg_id, &tmp_bool)) {
                fill_xpath(path, PSFP_CONFIG_PENDING_XPATH, bridge_name, bridge_name, gate_id);
                snprintf(tmp_value, MAX_STR_LEN, "%s", tmp_bool ? BASE_TRUE : BASE_FALSE);
                if (NULL == lyd_new_path(*parent, NULL, path, tmp_value, 0, 0)) {
                    SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                                path, ERR_NOT_CREATED_ELEMENT_STR);
                }
            }
            /* leaf config-change-error */
            if (0 == tsn_psfp_gt_get_config_change_error(port, sfg_id, &tmp_ui64)) {
                fill_xpath(path, PSFP_CONFIG_CHANGE_ERROR_XPATH, bridge_name, bridge_name, gate_id);
                snprintf(tmp_value, MAX_STR_LEN, "%llu", tmp_ui64);
                if (NULL == lyd_new_path(*parent, NULL, path, tmp_value, 0, 0)) {
                    SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                                path, ERR_NOT_CREATED_ELEMENT_STR);
                }
            }
        }
    }
    tsn_port_close(port);
    return SR_ERR_OK;
}

struct lyd_node *new_flow_meters_container(struct ly_ctx *ctx,
                                           struct lyd_node *parent,
                                           char *bridge_name)
{
    struct lyd_node *flow_meters = NULL;
    struct port_hdl *port = NULL;
    const struct lys_module *module = NULL;
    uint32_t max_flow_meters = 0;
    char tmp_str[MAX_STR_LEN] = "";

    SRP_LOG_DBG(DEBUG_MSG_FUN_CALLED_WITH_ONE_PARAM, __func__, bridge_name);

    module = ly_ctx_load_module(ctx, PSFP_MODULE_NAME_STR, NULL);

    if (NULL == module) {
        SRP_LOG_ERR(ERROR_MSG_FUN_AND_MSG, __func__, ERR_MSG_LOAD_MODULE_STR);
        return NULL;
    }

    /* new stream-filters container */
    if (EXIT_FAILURE == new_container(parent, &flow_meters, module,
                                      PSFP_FLOW_METERS_STR)) {
        return NULL;
    }

    port = tsn_port_open_by_name(bridge_name);
    if (NULL == port) {
        SRP_LOG_ERR(ERROR_MSG_MOD_FUNC_EL_MSG, PSFP_MODULE_NAME_STR, __func__,
                    bridge_name, ERR_PORT_NOT_OPEN_STR);
        return NULL;
    }

    /* leaf max-flow-meter-instances */
    if (0 == tsn_psfp_get_max_flow_meters(port, &max_flow_meters)) {
        snprintf(tmp_str, MAX_STR_LEN, "%u", max_flow_meters);
        if (EXIT_FAILURE == new_node(flow_meters, module,
                                     PSFP_MAX_FLOW_METERS_STR, tmp_str)) {
            tsn_port_close(port);
            return NULL;
        }
    }
    tsn_port_close(port);

    return flow_meters;
}
