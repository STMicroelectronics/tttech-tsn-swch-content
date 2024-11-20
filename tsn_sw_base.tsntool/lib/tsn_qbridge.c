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
#include "tsn_qbridge.h"

#define IEEE8021QBRIDGE_DIR "ieee8021QBridge/"

static const uint32_t tsn_module = TSN_QBRIDGE;

static const sysfs_fn qbridge_fn[MAX_TSN_QBRIDGE_FUNCTIONS] = {
    [MAX_VLAN_ID]         = IEEE8021QBRIDGE_DIR"MaxVlanId",
    [MAX_SUPPORTED_VLANS] = IEEE8021QBRIDGE_DIR"MaxSupportedVlans",
    [NUM_VLANS]           = IEEE8021QBRIDGE_DIR"NumVlans",
    [VID2FID_TABLE]       = IEEE8021QBRIDGE_DIR"vid2fid",
    [FID2VID_TABLE]       = IEEE8021QBRIDGE_DIR"fid2vid",
};

tsn_return_value tsn_qbridge_get_max_vlan_id(struct port_hdl *bridge,
                                             uint32_t *max_vlan_id)
{
    tsn_file_desc fd = { .id = MAX_VLAN_ID};
    ASSERT_PARAMS(bridge && max_vlan_id);
    ASSERT_FOPEN(fd, bridge, qbridge_fn[fd.id], O_RDONLY);
    ASSERT_FSCAN(fd, "%"SCNu32, 1, max_vlan_id);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_qbridge_get_max_supported_vlans(const struct port_hdl *bridge,
                                                     uint32_t *max_supported_vlans)
{
    tsn_file_desc fd = { .id = MAX_SUPPORTED_VLANS};
    ASSERT_PARAMS(bridge && max_supported_vlans);
    ASSERT_FOPEN(fd, bridge, qbridge_fn[fd.id], O_RDONLY);
    ASSERT_FSCAN(fd, "%"SCNu32, 1, max_supported_vlans);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_qbridge_get_num_vlans(struct port_hdl *bridge,
                                           uint32_t *num_vlans)
{
    tsn_file_desc fd = { .id = NUM_VLANS};
    ASSERT_PARAMS(bridge && num_vlans);
    ASSERT_FOPEN(fd, bridge, qbridge_fn[fd.id], O_RDONLY);
    ASSERT_FSCAN(fd, "%"SCNu32, 1, num_vlans);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_qbridge_set_vid2fid(struct port_hdl *bridge, uint32_t vid,
                                         uint32_t fid)
{
    tsn_file_desc fd = { .id = VID2FID_TABLE};
    tsn_return_value ret = E_TSN_LIB_NO_ERROR;
    unsigned int max_vid = 0;
    unsigned int max_fid = 0;
    ASSERT_PARAMS(bridge);
    if ((ret = tsn_qbridge_get_max_vlan_id(bridge, &max_vid)) != E_TSN_LIB_NO_ERROR)
        return (E_TSN_LIB_MAX_VID|TSN_ERRNO_ERROR_CODE(ret));
    if ((ret = tsn_qbridge_get_max_supported_vlans(bridge, &max_fid)) != E_TSN_LIB_NO_ERROR)
        return (E_TSN_LIB_MAX_FID|TSN_ERRNO_ERROR_CODE(ret));
    if (vid > max_vid) return E_TSN_LIB_MAX_VID_ERANGE;
    else if (fid > max_fid) return E_TSN_LIB_MAX_FID_ERANGE;
    ASSERT_FOPEN(fd, bridge, qbridge_fn[fd.id], O_RDWR);
    ASSERT_FWRENTRY(fd, fid, vid);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_qbridge_get_vid2fid(struct port_hdl *bridge, uint32_t vid,
                                         uint32_t *fid)
{
    tsn_file_desc fd = { .id = VID2FID_TABLE};
    tsn_return_value ret = E_TSN_LIB_NO_ERROR;
    unsigned int max_vid = 0;
    ASSERT_PARAMS(bridge && fid);
    if ((ret = tsn_qbridge_get_max_vlan_id(bridge, &max_vid)) != E_TSN_LIB_NO_ERROR)
        return (E_TSN_LIB_MAX_VID|TSN_ERRNO_ERROR_CODE(ret));
    if (vid > max_vid)
        return E_TSN_LIB_MAX_VID_ERANGE;
    ASSERT_FOPEN(fd, bridge, qbridge_fn[fd.id], O_RDONLY);
    ASSERT_FRDENTRY(fd, fid, vid);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_qbridge_get_fid2vid(struct port_hdl *bridge, uint32_t fid,
                                         uint32_t max_vids,
                                         struct qbr_fid2vid_entry *fid2vid)
{
    tsn_file_desc fd = { .id = FID2VID_TABLE};
    tsn_return_value ret = E_TSN_LIB_NO_ERROR;
    unsigned int max_fid = 0;
    ASSERT_PARAMS(bridge && fid2vid);
    if ((ret = tsn_qbridge_get_max_supported_vlans(bridge, &max_fid)) != E_TSN_LIB_NO_ERROR)
        return (E_TSN_LIB_MAX_FID|TSN_ERRNO_ERROR_CODE(ret));
    if (fid > max_fid)
        return E_TSN_LIB_MAX_FID_ERANGE;
    ASSERT_FOPEN(fd, bridge, qbridge_fn[fd.id], O_RDONLY);
    ASSERT_FRDSUBLIST(fd, fid2vid, fid, max_vids, max_vids);
    ASSERT_FCLOSE(fd);
}

const char *tsn_qbridge_get_known_ernno_msg(tsn_return_value return_code)
{
    (void) return_code;
    return read_lib_error_msg(E_TSN_LIB_NO_ERROR);
}

static uint32_t get_return_header(tsn_file_desc *fd)
{
    return ((tsn_module<<24)|((fd->id)<<16));
}
