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

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <inttypes.h>

#include "tsn_private.h"
#include "tsn_bridge.h"

#define IEEE8021BRIDGE_DIR "ieee8021Bridge/"

static const uint32_t tsn_module = TSN_BRIDGE;
static uint32_t get_return_header(tsn_file_desc *fd);

static const sysfs_fn bridge_fn[MAX_TSN_BRIDGE_FUNCTIONS] = {
    [PORT_NUM_TRAFFIC_CLASSES] = IEEE8021BRIDGE_DIR"portNumTrafficClasses",
    [PORT_DFLT_USER_PRIORITY]  = IEEE8021BRIDGE_DIR"portDefaultUserPriority",
    [PRIORITY_REGEN_TBL]       = IEEE8021BRIDGE_DIR"portUserPriorityRegenTable",
    [TRAFFIC_CLASS_TBL]        = IEEE8021BRIDGE_DIR"portTrafficClassTable",
    [PORT_ATTR_AND_CAPS]       = "", // provided by port_caps structure
    [PORT_ID]                  = "phys_port_name",  // provided by ndo_get_phys_port_name
    [BRIDGE_ATTR_AND_CAPS]     = "", // provided by bridge_caps structure
    [DELAYS_TABLE]             = IEEE8021BRIDGE_DIR"delays",
    [PORT_DELAYS_TABLE]        = IEEE8021BRIDGE_DIR"port_delays",
};

/* The following structures arrays are meant to allow to group and read sysfs values toguether and write them in a bulk to a provided structure using ASSERT_RD_STRUCT
   format { sysfspath, MACRO_TO_USE_TO_GET_VALUE, 						{params_for_macro}, sizeof_field_to_write_to_struct}
		       ^ only supporting ASSERT_FSCAN for now, but should be easy to extend     	 ^union (for now only containing the format parameter)
   The structure array has to end always with END_OF_STRUCT_FN_ARRAY
*/
struct sysfs_struct_fn port_caps[MAX_SYSFS_VALUES] = {
        {IEEE8021BRIDGE_DIR"portExternal", "ASSERT_FSCAN" , {"%d"}, sizeof(int)},
        {IEEE8021BRIDGE_DIR"portType", "ASSERT_FSCAN" , {"%d"}, sizeof(int)},
        {IEEE8021BRIDGE_DIR"portAcceptableFrameTypes", "ASSERT_FSCAN" , {"%d"}, sizeof(int)},
        {IEEE8021BRIDGE_DIR"portEnableIngressFiltering", "ASSERT_FSCAN" , {"%"SCNu8}, sizeof(bool)},
        {IEEE8021BRIDGE_DIR"portTaggingCapable", "ASSERT_FSCAN" , {"%"SCNu8}, sizeof(bool)},
        {IEEE8021BRIDGE_DIR"portConfigurableAcceptableFrameTypes", "ASSERT_FSCAN" , {"%"SCNu8}, sizeof(bool)},
        {IEEE8021BRIDGE_DIR"portIngressFilteringCapable", "ASSERT_FSCAN" , {"%"SCNu8}, sizeof(bool)},
        {IEEE8021BRIDGE_DIR"portTypeCapCustomerVlan", "ASSERT_FSCAN" , {"%"SCNu8}, sizeof(bool)},
        {IEEE8021BRIDGE_DIR"portTypeCapProviderNetwork", "ASSERT_FSCAN" , {"%"SCNu8}, sizeof(bool)},
        {IEEE8021BRIDGE_DIR"portTypeCapCustomerNetwork", "ASSERT_FSCAN" , {"%"SCNu8}, sizeof(bool)},
        {IEEE8021BRIDGE_DIR"portTypeCapCustomerEdge", "ASSERT_FSCAN" , {"%"SCNu8}, sizeof(bool)},
        {IEEE8021BRIDGE_DIR"portTypeCapCustomerBackbone", "ASSERT_FSCAN" , {"%"SCNu8}, sizeof(bool)},
        {IEEE8021BRIDGE_DIR"portTypeCapVirtualInstance", "ASSERT_FSCAN" , {"%"SCNu8}, sizeof(bool)},
        {IEEE8021BRIDGE_DIR"portTypeCapDBridge", "ASSERT_FSCAN" , {"%"SCNu8}, sizeof(bool)},
        {IEEE8021BRIDGE_DIR"portTypeCapRemoteCustomerAccess", "ASSERT_FSCAN" , {"%"SCNu8}, sizeof(bool)},
        {IEEE8021BRIDGE_DIR"portTypeCapStationFacing",  "ASSERT_FSCAN" , {"%"SCNu8}, sizeof(bool)},
        {IEEE8021BRIDGE_DIR"portTypeCapUplinkAccess", "ASSERT_FSCAN" , {"%"SCNu8}, sizeof(bool)},
        {IEEE8021BRIDGE_DIR"portTypeCapUplinkRelay", "ASSERT_FSCAN" , {"%"SCNu8}, sizeof(bool)},
        END_OF_STRUCT_FN_ARRAY
};

struct sysfs_struct_fn bridge_caps[MAX_SYSFS_VALUES] = {
        {IEEE8021BRIDGE_DIR"BridgeType", "ASSERT_FSCAN" , {"%d"}, sizeof(int)},
        {IEEE8021BRIDGE_DIR"BridgeNumPorts", "ASSERT_FSCAN" , {"%d"}, sizeof(int)},
        {IEEE8021BRIDGE_DIR"BridgeComponents", "ASSERT_FSCAN" , {"%"SCNu8}, sizeof(bool)},
        {IEEE8021BRIDGE_DIR"BridgeTrafficClassesEnabled", "ASSERT_FSCAN" , {"%"SCNu8}, sizeof(bool)},
        {IEEE8021BRIDGE_DIR"ComponentId", "ASSERT_FSCAN" , {"%d"}, sizeof(int)},
        {IEEE8021BRIDGE_DIR"ComponentType", "ASSERT_FSCAN" , {"%d"}, sizeof(int)},
        {IEEE8021BRIDGE_DIR"ComponentNumPorts", "ASSERT_FSCAN" , {"%d"}, sizeof(int)},
        {IEEE8021BRIDGE_DIR"FdbSize", "ASSERT_FSCAN" , {"%d"}, sizeof(int)},
        {IEEE8021BRIDGE_DIR"FdbStaticEntries", "ASSERT_FSCAN" , {"%d"}, sizeof(int)},
        {IEEE8021BRIDGE_DIR"ExtendedFilteringServices", "ASSERT_FSCAN" , {"%"SCNu8}, sizeof(bool)},
        {IEEE8021BRIDGE_DIR"TrafficClasses", "ASSERT_FSCAN" , {"%"SCNu8}, sizeof(bool)},
        {IEEE8021BRIDGE_DIR"StaticEntryIndividualPort", "ASSERT_FSCAN" , {"%"SCNu8}, sizeof(bool)},
        {IEEE8021BRIDGE_DIR"IVLCapable", "ASSERT_FSCAN" , {"%"SCNu8}, sizeof(bool)},
        {IEEE8021BRIDGE_DIR"SVLCapable", "ASSERT_FSCAN" , {"%"SCNu8}, sizeof(bool)},
        {IEEE8021BRIDGE_DIR"HybridCapable", "ASSERT_FSCAN" , {"%"SCNu8}, sizeof(bool)},
        {IEEE8021BRIDGE_DIR"ConfigurablePvidTagging",  "ASSERT_FSCAN" , {"%"SCNu8}, sizeof(bool)},
        {IEEE8021BRIDGE_DIR"LocalVlanCapable", "ASSERT_FSCAN" , {"%"SCNu8}, sizeof(bool)},
        END_OF_STRUCT_FN_ARRAY
};

tsn_return_value tsn_fqtss_get_prio_regen_table(struct port_hdl *port, unsigned int prio,
                                                uint8_t *admin_sentry)
{
    tsn_file_desc fd = { .id = PRIORITY_REGEN_TBL};
    ASSERT_PARAMS(port && admin_sentry);
    ASSERT_FOPEN(fd, port, bridge_fn[fd.id], O_RDONLY);
    ASSERT_FRDENTRY(fd, admin_sentry, prio);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_fqtss_set_prio_regen_table(struct port_hdl *port,
                                                unsigned int pcp,
                                                const uint8_t admin_sentry)
{
    tsn_file_desc fd = { .id = PRIORITY_REGEN_TBL};
    ASSERT_PARAMS(port);
    ASSERT_FOPEN(fd, port, bridge_fn[fd.id], O_RDWR);
    ASSERT_FWRENTRY(fd, admin_sentry, pcp);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_fqtss_set_tc_table(struct port_hdl *port,
                                        unsigned int pcp,
                                        const uint8_t admin_sentry)
{
    tsn_file_desc fd = { .id = TRAFFIC_CLASS_TBL};
    ASSERT_PARAMS(port);
    ASSERT_FOPEN(fd, port, bridge_fn[fd.id], O_RDWR);
    ASSERT_FWRENTRY(fd, admin_sentry, pcp);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_fqtss_get_tc_table(struct port_hdl *port,
                                        unsigned int prio,
                                        uint8_t *admin_sentry)
{
    tsn_file_desc fd = { .id = TRAFFIC_CLASS_TBL};
    ASSERT_PARAMS(port && admin_sentry);
    ASSERT_FOPEN(fd, port, bridge_fn[fd.id], O_RDONLY);
    ASSERT_FRDENTRY(fd, admin_sentry, prio);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_bridge_get_delays(struct port_hdl *bridge,
                                       uint32_t ingress_pt,
                                       uint32_t egress_pt,
                                       uint32_t tc,
                                       struct br_delay_entry *delay_entry)

{
    uint32_t index;

    if (egress_pt == ingress_pt) {
        /* delay from a port to itself makes no sense*/
        return E_TSN_LIB_FRDENTRY;
    }
    /* There is a matrix of MAX_PORT X MAX_PORT-1 (from each port to all other ports).
     * Each matrix item is an array of MAXTC (max number of traffic classes) delay entries.
     * For this reason there is a jump in egress port if ingress port < egress port
     * port */
    if (ingress_pt > egress_pt) {
        index = tc * sizeof(struct br_delay_entry) +
                (egress_pt - 1) * (sizeof(struct br_delay_entry) * MAXTC) +
                (ingress_pt - 1) * (MAX_PORT -1) * (sizeof(struct br_delay_entry) * MAXTC);
    } else {
        index = tc * sizeof(struct br_delay_entry) +
                (egress_pt - 2) * (sizeof(struct br_delay_entry) * MAXTC) +
                (ingress_pt - 1) * (MAX_PORT -1) * (sizeof(struct br_delay_entry) * MAXTC);
    }
    tsn_file_desc fd = { .id = DELAYS_TABLE};
    ASSERT_PARAMS(bridge && delay_entry);
    ASSERT_FOPEN(fd, bridge, bridge_fn[fd.id], O_RDONLY);
    ASSERT_FRDENTRY(fd, delay_entry, index);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_bridge_get_indexed_port_delays(struct port_hdl *port,
						    uint32_t index, tsn_file_desc fd,
                                            struct pt_delay_entry *delay_entry)
{
    ASSERT_PARAMS(port && delay_entry);
    ASSERT_FOPEN(fd, port, bridge_fn[fd.id], O_RDONLY);
    ASSERT_FRDENTRY(fd, delay_entry, index);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_bridge_get_port_delays(struct port_hdl *port,
	                                    uint32_t speed,
                                            struct pt_delay_entry *delay_entry)
{
    tsn_file_desc fd = { .id = PORT_DELAYS_TABLE};
    switch (speed) {
    case 10:
	return tsn_bridge_get_indexed_port_delays(port, 0, fd, delay_entry);
    case 100:
	return tsn_bridge_get_indexed_port_delays(port, 1, fd, delay_entry);
    case 1000:
	return tsn_bridge_get_indexed_port_delays(port, 2, fd, delay_entry);
    default:
        return RETURN_HEADER|E_TSN_LIB_INVALID_SPEED;
    }
}

tsn_return_value tsn_bridge_get_num_traffic_classes(struct port_hdl *port,
                                                    uint32_t *num_tcs)
{
    tsn_file_desc fd = { .id = PORT_NUM_TRAFFIC_CLASSES};
    ASSERT_PARAMS(port && num_tcs);
    ASSERT_FOPEN(fd, port, bridge_fn[fd.id], O_RDONLY);
    ASSERT_FSCAN(fd, "%"SCNu32, 1, num_tcs);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_bridge_set_default_prio(struct port_hdl *port,
                                             uint32_t dflt_prio)
{
    tsn_file_desc fd = { .id = PORT_DFLT_USER_PRIORITY};
    ASSERT_PARAMS(port);
    ASSERT_FOPEN(fd, port, bridge_fn[fd.id], O_RDWR);
    ASSERT_FPRINT(fd, "%"PRIu32, (unsigned int)dflt_prio);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_bridge_get_default_prio(struct port_hdl *port,
                                             uint32_t *dflt_prio)
{
    tsn_file_desc fd = { .id = PORT_DFLT_USER_PRIORITY};
    ASSERT_PARAMS(port && dflt_prio);
    ASSERT_FOPEN(fd, port, bridge_fn[fd.id], O_RDONLY);
    ASSERT_FSCAN(fd, "%"SCNu32, 1, dflt_prio);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_bridge_port_id(struct port_hdl *port, uint32_t *port_id)
{
    tsn_file_desc fd = { .id = PORT_ID};
    ASSERT_PARAMS(port && port_id);
    ASSERT_FOPEN(fd, port, bridge_fn[fd.id], O_RDONLY);
    ASSERT_FSCAN(fd, "p%"SCNu32, 1, port_id);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_bridge_port_attr(struct port_hdl *port,
                                      struct brport_attr_and_caps *caps)
{
    tsn_file_desc fd = { .id = MAX_TSN_BRIDGE_FUNCTIONS};
    ASSERT_PARAMS(port && caps);
    ASSERT_RD_STRUCT(fd, port, port_caps, O_RDONLY, caps);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_bridge_attr(struct port_hdl *port,
                                 struct bridge_attr_and_caps *caps)
{
    tsn_file_desc fd = { .id = MAX_TSN_BRIDGE_FUNCTIONS};
    ASSERT_PARAMS(port && caps);
    ASSERT_RD_STRUCT(fd, port, bridge_caps, O_RDONLY, caps);
    ASSERT_FCLOSE(fd);
}

const char *tsn_bridge_get_known_ernno_msg(tsn_return_value return_code)
{
    return read_bridge_error_msg(return_code);
}

static uint32_t get_return_header(tsn_file_desc *fd)
{
    return ((fd-> id < MAX_TSN_BRIDGE_FUNCTIONS) ?
            (tsn_module<<24)|((fd->id)<<16) : E_TSN_LIB_NO_ERROR);
}
