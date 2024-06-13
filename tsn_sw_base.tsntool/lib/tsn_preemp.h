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

#ifndef TSN_PREEMP_H_
#define TSN_PREEMP_H_

#include "tsn_def.h"

/** @addtogroup tsn_preemp_api Preemption management data access abstraction API */

/**
 * @ingroup tsn_preemp_api
 * @brief Get an entry of the preemption status table
 *
 * @param[in]    port             Handler of the port
 * @param[in]    prio             Which entry is going to be retrieved(0-7)
 * @param[out]   preemp_status    Retrieved Preemption Status entry
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR     See tsn_return_value
 *
 **/
tsn_return_value tsn_preemp_get_status_table(struct port_hdl *port,
                                             uint8_t prio,
                                             preemp_stat *preemp_status);

/**
 * @ingroup tsn_preemp_api
 * @brief Modify an entry of the preemption status table
 *
 * @param[in]    port             Handler of the port
 * @param[in]    prio             Which entry is going to be retrieved(0-7)
 * @param[in]    preemp_status    Preemption Status entry to be set
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR     See tsn_return_value
 *
 **/
tsn_return_value tsn_preemp_set_status_table(struct port_hdl *port,
                                             uint8_t prio,
                                             const preemp_stat preemp_status);
/**
 * @ingroup tsn_preemp_api
 * @brief Get the hold advace value
 *
 * @param[in]    port             Handler of the port
 * @param[out]   hold_advance     Retrieved Hold Advance value
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR     See tsn_return_value
 *
 **/
tsn_return_value tsn_preemp_get_hold_advance(struct port_hdl *port,
                                             uint32_t *hold_advance);

/**
 * @ingroup tsn_preemp_api
 * @brief Get the hold request value
 *
 * @param[in]    port             Handler of the port
 * @param[out]   hold_request     Retrieved Hold Request value
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR     See tsn_return_value
 *
 **/
tsn_return_value tsn_preemp_get_hold_request(struct port_hdl *port,
                                             uint32_t *hold_request);

/**
 * @ingroup tsn_preemp_api
 * @brief Get the preemption active value
 *
 * @param[in]    port             Handler of the port
 * @param[out]   preemp_active    Retrieved Preemption Active value
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR     See tsn_return_value
 *
 **/
tsn_return_value tsn_preemp_get_preemption_active(struct port_hdl *port,
                                                  frame_preemp_active *preemp_active);

/**
 * @ingroup tsn_preemp_api
 * @brief Get the release advance value
 *
 * @param[in]    port             Handler of the port
 * @param[out]   rel_advance      Retrieved Release Advance value
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR     See tsn_return_value
 *
 **/
tsn_return_value tsn_preemp_get_release_advance(struct port_hdl *port,
                                                uint32_t *rel_advance);

/* 802.3br aMACMerge functions */
/**
 * @ingroup tsn_preemp_api
 * @brief Get the information whether the MAC Merge sublayer is supported on
 * the device. If it is supported, 1 is returned, otherwise it is 0.
 *
 * @param[in]    port             Handler of the port
 * @param[out]   support          Pointer to retrieved value
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR     See tsn_return_value
 *
 **/
tsn_return_value tsn_preemp_get_support(struct port_hdl *port, bool *support);

/**
 * @ingroup tsn_preemp_api
 * @brief Get the information about verification status. Status can be one of
 * the following: Unknown, Initial, Verifying, Succeeded, Failed, Disabled.
 * For more information check 802.3br-2016 30.14.1.2
 *
 * @param[in]    port             Handler of the port
 * @param[out]   verify_status    Pointer to retrieved verification status value
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR     See tsn_return_value
 *
 **/
tsn_return_value tsn_preemp_get_verify_status(struct port_hdl *port,
                                              preemp_vrfy_stat *verify_status);

/**
 * @ingroup tsn_preemp_api
 * @brief Get the information whether the transmit preemption is enabled or not
 *
 * @param[in]    port             Handler of the port
 * @param[out]   tx_en            Pointer to retrieved transmit-enable value
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR     See tsn_return_value
 *
 **/
tsn_return_value tsn_preemp_get_tx_enabled(struct port_hdl *port, bool *tx_en);

/**
 * @ingroup tsn_preemp_api
 * @brief Enable or disable preemption
 *
 * @param[in]    port             Handler of the port
 * @param[out]   tx_en            Value indicating whether to enable(1) or
 *                                disable(0) preemption
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR     See tsn_return_value
 *
 **/
tsn_return_value tsn_preemp_set_tx_enable(struct port_hdl *port,
                                          const bool tx_en);

/**
 * @ingroup tsn_preemp_api
 * @brief Indicates whether the status of the Verify function of the MAC Merge
 * sublayer.
 *
 * @param[in]    port             Handler of the port
 * @param[out]   verify_tx_dis    Pointer to the retrieved value
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR     See tsn_return_value
 *
 **/
tsn_return_value tsn_preemp_get_verify_tx_disabled(struct port_hdl *port,
                                                   bool *verify_tx_dis);

/**
 * @ingroup tsn_preemp_api
 * @brief Enable/disable preemption Verification function
 *
 * @param[in]    port             Handler of the port
 * @param[out]   verify_tx_dis    Value indicating whether to enable(1) or
 *                                disable Verification function
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR     See tsn_return_value
 *
 **/
tsn_return_value tsn_preemp_set_verify_tx_disable(struct port_hdl *port,
                                                  const bool verify_tx_dis);

/**
 * @ingroup tsn_preemp_api
 * @brief Value that denotes transmit preemption status, which can be unknown,
 * inactive or active.
 *
 * @param[in]    port             Handler of the port
 * @param[out]   status_tx        Pointer to the retrieved value of transmission
 *                                preemption status
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR     See tsn_return_value
 *
 **/
tsn_return_value tsn_preemp_get_status_tx(struct port_hdl *port,
                                          preemp_tx_stat *status_tx);

/**
 * @ingroup tsn_preemp_api
 * @brief Get the value of the time between verification attempts in miliseconds
 *
 * @param[in]    port             Handler of the port
 * @param[out]   time_ms          Pointer to the retrieved verification time
 *                                value
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR     See tsn_return_value
 *
 **/
tsn_return_value tsn_preemp_get_verify_time(struct port_hdl *port,
                                            uint8_t *time_ms);

/**
 * @ingroup tsn_preemp_api
 * @brief Set the value of the time between verification attempts in miliseconds
 * Allowed range of values is 1-128 (inclusive)
 *
 * @param[in]    port             Handler of the port
 * @param[out]   time_ms          Value to set the verification time to
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR     See tsn_return_value
 *
 **/
tsn_return_value tsn_preemp_set_verify_time(struct port_hdl *port,
                                            const uint8_t time_ms);

/**
 * @ingroup tsn_preemp_api
 * @brief Value of the variable addFragSize which controls the minimum non-final
 * mPacket length (check 802.3br-2016 99.4.7.3)
 *
 * @param[in]    port             Handler of the port
 * @param[out]   frag_size        Pointer of the retrieved value of addFragSize
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR     See tsn_return_value
 *
 **/
tsn_return_value tsn_preemp_get_frag_size(struct port_hdl *port,
                                          preemp_fragsz *frag_size);

/**
 * @ingroup tsn_preemp_api
 * @brief Set the value of the addFragSize, which controls the minimum non-final
 * mPacket length (check 802.3br-2016 99.4.7.3)
 *
 * @param[in]    port             Handler of the port
 * @param[out]   frag_size        The value to set the addFragSize to
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR     See tsn_return_value
 *
 **/
tsn_return_value tsn_preemp_set_frag_size(struct port_hdl *port,
                                          const preemp_fragsz frag_size);

/**
 * @ingroup tsn_preemp_api
 * @brief Get the value of the counter which is incremented by one every time
 * the ASSEMBLY_ERROR happens (check 802.3br-2016 30.14.1.8)
 *
 * @param[in]    port             Handler of the port
 * @param[out]   asserr_cnt       Pointer to the retrieved counter value
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR     See tsn_return_value
 *
 **/
tsn_return_value tsn_preemp_get_asserr_cnt(struct port_hdl *port,
                                           uint32_t *asserr_cnt);

/**
 * @ingroup tsn_preemp_api
 * @brief Get the value of the counter which counts MAC frames/MAC frame
 * fragments which are rejected due to unknown SMD value or arriving with an
 * SMD-C when no frame is in progress
 *
 * @param[in]    port             Handler of the port
 * @param[out]   smderr_cnt       Pointer to the retrieved counter value
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR     See tsn_return_value
 *
 **/
tsn_return_value tsn_preemp_get_smderr_cnt(struct port_hdl *port,
                                           uint32_t *smderr_cnt);

/**
 * @ingroup tsn_preemp_api
 * @brief Get the value of the counter which counts MAC frames that were
 * successfully reassembled and delivered to MAC.
 *
 * @param[in]    port             Handler of the port
 * @param[out]   assok_cnt        Pointer to the retrieved counter value
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR     See tsn_return_value
 *
 **/
tsn_return_value tsn_preemp_get_assok_cnt(struct port_hdl *port,
                                          uint32_t *assok_cnt);

/**
 * @ingroup tsn_preemp_api
 * @brief Get the value of the counter which counts the additional mPacktes
 * received due to preemption.
 *
 * @param[in]    port             Handler of the port
 * @param[out]   fragrx_cnt       Pointer to the retrieved counter value
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR     See tsn_return_value
 *
 **/
tsn_return_value tsn_preemp_get_fragrx_cnt(struct port_hdl *port,
                                           uint32_t *fragrx_cnt);

/**
 * @ingroup tsn_preemp_api
 * @brief Get the value of the counter which counts number of additional
 * mPackets which are transmitted due to preemption
 *
 * @param[in]    port             Handler of the port
 * @param[out]   fragtx_cnt       Pointer to the retrieved counter value
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR     See tsn_return_value
 *
 **/
tsn_return_value tsn_preemp_get_fragtx_cnt(struct port_hdl *port,
                                           uint32_t *fragtx_cnt);

/**
 * @ingroup tsn_preemp_api
 * @brief Get the value of the counter which counts number of times the hold
 * request transitioned from HOLD to RELEASE (check 802.3br-2016 99.4.7.3
 * variable hold)
 *
 * @param[in]    port             Handler of the port
 * @param[out]   hold_cnt         Pointer to the retrieved counter value
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR     See tsn_return_value
 *
 **/
tsn_return_value tsn_preemp_get_hold_cnt(struct port_hdl *port,
                                         uint32_t *hold_cnt);

const char *tsn_preemp_get_known_ernno_msg(tsn_return_value return_code);

#endif /*TSN_PREEMP_H_*/
