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

#include "tsn_def.h"

static const char *lib_error_msg[TSN_LIB_ERROR_CODE(E_TSN_LIB_MAX_ERROR)] = {
	[E_TSN_LIB_NO_ERROR]  				      = "Operation performed successfully",
        [TSN_LIB_ERROR_CODE(E_TSN_LIB_NOSUCHFILE)]            = "port/bridge path does not exist or capability is not supported on device",
        [TSN_LIB_ERROR_CODE(E_TSN_LIB_SCAN)]                  = "Failed reading a single value from driver",
        [TSN_LIB_ERROR_CODE(E_TSN_LIB_PRINT)]                 = "Failed writing a single value to driver",
        [TSN_LIB_ERROR_CODE(E_TSN_LIB_RDLIST)]                = "Failed reading a list from driver",
        [TSN_LIB_ERROR_CODE(E_TSN_LIB_WRLIST)]                = "Failed writing a list to driver",
        [TSN_LIB_ERROR_CODE(E_TSN_LIB_RDENTRY)]               = "Failed reading a list entry from driver",
        [TSN_LIB_ERROR_CODE(E_TSN_LIB_WRENTRY)]               = "Failed writing a list entry to driver",
        [TSN_LIB_ERROR_CODE(E_TSN_LIB_FCLOSE)]                = "Failed closing the interface, errno is reported in lower 8 bits for more information",
        [TSN_LIB_ERROR_CODE(E_TSN_LIB_FSCAN)]                 = "Failed reading a single value from driver, errno is reported in lower 8 bits for more information",
        [TSN_LIB_ERROR_CODE(E_TSN_LIB_FPRINT)]                = "Failed writing a single value to driver, errno is reported in lower 8 bits for more information",
        [TSN_LIB_ERROR_CODE(E_TSN_LIB_FRDLIST)]               = "Failed reading a list from driver, errno is reported in lower 8 bits for more information",
        [TSN_LIB_ERROR_CODE(E_TSN_LIB_FRDSUBLIST)]            = "Failed reading a sublist from driver, errno is reported in lower 8 bits for more information",
        [TSN_LIB_ERROR_CODE(E_TSN_LIB_FWRLIST)]               = "Failed writing a list to driver, errno is reported in lower 8 bits for more information",
        [TSN_LIB_ERROR_CODE(E_TSN_LIB_FRDENTRY)]              = "Failed reading a list entry from driver, errno is reported in lower 8 bits for more information",
        [TSN_LIB_ERROR_CODE(E_TSN_LIB_FWRENTRY)]              = "Failed writing a list entry to driver, errno is reported in lower 8 bits for more information",
        [TSN_LIB_ERROR_CODE(E_TSN_LIB_RD_STRUCT)]             = "Function not implemented over structures",
        [TSN_LIB_ERROR_CODE(E_TSN_LIB_PARAMS)]                = "One or more of the input parameters was a null pointer",
        [TSN_LIB_ERROR_CODE(E_TSN_LIB_MAX_MSTI)]              = "There was an error getting the max msti value from driver, errno is reported in lower 8 bits for more information",
        [TSN_LIB_ERROR_CODE(E_TSN_LIB_MAX_MSTI_ERANGE)]       = "Max msti provided/requested is out of range for the device",
        [TSN_LIB_ERROR_CODE(E_TSN_LIB_MAX_FID)]               = "There was an error getting the max number of vlans/fids from driver, errno is reported in lower 8 bits for more information",
        [TSN_LIB_ERROR_CODE(E_TSN_LIB_MAX_FID_ERANGE)]        = "Max fid provided/requested is out of range for the device",
        [TSN_LIB_ERROR_CODE(E_TSN_LIB_MAX_VID)]               = "There was an error getting the biggest vlan id suppported on the device",
        [TSN_LIB_ERROR_CODE(E_TSN_LIB_MAX_VID_ERANGE)]        = "Max vid provided/requested is out of range for the device",
        [TSN_LIB_ERROR_CODE(E_TSN_LIB_ST_OP_ERANGE)]          = "Gate OP is not supported on device",
        [TSN_LIB_ERROR_CODE(E_TSN_LIB_PSFP_GT_OP_ERANGE)]     = "Gate OP is not supported on device",
	[TSN_LIB_ERROR_CODE(E_TSN_LIB_INVALID_SPEED)]         = "Requested Speed not supported",
};

static const uint32_t hash_map_errno_tsn_err[MAX_TSN_MODULES][MAX_TSN_FUNCTIONS][MAX_ERRNO] = {
	[TSN_ST][MAX_SDU][EINVAL]                                  = TSN_ST_RET_INV_ENTRY_POS,
	[TSN_ST][ADMIN_CONTROL_LIST][EINVAL]                       = TSN_ST_RET_INV_ENTRY_POS,
	[TSN_ST][OPER_CONTROL_LIST][EINVAL]                        = TSN_ST_RET_INV_ENTRY_POS,
	[TSN_ST][ADMIN_BASE_TIME][EINVAL]                          = TSN_ST_RET_INV_PARAM,
	[TSN_ST][GATE_ENABLED][EINVAL]                             = TSN_ST_RET_INV_PARAM,
	[TSN_ST][ADMIN_CONTROL_LIST][EFBIG]                        = TSN_ST_RET_LIST_TOO_BIG,
	[TSN_ST][ADMIN_CONTROL_LIST][ENOMEM]                       = TSN_ST_RET_NOMEM,
	[TSN_ST][OPER_CONTROL_LIST][ENOENT]                        = TSN_ST_RET_NOT_ACTIVE,
	[TSN_ST][ADMIN_CONTROL_LIST][ENOENT]                       = TSN_ST_RET_CTLIST_EMPTY,
	[TSN_ST][OPER_CYCLE_TIME][ENOENT]                          = TSN_ST_RET_NOT_ACTIVE,
	[TSN_ST][OPER_CYCLE_TIME_EXTENSION][ENOENT]                = TSN_ST_RET_NOT_ACTIVE,
	[TSN_ST][OPER_BASE_TIME][ENOENT]                           = TSN_ST_RET_NOT_ACTIVE,
	[TSN_ST][ADMIN_CYCLE_TIME][ERANGE]                         = TSN_ST_RET_INV_RANGE,
	[TSN_ST][MAX_SDU][ERANGE]                                  = TSN_ST_RET_INV_RANGE,
	[TSN_ST][CONFIG_CHANGE][EFAULT]                            = TSN_ST_RET_INV_ADMIN_PARAM,
	[TSN_ST][GATE_ENABLED][EIO]                                = TSN_ST_RET_UNKNOWN_LINK_SPEED,
	[TSN_ST][CONFIG_CHANGE][EIO]                               = TSN_ST_RET_UNKNOWN_LINK_SPEED,
	[TSN_ST][OPER_CONTROL_LIST][EBUSY]                         = TSN_ST_RET_HW_ACCESS_ERROR,
	[TSN_ST][GATE_ENABLED][EBUSY]                              = TSN_ST_RET_HW_ACCESS_ERROR,
	[TSN_ST][CONFIG_CHANGE][EBUSY]                             = TSN_ST_RET_HW_ACCESS_ERROR,
	[TSN_PSFP][PSFP_GT_ADMIN_CONTROL_LIST][EINVAL]             = TSN_PSFP_RET_INV_ENTRY_POS,
	[TSN_PSFP][PSFP_GT_OPER_CONTROL_LIST][EINVAL]              = TSN_PSFP_RET_INV_ENTRY_POS,
	[TSN_PSFP][PSFP_GT_ADMIN_BASE_TIME][EINVAL]                = TSN_PSFP_RET_INV_PARAM,
	[TSN_PSFP][PSFP_GT_GATE_ENABLED][EINVAL]                   = TSN_PSFP_RET_INV_PARAM,
	[TSN_PSFP][PSFP_GT_ADMIN_CONTROL_LIST][EFBIG]              = TSN_PSFP_RET_LIST_TOO_BIG,
	[TSN_PSFP][PSFP_GT_ADMIN_CONTROL_LIST][ENOMEM]             = TSN_PSFP_RET_NOMEM,
	[TSN_PSFP][PSFP_GT_OPER_CONTROL_LIST][ENOENT]              = TSN_PSFP_RET_NOT_ACTIVE,
	[TSN_PSFP][PSFP_GT_ADMIN_CONTROL_LIST][ENOENT]             = TSN_PSFP_RET_CTLIST_EMPTY,
	[TSN_PSFP][PSFP_GT_OPER_CYCLE_TIME][ENOENT]                = TSN_PSFP_RET_NOT_ACTIVE,
	[TSN_PSFP][PSFP_GT_OPER_CYCLE_TIME_EXTENSION][ENOENT]      = TSN_PSFP_RET_NOT_ACTIVE,
	[TSN_PSFP][PSFP_GT_OPER_BASE_TIME][ENOENT]                 = TSN_PSFP_RET_NOT_ACTIVE,
	[TSN_PSFP][PSFP_GT_ADMIN_CYCLE_TIME][ERANGE]               = TSN_PSFP_RET_INV_RANGE,
	[TSN_PSFP][PSFP_GT_CONFIG_CHANGE][EFAULT]                  = TSN_PSFP_RET_INV_ADMIN_PARAM,
	[TSN_PSFP][PSFP_GT_OPER_CONTROL_LIST][EBUSY]               = TSN_PSFP_RET_HW_ACCESS_ERROR,
	[TSN_PSFP][PSFP_GT_GATE_ENABLED][EBUSY]                    = TSN_PSFP_RET_HW_ACCESS_ERROR,
	[TSN_PSFP][PSFP_GT_CONFIG_CHANGE][EBUSY]                   = TSN_PSFP_RET_HW_ACCESS_ERROR,
	[TSN_PSFP][PSFP_HIT_TH_LOW][ERANGE]                        = TSN_PSFP_RET_INV_RANGE,
	[TSN_PSFP][PSFP_HIT_TH_HIGH][ERANGE]                       = TSN_PSFP_RET_INV_RANGE,
	[TSN_SID][SID_PORT_POS][ENOENT]                            = TSN_SID_RET_ADD_ORD_NOT_EXIST,
	[TSN_SID][SID_PORT_POS_DEL][ENOENT]                        = TSN_SID_RET_DEL_ORD_NOT_EXIST,
	[TSN_SID][SID_SET_STRHDL][ENOENT]                          = TSN_SID_RET_ENTRY_NOT_EXIST,
	[TSN_SID][SID_IDENT_PARAMS][ENOENT]                        = TSN_SID_RET_ENTRY_NOT_EXIST,
	[TSN_SID][SID_IDENT_PARAMS][EEXIST]                        = TSN_SID_RET_ORD_EXISTS,
	[TSN_SID][SID_IDENT_PARAMS][ENOSPC]                        = TSN_SID_RET_CHECK_DRIVER_LOG,
	[TSN_SID][SID_DELETE][ENOENT]                              = TSN_SID_RET_ENTRY_NOT_EXIST,
	[TSN_SID][SID_INPUT_CPS][EINVAL]                           = TSN_SID_RET_INVALID,
	[TSN_SID][SID_OUTPUT_CPS][EINVAL]                          = TSN_SID_RET_INVALID,
	[TSN_SID][SID_INPUT_CPS][EMSGSIZE]                         = TSN_SID_RET_INVALID,
	[TSN_SID][SID_OUTPUT_CPS][EMSGSIZE]                        = TSN_SID_RET_INVALID,
	[TSN_BRIDGE][PORT_DELAYS_TABLE][EMSGSIZE]                  = TSN_BRIDGE_DUMMY,
};

/* Error mesages for specific modules */

/* Definition of error messages to be reported for st module, it uses the specific return code and the function index to report slightly different messages depending of the function */
static const char *st_error_msg[TSN_ST_RET_MAX_ERROR][MAX_TSN_ST_FUNCTIONS] = {
	[TSN_ST_RET_INV_ENTRY_POS][ADMIN_CONTROL_LIST]          = "Invalid entry position or entry size",
	[TSN_ST_RET_INV_ENTRY_POS][OPER_CONTROL_LIST]           = "Invalid entry position or entry size",
	[TSN_ST_RET_INV_ENTRY_POS][MAX_SDU]                     = "Invalid entry position or entry size",
	[TSN_ST_RET_INV_PARAM][ADMIN_BASE_TIME]                 = "Invalid base time",
	[TSN_ST_RET_INV_PARAM][GATE_ENABLED]                    = "Invalid parameter(s)",
	[TSN_ST_RET_LIST_TOO_BIG][ADMIN_CONTROL_LIST]           = "Too many entries in the list",
	[TSN_ST_RET_NOMEM][ADMIN_CONTROL_LIST]                  = "Cannot allocate kernel memory",
	[TSN_ST_RET_NOT_ACTIVE][OPER_CONTROL_LIST]              = "Not configured",
	[TSN_ST_RET_NOT_ACTIVE][OPER_CYCLE_TIME]                = "n/a",
	[TSN_ST_RET_NOT_ACTIVE][OPER_CYCLE_TIME_EXTENSION]      = "n/a",
	[TSN_ST_RET_NOT_ACTIVE][OPER_BASE_TIME]                 = "n/a",
	[TSN_ST_RET_CTLIST_EMPTY][ADMIN_CONTROL_LIST]           = "Control list is empty",
	[TSN_ST_RET_INV_RANGE][ADMIN_CYCLE_TIME]                = "Cycle time out of range",
	[TSN_ST_RET_INV_RANGE][MAX_SDU]                         = "SDU out of range",
	[TSN_ST_RET_INV_ADMIN_PARAM][CONFIG_CHANGE]             = "Invalid admin parameter(s)",
	[TSN_ST_RET_UNKNOWN_LINK_SPEED][GATE_ENABLED]           = "Unknown link speed",
	[TSN_ST_RET_UNKNOWN_LINK_SPEED][CONFIG_CHANGE]          = "Unknown link speed",
	[TSN_ST_RET_HW_ACCESS_ERROR][OPER_CONTROL_LIST]         = "Internal hardware access error",
	[TSN_ST_RET_HW_ACCESS_ERROR][GATE_ENABLED]              = "Internal hardware access error",
	[TSN_ST_RET_HW_ACCESS_ERROR][CONFIG_CHANGE]             = "Internal hardware access error",
};

/* Definition of error messages to be reported for psfp module, it uses the specific return code and the function index to report slightly different messages depending of the function */
static const char *psfp_error_msg[TSN_PSFP_RET_MAX_ERROR][MAX_TSN_PSFP_FUNCTIONS] = {
	[TSN_PSFP_RET_INV_ENTRY_POS][PSFP_GT_ADMIN_CONTROL_LIST]        = "Invalid entry position or entry size",
	[TSN_PSFP_RET_INV_ENTRY_POS][PSFP_GT_OPER_CONTROL_LIST]         = "Invalid entry position or entry size",
	[TSN_PSFP_RET_INV_PARAM][PSFP_GT_ADMIN_BASE_TIME]               = "Invalid base time",
	[TSN_PSFP_RET_INV_PARAM][PSFP_GT_GATE_ENABLED]                  = "Invalid parameter(s)",
	[TSN_PSFP_RET_LIST_TOO_BIG][PSFP_GT_ADMIN_CONTROL_LIST]         = "Too many entries in the list",
	[TSN_PSFP_RET_NOMEM][PSFP_GT_ADMIN_CONTROL_LIST]                = "Cannot allocate kernel memory",
	[TSN_PSFP_RET_NOT_ACTIVE][PSFP_GT_OPER_CONTROL_LIST]            = "Not configured",
	[TSN_PSFP_RET_NOT_ACTIVE][PSFP_GT_OPER_CYCLE_TIME]              = "n/a",
	[TSN_PSFP_RET_NOT_ACTIVE][PSFP_GT_OPER_CYCLE_TIME_EXTENSION]    = "n/a",
	[TSN_PSFP_RET_NOT_ACTIVE][PSFP_GT_OPER_BASE_TIME]               = "n/a",
	[TSN_PSFP_RET_CTLIST_EMPTY][PSFP_GT_ADMIN_CONTROL_LIST]         = "Control list is empty",
	[TSN_PSFP_RET_INV_RANGE][PSFP_GT_ADMIN_CYCLE_TIME]              = "Cycle time out of range",
	[TSN_PSFP_RET_INV_ADMIN_PARAM][PSFP_GT_CONFIG_CHANGE]           = "Invalid admin parameter(s)",
	[TSN_PSFP_RET_HW_ACCESS_ERROR][PSFP_GT_OPER_CONTROL_LIST]       = "Internal hardware access error",
	[TSN_PSFP_RET_HW_ACCESS_ERROR][PSFP_GT_GATE_ENABLED]            = "Internal hardware access error",
	[TSN_PSFP_RET_HW_ACCESS_ERROR][PSFP_GT_CONFIG_CHANGE]           = "Internal hardware access error",
	[TSN_PSFP_RET_INV_RANGE][PSFP_HIT_TH_LOW]               = "Low threshold out of range",
	[TSN_PSFP_RET_INV_RANGE][PSFP_HIT_TH_HIGH]              = "High threshold out of range",
};

/* Definition of error messages to be reported for sid module, it uses the specific return code and the function index to report slightly different messages depending of the function */
static const char *sid_error_msg[TSN_SID_RET_MAX_ERROR][MAX_TSN_SID_FUNCTIONS] = {
	[TSN_SID_RET_ADD_ORD_NOT_EXIST][SID_PORT_POS]            = "ORD does not exist",
	[TSN_SID_RET_DEL_ORD_NOT_EXIST][SID_PORT_POS_DEL]        = "ORD does not exist",
	[TSN_SID_RET_ENTRY_NOT_EXIST][SID_SET_STRHDL]            = "ORD entry does not exist",
	[TSN_SID_RET_ENTRY_NOT_EXIST][SID_IDENT_PARAMS]          = "ORD entry does not exist",
	[TSN_SID_RET_ORD_EXISTS][SID_IDENT_PARAMS]               = "ORD already exists",
	[TSN_SID_RET_CHECK_DRIVER_LOG][SID_IDENT_PARAMS]         = "Please check syslog for driver error",
	[TSN_SID_RET_ENTRY_NOT_EXIST][SID_DELETE]                = "ORD entry does not exist",
	[TSN_SID_RET_INVALID][SID_INPUT_CPS]                     = "STREAM_HDL not valid",
	[TSN_SID_RET_INVALID][SID_OUTPUT_CPS]                    = "STREAM_HDL not valid",
};

static const char *bridge_error_msg[TSN_BRIDGE_RET_MAX_ERROR][MAX_TSN_BRIDGE_FUNCTIONS] = {
	[TSN_BRIDGE_DUMMY][PORT_DELAYS_TABLE]                    = "Interface file not long enough, check parameter values",
};

const char *read_lib_error_msg(tsn_return_value error_code) {
	return lib_error_msg[TSN_LIB_ERROR_CODE(error_code)];
}

uint8_t read_hash_map_errno_tsn_err(uint32_t error_code) {
    return hash_map_errno_tsn_err[TSN_MODULE_INDEX_CODE(error_code)][TSN_FUNCTION_INDEX_CODE(error_code)][TSN_ERRNO_ERROR_CODE(error_code)];
}

const char *read_st_error_msg(tsn_return_value error_code) {
    return st_error_msg[TSN_GET_ERROR_INDEX(error_code)][TSN_FUNCTION_INDEX_CODE(error_code)];
}

const char *read_psfp_error_msg(tsn_return_value error_code) {
	return psfp_error_msg[TSN_GET_ERROR_INDEX(error_code)][TSN_FUNCTION_INDEX_CODE(error_code)];
}

const char *read_bridge_error_msg(tsn_return_value error_code) {
	return bridge_error_msg[TSN_GET_ERROR_INDEX(error_code)][TSN_FUNCTION_INDEX_CODE(error_code)];
}

const char *read_sid_error_msg(tsn_return_value error_code) {
	return sid_error_msg[TSN_GET_ERROR_INDEX(error_code)][TSN_FUNCTION_INDEX_CODE(error_code)];
}
