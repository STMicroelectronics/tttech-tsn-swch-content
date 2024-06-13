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
#include <net/if.h>
#include <sys/types.h>
#include <dirent.h>
#include "tsn_private.h"
#include "tsn_st.h"
#include "tsn_preemp.h"
#include "tsn_bridge.h"
#include "tsn_psfp.h"

/* We dont have specific error codes here */
static const uint32_t tsn_module = 0;

struct port_hdl *tsn_port_open_by_name(const char *ifname) {
    struct port_hdl *phdl = NULL;
    if (if_nametoindex(ifname) != 0) {
        phdl = malloc(sizeof(*phdl));
        if (phdl) {
            strlcpy(phdl->sysfs_path, SYSFS_PORT_BASE, sizeof(phdl->sysfs_path));
            strlcat(phdl->sysfs_path, ifname, sizeof(phdl->sysfs_path));
        }
    }
    return phdl;
}

struct port_hdl *tsn_port_open_by_index(unsigned int ifindex) {
    struct port_hdl *phdl = NULL;
    char buf[IF_NAMESIZE];
    if (if_indextoname(ifindex, buf) != NULL)
        phdl = tsn_port_open_by_name(buf);
    return phdl;
}

char *tsn_get_name_by_hdl(struct port_hdl *port) {
    return (char*)(port->sysfs_path + strlen(SYSFS_PORT_BASE));
}

char *tsn_get_name_by_id(struct port_hdl *bridge, uint32_t port_id) {
    struct port_hdl phdl = *bridge;
    uint32_t p_id = TSN_PORT_ID_INVALID;
    char *port_name = NULL;
    DIR *dirp;
    struct dirent *dp;
    uint32_t ret = 0u;

    dirp = opendir(bridge->sysfs_path);

    while (dirp) {
    if ((dp = readdir(dirp)) != NULL) {
        if (strncmp(dp->d_name, "lower_", (size_t)6) == 0) {
            strncat(phdl.sysfs_path, "/", sizeof(phdl.sysfs_path) - 1);
            strncat(phdl.sysfs_path, dp->d_name, sizeof(phdl.sysfs_path) - 1);
            ret = tsn_bridge_port_id(&phdl, &p_id);
            if ((!ret) && (p_id == port_id)) break;
        }
    }
    else {
        ret = ENOENT;
        break;
    }
    phdl = *bridge;
    }
    closedir(dirp);
    if (!ret) {
        port_name = (char*)malloc(sizeof(char)*IF_NAMESIZE); /* User is responsible to free the memory!!! */
        if (port_name)
            strncpy(port_name, (char*)(phdl.sysfs_path + strlen(bridge->sysfs_path) + strlen("/lower_")), (size_t)(IF_NAMESIZE-1));
    }
    return port_name;
}

uint32_t tsn_get_id_by_name(struct port_hdl *bridge, char *port_name) {
    struct port_hdl phdl;
    uint32_t ret = 0u;
    uint32_t port_id = TSN_PORT_ID_INVALID;

    phdl = *bridge;
    strncat(phdl.sysfs_path, "/lower_", sizeof(phdl.sysfs_path) - 1);
    strncat(phdl.sysfs_path, port_name, sizeof(phdl.sysfs_path) - 1);
    ret = tsn_bridge_port_id(&phdl, &port_id);
    if (ret != 0u) {
        /* port_id not found */
        port_id = TSN_PORT_ID_INVALID;
    }
    return port_id;
}

int compare_function(const void *value1, const void *value2) {
    if (*(uint32_t*)value1 > *(uint32_t*)value2) return  1;
    if (*(uint32_t*)value1 < *(uint32_t*)value2) return -1;
    return 0;

}

uint32_t *tsn_get_list_port_ids(struct port_hdl *bridge, uint32_t *num_ports) {
    DIR *dirp;
    struct dirent *dp;
    uint32_t pt_id, ret = 0u;
    struct port_hdl phdl_port = *bridge;
    uint32_t *list_pt_id, *work_pt;
    struct bridge_attr_and_caps caps = {0};
    int i = 0;

    *num_ports = 0;

    /* find all ports of the bridge and write port id's into list */
    /* read number of ports bridge has */
    if ((ret = tsn_bridge_attr(bridge, &caps)) != 0u) {
        return NULL;
    }
    else {
        *num_ports = caps.BridgeNumPorts;
    }

    /* allocate memory for the list of port ids; User is responsible to free the memory!!! */
    list_pt_id = (uint32_t*)malloc(sizeof(uint32_t) * caps.BridgeNumPorts);
    if (!list_pt_id) {
        *num_ports = 0;
        return NULL;
    }

    dirp = opendir(bridge->sysfs_path);
    work_pt = list_pt_id;

    while ((dirp) && (i < caps.BridgeNumPorts)) {
        /* find all ports of bridge, read port id's and write them to the list */
        if ((dp = readdir(dirp)) != NULL) {
            if (strncmp(dp->d_name, "lower_", (size_t)6) == 0) {
                strncat(phdl_port.sysfs_path, "/", sizeof(phdl_port.sysfs_path) - 1);
                strncat(phdl_port.sysfs_path, dp->d_name, sizeof(phdl_port.sysfs_path) - 1);
                ret = tsn_bridge_port_id(&phdl_port, &pt_id);
                if (ret == 0u) {
                    *work_pt = pt_id;
                    i++;
                    work_pt++;
                }
                phdl_port = *bridge;
            }
        }
        else {
            /* EOF reached or no file in directory.*/
            break;
        }
    }

    closedir(dirp);

    if (i != caps.BridgeNumPorts) {
        /* read number of ports don't fit to ports found in sysfs */
        *num_ports = 0;
        free(list_pt_id);
        return NULL;
    }
    /* all port ids put to list */
    qsort(list_pt_id, caps.BridgeNumPorts, sizeof(uint32_t), compare_function);
    return list_pt_id;

}

void tsn_port_close(struct port_hdl *phdl) {
    if (phdl)
        free(phdl);
}

bool tsn_is_feat_supported(struct port_hdl *phdl, enum tsn_feat feat_req) {
    bool rbool;
    uint32_t ru32;
    int r;

    switch (feat_req) {
        case FEAT_ST:
            r = tsn_st_get_gate_enabled(phdl, &rbool);
            break;
        case FEAT_PREE:
            r = tsn_preemp_get_hold_advance(phdl, &ru32);
            break;
        case FEAT_MACMERGE:
            r = tsn_preemp_get_support(phdl, &rbool);
            if ((!r) && (!rbool)) /* if (readable) object reports 'not supported' ... */
                r = -1;           /* ... we treat it like an errornous return value.  */
            break;
        default:
            r = -1; /* force unsupported */
    }

    return (r == 0);
}

const char *tsn_lib_get_known_ernno_msg(tsn_return_value error_code) {
    const char *ret_msg = NULL;
    (void)get_return_header;
    /* Get module raising the error */
    if (TSN_GET_ERROR_INDEX(error_code) != E_TSN_LIB_NO_ERROR) {
        switch (TSN_MODULE_INDEX_CODE(error_code)) {
            case TSN_BRIDGE:
                ret_msg = tsn_bridge_get_known_ernno_msg(error_code);
                break;
            case TSN_EDGEX:
                ret_msg = tsn_edgex_get_known_ernno_msg(error_code);
                break;
            case TSN_FQTSS:
                ret_msg = tsn_fqtss_get_known_ernno_msg(error_code);
                break;
            case TSN_FRER:
                ret_msg = tsn_frer_get_known_ernno_msg(error_code);
                break;
            case TSN_MSTP:
                ret_msg = tsn_mstp_get_known_ernno_msg(error_code);
                break;
            case TSN_PREEMP:
                ret_msg = tsn_preemp_get_known_ernno_msg(error_code);
                break;
            case TSN_QBRIDGE:
                ret_msg = tsn_qbridge_get_known_ernno_msg(error_code);
                break;
            case TSN_SID:
                ret_msg = tsn_sid_get_known_ernno_msg(error_code);
                break;
            case TSN_ST:
                ret_msg = tsn_st_get_known_ernno_msg(error_code);
                break;
            case TSN_PSFP:
                ret_msg = tsn_psfp_get_known_ernno_msg(error_code);
                break;
            default:
                ret_msg = read_lib_error_msg(E_TSN_LIB_NO_ERROR);
        }
    }
    else {
        ret_msg = strerror(TSN_ERRNO_ERROR_CODE(error_code));
    }
    return ret_msg;
}

/* We dont have specific error codes here */
static uint32_t get_return_header(tsn_file_desc *fd) {
    (void)fd;
    (void)tsn_module;
    return E_TSN_LIB_NO_ERROR;
}
