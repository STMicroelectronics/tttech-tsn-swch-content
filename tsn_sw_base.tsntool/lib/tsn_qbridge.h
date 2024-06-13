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

#ifndef TSN_QBRIDGE_H_
#define TSN_QBRIDGE_H_

#include "tsn_def.h"

/** @addtogroup tsn_qbridge_api Bridge management data access abstraction API */

/**
 * @ingroup tsn_qbridge_api
 * @brief maximum VLAN supported by the implementation
 *
 * @param[in]    bridge             Handler of the bridge
 * @param[out]   max_vlan_id      maximum VLAN Id
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR       See tsn_return_value
 *
 **/
tsn_return_value tsn_qbridge_get_max_vlan_id(struct port_hdl *bridge,
                                             uint32_t *max_vlan_id);

/**
 * @ingroup tsn_qbridge_api
 * @brief maximum number of concurrently supported VLANs
 *
 * @param[in]    bridge                  Handler of the bridge
 * @param[out]   max_supported_vlans   maximum number of VLANs
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR       See tsn_return_value
 *
 **/
tsn_return_value tsn_qbridge_get_max_supported_vlans(const struct port_hdl *bridge,
                                                     uint32_t *max_supported_vlans);

/**
 * @ingroup tsn_qbridge_api
 * @brief current number of VLANs
 *
 * @param[in]    bridge             Handler of the bridge
 * @param[out]   num_vlans        current number of VLANs
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR       See tsn_return_value
 *
 **/
tsn_return_value tsn_qbridge_get_num_vlans(struct port_hdl *bridge,
                                           uint32_t *num_vlans);

/**
 * @ingroup tsn_qbridge_api
 * @brief Assign VID to FID
 *
 * @param[in]    bridge           Handler of the bridge
 * @param[in]    vid              VID to assign
 * @param[in]    fid              FID to assign to
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR       See tsn_return_value
 *
 **/
tsn_return_value tsn_qbridge_set_vid2fid(struct port_hdl *bridge,
                                         uint32_t vid, uint32_t fid);

/**
 * @ingroup tsn_qbridge_api
 * @brief Get VID-to-FID assignment
 *
 * @param[in]    bridge           Handler of the bridge
 * @param[in]    vid              VID to query
 * @param[out]   fid              FID the given VID is assigned to
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR       See tsn_return_value
 *
 **/
tsn_return_value tsn_qbridge_get_vid2fid(struct port_hdl *bridge,
                                         uint32_t vid, uint32_t *fid);

/**
 * @ingroup tsn_qbridge_api
 * @brief Get FID-to-VID assignment
 *
 * @param[in]    bridge           Handler of the bridge
 * @param[in]    fid              FID to query
 * @param[in]    max_vids         Max number of vids
 * @param[out]   fid2vid          VID list of assigned to the given FID
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR       See tsn_return_value
 *
 **/
tsn_return_value tsn_qbridge_get_fid2vid(struct port_hdl *bridge,
                                         uint32_t fid, uint32_t max_vids,
                                         struct qbr_fid2vid_entry *fid2vid);

const char *tsn_qbridge_get_known_ernno_msg(tsn_return_value return_code);

#endif /*TSN_QBRIDGE_H_*/
