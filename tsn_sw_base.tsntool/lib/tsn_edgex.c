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

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <inttypes.h>

#include "tsn_private.h"
#include "tsn_edgex.h"

#define EDGEX_DIR "edgex-ext/"

static const uint32_t tsn_module = TSN_EDGEX;

static const sysfs_fn edgex_fn[MAX_TSN_EDGEX_FUNCTIONS] = {
    [CUT_THROUGH] = EDGEX_DIR"cutThrough",
    [MIRROR_PORT] = EDGEX_DIR"mirrorPort",
};

tsn_return_value tsn_edgex_get_cut_through(struct port_hdl *port,
                                           bool *cut_through)
{
    tsn_file_desc fd = { .id = CUT_THROUGH};
    bool _ct;
    ASSERT_PARAMS(port && cut_through);
    ASSERT_FOPEN(fd, port, edgex_fn[fd.id], O_RDONLY);
    ASSERT_FSCAN(fd, "%"SCNu8, 1, (uint8_t *)&_ct);
    *cut_through = _ct;
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_edgex_set_cut_through(struct port_hdl *port,
                                           const bool cut_through)
{
    tsn_file_desc fd = { .id = CUT_THROUGH};
    ASSERT_PARAMS(port);
    ASSERT_FOPEN(fd, port, edgex_fn[fd.id], O_RDWR);
    ASSERT_FPRINT(fd, "%"PRIu8, (uint8_t)cut_through);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_edgex_set_mirror_portid(struct port_hdl *port,
                                             const int mirr_port_id)
{
    tsn_file_desc fd = { .id = MIRROR_PORT};
    ASSERT_PARAMS(port);
    ASSERT_FOPEN(fd, port, edgex_fn[fd.id], O_RDWR);
    ASSERT_FPRINT(fd, "%d", mirr_port_id);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_edgex_get_mirror_portid(struct port_hdl *port,
                                             int *mirr_port_id)
{
    tsn_file_desc fd = { .id = MIRROR_PORT};
    ASSERT_PARAMS(port && mirr_port_id);
    ASSERT_FOPEN(fd, port, edgex_fn[fd.id], O_RDONLY);
    ASSERT_FSCAN(fd, "%d", 1, mirr_port_id);
    ASSERT_FCLOSE(fd);
}

const char *tsn_edgex_get_known_ernno_msg(tsn_return_value return_code)
{
    (void) return_code;
    return read_lib_error_msg(E_TSN_LIB_NO_ERROR);
}

static uint32_t get_return_header(tsn_file_desc *fd)
{
    return ((tsn_module<<24)|((fd->id)<<16));
}
