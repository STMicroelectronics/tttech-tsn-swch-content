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

#ifndef TSN_DEF_H_
#define TSN_DEF_H_

#include <stdint.h>
#include <stdbool.h>

enum tsn_feat {
    FEAT_ST        = 0,     /* IEEE 802.1Qbv */
    FEAT_PREE,              /* IEEE 802.1Qbu */
    FEAT_MACMERGE,          /* IEEE 802.3br */
};

#define TSN_ST_OP_SET_GATE_STATES     ((uint8_t)0)
#define TSN_ST_OP_SET_AND_HOLD_MAC    ((uint8_t)1)
#define TSN_ST_OP_SET_AND_RELEASE_MAC ((uint8_t)2)
#define TSN_ST_OP_EXCESS              ((uint8_t)0xff)

#define SCHED_DEF_GATE_STATES ((uint8_t)0xff) // gate states open
#define MAX_NO_TRAFFIC_CLASSES ((uint8_t)8)
#define MAX_NR_QUEUES ((uint8_t)8) // priority queues, max number of PCP's
#define TSN_ETH_ALEN 6 // avoid depending on possibly problematic headers

#define TSN_PORT_ID_INVALID           (0xffffu)

struct port_hdl;

/** @ingroup tsn_st_api
    rational number representation. */
struct tsn_st_rational_number {
    uint32_t numerator;
    uint32_t denominator;
};

/** @ingroup tsn_st_api
    ptp time representation. */
struct tsn_st_ptp_time {
    uint64_t sec;   ///< 48 bit seconds
    uint32_t nsec;  ///< 32 bit nanoseconds (always < 1000000000)
    /// Currently subnanoseconds left out
};

/* NOTE: MUST BE IDENTICAL TO SYSFS DEFINITION! */
/** @ingroup tsn_st_api
    gate control entry representation. */
struct st_gate_control_entry {
    uint32_t interval;
    uint8_t  op;
    uint8_t  gsv;
    uint16_t padding;
} __attribute__((packed));

/** @ingroup tsn_bridge_api
    bridge port attributes and capabilities. */
struct brport_attr_and_caps {
    int portExternal;
    int portType;
    int portAcceptableFrameTypes;
    bool portEnableIngressFiltering;
    bool portTaggingCapable;
    bool portConfigurableAcceptableFrameTypes;
    bool portIngressFilteringCapable;
    /*Port Capabilities:*/
    bool portTypeCapCustomerVlan;
    bool portTypeCapProviderNetwork;
    bool portTypeCapCustomerNetwork;
    bool portTypeCapCustomerEdge;
    bool portTypeCapCustomerBackbone;
    bool portTypeCapVirtualInstance;
    bool portTypeCapDBridge;
    bool portTypeCapRemoteCustomerAccess;
    bool portTypeCapStationFacing;
    bool portTypeCapUplinkAccess;
    bool portTypeCapUplinkRelay;
}__attribute__((packed));

struct bridge_attr_and_caps {
    int BridgeType;
    int BridgeNumPorts;
    bool BridgeComponents;
    bool BridgeTrafficClassesEnabled;
    /*Component Attributes:*/
    int ComponentId;
    int ComponentType;
    int ComponentNumPorts;
    /*FDB Attributes:*/
    int FdbSize;
    int FdbStaticEntries;
    /*Capability Attributes:*/
    bool ExtendedFilteringServices;
    bool TrafficClasses;
    bool StaticEntryIndividualPort;
    bool IVLCapable;
    bool SVLCapable;
    bool HybridCapable;
    bool ConfigurablePvidTagging;
    bool LocalVlanCapable;
    /*bin Attributes */
}__attribute__((packed));

/** @ingroup tsn_preemp_api
    enumeration of allowed preemption status values. */
typedef enum {
    UNKNOWN = 0, /* non-standard, used to denote error */
    EXPRESS = 1,
    PREEMPT = 2,
} preemp_stat;

typedef enum {
    IDLE   = 1,
    ACTIVE = 2,
} frame_preemp_active;

typedef enum {
    FRAG_64  = 0,
    FRAG_128 = 1,
    FRAG_192 = 2,
    FRAG_256 = 3,
    FRAG_UNKNOWN = 4, /* non-standard, used to denote error */
} preemp_fragsz;

typedef enum {
    VRFY_UNKNOWN   = 1,
    VRFY_INITIAL   = 2,
    VRFY_VERIFYING = 3,
    VRFY_SUCCEEDED = 4,
    VRFY_FAILED    = 5,
    VRFY_DISABLED  = 6,
    VRFY_INVALID   = 7,  /* non-standard, used to denote error */
} preemp_vrfy_stat;

typedef enum {
    TXSTAT_UNKNOWN  = 1,
    TXSTAT_INACTIVE = 2,
    TXSTAT_ACTIVE   = 3,
    TXSTAT_INVALID  = 4,  /* non-standard, used to denote error */
} preemp_tx_stat;

#define MAXTC 8
#define MAX_PORT 16

struct br_delay_entry {
    uint16_t           port_ingress;
    uint16_t           port_egress;
    uint16_t           traffic_class;
    uint64_t           indep_min_delay_ns; 	// in nanoseconds
    uint64_t           indep_max_delay_ns; 	// in nanoseconds
    uint32_t           dep_min_delay_ps;	// in picoseconds
    uint32_t           dep_max_delay_ps;	// in picoseconds
} __attribute__((packed));

struct pt_delay_entry {
    uint16_t port;
    uint16_t speed;
    uint64_t dep_rx_delay_min_ps; //picoseconds
    uint64_t dep_rx_delay_max_ps; //picoseconds
    uint32_t indep_rx_delay_min_ns;
    uint32_t indep_rx_delay_max_ns;
    uint32_t indep_rly_delay_min_ns;
    uint32_t indep_rly_delay_max_ns;
    uint32_t indep_tx_delay_min_ns;
    uint32_t indep_tx_delay_max_ns;
} __attribute__((packed));

struct qbr_fid2vid_entry {
    uint32_t vid;
    // no further padding necessary
} __attribute__((packed));

struct msti_list_entry {
    uint32_t mstid;
    // no further padding necessary
} __attribute__((packed));

typedef enum {
    PORT_DISABLED   = 0,
    PORT_LISTENING  = 1,
    PORT_LEARNING   = 2,
    PORT_FORWARDING = 3,
    PORT_BLOCKING   = 4,
    MAX_PORT_STATES = 5,
} port_mstp_state;

/*Stream Identification -> SID*/
typedef enum {
    SID_UNKNOWN  = 0,
    SID_NULL = 1,
    SID_SOURCE = 2,
    SID_ACTIVE = 3,
    SID_IPSTR = 4,
    SID_IDENT_MAX
} sid_ident_type;

typedef enum {
    SID_TAGGED = 1,
    SID_PRIORITY = 2,
    SID_ALL = 3,
} sid_tag;

/*Check IEEE802.1CB, section 9.1.1.2-9.1.1.5*/
typedef enum {
    IN_FAC_OUT = 0,
    IN_FAC_IN,
    OUT_FAC_OUT,
    OUT_FAC_IN,
    POS_UNKNOWN,
} sid_pos;

struct port_list {
    uint16_t in_fac_out;
    uint16_t out_fac_out;
    uint16_t in_fac_in;
    uint16_t out_fac_in;
} __attribute__((packed));

struct port_pos {
    uint16_t port;
    sid_pos pos;
} __attribute__((packed));

/*Currently supported only NULL and SOURCE sid for which params have same format
 * in case of other methods (active) need to be implemented, params field to be
 * reimplemented */
/*Check IEEE802.1CB, section 9.1.2*/
struct ident_params {
    uint8_t addr[TSN_ETH_ALEN];
    sid_tag tag;
    uint16_t vid;
    sid_ident_type id_type;
    uint16_t str_hdl;
} __attribute__((packed));

typedef enum {
    TSN_BRIDGE = 0u,
    TSN_EDGEX,
    TSN_FQTSS,
    TSN_FRER,
    TSN_MSTP,
    TSN_PREEMP,
    TSN_QBRIDGE,
    TSN_SID,
    TSN_ST,
    TSN_PSFP,
    MAX_TSN_MODULES,
} tsn_modules;

typedef enum {
    PORT_NUM_TRAFFIC_CLASSES,
    PORT_DFLT_USER_PRIORITY,
    PRIORITY_REGEN_TBL,
    TRAFFIC_CLASS_TBL,
    PORT_ATTR_AND_CAPS,
    PORT_ID,
    BRIDGE_ATTR_AND_CAPS,
    DELAYS_TABLE,
    PORT_DELAYS_TABLE,
    MAX_TSN_BRIDGE_FUNCTIONS  /* Add new indexes before MAX_TSN_PREEMP_FUNCTIONS and add related filename on sysfs_files */
} bridge_fct;

typedef enum {
    CUT_THROUGH = 0u,
    MIRROR_PORT,
    MAX_TSN_EDGEX_FUNCTIONS, /* Add new indexes before MAX_TSN_EDGX_FUNCTIONS
                                and add related filename on sysfs_files */
} edgex_fct;

typedef enum {
    ADMIN_IDLE_SLOPE_TABLE = 0u,
    OPER_IDLE_SLOPE_TABLE,
    TX_SEL_ALGORITHM_TABLE,
    MAX_TSN_FQTSS_FUNCTIONS  /* Add new indexes before MAX_TSN_PREEMP_FUNCTIONS and add related filename on sysfs_files */
} fqtss_fct;

typedef enum {
        FRER_SEQGEN_CREATE = 0u,
        FRER_SEQGEN_ADD_STRHDL,
        FRER_SEQGEN_ADD_DIR,
        FRER_SEQGEN_DEL_STRHDL,
        FRER_SEQGEN_DEL_FN,
        FRER_SEQGEN_FN_CNT,
        FRER_SEQGEN_FN_LIST,
        FRER_SEQGEN_SID2FN,
        FRER_SEQGEN_FN2DIR,
        FRER_SEQGEN_FN2SID,
        FRER_SEQENC_CREATE,
        FRER_SEQENC_ACTIVE,
        FRER_SEQENC_TYPE,
        FRER_SEQENC_ADDSTR,
        FRER_SEQENC_DELSTR,
        FRER_SEQENC_DELPORT,
        FRER_SEQENC_PORT2STR,
        FRER_SEQENC_STR2PORT,
        FRER_SEQREC_CREATE,
        FRER_SEQREC_ADDSTR,
        FRER_SEQREC_ADDPORT,
        FRER_SEQREC_TAKENOSEQ,
        FRER_SEQREC_INDRCV,
        FRER_SEQREC_LATERR,
        FRER_SEQREC_ALG,
        FRER_SEQREC_HISTLEN,
        FRER_SEQREC_DIR,
        FRER_SEQREC_RCVRESET,
        FRER_SEQREC_DELPORT,
        FRER_SEQREC_DELSTR,
        FRER_SEQREC_DELFN,
        FRER_SEQREC_FN2SID,
        FRER_SEQREC_PTMASK,
        FRER_SEQREC_FN_CNT,
        FRER_SEQREC_FN_LIST,
        FRER_SEQREC_CNT_OOO,
        FRER_SEQREC_CNT_TAGLESS,
        FRER_SEQREC_CNT_LOST,
        FRER_SEQREC_CNT_RESETS,
        FRER_SEQREC_CNT_ROGUE,
        FRER_SEQREC_CNT_DISCARDED,
        FRER_SEQREC_CNT_PASSED,
        FRER_NUM_PORTS,
        MAX_TSN_FRER_FUNCTIONS
} frer;

typedef enum {
    FLUSH_TREE = 0u,
    MAX_MSITD,
    CREATE_MSTI,
    DELETE_MSTI,
    GET_FID2MSTI,
    SET_FID2MSTI,
    GET_MSTI_LIST,
    GET_PORT_STATE,
    SET_PORT_STATE,
    MAX_TSN_MSTP_FUNCTIONS  /* Add new indexes before MAX_TSN_FUNCTIONS and add related filename on sysfs_files */
} mstp_fct;

typedef enum {
    HOLD_ADVANCE = 0u,
    HOLD_REQUEST,
    PREEMPTION_ACTIVE,
    RELEASE_ADVANCE,
    STATUS_TABLE,
    SUPPORT,
    VERIFY_STATUS,
    TX_EN,
    VERIFY_TX_DIS,
    STATUS_TX,
    TIME_MS,
    FRAG_SIZE,
    ASSERR_CNT,
    SMDERR_CNT,
    ASSOK_CNT,
    FRAGRX_CNT,
    FRAGTX_CNT,
    HOLD_CNT,
    MAX_TSN_PREEMP_FUNCTIONS  /* Add new indexes before MAX_TSN_PREEMP_FUNCTIONS and add related filename on sysfs_files */
} preemp_fct;

typedef enum {
    MAX_VLAN_ID = 0u,
    MAX_SUPPORTED_VLANS,
    NUM_VLANS,
    VID2FID_TABLE,
    FID2VID_TABLE,
    MAX_TSN_QBRIDGE_FUNCTIONS
} qbridge_fct;

typedef enum {
    SID_IDENT_PARAMS = 0u,
    SID_DELETE,
    SID_PORT_POS,
    SID_PORT_POS_DEL,
    SID_SET_STRHDL,
    SID_MAX_SUPPORTED,
    SID_CURRENT_CNT,
    SID_LIST_ENTRIES,
    SID_INPUT_CPS,
    SID_OUTPUT_CPS,
    SID_TAB_LEN,
    SID_IDENT_TYPES,
    SID_SUP_PORTS,
    MAX_TSN_SID_FUNCTIONS
} st_ident;

typedef enum {
    MAX_SDU = 0u,
    OVERRUN,
    GATE_ENABLED,
    ADMIN_GATE_STATES,
    OPER_GATE_STATES,
    ADMIN_CONTROL_LIST_LENGTH,
    OPER_CONTROL_LIST_LENGTH,
    ADMIN_CONTROL_LIST,
    OPER_CONTROL_LIST,
    ADMIN_CYCLE_TIME,
    OPER_CYCLE_TIME,
    ADMIN_CYCLE_TIME_EXTENSION,
    OPER_CYCLE_TIME_EXTENSION,
    ADMIN_BASE_TIME,
    OPER_BASE_TIME,
    CONFIG_CHANGE,
    CONFIG_CHANGE_TIME,
    TICK_GRANULARITY,
    CURRENT_TIME,
    CONFIG_PENDING,
    CONFIG_CHANGE_ERROR,
    SUPPORTED_LIST_MAX,
    ST_SUPPORTED_CYCLE_MAX,
    ST_SUPPORTED_INTERVAL_MAX,
    MAX_TSN_ST_FUNCTIONS  /* Add new indexes before MAX_TSN_FUNCTIONS and add related filename on sysfs_files */
} st_fct;

typedef enum {
    PSFP_MAX_FILTERS = 0u,
    PSFP_MAX_GATES,
    PSFP_MAX_FLOW_METERS,
    PSFP_SUPPORTED_LIST_MAX,
    PSFP_SUPPORTED_CYCLE_MAX,
    PSFP_SUPPORTED_INTERVAL_MAX,
    PSFP_FLT_TABLE,
    PSFP_FLT_DELETE,
    PSFP_FLT_MATCHING_FRAMES_CNT,
    PSFP_FLT_PASSING_FRAMES_CNT,
    PSFP_FLT_NOT_PASSING_FRAMES_CNT,
    PSFP_FLT_PASSING_SDU_CNT,
    PSFP_FLT_NOT_PASSING_SDU_CNT,
    PSFP_FLT_RED_FRAMES_CNT,
    PSFP_HIT_GATE,
    PSFP_HIT_TH_LOW,
    PSFP_HIT_TH_HIGH,
    PSFP_HIT_LOW_CNT,
    PSFP_HIT_HIGH_CNT,
    PSFP_GT_GATE_ENABLED,
    PSFP_GT_ADMIN_GATE_STATES,
    PSFP_GT_OPER_GATE_STATES,
    PSFP_GT_ADMIN_CONTROL_LIST_LENGTH,
    PSFP_GT_OPER_CONTROL_LIST_LENGTH,
    PSFP_GT_ADMIN_CONTROL_LIST,
    PSFP_GT_OPER_CONTROL_LIST,
    PSFP_GT_ADMIN_CYCLE_TIME,
    PSFP_GT_OPER_CYCLE_TIME,
    PSFP_GT_ADMIN_CYCLE_TIME_EXTENSION,
    PSFP_GT_OPER_CYCLE_TIME_EXTENSION,
    PSFP_GT_ADMIN_BASE_TIME,
    PSFP_GT_OPER_BASE_TIME,
    PSFP_GT_CONFIG_CHANGE,
    PSFP_GT_CONFIG_CHANGE_TIME,
    PSFP_GT_TICK_GRANULARITY,
    PSFP_GT_CURRENT_TIME,
    PSFP_GT_CONFIG_PENDING,
    PSFP_GT_CONFIG_CHANGE_ERROR,
    PSFP_GT_ADMIN_IPV,
    PSFP_GT_OPER_IPV,
    PSFP_GT_CLOSED_INV_RX_ENABLE,
    PSFP_GT_CLOSED_INV_RX,
    MAX_TSN_PSFP_FUNCTIONS  /* Add new indexes before MAX_TSN_PSFP_FUNCTIONS and add related filename on sysfs_files */
} psfp_fct;

struct psfp_flt_params {
    int32_t str_hdl;
    int32_t prio;
    uint32_t gate_id;
    uint32_t max_sdu;
    int32_t fmtr_id;
    bool blk_oversz_ena;
    bool blk_oversz;
} __attribute((packed));

#define PSFP_FLT_ANY_STR_HDL    -1
#define PSFP_FLT_ANY_PRIO       -1
#define PSFP_FLT_NO_FMTR        -1

/* NOTE: MUST BE IDENTICAL TO SYSFS DEFINITION! */
/** @ingroup tsn_psfp_api
    gate control entry representation. */
struct psfp_gate_control_entry {
    uint32_t interval;
    uint8_t  op;
    uint8_t  gsv;
    int8_t   ipv;
    uint8_t  padding;
} __attribute__((packed));

#define PSFP_GT_GSV_OPEN        0x01
#define PSFP_GT_GSV_CLOSED      0x00

#define TSN_PSFP_GT_OP_SET_GATE_AND_IPV ((uint8_t)0)

#define PSFP_GT_IPV_NULL        -1

#include "tsn_error.h"

#endif /*TSN_DEF_H_*/
