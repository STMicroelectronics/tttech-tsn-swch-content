/*
 * TTTech TSN Library (libtsn)
 * Copyright(c) 2017 TTTech Computertechnik AG.
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

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <inttypes.h>

#include "tsn_private.h"
#include "tsn_psfp.h"
#include "tsn_def.h"

#define IEEE8021PSFP_DIR "ieee8021PSFP/"

static const uint32_t tsn_module = TSN_PSFP;

static const sysfs_fn psfp_fn[MAX_TSN_PSFP_FUNCTIONS] = {
    [PSFP_MAX_FILTERS] = IEEE8021PSFP_DIR"tsnPsfpMaxStreamFilterInstances",
    [PSFP_MAX_GATES] = IEEE8021PSFP_DIR"tsnPsfpMaxStreamGateInstances",
    [PSFP_MAX_FLOW_METERS] = IEEE8021PSFP_DIR"tsnPsfpMaxFlowMeterInstances",
    [PSFP_SUPPORTED_LIST_MAX] = IEEE8021PSFP_DIR"tsnPsfpSupportedListMax",
    [PSFP_SUPPORTED_CYCLE_MAX] = IEEE8021PSFP_DIR"tsnPsfpSupportedCycleMax",
    [PSFP_SUPPORTED_INTERVAL_MAX] = IEEE8021PSFP_DIR"tsnPsfpSupportedIntervalMax",
    [PSFP_FLT_TABLE] = IEEE8021PSFP_DIR"tsnPsfpStreamFilterTable",
    [PSFP_FLT_DELETE] = IEEE8021PSFP_DIR"tsnPsfpStreamFilterDelete",
    [PSFP_FLT_MATCHING_FRAMES_CNT] = IEEE8021PSFP_DIR"tsnPsfpMatchingFramesCount",
    [PSFP_FLT_PASSING_FRAMES_CNT] = IEEE8021PSFP_DIR"tsnPsfpPassingFramesCount",
    [PSFP_FLT_NOT_PASSING_FRAMES_CNT] = IEEE8021PSFP_DIR"tsnPsfpNotPassingFramesCount",
    [PSFP_FLT_PASSING_SDU_CNT] = IEEE8021PSFP_DIR"tsnPsfpPassingSDUCount",
    [PSFP_FLT_NOT_PASSING_SDU_CNT] = IEEE8021PSFP_DIR"tsnPsfpNotPassingSDUCount",
    [PSFP_FLT_RED_FRAMES_CNT] = IEEE8021PSFP_DIR"tsnPsfpREDFramesCount",
    [PSFP_GT_GATE_ENABLED] = IEEE8021PSFP_DIR"PSFPGateEnabled",
    [PSFP_GT_ADMIN_GATE_STATES] = IEEE8021PSFP_DIR"PSFPAdminGateStates",
    [PSFP_GT_OPER_GATE_STATES] = IEEE8021PSFP_DIR"PSFPOperGateStates",
    [PSFP_GT_ADMIN_CONTROL_LIST_LENGTH] = IEEE8021PSFP_DIR"PSFPAdminControlListLength",
    [PSFP_GT_OPER_CONTROL_LIST_LENGTH] = IEEE8021PSFP_DIR"PSFPOperControlListLength",
    [PSFP_GT_ADMIN_CONTROL_LIST] = IEEE8021PSFP_DIR"PSFPAdminControlList",
    [PSFP_GT_OPER_CONTROL_LIST] = IEEE8021PSFP_DIR"PSFPOperControlList",
    [PSFP_GT_ADMIN_CYCLE_TIME] = IEEE8021PSFP_DIR"PSFPAdminCycleTime",
    [PSFP_GT_OPER_CYCLE_TIME] = IEEE8021PSFP_DIR"PSFPOperCycleTime",
    [PSFP_GT_ADMIN_CYCLE_TIME_EXTENSION] = IEEE8021PSFP_DIR"PSFPAdminCycleTimeExtension",
    [PSFP_GT_OPER_CYCLE_TIME_EXTENSION] = IEEE8021PSFP_DIR"PSFPOperCycleTimeExtension",
    [PSFP_GT_ADMIN_BASE_TIME] = IEEE8021PSFP_DIR"PSFPAdminBaseTime",
    [PSFP_GT_OPER_BASE_TIME] = IEEE8021PSFP_DIR"PSFPOperBaseTime",
    [PSFP_GT_CONFIG_CHANGE] = IEEE8021PSFP_DIR"PSFPConfigChange",
    [PSFP_GT_CONFIG_CHANGE_TIME] = IEEE8021PSFP_DIR"PSFPConfigChangeTime",
    [PSFP_GT_TICK_GRANULARITY] = IEEE8021PSFP_DIR"PSFPTickGranularity",
    [PSFP_GT_CURRENT_TIME   ] = IEEE8021PSFP_DIR"PSFPCurrentTime",
    [PSFP_GT_CONFIG_PENDING] = IEEE8021PSFP_DIR"PSFPConfigPending",
    [PSFP_GT_CONFIG_CHANGE_ERROR] = IEEE8021PSFP_DIR"PSFPConfigChangeError",
    [PSFP_GT_ADMIN_IPV] = IEEE8021PSFP_DIR"PSFPAdminIPV",
    [PSFP_GT_OPER_IPV] = IEEE8021PSFP_DIR"PSFPOperIPV",
    [PSFP_GT_CLOSED_INV_RX_ENABLE] = IEEE8021PSFP_DIR"PSFPGateClosedDueToInvalidRxEnable",
    [PSFP_GT_CLOSED_INV_RX] = IEEE8021PSFP_DIR"PSFPGateClosedDueToInvalidRx",
    [PSFP_HIT_GATE] = IEEE8021PSFP_DIR"_ExpectedGateThresholdGateSelect",
    [PSFP_HIT_TH_LOW] = IEEE8021PSFP_DIR"_ExpectedGateThresholdLow",
    [PSFP_HIT_TH_HIGH] = IEEE8021PSFP_DIR"_ExpectedGateThresholdHigh",
    [PSFP_HIT_LOW_CNT] = IEEE8021PSFP_DIR"_ExpectedGateThresholdLowViolationCount",
    [PSFP_HIT_HIGH_CNT] = IEEE8021PSFP_DIR"_ExpectedGateThresholdHighViolationCount",
};

tsn_return_value tsn_psfp_get_max_filters(struct port_hdl *brdev,
                                          uint32_t *max_filters)
{
    tsn_file_desc fd = { .id = PSFP_MAX_FILTERS };
    ASSERT_PARAMS(brdev && max_filters);
    ASSERT_FOPEN(fd, brdev, psfp_fn[fd.id], O_RDONLY);
    ASSERT_FSCAN(fd, "%"SCNu32, 1, max_filters);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_psfp_get_max_gates(struct port_hdl *brdev,
                                        uint32_t *max_gates)
{
    tsn_file_desc fd = { .id = PSFP_MAX_GATES };
    ASSERT_PARAMS(brdev && max_gates);
    ASSERT_FOPEN(fd, brdev, psfp_fn[fd.id], O_RDONLY);
    ASSERT_FSCAN(fd, "%"SCNu32, 1, max_gates);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_psfp_get_max_flow_meters(struct port_hdl *brdev,
                                        uint32_t *max_flow_meters)
{
    tsn_file_desc fd = { .id = PSFP_MAX_FLOW_METERS };
    ASSERT_PARAMS(brdev && max_flow_meters);
    ASSERT_FOPEN(fd, brdev, psfp_fn[fd.id], O_RDONLY);
    ASSERT_FSCAN(fd, "%"SCNu32, 1, max_flow_meters);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_psfp_get_supported_list_max(struct port_hdl *brdev,
                                                 uint32_t *max_sup)
{
    tsn_file_desc fd = { .id = PSFP_SUPPORTED_LIST_MAX };
    ASSERT_PARAMS(brdev && max_sup);
    ASSERT_FOPEN(fd, brdev, psfp_fn[fd.id], O_RDONLY);
    ASSERT_FSCAN(fd, "%"SCNu32, 1, max_sup);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_psfp_get_supported_cycle_max(struct port_hdl *brdev,
                                                  struct tsn_st_rational_number *max_cyc)
{
    tsn_file_desc fd = { .id = PSFP_SUPPORTED_CYCLE_MAX };
    ASSERT_PARAMS(brdev && max_cyc);
    ASSERT_FOPEN(fd, brdev, psfp_fn[fd.id], O_RDONLY);
    ASSERT_FSCAN(fd, "%"SCNu32 "/%"SCNu32, 2,
                 &max_cyc->numerator, &max_cyc->denominator);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_psfp_get_supported_interval_max(struct port_hdl *brdev,
                                                     uint32_t *max_int_ns)
{
    tsn_file_desc fd = { .id = PSFP_SUPPORTED_INTERVAL_MAX };
    ASSERT_PARAMS(brdev && max_int_ns);
    ASSERT_FOPEN(fd, brdev, psfp_fn[fd.id], O_RDONLY);
    ASSERT_FSCAN(fd, "%"SCNu32, 1, max_int_ns);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_psfp_flt_create(struct port_hdl *brdev, uint32_t flt_id,
                                     const struct psfp_flt_params *params)
{
    tsn_file_desc fd = { .id = PSFP_FLT_TABLE };
    ASSERT_PARAMS(brdev && params);
    ASSERT_FOPEN(fd, brdev, psfp_fn[fd.id], O_WRONLY);
    ASSERT_FWRENTRY(fd, *params, flt_id);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_psfp_flt_get_params(struct port_hdl *brdev,
                                         uint32_t flt_id,
                                         struct psfp_flt_params *params)
{
    tsn_file_desc fd = { .id = PSFP_FLT_TABLE };
    ASSERT_PARAMS(brdev && params);
    ASSERT_FOPEN(fd, brdev, psfp_fn[fd.id], O_RDONLY);
    ASSERT_FRDENTRY(fd, params, flt_id);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_psfp_flt_delete(struct port_hdl *brdev, uint32_t flt_id)
{
    tsn_file_desc fd = { .id = PSFP_FLT_DELETE };
    ASSERT_PARAMS(brdev);
    ASSERT_FOPEN(fd, brdev, psfp_fn[fd.id], O_WRONLY);
    ASSERT_FPRINT(fd, "%"PRIu32, flt_id);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_psfp_flt_get_matching_frames_cnt(struct port_hdl *brdev,
                                                      uint32_t flt_id,
                                                      uint32_t *cnt_val)
{
    tsn_file_desc fd = { .id = PSFP_FLT_MATCHING_FRAMES_CNT };
    ASSERT_PARAMS(brdev && cnt_val);
    ASSERT_FOPEN(fd, brdev, psfp_fn[fd.id], O_RDONLY);
    ASSERT_FRDENTRY(fd, cnt_val, flt_id);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_psfp_flt_get_passing_frames_cnt(struct port_hdl *brdev,
                                                     uint32_t flt_id,
                                                     uint32_t *cnt_val)
{
    tsn_file_desc fd = { .id = PSFP_FLT_PASSING_FRAMES_CNT };
    ASSERT_PARAMS(brdev && cnt_val);
    ASSERT_FOPEN(fd, brdev, psfp_fn[fd.id], O_RDONLY);
    ASSERT_FRDENTRY(fd, cnt_val, flt_id);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_psfp_flt_get_not_passing_frames_cnt(struct port_hdl *brdev,
                                                         uint32_t flt_id,
                                                         uint32_t *cnt_val)
{
    tsn_file_desc fd = { .id = PSFP_FLT_NOT_PASSING_FRAMES_CNT };
    ASSERT_PARAMS(brdev && cnt_val);
    ASSERT_FOPEN(fd, brdev, psfp_fn[fd.id], O_RDONLY);
    ASSERT_FRDENTRY(fd, cnt_val, flt_id);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_psfp_flt_get_passing_sdu_cnt(struct port_hdl *brdev,
                                                  uint32_t flt_id,
                                                  uint32_t *cnt_val)
{
    tsn_file_desc fd = { .id = PSFP_FLT_PASSING_SDU_CNT };
    ASSERT_PARAMS(brdev && cnt_val);
    ASSERT_FOPEN(fd, brdev, psfp_fn[fd.id], O_RDONLY);
    ASSERT_FRDENTRY(fd, cnt_val, flt_id);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_psfp_flt_get_not_passing_sdu_cnt(struct port_hdl *brdev,
                                                      uint32_t flt_id,
                                                      uint32_t *cnt_val)
{
    tsn_file_desc fd = { .id = PSFP_FLT_NOT_PASSING_SDU_CNT };
    ASSERT_PARAMS(brdev && cnt_val);
    ASSERT_FOPEN(fd, brdev, psfp_fn[fd.id], O_RDONLY);
    ASSERT_FRDENTRY(fd, cnt_val, flt_id);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_psfp_flt_get_red_frames_cnt(struct port_hdl *brdev,
                                                 uint32_t flt_id,
                                                 uint32_t *cnt_val)
{
    tsn_file_desc fd = { .id = PSFP_FLT_RED_FRAMES_CNT };
    ASSERT_PARAMS(brdev && cnt_val);
    ASSERT_FOPEN(fd, brdev, psfp_fn[fd.id], O_RDONLY);
    ASSERT_FRDENTRY(fd, cnt_val, flt_id);
    ASSERT_FCLOSE(fd);
}

static tsn_return_value tsn_psfp_gt_get_bool(int id,
                                             struct port_hdl *brdev,
                                             uint32_t gate,
                                             bool *value)
{
    tsn_file_desc fd = { .id = id };
    uint8_t tmp_value;
    ASSERT_PARAMS(brdev && value);
    ASSERT_FOPEN(fd, brdev, psfp_fn[fd.id], O_RDONLY);
    ASSERT_FRDENTRY(fd, &tmp_value, gate);
    *value = tmp_value;
    ASSERT_FCLOSE(fd);
}

static tsn_return_value tsn_psfp_gt_set_bool(int id,
                                             struct port_hdl *brdev,
                                             uint32_t gate,
                                             bool value)
{
    tsn_file_desc fd = { .id = id };
    uint8_t tmp_value = value;
    ASSERT_PARAMS(brdev);
    ASSERT_FOPEN(fd, brdev, psfp_fn[fd.id], O_WRONLY);
    ASSERT_FWRENTRY(fd, tmp_value, gate);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_psfp_gt_get_gate_enabled(struct port_hdl *brdev,
                                              uint32_t gate,
                                              bool *gate_enabled)
{
    return tsn_psfp_gt_get_bool(PSFP_GT_GATE_ENABLED, brdev, gate,
                                gate_enabled);
}

tsn_return_value tsn_psfp_gt_set_gate_enabled(struct port_hdl *brdev,
                                              uint32_t gate,
                                              bool gate_enabled)
{
    return tsn_psfp_gt_set_bool(PSFP_GT_GATE_ENABLED, brdev, gate,
                                gate_enabled);
}

tsn_return_value tsn_psfp_gt_get_admin_gate_states(struct port_hdl *brdev,
                                                   uint32_t gate,
                                                   uint8_t *admin_gate_state)
{
    tsn_file_desc fd = { .id = PSFP_GT_ADMIN_GATE_STATES };
    ASSERT_PARAMS(brdev && admin_gate_state);
    ASSERT_FOPEN(fd, brdev, psfp_fn[fd.id], O_RDONLY);
    ASSERT_FRDENTRY(fd, admin_gate_state, gate);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_psfp_gt_set_admin_gate_states(struct port_hdl *brdev,
                                                   uint32_t gate,
                                                   uint8_t admin_gate_state)
{
    tsn_file_desc fd = { .id = PSFP_GT_ADMIN_GATE_STATES };
    ASSERT_PARAMS(brdev);
    ASSERT_FOPEN(fd, brdev, psfp_fn[fd.id], O_WRONLY);
    ASSERT_FWRENTRY(fd, admin_gate_state, gate);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_psfp_gt_get_oper_gate_states(struct port_hdl *brdev,
                                                  uint32_t gate,
                                                  uint8_t *oper_gate_state)
{
    tsn_file_desc fd = { .id = PSFP_GT_OPER_GATE_STATES };
    ASSERT_PARAMS(brdev && oper_gate_state);
    ASSERT_FOPEN(fd, brdev, psfp_fn[fd.id], O_RDONLY);
    ASSERT_FRDENTRY(fd, oper_gate_state, gate);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_psfp_gt_get_admin_control_list_length(struct port_hdl *brdev,
                                                           uint32_t gate,
                                                           uint32_t *acl_length)
{
    tsn_file_desc fd = { .id = PSFP_GT_ADMIN_CONTROL_LIST_LENGTH };
    ASSERT_PARAMS(brdev && acl_length);
    ASSERT_FOPEN(fd, brdev, psfp_fn[fd.id], O_RDONLY);
    ASSERT_FRDENTRY(fd, acl_length, gate);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_psfp_gt_set_admin_control_list_length(struct port_hdl *brdev,
                                                           uint32_t gate,
                                                           uint32_t acl_length)
{
    tsn_file_desc fd = { .id = PSFP_GT_ADMIN_CONTROL_LIST_LENGTH };
    ASSERT_PARAMS(brdev);
    ASSERT_FOPEN(fd, brdev, psfp_fn[fd.id], O_WRONLY);
    ASSERT_FWRENTRY(fd, acl_length, gate);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_psfp_gt_get_oper_control_list_length(struct port_hdl *brdev,
                                                          uint32_t gate,
                                                          uint32_t *acl_length)
{
    tsn_file_desc fd = { .id = PSFP_GT_OPER_CONTROL_LIST_LENGTH };
    ASSERT_PARAMS(brdev && acl_length);
    ASSERT_FOPEN(fd, brdev, psfp_fn[fd.id], O_RDONLY);
    ASSERT_FRDENTRY(fd, acl_length, gate);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_psfp_gt_get_admin_control_list(struct port_hdl *brdev,
                                                    uint32_t gate,
                                                    uint32_t entries,
                                                    struct psfp_gate_control_entry *acl_entry)
{
    tsn_file_desc fd = { .id = PSFP_GT_ADMIN_CONTROL_LIST };
    uint32_t max_entries;
    ASSERT_PARAMS(brdev && acl_entry);
    tsn_return_value ret = tsn_psfp_get_supported_list_max(brdev, &max_entries);
    if (ret)
        return ret;
    ASSERT_FOPEN(fd, brdev, psfp_fn[fd.id], O_RDONLY);
    ASSERT_FRDSUBLIST(fd, acl_entry, max_entries, gate, entries);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_psfp_gt_set_admin_control_list(struct port_hdl *brdev,
                                                    uint32_t gate,
                                                    uint32_t entries,
                                                    const struct psfp_gate_control_entry *acl_entry)
{
    tsn_file_desc fd = { .id = PSFP_GT_ADMIN_CONTROL_LIST };
    uint32_t max_entries;
    ASSERT_PARAMS(brdev && acl_entry);
    tsn_return_value ret = tsn_psfp_get_supported_list_max(brdev, &max_entries);
    if (ret)
        return ret;
    ASSERT_FOPEN(fd, brdev, psfp_fn[fd.id], O_WRONLY);
    ASSERT_FWRSUBLIST(fd, acl_entry, max_entries, gate, entries);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_psfp_gt_get_oper_control_list(struct port_hdl *brdev,
                                                   uint32_t gate,
                                                   uint32_t entries,
                                                   struct psfp_gate_control_entry *acl_entry)
{
    tsn_file_desc fd = { .id = PSFP_GT_OPER_CONTROL_LIST };
    uint32_t max_entries;
    ASSERT_PARAMS(brdev && acl_entry);
    tsn_return_value ret = tsn_psfp_get_supported_list_max(brdev, &max_entries);
    if (ret)
        return ret;
    ASSERT_FOPEN(fd, brdev, psfp_fn[fd.id], O_RDONLY);
    ASSERT_FRDSUBLIST(fd, acl_entry, max_entries, gate, entries);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_psfp_gt_get_admin_cycle_time(struct port_hdl *brdev,
                                                  uint32_t gate,
                                                  struct tsn_st_rational_number *admin_ctime)
{
    tsn_file_desc fd = { .id = PSFP_GT_ADMIN_CYCLE_TIME };
    ASSERT_PARAMS(brdev && admin_ctime);
    ASSERT_FOPEN(fd, brdev, psfp_fn[fd.id], O_RDONLY);
    ASSERT_FRDENTRY(fd, admin_ctime, gate);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_psfp_gt_set_admin_cycle_time(struct port_hdl *brdev,
                                                  uint32_t gate,
                                                  const struct tsn_st_rational_number *admin_ctime)
{
    tsn_file_desc fd = { .id = PSFP_GT_ADMIN_CYCLE_TIME };
    ASSERT_PARAMS(brdev && admin_ctime);
    ASSERT_FOPEN(fd, brdev, psfp_fn[fd.id], O_WRONLY);
    ASSERT_FWRENTRY(fd, *admin_ctime, gate);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_psfp_gt_get_oper_cycle_time(struct port_hdl *brdev,
                                                  uint32_t gate,
                                                  struct tsn_st_rational_number *oper_ctime)
{
    tsn_file_desc fd = { .id = PSFP_GT_OPER_CYCLE_TIME };
    ASSERT_PARAMS(brdev && oper_ctime);
    ASSERT_FOPEN(fd, brdev, psfp_fn[fd.id], O_RDONLY);
    ASSERT_FRDENTRY(fd, oper_ctime, gate);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_psfp_gt_get_admin_cycle_time_extension(struct port_hdl *brdev,
                                                            uint32_t gate,
                                                            uint32_t *admin_ctime_ext)
{
    tsn_file_desc fd = { .id = PSFP_GT_ADMIN_CYCLE_TIME_EXTENSION };
    ASSERT_PARAMS(brdev && admin_ctime_ext);
    ASSERT_FOPEN(fd, brdev, psfp_fn[fd.id], O_RDONLY);
    ASSERT_FRDENTRY(fd, admin_ctime_ext, gate);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_psfp_gt_set_admin_cycle_time_extension(struct port_hdl *brdev,
                                                            uint32_t gate,
                                                            uint32_t admin_ctime_ext)
{
    tsn_file_desc fd = { .id = PSFP_GT_ADMIN_CYCLE_TIME_EXTENSION };
    ASSERT_PARAMS(brdev);
    ASSERT_FOPEN(fd, brdev, psfp_fn[fd.id], O_WRONLY);
    ASSERT_FWRENTRY(fd, admin_ctime_ext, gate);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_psfp_gt_get_oper_cycle_time_extension(struct port_hdl *brdev,
                                                           uint32_t gate,
                                                           uint32_t *oper_ctime_ext)
{
    tsn_file_desc fd = { .id = PSFP_GT_OPER_CYCLE_TIME_EXTENSION };
    ASSERT_PARAMS(brdev && oper_ctime_ext);
    ASSERT_FOPEN(fd, brdev, psfp_fn[fd.id], O_RDONLY);
    ASSERT_FRDENTRY(fd, oper_ctime_ext, gate);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_psfp_gt_get_admin_base_time(struct port_hdl *brdev,
                                                 uint32_t gate,
                                                 struct tsn_st_ptp_time *admin_btime)
{
    tsn_file_desc fd = { .id = PSFP_GT_ADMIN_BASE_TIME };
    ASSERT_PARAMS(brdev && admin_btime);
    ASSERT_FOPEN(fd, brdev, psfp_fn[fd.id], O_RDONLY);
    ASSERT_FRDENTRY(fd, admin_btime, gate);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_psfp_gt_set_admin_base_time(struct port_hdl *brdev,
                                                 uint32_t gate,
                                                 const struct tsn_st_ptp_time *admin_btime)
{
    tsn_file_desc fd = { .id = PSFP_GT_ADMIN_BASE_TIME };
    ASSERT_PARAMS(brdev && admin_btime);
    ASSERT_FOPEN(fd, brdev, psfp_fn[fd.id], O_WRONLY);
    ASSERT_FWRENTRY(fd, *admin_btime, gate);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_psfp_gt_get_oper_base_time(struct port_hdl *brdev,
                                                uint32_t gate,
                                                struct tsn_st_ptp_time *oper_btime)
{
    tsn_file_desc fd = { .id = PSFP_GT_OPER_BASE_TIME };
    ASSERT_PARAMS(brdev && oper_btime);
    ASSERT_FOPEN(fd, brdev, psfp_fn[fd.id], O_RDONLY);
    ASSERT_FRDENTRY(fd, oper_btime, gate);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_psfp_gt_get_config_change(struct port_hdl *brdev,
                                               uint32_t gate,
                                               bool *config_change)
{
    return tsn_psfp_gt_get_bool(PSFP_GT_CONFIG_CHANGE, brdev, gate,
                                config_change);
}

tsn_return_value tsn_psfp_gt_set_config_change(struct port_hdl *brdev,
                                               uint32_t gate,
                                               bool config_change)
{
    return tsn_psfp_gt_set_bool(PSFP_GT_CONFIG_CHANGE, brdev, gate,
                                config_change);
}

tsn_return_value tsn_psfp_gt_get_config_change_time(struct port_hdl *brdev,
                                                    uint32_t gate,
                                                    struct tsn_st_ptp_time *config_change_time)
{
    tsn_file_desc fd = { .id = PSFP_GT_CONFIG_CHANGE_TIME };
    ASSERT_PARAMS(brdev && config_change_time);
    ASSERT_FOPEN(fd, brdev, psfp_fn[fd.id], O_RDONLY);
    ASSERT_FRDENTRY(fd, config_change_time, gate);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_psfp_gt_set_config_change_time(struct port_hdl *brdev,
                                                    uint32_t gate,
                                                    const struct tsn_st_ptp_time *config_change_time)
{
    tsn_file_desc fd = { .id = PSFP_GT_CONFIG_CHANGE_TIME };
    ASSERT_PARAMS(brdev && config_change_time);
    ASSERT_FOPEN(fd, brdev, psfp_fn[fd.id], O_WRONLY);
    ASSERT_FWRENTRY(fd, *config_change_time, gate);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_psfp_gt_get_tick_granularity(struct port_hdl *brdev,
                                                  uint32_t gate,
                                                  uint32_t *t_granularity)
{
    tsn_file_desc fd = { .id = PSFP_GT_TICK_GRANULARITY };
    ASSERT_PARAMS(brdev && t_granularity);
    ASSERT_FOPEN(fd, brdev, psfp_fn[fd.id], O_RDONLY);
    ASSERT_FRDENTRY(fd, t_granularity, gate);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_psfp_gt_get_current_time(struct port_hdl *brdev,
                                              uint32_t gate,
                                              struct tsn_st_ptp_time *current_time)
{
    tsn_file_desc fd = { .id = PSFP_GT_CURRENT_TIME };
    ASSERT_PARAMS(brdev && current_time);
    ASSERT_FOPEN(fd, brdev, psfp_fn[fd.id], O_RDONLY);
    ASSERT_FRDENTRY(fd, current_time, gate);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_psfp_gt_get_config_pending(struct port_hdl *brdev,
                                                uint32_t gate,
                                                bool *conf_pending)
{
    return tsn_psfp_gt_get_bool(PSFP_GT_CONFIG_PENDING, brdev, gate,
                                conf_pending);
}

tsn_return_value tsn_psfp_gt_get_config_change_error(struct port_hdl *brdev,
                                                     uint32_t gate,
                                                     uint64_t *conf_chg_error)
{
    tsn_file_desc fd = { .id = PSFP_GT_CONFIG_CHANGE_ERROR };
    ASSERT_PARAMS(brdev && conf_chg_error);
    ASSERT_FOPEN(fd, brdev, psfp_fn[fd.id], O_RDONLY);
    ASSERT_FRDENTRY(fd, conf_chg_error, gate);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_psfp_gt_get_admin_ipv(struct port_hdl *brdev,
                                           uint32_t gate,
                                           int8_t *admin_ipv)
{
    tsn_file_desc fd = { .id = PSFP_GT_ADMIN_IPV };
    ASSERT_PARAMS(brdev && admin_ipv);
    ASSERT_FOPEN(fd, brdev, psfp_fn[fd.id], O_RDONLY);
    ASSERT_FRDENTRY(fd, admin_ipv, gate);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_psfp_gt_set_admin_ipv(struct port_hdl *brdev,
                                           uint32_t gate,
                                           int8_t admin_ipv)
{
    tsn_file_desc fd = { .id = PSFP_GT_ADMIN_IPV };
    ASSERT_PARAMS(brdev);
    ASSERT_FOPEN(fd, brdev, psfp_fn[fd.id], O_WRONLY);
    ASSERT_FWRENTRY(fd, admin_ipv, gate);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_psfp_gt_get_oper_ipv(struct port_hdl *brdev,
                                          uint32_t gate,
                                          uint8_t *oper_ipv)
{
    tsn_file_desc fd = { .id = PSFP_GT_OPER_IPV };
    ASSERT_PARAMS(brdev && oper_ipv);
    ASSERT_FOPEN(fd, brdev, psfp_fn[fd.id], O_RDONLY);
    ASSERT_FRDENTRY(fd, oper_ipv, gate);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_psfp_gt_get_closed_inv_rx_enable(struct port_hdl *brdev,
                                                      uint32_t gate,
                                                      bool *gate_closed_ena)
{
    return tsn_psfp_gt_get_bool(PSFP_GT_CLOSED_INV_RX_ENABLE, brdev, gate,
                                gate_closed_ena);
}

tsn_return_value tsn_psfp_gt_set_closed_inv_rx_enable(struct port_hdl *brdev,
                                                      uint32_t gate,
                                                      bool gate_closed_ena)
{
    return tsn_psfp_gt_set_bool(PSFP_GT_CLOSED_INV_RX_ENABLE, brdev, gate,
                                gate_closed_ena);
}

tsn_return_value tsn_psfp_gt_get_closed_inv_rx(struct port_hdl *brdev,
                                               uint32_t gate,
                                               bool *gate_closed)
{
    return tsn_psfp_gt_get_bool(PSFP_GT_CLOSED_INV_RX, brdev, gate,
                                gate_closed);
}

tsn_return_value tsn_psfp_gt_set_closed_inv_rx(struct port_hdl *brdev,
                                               uint32_t gate,
                                               bool gate_closed)
{
    return tsn_psfp_gt_set_bool(PSFP_GT_CLOSED_INV_RX, brdev, gate,
                                gate_closed);
}

tsn_return_value tsn_psfp_hit_set_gate(struct port_hdl *brdev,
                                       unsigned int gate)
{
    tsn_file_desc fd = { .id = PSFP_HIT_GATE };
    ASSERT_PARAMS(brdev);
    ASSERT_FOPEN(fd, brdev, psfp_fn[fd.id], O_WRONLY);
    ASSERT_FPRINT(fd, "%u", gate);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_psfp_hit_get_gate(struct port_hdl *brdev,
                                       unsigned int *gate)
{
    tsn_file_desc fd = { .id = PSFP_HIT_GATE };
    ASSERT_PARAMS(brdev && gate);
    ASSERT_FOPEN(fd, brdev, psfp_fn[fd.id], O_RDONLY);
    ASSERT_FSCAN(fd, "%u", 1, gate);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_psfp_hit_set_low_th(struct port_hdl *brdev,
                                         unsigned int th_ns)
{
    tsn_file_desc fd = { .id = PSFP_HIT_TH_LOW };
    ASSERT_PARAMS(brdev);
    ASSERT_FOPEN(fd, brdev, psfp_fn[fd.id], O_WRONLY);
    ASSERT_FPRINT(fd, "%u", th_ns);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_psfp_hit_get_low_th(struct port_hdl *brdev,
                                         unsigned int *th_ns)
{
    tsn_file_desc fd = { .id = PSFP_HIT_TH_LOW };
    ASSERT_PARAMS(brdev && th_ns);
    ASSERT_FOPEN(fd, brdev, psfp_fn[fd.id], O_RDONLY);
    ASSERT_FSCAN(fd, "%u", 1, th_ns);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_psfp_hit_set_high_th(struct port_hdl *brdev,
                                         unsigned int th_ns)
{
    tsn_file_desc fd = { .id = PSFP_HIT_TH_HIGH };
    ASSERT_PARAMS(brdev);
    ASSERT_FOPEN(fd, brdev, psfp_fn[fd.id], O_WRONLY);
    ASSERT_FPRINT(fd, "%u", th_ns);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_psfp_hit_get_high_th(struct port_hdl *brdev,
                                          unsigned int *th_ns)
{
    tsn_file_desc fd = { .id = PSFP_HIT_TH_HIGH };
    ASSERT_PARAMS(brdev && th_ns);
    ASSERT_FOPEN(fd, brdev, psfp_fn[fd.id], O_RDONLY);
    ASSERT_FSCAN(fd, "%u", 1, th_ns);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_psfp_hit_get_low_cnt(struct port_hdl *brdev,
                                          uint32_t str_hdl,
                                          uint32_t *cnt_val)
{
    tsn_file_desc fd = { .id = PSFP_HIT_LOW_CNT };
    ASSERT_PARAMS(brdev && cnt_val);
    ASSERT_FOPEN(fd, brdev, psfp_fn[fd.id], O_RDONLY);
    ASSERT_FRDENTRY(fd, cnt_val, str_hdl);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_psfp_hit_get_high_cnt(struct port_hdl *brdev,
                                           uint32_t str_hdl,
                                           uint32_t *cnt_val)
{
    tsn_file_desc fd = { .id = PSFP_HIT_HIGH_CNT };
    ASSERT_PARAMS(brdev && cnt_val);
    ASSERT_FOPEN(fd, brdev, psfp_fn[fd.id], O_RDONLY);
    ASSERT_FRDENTRY(fd, cnt_val, str_hdl);
    ASSERT_FCLOSE(fd);
}

const char *tsn_psfp_get_known_ernno_msg(tsn_return_value return_code)
{
    (void) return_code;
    return read_psfp_error_msg(return_code);
}

static uint32_t get_return_header(tsn_file_desc *fd)
{
    return ((fd->id < MAX_TSN_PSFP_FUNCTIONS) ?
            (tsn_module<<24)|((fd->id)<<16) : E_TSN_LIB_NO_ERROR);
}
