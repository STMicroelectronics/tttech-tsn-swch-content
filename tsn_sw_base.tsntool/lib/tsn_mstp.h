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

#ifndef TSN_MSTP_H_
#define TSN_MSTP_H_

#include "tsn_def.h"

/** @addtogroup tsn_mstp_api MSTP device management API */

/**
 * @ingroup tsn_mstp_api
 * @brief Flush all FDB entries related to the given port in the given tree
 *
 * @param[in]    port             Handler of the port
 * @param[in]    mstid            Which tree to flush. Range is 0(CIST)-0xFFF.
 *                                Note, that the device may furtherly restict
 *                                the use of reserved MSTIDs
 *                                (see IEEE 802.1Q-2014, Sect. 8.6.1)
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR     See tsn_return_value
 *
 **/
tsn_return_value tsn_mstp_flush_tree_port(struct port_hdl *port,
                                          unsigned int mstid);

/**
 * @ingroup tsn_mstp_api
 * @brief Shows the maximum number of msti supported
 *
 * @param[in]    port             Handler of the port
 * @param[out]   num_msti         Maximum number of msti supported
 *                                Note, that the device may furtherly restict
 *                                the use of reserved MSTIDs
 *                                (see IEEE 802.1Q-2014, Sect. 8.6.1)
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR     See tsn_return_value
 *
 **/
tsn_return_value tsn_mstp_get_max_msti(const struct port_hdl *port,
                                       unsigned int * const num_msti);

/**
 * @ingroup tsn_mstp_api
 * @brief Create a new mstp tree
 *
 * @param[in]    port             Handler of the port
 * @param[in]    mstid            Which tree to create
 *                                Note, that the device may furtherly restict
 *                                the use of reserved MSTIDs
 *                                (see IEEE 802.1Q-2014, Sect. 8.6.1)
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR     See tsn_return_value
 *
 **/
tsn_return_value tsn_mstp_create_msti(const struct port_hdl *port,
                                      const unsigned int mstid);

/**
 * @ingroup tsn_mstp_api
 * @brief Deletes an existing mstp tree
 *
 * @param[in]    port             Handler of the port
 * @param[in]    mstid            Which tree to remove
 *                                Note, that the device may furtherly restict
 *                                the use of reserved MSTIDs
 *                                (see IEEE 802.1Q-2014, Sect. 8.6.1)
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR     See tsn_return_value
 *
 **/
tsn_return_value tsn_mstp_delete_msti(const struct port_hdl *port,
                                      const unsigned int mstid);

/**
 * @ingroup tsn_mstp_api
 * @brief Shows the corresponding mstid for a given fid
 *
 * @param[in]    port             Handler of the port
 * @param[in]    fid              Which fid
 * @param[out]   mstid            Which tree it belongs
 *                                Note, that the device may furtherly restict
 *                                the use of reserved MSTIDs
 *                                (see IEEE 802.1Q-2014, Sect. 8.6.1)
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR     See tsn_return_value
 *
 **/
tsn_return_value tsn_mstp_get_fid_to_msti(const struct port_hdl *port,
                                          const unsigned int fid,
                                          unsigned int * const mstid);

/**
 * @ingroup tsn_mstp_api
 * @brief Creates the corresponding relationship between a given mstid and a given fid
 *
 * @param[in]    port             Handler of the port
 * @param[in]    fid              Which fid
 * @param[in]    mstid            Which tree it belongs
 *                                Note, that the device may furtherly restict
 *                                the use of reserved MSTIDs
 *                                (see IEEE 802.1Q-2014, Sect. 8.6.1)
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR     See tsn_return_value
 *
 **/
tsn_return_value tsn_mstp_set_fid_to_msti(const struct port_hdl *port,
                                          const unsigned int fid,
                                          const unsigned int mstid);

/**
 * @ingroup tsn_mstp_api
 * @brief Returns the list of used mstids in the system
 *
 * @param[in]    port             Handler of the port
 * @param[out]   msti_list        List of mstid in the system
 *                                Note, that the device may furtherly restict
 *                                the use of reserved MSTIDs
 *                                (see IEEE 802.1Q-2014, Sect. 8.6.1)
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR     See tsn_return_value
 *
 **/
tsn_return_value tsn_mstp_get_msti_list(const struct port_hdl *port,
                                        struct msti_list_entry *msti_list);

/**
 * @ingroup tsn_mstp_api
 * @brief Returns the port state per given tree and port
 *
 * @param[in]    port             Handler of the port
 * @param[in]    mstid            ID of the tree
 * @param[out]   port_state       State of the port
 *                                Note, that the device may furtherly restict
 *                                the use of reserved MSTIDs
 *                                (see IEEE 802.1Q-2014, Sect. 8.6.1)
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR     See tsn_return_value
 *
 **/
tsn_return_value tsn_mstp_get_port_state(const struct port_hdl *port,
                                         const unsigned int mstid,
                                         port_mstp_state * const port_state);

/**
 * @ingroup tsn_mstp_api
 * @brief Writes the port state per given tree and port
 *
 * @param[in]    port             Handler of the port
 * @param[in]    mstid            ID of the tree
 * @param[in]   port_state        State of the port
 *                                Note, that the device may furtherly restict
 *                                the use of reserved MSTIDs
 *                                (see IEEE 802.1Q-2014, Sect. 8.6.1)
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR     See tsn_return_value
 *
 **/
tsn_return_value tsn_mstp_set_port_state(const struct port_hdl *port,
                                         const unsigned int mstid,
                                         const port_mstp_state port_state);

const char *tsn_mstp_get_known_ernno_msg(tsn_return_value return_code);

#endif /*TSN_MSTP_H_*/
