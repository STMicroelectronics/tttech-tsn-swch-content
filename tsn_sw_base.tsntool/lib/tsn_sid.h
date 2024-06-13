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

#ifndef TSN_SID_H_
#define TSN_SID_H_

/**
 * @ingroup tsn_sid_api
 * @brief Delete entry from the Stream Identification database
 *
 * @param[in]    brdev        Bridge handler
 * @param[in]    ord          Ordinal number to be erased
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR See tsn_return_value
 *
 **/
tsn_return_value tsn_sid_delete_entry(struct port_hdl *brdev, uint16_t ord);

/**
 * @ingroup tsn_sid_api
 * @brief Get the maximum supported stream handle in Stream Identification
 *        database
 *
 * @param[in]    brdev        Bridge handler
 * @param[out]   max_sup      Retrieved maximum supported stream handle
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR   See tsn_return_value
 *
 **/
tsn_return_value tsn_sid_get_max_sup(struct port_hdl *brdev, uint32_t *max_sup);

/**
 * @ingroup tsn_sid_api
 * @brief Get the number of streams currently added to Stream Identification
 *        database
 *
 * @param[in]    brdev        Bridge handler
 * @param[out]   cnt          Retrieved number of supported streams
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR   See tsn_return_value
 *
 **/
tsn_return_value tsn_sid_get_current_cnt(struct port_hdl *brdev, uint32_t *cnt);

/**
 * @ingroup tsn_sid_api
 * @brief Get the maximum (theoretical) number of entries they can be stored
 *        in the Stream Identification database
 *
 * @param[in]    brdev        Bridge handler
 * @param[out]   len          Retrieved number of supported entries
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR   See tsn_return_value
 *
 **/
tsn_return_value tsn_sid_get_tab_length(struct port_hdl *brdev, uint32_t *len);

/**
 * @ingroup tsn_sid_api
 * @brief Check if the requested stream identification type is supported
 *
 * @param[in]    brdev        Bridge handler
 * @param[in]    id_type      Requested identification type
 * @param[out]   supported    Retrieved result
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR   See tsn_return_value
 *
 **/
tsn_return_value tsn_sid_get_ident_type(struct port_hdl *brdev,
                                        sid_ident_type id_type,
                                        bool *supported);

/**
 * @ingroup tsn_sid_api
 * @brief Get the list (port mask) of ports they support Stream Identification
 * (bit0 -> port0, bit1 -> port 1 etc)
 *
 * @param[in]    brdev        Bridge handler
 * @param[out]   res          Port mask (list of ports)
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR   See tsn_return_value
 *
 **/
tsn_return_value tsn_sid_get_supported_ports(struct port_hdl *brdev,
                                             uint16_t *port_mask);

/**
 * @ingroup tsn_sid_api
 * @brief Get the Value of Per Port Per Stream counter for the input packets
 *
 * @param[in]    brdev        Bridge handler
 * @param[out]   str_hdl      Stream Handle for which to the counter value is
 *                            retrieved
 * @param[out]   cnt_val      Pointer to retrieved counter value
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR   See tsn_return_value
 *
 **/
tsn_return_value tsn_sid_get_input_cps_cnt(struct port_hdl *brdev,
                                           uint32_t str_hdl,
                                           uint32_t *cnt_val);

/**
 * @ingroup tsn_sid_api
 * @brief Get the Value of Per Port Per Stream counter for the output packets
 *
 * @param[in]    brdev        Bridge handler
 * @param[out]   str_hdl      Stream Handle for which to the counter value is
 *                            retrieved
 * @param[out]   cnt_val      Pointer to retrieved counter value
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR   See tsn_return_value
 *
 **/
tsn_return_value tsn_sid_get_output_cps_cnt(struct port_hdl *brdev,
                                            uint32_t str_hdl,
                                            uint32_t *cnt_val);

/**
 * @ingroup tsn_sid_api
 * @brief Create new entry in Stream Identification database and write
 * Stream Identification parameters to the added entry.
 * Parameters are defined in IEEE 802.1CB 9.1.1.6, 9.1.1.7.
 * Section Stream Identification function is not applied until all of SID
 * parameters are set correctly (IEEE 802.1CB, Section 9)
 * @note
 *
 * @param[in]    brdev        Bridge handler
 * @param[in]    ord          Ordinal number
 * @param[in]    sid          Pointer containing Stream Identification data
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR   See tsn_return_value
 *
 **/
tsn_return_value tsn_sid_add_ident(struct port_hdl *brdev, unsigned int ord,
                                   const struct ident_params *entry);

/**
 * @ingroup tsn_sid_api
 * @brief Write Port-Position list to the already added entry.
 * Parameters are defined in IEEE 802.1CB 9.1.1.2 - 9.1.1.5.
 * Section Stream Identification function is not applied until all of SID
 * parameters are set correctly (IEEE 802.1CB, Section 9)
 * @note
 *
 * @param[in]    brdev        Bridge handler
 * @param[in]    ord          Ordinal number
 * @param[in]    sid          Pointer containing Port-Position data (one entry
 *                            per function call)
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR   See tsn_return_value
 *
 **/
tsn_return_value tsn_sid_add_port_pos(struct port_hdl *brdev, unsigned int ord,
                                      const struct port_pos *sid);

/**
 * @ingroup tsn_sid_api
 * @brief Delete Port-Position list from the existing entry.
 * Parameters are defined in IEEE 802.1CB 9.1.1.2 - 9.1.1.5.
 * @note
 *
 * @param[in]    brdev        Bridge handler
 * @param[in]    ord          Ordinal number
 * @param[in]    sid          Pointer containing Port-Position data (one entry
 *                            per function call)
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR   See tsn_return_value
 *
 **/
tsn_return_value tsn_sid_del_port_pos(struct port_hdl *brdev, unsigned int ord,
                                      const struct port_pos *sid);

/**
 * @ingroup tsn_sid_api
 * @brief Assign a stream handle to an ordinal number.
 * @note
 *
 * @param[in]    brdev        Bridge handler
 * @param[in]    ord          Ordinal number
 * @param[in]    strhdl       Stream handle
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR   See tsn_return_value
 *
 **/
tsn_return_value tsn_sid_set_strhdl(struct port_hdl *brdev, unsigned int ord,
                                    uint16_t strhdl);

/**
 * @ingroup tsn_sid_api
 * @brief Get Stream Identification parameters of the entry defined by Stream
 * Handle number. Parameters are defined in IEEE 802.1CB 9.1.1.6, 9.1.1.7.
 *
 * @param[in]    brdev        Bridge handler
 * @param[in]    ord          Ordinal number for which the data is retrieved
 * @param[out]   entry        Pointer to the structure containing Stream
 *                            Identification parameters
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR   See tsn_return_value
 *
 **/
tsn_return_value tsn_sid_get_ident_params(struct port_hdl *brdev,
                                          unsigned int ord,
                                          struct ident_params *entry);

/**
 * @ingroup tsn_sid_api
 * @brief Get the list of all Stream Handle numbers currently added to the
 * Stream Identification function
 *
 * @param[in]    brdev        Bridge handler
 * @param[in ]   cnt          Number of streams currently added to the function
 * @param[out]   ord          Pointer to the array containing retrieved ordinal
 *                            numbers
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR   See tsn_return_value
 *
 **/
tsn_return_value tsn_sid_get_entries_list(struct port_hdl *brdev, uint32_t cnt,
                                          uint16_t *ord);

/**
 * @ingroup tsn_sid_internal
 * @brief Get the data about current setting of Port-Position. Function can
 * retrieve one (out of four) positions. Returned value is the port mask which
 * denotes on which ports is the Stream Identification function installed
 * (bit0 -> port0, bit1 -> port 1 etc)
 *
 * @param[in]    brdev        Bridge handler
 * @param[in]    ord          Ordinal number for which function is called
 * @param[in]    pos          Requested position
 * @param[out]   res          Port mask for the requested position
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR   See tsn_return_value
 *
 **/
tsn_return_value tsn_sid_get_port_pos(struct port_hdl *brdev, unsigned int ord,
                                      sid_pos pos, uint16_t *res);

/**
 * @ingroup tsn_sid_internal
 * @brief Get the information In - Facing - Input position for Stream
 * Identification function defined with Stream Handle. Result is the port mask
 * which tells on which ports is the function with this position installed. Bit
 * 0 denotes port 0, bit 1 port 1 etc.
 * @note
 *
 * @param[in]    br           Bridge handler
 * @param[in]    ord          Ordinal number for which data is retrieved
 * @param[in]    res          Port mask for the requested position
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR   See tsn_return_value
 *
 **/
tsn_return_value tsn_sid_get_infacin(struct port_hdl *br, uint16_t ord,
                                     uint16_t *res);

/**
 * @ingroup tsn_sid_internal
 * @brief Get the information In - Facing - Output position for Stream
 * Identification function defined with Stream Handle. Result is the port mask
 * which tells on which ports is the function with this position installed. Bit
 * 0 denotes port 0, bit 1 port 1 etc.
 * @note
 *
 * @param[in]    br           Bridge handler
 * @param[in]    ord          Ordinal number for which data is retrieved
 * @param[in]    res          Port mask for the in-fac-out position
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR   See tsn_return_value
 *
 **/
tsn_return_value tsn_sid_get_infacout(struct port_hdl *br, uint16_t ord,
                                      uint16_t *res);

/**
 * @ingroup tsn_sid_internal
 * @brief Get the information Out - Facing - Input position for Stream
 * Identification function defined with Stream Handle. Result is the port mask
 * which tells on which ports is the function with this position installed. Bit
 * 0 denotes port 0, bit 1 port 1 etc.
 * @note
 *
 * @param[in]    br           Bridge handler
 * @param[in]    ord          Ordinal number for which data is retrieved
 * @param[in]    res          Port mask for the out-fac-in position
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR   See tsn_return_value
 *
 **/
tsn_return_value tsn_sid_get_outfacin(struct port_hdl *br, uint16_t ord,
                                      uint16_t *res);

/**
 * @ingroup tsn_sid_internal
 * @brief Get the information Out - Facing - Output position for Stream
 * Identification function defined with Stream Handle. Result is the port mask
 * which tells on which ports is the function with this position installed. Bit
 * 0 denotes port 0, bit 1 port 1 etc.
 * @note
 *
 * @param[in]    br           Bridge handler
 * @param[in]    ord          Ordinal number for which data is retrieved
 * @param[in]    res          Port mask for the out-fac-out position
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR   See tsn_return_value
 *
 **/
tsn_return_value tsn_sid_get_outfacout(struct port_hdl *br, uint16_t ord,
                                       uint16_t *res);

const char *tsn_sid_get_known_ernno_msg(tsn_return_value return_code);

#endif /*TSN_SID_H_*/
