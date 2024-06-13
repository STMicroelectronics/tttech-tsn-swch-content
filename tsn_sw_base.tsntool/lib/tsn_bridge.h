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

#ifndef TSN_BRIDGE_H_
#define TSN_BRIDGE_H_

#include "tsn_def.h"

/** @addtogroup tsn_bridge_api Bridge management data access abstraction API */

/**
 * @ingroup tsn_bridge_api
 * @brief Get the supported number of traffic classes for a port
 *
 * @param[in]    port       Handler of the port
 * @param[out]   num_tcs    number of supported traffic classes
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NOERROR      See tsn_return_value
 *
 **/
tsn_return_value tsn_bridge_get_num_traffic_classes(struct port_hdl *port,
                                                    uint32_t *num_tcs);

/**
 * @ingroup tsn_bridge_api
 * @brief Set the port default priority, i.e., the priority to which untagged
 *        frames are assinged to on ingress (compare to PVID).
 *
 * @param[in]    port       Handler of the port
 * @param[out]   dflt_prio  default priotity
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NOERROR      See tsn_return_value
 *
 **/
tsn_return_value tsn_bridge_set_default_prio(struct port_hdl *port,
                                             uint32_t dflt_prio);

/**
 * @ingroup tsn_bridge_api
 * @brief Get the port default priority, i.e., the priority to which untagged
 *        frames are assinged to on ingress (compare to PVID).
 *
 * @param[in]    port       Handler of the port
 * @param[out]   dflt_prio  default priotity
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NOERROR      See tsn_return_value
 *
 **/
tsn_return_value tsn_bridge_get_default_prio(struct port_hdl *port,
                                             uint32_t *dflt_prio);

/**
 * @ingroup tsn_bridge_api
 * @brief Get the port ID as reported via netdev_ops (ndo_get_phys_port_name)
 *
 * @param[in]    port       Handler of the port
 * @param[out]   port_id    Port ID
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NOERROR      See tsn_return_value
 *
 **/
tsn_return_value tsn_bridge_port_id(struct port_hdl *port,
                                    uint32_t *port_id);

/**
 * @ingroup tsn_bridge_api
 * @brief Get an entry of the Priority regeneration table
 *
 * @param[in]    port             Handler of the port
 * @param[in]    pcp              Which entry is going to be set(0-7)
 * @param[in]    admin_sentry     Admin Idle Slope entry to be set
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NOERROR      See tsn_return_value
 *
 **/
tsn_return_value tsn_fqtss_get_prio_regen_table(struct port_hdl *port,
                                                unsigned int pcp,
                                                uint8_t *admin_sentry);

/**
 * @ingroup tsn_bridge_api
 * @brief Get an entry of the Traffic class table
 *
 * @param[in]    port             Handler of the port
 * @param[in]    prio             Which entry is going to be set(0-7)
 * @param[in]    admin_sentry     Admin Idle Slope entry to be set
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NOERROR      See tsn_return_value
 *
 **/
tsn_return_value tsn_fqtss_get_tc_table(struct port_hdl *port,
                                        unsigned int prio,
                                        uint8_t *admin_sentry);

/**
 * @ingroup tsn_bridge_api
 * @brief Modify an entry of the Priority regeneration table
 *
 * @param[in]    port             Handler of the port
 * @param[in]    pcp              Which entry is going to be set(0-7)
 * @param[in]    admin_sentry     Admin Idle Slope entry to be set
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NOERROR      See tsn_return_value
 *
 **/
tsn_return_value tsn_fqtss_set_prio_regen_table(struct port_hdl *port,
                                                unsigned int pcp,
                                                const uint8_t admin_sentry);
/**
 * @ingroup tsn_bridge_api
 * @brief Modify an entry of the Priority regeneration table
 *
 * @param[in]    port             Handler of the port
 * @param[in]    prio             Which entry is going to be set(0-7)
 * @param[in]    admin_sentry     Admin Idle Slope entry to be set
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NOERROR      See tsn_return_value
 *
 **/
tsn_return_value tsn_fqtss_set_tc_table(struct port_hdl *port,
                                        unsigned int prio,
                                        const uint8_t admin_sentry);

/**
 * @ingroup tsn_bridge_api
 * @brief Get the complete structure for brport attributes and capabilities
 *
 * @param[in]    port             Handler of the port
 * @param[out]   caps             brport capabilities and attributes
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NOERROR      See tsn_return_value
 *
 **/
tsn_return_value tsn_bridge_port_attr(struct port_hdl *port,
                                      struct brport_attr_and_caps *caps);

/**
 * @ingroup tsn_bridge_api
 * @brief Get the complete structure for bridge attributes and capabilities
 *
 * @param[in]    port             Handler of the bridge
 * @param[out]   caps             bridge capabilities and attributes
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NOERROR      See tsn_return_value
 *
 **/
tsn_return_value tsn_bridge_attr(struct port_hdl *port,
                                 struct bridge_attr_and_caps *caps);

/**
 * @ingroup tsn_bridge_api
 * @brief Get min and max dependent delays and min and max independent delays
 *        for one port to another for a given traffic class
 *
 * @param[in]    bridge           Handler of the bridge
 * @param[in]    ingress_pt       rx port for delay
 * @param[in]    egress_pt        tx port for delay
 * @param[in]    tc               traffic class
 * @param[out]   delay_entry      delays for input ports and traffic class
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NOERROR      See tsn_return_value
 *
 **/
tsn_return_value tsn_bridge_get_delays(struct port_hdl *bridge,
                                       uint32_t ingress_pt,
                                       uint32_t egress_pt,
                                       uint32_t tc,
                                       struct br_delay_entry *delay_entry);

/**
 * @ingroup tsn_bridge_api
 * @brief Get min and max enhanced port-based delays
 *        for one port for a given speed
 *
 * @param[in]    port             Port for delay
 * @param[in]    speed            speed
 * @param[out]   delay_entry      delays for port and speed
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NOERROR      See tsn_return_value
 *
 **/
tsn_return_value tsn_bridge_get_port_delays(struct port_hdl *port,
	                                    uint32_t speed,
                                            struct pt_delay_entry *delay_entry);

const char *tsn_bridge_get_known_ernno_msg(tsn_return_value return_code);

#endif /*TSN_BRIDGE_H_*/
