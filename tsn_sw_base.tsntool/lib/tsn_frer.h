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

#ifndef TSN_FRER_H_
#define TSN_FRER_H_

/**
 * @ingroup tsn_frer_api
 * @brief Create new Sequence Generation entry, where Sequence Generation
 * function is chosen from the Sequence Generation table according to index
 * given as a parameter of this function.
 *
 * @param[in]	  brdev		 Bridge handler
 * @param[in]	  fnidx		 Sequence Generation function index
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR    See tsn_return_value
 *
 **/
tsn_return_value tsn_frer_seqgen_create(struct port_hdl *brdev, uint16_t fnidx);

/**
 * @ingroup tsn_frer_api
 * @brief Add stream handle to the already created Sequence Generation function.
 * Stream handle can be added only to the already existing Sequence Generation
 * function.
 *
 * @param[in]	  brdev		 Bridge handler
 * @param[in]	  idx		 Index of the Sequence Generation function
 * @param[in]	  strhdl	 Stream handle to be added to the function
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR    See tsn_return_value
 *
 **/
tsn_return_value tsn_frer_seqgen_add_strhdl(struct port_hdl *brdev,
                                            unsigned int idx,
                                            const uint16_t strhdl);
/**
 * @ingroup tsn_frer_api
 * @brief Assign direction to the already created Sequence Generation function.
 * Direction can be assigned only to the already existing Sequence Generation
 * function.
 *
 * @param[in]	  brdev		 Bridge handler
 * @param[in]	  idx		 Index of the Sequence Generation function
 * @param[in]	  dir		 Direction to be assigned to the function
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR    See tsn_return_value
 *
 **/
tsn_return_value tsn_frer_seqgen_add_dir(struct port_hdl *brdev,
                                         unsigned int idx,
                                         const uint8_t dir);

/**
 * @ingroup tsn_frer_api
 * @brief Delete Stream handle from the list of stream handles belonging to
 * certain Sequence Generation function.
 *
 * @param[in]	  brdev		 Bridge handler
 * @param[in]	  strhdl	 Stream handle to be removed
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR    See tsn_return_value
 *
 **/
tsn_return_value tsn_frer_seqgen_del_strhdl(struct port_hdl *brdev,
                                            uint16_t strhdl);

/**
 * @ingroup tsn_frer_api
 * @brief Delete Sequence Generation function
 *
 * @param[in]	  brdev		 Bridge handler
 * @param[in]	  fnidx		 Sequence Generation function to be deleted
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR    See tsn_return_value
 *
 **/
tsn_return_value tsn_frer_seqgen_del_fn(struct port_hdl *brdev, uint16_t fnidx);

/**
 * @ingroup tsn_frer_api
 * @brief Get count of currently added Sequence Generation functions on the
 * device.
 *
 * @param[in]	  brdev		 Bridge handler
 * @param[out]	  cnt		 Pointer to retrieved count of Sequence Generation
 * 				 functions
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR    See tsn_return_value
 *
 **/
tsn_return_value tsn_frer_seqgen_fn_cnt(struct port_hdl *brdev, uint16_t *cnt);

/**
 * @ingroup tsn_frer_api
 * @brief Get the list of all Sequence Generation functions currently added to
 * the device. Function will retrieve the list of function indexes.
 *
 * @param[in]	  brdev		 Bridge handler
 * @param[in]	  cnt		 Count of currently added functions
 * @param[out]	  fn_list	 Pointer to retrieved list of Sequence Generation
 * 				 functions
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR    See tsn_return_value
 *
 **/
tsn_return_value tsn_frer_seqgen_list_fn(struct port_hdl *brdev, uint32_t cnt,
                                         uint16_t *fn_list);
/**
 * @ingroup tsn_frer_api
 * @brief Get the Sequence Generation function to which the Stream handle is
 * assigned
 *
 * @param[in]	  brdev		 Bridge handler
 * @param[in]	  idx		 Index of the Stream handle of interest
 * @param[out]	  fn		 Pointer to retrieved Sequence Generation function
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR    See tsn_return_value
 *
 **/
tsn_return_value tsn_frer_seqgen_sid2fn(struct port_hdl *brdev,
                                        unsigned int idx,
                                        uint16_t *fn);

/**
 * @ingroup tsn_frer_api
 * @brief Get the implemented direction of the Sequence Generation function
 *
 * @param[in]	  brdev		 Bridge handler
 * @param[in]	  idx		 Index of the Sequence Generation function
 * @param[out]	  dir		 Pointer to retrieved value of the direction of
 * 				 requested Sequence generation function
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR    See tsn_return_value
 *
 **/
tsn_return_value tsn_frer_seqgen_fn2dir(struct port_hdl *brdev,
                                        unsigned int idx,
                                        uint8_t *dir);

/**
 * @ingroup tsn_frer_api
 * @brief Get the list of stream handles assigned to certain Sequence Generation
 * function.
 *
 * @param[in]	  brdev		 Bridge handler
 * @param[in]	  idx		 Index of the requested Sequence Generation function
 * @param[out]	  strlist	 Pointer to retrieved list of stream handles
 * @param[in]	  cnt		 Maximum number of streams which can be assigned to
 * 				 single Sequence Generation function
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR    See tsn_return_value
 *
 **/
tsn_return_value tsn_frer_seqgen_fn2sid(struct port_hdl *brdev,
                                        unsigned int idx,
                                        uint16_t *strlist, uint32_t cnt);

/**
 * @ingroup tsn_frer_api
 * @brief Create new Sequence encode-decode entry. This entry is defined by
 * port and direction. (See 802.1cb, 10.5 Sequence identification table)
 *
 * @param[in]	  port		 Port handler
 * @param[in]	  dir		 Direction indicating if the function is placed on
 * 				 out-facing(1) or in-facing(0) side of port
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR    See tsn_return_value
 *
 **/
tsn_return_value tsn_frer_seqenc_create(struct port_hdl *port, uint8_t dir);

/**
 * @ingroup tsn_frer_api
 * @brief Set the value which indicates if the Sequence Encode-Decode entry is
 * passive(0) or active(1). Passive means that it is used only to decode input
 * packets, while active means that it is used for both recognizing input and
 * encoding output packets.
 *
 * @param[in]	  port		 Port handler
 * @param[in]	  active	 Value of the Active parameter
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR    See tsn_return_value
 *
 **/
tsn_return_value tsn_frer_seqenc_setactive(struct port_hdl *port,
                                           uint8_t active);

/**
 * @ingroup tsn_frer_api
 * @brief Set the value which indicates the type of the sequence encode
 * function: Unknown(0), R-Tag(1), HSR-Tag(2), PRP-Trailer(3).
 *
 * @param[in]	  port		 Port handler
 * @param[in]	  type	 	 Value of the Type parameter
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR    See tsn_return_value
 *
 **/
tsn_return_value tsn_frer_seqenc_settype(struct port_hdl *port, uint8_t type);

/**
 * @ingroup tsn_frer_api
 * @brief Retrieve the information whether this Sequence Encode-Decode entry is
 * passive or active.
 *
 * @param[in]	  port		 Port handler
 * @param[in]	  active	 Pointer to the Active parameter
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR    See tsn_return_value
 *
 **/
tsn_return_value tsn_frer_seqenc_getactive(struct port_hdl *port,
                                           uint32_t *active);

/**
 * @ingroup tsn_frer_api
 * @brief Retrieve the information which type this Sequence Encode-Decode entry
 * has.
 *
 * @param[in]	  port		 Port handler
 * @param[in]	  type	 	 Pointer to the Type parameter
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR    See tsn_return_value
 *
 **/
tsn_return_value tsn_frer_seqenc_gettype(struct port_hdl *port, uint32_t *type);

/**
 * @ingroup tsn_frer_api
 * @brief Retrieve the information which direction is set for this Sequence Encode-Decode entry
 *
 * @param[in]	  port		 Port handler
 * @param[in]	  direction	 Pointer to the direction parameter
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR    See tsn_return_value
 *
 **/
tsn_return_value tsn_frer_seqenc_getdir(struct port_hdl *port,
                                        uint8_t *direction);

/**
 * @ingroup tsn_frer_api
 * @brief Add stream to the list of stream handles which correspond to Sequence
 * Encode-Decode entry. List of streams has common type of encapsulation (See
 * IEEE 802.1cb 10.5.1.5)
 *
 * @param[in]	  brdev		 Bridge handler
 * @param[in]	  strhdl	 Stream handle to be added
 * @param[in]	  type   	 Type of encapsulation for the stream handle
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR    See tsn_return_value
 *
 **/
tsn_return_value tsn_frer_seqenc_add_str(struct port_hdl *brdev,
                                         uint16_t strhdl);

/**
 * @ingroup tsn_frer_api
 * @brief Delete all assigned data (i.e. Stream list) for a certain port.
 *
 * @param[in]	  port		 Port handler for which Sequence Encode-Decode data
 * 				 is erased.
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR    See tsn_return_value
 *
 **/
tsn_return_value tsn_frer_seqenc_delport(struct port_hdl *port);

/**
 * @ingroup tsn_frer_api
 * @brief Delete stream from the Stream list from a certain port.
 *
 * @param[in]	  port		 Port handler
 * @param[in]	  strhdl	 Stream handle to be removed
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR    See tsn_return_value
 *
 **/
tsn_return_value tsn_frer_seqenc_delstr(struct port_hdl *port, uint16_t strhdl);

/**
 * @ingroup tsn_frer_api
 * @brief Show ports to which certain Stream handle is assigned. List of ports
 * is deducted from the port mask, where bit 1 denotes port 1, bit 2 port 2 etc.
 * If certain Stream handle is assigned to port x, that means that there is
 * Sequence Encode-Decode function for that Stream handle on port x
 *
 * @param[in]	  brdev		 Bridge handler
 * @param[in]	  strhdl	 Stream handle for which information is requested
 * @param[in]	  portmask	 Pointer to the Port mask
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR    See tsn_return_value
 *
 **/
tsn_return_value tsn_frer_seqenc_str2port(struct port_hdl *brdev,
                                          unsigned int strhdl,
                                          uint16_t *portmask);

/**
 * @ingroup tsn_frer_api
 * @brief Show list of streams assigned to certain port. List of shown streams
 * have Sequence Encode-Decode function for this certain port.
 *
 * @param[in]	  port		 Port handler
 * @param[in]	  cnt		 Maximum number of streams which are able to be
 * 			 	 assigned to this port
 * @param[in]	  strlist	 Pointer to the retrieved Stream handle list
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR    See tsn_return_value
 *
 **/
tsn_return_value tsn_frer_seqenc_port2str(struct port_hdl *port, uint32_t cnt,
                            uint16_t *strlist);
/**
 * @ingroup tsn_frer_internal
 * @brief Maximum number of ports on certain bridge device
 *
 * @param[in]	  brdev		 Bridge handler
 * @param[in]	  max_ports	 Pointer to the maximum number of ports
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR    See tsn_return_value
 *
 **/
tsn_return_value tsn_frer_get_max_ports(struct port_hdl *brdev,
                                        uint32_t *max_ports);

/**
 * @ingroup tsn_frer_api
 * @brief Creates an instance of Sequence Recovery function
 *
 * @param[in]	  brdev		 Bridge handler
 * @param[in]	  fnidx		 Sequence Recovery function index
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR    See tsn_return_value
 *
 **/
tsn_return_value tsn_frer_seqrec_create(struct port_hdl *brdev, uint16_t fnidx);

/**
 * @ingroup tsn_frer_api
 * @brief Add stream to the Stream list of the existing Sequence Recovery
 * function
 *
 * @param[in]	  brdev		 Bridge handler
 * @param[in]	  fnidx		 Sequence Recovery function index
 * @param[in]	  strhdl	 Stream Handle to be added
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR    See tsn_return_value
 *
 **/
tsn_return_value tsn_frer_seqrec_addstr(struct port_hdl *brdev, uint16_t fnidx,
                                        const uint16_t sthdlr);

/**
 * @ingroup tsn_frer_api
 * @brief Add port to the Port list of the existing Sequence Recovery function
 *
 * @param[in]	  port	 	 Port handle
 * @param[in]	  fnidx		 Sequence Recovery function index
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR    See tsn_return_value
 *
 **/
tsn_return_value tsn_frer_seqrec_addport(struct port_hdl *port, uint16_t fnidx);

/**
 * @ingroup tsn_frer_api
 * @brief Set parameter Take No Sequence for the existing Sequence Recovery
 * function
 *
 * @param[in]	  brdev		 Bridge handler
 * @param[in]	  fnidx		 Sequence Recovery function index
 * @param[in]	  tns		 Value of parameter Take No Sequence
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR    See tsn_return_value
 *
 **/
tsn_return_value tsn_frer_seqrec_set_takenoseq(struct port_hdl *brdev,
                                               uint16_t fnidx,
                                               const uint8_t tns);

/**
 * @ingroup tsn_frer_api
 * @brief Set parameter Individual recovery  for the existing Sequence Recovery
 * function
 *
 * @param[in]	  brdev		 Bridge handler
 * @param[in]	  fnidx		 Sequence Recovery function index
 * @param[in]	  indrcv	 Value of parameter Individual Recovery
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR    See tsn_return_value
 *
 **/
tsn_return_value tsn_frer_seqrec_set_indrcv(struct port_hdl *brdev,
                                            uint16_t fnidx,
                                            const uint8_t indrcv);

/**
 * @ingroup tsn_frer_api
 * @brief Set the value of Latent Error for the existing Sequence Recovery
 * function
 *
 * @param[in]	  brdev		 Bridge handler
 * @param[in]	  fnidx		 Sequence Recovery function index
 * @param[in]	  laterr	 Value of parameter Latent Error
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR    See tsn_return_value
 *
 **/
tsn_return_value tsn_frer_seqrec_set_laterr(struct port_hdl *brdev,
                                            uint16_t fnidx,
                                            const uint8_t laterr);

/**
 * @ingroup tsn_frer_api
 * @brief Choose the algorithm for the existing Sequence Recovery function
 *
 * @param[in]	  brdev		 Bridge handler
 * @param[in]	  fnidx		 Sequence Recovery function index
 * @param[in]	  alg		 Chosen Algorithm
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR    See tsn_return_value
 *
 **/
tsn_return_value tsn_frer_seqrec_set_alg(struct port_hdl *brdev, uint16_t fnidx,
                                         const uint32_t alg);

/**
 * @ingroup tsn_frer_api
 * @brief Set the value of Direction for the existing Sequence Recovery
 * function
 *
 * @param[in]	  brdev		 Bridge handler
 * @param[in]	  fnidx		 Sequence Recovery function index
 * @param[in]	  dir	 	 Value of parameter direction
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR    See tsn_return_value
 *
 **/
tsn_return_value tsn_frer_seqrec_set_dir(struct port_hdl *brdev, uint16_t fnidx,
                                         const uint8_t dir);

/**
 * @ingroup tsn_frer_api
 * @brief Set the value of History Length for the existing Sequence Recovery
 * function
 *
 * @param[in]	  brdev		 Bridge handler
 * @param[in]	  fnidx		 Sequence Recovery function index
 * @param[in]	  histlen	 Value of parameter History Length
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR    See tsn_return_value
 *
 **/
tsn_return_value tsn_frer_seqrec_set_histlen(struct port_hdl *brdev,
                                             uint16_t fnidx,
                                             const uint8_t histlen);

/**
 * @ingroup tsn_frer_api
 * @brief Set the value of the timeout period in milliseconds for the
 * Recovery Timeout to happen (see IEEE 802.1cb(2017) 10.4.1.7)
 *
 * @param[in]	  brdev		 Bridge handler
 * @param[in]	  fnidx		 Sequence Recovery function index
 * @param[in]	  rsttime	 Value of the Recovery Timeout
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR    See tsn_return_value
 *
 **/
tsn_return_value tsn_frer_seqrec_set_recreset(struct port_hdl *brdev,
                                              uint16_t fnidx,
                                              const uint16_t rsttime);

/**
 * @ingroup tsn_frer_api
 * @brief Delete port from the Port List of the Sequence Recovery function
 *
 * @param[in]	  port		 Port handle
 * @param[in]	  fnidx		 Sequence Recovery function index
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR    See tsn_return_value
 *
 **/
tsn_return_value tsn_frer_seqrec_delport(struct port_hdl *port, uint16_t fnidx);

/**
 * @ingroup tsn_frer_api
 * @brief Delete stream from the Stream list of the Sequence Recovery function
 *
 * @param[in]	  brdev		 Bridge handler
 * @param[in]	  fnidx		 Sequence Recovery function index
 * @param[in]	  strhdl	 Stream Handle which is to be removed
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR    See tsn_return_value
 *
 **/
tsn_return_value tsn_frer_seqrec_delstr(struct port_hdl *brdev, uint16_t fnidx,
                                        const uint16_t sthdlr);

/**
 * @ingroup tsn_frer_api
 * @brief Delete Sequence Recovery function data
 *
 * @param[in]	  brdev		 Bridge handler
 * @param[in]	  fnidx		 Sequence Recovery function index
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR    See tsn_return_value
 *
 **/
tsn_return_value tsn_frer_seqrec_delfn(struct port_hdl *brdev, uint16_t fnidx);

/**
 * @ingroup tsn_frer_api
 * @brief Get the list of Stream Handles which belong to the Stream List of
 * certain Sequence Recovery function
 *
 * @param[in]	  brdev		 Bridge handler
 * @param[in]	  fnidx		 Sequence Recovery function index
 * @param[out]	  strlist	 Pointer to the Stream List
 * @param[in]	  cnt		 Maximum number of Stream Handles in the List
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR    See tsn_return_value
 *
 **/
tsn_return_value tsn_frer_seqrec_fn2sid(struct port_hdl *brdev,
                                        unsigned int fnidx,
                                        uint16_t *strlist, uint32_t cnt);

/**
 * @ingroup tsn_frer_api
 * @brief Get the value of the direction set for certain Sequence Recovery
 * function
 *
 * @param[in]	  brdev		 Bridge handler
 * @param[in]	  fnidx		 Sequence Recovery function index
 * @param[out]	  dir		 Pointer to the retrieved value of the parameter
 * 							 direction
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR    See tsn_return_value
 *
 **/
tsn_return_value tsn_frer_seqrec_get_dir(struct port_hdl *brdev, uint16_t fnidx,
                                         uint8_t *dir);

/**
 * @ingroup tsn_frer_api
 * @brief Get the value of the Algorithm set for certain Sequence Recovery
 * function
 *
 * @param[in]	  brdev		 Bridge handler
 * @param[in]	  fnidx		 Sequence Recovery function index
 * @param[out]	  alg		 Pointer to the retrieved value of the parameter
 * 							 algorithm
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR    See tsn_return_value
 *
 **/
tsn_return_value tsn_frer_seqrec_get_alg(struct port_hdl *brdev,
                                         unsigned int fnidx,
                                         uint32_t *alg);

/**
 * @ingroup tsn_frer_api
 * @brief Get the value of the History Length set for certain Sequence Recovery
 * function
 *
 * @param[in]	  brdev		 Bridge handler
 * @param[in]	  fnidx		 Sequence Recovery function index
 * @param[out]	  histlen	 Pointer to the retrieved value of the parameter
 * 				 History length
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR    See tsn_return_value
 *
 **/
tsn_return_value tsn_frer_seqrec_get_histlen(struct port_hdl *brdev,
                                             unsigned int fnidx,
                                             uint8_t *histlen);

/**
 * @ingroup tsn_frer_api
 * @brief Get the value of the Recovery Reset set for certain Sequence Recovery
 * function
 *
 * @param[in]	  brdev		 Bridge handler
 * @param[in]	  fnidx		 Sequence Recovery function index
 * @param[out]	  rsttime	 Pointer to the retrieved value of the parameter
 * 				 Recovery Reset Timeout
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR    See tsn_return_value
 *
 **/
tsn_return_value tsn_frer_seqrec_get_recreset(struct port_hdl *brdev,
                                              unsigned int fnidx,
                                              uint16_t *rsttime);

/**
 * @ingroup tsn_frer_api
 * @brief Get the value of the Take No Sequence set for certain Sequence
 * Recovery function
 *
 * @param[in]	  brdev		 Bridge handler
 * @param[in]	  fnidx		 Sequence Recovery function index
 * @param[out]	  tns		 Pointer to the retrieved value of the parameter
 * 				 Take No Sequence
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR    See tsn_return_value
 *
 **/
tsn_return_value tsn_frer_seqrec_get_takenoseq(struct port_hdl *brdev,
                                               unsigned int fnidx,
                                               uint8_t *tns);

/**
 * @ingroup tsn_frer_api
 * @brief Get the value of the Individual Recovery set for certain Sequence
 * Recovery function
 *
 * @param[in]	  brdev		 Bridge handler
 * @param[in]	  fnidx		 Sequence Recovery function index
 * @param[out]	  indrcv	 Pointer to the retrieved value of the parameter
 * 				 Individual Recovery
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR    See tsn_return_value
 *
 **/
tsn_return_value tsn_frer_seqrec_get_indrcv(struct port_hdl *brdev,
                                            unsigned int fnidx,
                                            uint8_t *indrcv);

/**
 * @ingroup tsn_frer_api
 * @brief Get the value of the Latent Error set for certain Sequence Recovery
 * function
 *
 * @param[in]	  brdev		 Bridge handler
 * @param[in]	  fnidx		 Sequence Recovery function index
 * @param[out]	  laterr	 Pointer to the retrieved value of the parameter
 * 				 Latent Error
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR    See tsn_return_value
 *
 **/
tsn_return_value tsn_frer_seqrec_get_laterr(struct port_hdl *brdev,
                                            unsigned int fnidx,
                                            uint8_t *laterr);

/**
 * @ingroup tsn_frer_api
 * @brief Get the list of the ports on which the Sequence Recovery function is
 * installed. Function will retrieve pointer value which is a port mask, where
 * bit 1 corresponds to port 1, bit 2 - port 2 etc. In case bit is set to 1,
 * Sequence Recovery function is installed on that port.
 *
 * @param[in]	  brdev		 Bridge handler
 * @param[in]	  fnidx		 Sequence Recovery function index
 * @param[out]    port		 Pointer to the retrieved port mask
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR    See tsn_return_value
 *
 **/
tsn_return_value tsn_frer_seqrec_get_ptmask(struct port_hdl *brdev,
                                            unsigned int fnidx,
                                            uint16_t *port);

/**
 * @ingroup tsn_frer_api
 * @brief Get the number of Sequence Recovery function installed on bridge
 *
 * @param[in]	  brdev		 Bridge handler
 * @param[out]	  cnt		 Pointer to the retrieved function count
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR    See tsn_return_value
 *
 **/
tsn_return_value tsn_frer_seqrec_fn_cnt(struct port_hdl *brdev, uint16_t *cnt);

/**
 * @ingroup tsn_frer_api
 * @brief Get the list of all Sequence Recovery function indexes installed
 * on the bridge device. Each Sequence Recovery function is identified by the
 * function index
 *
 * @param[in]	  brdev		 Bridge handler
 * @param[in]	  cnt		 Number of the Sequence Recovery functions
 * 				 currently installed
 * @param[in]	  fn_list	 Pointer to the retrieved function list
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR    See tsn_return_value
 *
 **/
tsn_return_value tsn_frer_seqrec_list_fn(struct port_hdl *brdev, uint32_t cnt,
                                         uint16_t *fn_list);

/**
 * @ingroup tsn_frer_api
 * @brief Counter which counts how many packets with 'out of order' sequence
 * number have been received. Out of order means that packet's sequence number
 * is not larger by one compared to previous packet
 *
 * @param[in]	  brdev		 Bridge handler
 * @param[in]	  fnidx		 Sequence Recovery function index
 * @param[out]	  cnt 		 Pointer to the retrieved counter value
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR    See tsn_return_value
 *
 **/
tsn_return_value tsn_frer_seqrec_cnt_outoforder(struct port_hdl *brdev,
                                                unsigned int fnidx,
                                                uint32_t *cnt);

/**
 * @ingroup tsn_frer_api
 * @brief Counter counting how many packets which have no sequence_number
 * parameter have been received
 *
 * @param[in]	  port		 Port handle
 * @param[in]	  strhdl	 Stream Handle Number
 * @param[out]	  cnt	 	 Pointer to the retrieved counter value
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR    See tsn_return_value
 *
 **/
tsn_return_value tsn_frer_seqrec_cnt_tagless(struct port_hdl *port,
                                             unsigned int strhdl,
                                             uint32_t *cnt);

/**
 * @ingroup tsn_frer_api
 * @brief Counter counting how many packets passed up the stack by the
 * Vector or Match recovery algorithm
 *
 * @param[in]	  port		 Port handle
 * @param[in]	  strhdl	 Stream Handle Number
 * @param[out]	  cnt		 Pointer to the retrieved counter value
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR    See tsn_return_value
 *
 **/
tsn_return_value tsn_frer_seqrec_cnt_passed(struct port_hdl *port,
                                            unsigned int strhdl,
                                            uint32_t *cnt);

/**
 * @ingroup tsn_frer_api
 * @brief Counter counting how many packets are discarted due to duplicate
 * sequence number by the Vector or Match recovery algorithm
 *
 * @param[in]	  port		 Port handle
 * @param[in]	  strhdl	 Stream Handle Number
 * @param[out]	  cnt	 	 Pointer to the retrieved counter value
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR    See tsn_return_value
 *
 **/
tsn_return_value tsn_frer_seqrec_cnt_discarded(struct port_hdl *port,
                                               unsigned int strhdl,
                                               uint32_t *cnt);

/**
 * @ingroup tsn_frer_api
 * @brief Counter counting how many packets have been discarded because their
 * Sequence Number was bigger than value of the History Length parameter. This
 * counter is only valid for Vector Recovery Algorithm.
 *
 * @param[in]	  port		 Port handle
 * @param[in]	  strhdl	 Stream Handle Number
 * @param[out]	  cnt	 	 Pointer to the retrieved counter value
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR    See tsn_return_value
 *
 **/
tsn_return_value tsn_frer_seqrec_cnt_rogue(struct port_hdl *port,
                                           unsigned int strhdl,
                                           uint32_t *cnt);

/**
 * @ingroup tsn_frer_api
 * @brief Counter counting how many packets are lost because their Sequence
 * Number wasn't received on any ingress port.
 *
 * @param[in]	  brdev		 Bridge handler
 * @param[in]	  strhdl	 Stream Handle Number
 * @param[out]	  cnt	 	 Pointer to the retrieved counter value
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR    See tsn_return_value
 *
 **/
tsn_return_value tsn_frer_seqrec_cnt_lost(struct port_hdl *brdev,
                                          unsigned int strhdl,
                                          uint32_t *cnt);

/**
 * @ingroup tsn_frer_api
 * @brief Counter counting the number of times Sequence Recovery function was
 * reset.
 *
 * @param[in]	  brdev		 Bridge handler
 * @param[in]	  strhdl	 Stream Handle Number
 * @param[out]	  cnt	 	 Pointer to the retrieved counter value
 *
 * @return enum tsn_return_value
 * @retval E_TSN_LIB_NO_ERROR    See tsn_return_value
 *
 **/
tsn_return_value tsn_frer_seqrec_cnt_resets(struct port_hdl *brdev,
                                            unsigned int strhdl,
                                            uint32_t *cnt);

const char *tsn_frer_get_known_ernno_msg(tsn_return_value return_code);

#endif /*TSN_FRER_H_*/
