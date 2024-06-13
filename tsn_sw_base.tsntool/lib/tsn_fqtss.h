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

#ifndef TSN_FQTSS_H_
#define TSN_FQTSS_H_

#include "tsn_def.h"

/** @defgroup tsn_fqtss_api FQTSS management data access abstraction API */

// RW: adminIdleSlopeTable
/**
 * @ingroup tsn_fqtss_api
 * @brief Get an entry of the admin idle slope table
 *
 * @param[in]    port             Handler of the port
 * @param[in]    prio             Which entry is going to be retrieved(0-7)
 * @param[out]   admin_stable     Retrieved Admin Idle Slope entry
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR     See tsn_return_value
 *
 **/
tsn_return_value tsn_fqtss_get_admin_slope_table(struct port_hdl *port,
                                                 unsigned int prio,
                                                 uint64_t *admin_stable);

/**
 * @ingroup tsn_fqtss_api
 * @brief Modify an entry of the admin idle slope table
 *
 * @param[in]    port             Handler of the port
 * @param[in]    prio             Which entry is going to be set(0-7)
 * @param[in]    admin_stable     Admin Idle Slope entry to be set
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR     See tsn_return_value
 *
 **/
tsn_return_value tsn_fqtss_set_admin_slope_table(struct port_hdl *port,
                                                 unsigned int prio,
                                                 const uint64_t admin_stable);

// R: operIdleSlopeTable
/**
 * @ingroup tsn_fqtss_api
 * @brief Get an entry of the oper idle slope table
 *
 * @param[in]    port             Handler of the port
 * @param[in]    prio             Which entry is going to be retrieved(0-7)
 * @param[out]   oper_stable      Retrieved Oper Idle Slope entry
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR     See tsn_return_value
 *
 **/
tsn_return_value tsn_fqtss_get_oper_slope_table(struct port_hdl *port,
                                                unsigned int prio,
                                                uint64_t *oper_stable);

/**
 * @ingroup tsn_fqtss_api
 * @brief Get an entry of the transmission selection algorithm table
 *
 * @param[in]    port             Handler of the port
 * @param[in]    prio             Which entry is going to be retrieved(0-7)
 * @param[out]   txsel_sentry     Retrieved Transmission Selection Algorithm entry
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR     See tsn_return_value
 *
**/
tsn_return_value tsn_fqtss_get_tx_selection_table(struct port_hdl *port,
                                                  unsigned int prio,
                                                  uint8_t *txsel_sentry);

/**
 * @ingroup tsn_fqtss_api
 * @brief Modify an entry of the transmission selection algorithm table
 *
 * @param[in]    port             Handler of the port
 * @param[in]    prio             Which entry is going to be set(0-7)
 * @param[in]    txsel_sentry     Transmission selection algorithm to be set
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR     See tsn_return_value
 *
 **/
tsn_return_value tsn_fqtss_set_tx_selection_table(struct port_hdl *port,
                                                  unsigned int prio,
                                                  const uint8_t txsel_sentry);

const char *tsn_fqtss_get_known_ernno_msg(tsn_return_value return_code);

#endif /*TSN_FQTSS_H_*/
