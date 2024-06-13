/*
 * TTTech ieee802-dot1ab-lldp-yang-module
 * Copyright(c) 2019 TTTech Industrial Automation AG.
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

/* common includes */
#include "common_defines.h"
#include "common.h"
/* sysrepo includes */
#include <sysrepo.h>
//#include <sysrepo/plugins.h>
#include <sysrepo/values.h>
#include <sysrepo/xpath.h>
/* libbase includes */
#include "libbase/base_defines.h"
#include "libbase/base_prints.h"
#include "libbase/base_private_translators.h"
#include "libbase/base_linux_tool_helpers.h"
#include "libbase/base_private_syscalls.h"
/* liblldpctl includes */
#include "lldpctl.h"
#include "lldp-const.h"
/* module specific includes */
#include "lldp_defines.h"

/* structure holding all the subscriptions */
sr_subscription_ctx_t *subscription;

/* Variable to detect is callback triggered for the first time The startup
 * datastore is copied to the running before this plugin is initialized. So, at
 * first time plugin started we need just to copy data to startup and running.
 * It is not necessary to trigger setter function. "plugin_init" will be '0' if
 * plugin is not initialized. Set "plugin_init" to '1' after is initialized.
 * The will be after fill startup datastore function.
 */
int plugin_init = 0;

/**
 * @brief This function checks if port with desired dest-mac-address exists on
 * system.
 *
 * @param[in]   conn        Structure referencing a connection with lldpd.
 * @param[in]   name        Port name.
 * @param[in]   address     Port dest-mac-address.
 * @return      Returns port (pointer to lldpctl_atom_t struct) if founded,
 *              NULL instead.
 */
lldpctl_atom_t *check_port(lldpctl_conn_t *conn, char *name, char *address)
{
    lldpctl_atom_t *tmp_atom = NULL;
    lldpctl_atom_t *port = NULL;
    lldpctl_atom_t *interface = NULL;
    char port_name[MAX_STR_LEN] = "";
    char port_address[MAX_STR_LEN] = "";

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    tmp_atom = lldpctl_get_interfaces(conn);
    lldpctl_atom_foreach(tmp_atom, interface) {
        port = lldpctl_get_port(interface);
        snprintf(port_name, MAX_STR_LEN, "%s", lldpctl_atom_get_str(port, lldpctl_k_port_name));
        snprintf(port_address, MAX_STR_LEN, "%s", lldpctl_atom_get_str(port, lldpctl_k_port_id));
        change_addr_colons_to_dashes(port_address);
        if ((strncmp(name, port_name, MAX_STR_LEN) == 0) &&
            (strncmp(address, port_address, MAX_STR_LEN) == 0)) {
            return port;
        }
    }
    lldpctl_atom_dec_ref(tmp_atom);

    return NULL;
}

/**
 * @brief This function checks if neighbor with desired remote-index exists
 * inside desired port.
 *
 * @param[in]   port                Structure representing the port.
 * @param[in]   remote_index        Neighbors remote-index.
 * @return[in]  Returns neighbor (pointer to lldpctl_atom_t struct) if founded,
 *              NULL instead.
 */
lldpctl_atom_t *check_neighbor(lldpctl_atom_t *port, char *remote_index)
{
    lldpctl_atom_t *neighbors;
    lldpctl_atom_t *neighbor;

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    neighbors = lldpctl_atom_get(port, lldpctl_k_port_neighbors);

    lldpctl_atom_foreach(neighbors, neighbor) {
        /* If neighbor with remote_index is find inside port, return it */
        if (strncmp((char*)lldpctl_atom_get_str(neighbor, lldpctl_k_chassis_index),
                    remote_index, MAX_STR_LEN) == 0) {
            return neighbor;
        }
    }
    lldpctl_atom_dec_ref(neighbors);

    return NULL;
}

#if 0
/**
 * @brief This function fills in the port name and dest-mac-address.
 *
 * @param[in]   port        Structure representing the port.
 * @param[in]   name        Port name to be filled.
 * @param[in]   address     Port dest-mac-address to be filled.
  */
static void get_port_name_and_address(lldpctl_atom_t *port, char *name,
                                      char *address)
{
  SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

  strncpy(name,lldpctl_atom_get_str(port, lldpctl_k_port_name), (MAX_STR_LEN-1));

  strncpy(address, lldpctl_atom_get_str(port, lldpctl_k_port_id), (MAX_STR_LEN-1));

  change_addr_colons_to_dashes(address);
}
#endif

/**
 * @brief This function fills system-capabilities-supported and
 * system-capabilities-enabled from container local-system-data.
 *
 * @param[out]  caps_supported  Pointer to char that will contain
 *                              system-capabilities-supported.
 * @param[out]  caps_enabled    Pointer to char that will contain
 *                              system-capabilities-enabled.
 * @param[in]   lcl_chassis     Pointer to lldpctl_atom_t which represents
 *                              information related to the local chassis.
 * @return      EXIT_SUCCESS.
 */
static int fill_sys_caps_data(char *caps_supported, char *caps_enabled,
                              lldpctl_atom_t *lcl_chassis)
{
    char supported_capabilities_array[SYS_CAPABILITY_MAX_NUM][SYS_CAPABILITY_LENGTH] = {0};
    char enabled_capabilities_array[SYS_CAPABILITY_MAX_NUM][SYS_CAPABILITY_LENGTH] = {0};
    int i = 0;
    int j = 0;
    int k = 0;

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    for (k = 0 ; k < 8 ; k++) {
        if (lldpctl_atom_get_int(lcl_chassis, lldpctl_k_chassis_cap_available) & sys_cap_bits[k]) {
            size_t len = strlen(sys_cap_str_array[k]);
            memcpy(supported_capabilities_array[i], sys_cap_str_array[k], len);
            i++;
            if (lldpctl_atom_get_int(lcl_chassis, lldpctl_k_chassis_cap_enabled) & sys_cap_bits[k]) {
                memcpy(enabled_capabilities_array[j], sys_cap_str_array[k], len);
                j++;
            }
        }
    }

    /* create sys-cap-supported value */
    i = 0;
    size_t pos = 0;
    while ((i < SYS_CAPABILITY_MAX_NUM) &&
           (0 != strncmp(supported_capabilities_array[i], "", SYS_CAPABILITY_LENGTH))) {
        if (0 == i) {
            // first element
            memset(caps_supported, 0, MAX_STR_LEN);
        }
        else {
            // any other element
            caps_supported[pos++] = ' ';
        }
        size_t len = strlen(supported_capabilities_array[i]);
        memcpy(&caps_supported[pos],
               supported_capabilities_array[i],
               len);
        pos += len;
        i++;
    }

    /* create sys-cap-enabled value */
    i = 0;
    pos = 0;
    while ((i < SYS_CAPABILITY_MAX_NUM) &&
           (0 != strncmp(enabled_capabilities_array[i], "", SYS_CAPABILITY_LENGTH))) {
        if (0 == i) {
            // first element
            memset(caps_enabled, 0, MAX_STR_LEN);
        } else {
            // any other element
            caps_enabled[pos++] = ' ';
        }
        size_t len = strlen(enabled_capabilities_array[i]);
        memcpy(&caps_enabled[pos],
               enabled_capabilities_array[i],
               len);
        pos += len;
        i++;
    }

    return EXIT_SUCCESS;
}

/**
 * @brief Callback to be called by the event of changing any running datastore
 * content within the module.
 *
 * @param[in]   session         Implicit session (do not stop) with information
 *                              about the changed data (retrieved by
 *                              sr_get_changes_iter) the event originator
 *                              session IDs.
 * @param[in]   module_name     Name of the module where the change has
 *                              occurred.
 * @param[in]   xpath           XPath used when subscribing, NULL if the whole
 *                              module was subscribed to.
 * @param[in]   event           Type of the notification event that has
 *                              occurred.
 * @param[in]   request_id      Request ID unique for the specific module_name.
 *                              Connected events for one request (SR_EV_CHANGE
 *                              and SR_EV_DONE, for example) have the same
 *                              request ID.
 * @param[in]   private_data    Private context opaque to sysrepo, as passed to
 *                              sr_module_change_subscribe call.
 * @return      Error code (SR_ERR_OK on success).
 */
static int module_change_cb(sr_session_ctx_t *session, const char *module_name,
                            const char *xpath, sr_event_t event,
                            uint32_t request_id, void *private_data)
{
    (void)session;
    (void)module_name;
    (void)xpath;
    (void)event;
    (void)request_id;
    (void)private_data;

    return SR_ERR_OK;
#if 0
    sr_val_t *values = NULL;
    size_t count = 0;
    int rc = SR_ERR_OK;

    SRP_LOG_DBGMSG("Retrieve current configuration.\n");

    rc = sr_get_items(session, "/ieee802-dot1ab-lldp:ieee802-dot1ab-lldp//*",
                      0, &values, &count);
    if (SR_ERR_OK != rc) {
        SRP_LOG_ERR("Error by sr_get_items: %s", sr_strerror(rc));
        return rc;
    }
    for (size_t i = 0; i < count; i++){
        sr_print_val(&values[i]);
    }
    sr_free_values(values, count);

    return rc;
#endif
}

/**
 * @brief Creates a group of the leafs according to container
 * "local-system-data" from ieee802-dot1ab-lldp yang module..
 *
 * @param[out]  parent          Pointer to an existing parent of the requested
 *                              nodes. Is NULL for top-level nodes. Caller is
 *                              supposed to append the requested nodes to this
 *                              data subtree and return either the original
 *                              parent or a top-level node.
 * @param[in]   lcl_chassis     The pointer to lldpctl_atom_t which represents
 *                              the port for which list should be created.
 * @return      Error code (SR_ERR_OK on success).
 */
static int create_local_system_data_leafs(struct lyd_node **parent,
                                          lldpctl_atom_t *lcl_chassis)
{
    const char *tmp_str = NULL;
    char tmp_value[MAX_STR_LEN] = {0};
    char caps_supported[MAX_STR_LEN] = {0};
    char caps_enabled[MAX_STR_LEN] = {0};

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    /* leaf chassis-id-subtype */
    tmp_str = lldpctl_atom_get_str(lcl_chassis, lldpctl_k_chassis_id_subtype);
    if (tmp_str) {
        snprintf(tmp_value, MAX_STR_LEN, "%s", tmp_str);
        if (EXIT_SUCCESS == translate_single_string_using_map(tmp_value, chassis_id_subtype_map)) {
            if (NULL == lyd_new_path(*parent, NULL, LLDP_LOC_SYS_DATA_CHASSIS_ID_SUBTYPE_XPATH, tmp_value, 0, 0)) {
                SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                            LLDP_CHASSIS_ID_SYBTYPE_STR, ERR_NOT_CREATED_ELEMENT_STR);
            }
        }
    }

    /* leaf chassis-id */
    tmp_str = lldpctl_atom_get_str(lcl_chassis, lldpctl_k_chassis_id);
    if (tmp_str) {
        if (NULL == lyd_new_path(*parent, NULL, LLDP_LOC_SYS_DATA_CHASSIS_ID_XPATH, (char*)tmp_str, 0, 0)) {
            SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                        LLDP_CHASSIS_ID_STR, ERR_NOT_CREATED_ELEMENT_STR);
        }
    }

    /* leaf system-name */
    tmp_str = lldpctl_atom_get_str(lcl_chassis, lldpctl_k_chassis_name);
    if (tmp_str) {
        if (NULL == lyd_new_path(*parent, NULL, LLDP_LOC_SYS_DATA_SYS_NAME_XPATH, (char*)tmp_str, 0, 0)) {
            SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                        LLDP_SYS_NAME_STR, ERR_NOT_CREATED_ELEMENT_STR);
        }
    }

    /* leaf system-description */
    tmp_str = lldpctl_atom_get_str(lcl_chassis, lldpctl_k_chassis_descr);
    if (tmp_str) {
        if (NULL == lyd_new_path(*parent, NULL, LLDP_LOC_SYS_DATA_SYS_DESC_XPATH, (char*)tmp_str, 0, 0)) {
            SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                        LLDP_SYS_DESCR_STR, ERR_NOT_CREATED_ELEMENT_STR);
        }
    }

    /* leafs  system-capabilities-supported and system-capabilities-enabled */
    if (EXIT_SUCCESS == fill_sys_caps_data(caps_supported, caps_enabled, lcl_chassis)) {
        if (0 != strncmp(caps_supported, "", MAX_STR_LEN)) {
            if (NULL == lyd_new_path(*parent, NULL, LLDP_LOC_SYS_DATA_SYS_CAP_XPATH, caps_supported, 0, 0)) {
                SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                            LLDP_SYS_CAP_SUPPPORTED_STR, ERR_NOT_CREATED_ELEMENT_STR);
            }
        }

        if (0 != strncmp(caps_enabled, "", MAX_STR_LEN)) {
            if (NULL == lyd_new_path(*parent, NULL, LLDP_LOC_SYS_DATA_SYS_CAP_EN_XPATH, caps_enabled, 0, 0)) {
                SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                            LLDP_SYS_CAP_ENABLED_STR, ERR_NOT_CREATED_ELEMENT_STR);
            }
        }
    }

    return SR_ERR_OK;
}

/**
 * @brief Creates a new container "rx-statistics" from ieee802-dot1ab-lldp yang
 * module.
 *
 * @param[out]  parent              Pointer to an existing parent of the
 *                                  requested nodes. Is NULL for top-level
 *                                  nodes. Caller is supposed to append the
 *                                  requested nodes to this data subtree and
 *                                  return either the original parent or a
 *                                  top-level node.
 * @param[in]   port                The pointer to lldpctl_atom_t which
 *                                  represents the port for which list should
 *                                  be created.
 * @param[in]   port_name           Port name.
 * @param[in]   dest_mac_address    Port mac address.
 * @return      Error code (SR_ERR_OK on success).
 */
static int new_rx_statistics(struct lyd_node **parent, lldpctl_atom_t *port,
                             char *port_name, char *dest_mac_address)
{
    char path[MAX_STR_LEN] = "";
    char tmp_value[MAX_STR_LEN] = "";

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    /* leaf total-ageouts */
    snprintf(tmp_value, MAX_STR_LEN, "%ld", lldpctl_atom_get_int(port, lldpctl_k_ageout_cnt));
    fill_xpath(path, LLDP_PORT_RX_STAT_TOT_AGEOUTS_XPATH, port_name, dest_mac_address);
    if (NULL == lyd_new_path(*parent, NULL, path, tmp_value, 0, 0)) {
        SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                    LLDP_TOTAL_AGEOUTS_STR, ERR_NOT_CREATED_ELEMENT_STR);
    }

    /* leaf total-discarded-frames */
    snprintf(tmp_value, MAX_STR_LEN, "%ld", lldpctl_atom_get_int(port, lldpctl_k_rx_discarded_cnt));
    fill_xpath(path, LLDP_PORT_RX_STAT_TOT_DIS_FRAMES_XPATH, port_name, dest_mac_address);
    if (NULL == lyd_new_path(*parent, NULL, path, tmp_value, 0, 0)) {
        SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                    LLDP_TOTAL_DSCRD_FRAMES_STR, ERR_NOT_CREATED_ELEMENT_STR);
    }

    /* leaf total-frames */
    snprintf(tmp_value, MAX_STR_LEN, "%ld", lldpctl_atom_get_int(port, lldpctl_k_rx_cnt));
    fill_xpath(path, LLDP_PORT_RX_STAT_TOT_FRAMES_XPATH, port_name, dest_mac_address);
    if (NULL == lyd_new_path(*parent, NULL, path, tmp_value, 0, 0)) {
        SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                    LLDP_TOTAL_FRAMES_STR, ERR_NOT_CREATED_ELEMENT_STR);
    }

    /* leaf total-discarded-tlvs */
    snprintf(tmp_value, MAX_STR_LEN, "%ld", lldpctl_atom_get_int(port, lldpctl_k_rx_discarded_cnt));
    fill_xpath(path, LLDP_PORT_RX_STAT_TOT_DIS_TLVS_XPATH, port_name, dest_mac_address);
    if (NULL == lyd_new_path(*parent, NULL, path, tmp_value, 0, 0)) {
        SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                    LLDP_TOTAL_DSCRD_TLVS_STR, ERR_NOT_CREATED_ELEMENT_STR);
    }

    /* leaf total-unrecognized-tlvs */
    snprintf(tmp_value, MAX_STR_LEN, "%ld", lldpctl_atom_get_int(port, lldpctl_k_rx_unrecognized_cnt));
    fill_xpath(path, LLDP_PORT_RX_STAT_TOT_UN_TLVS_XPATH, port_name, dest_mac_address);
    if (NULL == lyd_new_path(*parent, NULL, path, tmp_value, 0, 0)) {
        SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                    LLDP_TOTAL_UNRCGNZ_TLVS_STR, ERR_NOT_CREATED_ELEMENT_STR);
    }

    return SR_ERR_OK;
}

/**
 * @brief Creates a new container "tx-statistics" from ieee802-dot1ab-lldp yang
 * module.
 *
 * @param[out]  parent              Pointer to an existing parent of the
 *                                  requested nodes. Is NULL for top-level
 *                                  nodes. Caller is supposed to append the
 *                                  requested nodes to this data subtree and
 *                                  return either the original parent or a
 *                                  top-level node.
 * @param[in]   port                The pointer to lldpctl_atom_t which
 *                                  represents the port for which list should be
 *                                  created.
 * @param[in]   port_name           Port name.
 * @param[in]   dest_mac_address    Port mac address.
 * @return      Error code (SR_ERR_OK on success).
 */
static int new_tx_statistics(struct lyd_node **parent, lldpctl_atom_t *port,
                             char *port_name, char *dest_mac_address)
{
    char path[MAX_STR_LEN] = "";
    char tmp_value[MAX_STR_LEN] = "";

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    /* leaf total-frames */
    snprintf(tmp_value, MAX_STR_LEN, "%ld", lldpctl_atom_get_int(port, lldpctl_k_tx_cnt));
    fill_xpath(path, LLDP_PORT_TX_STAT_TOT_FRAMES_XPATH, port_name, dest_mac_address);
    if (NULL == lyd_new_path(*parent, NULL, path, tmp_value, 0, 0)) {
        SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                    LLDP_TOTAL_FRAMES_STR, ERR_NOT_CREATED_ELEMENT_STR);
    }

    return SR_ERR_OK;
}

/**
 * @brief Creates a new entry for list 'management-address' inside list
 * 'remote-systems-data' from ieee802-dot1ab-lldp yang module.
 *
 * @param[out]  parent              Pointer to an existing parent of the
 *                                  requested nodes. Is NULL for top-level
 *                                  nodes. Caller is supposed to append the
 *                                  requested nodes to this data subtree and
 *                                  return either the original parent or a
 *                                  top-level node.
 * @param[in]   mgmt                The pointer to lldpctl_atom_t which
 *                                  represents the port for which list should
 *                                  be created.
 * @param[in]   port_name           Port name.
 * @param[in]   dest_mac_address    Port mac address.
 * @param[in]   time_mark           Port time mark.
 * @param[in]   remote_index        Port remote index.
 * @return      Error code (SR_ERR_OK on success).
  */
static int new_managament_address(struct lyd_node **parent,
                                  lldpctl_atom_t *mgmt, char *port_name,
                                  char *dest_mac_address, uint32_t time_mark,
                                  char *remote_index)
{
    char path[2*MAX_STR_LEN] = "";
    char tmp_value[MAX_STR_LEN] = "";
    char tmp_mngmt_ip[MAX_STR_LEN] = "";

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    if (EXIT_SUCCESS == check_address_type((char*)lldpctl_atom_get_str(mgmt, lldpctl_k_mgmt_ip), tmp_value)) {
        if (0 == strncmp(tmp_value, "1", MAX_STR_LEN)) {
            snprintf(tmp_value, MAX_STR_LEN, "%s", "ietf-routing:ipv4");
        } else {
            snprintf(tmp_value, MAX_STR_LEN, "%s", "ietf-routing:ipv6");
        }
        /* get leaf address value */
        snprintf(tmp_mngmt_ip, MAX_STR_LEN, "%s",
                 (unsigned char*)lldpctl_atom_get_str(mgmt, lldpctl_k_mgmt_ip));

        snprintf(path, 2*MAX_STR_LEN, LLDP_MANAG_ADDRESS_ENTRY_XPATH,
                 port_name, dest_mac_address, time_mark, remote_index, tmp_value, tmp_mngmt_ip);
        if (NULL == lyd_new_path(*parent, NULL, path, NULL, 0, 0)) {
            SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                        LLDP_SYS_MAN_ADDR_STR, ERR_NOT_CREATED_ELEMENT_STR);
        }
    }

    return SR_ERR_OK;
}

/**
 * @brief Creates a new entry for list 'remote-systems-data' inside list 'port'
 * from ieee802-dot1ab-lldp yang module.
 *
 * @param[out]  parent              Pointer to an existing parent of the
 *                                  requested nodes. Is NULL for top-level
 *                                  nodes. Caller is supposed to append the
 *                                  requested nodes to this data subtree and
 *                                  return either the original parent or a
 *                                  top-level node.
 * @param[in]   neighbor            The pointer to lldpctl_atom_t which
 *                                  represents the port for which list should
 *                                  be created.
 * @param[in]   port_name           Port name.
 * @param[in]   dest_mac_address    Port mac address.
 * @return      Error code (SR_ERR_OK on success).
  */
static int new_remote_systems_data(struct lyd_node **parent,
                                   lldpctl_atom_t *neighbor, char *port_name,
                                   char *dest_mac_address)
{
    char path[MAX_STR_LEN] = "";
    char tmp_value[MAX_STR_LEN] = "";
    uint32_t time_mark = 0;
    char remote_index[MAX_STR_LEN] = "";
    char caps_supported[MAX_STR_LEN] = "";
    char caps_enabled[MAX_STR_LEN] = "";
    const char *tmp_str = NULL;
    lldpctl_atom_t *mgmts = NULL;
    lldpctl_atom_t *mgmt = NULL;

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    /* get time-mark */
    time_mark = (uint32_t)(time(NULL)-lldpctl_atom_get_int(neighbor, lldpctl_k_port_age));
    /* get remote-index */
    snprintf(remote_index, MAX_STR_LEN, "%s",
             (char*)lldpctl_atom_get_str(neighbor, lldpctl_k_chassis_index));

    /* leaf chassis-id-subtype */
    tmp_str = lldpctl_atom_get_str(neighbor, lldpctl_k_chassis_id_subtype);
    if (tmp_str) {
        snprintf(tmp_value, MAX_STR_LEN, "%s", tmp_str);
        if (EXIT_SUCCESS == translate_single_string_using_map(tmp_value, chassis_id_subtype_map)) {
            fill_xpath(path, LLDP_REMOTE_SYS_DATA_CHASSIS_ID_SUBTYPE_XPATH, port_name,
                       dest_mac_address, time_mark, remote_index);
            if (NULL == lyd_new_path(*parent, NULL, path, tmp_value, 0, 0)) {
                SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                            LLDP_CHASSIS_ID_SYBTYPE_STR, ERR_NOT_CREATED_ELEMENT_STR);
            }
        }
    }

    /* leaf chassis-id */
    tmp_str = lldpctl_atom_get_str(neighbor, lldpctl_k_chassis_id);
    if (tmp_str) {
        fill_xpath(path, LLDP_REMOTE_SYS_DATA_CHASSIS_ID_XPATH, port_name,
                   dest_mac_address, time_mark, remote_index);
        if (NULL == lyd_new_path(*parent, NULL, path, (char*)tmp_str, 0, 0)) {
            SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                        LLDP_CHASSIS_ID_STR, ERR_NOT_CREATED_ELEMENT_STR);
        }
    }

    /* leaf port-id-subtype */
    tmp_str = lldpctl_atom_get_str(neighbor, lldpctl_k_port_id_subtype);
    if (tmp_str) {
        snprintf(tmp_value, MAX_STR_LEN, "%s", tmp_str);
        if (EXIT_SUCCESS == translate_single_string_using_map(tmp_value, port_id_subtype_map)) {
            fill_xpath(path, LLDP_REMOTE_SYS_DATA_PORT_ID_SUBTYPE_XPATH, port_name,
                       dest_mac_address, time_mark, remote_index);
            if (NULL == lyd_new_path(*parent, NULL, path, tmp_value, 0, 0)) {
                SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                            LLDP_PORT_ID_SUBTYPE_STR, ERR_NOT_CREATED_ELEMENT_STR);
            }
        }
    }

    /* leaf port-id */
    tmp_str = lldpctl_atom_get_str(neighbor, lldpctl_k_port_id);
    if (tmp_str) {
        fill_xpath(path, LLDP_REMOTE_SYS_DATA_PORT_ID_XPATH, port_name,
                   dest_mac_address, time_mark, remote_index);
        if (NULL == lyd_new_path(*parent, NULL, path, (char*)tmp_str, 0, 0)) {
            SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                        LLDP_PORT_ID_STR, ERR_NOT_CREATED_ELEMENT_STR);
        }
    }

    /* leaf port-desc */
    tmp_str = lldpctl_atom_get_str(neighbor, lldpctl_k_port_descr);
    if (tmp_str) {
        fill_xpath(path, LLDP_REMOTE_SYS_DATA_PORT_DESC_XPATH, port_name,
                   dest_mac_address, time_mark, remote_index);
        if (NULL == lyd_new_path(*parent, NULL, path, (char*)tmp_str, 0, 0)) {
            SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                        LLDP_PORT_DESCR_STR, ERR_NOT_CREATED_ELEMENT_STR);
        }
    }

    /* leaf system-name */
    tmp_str = lldpctl_atom_get_str(neighbor, lldpctl_k_chassis_name);
    if (tmp_str) {
        fill_xpath(path, LLDP_REMOTE_SYS_DATA_SYS_NAME_XPATH, port_name,
                   dest_mac_address, time_mark, remote_index);
        if (NULL == lyd_new_path(*parent, NULL, path, (char*)tmp_str, 0, 0)) {
            SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                        LLDP_SYS_NAME_STR, ERR_NOT_CREATED_ELEMENT_STR);
        }
    }

    /* leaf system-description */
    tmp_str = lldpctl_atom_get_str(neighbor, lldpctl_k_chassis_descr);
    if (tmp_str) {
        fill_xpath(path, LLDP_REMOTE_SYS_DATA_SYS_DESC_XPATH, port_name,
                   dest_mac_address, time_mark, remote_index);
        if (NULL == lyd_new_path(*parent, NULL, path, (char*)tmp_str, 0, 0)) {
            SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                        LLDP_SYS_DESCR_STR, ERR_NOT_CREATED_ELEMENT_STR);
        }
    }

    /* leafs  system-capabilities-supported and system-capabilities-enabled */
    if (EXIT_SUCCESS == fill_sys_caps_data(caps_supported, caps_enabled, neighbor)) {
        if (0 != strncmp(caps_supported, "", MAX_STR_LEN)) {
            fill_xpath(path, LLDP_REMOTE_SYS_DATA_SYS_CAP_XPATH, port_name,
                       dest_mac_address, time_mark, remote_index);
            if (NULL == lyd_new_path(*parent, NULL, path, caps_supported, 0, 0)) {
                SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                            LLDP_SYS_CAP_SUPPPORTED_STR, ERR_NOT_CREATED_ELEMENT_STR);
            }
        }

        if (0 != strncmp(caps_enabled, "", MAX_STR_LEN)) {
            fill_xpath(path, LLDP_REMOTE_SYS_DATA_SYS_CAP_EN_XPATH, port_name,
                       dest_mac_address, time_mark, remote_index);
            if (NULL == lyd_new_path(*parent, NULL, path, caps_enabled, 0, 0)) {
                SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                            LLDP_SYS_CAP_ENABLED_STR, ERR_NOT_CREATED_ELEMENT_STR);
            }
        }
    }

    /* list management-address */
    mgmts = lldpctl_atom_get(neighbor, lldpctl_k_chassis_mgmt);
    lldpctl_atom_foreach(mgmts, mgmt) {
        if (SR_ERR_OK != new_managament_address(parent, mgmt, port_name,
                                                dest_mac_address, time_mark, remote_index)) {
            SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                        LLDP_SYS_MAN_ADDR_STR, ERR_NOT_CREATED_ELEMENT_STR);
        }
    }
    lldpctl_atom_dec_ref(mgmts);

    return SR_ERR_OK;
}

/**
 * @brief Creates a new list entry for list 'port' from ieee802-dot1ab-lldp
 * yang module.
 *
 * This function is for state data from list 'port'.
 *
 * @param[in]   parent          Pointer to an existing parent of the requested
 *                              nodes. Is NULL for top-level nodes. Caller is
 *                              supposed to append the requested nodes to this
 *                              data subtree and return either the original
 *                              parent or a top-level node.
 * @param[in]   port            Pointer to lldpctl_atom_t which represents the
 *                              port for which list should be created.
 * @return      Error code (SR_ERR_OK on success).
 */
static int new_port_list(struct lyd_node **parent, lldpctl_atom_t *port)
{
    char tmp_value[MAX_STR_LEN] = "";
    char port_name[MAX_STR_LEN] = "";
    char dest_mac_address[MAX_STR_LEN] = "";
    char path[MAX_STR_LEN] = "";
    const char *tmp_str = NULL;
    lldpctl_atom_t *neighbors = NULL;
    lldpctl_atom_t *neighbor = NULL;

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    /* get port-name */
    tmp_str = (char*)lldpctl_atom_get_str(port, lldpctl_k_port_name);
    if (tmp_str)
        snprintf(port_name, MAX_STR_LEN, "%s", tmp_str);

    /* get dest-mac-address */
    tmp_str = (char*)lldpctl_atom_get_str(port, lldpctl_k_port_id);
    if (tmp_str) {
        snprintf(dest_mac_address, MAX_STR_LEN, "%s", tmp_str);
        change_addr_colons_to_dashes(dest_mac_address);
    }

    /* leaf port-id */
    tmp_str = (char*)lldpctl_atom_get_str(port, lldpctl_k_port_id);
    if (tmp_str) {
        fill_xpath(path, LLDP_PORT_LIST_PORT_ID_XPATH, port_name, dest_mac_address);
        if (NULL == lyd_new_path(*parent, NULL, path, (char*)tmp_str, 0, 0)) {
            SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                        LLDP_PORT_ID_STR, ERR_NOT_CREATED_ELEMENT_STR);
        }
    }

    /* leaf port-id-subtype */
    tmp_str = (char*)lldpctl_atom_get_str(port, lldpctl_k_port_id_subtype);
    if (tmp_str) {
        snprintf(tmp_value, MAX_STR_LEN, "%s", tmp_str);
        if (EXIT_SUCCESS == translate_single_string_using_map(tmp_value, port_id_subtype_map)) {
            fill_xpath(path, LLDP_PORT_LIST_PORT_ID_SUBTYPE_XPATH, port_name, dest_mac_address);
            if (NULL == lyd_new_path(*parent, NULL, path, tmp_value, 0, 0)) {
                SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                            LLDP_PORT_ID_SUBTYPE_STR, ERR_NOT_CREATED_ELEMENT_STR);
            }
        }
    }

    /* leaf port-desc */
    tmp_str = (char*)lldpctl_atom_get_str(port, lldpctl_k_port_descr);
    if (tmp_str) {
        fill_xpath(path, LLDP_PORT_LIST_PORT_DESC, port_name, dest_mac_address);
        if (NULL == lyd_new_path(*parent, NULL, path, (char*)tmp_str, 0, 0)) {
            SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                        LLDP_PORT_DESCR_STR, ERR_NOT_CREATED_ELEMENT_STR);
        }
    }

    tmp_str = NULL;

    /* tx-statistics container */
    if (SR_ERR_OK != new_tx_statistics(parent, port, port_name, dest_mac_address)) {
        SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                    LLDP_TX_STATISTICS_STR, ERR_NOT_CREATED_ELEMENT_STR);
    }

    /* rx-statistics container */
    if (SR_ERR_OK != new_rx_statistics(parent, port, port_name, dest_mac_address)) {
        SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                    LLDP_RX_STATISTICS_STR, ERR_NOT_CREATED_ELEMENT_STR);
    }

    /* list remote-systems-data */
    neighbors = lldpctl_atom_get(port, lldpctl_k_port_neighbors);
    if (NULL != neighbors) {
        lldpctl_atom_foreach(neighbors, neighbor) {
            if (SR_ERR_OK != new_remote_systems_data(parent, neighbor, port_name, dest_mac_address)) {
                SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                            LLDP_REMOTE_SYS_DATA_STR, ERR_NOT_CREATED_ELEMENT_STR);
            }
        }
        lldpctl_atom_dec_ref(neighbors);
    }

    return SR_ERR_OK;
}

/**
 * @brief Creates a group of the leafs according to container
 * "remote-statistics" from ieee802-dot1ab-lldp yang module.
 *
 * @param[out]  parent          Pointer to an existing parent of the requested
 *                              nodes. Is NULL for top-level nodes. Caller is
 *                              supposed to append the requested nodes to this
 *                              data subtree and return either the original
 *                              parent or a top-level node.
 * @param[in]   interfaces      Contains the list of available interfaces.
 * @return      Error code (SR_ERR_OK on success).
 */
static int create_remote_statistics_leafs(struct lyd_node **parent,
                                          lldpctl_atom_t *interfaces)
{
    lldpctl_atom_t *interface;
    lldpctl_atom_t *port;
    long unsigned int rx_discarded_cnt = 0;
    long unsigned int ageout_cnt = 0;
    long unsigned int insert_cnt = 0;
    long unsigned int delete_cnt = 0;
    char tmp_value[MAX_STR_LEN] = "";

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    /* iterate over interfaces */
    lldpctl_atom_foreach(interfaces, interface) {
        port = lldpctl_get_port(interface);
        rx_discarded_cnt += lldpctl_atom_get_int(port, lldpctl_k_rx_discarded_cnt);
        ageout_cnt += lldpctl_atom_get_int(port, lldpctl_k_ageout_cnt);
        insert_cnt += lldpctl_atom_get_int(port, lldpctl_k_insert_cnt);
        delete_cnt += lldpctl_atom_get_int(port, lldpctl_k_delete_cnt);
        lldpctl_atom_dec_ref(port);
    }

    /* leaf remote-inserts */
    snprintf(tmp_value, MAX_STR_LEN, "%lu", insert_cnt);
    if (!lyd_new_path(*parent, NULL, LLDP_REMOTE_STAT_RM_INSTERTS_XPATH, tmp_value, 0, 0)) {
        SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                    LLDP_REMOTE_INSTERTS_STR, ERR_NOT_CREATED_ELEMENT_STR);
    }

    /* leaf remote-deletes */
    snprintf(tmp_value, MAX_STR_LEN, "%lu", delete_cnt);
    if (!lyd_new_path(*parent, NULL, LLDP_REMOTE_STAT_RM_DELETES_XPATH, tmp_value, 0, 0)) {
        SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                    LLDP_REMOTE_DELETES_STR, ERR_NOT_CREATED_ELEMENT_STR);
    }

    /* leaf remote-drops */
    snprintf(tmp_value, MAX_STR_LEN, "%lu", rx_discarded_cnt);
    if (!lyd_new_path(*parent, NULL, LLDP_REMOTE_STAT_RM_DROPS_XPATH, tmp_value, 0, 0)) {
        SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                    LLDP_REMOTE_DROPS_STR, ERR_NOT_CREATED_ELEMENT_STR);
    }

    /* leaf remote-ageouts */
    snprintf(tmp_value, MAX_STR_LEN, "%lu", ageout_cnt);
    if (!lyd_new_path(*parent, NULL, LLDP_REMOTE_STAT_RM_AGEOUTS_XPATH, tmp_value, 0, 0)) {
        SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                    LLDP_REMOTE_AGEOUTS_STR, ERR_NOT_CREATED_ELEMENT_STR);
    }

    return SR_ERR_OK;
}

/**
 * @brief This function does the lldp management including lldp get transport
 * and name, receive chassis/neighbors/config data, and creates containers or
 * set data.
 *
 * @param[in]   type            Contains the value used in switch loops for
 *                              detect lldp container or object.
 * @param[in]   node            The pointer to XML element which will be used
 *                              as parent or current node in this function.
 * @param[out]  parent          Pointer to an existing parent of the requested
 *                              nodes. Is NULL for top-level nodes. Caller is
 *                              supposed to append the requested nodes to this
 *                              data subtree and return either the original
 *                              parent or a top-level node.
 * @return      Pointer to struct sr_value_t.
 */
static int do_lldp_management(int type, sr_val_t *node,
                              struct lyd_node **parent)
{
    lldpctl_conn_t *conn = NULL;
    lldpctl_atom_t *tmp_atom = NULL;
    lldpctl_atom_t *interface = NULL;
    lldpctl_atom_t *port = NULL;
    long int tmp_value = 0;

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    /* Make a connection */
    conn = lldpctl_new_name(lldpctl_get_default_transport(), NULL, NULL, NULL);
    if (NULL == conn) {
        return SR_ERR_OPERATION_FAILED;
    }

    switch (type) {
    case LLDP_LOCAL_SYS_DATA:
        /* get local chassis data */
        tmp_atom = lldpctl_get_local_chassis(conn);
        /* container local-system-data */
        if (tmp_atom) {
            if (SR_ERR_OK != create_local_system_data_leafs(parent, tmp_atom)) {
                SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                            LLDP_LOCAL_SYS_DATA, ERR_NOT_CREATED_ELEMENT_STR);
            }
        }
        break;
    case LLDP_PORT:
        /* get interfaces data */
        tmp_atom = lldpctl_get_interfaces(conn);
        /* iterate over interfaces */
        lldpctl_atom_foreach(tmp_atom, interface) {
            port = lldpctl_get_port(interface);
            if (port) {
                if (SR_ERR_OK != new_port_list(parent, port)) {
                    SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                                LLDP_PORT_STR, ERR_NOT_CREATED_ELEMENT_STR);
                }
            }
        }
        break;
    case LLDP_REMOTE_STATS:
        /* get interfaces data */
        tmp_atom = lldpctl_get_interfaces(conn);
        if (tmp_atom) {
            if (SR_ERR_OK != create_remote_statistics_leafs(parent, tmp_atom)) {
                SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                            LLDP_REMOTE_STATS_STR, ERR_NOT_CREATED_ELEMENT_STR);
            }
        }
        break;
    case LLDP_MSG_TX_INTERVAL:
        /* get configuration data */
        tmp_atom = lldpctl_get_configuration(conn);
        /* set message-tx-interval */
        if (tmp_atom) {
            tmp_value = node->data.uint32_val;
            tmp_atom = lldpctl_atom_set_int(tmp_atom, lldpctl_k_config_tx_interval, tmp_value);
        }
        break;
    case LLDP_TX_HOLD_MULTIPLIER:
        /* get configuration data */
        tmp_atom = lldpctl_get_configuration(conn);
        /* set message-tx-interval */
        if (tmp_atom) {
            tmp_value = node->data.uint32_val;
            tmp_atom = lldpctl_atom_set_int(tmp_atom, lldpctl_k_config_tx_hold, tmp_value);
        }
        break;
    default:
        SRP_LOG_ERR(ERROR_MSG_MOD_FUNC_MSG, LLDP_MODULE_NAME_STR,
                    LLDP_MODULE_NAME_STR, LLDP_WRONG_DATA_TYPE_STR);
        return SR_ERR_OPERATION_FAILED;
    }

    /* release atoms and connection */
    if (tmp_atom) {
        lldpctl_atom_dec_ref(tmp_atom);
    }
    if (port) {
        lldpctl_atom_dec_ref(port);
    }
    if (conn) {
        lldpctl_release(conn);
    }

    return SR_ERR_OK;
}

/**
 * @brief Callback to be called by the event of changing content of node with
 * "/ieee802-dot1ab-lldp:lldp/message-tx-hold-multiplier" or
 * "/ieee802-dot1ab-lldp:lldp/message-tx-interval" xpath.
 *
 *  Subscribe to it by sr_module_change_subscribe call.
 *
 * @param[in]   session         Implicit session (do not stop) with information
 *                              about the changed data (retrieved by
 *                              sr_get_changes_iter) the event originator
 *                              session IDs.
 * @param[in]   module_name     Name of the module where the change has
 *                              occurred.
 * @param[in]   xpath           XPath used when subscribing, NULL if the whole
 *                              module was subscribed to.
 * @param[in]   event           Type of the notification event that has
 *                              occurred.
 * @param[in]   request_id      Request ID unique for the specific module_name.
 *                              Connected events for one request (SR_EV_CHANGE
 *                              and SR_EV_DONE, for example) have the same
 *                              request ID.
 * @param[in]   private_data    Private context opaque to sysrepo, as passed to
 *                              sr_module_change_subscribe call.
 * @return      Error code (SR_ERR_OK on success).
 */
static int lldpd_element_cb(sr_session_ctx_t *session, const char *module_name,
                            const char *xpath, sr_event_t event,
                            uint32_t request_id, void *private_data)
{
    (void)event;
    (void)module_name;
    (void)request_id;
    (void)private_data;

    int rc = SR_ERR_OK;
    sr_change_oper_t op = {0};
    sr_change_iter_t *iter = NULL;
    sr_val_t *old_value = NULL;
    sr_val_t *new_value = NULL;
    sr_val_t *node = NULL;
    int obj_index = 0;

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    if ((SR_EV_DONE == event) || (0 == plugin_init)) {
        SRP_LOG_DBG(DEBUG_MSG_WITH_TWO_PARAM, DBG_APPLYING_CHANGES_MSG, __func__);
        return SR_ERR_OK;
    }

    rc = sr_get_changes_iter(session, xpath, &iter);
    if (SR_ERR_OK != rc) {
        return rc;
    }

    rc = sr_get_change_next(session, iter, &op, &old_value, &new_value);
    node = (op == SR_OP_DELETED) ? old_value : new_value;
    if (SR_ERR_OK != rc) {
        return rc;
    }

    /* If user changed values of leaf then node->dflt is "0". If sysrepo
     * changes value of leaf, because it has default value, that node->dflt
     * will be "1". This condition is added because of booting, i.e. applying
     * startup configuration. Every leaf that has default value will be
     * triggered with ADD operation if he does not exist inside startup
     * configuration. Its node->dflt flag will be "1" then.
     */
    if (((op == SR_OP_MODIFIED) || (op == SR_OP_CREATED)) &&
        (0 == node->dflt)) {
        obj_index = find_obj_index_by_name(sr_xpath_node_name(xpath), lldp_map, LLDP_MAX_OBJ_NUMBER);
        if (SR_ERR_OPERATION_FAILED == do_lldp_management(obj_index, node, NULL)) {
            SRP_LOG_ERR(ERROR_MSG_FUN_SETTER_FAILED, __func__);
            sr_set_error(session, xpath, ERR_SET_FUNC_FAILED_XPATH_STR, xpath);
            rc = SR_ERR_OPERATION_FAILED;
        }
    }

    sr_free_val(old_value);
    sr_free_val(new_value);
    node = NULL;
    sr_free_change_iter(iter);

    return rc;
}

/**
 * @brief Callback to be called when operational data of list port is
 * requested.
 *
 * Subscribe to it by sr_oper_get_items_subscribe call.
 *
 * @param[in]   session         Implicit session (do not stop) with information
 *                              about the event originator session IDs.
 * @param[out]  module_name     Name of the affected module.
 * @param[out]  path            Path identifying the subtree that is supposed
 *                              to be provided, same as the one used for the
 *                              subscription.
 * @param[in]   request_xpat    XPath as requested by a client. Can be NULL.
 * @param[in]   request_id      Request ID unique for the specific module_name.
 * @param[in]   parent          Pointer to an existing parent of the requested
 *                              nodes. Is NULL for top-level nodes. Caller is
 *                              supposed to append the requested nodes to this
 *                              data subtree and return either the original
 *                              parent or a top-level node.
 * @param[in]   private_data    Private context opaque to sysrepo, as passed to
 *                              sr_oper_get_items_subscribe call.
 * @return      Error code (SR_ERR_OK on success).
 */
static int lldp_list_port_state_cb(sr_session_ctx_t *session,
                                   const char *module_name, const char *path,
                                   const char *request_xpath,
                                   uint32_t request_id,
                                   struct lyd_node **parent, void *private_data)
{
    (void)session;
    (void)request_id;
    (void)module_name;
    (void)request_xpath;
    (void)request_id;
    (void)private_data;
    (void)path;

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    /* list port */
    if (SR_ERR_OK != do_lldp_management(LLDP_PORT, NULL, parent)) {
        return SR_ERR_OPERATION_FAILED;
    }

    return SR_ERR_OK;
}

/**
 * @brief Callback to be called when operational data of container
 * remote-statistics is requested. Subscribe to it by
 * sr_oper_get_items_subscribe call.
 *
 * @param[in]   session         Implicit session (do not stop) with information
 *                              about the event originator session IDs.
 * @param[out]  module_name     Name of the affected module.
 * @param[out]  path            Path identifying the subtree that is supposed
 *                              to be provided, same as the one used for the
 *                              subscription.
 * @param[in]   request_xpath   XPath as requested by a client. Can be NULL.
 * @param[in]   request_id      Request ID unique for the specific module_name.
 * @param[in]   parent          Pointer to an existing parent of the requested
 *                              nodes. Is NULL for top-level nodes. Caller is
 *                              supposed to append the requested nodes to this
 *                              data subtree and return either the original
 *                              parent or a top-level node.
 * @param[in]   private_data    Private context opaque to sysrepo, as passed to
 *                              sr_oper_get_items_subscribe call.
 * @return      Error code (SR_ERR_OK on success).
 */
static int lldp_remote_statistics_cb(sr_session_ctx_t *session,
                                     const char *module_name, const char *path,
                                     const char *request_xpath,
                                     uint32_t request_id,
                                     struct lyd_node **parent,
                                     void *private_data)
{
    (void)session;
    (void)request_id;
    (void)module_name;
    (void)request_xpath;
    (void)request_id;
    (void)private_data;
    (void)path;

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    /* container remote-statistics */
    if (SR_ERR_OK != do_lldp_management(LLDP_REMOTE_STATS, NULL, parent)) {
        return SR_ERR_OPERATION_FAILED;
    }

    return SR_ERR_OK;
}

/**
 * @brief Callback to be called when operational data of container
 * local-system-data is requested. Subscribe to it by
 * sr_oper_get_items_subscribe call.
 *
 * @param[in]   session         Implicit session (do not stop) with information
 *                              about the event originator session IDs.
 * @param[out]  module_name     Name of the affected module.
 * @param[out]  path            Path identifying the subtree that is supposed
 *                              to be provided, same as the one used for the
 *                              subscription.
 * @param[in]   request_xpath   XPath as requested by a client. Can be NULL.
 * @param[in]   request_id      Request ID unique for the specific module_name.
 * @param[in]   parent          Pointer to an existing parent of the requested
 *                              nodes. Is NULL for top-level nodes. Caller is
 *                              supposed to append the requested nodes to this
 *                              data subtree and return either the original
 *                              parent or a top-level node.
 * @param[in]   private_data    Private context opaque to sysrepo, as passed to
 *                              sr_oper_get_items_subscribe call.
 * @return      Error code (SR_ERR_OK on success).
 */
static int lldp_local_system_data_cb(sr_session_ctx_t *session,
                                     const char *module_name, const char *path,
                                     const char *request_xpath,
                                     uint32_t request_id,
                                     struct lyd_node **parent,
                                     void *private_data)
{
    (void)session;
    (void)request_id;
    (void)module_name;
    (void)request_xpath;
    (void)request_id;
    (void)private_data;
    (void)path;

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    /* container local-system-data */
    if (SR_ERR_OK != do_lldp_management(LLDP_LOCAL_SYS_DATA, NULL, parent)) {
        return SR_ERR_OPERATION_FAILED;
    }

    return SR_ERR_OK;
}

/**
 * @brief Creates a group of the leafs according to grouping lldp-cfg from
 * ieee802-dot1ab-lldp yang module.
 *
 * @param[in]   lldp_cfg        Pointer to struct lyd_node which is going to be
 *                              parent to created leafs.
 * @param[in]   module          Main schema node structure representing YANG
 *                              module.
 * @param[in]   conn            The structure needed to connect with lldpd.
 * @return      Pointer to struct lyd_node.
 */
struct lyd_node *create_lldp_grouping(struct lyd_node *lldp_cfg,
                                      const struct lys_module *module,
                                      lldpctl_conn_t *conn)
{
    lldpctl_atom_t *config = NULL;
    char tmp_value[MAX_STR_LEN] = "";

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    config = lldpctl_get_configuration(conn);

    /* get_message_fast_tx, container lldp, leaf message-fast-tx, hardcoded to default value "1" */
    if (EXIT_FAILURE == new_node(lldp_cfg, module, LLDP_MSG_FAST_TX_STR, BASE_ONE_STR)) {
        return NULL;
    }

    /* get_message_tx_hold_multiplier, container lldp, leaf message-tx-hold-multiplier */
    snprintf(tmp_value, MAX_STR_LEN, "%ld", lldpctl_atom_get_int(config, lldpctl_k_config_tx_hold));
    if (EXIT_FAILURE == new_node(lldp_cfg, module, LLDP_TX_HOLD_MULTIPLIER_STR, tmp_value)) {
        return NULL;
    }

    /* get_message_tx_interval, container lldp, leaf message-tx-interval */
    snprintf(tmp_value, MAX_STR_LEN, "%ld", lldpctl_atom_get_int(config, lldpctl_k_config_tx_interval));
    if (EXIT_FAILURE == new_node(lldp_cfg, module, LLDP_MSG_TX_INTERVAL_STR, tmp_value)) {
        return NULL;
    }

    /* get_reinit_delay, container lldp, leaf reinit-delay, hardcoded to default value "2" */
    if (EXIT_FAILURE == new_node(lldp_cfg, module, LLDP_REINIT_DELAY_STR, BASE_TWO_STR)) {
        return NULL;
    }

    /* get_tx_credit_max, container lldp, leaf tx-credit-max, hardcoded to default value "5" */
    if (EXIT_FAILURE == new_node(lldp_cfg, module, LLDP_TX_CREDIT_MAX_STR, BASE_FIVE_STR)) {
        return NULL;
    }

    /* get_tx_fast_init, container lldp, leaf tx-fast-init, hardcoded to default value "4" */
    if (EXIT_FAILURE == new_node(lldp_cfg, module, LLDP_TX_FAST_INIT_STR, BASE_FOUR_STR)) {
        return NULL;
    }

    /* get_notification_interval, container lldp, leaf notification-interval, hardcoded to default value "30" */
    if (EXIT_FAILURE == new_node(lldp_cfg, module, LLDP_NOTIF_INTERVAL_STR, BASE_THIRTY_STR)) {
        return NULL;
    }

    return lldp_cfg;
}

/**
 * @brief Creates a new entry for list management-address-tx-port (which is
 * config true).
 *
 * @param[out]  parent          Pointer to struct lyd_node which is going to be
 *                              parent to created entry.
 * @param[in]   mgmt            Pointer to lldpctl_atom_t which represents the
 *                              port for which list should be created.
 * @param[in]   module          Main schema node structure representing YANG
 *                              module.
 * @return      xmlNodePtr or NULL if element not created.
 */
struct lyd_node *new_managament_address_tx_port(struct lyd_node *parent,
                                                lldpctl_atom_t *mgmt,
                                                const struct lys_module *module)
{
    struct lyd_node *management_address = NULL;
    char tmp_value[MAX_STR_LEN] = "";
    char address_type[MAX_STR_LEN] = "1";
    // default value '1' for ipv4 address (look at libbase check_address_type)

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    if (EXIT_FAILURE == new_container(parent, &management_address, module,
                                      LLDP_MAN_ADDR_TX_PORT_STR)) {
        return NULL;
    }
    /* set man_address->id */
    /* here we check is address ipv4 or ipv6 */
    check_address_type((char*)lldpctl_atom_get_str(mgmt, lldpctl_k_mgmt_ip), tmp_value);
    if (0 == strncmp(tmp_value, address_type, MAX_STR_LEN)) {
        snprintf(tmp_value, MAX_STR_LEN, "%s", "ietf-routing:ipv4");
    } else {
        snprintf(tmp_value, MAX_STR_LEN, "%s", "ietf-routing:ipv6");
    }
    /* leaf address-subtype */
    if (EXIT_FAILURE == new_node(management_address, module,
                                 LLDP_ADDR_SUBTYPE_STR, tmp_value)) {
        return NULL;
    }

    /* leaf man-address */
    snprintf(tmp_value, MAX_STR_LEN, "%s",
             (unsigned char*)lldpctl_atom_get_str(mgmt, lldpctl_k_mgmt_ip));
    if (EXIT_FAILURE == new_node(management_address, module,
                                 LLDP_MAN_ADDRESS_STR, tmp_value)) {
        return NULL;
    }

    return management_address;
}

/**
 * @brief This function creates a new entry for list port (which is config
 * true).
 *
 * @param[out]  root            Pointer to struct lyd_node which is going to be
 *                              parent to created entry.
 * @param[in]   module          Main schema node structure representing YANG
 *                              module.
 * @param[in]   port            Pointer to lldpctl_atom_t which represents the
 *                              port for which list should be created.
 * @return      Pointer to struct lyd_node or NULL if element not created.
 */
struct lyd_node *create_list_port_entry(struct lyd_node *root,
                                        const struct lys_module *module,
                                        lldpctl_atom_t *port)
{
    char tmp_value[MAX_STR_LEN] = "";
    struct lyd_node *entry;
    lldpctl_atom_t *mgmts;
    lldpctl_atom_t *mgmt;

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    /* list port entry */
    if (EXIT_FAILURE == new_container(root, &entry, module, LLDP_PORT_STR)) {
        return NULL;
    }

    /* leaf name */
    if (EXIT_FAILURE == new_node(entry, module, LLDP_PORT_NAME_STR,
                                 (char*)lldpctl_atom_get_str(port, lldpctl_k_port_name))) {
        return NULL;
    }

    /* leaf dest-mac-address */
    snprintf(tmp_value, MAX_STR_LEN, "%s", lldpctl_atom_get_str(port, lldpctl_k_port_id));
    change_addr_colons_to_dashes(tmp_value);
    if (EXIT_FAILURE == new_node(entry, module, LLDP_DEST_MAC_ADDRESS_STR, tmp_value)) {
        return NULL;
    }

    mgmts = lldpctl_atom_get(port, lldpctl_k_chassis_mgmt);
    lldpctl_atom_foreach(mgmts, mgmt) {
        /* list management-address-tx-port */
        if (NULL == new_managament_address_tx_port(entry, mgmt, module)) {
            SRP_LOG_INF(ERROR_MSG_MOD_FUNC_EL_MSG, LLDP_MODULE_NAME_STR, __func__,
                        LLDP_MAN_ADDR_TX_PORT_STR, ERR_NOT_CREATED_ELEMENT_STR);
        }
    }
    lldpctl_atom_dec_ref(mgmts);

    return root;
}

/**
 * @brief Creates a list port (which is config true).
 *
 * @param[out]  root            Pointer to struct lyd_node which is going to be
 *                              parent to created leafs.
 * @param[in]   module          Main schema node structure representing YANG
 *                              module.
 * @param[in]   conn            The structure needed to connect with lldpd.
 * @return      Pointer to struct lyd_node.
 */
struct lyd_node *create_list_port(struct lyd_node *root,
                                  const struct lys_module *module,
                                  lldpctl_conn_t *conn)
{
    lldpctl_atom_t *tmp_atom = NULL;
    lldpctl_atom_t *interface = NULL;
    lldpctl_atom_t *port = NULL;

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    tmp_atom = lldpctl_get_interfaces(conn);

    lldpctl_atom_foreach(tmp_atom, interface) {
        port = lldpctl_get_port(interface);
        if (NULL == create_list_port_entry(root, module, port)) {
            return NULL;
        }
    }
    lldpctl_atom_dec_ref(port);

    return root;
}

/**
 * @brief Creates a startup datastore for ieee802-dot1ab-lldp module. Function
 * is going to be called once, due to sysrepo-plugind startup.
 *
 * @param[in]   session         Sysrepo session that can be used for any API
 *                              calls needed for plugin cleanup (mainly for
 *                              unsubscribing of subscriptions initialized in
 *                              sr_plugin_init_cb).
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
static int lldp_fill_datastore(sr_session_ctx_t *session)
{
    const struct ly_ctx *ctx = NULL;
    lldpctl_conn_t *conn = NULL;
    struct lyd_node *root = NULL;
    const struct lys_module *module = NULL;
    int fd = -1;
    char *path = NULL;

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    if (0 == sr_path_check_startup_done_file(LLDP_MODULE_NAME, &path)) {
        ctx = sr_get_context(sr_session_get_connection(session));
        if (NULL == ctx) {
            SRP_LOG_ERR(ERROR_MSG_FUN_AND_MSG, __func__,
                        ERR_SESSION_CTX_FAILED_STR);
            free(path);
            return EXIT_FAILURE;
        }

        /* create ieee802-dot1ab-lldp root element (container lldp) */
        root = lyd_new_path(NULL, ctx, "/ieee802-dot1ab-lldp:lldp", NULL, 0, 0);
        if (NULL == root) {
            SRP_LOG_ERR(ERROR_MSG_FUN_AND_MSG, __func__,
                        ERR_NOT_CREATED_ROOT_ELEMENT_STR);
            free(path);
            return EXIT_FAILURE;
        }

        /* load "iana-if-type" module to use necessary 'identity' types */
        if (NULL == ly_ctx_load_module((struct ly_ctx*)ctx, "ietf-routing", NULL)) {
            SRP_LOG_ERR(ERROR_MSG_FUN_AND_MSG, __func__,
                        ERR_MSG_LOAD_MODULE_STR);
            free(path);
            return EXIT_FAILURE;
        }

        conn = lldpctl_new_name(lldpctl_get_default_transport(), NULL, NULL, NULL);
        if (NULL == conn) {
            free(path);
            return EXIT_FAILURE;
        }
        /* create configuration lldp grouping leafs */
        if (NULL == create_lldp_grouping(root, module, conn)) {
            free(path);
            return EXIT_FAILURE;
        }
        /* create list port leafs */
        if (NULL == create_list_port(root, module, conn)) {
            free(path);
            return EXIT_FAILURE;
        }

        if (conn) {
            lldpctl_release(conn);
        }

        /******** NOTE ******/
        /* The startup to running datastore copy is done before the plugin is
         * started. So, here we will replace the current startup and running
         * datastores with the subtree called 'root' in this function.
         */
        /* Switch session to statup datastore */
        if (SR_ERR_OK != sr_session_switch_ds(session, SR_DS_STARTUP)) {
            SRP_LOG_ERR(ERROR_MSG_FUN_AND_MSG, __func__,
                        ERR_SWITCH_DATASTORE_FAILED_STR);
            free(path);
            return EXIT_FAILURE;
        }

        /* Replace current startup configuration with created 'root' subtree */
        if (SR_ERR_OK != sr_replace_config(session, LLDP_MODULE_NAME_STR,
                                           root, 0, 0)) {
            SRP_LOG_ERR(ERROR_MSG_FUN_AND_MSG, __func__,
                        ERR_REPLACE_CONFIG_FAILED_STR);
            free(path);
            return EXIT_FAILURE;
        }

        /* Switch session to running datastore */
        if (SR_ERR_OK != sr_session_switch_ds(session, SR_DS_RUNNING)) {
            SRP_LOG_ERR(ERROR_MSG_FUN_AND_MSG, __func__,
                        ERR_SWITCH_DATASTORE_FAILED_STR);
            free(path);
            return EXIT_FAILURE;
        }

        /* Copy startup to running datastore */
        if (SR_ERR_OK != sr_copy_config(session, LLDP_MODULE_NAME_STR,
                                        SR_DS_STARTUP, 0, 0)) {
            SRP_LOG_ERR(ERROR_MSG_FUN_AND_MSG, __func__,
                        ERR_COPY_DATASTORE_FAILED_STR);
            free(path);
            return EXIT_FAILURE;
        }

        fd = open(path, O_WRONLY | O_APPEND | O_CREAT, 0644);
        if (-1 == fd) {
            SRP_LOG_ERR("Unable to create file %s.", path);
            free(path);
            close(fd);
            return EXIT_FAILURE;
        }
        close(fd);
    }
    if (path) {
        free(path);
        path = NULL;
    }

    return EXIT_SUCCESS;
}

/**
 * @brief Sysrepo plugin initialization callback.
 *
 * @param[in]   session         Sysrepo session that can be used for any API
 *                              calls needed for plugin initialization (mainly
 *                              for reading of startup configuration and
 *                              subscribing for notifications).
 * @param[in]   private_data    Private context (opaque to sysrepo) that will
 *                              be passed to sr_plugin_cleanup_cb when plugin
 *                              cleanup is requested.
 * @return      Error code (SR_ERR_OK on success). If an error is returned,
 *              plugin will be considered as uninitialized.
 */
int sr_plugin_init_cb(sr_session_ctx_t *session, void **private_data)
{
    (void)private_data;
    int rc = SR_ERR_OK;
    char *path = NULL;

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    /* Set plugin_init to "1" so callbacks are triggered when startup configuration is copied to running. */
    if (0 != sr_path_check_startup_done_file(LLDP_MODULE_NAME, &path)) {
        plugin_init = 1;
    }

    do {
        /* subscribe for /ieee802-dot1ab-lldp module changes */
        rc = sr_module_change_subscribe(session, LLDP_MODULE_NAME_STR, NULL,
                                        module_change_cb, NULL, 0, SR_SUBSCR_ENABLED, &subscription);
        if (SR_ERR_OK != rc) break;

        /* subscribe for /ieee802-dot1ab-lldp:lldp/bridge module changes */
        rc = sr_module_change_subscribe(session, LLDP_MODULE_NAME_STR, "/ieee802-dot1ab-lldp:lldp/bridge",
                                        mod_not_supported_cb, NULL, 1, SR_SUBSCR_ENABLED, &subscription);
        if (rc != SR_ERR_OK) break;

        /* subscribe for /ieee802-dot1ab-lldp:lldp/message-fast-tx module changes */
        rc = sr_module_change_subscribe(session, LLDP_MODULE_NAME_STR, "/ieee802-dot1ab-lldp:lldp/message-fast-tx",
                                        mod_not_supported_cb, NULL, 1, SR_SUBSCR_ENABLED, &subscription);
        if (rc != SR_ERR_OK) break;

        /* subscribe for /ieee802-dot1ab-lldp:lldp/message-tx-hold-multiplier module changes */
        rc = sr_module_change_subscribe(session, LLDP_MODULE_NAME_STR, "/ieee802-dot1ab-lldp:lldp/message-tx-hold-multiplier",
                                        lldpd_element_cb, NULL, 2, SR_SUBSCR_ENABLED, &subscription);
        if (rc != SR_ERR_OK) break;

        /* subscribe for /ieee802-dot1ab-lldp:lldp/message-tx-interval module changes */
        rc = sr_module_change_subscribe(session, LLDP_MODULE_NAME_STR, "/ieee802-dot1ab-lldp:lldp/message-tx-interval",
                                        lldpd_element_cb, NULL, 1, SR_SUBSCR_ENABLED, &subscription);
        if (rc != SR_ERR_OK) break;

        /* subscribe for /ieee802-dot1ab-lldp:lldp/reinit-delay module changes */
        rc = sr_module_change_subscribe(session, LLDP_MODULE_NAME_STR, "/ieee802-dot1ab-lldp:lldp/reinit-delay",
                                        mod_not_supported_cb, NULL, 1, SR_SUBSCR_ENABLED, &subscription);
        if (rc != SR_ERR_OK) break;

        /* subscribe for /ieee802-dot1ab-lldp:lldp/tx-credit-max module changes */
        rc = sr_module_change_subscribe(session, LLDP_MODULE_NAME_STR, "/ieee802-dot1ab-lldp:lldp/tx-credit-max",
                                        mod_not_supported_cb, NULL, 1, SR_SUBSCR_ENABLED, &subscription);
        if (rc != SR_ERR_OK) break;

        /* subscribe for /ieee802-dot1ab-lldp:lldp/tx-fast-init module changes */
        rc = sr_module_change_subscribe(session, LLDP_MODULE_NAME_STR, "/ieee802-dot1ab-lldp:lldp/tx-fast-init",
                                        mod_not_supported_cb, NULL, 1, SR_SUBSCR_ENABLED, &subscription);
        if (rc != SR_ERR_OK) break;

        /* subscribe for /ieee802-dot1ab-lldp:lldp/notification-interval module changes */
        rc = sr_module_change_subscribe(session, LLDP_MODULE_NAME_STR, "/ieee802-dot1ab-lldp:lldp/notification-interval",
                                        mod_not_supported_cb, NULL, 1, SR_SUBSCR_ENABLED, &subscription);
        if (rc != SR_ERR_OK) break;

        /* subscribe for /ieee802-dot1ab-lldp:lldp/port module changes */
        rc = sr_module_change_subscribe(session, LLDP_MODULE_NAME_STR, "/ieee802-dot1ab-lldp:lldp/port",
                                        mod_not_supported_cb, NULL, 1, SR_SUBSCR_ENABLED, &subscription);
        if (rc != SR_ERR_OK) break;

        /* subscribe as state data provider for the ieee802-dot1ab-lldp:lldp/port data */
        rc = sr_oper_get_items_subscribe(session, LLDP_MODULE_NAME_STR, "/ieee802-dot1ab-lldp:lldp/port",
                                         lldp_list_port_state_cb, NULL, SR_SUBSCR_DEFAULT , &subscription);
        if (rc != SR_ERR_OK) break;

        /* subscribe as state data provider for the ieee802-dot1ab-lldp:lldp/local-system-data data */
        rc = sr_oper_get_items_subscribe(session, LLDP_MODULE_NAME_STR, "/ieee802-dot1ab-lldp:lldp/local-system-data",
                                         lldp_local_system_data_cb, NULL, SR_SUBSCR_DEFAULT , &subscription);
        if (rc != SR_ERR_OK) break;

        /* subscribe as state data provider for the ieee802-dot1ab-lldp:lldp/remote-statistics data */
        rc = sr_oper_get_items_subscribe(session, LLDP_MODULE_NAME_STR, "/ieee802-dot1ab-lldp:lldp/remote-statistics",
                                         lldp_remote_statistics_cb, NULL, SR_SUBSCR_DEFAULT , &subscription);
        if (rc != SR_ERR_OK) break;

    } while (0);

    if (SR_ERR_OK != rc) {
        SRP_LOG_ERR(ERR_MODULE_INIT_FAILED_STR, LLDP_MODULE_NAME_STR, sr_strerror(rc));
        sr_unsubscribe(subscription);
        return rc;
    }

    if (EXIT_FAILURE == lldp_fill_datastore(session)) {
        SRP_LOG_ERR(ERROR_MSG_MOD_FUNC_MSG, LLDP_MODULE_NAME_STR, __func__,
                    ERR_MSG_EMPTY_STARTUP_DS_STR);
        return SR_ERR_OPERATION_FAILED;
    }

    plugin_init = 1;

    SRP_LOG_INF(INF_MODULE_INIT_SUCCESS_STR, LLDP_MODULE_NAME_STR);

    return rc;
}

/**
 * @brief Sysrepo plugin cleanup callback.
 *
 * @param[in]   session         Sysrepo session that can be used for any API
 *                              calls needed for plugin cleanup (mainly for
 *                              unsubscribing of subscriptions initialized in
 *                              sr_plugin_init_cb).
 * @param[in]   private_data    Private context as passed in sr_plugin_init_cb.
 */
void sr_plugin_cleanup_cb(sr_session_ctx_t *session, void *private_data)
{
    (void)private_data;
    (void)session;

    /* nothing to cleanup except freeing the subscriptions */
    sr_unsubscribe(subscription);
    SRP_LOG_INF(INF_MODULE_CLEANUP_STR, LLDP_MODULE_NAME_STR);
}
