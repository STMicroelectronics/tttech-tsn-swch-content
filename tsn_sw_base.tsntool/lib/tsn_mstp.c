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
#include "tsn_mstp.h"
#include "tsn_qbridge.h"

#define IEEE8021MSTP_DIR "ieee8021Mstp/"

#define _MAX_MSTID 0xFFE

static const uint32_t tsn_module = TSN_MSTP;

static const sysfs_fn mstp_fn[MAX_TSN_MSTP_FUNCTIONS] = {
    [FLUSH_TREE] 	= IEEE8021MSTP_DIR"flushTree",
    [MAX_MSITD] 	= IEEE8021MSTP_DIR"maxMsti",
    [CREATE_MSTI] 	= IEEE8021MSTP_DIR"createMsti",
    [DELETE_MSTI] 	= IEEE8021MSTP_DIR"deleteMsti",
    [GET_FID2MSTI] 	= IEEE8021MSTP_DIR"fid2msti",
    [SET_FID2MSTI] 	= IEEE8021MSTP_DIR"fid2msti",
    [GET_MSTI_LIST] 	= IEEE8021MSTP_DIR"mstiList",
    [GET_PORT_STATE] 	= IEEE8021MSTP_DIR"treePortState",
    [SET_PORT_STATE] 	= IEEE8021MSTP_DIR"treePortState",
};

tsn_return_value tsn_mstp_flush_tree_port(struct port_hdl *port,
                                          unsigned int mstid)
{
    tsn_file_desc fd = { .id = FLUSH_TREE};
    ASSERT_PARAMS(port);
    struct port_hdl master_br = *port;
    strlcat(master_br.sysfs_path, "/master", sizeof(master_br.sysfs_path));
    if (mstid > _MAX_MSTID)
        return E_TSN_LIB_MAX_MSTI_ERANGE;
    ASSERT_FOPEN(fd, port, mstp_fn[fd.id], O_WRONLY);
    ASSERT_FPRINT(fd, "%u", mstid);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_mstp_get_max_msti(const struct port_hdl *port,
                                       unsigned int * const num_msti)
{
    tsn_file_desc fd = { .id = MAX_MSITD};
    ASSERT_PARAMS(port && num_msti);
    ASSERT_FOPEN(fd, port, mstp_fn[fd.id], O_RDONLY);
    ASSERT_FSCAN(fd, "%u", 1, num_msti);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_mstp_create_msti(const struct port_hdl *port,
                                      const unsigned int mstid)
{
    tsn_file_desc fd = { .id = CREATE_MSTI};
    ASSERT_PARAMS(port);
    if (mstid > _MAX_MSTID)
        return E_TSN_LIB_MAX_MSTI_ERANGE;
    ASSERT_FOPEN(fd, port, mstp_fn[fd.id], O_WRONLY);
    ASSERT_FPRINT(fd, "%u", mstid);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_mstp_delete_msti(const struct port_hdl *port,
                                      const unsigned int mstid)
{
    tsn_file_desc fd = { .id = DELETE_MSTI};
    ASSERT_PARAMS(port);
    if (mstid > _MAX_MSTID)
        return E_TSN_LIB_MAX_MSTI_ERANGE;
    ASSERT_FOPEN(fd, port, mstp_fn[fd.id], O_WRONLY);
    ASSERT_FPRINT(fd, "%u", mstid);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_mstp_get_fid_to_msti(const struct port_hdl *port,
                                          const unsigned int fid,
                                          unsigned int * const mstid)
{
    tsn_file_desc fd = { .id = GET_FID2MSTI};
    tsn_return_value ret = E_TSN_LIB_NO_ERROR;
    unsigned int max_fid = 0;
    ASSERT_PARAMS(port && mstid);
    if ((ret = tsn_qbridge_get_max_supported_vlans((struct port_hdl *)port, &max_fid)) != E_TSN_LIB_NO_ERROR)
        return (E_TSN_LIB_MAX_FID|TSN_ERRNO_ERROR_CODE(ret));
    if (fid >= max_fid)
        return E_TSN_LIB_MAX_FID_ERANGE;
    ASSERT_FOPEN(fd, port, mstp_fn[fd.id], O_RDONLY);
    ASSERT_FRDENTRY(fd, mstid, fid);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_mstp_set_fid_to_msti(const struct port_hdl *port,
                                          const unsigned int fid,
                                          const unsigned int mstid)
{
    tsn_file_desc fd= { .id = SET_FID2MSTI};
    tsn_return_value ret = E_TSN_LIB_NO_ERROR;
    unsigned int max_fid = 0;
    ASSERT_PARAMS(port);
    if ((ret = tsn_qbridge_get_max_supported_vlans(port, &max_fid)) != E_TSN_LIB_NO_ERROR)
        return (E_TSN_LIB_MAX_FID|TSN_ERRNO_ERROR_CODE(ret));
    if (mstid > _MAX_MSTID) return E_TSN_LIB_MAX_MSTI_ERANGE;
    else if (fid > max_fid) return E_TSN_LIB_MAX_FID_ERANGE;
    ASSERT_FOPEN(fd, port, mstp_fn[fd.id], O_RDWR);
    ASSERT_FWRENTRY(fd, mstid, fid);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_mstp_get_msti_list(const struct port_hdl *port,
                                        struct msti_list_entry * msti_list)
{
    tsn_file_desc fd = { .id = GET_MSTI_LIST};
    tsn_return_value ret = E_TSN_LIB_NO_ERROR;
    unsigned int max_msti = 0;
    ASSERT_PARAMS(port && msti_list);
    if ((ret = tsn_mstp_get_max_msti(port, &max_msti)) != E_TSN_LIB_NO_ERROR)
        return (E_TSN_LIB_MAX_MSTI|TSN_ERRNO_ERROR_CODE(ret));
    ASSERT_FOPEN(fd, port, mstp_fn[fd.id], O_RDONLY);
    ASSERT_FRDLIST(fd, msti_list, max_msti);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_mstp_get_port_state(const struct port_hdl *port,
                                         const unsigned int mstid,
                                         port_mstp_state * const port_state)
{
    tsn_file_desc fd = { .id = GET_PORT_STATE};
    unsigned int state = 0; // Blocking is default state
    ASSERT_PARAMS(port && port_state);
    struct port_hdl master_br = *port;
    strlcat(master_br.sysfs_path, "/master", sizeof(master_br.sysfs_path));
    if (mstid > _MAX_MSTID)
        return E_TSN_LIB_MAX_MSTI_ERANGE;
    ASSERT_FOPEN(fd, port, mstp_fn[fd.id], O_RDONLY);
    ASSERT_FRDENTRY(fd, &state, mstid);
    *port_state = (port_mstp_state) state;
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_mstp_set_port_state(const struct port_hdl *port,
                                         const unsigned int mstid,
                                         const port_mstp_state port_state)
{
    tsn_file_desc fd = { .id = SET_PORT_STATE};
    unsigned int state = (unsigned int) port_state;
    ASSERT_PARAMS(port);
    struct port_hdl master_br = *port;
    strlcat(master_br.sysfs_path, "/master", sizeof(master_br.sysfs_path));
    if (mstid > _MAX_MSTID)
        return E_TSN_LIB_MAX_MSTI_ERANGE;
    ASSERT_FOPEN(fd, port, mstp_fn[fd.id], O_RDWR);
    ASSERT_FWRENTRY(fd, state, mstid);
    ASSERT_FCLOSE(fd);
}

const char *tsn_mstp_get_known_ernno_msg(tsn_return_value return_code)
{
    (void) return_code;
    return read_lib_error_msg(E_TSN_LIB_NO_ERROR);
}

static uint32_t get_return_header(tsn_file_desc *fd)
{
    return ((tsn_module<<24)|((fd->id)<<16));
}
