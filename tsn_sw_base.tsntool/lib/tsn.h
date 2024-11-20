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

#ifndef TSN_H_
#define TSN_H_

#include "tsn_def.h"

/** @addtogroup tsn_api Port handling abstraction API */

/**
 * @ingroup tsn_api
 * @brief Returns a proper port_hdl for the given port name
 *
 * @param[in]    ifname    Name of the port
 *
 * @return struct port_hdl
 *
 **/
struct port_hdl *tsn_port_open_by_name(const char *ifname);

/**
 * @ingroup tsn_api
 * @brief Returns a proper port_hdl for the given interface index
 *
 * @param[in]    ifindex    Index of the port
 *
 * @return struct port_hdl
 *
 **/
struct port_hdl *tsn_port_open_by_index(unsigned int ifindex);

/**
 * @ingroup tsn_api
 * @brief Returns the name of port/bridge for a given port_hdl
 *
 * @param[in]    port    Handler of the port
 *
 * @return char
 *
 **/
char *tsn_get_name_by_hdl(struct port_hdl *port);

/**
 * @ingroup tsn_api
 * @brief Returns the name of port/bridge for a given port ref id
 *        The user is responsible for deallocating the memory after getting the port name
 *
 * @param[in]    bridge    Handler of the bridge
 * @param[in]    port_id   port ref inside bridge
 *
 * @return char
 *
 **/
char *tsn_get_name_by_id(struct port_hdl *bridge, uint32_t port_id);

/**
 * @ingroup tsn_api
 * @brief Returns the port ref id inside bridge for a given name of port/bridge
 *        If there is no port with the given name the function returns max uint32_t value.
 *
 * @param[in]    bridge      Handler of the bridge
 * @param[in]    port_name   name of port
 *
 * @return uint32_t
 *
 **/
uint32_t tsn_get_id_by_name(struct port_hdl *bridge, char *port_name);

/**
 * @ingroup tsn_api
 * @brief Free the given port handler
 *
 * @param[in]    phdl    Handler to be freed
 *
 **/
void tsn_port_close(struct port_hdl *phdl);

/**
 * @ingroup tsn_api
 * @brief Returns a pointer to the list of port Id's of the bridge
 *        If there occurs any problem during list creation, the function
 *        returns a NULL pointer and 0 for the number or ports.
 *        The user is responsible for deallocating the memory after getting the
 *        list of port id's
 *
 * @param[in]    bridge Handler of the bridge
 * @param[out]   num_ports   number of ports of the bridge
 *
 * @return uint32_t*
 *
 **/
uint32_t *tsn_get_list_port_ids(struct port_hdl *bridge, uint32_t *num_ports);

/**
 * @ingroup tsn_api
 * @brief Check if feature is supported on board
 *
 * @param[in]    phdl           Handler to be freed
 * @param[in]    feat_req       Feature to be queried
 *
 * @return bool
 * @retval false                  Feature not supported
 * @retval true                   Feature is supported
 **/

bool tsn_is_feat_supported(struct port_hdl *phdl, enum tsn_feat feat_req);

#endif /*TSN_H_*/
