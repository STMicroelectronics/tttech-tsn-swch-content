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

#ifndef TSN_EDGEX_H_
#define TSN_EDGEX_H_

#include "tsn_def.h"

/** @addtogroup tsn_edgex_api Edgex management data access abstraction API */

/**
 * @ingroup tsn edgex_api
 * @brief Get the cut through value
 *
 * @param[in]    port             Handler of the port
 * @param[out]   cut_through      Cut Through Enabled value
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR     See tsn_return_value
 *
 **/
tsn_return_value tsn_edgex_get_cut_through(struct port_hdl *port,
                                           bool *cut_through);

/**
 * @ingroup tsn_edgex_api
 * @brief Modify the cut through value
 *
 * @param[in]    port             Handler of the port
 * @param[in]    cut_through      Cut Through value to be set
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR     See tsn_return_value
 *
 **/
tsn_return_value tsn_edgex_set_cut_through(struct port_hdl *port,
                                           const bool cut_through);

/**
 * @ingroup tsn edgex_api
 * @brief Setup mirror port. All traffic incoming on port will be mirrored to the
 *        port with the given ID.
 *
 * @param[in]    port             Handler of the port
 * @param[in]    mirr_port_id     ID of the port to which traffic shall be
 *                                mirrored to.
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR     See tsn_return_value
 *
 **/
tsn_return_value tsn_edgex_set_mirror_portid(struct port_hdl *port,
                                             const int mirr_port_id);

/**
 * @ingroup tsn edgex_api
 * @brief Get mirror port, i.e., the port to which all traffic incoming on port
 *        will be mirrored to.
 *
 * @param[in]    port             Handler of the port
 * @param[in]    mirr_port_id     ID of the port to which traffic is
 *                                mirrored to.
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR     See tsn_return_value
 *
 **/
tsn_return_value tsn_edgex_get_mirror_portid(struct port_hdl *port,
                                             int *mirr_port_id);

const char *tsn_edgex_get_known_ernno_msg(tsn_return_value return_code);

#endif /*TSN_EDGEX_H_*/
