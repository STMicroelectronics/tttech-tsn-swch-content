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
#include "tsn_sid.h"
#include "tsn_def.h"

#define IEEE8021SID_DIR "ieee8021SID/"

static const uint32_t tsn_module = TSN_SID;

static const sysfs_fn str_ident_fn [MAX_TSN_SID_FUNCTIONS] = {
    [SID_IDENT_PARAMS]  = IEEE8021SID_DIR"tsnSidIdentParams",
    [SID_DELETE]        = IEEE8021SID_DIR"tsnSidDelete",
    [SID_PORT_POS]      = IEEE8021SID_DIR"tsnSidPortPos",
    [SID_PORT_POS_DEL]  = IEEE8021SID_DIR"tsnSidPortPosDelete",
    [SID_SET_STRHDL]    = IEEE8021SID_DIR"tsnSidSetStrHdl",
    [SID_MAX_SUPPORTED] = IEEE8021SID_DIR"tsnSidMaxSupported",
    [SID_CURRENT_CNT]   = IEEE8021SID_DIR"tsnSidCnt",
    [SID_LIST_ENTRIES]  = IEEE8021SID_DIR"tsnListEntries",
    [SID_INPUT_CPS]     = IEEE8021SID_DIR"tsnCpsSidInputPackets",
    [SID_OUTPUT_CPS]    = IEEE8021SID_DIR"tsnCpsSidOutputPackets",
    [SID_TAB_LEN]       = IEEE8021SID_DIR"tsnSidTableLength",
    [SID_IDENT_TYPES]   = IEEE8021SID_DIR"tsnSupportedIdentTypes",
    [SID_SUP_PORTS]     = IEEE8021SID_DIR"tsnSidSupportedPorts",
};

tsn_return_value tsn_sid_add_ident(struct port_hdl *brdev, unsigned int ord,
                                   const struct ident_params *entry)
{
    tsn_file_desc fd = { .id = SID_IDENT_PARAMS};
    ASSERT_PARAMS(brdev && entry);
    ASSERT_FOPEN(fd, brdev, str_ident_fn[fd.id], O_WRONLY);
    ASSERT_FWRENTRY(fd, *entry, ord);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_sid_delete_entry(struct port_hdl *brdev, uint16_t ord)
{
    tsn_file_desc fd = { .id = SID_DELETE};
    uint16_t val = 0;
    ASSERT_PARAMS(brdev);
    ASSERT_FOPEN(fd, brdev, str_ident_fn[fd.id], O_WRONLY);
    ASSERT_FWRENTRY(fd, val, ord);
    ASSERT_FCLOSE(fd);
 }

tsn_return_value tsn_sid_add_port_pos(struct port_hdl *brdev, unsigned int ord,
                                      const struct port_pos *sid)
{
    tsn_file_desc fd = { .id = SID_PORT_POS};
    ASSERT_PARAMS(brdev && sid);
    ASSERT_FOPEN(fd, brdev, str_ident_fn[fd.id], O_WRONLY);
    ASSERT_FWRENTRY(fd, *sid, ord);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_sid_get_port_pos(struct port_hdl *brdev, unsigned int ord,
                                      sid_pos pos, uint16_t *res)
{
    tsn_file_desc fd =  { .id = SID_PORT_POS};
    struct port_list entry;
    ASSERT_PARAMS(brdev && res);
    ASSERT_FOPEN(fd, brdev, str_ident_fn[fd.id], O_RDONLY);
    ASSERT_FRDENTRY(fd, &entry, ord);

    if (pos == IN_FAC_OUT)
        *res = entry.in_fac_out;
    else if (pos == IN_FAC_IN)
        *res = entry.in_fac_in;
    else if (pos == OUT_FAC_IN)
        *res = entry.out_fac_in;
    else if (pos == OUT_FAC_OUT)
        *res = entry.out_fac_out;
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_sid_del_port_pos(struct port_hdl *brdev, unsigned int ord,
                                      const struct port_pos *sid)
{
    tsn_file_desc fd = { .id = SID_PORT_POS_DEL};
    ASSERT_PARAMS(brdev && sid);
    ASSERT_FOPEN(fd, brdev, str_ident_fn[fd.id], O_WRONLY);
    ASSERT_FWRENTRY(fd, *sid, ord);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_sid_set_strhdl(struct port_hdl *brdev, unsigned int ord,
                                    uint16_t strhdl)
{
    tsn_file_desc fd = { .id = SID_SET_STRHDL};
    ASSERT_PARAMS(brdev);
    ASSERT_FOPEN(fd, brdev, str_ident_fn[fd.id], O_WRONLY);
    ASSERT_FWRENTRY(fd, strhdl, ord);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_sid_get_current_cnt(struct port_hdl *brdev, uint32_t *cnt)
{
    tsn_file_desc fd = { .id = SID_CURRENT_CNT};
    ASSERT_PARAMS(brdev && cnt);
    ASSERT_FOPEN(fd, brdev, str_ident_fn[fd.id], O_RDONLY);
    ASSERT_FSCAN(fd, "%"SCNu32, 1, cnt);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_sid_get_tab_length(struct port_hdl *brdev, uint32_t *len)
{
    tsn_file_desc fd = { .id = SID_TAB_LEN};
    ASSERT_PARAMS(brdev && len);
    ASSERT_FOPEN(fd, brdev, str_ident_fn[fd.id], O_RDONLY);
    ASSERT_FSCAN(fd, "%"SCNu32, 1, len);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_sid_get_ident_type(struct port_hdl *brdev,
					sid_ident_type id_type,
					bool *supported)
{
    tsn_file_desc fd =  { .id = SID_IDENT_TYPES};
    uint8_t res;
    ASSERT_PARAMS(brdev && supported);
    ASSERT_FOPEN(fd, brdev, str_ident_fn[fd.id], O_RDONLY);
    ASSERT_FRDENTRY(fd, &res, id_type);
    *supported = (bool)res;
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_sid_get_supported_ports(struct port_hdl *brdev,
                                             uint16_t *port_mask)
{
    tsn_file_desc fd = { .id = SID_SUP_PORTS};
    ASSERT_PARAMS(brdev && port_mask);
    ASSERT_FOPEN(fd, brdev, str_ident_fn[fd.id], O_RDONLY);
    ASSERT_FSCAN(fd, "%"SCNu16, 1, port_mask);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_sid_get_max_sup(struct port_hdl *brdev, uint32_t *max_sup)
{
    tsn_file_desc fd = { .id = SID_MAX_SUPPORTED};
    ASSERT_PARAMS(brdev && max_sup);
    ASSERT_FOPEN(fd, brdev, str_ident_fn[fd.id], O_RDONLY);
    ASSERT_FSCAN(fd, "%"SCNu32, 1, max_sup);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_sid_get_entries_list(struct port_hdl *brdev, uint32_t cnt,
                                          uint16_t *ord)
{
    tsn_file_desc fd = { .id = SID_LIST_ENTRIES};
    ASSERT_PARAMS(brdev && ord);
    ASSERT_FOPEN_R(fd, brdev, str_ident_fn[fd.id]);
    ASSERT_FRDLIST(fd, ord, cnt);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_sid_get_input_cps_cnt(struct port_hdl *brdev,
                                           uint32_t str_hdl,
                                           uint32_t *cnt_val)
{
    tsn_file_desc fd = { .id = SID_INPUT_CPS};
    ASSERT_PARAMS(brdev && cnt_val);
    ASSERT_FOPEN(fd, brdev, str_ident_fn[fd.id], O_RDONLY);
    ASSERT_FRDENTRY(fd, cnt_val, str_hdl);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_sid_get_output_cps_cnt(struct port_hdl *brdev,
                                            uint32_t str_hdl,
                                            uint32_t *cnt_val)
{
    tsn_file_desc fd = { .id = SID_OUTPUT_CPS};
    ASSERT_PARAMS(brdev && cnt_val);
    ASSERT_FOPEN(fd, brdev, str_ident_fn[fd.id], O_RDONLY);
    ASSERT_FRDENTRY(fd, cnt_val, str_hdl);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_sid_get_ident_params(struct port_hdl *brdev,
                                          unsigned int ord,
                                          struct ident_params *entry)
{
    tsn_file_desc fd =  { .id = SID_IDENT_PARAMS};
    ASSERT_PARAMS(brdev && entry);
    ASSERT_FOPEN(fd, brdev, str_ident_fn[fd.id], O_RDONLY);
    ASSERT_FRDENTRY(fd, entry, ord);
    ASSERT_FCLOSE(fd);
}

tsn_return_value tsn_sid_get_infacin(struct port_hdl *br, uint16_t strhdl,
                                     uint16_t *res)
{
    return tsn_sid_get_port_pos(br, strhdl, IN_FAC_IN, res);
}

tsn_return_value tsn_sid_get_infacout(struct port_hdl *br, uint16_t strhdl,
                                      uint16_t *res)
{
    return tsn_sid_get_port_pos(br, strhdl, IN_FAC_OUT, res);
}

tsn_return_value tsn_sid_get_outfacin(struct port_hdl *br, uint16_t ord,
                                      uint16_t *res)
{
    return tsn_sid_get_port_pos(br, ord, OUT_FAC_IN, res);
}

tsn_return_value tsn_sid_get_outfacout(struct port_hdl *br, uint16_t strhdl,
                                       uint16_t *res)
{
    return tsn_sid_get_port_pos(br, strhdl, OUT_FAC_OUT, res);
}

const char *tsn_sid_get_known_ernno_msg(tsn_return_value return_code)
{
    (void) return_code;
    return read_sid_error_msg(return_code);
}

static uint32_t get_return_header(tsn_file_desc *fd)
{
    return ((fd-> id < MAX_TSN_SID_FUNCTIONS) ?
            (tsn_module<<24)|((fd->id)<<16) : E_TSN_LIB_NO_ERROR);
}
