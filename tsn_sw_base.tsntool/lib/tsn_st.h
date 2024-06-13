/*
 * TTTech TSN Library (libtsn)
 * Copyright(c) 2017 TTTech Industrial Automation AG.
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

#ifndef TSN_ST_H_
#define TSN_ST_H_

#include "tsn_def.h"

/** @addtogroup tsn_st_api Scheduling management data access abstraction API */

// RW: queueMaxSDUTable, NOTE: TransmissionOverrun is RO
/**
 * @ingroup tsn_st_api
 * @brief Get an entry of the max sdu table
 *
 * @param[in]    port             Handler of the port
 * @param[in]    prio             Which entry is going to be retrieved(0-7)
 * @param[out]   max_sdu          Retrieved Max SDU entry
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR     See tsn_return_value
 *
 **/
tsn_return_value tsn_st_get_max_sdu(struct port_hdl *port, unsigned int prio,
                                    uint32_t *max_sdu);

/**
 * @ingroup tsn_st_api
 * @brief Get an entry of the transmission overrun table
 *
 * @param[in]    port             Handler of the port
 * @param[in]    prio             Which entry is going to be retrieved(0-7)
 * @param[out]   overrun          Retrieved Transmission Overrun entry
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR           See tsn_return_value
 *
 **/
tsn_return_value tsn_st_get_overrun(struct port_hdl *port, unsigned int prio,
                                    uint64_t *overrun);

/**
 * @ingroup tsn_st_api
 * @brief Modify an entry of the max sdu table
 *
 * @param[in]    port             Handler of the port
 * @param[in]    prio             Which entry is going to be retrieved(0-7)
 * @param[in]    max_sdu          Max SDU entry to be set
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR           See tsn_return_value
 *
 **/
tsn_return_value tsn_st_set_max_sdu(struct port_hdl *port, unsigned int prio,
                                    const uint32_t max_sdu);

// RW: GateEnabled
/**
 * @ingroup tsn_st_api
 * @brief Get the gate enabled value
 *
 * @param[in]    port             Handler of the port
 * @param[out]   gate_enabled     Retrieved Gate Enabled value
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR           See tsn_return_value
 *
 **/
tsn_return_value tsn_st_get_gate_enabled(struct port_hdl *port,
                                         bool *gate_enabled);

/**
 * @ingroup tsn_st_api
 * @brief Modify the gate enabled value
 *
 * @param[in]    port             Handler of the port
 * @param[in]    gate_enabled     Gate Enabled value to be set
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR           See tsn_return_value
 *
 **/
tsn_return_value tsn_st_set_gate_enabled(struct port_hdl *port,
                                         const bool gate_enabled);

// RW: AdminGateStates
/**
 * @ingroup tsn_st_api
 * @brief Get the admin gate state value
 *
 * @param[in]    port             Handler of the port
 * @param[out]   admin_gate_state Retrieved Admin Gate State value
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR           See tsn_return_value
 *
 **/
tsn_return_value tsn_st_get_admin_gate_states(struct port_hdl *port,
                                              uint8_t *admin_gate_state);

/**
 * @ingroup tsn_st_api
 * @brief Modify the admin gate state value
 *
 * @param[in]    port             Handler of the port
 * @param[in]    admin_gate_state Admin Gate State value to be set
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR           See tsn_return_value
 *
 **/
tsn_return_value tsn_st_set_admin_gate_states(struct port_hdl *port,
                                              const uint8_t admin_gate_state);

// R: OperGateStates
/**
 * @ingroup tsn_st_api
 * @brief Get the oper gate state value
 *
 * @param[in]    port             Handler of the port
 * @param[out]   oper_gate_state  Retrieved Oper Gate State value
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR           See tsn_return_value
 *
 **/
tsn_return_value tsn_st_get_oper_gate_states(struct port_hdl *port,
                                             uint8_t *oper_gate_state);

// RW: AdminControlListLength
/**
 * @ingroup tsn_st_api
 * @brief Get the admin control list length value
 *
 * @param[in]    port             Handler of the port
 * @param[out]   acl_length       Retrieved Admin Control List Length value
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR           See tsn_return_value
 *
 **/
tsn_return_value tsn_st_get_admin_control_list_length(struct port_hdl *port,
                                                      uint32_t *acl_length);
/**
 * @ingroup tsn_st_api
 * @brief Modify the admin control list length value
 *
 * @param[in]    port             Handler of the port
 * @param[in]    acl_length       Admin Control List Length value to be set
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR           See tsn_return_value
 *
 **/
tsn_return_value tsn_st_set_admin_control_list_length(struct port_hdl *port,
                                                      const uint32_t acl_length);

// R: OperControlListLength
/**
 * @ingroup tsn_st_api
 * @brief Get the oper control list length value
 *
 * @param[in]    port             Handler of the port
 * @param[out]   ocl_length       Retrieved Oper Control List Length value
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR           See tsn_return_value
 *
 **/
tsn_return_value tsn_st_get_oper_control_list_length(struct port_hdl *port,
                                                     uint32_t *ocl_length);

// RW: AdminControlList
/**
 * @ingroup tsn_st_api
 * @brief Get an admin control list entry value
 *
 * @param[in]    port             Handler of the port
 * @param[in]    entries          Maximum number of Control List Entries to be read.
 * @param[out]   acl_entry        Admin Control List
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR           See tsn_return_value
 *
 **/
tsn_return_value tsn_st_get_admin_control_list(struct port_hdl *port,
                                               uint32_t entries,
                                               struct st_gate_control_entry *acl_entry);

/**
 * @ingroup tsn_st_api
 * @brief Modify an admin control list entry value
 *
 * @param[in]    port             Handler of the port
 * @param[in]    entries          Number of elements in given Admin Control List
 * @param[in]    acl_entry        Admin Control List
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR           See tsn_return_value
 *
 **/
tsn_return_value tsn_st_set_admin_control_list(struct port_hdl *port,
                                               uint32_t entries,
                                               const struct st_gate_control_entry *acl_entry);

// R: OperControlList
/**
 * @ingroup tsn_st_api
 * @brief Get an oper control list entry value
 *
 * @param[in]    port             Handler of the port
 * @param[in]    entries          Maximum number of Control List Entries to be read.
 * @param[out]   ocl_entry        Oper Control List
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR           See tsn_return_value
 *
 **/
tsn_return_value tsn_st_get_oper_control_list(struct port_hdl *port,
                                              uint32_t entries,
                                              struct st_gate_control_entry *ocl_entry);

// RW: AdminCycleTime
/**
 * @ingroup tsn_st_api
 * @brief Get the admin cycle time value
 *
 * @param[in]    port             Handler of the port
 * @param[out]   admin_ctime      Retrieved Admin Cycle Time value
 *
 * @return int                    Linux system error code (errno)
 * @retval 0                      Admin Cycle Time value successfully retrieved
 *
 **/
tsn_return_value tsn_st_get_admin_cycle_time(struct port_hdl *port,
                                             struct tsn_st_rational_number *admin_ctime);

/**
 * @ingroup tsn_st_api
 * @brief Modify the admin cycle time value
 *
 * @param[in]    port             Handler of the port
 * @param[in]    admin_ctime      Admin Cycle Time value to be set
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR           See tsn_return_value
 *
 **/
tsn_return_value tsn_st_set_admin_cycle_time(struct port_hdl *port,
                                             const struct tsn_st_rational_number *admin_ctime);

// R: OperCycleTime
/**
 * @ingroup tsn_st_api
 * @brief Get the oper cycle time value
 *
 * @param[in]    port             Handler of the port
 * @param[out]   oper_ctime       Retrieved Oper Cycle Time value
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR           See tsn_return_value
 *
 **/
tsn_return_value tsn_st_get_oper_cycle_time(struct port_hdl *port,
                                            struct tsn_st_rational_number *oper_ctime);

// RW: AdminCycleTimeExtension
/**
 * @ingroup tsn_st_api
 * @brief Get the admin cycle time extension value
 *
 * @param[in]    port             Handler of the port
 * @param[out]   admin_ctime_ext  Retrieved Admin Cycle Time Extension value
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR           See tsn_return_value
 *
 **/
tsn_return_value tsn_st_get_admin_cycle_time_extension(struct port_hdl *port,
                                                       uint32_t *admin_ctime_ext);

/**
 * @ingroup tsn_st_api
 * @brief Modify the admin cycle time extension value
 *
 * @param[in]    port             Handler of the port
 * @param[in]    admin_ctime_ext  Admin Cycle Time Extension value to be set
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR           See tsn_return_value
 *
 **/
tsn_return_value tsn_st_set_admin_cycle_time_extension(struct port_hdl *port,
                                                       const uint32_t admin_ctime_ext);

// R: OperCycleTimeExtension
/**
 * @ingroup tsn_st_api
 * @brief Get the oper cycle time extension value
 *
 * @param[in]    port             Handler of the port
 * @param[out]   oper_ctime_ext   Retrieved Oper Cycle Time Extension value
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR           See tsn_return_value
 *
 **/
tsn_return_value tsn_st_get_oper_cycle_time_extension(struct port_hdl *port,
                                                      uint32_t *oper_ctime_ext);

// RW: AdminBaseTime
/**
 * @ingroup tsn_st_api
 * @brief Get the admin base time value
 *
 * @param[in]    port             Handler of the port
 * @param[out]   admin_btime      Retrieved Admin Base Time value
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR           See tsn_return_value
 *
 **/
tsn_return_value tsn_st_get_admin_base_time(struct port_hdl *port,
                                            struct tsn_st_ptp_time *admin_btime);

/**
 * @ingroup tsn_st_api
 * @brief Modify the admin base time value
 *
 * @param[in]    port             Handler of the port
 * @param[in]    admin_btime      Admin Base Time value to be set
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR           See tsn_return_value
 *
 **/
tsn_return_value tsn_st_set_admin_base_time(struct port_hdl *port,
                                            const struct tsn_st_ptp_time *admin_btime);

// R: OperBaseTime
/**
 * @ingroup tsn_st_api
 * @brief Get the oper base time value
 *
 * @param[in]    port             Handler of the port
 * @param[out]   oper_btime       Retrieved Oper Base Time value
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR           See tsn_return_value
 *
 **/
tsn_return_value tsn_st_get_oper_base_time(struct port_hdl *port,
                                           struct tsn_st_ptp_time *oper_btime);

// RW: ConfigChange
/**
 * @ingroup tsn_st_api
 * @brief Get the config change value
 *
 * @param[in]    port             Handler of the port
 * @param[out]   config_change    Retrieved Config Change value
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR           See tsn_return_value
 *
 **/
tsn_return_value tsn_st_get_config_change(struct port_hdl *port,
                                          bool *config_change);

/**
 * @ingroup tsn_st_api
 * @brief Modify the config change value
 *
 * @param[in]    port             Handler of the port
 * @param[in]    config_change    Config Change value to be set
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR           See tsn_return_value
 *
 **/
tsn_return_value tsn_st_set_config_change(struct port_hdl *port,
                                          const bool config_change);

// R: ConfigChangeTime
/**
 * @ingroup tsn_st_api
 * @brief Get the actual config change time, i.e., the time at which the
 *        administrative variables are copied across to the corresponding
 *        operational variables.
 *
 * @param[in]    port                Handler of the port
 * @param[out]   config_change_time  Retrieved Config Change Time value
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR           See tsn_return_value
 *
 **/
tsn_return_value tsn_st_get_config_change_time(struct port_hdl *port,
                                               struct tsn_st_ptp_time *config_change_time);

// R: TickGranularity
/**
 * @ingroup tsn_st_api
 * @brief Get the tick granularity value
 *
 * @param[in]    port             Handler of the port
 * @param[out]   t_granularity    Retrieved Tick Granularity value
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR           See tsn_return_value
 *
 **/
tsn_return_value tsn_st_get_tick_granularity(struct port_hdl *port,
                                             uint32_t *t_granularity);

// R: CurrentTime
/**
 * @ingroup tsn_st_api
 * @brief Get the current time value
 *
 * @param[in]    port             Handler of the port
 * @param[out]   current_time     Retrieved Current Time value
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR           See tsn_return_value
 *
 **/
tsn_return_value tsn_st_get_current_time(struct port_hdl *port,
                                         struct tsn_st_ptp_time *current_time);

// R: ConfigPending
/**
 * @ingroup tsn_st_api
 * @brief Get the congig pending value
 *
 * @param[in]    port             Handler of the port
 * @param[out]   conf_pending     Retrieved Config Pending value
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR           See tsn_return_value
 *
 **/
tsn_return_value tsn_st_get_config_pending(struct port_hdl *port,
                                           bool *conf_pending);

// R: ConfigChangeError
/**
 * @ingroup tsn_st_api
 * @brief Get the congig change error value
 *
 * @param[in]    port             Handler of the port
 * @param[out]   conf_chg_error   Retrieved Config Change Error value
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR           See tsn_return_value
 *
 **/
tsn_return_value tsn_st_get_config_change_error(struct port_hdl *port,
                                                int *conf_chg_error);

// R: SupportedListMax
/**
 * @ingroup tsn_st_api
 * @brief Get the Supported List Max value
 *
 * @param[in]    port             Handler of the port
 * @param[out]   supp_list_max    Retrieved Supported List Max value
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR           See tsn_return_value
 *
 **/
tsn_return_value tsn_st_get_supported_list_max(struct port_hdl *port,
                                               uint32_t *supp_list_max);

/**
 * @ingroup tsn_st_api
 * @brief Get the maximum supported cycle time
 *
 * @param[in]    brdev          Bridge handler
 * @param[out]   max_cyc        Retrieved maximum supported cycle time
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR   See tsn_return_value
 *
 **/
tsn_return_value tsn_st_get_supported_cycle_max(struct port_hdl *brdev,
                                                struct tsn_st_rational_number *max_cyc);

/**
 * @ingroup tsn_st_api
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
tsn_return_value tsn_st_get_supported_interval_max(struct port_hdl *brdev,
                                                   uint32_t *max_int_ns);

const char *tsn_st_get_known_ernno_msg(tsn_return_value return_code);

#endif /*TSN_ST_H_*/
