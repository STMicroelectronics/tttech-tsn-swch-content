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

#ifndef TSN_ERROR_
#define TSN_ERROR_

#include <errno.h>
#include <stdint.h>

#define TSN_LIB_ERROR_CODE(return_value)		(((return_value) & 0xFF00)>>8)
#define TSN_ERRNO_ERROR_CODE(return_value)		((return_value) & 0xFF)
#define TSN_FUNCTION_INDEX_CODE(return_value)           (((return_value) & 0xFF0000)>>16)
#define TSN_MODULE_INDEX_CODE(return_value)		(((return_value) & 0xFF000000)>>24)

/** @ingroup internal
    Port handler with sysfs port path */

/* We shift 8 position so it can be reported along with errno from driver */

/** @ingroup tsn_error
    error enum representation. */
typedef enum {
	E_TSN_LIB_NO_ERROR			= 0,		/* THIS IS FINE Operation performed as expected */
	E_TSN_LIB_NOSUCHFILE 			= 1<<8,		/* port/bridge path does not exist or capability is not supported on device */
	E_TSN_LIB_SCAN 				= 2<<8,		/* Failed reading a single value from driver */
	E_TSN_LIB_PRINT				= 3<<8,		/* Failed writing a single value to driver */
	E_TSN_LIB_RDLIST			= 4<<8,		/* Failed reading a list from driver */
	E_TSN_LIB_WRLIST			= 5<<8,		/* Failed writing a list to driver */
	E_TSN_LIB_RDENTRY			= 6<<8,		/* Failed reading a list entry from driver */
	E_TSN_LIB_WRENTRY			= 7<<8,		/* Failed writing a list entry to driver */
	E_TSN_LIB_FCLOSE			= 8<<8,		/* Failed closing the interface, errno is reported in lower 8 bits for more information */
	E_TSN_LIB_FSCAN				= 9<<8,		/* Failed reading a single value from driver, errno is reported in lower 8 bits for more information */
	E_TSN_LIB_FPRINT			= 10<<8,	/* Failed writing a single value to driver, errno is reported in lower 8 bits for more information */
	E_TSN_LIB_FRDLIST			= 11<<8,	/* Failed reading a list from driver, errno is reported in lower 8 bits for more information */
	E_TSN_LIB_FRDSUBLIST			= 12<<8,	/* Failed reading a sublist from driver, errno is reported in lower 8 bits for more information */
	E_TSN_LIB_FWRLIST			= 13<<8,	/* Failed writing a list to driver, errno is reported in lower 8 bits for more information */
	E_TSN_LIB_FWRSUBLIST			= 14<<8,	/* Failed writing a sublist to driver, errno is reported in lower 8 bits for more information */
	E_TSN_LIB_FRDENTRY			= 15<<8,	/* Failed reading a list entry from driver, errno is reported in lower 8 bits for more information */
	E_TSN_LIB_FWRENTRY			= 16<<8,	/* Failed writing a list entry to driver, errno is reported in lower 8 bits for more information */
	E_TSN_LIB_RD_STRUCT			= 17<<8,	/* Function not implemented over structures */
	E_TSN_LIB_PARAMS			= 18<<8,	/* One or more parameters are NULL */
        E_TSN_LIB_MAX_MSTI			= 19<<8,	/* There was an error getting the max msti value from driver, errno is reported in lower 8 bits for more information */
	E_TSN_LIB_MAX_MSTI_ERANGE		= 20<<8,	/* Max msti provided/requested is out of range for the device */
	E_TSN_LIB_MAX_FID			= 21<<8,	/* There was an error getting the max number of vlans/fids from driver, errno is reported in lower 8 bits for more information */
	E_TSN_LIB_MAX_FID_ERANGE		= 22<<8,	/* Max fid provided/requested is out of range for the device */
	E_TSN_LIB_MAX_VID			= 23<<8,	/* There was an error getting the biggest vlan id suppported on the device */
	E_TSN_LIB_MAX_VID_ERANGE		= 24<<8,	/* Max vid provided/requested is out of range for the device */
	E_TSN_LIB_ST_OP_ERANGE			= 25<<8,	/* Gate OP is not supported on device */
	E_TSN_LIB_PSFP_GT_OP_ERANGE		= 26<<8,	/* Gate OP is not supported on device */
	E_TSN_LIB_INVALID_SPEED                 = 27<<8,        /* Requested speed not supported */
	E_TSN_LIB_MAX_ERROR			= ((E_TSN_LIB_INVALID_SPEED) + (1<<8)),
	E_TSN_KNOWN_ERRNO_RET_MAX		= 0x7FFFFFFF,	/* Extra 15 bits to translate driver specific errno codes to more detailed messages i.e. Cycle time out of range, Not configured...*/
								/* 7 MSB are used for module calling i.e. st, mstp, frer ..., 8 LSB used for specific functions */
								/* Why only 7 bits? Because ISO c restrict enums to a signed integer */
} tsn_return_value;

/**
 * @ingroup tsn_api
 * @brief Return specific driver known error messages derived from errno
 *
 * @param[in]    error_code       Full return code returned by a libtsn function
 *
 * @return char *
 *
 **/
const char *tsn_lib_get_known_ernno_msg(tsn_return_value error_code);

uint8_t read_hash_map_errno_tsn_err(uint32_t error_code);
const char *read_lib_error_msg(tsn_return_value error_code);
const char *read_st_error_msg(tsn_return_value error_code);
const char *read_psfp_error_msg(tsn_return_value error_code);
const char *read_bridge_error_msg(tsn_return_value error_code);
const char *read_sid_error_msg(tsn_return_value error_code);

/* Defined on c files */
extern const char *tsn_bridge_get_known_ernno_msg(tsn_return_value error_code);
extern const char *tsn_edgex_get_known_ernno_msg(tsn_return_value error_code);
extern const char *tsn_fqtss_get_known_ernno_msg(tsn_return_value error_code);
extern const char *tsn_frer_get_known_ernno_msg(tsn_return_value error_code);
extern const char *tsn_mstp_get_known_ernno_msg(tsn_return_value error_code);
extern const char *tsn_qbridge_get_known_ernno_msg(tsn_return_value error_code);
extern const char *tsn_sid_get_known_ernno_msg(tsn_return_value error_code);
extern const char *tsn_st_get_known_ernno_msg(tsn_return_value error_code);

/* This returns the specific error code on the tsn_"module"_error_code enums */
#define TSN_GET_ERROR_INDEX(return_value)                read_hash_map_errno_tsn_err(return_value)

/* Error codes for specific modules */

/* Enum for st module, it translate errno to more specific error codes */
typedef enum {
	TSN_ST_RET_INV_ENTRY_POS,
	TSN_ST_RET_INV_PARAM,
	TSN_ST_RET_LIST_TOO_BIG,
	TSN_ST_RET_NOMEM,
	TSN_ST_RET_NOT_ACTIVE,
	TSN_ST_RET_CTLIST_EMPTY,
	TSN_ST_RET_INV_RANGE,
	TSN_ST_RET_INV_ADMIN_PARAM,
	TSN_ST_RET_UNKNOWN_LINK_SPEED,
	TSN_ST_RET_HW_ACCESS_ERROR,
	TSN_ST_RET_MAX_ERROR,
} tsn_st_error_code;

/* Enum for psfp module, it translate errno to more specific error codes */
typedef enum {
	TSN_PSFP_RET_INV_ENTRY_POS,
	TSN_PSFP_RET_INV_PARAM,
	TSN_PSFP_RET_LIST_TOO_BIG,
	TSN_PSFP_RET_NOMEM,
	TSN_PSFP_RET_NOT_ACTIVE,
	TSN_PSFP_RET_CTLIST_EMPTY,
	TSN_PSFP_RET_INV_RANGE,
	TSN_PSFP_RET_INV_ADMIN_PARAM,
	TSN_PSFP_RET_UNKNOWN_LINK_SPEED,
	TSN_PSFP_RET_HW_ACCESS_ERROR,
	TSN_PSFP_RET_MAX_ERROR,
} tsn_psfp_error_code;

/* Enum for sid module, it translate errno to more specific error codes */
typedef enum {
	TSN_SID_RET_ADD_ORD_NOT_EXIST = 1,
	TSN_SID_RET_DEL_ORD_NOT_EXIST,
	TSN_SID_RET_ENTRY_NOT_EXIST,
	TSN_SID_RET_ORD_EXISTS,
	TSN_SID_RET_CHECK_DRIVER_LOG,
	TSN_SID_RET_INVALID,
	TSN_SID_RET_MAX_ERROR,
} tsn_sid_error_code;

/* Enum for BRIDGE module, it translate errno to more specific error codes */
typedef enum {
	TSN_BRIDGE_DUMMY = 1, //to be replaced by first real bridge error
	TSN_BRIDGE_RET_MAX_ERROR,
} tsn_bridge_error_code;

/* Approximation to max number of functions to use in hash table, faster than making a function to find the max and it should keep the max at a reasonable number even if adding new tsn modules */
/* Around 63 entries vs 42, so with a proper max function some memory can be saved */
#define MAX_TSN_FUNCTIONS		(MAX_TSN_BRIDGE_FUNCTIONS|MAX_TSN_EDGEX_FUNCTIONS|MAX_TSN_FRER_FUNCTIONS|MAX_TSN_FQTSS_FUNCTIONS \
			 		 |MAX_TSN_MSTP_FUNCTIONS|MAX_TSN_PREEMP_FUNCTIONS|MAX_TSN_QBRIDGE_FUNCTIONS|MAX_TSN_SID_FUNCTIONS|MAX_TSN_ST_FUNCTIONS)

/* Another hash array could be used for only hashing known errnos and, thus, reduce memory consumption. Nevertheless the error msg matrix should be the most memory heavy */
#define MAX_ERRNO			EHWPOISON /* Assumption that no bigger error will be returned, anyways it will always be lower */

#endif /*TSN_ERROR_*/
