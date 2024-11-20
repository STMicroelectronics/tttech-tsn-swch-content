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

#ifndef TSN_PSFP_H_
#define TSN_PSFP_H_

#include "tsn_def.h"

/**
 * @ingroup tsn_psfp_api
 * @brief Get the number of maximum supported stream filter instances
 *
 * @param[in]    brdev          Bridge handler
 * @param[out]   max_filters    Retrieved number of supported filters
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR   See tsn_return_value
 *
 **/
tsn_return_value tsn_psfp_get_max_filters(struct port_hdl *brdev,
                                          uint32_t *max_filters);

/**
 * @ingroup tsn_psfp_api
 * @brief Get the number of maximum supported stream gate instances
 *
 * @param[in]    brdev          Bridge handler
 * @param[out]   max_gates      Retrieved number of supported gates
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR   See tsn_return_value
 *
 **/
tsn_return_value tsn_psfp_get_max_gates(struct port_hdl *brdev,
                                        uint32_t *max_gates);

/**
 * @ingroup tsn_psfp_api
 * @brief Get the number of maximum supported flow meter instances
 *
 * @param[in]    brdev              Bridge handler
 * @param[out]   max_flow_meters    Retrieved number of supported flow meters
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR       See tsn_return_value
 *
 **/
tsn_return_value tsn_psfp_get_max_flow_meters(struct port_hdl *brdev,
                                              uint32_t *max_flow_meters);

/**
 * @ingroup tsn_psfp_api
 * @brief Get the number of maximum supported rows in schedule table
 *
 * @param[in]    brdev          Bridge handler
 * @param[out]   max_sup        Retrieved number of supported rows in schedule
 *                              table
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR   See tsn_return_value
 *
 **/
tsn_return_value tsn_psfp_get_supported_list_max(struct port_hdl *brdev,
                                                 uint32_t *max_sup);

/**
 * @ingroup tsn_psfp_api
 * @brief Get the maximum supported cycle time
 *
 * @param[in]    brdev          Bridge handler
 * @param[out]   max_cyc        Retrieved maximum supported cycle time
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR   See tsn_return_value
 *
 **/
tsn_return_value tsn_psfp_get_supported_cycle_max(struct port_hdl *brdev,
                                                  struct tsn_st_rational_number *max_cyc);

/**
 * @ingroup tsn_psfp_api
 * @brief Get the maximum supported interval length for rows in schedule table
 *
 * @param[in]    brdev          Bridge handler
 * @param[out]   max_int_ns     Retrieved maximum supported interval length
 *                              for rows in schedule table in nanoseconds
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR   See tsn_return_value
 *
 **/
tsn_return_value tsn_psfp_get_supported_interval_max(struct port_hdl *brdev,
                                                     uint32_t *max_int_ns);

/**
 * @ingroup tsn_psfp_api
 * @brief Create a new entry in stream filter instance table
 *
 * @param[in]    brdev          Bridge handler
 * @param[in]    flt_id         Filter instance number
 * @param[in]    params         Parameters that define the new filter.
 *                              Parameters are defined in IEEE 802.1Qci 12.31.2.
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR   See tsn_return_value
 *
 **/
tsn_return_value tsn_psfp_flt_create(struct port_hdl *brdev, uint32_t flt_id,
                                     const struct psfp_flt_params *params);

/**
 * @ingroup tsn_psfp_api
 * @brief Get stream filter parameters of filter instance table entry
 *
 * @param[in]    brdev          Bridge handler
 * @param[in]    flt_id         Filter instance number
 * @param[out]   params         Retrieved parameters of the filter instance
 *                              entry. Parameters are defined in
 *                              IEEE 802.1Qci 12.31.2.
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR   See tsn_return_value
 *
 **/
tsn_return_value tsn_psfp_flt_get_params(struct port_hdl *brdev,
                                         uint32_t flt_id,
                                         struct psfp_flt_params *params);

/**
 * @ingroup tsn_psfp_api
 * @brief Delete entry of stream filter instance table
 *
 * @param[in]    brdev          Bridge handler
 * @param[in]    flt_id         Filter instance number
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR   See tsn_return_value
 *
 **/
tsn_return_value tsn_psfp_flt_delete(struct port_hdl *brdev, uint32_t flt_id);

/**
 * @ingroup tsn_psfp_api
 * @brief Get the counter value of received frames matching filter
 *
 * @param[in]    brdev          Bridge handler
 * @param[in]    flt_id         Filter instance number
 * @param[out]   cnt_val        Pointer to retrieved counter value
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR   See tsn_return_value
 *
 **/
tsn_return_value tsn_psfp_flt_get_matching_frames_cnt(struct port_hdl *brdev,
                                                      uint32_t flt_id,
                                                      uint32_t *cnt_val);

/**
 * @ingroup tsn_psfp_api
 * @brief Get the counter value of received frames passing associated gate
 *
 * @param[in]    brdev          Bridge handler
 * @param[in]    flt_id         Filter instance number
 * @param[out]   cnt_val        Pointer to retrieved counter value
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR   See tsn_return_value
 *
 **/
tsn_return_value tsn_psfp_flt_get_passing_frames_cnt(struct port_hdl *brdev,
                                                     uint32_t flt_id,
                                                     uint32_t *cnt_val);

/**
 * @ingroup tsn_psfp_api
 * @brief Get the counter value of received frames not passing associated gate
 *
 * @param[in]    brdev          Bridge handler
 * @param[in]    flt_id         Filter instance number
 * @param[out]   cnt_val        Pointer to retrieved counter value
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR   See tsn_return_value
 *
 **/
tsn_return_value tsn_psfp_flt_get_not_passing_frames_cnt(struct port_hdl *brdev,
                                                         uint32_t flt_id,
                                                         uint32_t *cnt_val);

/**
 * @ingroup tsn_psfp_api
 * @brief Get the counter value of received frames passing SDU size filter
 *
 * @param[in]    brdev          Bridge handler
 * @param[in]    flt_id         Filter instance number
 * @param[out]   cnt_val        Pointer to retrieved counter value
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR   See tsn_return_value
 *
 **/
tsn_return_value tsn_psfp_flt_get_passing_sdu_cnt(struct port_hdl *brdev,
                                                  uint32_t flt_id,
                                                  uint32_t *cnt_val);

/**
 * @ingroup tsn_psfp_api
 * @brief Get the counter value of received frames not passing SDU size filter
 *
 * @param[in]    brdev          Bridge handler
 * @param[in]    flt_id         Filter instance number
 * @param[out]   cnt_val        Pointer to retrieved counter value
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR   See tsn_return_value
 *
 **/
tsn_return_value tsn_psfp_flt_get_not_passing_sdu_cnt(struct port_hdl *brdev,
                                                      uint32_t flt_id,
                                                      uint32_t *cnt_val);

/**
 * @ingroup tsn_psfp_api
 * @brief Get the counter value of received RED frames associated with filter
 *
 * @param[in]    brdev          Bridge handler
 * @param[in]    flt_id         Filter instance number
 * @param[out]   cnt_val        Pointer to retrieved counter value
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR   See tsn_return_value
 *
 **/
tsn_return_value tsn_psfp_flt_get_red_frames_cnt(struct port_hdl *brdev,
                                                 uint32_t flt_id,
                                                 uint32_t *cnt_val);

/**
 * @ingroup tsn_psfp_api
 * @brief Get the gate enabled value
 *
 * @param[in]    brdev          Bridge handler
 * @param[in]    gate           Gate instance
 * @param[out]   gate_enabled   Retrieved Gate Enabled value
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR   See tsn_return_value
 *
 **/
tsn_return_value tsn_psfp_gt_get_gate_enabled(struct port_hdl *brdev,
                                              uint32_t gate,
                                              bool *gate_enabled);

/**
 * @ingroup tsn_psfp_api
 * @brief Modify the gate enabled value
 *
 * @param[in]    brdev          Bridge handler
 * @param[in]    gate           Gate instance
 * @param[in]    gate_enabled   Gate Enabled value to be set
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR   See tsn_return_value
 *
 **/
tsn_return_value tsn_psfp_gt_set_gate_enabled(struct port_hdl *brdev,
                                              uint32_t gate,
                                              bool gate_enabled);

/**
 * @ingroup tsn_psfp_api
 * @brief Get the admin gate state value
 *
 * @param[in]    brdev              Bridge handler
 * @param[in]    gate               Gate instance
 * @param[out]   admin_gate_state   Retrieved Admin Gate State value,
 *                                  bit 0 is set (1) when gate is open and
 *                                  reset (0) when gate is closed.
 *
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR       See tsn_return_value
 *
 **/
tsn_return_value tsn_psfp_gt_get_admin_gate_states(struct port_hdl *brdev,
                                                   uint32_t gate,
                                                   uint8_t *admin_gate_state);

/**
 * @ingroup tsn_psfp_api
 * @brief Modify the admin gate state value
 *
 * @param[in]    brdev              Bridge handler
 * @param[in]    gate               Gate instance
 * @param[in]    admin_gate_state   Admin Gate State value to be set,
 *                                  set bit 0 to open the gate and
 *                                  reset it to close to gate.
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR       See tsn_return_value
 *
 **/
tsn_return_value tsn_psfp_gt_set_admin_gate_states(struct port_hdl *brdev,
                                                   uint32_t gate,
                                                   uint8_t admin_gate_state);

/**
 * @ingroup tsn_psfp_api
 * @brief Get the oper gate state value
 *
 * @param[in]    brdev              Bridge handler
 * @param[in]    gate               Gate instance
 * @param[out]   oper_gate_state    Retrieved Oper Gate State value,
 *                                  bit 0 is set (1) when gate is open and
 *                                  reset (0) when gate is closed.
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR       See tsn_return_value
 *
 **/
tsn_return_value tsn_psfp_gt_get_oper_gate_states(struct port_hdl *brdev,
                                                  uint32_t gate,
                                                  uint8_t *oper_gate_state);

/**
 * @ingroup tsn_psfp_api
 * @brief Get the admin control list length value
 *
 * @param[in]    brdev          Bridge handler
 * @param[in]    gate           Gate instance
 * @param[out]   acl_length     Retrieved Admin Control List Length value
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR   See tsn_return_value
 *
 **/
tsn_return_value tsn_psfp_gt_get_admin_control_list_length(struct port_hdl *brdev,
                                                           uint32_t gate,
                                                           uint32_t *acl_length);

/**
 * @ingroup tsn_psfp_api
 * @brief Modify the admin control list length value
 *
 * @param[in]    brdev          Bridge handler
 * @param[in]    gate           Gate instance
 * @param[in]    acl_length     Admin Control List Length value to be set
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR   See tsn_return_value
 *
 **/
tsn_return_value tsn_psfp_gt_set_admin_control_list_length(struct port_hdl *brdev,
                                                           uint32_t gate,
                                                           uint32_t acl_length);

/**
 * @ingroup tsn_psfp_api
 * @brief Get the oper control list length value
 *
 * @param[in]    brdev          Bridge handler
 * @param[in]    gate           Gate instance
 * @param[out]   ocl_length     Retrieved Oper Control List Length value
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR   See tsn_return_value
 *
 **/
tsn_return_value tsn_psfp_gt_get_oper_control_list_length(struct port_hdl *brdev,
                                                          uint32_t gate,
                                                          uint32_t *ocl_length);

/**
 * @ingroup tsn_psfp_api
 * @brief Get an admin control list entry value
 *
 * @param[in]    brdev          Bridge handler
 * @param[in]    gate           Gate instance
 * @param[in]    entries        Maximum number of Control List Entries to be read.
 * @param[out]   acl_entry      Admin Control List
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR   See tsn_return_value
 *
 **/
tsn_return_value tsn_psfp_gt_get_admin_control_list(struct port_hdl *brdev,
                                                    uint32_t gate,
                                                    uint32_t entries,
                                                    struct psfp_gate_control_entry *acl_entry);

/**
 * @ingroup tsn_psfp_api
 * @brief Modify an admin control list entry value
 *
 * @param[in]    brdev          Bridge handler
 * @param[in]    gate           Gate instance
 * @param[in]    entries        Number of elements in given Admin Control List
 * @param[in]    acl_entry      Admin Control List
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR   See tsn_return_value
 *
 **/
tsn_return_value tsn_psfp_gt_set_admin_control_list(struct port_hdl *brdev,
                                                    uint32_t gate,
                                                    uint32_t entries,
                                                    const struct psfp_gate_control_entry *acl_entry);

/**
 * @ingroup tsn_psfp_api
 * @brief Get an oper control list entry value
 *
 * @param[in]    brdev          Bridge handler
 * @param[in]    gate           Gate instance
 * @param[in]    entries        Maximum number of Control List Entries to be read.
 * @param[out]   ocl_entry      Oper Control List
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR   See tsn_return_value
 *
 **/
tsn_return_value tsn_psfp_gt_get_oper_control_list(struct port_hdl *brdev,
                                                   uint32_t gate,
                                                   uint32_t entries,
                                                   struct psfp_gate_control_entry *ocl_entry);

/**
 * @ingroup tsn_psfp_api
 * @brief Get the admin cycle time value
 *
 * @param[in]    brdev          Bridge handler
 * @param[in]    gate           Gate instance
 * @param[out]   admin_ctime    Retrieved Admin Cycle Time value
 *
 * @return int                  Linux system error code (errno)
 * @retval 0                    Admin Cycle Time value successfully retrieved
 *
 **/
tsn_return_value tsn_psfp_gt_get_admin_cycle_time(struct port_hdl *brdev,
                                                  uint32_t gate,
                                                  struct tsn_st_rational_number *admin_ctime);

/**
 * @ingroup tsn_psfp_api
 * @brief Modify the admin cycle time value
 *
 * @param[in]    brdev          Bridge handler
 * @param[in]    gate           Gate instance
 * @param[in]    admin_ctime    Admin Cycle Time value to be set
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR   See tsn_return_value
 *
 **/
tsn_return_value tsn_psfp_gt_set_admin_cycle_time(struct port_hdl *brdev,
                                                  uint32_t gate,
                                                  const struct tsn_st_rational_number *admin_ctime);

/**
 * @ingroup tsn_psfp_api
 * @brief Get the oper cycle time value
 *
 * @param[in]    brdev          Bridge handler
 * @param[in]    gate           Gate instance
 * @param[out]   oper_ctime     Retrieved Oper Cycle Time value
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR   See tsn_return_value
 *
 **/
tsn_return_value tsn_psfp_gt_get_oper_cycle_time(struct port_hdl *brdev,
                                                 uint32_t gate,
                                                 struct tsn_st_rational_number *oper_ctime);

/**
 * @ingroup tsn_psfp_api
 * @brief Get the admin cycle time extension value
 *
 * @param[in]    brdev              Bridge handler
 * @param[in]    gate               Gate instance
 * @param[out]   admin_ctime_ext    Retrieved Admin Cycle Time Extension value
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR       See tsn_return_value
 *
 **/
tsn_return_value tsn_psfp_gt_get_admin_cycle_time_extension(struct port_hdl *brdev,
                                                            uint32_t gate,
                                                            uint32_t *admin_ctime_ext);

/**
 * @ingroup tsn_psfp_api
 * @brief Modify the admin cycle time extension value
 *
 * @param[in]    brdev              Bridge handler
 * @param[in]    gate               Gate instance
 * @param[in]    admin_ctime_ext    Admin Cycle Time Extension value to be set
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR       See tsn_return_value
 *
 **/
tsn_return_value tsn_psfp_gt_set_admin_cycle_time_extension(struct port_hdl *brdev,
                                                            uint32_t gate,
                                                            uint32_t admin_ctime_ext);

/**
 * @ingroup tsn_psfp_api
 * @brief Get the oper cycle time extension value
 *
 * @param[in]    brdev              Bridge handler
 * @param[in]    gate               Gate instance
 * @param[out]   oper_ctime_ext     Retrieved Oper Cycle Time Extension value
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR       See tsn_return_value
 *
 **/
tsn_return_value tsn_psfp_gt_get_oper_cycle_time_extension(struct port_hdl *brdev,
                                                           uint32_t gate,
                                                           uint32_t *oper_ctime_ext);

/**
 * @ingroup tsn_psfp_api
 * @brief Get the admin base time value
 *
 * @param[in]    brdev          Bridge handler
 * @param[in]    gate           Gate instance
 * @param[out]   admin_btime    Retrieved Admin Base Time value
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR   See tsn_return_value
 *
 **/
tsn_return_value tsn_psfp_gt_get_admin_base_time(struct port_hdl *brdev,
                                                 uint32_t gate,
                                                 struct tsn_st_ptp_time *admin_btime);

/**
 * @ingroup tsn_psfp_api
 * @brief Modify the admin base time value
 *
 * @param[in]    brdev          Bridge handler
 * @param[in]    gate           Gate instance
 * @param[in]    admin_btime    Admin Base Time value to be set
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR   See tsn_return_value
 *
 **/
tsn_return_value tsn_psfp_gt_set_admin_base_time(struct port_hdl *brdev,
                                                 uint32_t gate,
                                                 const struct tsn_st_ptp_time *admin_btime);

/**
 * @ingroup tsn_psfp_api
 * @brief Get the oper base time value
 *
 * @param[in]    brdev          Bridge handler
 * @param[in]    gate           Gate instance
 * @param[out]   oper_btime     Retrieved Oper Base Time value
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR   See tsn_return_value
 *
 **/
tsn_return_value tsn_psfp_gt_get_oper_base_time(struct port_hdl *brdev,
                                                uint32_t gate,
                                                struct tsn_st_ptp_time *oper_btime);

/**
 * @ingroup tsn_psfp_api
 * @brief Get the config change value
 *
 * @param[in]    brdev          Bridge handler
 * @param[in]    gate           Gate instance
 * @param[out]   config_change  Retrieved Config Change value
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR   See tsn_return_value
 *
 **/
tsn_return_value tsn_psfp_gt_get_config_change(struct port_hdl *brdev,
                                               uint32_t gate,
                                               bool *config_change);

/**
 * @ingroup tsn_psfp_api
 * @brief Modify the config change value
 *
 * @param[in]    brdev          Bridge handler
 * @param[in]    gate           Gate instance
 * @param[in]    config_change  Config Change value to be set
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR   See tsn_return_value
 *
 **/
tsn_return_value tsn_psfp_gt_set_config_change(struct port_hdl *brdev,
                                               uint32_t gate,
                                               bool config_change);

/**
 * @ingroup tsn_psfp_api
 * @brief Get the actual config change time, i.e., the time at which the
 *        administrative variables are copied across to the corresponding
 *        operational variables.
 *
 * @param[in]    brdev              Bridge handler
 * @param[in]    gate               Gate instance
 * @param[out]   config_change_time Retrieved Config Change Time value
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR       See tsn_return_value
 *
 **/
tsn_return_value tsn_psfp_gt_get_config_change_time(struct port_hdl *brdev,
                                                    uint32_t gate,
                                                    struct tsn_st_ptp_time *config_change_time);

/**
 * @ingroup tsn_psfp_api
 * @brief Get the tick granularity value
 *
 * @param[in]    brdev          Bridge handler
 * @param[in]    gate           Gate instance
 * @param[out]   t_granularity  Retrieved Tick Granularity value
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR   See tsn_return_value
 *
 **/
tsn_return_value tsn_psfp_gt_get_tick_granularity(struct port_hdl *brdev,
                                                  uint32_t gate,
                                                  uint32_t *t_granularity);

/**
 * @ingroup tsn_psfp_api
 * @brief Get the current time value
 *
 * @param[in]    brdev          Bridge handler
 * @param[in]    gate           Gate instance
 * @param[out]   current_time   Retrieved Current Time value
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR   See tsn_return_value
 *
 **/
tsn_return_value tsn_psfp_gt_get_current_time(struct port_hdl *brdev,
                                              uint32_t gate,
                                              struct tsn_st_ptp_time *current_time);

/**
 * @ingroup tsn_psfp_api
 * @brief Get the congig pending value
 *
 * @param[in]    brdev          Bridge handler
 * @param[in]    gate           Gate instance
 * @param[out]   conf_pending   Retrieved Config Pending value
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR   See tsn_return_value
 *
 **/
tsn_return_value tsn_psfp_gt_get_config_pending(struct port_hdl *brdev,
                                                uint32_t gate,
                                                bool *conf_pending);

/**
 * @ingroup tsn_psfp_api
 * @brief Get the congig change error value
 *
 * @param[in]    brdev          Bridge handler
 * @param[in]    gate           Gate instance
 * @param[out]   conf_chg_error Retrieved Config Change Error value
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR   See tsn_return_value
 *
 **/
tsn_return_value tsn_psfp_gt_get_config_change_error(struct port_hdl *brdev,
                                                     uint32_t gate,
                                                     uint64_t *conf_chg_error);

/**
 * @ingroup tsn_psfp_api
 * @brief Get the admin internal priority value (IPV)
 *
 * @param[in]    brdev          Bridge handler
 * @param[in]    gate           Gate instance
 * @param[out]   admin_ipv      Retrieved Admin IPV.
 *                              PSFP_GT_IPV_NULL denotes the null value.
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR   See tsn_return_value
 *
 **/
tsn_return_value tsn_psfp_gt_get_admin_ipv(struct port_hdl *brdev,
                                           uint32_t gate,
                                           int8_t *admin_ipv);

/**
 * @ingroup tsn_psfp_api
 * @brief Modify the admin internal priority value (IPV)
 *
 * @param[in]    brdev          Bridge handler
 * @param[in]    gate           Gate instance
 * @param[in]    admin_ipv      Admin IPV to be set
 *                              PSFP_GT_IPV_NULL denotes the null value.
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR   See tsn_return_value
 *
 **/
tsn_return_value tsn_psfp_gt_set_admin_ipv(struct port_hdl *brdev,
                                           uint32_t gate,
                                           int8_t admin_ipv);

/**
 * @ingroup tsn_psfp_api
 * @brief Get the oper internal priority value (IPV)
 *
 * @param[in]    brdev          Bridge handler
 * @param[in]    gate           Gate instance
 * @param[out]   oper_ipv       Retrieved Oper IPV
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR   See tsn_return_value
 *
 **/
tsn_return_value tsn_psfp_gt_get_oper_ipv(struct port_hdl *brdev,
                                          uint32_t gate,
                                          uint8_t *oper_ipv);

/**
 * @ingroup tsn_psfp_api
 * @brief Get the oper internal priority value (IPV)
 *
 * @param[in]    brdev              Bridge handler
 * @param[in]    gate               Gate instance
 * @param[out]   oper_ipv           Retrieved Oper IPV
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR       See tsn_return_value
 *
 **/
tsn_return_value tsn_psfp_gt_get_oper_ipv(struct port_hdl *brdev,
                                          uint32_t gate,
                                          uint8_t *oper_ipv);

/**
 * @ingroup tsn_psfp_api
 * @brief Get the Gate Closed Due to Invalid Rx Enable value
 *
 * @param[in]    brdev              Bridge handler
 * @param[in]    gate               Gate instance
 * @param[out]   gate_closed_ena    Retrieved enable value
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR       See tsn_return_value
 *
 **/
tsn_return_value tsn_psfp_gt_get_closed_inv_rx_enable(struct port_hdl *brdev,
                                                      uint32_t gate,
                                                      bool *gate_closed_ena);

/**
 * @ingroup tsn_psfp_api
 * @brief Set the Gate Closed Due to Invalid Rx Enable value
 *
 * @param[in]    brdev              Bridge handler
 * @param[in]    gate               Gate instance
 * @param[in]    gate_closed_ena    Enable value to be set
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR       See tsn_return_value
 *
 **/
tsn_return_value tsn_psfp_gt_set_closed_inv_rx_enable(struct port_hdl *brdev,
                                                      uint32_t gate,
                                                      bool gate_closed_ena);

/**
 * @ingroup tsn_psfp_api
 * @brief Get the Gate Closed Due to Invalid Rx value
 *
 * @param[in]    brdev          Bridge handler
 * @param[in]    gate           Gate instance
 * @param[out]   gate_closed    Retrieved gate closed value
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR   See tsn_return_value
 *
 **/
tsn_return_value tsn_psfp_gt_get_closed_inv_rx(struct port_hdl *brdev,
                                               uint32_t gate,
                                               bool *gate_closed);

/**
 * @ingroup tsn_psfp_api
 * @brief Set the Gate Closed Due to Invalid Rx value
 *
 * @param[in]    brdev          Bridge handler
 * @param[in]    gate           Gate instance
 * @param[in]    gate_closed    Retrieved gate closed value
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR   See tsn_return_value
 *
 **/
tsn_return_value tsn_psfp_gt_set_closed_inv_rx(struct port_hdl *brdev,
                                               uint32_t gate,
                                               bool gate_closed);

/**
 * @ingroup tsn_psfp_api
 * @brief Set the gate of per port per stream threshold counters
 *
 * @param[in]    brdev          Bridge handler
 * @param[in]    gate           Gate number to use for counting violations to
 *                              gate low and high thresholds.
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR   See tsn_return_value
 *
 **/
tsn_return_value tsn_psfp_hit_set_gate(struct port_hdl *brdev,
                                       unsigned int gate);

/**
 * @ingroup tsn_psfp_api
 * @brief Get the gate of per port per stream threshold counters
 *
 * @param[in]    brdev          Bridge handler
 * @param[out]   gate           Gate number used for counting violations to
 *                              gate low and high thresholds.
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR   See tsn_return_value
 *
 **/
tsn_return_value tsn_psfp_hit_get_gate(struct port_hdl *brdev,
                                       unsigned int *gate);

/**
 * @ingroup tsn_psfp_api
 * @brief Set the value of per port per stream low threshold
 *
 * @param[in]    brdev          Bridge handler
 * @param[in]    th_ns          Low threshold value in nanoseconds.
 *                              Frames that violate expected low threshold
 *                              will be included in corresponding low
 *                              threshold violation counter.
 *                              Note that these hit counters apply to gate 0.
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR   See tsn_return_value
 *
 **/
tsn_return_value tsn_psfp_hit_set_low_th(struct port_hdl *brdev,
                                         unsigned int th_ns);

/**
 * @ingroup tsn_psfp_api
 * @brief Get the value of per port per stream low threshold
 *
 * @param[in]    brdev          Bridge handler
 * @param[out]   th_ns          Retrieved low threshold value in nanoseconds
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR   See tsn_return_value
 *
 **/
tsn_return_value tsn_psfp_hit_get_low_th(struct port_hdl *brdev,
                                         unsigned int *th_ns);

/**
 * @ingroup tsn_psfp_api
 * @brief Set the value of per port per stream high threshold
 *
 * @param[in]    brdev          Bridge handler
 * @param[in]    th_ns          High threshold value in nanoseconds.
 *                              Frames that violate expected high threshold
 *                              will be included in corresponding high
 *                              threshold violation counter.
 *                              Note that these hit counters apply to gate 0.
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR   See tsn_return_value
 *
 **/
tsn_return_value tsn_psfp_hit_set_high_th(struct port_hdl *brdev,
                                          unsigned int th_ns);

/**
 * @ingroup tsn_psfp_api
 * @brief Get the value of per port per stream high threshold
 *
 * @param[in]    brdev          Bridge handler
 * @param[out]   th_ns          Retrieved high threshold value in nanoseconds
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR   See tsn_return_value
 *
 **/
tsn_return_value tsn_psfp_hit_get_high_th(struct port_hdl *brdev,
                                          unsigned int *th_ns);

/**
 * @ingroup tsn_psfp_api
 * @brief Get the value of per port per stream low threshold violation counter
 *
 * @param[in]    port           Port handler
 * @param[in]    str_hdl        Stream Handle for which to the counter value is
 *                              retrieved
 * @param[out]   cnt_val        Pointer to retrieved counter value
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR   See tsn_return_value
 *
 **/
tsn_return_value tsn_psfp_hit_get_low_cnt(struct port_hdl *port,
                                          uint32_t str_hdl,
                                          uint32_t *cnt_val);

/**
 * @ingroup tsn_psfp_api
 * @brief Get the value of per port per stream high threshold violation counter
 *
 * @param[in]    port           Port handler
 * @param[in]    str_hdl        Stream Handle for which to the counter value is
 *                              retrieved
 * @param[out]   cnt_val        Pointer to retrieved counter value
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR   See tsn_return_value
 *
 **/
tsn_return_value tsn_psfp_hit_get_high_cnt(struct port_hdl *port,
                                           uint32_t str_hdl,
                                           uint32_t *cnt_val);

const char *tsn_psfp_get_known_ernno_msg(tsn_return_value return_code);

#endif /*TSN_PSFP_H_*/
