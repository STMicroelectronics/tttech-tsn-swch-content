/*
 * TTTech ieee802-dot1q-bridge-yang-module
 * Copyright(c) 2019 TTTech Computertechnik AG.
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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

/* common includes */
#include "common_defines.h"
#include "common.h"
#include "config_change.h"
/* sysrepo includes */
#include <sysrepo.h>
#include <sysrepo/values.h>
#include <sysrepo/xpath.h>
/* libbase includes */
#include "libbase/base_defines.h"
#include "libbase/base_prints.h"
#include "libbase/base_private_translators.h"
#include "libbase/base_linux_tool_helpers.h"
#include "libbase/base_private_syscalls.h"
#include "libbase/base_private_parsers.h"
#include "libbase/base_q_bridge_linux_tools.h"
#include "libbase/base_mstp_linux_tools.h"
/* libtsn includes */
#include "libtsn/tsn.h"
#include "libtsn/tsn_def.h"
#include "libtsn/tsn_bridge.h"
#include "libtsn/tsn_qbridge.h"
#include "libtsn/tsn_mstp.h"
#include "libtsn/tsn_error.h"
/* module specific includes */
#include "q_bridge_defines.h"
#include "source/stream_filter_gates.h"

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

typedef struct filtering_entry filtering_entry_t;

/* structure used for storing vids added inside vid--to-fid-allocation list */
struct vid_to_fid_allocation {
    char** vids_array;  /* array of vids */
    int count;          /* number of vids */
};

// global variable for storing vid-to-fid-allocation vids.
struct vid_to_fid_allocation vid_to_fid_allocation_array = {
    .vids_array = NULL,
    .count = 0
};

/* structure used for storing fids added inside fid--to-vid-allocation list */
struct fid_to_vid_allocation {
    char** fid_array;   /* array of fids */
    int count;          /* number of fids */
};

// global variable for storing fid-to-vid-allocation fids.
struct fid_to_vid_allocation fid_to_vid_allocation_array = {
    .fid_array = NULL,
    .count = 0
};

/* structure used for storing vids added inside vlan list */
struct vlan {
    char** vid_array;   /* array of vids */
    int count;          /* number of vids */
};

// global variable for storing vlan vids.
struct vlan vlan_array = { .vid_array = NULL, .count = 0 };

#define TSN_ERRNO_ERROR_CODE(return_value)      ((return_value) & 0xFF)

/* List of values */
struct filtering_entry {
    sr_val_t *filtering_entry; /* parent of port-map */
    sr_val_t *port_map;
    sr_val_t *database_id;
    sr_val_t *address;
    sr_val_t *vids;
    sr_val_t *entry_type;
    sr_val_t *port_ref;
    sr_val_t *bridge_name;
    sr_val_t *static_filt_entry;
    sr_val_t *control_el;
};

/**
 * @brief Creates a container static-vlan-registration-entries inside list
 * port-map from ieee802-dot1q-bridge yang module.
 *
 * This function is for configuration data from list component.
 *
 * @param[out]  parent          Pointer to struct lyd_node which is going to be
 *                              parent to created entry.
 * @param[in]   module          Main schema node structure representing YANG
 *                              module.
 * @param[in]   port_name       Contains the name of the port.
 * @param[in]   bridge_vlan     Contains the bridge vlan value.
 * @return      struct lyd_node *or NULL if element not created.
 */
struct lyd_node *new_static_vlan_reg_entries(struct lyd_node *parent,
                                             const struct lys_module *module,
                                             char *port_name, char *bridge_vlan)
{
    struct lyd_node *entry = NULL;
    char result_array[BASE_MAX_VID][MAX_STR_LEN] = {0};

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    /* new port-map entry */
    if (EXIT_FAILURE == new_container(parent, &entry, module,
                                      Q_BRIDGE_STATIC_VLAN_REG_ENTRY_STR)) {
        return NULL;
    }

    /* container new_static_vlan_reg_entries, leaf registrar-admin-control */
    if (EXIT_FAILURE == new_node(entry, module,
                                 Q_BRIDGE_REG_ADMIN_CONTROL, Q_BRIDGE_NORMAL)) {
        return NULL;
    }

    if (EXIT_SUCCESS == get_vlan_transmitted(result_array, BASE_MAX_VID,
                                             MAX_STR_LEN, port_name, bridge_vlan)) {
        if (EXIT_FAILURE == translate_strings_using_map(result_array, transmitted_map)) {
            return NULL;
        }
        if (EXIT_FAILURE == new_node(entry, module,
                                     Q_BRIDGE_STATIC_VLAN_TRANSMITTED_STR,
                                     (char*)result_array)) {
            return NULL;
        }
    }

    return entry;
}

/**
 * @brief Creates a new entry for list port-map inside list
 * vlan-registration-entry from ieee802-dot1q-bridge yang module.
 *
 * This function is for configuration data from list component.
 *
 * @param[out]  parent          Pointer to struct lyd_node which is going to be
 *                              parent to created entry.
 * @param[in]   module          Main schema node structure representing YANG
 *                              module.
 * @param[in]   port_name       Contains the name of the port.
 * @param[in]   bridge_vlan     Contains the bridge vlan value.
 * @return      struct lyd_node *or NULL if element not created.
 */
struct lyd_node *new_fvre_port_map_list(struct lyd_node *parent,
                                        const struct lys_module *module,
                                        char *port_name, char *bridge_vlan)
{
    struct lyd_node *port_map_entry = NULL;
    char tmp_value[MAX_STR_LEN] = "";
    struct port_hdl *port = NULL;
    uint32_t port_id = 0;

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    /* new port-map entry */
    if (EXIT_FAILURE == new_container(parent, &port_map_entry, module,
                                      Q_BRIDGE_PORT_MAP)) {
        return NULL;
    }

    /* list port-map (inside port-map-grouping), leaf port-ref, it is port id
     * given by tsn_bridge_port_id
     */
    port = tsn_port_open_by_name(port_name);
    if (NULL == port) {
        SRP_LOG_ERR(ERROR_MSG_MOD_FUNC_EL_MSG, Q_BRIDGE_MODULE_NAME, __func__,
                    port_name, ERR_PORT_NOT_OPEN_STR);
        return NULL;
    }
    if (tsn_bridge_port_id(port, &port_id)) {
        SRP_LOG_ERR(ERROR_MSG_MOD_FUNC_EL_MSG, Q_BRIDGE_MODULE_NAME, __func__,
                    port_name, "Failed to get bridge port id.");
        tsn_port_close(port);
        return NULL;
    }
    tsn_port_close(port);

    /* list port-map (inside port-map-grouping), leaf port-ref,
     * it is port index
     */
    snprintf(tmp_value, MAX_STR_LEN, "%u", port_id);
    if (EXIT_FAILURE == new_node(port_map_entry, module,
                                 Q_BRIDGE_PORT_REF_STR, tmp_value)) {
        return NULL;
    }

    /* container static-vlan-registration-entries */
    if (NULL == new_static_vlan_reg_entries(port_map_entry, module,
                                            port_name, bridge_vlan)) {
        return NULL;
    }

    return port_map_entry;
}

/**
 * @brief Creates a new entry for list vlan-registration-entry inside container
 * filtering-database from ieee802-dot1q-bridge yang module.
 *
 * This function is for configuration data from list component.
 *
 * @param[out]  parent          Pointer to struct lyd_node which is going to be
 *                              parent to created entry.
 * @param[in]   module          Main schema node structure representing YANG
 *                              module.
 * @param[in]   bridge_vlan     Contains the bridge vlan value.
 * @param[in]   bridge_name     Contains the name of the bridge.
 * @return      struct lyd_node *or NULL if element not created.
 */
struct lyd_node *new_vlan_registration_entry(struct lyd_node *parent,
                                             const struct lys_module *module,
                                             char *bridge_vlan,
                                             char *bridge_name)
{
    (void)bridge_name;
    struct lyd_node *vlan_registartion_entry = NULL;
    char port_names[MAX_NUM_OF_INTERFACES][MAX_STR_LEN] = {0};
    int i = 0;

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    /* new vlan-registration-entry in container filtering-database */
    if (EXIT_FAILURE == new_container(parent, &vlan_registartion_entry, module,
                                      Q_BRIDGE_VLAN_REG_ENTRY)) {
        return NULL;
    }
    /* leaf database-id */
    if (EXIT_FAILURE == new_node(vlan_registartion_entry, module,
                                 Q_BRIDGE_DATABASE_ID, BASE_ONE_STR)) {
        return NULL;
    }
    /* leaf vids */
    if (EXIT_FAILURE == new_node(vlan_registartion_entry, module,
                                 Q_BRIDGE_VIDS_STR, bridge_vlan)) {
        return NULL;
    }
    /* leaf entry-type */
    if (EXIT_FAILURE == new_node(vlan_registartion_entry, module,
                                 Q_BRIDGE_ENTRY_TYPE, Q_BRIDGE_STATIC)) {
        return NULL;
    }

    /* bridge ports, first we get all port names */
    /* param_1 is vlan, param_2 is port_name */
    if (EXIT_FAILURE == get_bridge_ports_name_by_vlan(port_names, MAX_NUM_OF_INTERFACES,
                                                      MAX_STR_LEN, bridge_vlan, bridge_name)) {
        /* free (and unlink) the specified data subtree (vlan_registartion_entry) */
        lyd_free_withsiblings(vlan_registartion_entry);
        vlan_registartion_entry = NULL;
        return NULL;
    }

    /* vlan must have at least one port name */
    if (0 == strncmp(port_names[0], "", MAX_STR_LEN)) {
        /* free (and unlink) the specified data subtree (vlan_registartion_entry) */
        lyd_free_withsiblings(vlan_registartion_entry);
        vlan_registartion_entry = NULL;
        return NULL;
    }

    i = 0;
    while ((i < MAX_NUM_OF_INTERFACES) &&
           (0 != strncmp(port_names[i], "", MAX_STR_LEN))) {
        /* new port-map entry in list vlan-registration-entry */
        if (NULL == new_fvre_port_map_list(vlan_registartion_entry, module,
                                           port_names[i], bridge_vlan)) {
            /* free (and unlink) the specified data subtree (vlan_registartion_entry) */
            lyd_free_withsiblings(vlan_registartion_entry);
            vlan_registartion_entry = NULL;
        }
        i++;
    }

    return vlan_registartion_entry;
}

/**
 * @brief Creates container static-filtering-entries inside list port-map from
 * ieee802-dot1q-bridge yang module.
 *
 * This function is for configuration data from list component.
 *
 * @param[out]  parent          Pointer to struct lyd_node which is going to be
 *                              parent to created entry.
 * @param[in]   module          Main schema node structure representing YANG
 *                              module.
 * @param[in]   port_name       Contains the port name.
 * @return      struct lyd_node *or NULL if element not created.
 */
struct lyd_node *new_static_filtering_entries(struct lyd_node *parent,
                                              const struct lys_module *module,
                                              char *port_name)
{
    struct lyd_node *entry = NULL;
    char result_array[MAX_NUM_OF_INTERFACES][MAX_STR_LEN] = {0};

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    /* new static-vlan-registration-entry */
    if (EXIT_FAILURE == new_container(parent, &entry, module,
                                      Q_BRIDGE_STATIC_FILT_ENTRY)) {
        return NULL;
    }
    /* container new-static-filtering-entries, leaf control-element */
    if (EXIT_SUCCESS == get_control_element(result_array, MAX_NUM_OF_INTERFACES,
                                            MAX_STR_LEN, port_name)) {
        if (EXIT_FAILURE == translate_strings_using_map(result_array, control_element_map)) {
            return NULL;
        }
        if (EXIT_FAILURE == new_node(entry, module,
                                     Q_BRIDGE_CONTROL_EL_STR, (char*)result_array)) {
            return NULL;
        }
    }
    /* container new-static-filtering-entries, leaf connection-identifier */
    if (EXIT_SUCCESS == get_connection_identifier(result_array, MAX_NUM_OF_INTERFACES,
                                                  MAX_STR_LEN, port_name)) {
        if (EXIT_FAILURE == new_node(entry, module,
                                     Q_BRIDGE_CONNECT_ID_STR, result_array[0])) {
            return NULL;
        }
    }

    return entry;
}

/**
 * @brief Creates list port-map inside list filtering-entry from
 * ieee802-dot1q-bridge yang module.
 *
 * This function is for configuration data from list component.
 *
 * @param[out]  parent          Pointer to struct lyd_node which is going to be
 *                              parent to created entry.
 * @param[in]   module          Main schema node structure representing YANG
 *                              module.
 * @param[in]   port_names      Contains the port names.
 * @return      struct lyd_node *or NULL if element not created.
 */
struct lyd_node *new_ffe_port_map_list(
        struct lyd_node *parent,
        const struct lys_module *module,
        char port_names[MAX_NUM_OF_INTERFACES + Q_BRIDGE_PORT_MAX_NUM][MAX_STR_LEN])
{
    struct lyd_node *port_map_entry = NULL;
    char tmp_value[MAX_STR_LEN] = "";
    struct port_hdl *port = NULL;
    uint32_t port_id = 0;
    int i = 0;

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    while ((i < MAX_NUM_OF_INTERFACES) &&
           (0 != strncmp(port_names[i], "", MAX_STR_LEN))) {
        /* new port-map list entry */
        if (EXIT_FAILURE == new_container(parent, &port_map_entry, module,
                                          Q_BRIDGE_PORT_MAP)) {
            return NULL;
        }
        /* list port-map (inside port-map-grouping), leaf port-ref, it is port
         * id given by tsn_bridge_port_id
         */
        port = tsn_port_open_by_name(port_names[i]);
        if (NULL == port) {
            SRP_LOG_ERR(ERROR_MSG_MOD_FUNC_EL_MSG, Q_BRIDGE_NAME_STR, __func__,
                        port_names[i], ERR_PORT_NOT_OPEN_STR);
            return NULL;
        }
        if (tsn_bridge_port_id(port, &port_id)) {
            SRP_LOG_ERR(ERROR_MSG_MOD_FUNC_EL_MSG, Q_BRIDGE_MODULE_NAME, __func__,
                        port_names[i], "Failed to get bridge port id.");
            tsn_port_close(port);
            return NULL;
        }
        tsn_port_close(port);

        snprintf(tmp_value, MAX_STR_LEN, "%u", port_id);
        if (EXIT_FAILURE == new_node(port_map_entry, module,
                                     Q_BRIDGE_PORT_REF_STR, tmp_value)) {
            return NULL;
        }

        /* container static-filtering-entries inside list port-map */
        if (NULL == new_static_filtering_entries(port_map_entry, module,
                                                 port_names[i])) {
            return NULL;
        }
        i++;
    }

    return port_map_entry;
}

/**
 * @brief Creates a new entry for list filtering-entry inside container
 * filtering-database from ieee802-dot1q-bridge yang module.
 *
 * This function is for configuration data from list component.
 *
 * @param[out]  parent          Pointer to struct lyd_node which is going to be
 *                              parent to created entry.
 * @param[in]   module          Main schema node structure representing YANG
 *                              module.
 * @param[in]   key_names       Pointer to array of strings. Represents the key
 *                              leafs names.
 * @param[in]   key_values      Pointer to array of strings. Represents the key
 *                              leafs values.
 * @param[in]   key_num         Contains the number of the key leafs.
 * @param[in]   port_names      Contains the name of the ports.
 * @return      struct lyd_node *or NULL if element not created.
 */
struct lyd_node *new_filtering_filtering_entry(
        struct lyd_node *parent,
        const struct lys_module *module,
        char (*key_names)[MAX_STR_LEN],
        char (*key_values)[MAX_STR_LEN],
        int key_num,
        char port_names[MAX_NUM_OF_INTERFACES + Q_BRIDGE_PORT_MAX_NUM][MAX_STR_LEN])
{
    struct lyd_node *filtering_entry = NULL;

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    /* new filtering-entry in container filtering-database */
    if (EXIT_FAILURE == new_container(parent, &filtering_entry, module,
                                      Q_BRIDGE_FILTERING_ENTRY)) {
        return NULL;
    }

    /* list filtering-entry, create all keys */
    if (EXIT_FAILURE == create_list_keys(filtering_entry, module,
                                         key_names, key_values, key_num)) {
        return NULL;
    }
    /* leaf entry-type, from list filtering_entry, set to 'static',
     * 'static' is default and single enum value */
    if (EXIT_FAILURE == new_node(filtering_entry, module,
                                 Q_BRIDGE_ENTRY_TYPE, Q_BRIDGE_STATIC)) {
        return NULL;
    }

    /* port-map list in filtering-entry from filetering-database */
    if (NULL == new_ffe_port_map_list(filtering_entry, module, port_names)) {
        return NULL;
    }

    return filtering_entry;
}

/**
 * @brief Creates a new container filtering-database inside list component from
 * ieee802-dot1q-bridge yang module.
 *
 * This function is for configuration data from list component.
 *
 * @param[out]  parent          Pointer to struct lyd_node which is going to be
 *                              parent to created entry.
 * @param[in]   module          Main schema node structure representing YANG
 *                              module.
 * @param[in]   bridge_name     Contains the name of the bridge.
 * @return      struct lyd_node *or NULL if element not created.
 */
struct lyd_node *new_filtering_database(struct lyd_node *parent,
                                        const struct lys_module *module,
                                        char *bridge_name)
{
    struct lyd_node *filtering_database = NULL;
    struct lyd_node *filtering_entry = NULL;
    /* BASE_MAX_NUM_OF_STATIC_FDB_ENTRIES is used because the libbase
     * tsn_bridge_attr function can not be trusted
     */
    char port_address[BASE_MAX_NUM_OF_STATIC_FDB_ENTRIES][MAX_STR_LEN] = {0};
    char port_names[BASE_MAX_NUM_OF_STATIC_FDB_ENTRIES][MAX_STR_LEN] = {0};
    // max number of vids + max number of default vids per port
    char port_vlans[BASE_MAX_VID + Q_BRIDGE_PORT_MAX_NUM][BASE_FIVE] = {0};
    char bridge_vlans[BASE_MAX_VID][BASE_TEN] = {0};
    int i = 0;
    int j = 0;

    char key_names[BASE_THREE][MAX_STR_LEN] = {
        Q_BRIDGE_DATABASE_ID,
        Q_BRIDGE_VIDS_STR,
        Q_BRIDGE_ADDRESS_STR
    };
    char key_values[BASE_THREE][MAX_STR_LEN] = {0};

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    /* new filtering_database in component list */
    if (EXIT_FAILURE == new_container(parent, &filtering_database, module,
                                      Q_BRIDGE_FILTERING_DATABASE)) {
        return NULL;
    }
    /* list filtering-entry */
    /* gather all static mac addresses from fdb table */
    if (EXIT_SUCCESS == get_static_fdb_addresses(port_address, BASE_MAX_NUM_OF_STATIC_FDB_ENTRIES,
                                                 MAX_STR_LEN)) {
        /* per address */
        while ((i < BASE_MAX_NUM_OF_STATIC_FDB_ENTRIES) &&
               (0 != strncmp(port_address[i], "", MAX_STR_LEN))) {
            /* get all unique vids per unique static fdb address */
            if (EXIT_SUCCESS == get_vids_by_static_fdb_address(port_vlans, BASE_MAX_VID + Q_BRIDGE_PORT_MAX_NUM,
                                                               BASE_FIVE, port_address[i])) {
                j = 0;
                while ((j < (BASE_MAX_VID + Q_BRIDGE_PORT_MAX_NUM)) &&
                       (0 != strncmp(port_vlans[j], "", BASE_FIVE))) {
                    /* get all ports name for this vid and address
                     * even static entries without vid
                     */
                    if (EXIT_FAILURE == get_fdb_port_names_by_addr_and_vid(port_names, BASE_MAX_NUM_OF_STATIC_FDB_ENTRIES,
                                                                           MAX_STR_LEN, port_address[i], port_vlans[j])) {
                        return filtering_database;
                    }

                    snprintf(key_values[0], MAX_STR_LEN, "%s", BASE_ONE_STR);
                    memcpy(key_values[1], port_vlans[j], strlen(port_vlans[j]));
                    change_addr_colons_to_dashes(port_address[i]);
                    memcpy(key_values[2], port_address[i], strlen(port_address[i]));

                    filtering_entry = new_filtering_filtering_entry(filtering_database, module,
                                                                    key_names, key_values, BASE_THREE, port_names);

                    if (NULL == filtering_entry) {
                        return filtering_database;
                    }
                    // erase for next iteration
                    memset(port_names, 0, MAX_NUM_OF_INTERFACES * MAX_STR_LEN);
                    j++;
                }
            }
            i++;
        }
    }

    /* list vlan-registration-entry */
    if (EXIT_SUCCESS == get_bridge_vlans(bridge_vlans, BASE_MAX_VID, BASE_TEN, 1)) {
        // 1 for compressed vlan ids
        j = 0;
        while ((j < BASE_MAX_VID) &&
               (0 != strncmp("", bridge_vlans[j], BASE_TEN))) {
            if (NULL == new_vlan_registration_entry(filtering_database, module,
                                                    bridge_vlans[j], bridge_name)) {
                return NULL;
            }
            j++;
        }
    } else {
        return NULL;
    }

    return filtering_database;
}

/**
 * @brief Creates a new entry for list vid-to-fid inside container
 * filtering-database from ieee802-dot1q-bridge yang module.
 *
 * This function is for configuration data from list component.
 *
 * @param[out]  parent          Pointer to struct lyd_node which is going to be
 *                              parent to created entry.
 * @param[in]   module          Main schema node structure representing YANG
 *                              module.
 * @param[in]   bridge_name     Contains the name of the bridge.
 * @param[in]   tmp_vid         Contains the VLAN identifier.
 * @param[in]   mstp_vid        Contains the Filtering Database used by this
 *                              VLAN.
 * @param[in]   param_4         It is not used in this function. Used only to
 *                              satisfy the function prototype.
 * @return      struct lyd_node *or NULL if element not created.
 */
struct lyd_node *new_vid2fid_entry(struct lyd_node *parent,
                                   const struct lys_module *module,
                                   char *bridge_name,
                                   char *tmp_vid, char *mstp_vid, char *param_4)
{
    (void)param_4;

    struct lyd_node *vid2fid_entry = NULL;
    char result_array[BASE_MAX_VID][BASE_TEN] = {0};

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    /* TO DO:
     * We may have to add here additional check, that exist on netopeer1 and it
     * is commented. The check is inside new_vid2fid_entry function
     */

    /* new vid-to-fid in container bridge-vlan */
    if (EXIT_FAILURE == new_container(parent, &vid2fid_entry, module,
                                      Q_BRIDGE_VID2FID_STR)) {
        return NULL;
    }

    /* leaf vid, from list vid-to-fid, mandatory */
    if (EXIT_FAILURE == new_node(vid2fid_entry, module,
                                 Q_BRIDGE_VID_STR, tmp_vid)) {
        return NULL;
    }

    /* leaf fid, from list vid-to-fid */
    if (EXIT_SUCCESS == get_mstp_fid_by_vids(result_array, BASE_MAX_VID,
                                             BASE_TEN, bridge_name, mstp_vid)) {
        if (EXIT_FAILURE == new_node(vid2fid_entry, module,
                                     Q_BRIDGE_FID_STR, result_array[0])) {
            return NULL;
        }
    }

    return vid2fid_entry;
}

/**
 * @brief Creates a new container bridge-vlan inside list component from
 * ieee802-dot1q-bridge yang module.
 *
 * This function is for configuration data from list component.
 *
 * @param[out]  parent          Pointer to struct lyd_node which is going to be
 *                              parent to created entry.
 * @param[in]   module          Main schema node structure representing YANG
 *                              module.
 * @param[in]   bridge_name     Contains the name of the bridge.
 * @return      struct lyd_node *or NULL if element not created.
 */
struct lyd_node *new_bridge_vlan(struct lyd_node *parent,
                                 const struct lys_module *module,
                                 char *bridge_name)
{
    struct lyd_node *bridge_vlan = NULL;
    // array of individual vids or vid's intervals
    char mstp_vids_array[BASE_MAX_VID][MAX_STR_LEN] = {0};
    // array of individual vids
    char tmp_vids_array[BASE_MAX_VID][BASE_FIVE] = {0};
    char tmp_fid[BASE_MAX_VID][BASE_FIVE] = {0};
    int i = 0;
    int j = 0;

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    /* new bridge-vlan in component list */
    if (EXIT_FAILURE == new_container(parent, &bridge_vlan, module,
                                      Q_BRIDGE_VLAN_STR)) {
        return NULL;
    }

    /* list vid-to-fid */
    i = 0;
    if (EXIT_FAILURE == get_bridge_mstp_vids(mstp_vids_array, BASE_MAX_VID,
                                             MAX_STR_LEN, bridge_name)) {
        return NULL;
    }

    while ((i < BASE_MAX_VID) &&
           (0 != strncmp(mstp_vids_array[i], "", MAX_STR_LEN))) {
        // FID 0 is default, it is not useful
        if (EXIT_SUCCESS == get_mstp_fid_by_vids(tmp_fid, BASE_MAX_VID, BASE_FIVE,
                                                 bridge_name, mstp_vids_array[i]) &&
            (0 != strncmp(tmp_fid[0], BASE_ZERO_STR, BASE_FIVE)) &&
            (0 != strncmp(tmp_fid[0], "", BASE_FIVE))) {
            assemble_vids_as_array(tmp_vids_array, BASE_MAX_VID,
                                   BASE_FIVE, mstp_vids_array[i]);
            while ((j < BASE_MAX_VID) &&
                   (0 != strncmp(tmp_vids_array[j], "", BASE_FIVE))) {
                if (NULL == new_vid2fid_entry(bridge_vlan, module,
                                              bridge_name, tmp_vids_array[j], mstp_vids_array[i], NULL)) {
                    return NULL;
                }
                j++;
            }
            j = 0; // reset vid counter
        }
        i++; // increase fid counter
    }

    return bridge_vlan;
}

/**
 * @brief Creates a new entry for list fid-to-mstid-allocation inside container
 * bridge-mst from ieee802-dot1q-bridge yang module.
 *
 * This function is for configuration data from list component.
 *
 * @param[out]  parent          Pointer to struct lyd_node which is going to be
 *                              parent to created entry.
 * @param[in]   module          Main schema node structure representing YANG
 *                              module.
 * @param[in]   bridge_msti     Contains the name of the bridge mstid.
 * @param[in]   bridge_fids     Contains the name of the bridge fids.
 * @param[in]   param_3         It is not used in this function. Used only to
 *                              satisfy the function prototype.
 * @param[in]   param_4         It is not used in this function. Used only to
 *                              satisfy the function prototype.
 * @return      struct lyd_node *or NULL if element not created.
 */
struct lyd_node *new_fid2mstid_alloc_entry(struct lyd_node *parent,
                                           const struct lys_module *module,
                                           char *bridge_msti,
                                           char *bridge_fids,
                                           char *param_3, char *param_4)
{
    (void)param_3;
    (void)param_4;

    struct lyd_node *entry = NULL;

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    /* new fid-to-mstid-allocation in bridge-mst container */
    if (EXIT_FAILURE == new_container(parent, &entry, module,
                                      Q_BRIDGE_FID2MSTID_ALLOC_STR)) {
        return NULL;
    }

    /* leaf fids, from list fid-to-mstid-allocation, mandatory */
    if (EXIT_FAILURE == new_node(entry, module,
                                 Q_BRIDGE_FIDS_STR, bridge_fids)) {
        return NULL;
    }

    /* leaf mstid, from list fid-to-mstid-allocation */
    if (EXIT_FAILURE == new_node(entry, module,
                                 Q_BRIDGE_MSTID_STR, bridge_msti)) {
        return NULL;
    }

    return entry;
}

/**
 * @brief Creates a new container bridge-mst inside list component from
 * ieee802-dot1q-bridge yang module.
 *
 * This function is for configuration data from list component.
 *
 * @param[out]  parent          Pointer to struct lyd_node which is going to be
 *                              parent to created entry.
 * @param[in]   module          Main schema node structure representing YANG
 *                              module.
 * @param[in]   bridge_name     Contains the name of the bridge.
 * @return      struct lyd_node *or NULL if element not created.
 */
struct lyd_node *new_bridge_mst(struct lyd_node *parent,
                                const struct lys_module *module,
                                char *bridge_name)
{
    struct lyd_node *bridge_mst = NULL;
    char bridge_msti[BASE_MAX_VID][BASE_TEN] = {0};
    char bridge_fids[BASE_MAX_VID][MAX_STR_LEN] = {0};
    int i = 0;
    int j = 0;

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    /* new bridge-mst in component list */
    if (EXIT_FAILURE == new_container(parent, &bridge_mst, module,
                                      Q_BRIDGE_MST_STR)) {
        return NULL;
    }

    /* leaf-list mstid */
    if (EXIT_FAILURE == get_bridge_mstpi(bridge_msti, BASE_MAX_VID,
                                         BASE_TEN, bridge_name)) {
        return NULL;
    }

    while ((i < BASE_MAX_VID) &&
           (0 != strncmp(bridge_msti[i], "", BASE_TEN))) {
        if (EXIT_FAILURE == new_node(bridge_mst, module,
                                     Q_BRIDGE_MSTID_STR, bridge_msti[i])) {
            return NULL;
        }
        i++;
    }

    /* list fid-to-mstid-allocation in bridge-mst container */
    i = 0;
    while ((i < BASE_MAX_VID) &&
           (0 != strncmp(bridge_msti[i], "", BASE_TEN))) {
        if (EXIT_FAILURE != get_bridge_fids(bridge_fids, BASE_MAX_VID, MAX_STR_LEN,
                                            bridge_name, bridge_msti[i])) {
            while ((j< BASE_MAX_VID) &&
                   (0 != strncmp(bridge_fids[j], "", MAX_STR_LEN))) {
                if (NULL == new_fid2mstid_alloc_entry(bridge_mst, module,
                                                      bridge_msti[i], bridge_fids[j],
                                                      NULL, NULL)) {
                    return NULL;
                }
                j++;
            }
            j = 0;
        }
        i++;
    }

    return bridge_mst;
}

/**
 * @brief Creates a new list entry for list component from ieee802-dot1q-bridge
 * yang module.
 *
 * This function is for configuration data from list component.
 *
 * @param[out]  parent          Pointer to struct lyd_node which is going to be
 *                              parent to created entry.
 * @param[in]   module          Main schema node structure representing YANG
 *                              module.
 * @param[in]   ctx             libyang context handler.
 * @param[in]   bridge_name     Contains the name of the bridge.
 * @param[in]   caps            Bridge capabilities and attributes struct.
 * @return      struct lyd_node *or NULL if element not created.
 */
struct lyd_node *new_bridge_component(struct lyd_node *parent,
                                      const struct lys_module *module,
                                      struct ly_ctx *ctx,
                                      char *bridge_name,
                                      struct bridge_attr_and_caps caps)
{
    (void)ctx;

    struct lyd_node *component = NULL;
    char tmp_value[MAX_STR_LEN] = "";
    char result_array[MAX_NUM_OF_INTERFACES][MAX_STR_LEN] = {0};
    char bridge_msti[BASE_MAX_VID][BASE_TEN] = {0};

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    /* new component in bridge list */
    if (EXIT_FAILURE == new_container(parent, &component, module,
                                      Q_BRIDGE_COMPONENT)) {
        return NULL;
    }
    /* list bridge, list component, leaf name, key */
    if (EXIT_FAILURE == new_node(component, module,
                                 Q_BRIDGE_NAME_STR, bridge_name)) {
        return NULL;
    }
    /* list bridge, list component, leaf id */
    snprintf(tmp_value, MAX_STR_LEN, "%d", caps.ComponentId);
    if (EXIT_FAILURE == new_node(component, module,
                                 Q_BRIDGE_COMP_ID, tmp_value)) {
        return NULL;
    }
    /* list bridge, list component, leaf type, mandatory */
    snprintf(tmp_value, MAX_STR_LEN, "%d", caps.ComponentType);
    if (EXIT_FAILURE == translate_single_string_using_map(tmp_value, component_type_map)) {
        return NULL;
    }
    if (EXIT_FAILURE == new_node(component, module,
                                 Q_BRIDGE_COMP_TYPE, tmp_value)) {
        return NULL;
    }
    /* list bridge, list component, leaf address */
    if (EXIT_SUCCESS == get_interface_address(result_array, MAX_NUM_OF_INTERFACES,
                                              MAX_STR_LEN, bridge_name)) {
        change_addr_colons_to_dashes(result_array[0]);
        if (EXIT_FAILURE == new_node(component, module,
                                     Q_BRIDGE_ADDRESS_STR, result_array[0])) {
            return NULL;
        }
    }
    /* list bridge, list component, leaf traffic-class-enabled */
    snprintf(tmp_value, MAX_STR_LEN, "%s",
             (caps.BridgeTrafficClassesEnabled == true) ? BASE_TRUE : BASE_FALSE);
    if (EXIT_FAILURE == new_node(component, module,
                                 Q_BRIDGE_TC_ENABLED, tmp_value)) {
        return NULL;
    }
    /* container filtering-database,
     * exist only if when "bridge-type != 'two-port-mac-relay-bridge'"
     */
    /* check if bridge-type is not 'two-port-mac-relay-bridge' */
    if (EXIT_SUCCESS == get_bridge_type(result_array, MAX_NUM_OF_INTERFACES,
                                        MAX_STR_LEN, bridge_name)) {
        if ((0 != strncmp(Q_BRIDGE_TWO_PORT_MAC_RELAY, result_array[0], MAX_STR_LEN))) {
            /* container filtering-database */
            if (NULL == new_filtering_database(component, module, bridge_name)) {
                return NULL;
            }
            /* container bridge-vlan */
            if (NULL == new_bridge_vlan(component, module, bridge_name)) {
                return NULL;
            }
            /* check is mstp supported and is there any MSTI created and then
             * create the bridge-mst leafs
             */
            if (EXIT_SUCCESS == mstp_daemon_check_status(result_array, MAX_NUM_OF_INTERFACES, MAX_STR_LEN) &&
                EXIT_SUCCESS == get_bridge_mstpi(bridge_msti, BASE_MAX_VID, BASE_TEN, bridge_name)) {
                if (0 != strncmp(bridge_msti[0], "", BASE_TEN)) {
                    /* container bridge-mst */
                    if (NULL == new_bridge_mst(component, module, bridge_name)) {
                        return NULL;
                    }
                }
            }
        }
    }

    /* augment container stream-gates from ieee802-dot1q-stream-filters-gates yang module */
    if (NULL == new_stream_gates_container(ctx, component, bridge_name)) {
        return component;
    }

    /* augment container stream-filters from ieee802-dot1q-stream-filters-gates yang module */
    if (NULL == new_stream_filters_container(ctx, component, bridge_name)) {
        return component;
    }

    /* augment container flow-meters from ieee802-dot1q-psfp yang module */
    if (NULL == new_flow_meters_container(ctx, component, bridge_name)) {
        return component;
    }

    return component;
}

/**
 * @brief Creates a new list entry for list bridge from ieee802-dot1q-bridge
 * yang module.
 *
 * This function is for configuration data from list bridge.
 *
 * @param[out]  root            Pointer to struct lyd_node which is going to be
 *                              parent to created entry.
 * @param[in]   module          Main schema node structure representing YANG
 *                              module.
 * @param[in]   bridge_name     Contains the name of the bridge.
 * @param[in]   ctx             libyang context handler.
 * @return      struct lyd_node *or NULL if element not created.
 */
struct lyd_node *new_bridge_node(struct lyd_node *root,
                                 const struct lys_module *module,
                                 char *bridge_name, struct ly_ctx *ctx)
{
    struct lyd_node *bridge = NULL;
    struct port_hdl *port = NULL;
    struct bridge_attr_and_caps caps = {0};
    char tmp_value[MAX_STR_LEN] = "";
    char objects[MAX_NUM_OF_INTERFACES][MAX_STR_LEN] = {0};

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    /* new bridge in bridges container */
    if (EXIT_FAILURE == new_container(root, &bridge, module,
                                      Q_BRIDGE_BRIDGE_ENTRY)) {
        return NULL;
    }

    /* list bridge, leaf name, key */
    if (EXIT_FAILURE == new_node(bridge, module,
                                 Q_BRIDGE_NAME_STR, bridge_name)) {
        return NULL;
    }
    /* list bridge, leaf address, unique, mandatory */
    if (EXIT_SUCCESS == get_interface_address(objects, MAX_NUM_OF_INTERFACES,
                                              MAX_STR_LEN, bridge_name)) {
        change_addr_colons_to_dashes(objects[0]);
        if (EXIT_FAILURE == new_node(bridge, module,
                                     Q_BRIDGE_ADDRESS_STR, objects[0])) {
            return NULL;
        }
    }
    /* open bridge */
    port = tsn_port_open_by_name(bridge_name);
    if (NULL == port) {
        SRP_LOG_ERR(ERROR_MSG_MOD_FUNC_EL_MSG, Q_BRIDGE_NAME_STR, __func__,
                    bridge_name, ERR_PORT_NOT_OPEN_STR);
        return NULL;
    }
    if (0 != tsn_bridge_attr(port, &caps)) {
        SRP_LOG_ERR(ERROR_MSG_MOD_FUNC_EL_MSG, Q_BRIDGE_MODULE_NAME, __func__,
                    bridge_name, "Failed to get bridge capabilities and attributes.");
        tsn_port_close(port);
        return NULL;
    }
    tsn_port_close(port);

    /* list bridge, leaf bridge-type, mandatory */
    snprintf(tmp_value, MAX_STR_LEN, "%d", caps.BridgeType);
    if (EXIT_FAILURE == translate_single_string_using_map(tmp_value, bridge_type_map)) {
        return NULL;
    }
    if (EXIT_FAILURE == new_node(bridge, module, Q_BRIDGE_TYPE_STR, tmp_value)) {
        tsn_port_close(port);
        return NULL;
    }
    /* go through the list and process all components of this bridge */
    if (NULL == new_bridge_component(bridge, module, ctx, bridge_name, caps)) {
        return NULL;
    }

    return bridge;
}

/**
 * @brief Creates a startup datastore for ieee802-dot1q-bridge module. Function
 * is going to be called once, due to sysrepo-plugind startup.
 *
 * @param[in]   session     Sysrepo session that can be used for any API calls
 *                              needed for plugin cleanup (mainly for
 *                              unsubscribing of subscriptions initialized in
 *                              sr_plugin_init_cb).
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
static int bridge_fill_datastore(sr_session_ctx_t *session)
{
    const struct ly_ctx *ctx = NULL;
    const struct lys_module *module = NULL;
    struct lyd_node *root = NULL;
    struct lyd_node *entry = NULL;
    char bridge_names[MAX_NUM_OF_INTERFACES][MAX_STR_LEN]= {0};
    int i = 0;
    int fd = -1;
    char *path = NULL;
    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    /* this means that startup is empty */
    if (0 == sr_path_check_startup_done_file(BRIDGE_MODULE_NAME, &path)) {
        ctx = sr_get_context(sr_session_get_connection(session));
        if (NULL == ctx) {
            SRP_LOG_ERR(ERROR_MSG_FUN_AND_MSG, __func__,
                        ERR_SESSION_CTX_FAILED_STR);
            free(path);
            return EXIT_FAILURE;
        }

        /* create ieee802-dot1q-bridge. root element (container bridges) */
        root = lyd_new_path(NULL, ctx, "/ieee802-dot1q-bridge:bridges", NULL, 0, 0);
        if (NULL == root) {
            SRP_LOG_ERR(ERROR_MSG_FUN_AND_MSG, __func__,
                        ERR_NOT_CREATED_ROOT_ELEMENT_STR);
            free(path);
            return EXIT_FAILURE;
        }

        if (EXIT_FAILURE == get_bridge_names(bridge_names, MAX_NUM_OF_INTERFACES, MAX_STR_LEN)) {
            SRP_LOG_ERR(ERR_BRIDE_NAMES_FAILED_STR, __func__);
            free(path);
            return EXIT_SUCCESS;
        }

        /* go through the list and process all bridges */
        while ((i < MAX_NUM_OF_INTERFACES) &&
               (0 != strncmp(bridge_names[i], "", MAX_STR_LEN))) {
            entry = new_bridge_node(root, module, bridge_names[i], (struct ly_ctx*)ctx);
            if (NULL == entry) {
                SRP_LOG_WRN(ERROR_MSG_MOD_FUNC_MSG, Q_BRIDGE_MODULE_NAME, __func__,
                            ERR_MSG_EMPTY_STARTUP_DS_STR);
            }
            i++;
        }
        /******** NOTE *******/
        /* The startup to running datastore copy is done before the plugin is
         * started. So, here we will replace the current startup and running
         * datastores with the subtree called 'root' in this function.
         */

        /* switch to running datastore */
        if (SR_ERR_OK != sr_session_switch_ds(session, SR_DS_RUNNING)) {
            SRP_LOG_ERR(ERROR_MSG_FUN_AND_MSG, __func__,
                        ERR_SWITCH_DATASTORE_FAILED_STR);
            free(path);
            return EXIT_FAILURE;
        }

        /* Replace current running configuration with created 'root' subtree */
        if (SR_ERR_OK != sr_replace_config(session, Q_BRIDGE_MODULE_NAME,
                                           root, 0, 0)) {
            SRP_LOG_ERR(ERROR_MSG_FUN_AND_MSG, __func__,
                        ERR_REPLACE_CONFIG_FAILED_STR);
            free(path);
            return EXIT_FAILURE;
        }

        /* switch back to startup datastore */
        if (SR_ERR_OK != sr_session_switch_ds(session, SR_DS_STARTUP)) {
            SRP_LOG_ERR(ERROR_MSG_FUN_AND_MSG, __func__,
                        ERR_SWITCH_DATASTORE_FAILED_STR);
            free(path);
            return EXIT_FAILURE;
        }

        /* copy config from running to startup datastore */
        if (SR_ERR_OK != sr_copy_config(session, Q_BRIDGE_MODULE_NAME,
                                        SR_DS_RUNNING, 0, 0)) {
            SRP_LOG_ERR(ERROR_MSG_FUN_AND_MSG, __func__,
                        ERR_COPY_DATASTORE_FAILED_STR);
            free(path);
            return EXIT_FAILURE;
        }

        /* switch back to running datastore */
        if (SR_ERR_OK != sr_session_switch_ds(session, SR_DS_RUNNING)) {
            SRP_LOG_ERR(ERROR_MSG_FUN_AND_MSG, __func__,
                        ERR_SWITCH_DATASTORE_FAILED_STR);
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

    rc = sr_get_items(session, "/ieee802-dot1q-bridge:ieee802-dot1q-bridge//*",
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
 * @brief Callback to be called by the event of editing list bridge and list
 * component from ieee802-dot1q-bridge module.
 *
 * Subscribe to it by sr_module_change_subscribe call.
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
static int bridge_list_mod_not_supported_cb(sr_session_ctx_t *session,
                                            const char *module_name,
                                            const char *xpath,
                                            sr_event_t event,
                                            uint32_t request_id,
                                            void *private_data)
{
    (void)module_name;
    (void)request_id;
    (void)private_data;

    sr_change_oper_t op = {0};
    sr_change_iter_t *iter = NULL;
    sr_xpath_ctx_t st = {0};
    sr_val_t *old_value = NULL;
    sr_val_t *new_value = NULL;
    sr_val_t *node = NULL;
    char bridge_name[MAX_STR_LEN] = "";
    char bridge_path[MAX_STR_LEN] = "";
    char component_path[MAX_STR_LEN] = "";
    int rc = SR_ERR_OK;

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    if ((SR_EV_DONE == event) || (0 == plugin_init)) {
        SRP_LOG_DBG(DEBUG_MSG_WITH_TWO_PARAM, DBG_APPLYING_CHANGES_MSG, __func__);
        return SR_ERR_OK;
    }

    rc = sr_get_changes_iter(session, xpath, &iter);
    if (SR_ERR_OK != rc) {
        return rc;
    }

    while (SR_ERR_OK == sr_get_change_next(session, iter, &op,
                                           &old_value, &new_value)) {
        node = (op == SR_OP_DELETED) ? old_value : new_value;

        /* Get bridge name value. That is a key inside bridge and component list */
        if (EXIT_SUCCESS != get_key_value(session, node->xpath,
                                          Q_BRIDGE_BRIDGE_ENTRY,
                                          Q_BRIDGE_NAME_STR,
                                          &st, bridge_name)) {
            return SR_ERR_OPERATION_FAILED;
        }

        fill_xpath(bridge_path, BRIDGE_OPERATION, bridge_name);
        fill_xpath(component_path, BRIDGE_COMPONENT_OPERATION, bridge_name, bridge_name);

        /* if component entry was deleted and if callback was called for list component */
        if ((0 == strncmp(node->xpath, component_path, MAX_STR_LEN) &&
             (0 == strcmp(xpath, "/ieee802-dot1q-bridge:bridges/bridge/component"))) &&
            (op == SR_OP_DELETED)) {
            /* It is not possible to remove component entry */
            sr_set_error(session, node->xpath, ERR_OP_NOT_SUPPORTED_XPATH_STR, xpath);
            return SR_ERR_OPERATION_FAILED;
        }
        /* if bridge entry was deleted and if callback was called for list bridge */
        else if ((0 == strncmp(node->xpath, bridge_path, MAX_STR_LEN) &&
                  (0 == strcmp(xpath, "/ieee802-dot1q-bridge:bridges/bridge"))) &&
                 (op == SR_OP_DELETED)) {
            /* It is not possible to remove bridge entry */
            sr_set_error(session, node->xpath, ERR_OP_NOT_SUPPORTED_XPATH_STR, xpath);
            return SR_ERR_OPERATION_FAILED;
        }
        /* if component entry was added and if callback was called for list component */
        else if ((0 != strncmp(node->xpath, component_path, MAX_STR_LEN) &&
                  (0 == strcmp(xpath, "/ieee802-dot1q-bridge:bridges/bridge/component"))) &&
                 (op == SR_OP_CREATED)) {
            /* It is not possible to add component entry */
            sr_set_error(session, node->xpath, ERR_OP_NOT_SUPPORTED_XPATH_STR, xpath);
            return SR_ERR_OPERATION_FAILED;
        }
        /* if bridge entry was added and if callback was called for list bridge */
        else if ((0 != strncmp(node->xpath, bridge_path, MAX_STR_LEN) &&
                  (0 == strcmp(xpath, "/ieee802-dot1q-bridge:bridges/bridge"))) &&
                 (op == SR_OP_CREATED)) {
            /* It is not possible to add bridge entry */
            sr_set_error(session, node->xpath, ERR_OP_NOT_SUPPORTED_XPATH_STR, xpath);
            return SR_ERR_OPERATION_FAILED;
        }
    }

    sr_free_val(old_value);
    sr_free_val(new_value);
    sr_free_change_iter(iter);
    node = NULL;
    iter = NULL;

    return SR_ERR_OK;
}

/**
 * @brief Callback to be called by the event of changing content of node with
 * next xpath:
 * "/ieee802-dot1q-bridge:bridges/bridge/component/filtering-database/vlan-registration-entry/entry-type"
 *
 * Subscribe to it by sr_module_change_subscribe call.
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
static int bridge_vlan_entry_type_cb(sr_session_ctx_t *session,
                                     const char *module_name,
                                     const char *xpath,
                                     sr_event_t event,
                                     uint32_t request_id,
                                     void *private_data)
{
    (void)module_name;
    (void)request_id;
    (void)private_data;

    int rc = SR_ERR_OK;
    sr_change_oper_t op = {0};
    sr_change_iter_t *iter = NULL;
    sr_val_t *old_value = NULL;
    sr_val_t *new_value = NULL;
    sr_val_t *node = NULL;

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    if ((SR_EV_DONE == event) || (0 == plugin_init)) {
        SRP_LOG_DBG(DEBUG_MSG_WITH_TWO_PARAM, DBG_APPLYING_CHANGES_MSG, __func__);
        return SR_ERR_OK;
    }

    rc = sr_get_changes_iter(session, xpath, &iter);
    if (SR_ERR_OK != rc) {
        return rc;
    }

    while (SR_ERR_OK == sr_get_change_next(session, iter, &op,
                                           &old_value, &new_value)) {
        node = (op == SR_OP_DELETED) ? old_value : new_value;

        if ((op == SR_OP_CREATED)) {
            if (0 != strncmp(node->data.enum_val, Q_BRIDGE_STATIC, MAX_STR_LEN)) {
                sr_set_error(session, node->xpath,
                             "only \"static\" is currently supported for '%s'.",
                             node->xpath);
                return SR_ERR_OPERATION_FAILED;
            }
        }

        if ((op == SR_OP_MODIFIED)) {
            sr_set_error(session, node->xpath,
                         ERR_MOD_OP_NOT_SUPPORTED_XPATH_STR, node->xpath);
            return SR_ERR_OPERATION_FAILED;
        }
    }

    sr_free_val(old_value);
    sr_free_val(new_value);
    sr_free_change_iter(iter);
    node = NULL;
    iter = NULL;

    return SR_ERR_OK;
}

/**
 * @brief Callback to be called by the event of changing content of list
 * vlan-registration-entry inside container filtering-database.
 *
 * Subscribe to it by sr_module_change_subscribe call.
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
static int bridge_vlan_reg_entry_cb(sr_session_ctx_t *session,
                                    const char *module_name,
                                    const char *xpath,
                                    sr_event_t event,
                                    uint32_t request_id,
                                    void *private_data)
{
    (void)module_name;
    (void)request_id;
    (void)private_data;

    int rc = SR_ERR_OK;
    tsn_return_value tsn_ret_val = E_TSN_LIB_NO_ERROR;
    sr_change_oper_t op = {0};
    sr_xpath_ctx_t st = {0};
    sr_change_iter_t *iter = NULL;
    sr_val_t *old_value = NULL;
    sr_val_t *new_value = NULL;
    sr_val_t *node = NULL;
    char bridge_name[MAX_STR_LEN] = "";
    char vids[MAX_STR_LEN] = "";
    char database_id[MAX_STR_LEN] = "";
    char path[MAX_STR_LEN] = "";
    char tmp_vids_array[BASE_MAX_VID][BASE_TEN] = {0};
    char vids_array[BASE_MAX_VID][BASE_TEN] = {0};
    char bridge_vlans[BASE_MAX_VID][BASE_TEN] = {0};
    char dynamic_entries[MAX_NUM_OF_INTERFACES][MAX_STR_LEN] = {0};
    int j = 0;
    int i = 0;
    uint32_t num_vlans = 0;
    uint32_t max_supported_vlans = 0;
    uint32_t num_dynamic_vlans = 0;
    uint32_t new_vids_cnt = 0;
    int vid_exist = 0;
    struct port_hdl *bridge_hdl = NULL;
    /* structures used to get port-map entries */
    sr_val_t *items = NULL;
    size_t counter = 0;

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    if ((SR_EV_DONE == event) || (0 == plugin_init)) {
        SRP_LOG_DBG(DEBUG_MSG_WITH_TWO_PARAM, DBG_APPLYING_CHANGES_MSG, __func__);
        return SR_ERR_OK;
    }

    rc = sr_get_changes_iter(session, xpath, &iter);
    if (SR_ERR_OK != rc) {
        return rc;
    }

    while (SR_ERR_OK == sr_get_change_next(session, iter, &op,
                                           &old_value, &new_value)) {
        node = (op == SR_OP_DELETED) ? old_value : new_value;

        /* if vlan-registration-entry is deleted or added */
        if ((true == sr_xpath_node_name_eq(node->xpath, Q_BRIDGE_VIDS_STR)) &&
            ((op == SR_OP_CREATED) ||
             (op == SR_OP_DELETED))) {
            /* Get bridge name value. That is a key inside bridge and component list */
            if (EXIT_SUCCESS != get_key_value(session, node->xpath,
                                              Q_BRIDGE_BRIDGE_ENTRY,
                                              Q_BRIDGE_NAME_STR,
                                              &st, bridge_name)) {
                return SR_ERR_OPERATION_FAILED;
            }

            /* Get vids value. That is a key inside vlan-registration-entry list */
            if (EXIT_SUCCESS != get_key_value(session, node->xpath,
                                              Q_BRIDGE_VLAN_REG_ENTRY,
                                              Q_BRIDGE_VIDS_STR, &st, vids)) {
                return SR_ERR_OPERATION_FAILED;
            }

            /* Get database-id value. That is a key inside vlan-registration-entry list */
            if (EXIT_SUCCESS != get_key_value(session, node->xpath,
                                              Q_BRIDGE_VLAN_REG_ENTRY,
                                              Q_BRIDGE_DATABASE_ID,
                                              &st, database_id)) {
                return SR_ERR_OPERATION_FAILED;
            }

            /* Part that checks if vids contains descending interval is not
             * covered by sysrepo
             */
            if (does_vids_string_contains_descending_interval(vids)) {
                sr_set_error(session, node->xpath,
                             ERR_BRIDGE_DESCENDING_VIDS_XPATH_STR,
                             vids, node->xpath);
                return SR_ERR_OPERATION_FAILED;
            }
            /* Checking valid vid range is not covered by sysrepo, so we need
             * to do it
             */
            if (does_vids_string_contains_invalid_vid(vids)) {
                sr_set_error(session, node->xpath,
                             ERR_BRIDGE_INVALID_RANGE_VIDS_XPATH_STR,
                             vids, node->xpath);
                return SR_ERR_OPERATION_FAILED;
            }

            fill_xpath(path, BRIDGE_PORT_MAP_XPATH,
                       bridge_name, bridge_name, database_id, vids);

            rc = sr_get_items(session, path, 0, 0, &items, &counter);

            if ((SR_ERR_OK != rc) && (SR_ERR_NOT_FOUND != rc)) // if item is not found the count is 0
            {
                sr_set_error(session, path, ERR_FAILED_GET_OBJ_STR, path);
                return SR_ERR_OPERATION_FAILED;
            }

            if ((op == SR_OP_CREATED)) {
                /* if operations are SR_OP_CREATED then check if new
                 * vlan-registration-entry contains list port map
                 *
                 * if operations are SR_OP_DELETED there is no need to check if
                 * port-maps exist
                 */

                /* It is not possible to mod/add/rem vlan-registration-entry if
                 * there is no port-map defined. If there is port-map defined,
                 * the manipulation of port-map will be covered by port-map
                 * callback function.
                 */
                if ((items == NULL) && (counter == 0)) {
                    sr_set_error(session, path,
                                 ERR_BRIDGE_PORT_MAP_MISSING_XPATH_STR,
                                 vids, path);
                    return SR_ERR_OPERATION_FAILED;
                }
            }

            /* Check vlan-registration-entries */
            /* Assemble vids array from vids that we got in edit-config
             * request, and put it to the new vids_array which will contain all
             * single vids from all vlan-registration-entries
             */
            assemble_vids_as_array(tmp_vids_array, BASE_MAX_VID, BASE_TEN, vids);
            while ((i < BASE_MAX_VID) &&
                   (0 != strncmp(tmp_vids_array[i], "", BASE_TEN))) {
                memcpy(vids_array[j], tmp_vids_array[i], strlen(tmp_vids_array[i]));
                i++;
                j++;
            }
            i = 0;
        }
    }

    /* After bridge name is found, open port to get current number of used VLANs */
    bridge_hdl = tsn_port_open_by_name(bridge_name);
    if (NULL == bridge_hdl) {
        sr_set_error(session, node->xpath, ERR_PORT_NOT_OPEN_XPATH_STR, bridge_name);
        return SR_ERR_OPERATION_FAILED;
    }

    if (E_TSN_LIB_NO_ERROR != (tsn_ret_val = tsn_qbridge_get_num_vlans(bridge_hdl, &num_vlans))) {
        sr_set_error(session, node->xpath, ERR_GETTER_EXT_FMT_STR,
                     Q_BRIDGE_VLAN_STR, LIT_VAL_STR, BASE_PORT_STR, bridge_name,
                     tsn_lib_get_known_ernno_msg(tsn_ret_val));
        tsn_port_close(bridge_hdl);
        return SR_ERR_OPERATION_FAILED;
    }

    if (E_TSN_LIB_NO_ERROR != (tsn_ret_val = tsn_qbridge_get_max_supported_vlans(bridge_hdl, &max_supported_vlans))) {
        sr_set_error(session, node->xpath, ERR_GETTER_EXT_FMT_STR,
                     Q_BRIDGE_MAX_VIDS_STR, LIT_VAL_STR, BASE_PORT_STR,
                     bridge_name, tsn_lib_get_known_ernno_msg(tsn_ret_val));
        tsn_port_close(bridge_hdl);
        return SR_ERR_OPERATION_FAILED;
    }

    /* get current number of used dynamic VLANs */
    if (EXIT_SUCCESS != get_dynamic_entries(dynamic_entries, MAX_NUM_OF_INTERFACES,
                                            MAX_STR_LEN, bridge_name)) {
        sr_set_error(session, node->xpath, ERR_GET_FUNC_FAILED_STR,
                     Q_BRIDGE_DYNAMIC_VLAN_REG_ENTRY_STR, bridge_name);
        tsn_port_close(bridge_hdl);
        return SR_ERR_OPERATION_FAILED;
    }

    num_dynamic_vlans = strtoul(dynamic_entries[0], NULL, 10);

    /* get all vlans from the board */
    if (EXIT_SUCCESS != get_bridge_vlans(bridge_vlans, BASE_MAX_VID, BASE_TEN, 0)) {
        sr_set_error(session, node->xpath, ERR_GET_FUNC_FAILED_STR,
                     Q_BRIDGE_VLAN_STR, bridge_name);
        tsn_port_close(bridge_hdl);
        return SR_ERR_OPERATION_FAILED;
    }

    /* take only uniq elements */
    array_uniq_elements(0, 0, 0, 0, NULL, BASE_RC_NO, vids_array,
                        BASE_MAX_VID, BASE_TEN);
    i = 0;
    new_vids_cnt = 0;
    /* Count the sent vids which do not exist on the board */
    while ((i < BASE_MAX_VID) && (0 != strncmp(vids_array[i], "", BASE_TEN))) {
        /* go through vlans from the board */
        j = 0;
        while ((j < BASE_MAX_VID) &&
               (0 != strncmp("", bridge_vlans[j], BASE_TEN))) {
            if (0 == strncmp(vids_array[i], bridge_vlans[j], BASE_TEN)) {
                vid_exist++;
                break;
            }
            j++;
        }

        if (0 == vid_exist) {
            new_vids_cnt++;
        }
        vid_exist = 0;
        i++;
    }

    /* check is total number of vids(static-vlan-registration-entries +
     * dynamic-vlan-registration-entries + new entries) less than max supported
     * vlans
     */
    if ((num_vlans + num_dynamic_vlans + new_vids_cnt) > max_supported_vlans) {
        sr_set_error(session, node->xpath, ERR_BRIDGE_TOO_MANY_VIDS_XPATH_STR,
                     max_supported_vlans, node->xpath);
        tsn_port_close(bridge_hdl);
        return SR_ERR_OPERATION_FAILED;
    }

    sr_free_val(old_value);
    sr_free_val(new_value);
    sr_free_values(items, counter);
    sr_free_change_iter(iter);
    node = NULL;
    iter = NULL;

    return SR_ERR_OK;
}

/**
 * @brief Callback to be called by the event of changing content of list
 * port-map inside list vlan-registration-entry.
 *
 * Subscribe to it by sr_module_change_subscribe call.
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
static int bridge_vlan_entry_port_map_cb(sr_session_ctx_t *session,
                                         const char *module_name,
                                         const char *xpath,
                                         sr_event_t event,
                                         uint32_t request_id,
                                         void *private_data)
{
    (void)module_name;
    (void)request_id;
    (void)private_data;

    int rc = SR_ERR_OK;
    sr_change_oper_t op = {0};
    sr_xpath_ctx_t st = {0};
    sr_change_iter_t *iter = NULL;
    sr_val_t *old_value = NULL;
    sr_val_t *new_value = NULL;
    sr_val_t *node = NULL;
    sr_val_t *vlan_transmitted = NULL;
    sr_val_t *vlan_reg_entry = NULL;
    size_t val_cnt = 0;
    char *port_name = NULL;
    char bridge_name[MAX_STR_LEN] = "";
    char vids[MAX_STR_LEN] = "";
    char database_id[MAX_STR_LEN] = "";
    char path[MAX_STR_LEN] = "";
    char port_ref_value[MAX_STR_LEN] = "";
    char pvid_array[BASE_MAX_VID][BASE_FIVE] = {0};
    char vids_array[BASE_MAX_VID][BASE_TEN] = {0};
    struct port_hdl *bridge_hdl = NULL;
    int i = 0;

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    if ((SR_EV_DONE == event) || (0 == plugin_init)) {
        SRP_LOG_DBG(DEBUG_MSG_WITH_TWO_PARAM, DBG_APPLYING_CHANGES_MSG, __func__);
        return SR_ERR_OK;
    }

    rc = sr_get_changes_iter(session, xpath, &iter);
    if (SR_ERR_OK != rc) {
        return rc;
    }

    while (SR_ERR_OK == sr_get_change_next(session, iter, &op,
                                           &old_value, &new_value)) {
        node = (op == SR_OP_DELETED) ? old_value : new_value;

        /* if port-map entry is deleted or added */
        if ((true == sr_xpath_node_name_eq(node->xpath, Q_BRIDGE_PORT_REF_STR)) &&
            ((op == SR_OP_CREATED) ||
             (op == SR_OP_DELETED))) {
            /* Get database-id value. That is a key inside vlan-registration-entry list */
            if (EXIT_SUCCESS != get_key_value(session, node->xpath,
                                              Q_BRIDGE_VLAN_REG_ENTRY,
                                              Q_BRIDGE_DATABASE_ID,
                                              &st, database_id)) {
                return SR_ERR_OPERATION_FAILED;
            }

            if (0 != strncmp(database_id, BASE_ONE_STR, 1)) {
                sr_set_error(session, node->xpath,
                             ERR_BRIDGE_DATABE_ID_SUPPORETD_XPATH_STR,
                             BASE_ONE_STR, node->xpath);
                return SR_ERR_OPERATION_FAILED;
            }

            /* Get vids value. That is a key inside vlan-registration-entry list */
            if (EXIT_SUCCESS != get_key_value(session, node->xpath,
                                              Q_BRIDGE_VLAN_REG_ENTRY,
                                              Q_BRIDGE_VIDS_STR, &st, vids)) {
                return SR_ERR_OPERATION_FAILED;
            }

            /* Part that checks if vids contains descending interval is not
             * covered by sysrepo
             */
            if (does_vids_string_contains_descending_interval(vids)) {
                sr_set_error(session, node->xpath,
                             ERR_BRIDGE_DESCENDING_VIDS_XPATH_STR,
                             vids, node->xpath);
                return SR_ERR_OPERATION_FAILED;
            }
            /* Checking valid vid range is not covered by sysrepo, so we need
             * to do it
             */
            if (does_vids_string_contains_invalid_vid(vids)) {
                sr_set_error(session, node->xpath,
                             ERR_BRIDGE_INVALID_RANGE_VIDS_XPATH_STR,
                             vids, node->xpath);
                return SR_ERR_OPERATION_FAILED;
            }

            /* Assemble vids array from vids that we got in edit-config request */
            assemble_vids_as_array(vids_array, BASE_MAX_VID, BASE_TEN, vids);

            /* Get bridge name value. That is a key inside bridge and component list */
            if (EXIT_SUCCESS != get_key_value(session, node->xpath,
                                              Q_BRIDGE_BRIDGE_ENTRY,
                                              Q_BRIDGE_NAME_STR,
                                              &st, bridge_name)) {
                return SR_ERR_OPERATION_FAILED;
            }

            /* After bridge name is found, open port to get current number of used VLANs */
            bridge_hdl = tsn_port_open_by_name(bridge_name);
            if (NULL == bridge_hdl) {
                sr_set_error(session, node->xpath,
                             ERR_PORT_NOT_OPEN_XPATH_STR, bridge_name);
                return SR_ERR_OPERATION_FAILED;
            }

            /* Get bridge name again, because tsn_port_open_bt_name sets bridge name to empty */
            if (EXIT_SUCCESS != get_key_value(session, node->xpath,
                                              Q_BRIDGE_BRIDGE_ENTRY,
                                              Q_BRIDGE_NAME_STR,
                                              &st, bridge_name)) {
                return SR_ERR_OPERATION_FAILED;
            }

            /* Get port-ref value from xpath of node */
            if (EXIT_SUCCESS != get_key_value(session, node->xpath,
                                              Q_BRIDGE_PORT_MAP,
                                              Q_BRIDGE_PORT_REF_STR,
                                              &st, port_ref_value)) {
                return SR_ERR_OPERATION_FAILED;
            }

            /* check all port-ref values
             *
             * we need id to identify every port-map list with proper port-ref
             * leaf
             *
             * retrieve vlan-registration-entry subtree that is stored under
             * the provided xpath
             */
            if (op != SR_OP_DELETED) {
                fill_xpath(path, BRIDGE_PORT_REF_XPATH, bridge_name, bridge_name, database_id, vids);
                if (SR_ERR_OK != sr_get_items(session, path, 0, 0, &vlan_reg_entry, &val_cnt)) {
                    sr_set_error(session, path, ERR_FAILED_GET_OBJ_STR, path);
                    tsn_port_close(bridge_hdl);
                    return SR_ERR_OPERATION_FAILED;
                }

                i = 0;
                /* Go trough vlan-registration-entry check all port-ref values
                 * we need id to identify every port-map list with proper
                 * port-ref leaf
                 */
                for (i = 0; i < (int)val_cnt; i++) {
                    /* If port-ref was found */
                    if (0 == strncmp(sr_xpath_node_name(vlan_reg_entry[i].xpath),
                                     Q_BRIDGE_PORT_REF_STR, MAX_STR_LEN)) {
                        port_name = tsn_get_name_by_id(bridge_hdl, vlan_reg_entry[i].data.uint32_val);
                        if (NULL == port_name) {
                            sr_set_error(session, path,
                                         "Failed to get port name by ID at '%s'.",
                                         path);
                            tsn_port_close(bridge_hdl);
                            return SR_ERR_OPERATION_FAILED;
                        }
                    }
                }
                sr_free_values(vlan_reg_entry, val_cnt);

                /* get vlan-transmitted value */
                fill_xpath(path, BRIDGE_VLAN_TRNASMITTED_XPATH, bridge_name, bridge_name,
                           database_id, vids, port_ref_value);
                if (SR_ERR_OK != sr_get_item(session, path, 0, &vlan_transmitted)) {
                    sr_set_error(session, path, ERR_FAILED_GET_OBJ_STR, path);
                    tsn_port_close(bridge_hdl);
                    return SR_ERR_OPERATION_FAILED;
                }
            }

            /* Get port-ref value from xpath of node */
            if (EXIT_SUCCESS != get_key_value(session, node->xpath,
                                              Q_BRIDGE_PORT_MAP,
                                              Q_BRIDGE_PORT_REF_STR,
                                              &st, port_ref_value)) {
                return SR_ERR_OPERATION_FAILED;
            }

            /* get interface name from port_ref_value */
            port_name = tsn_get_name_by_id(bridge_hdl, (uint32_t)strtoul(port_ref_value, NULL, 10));
            if (NULL == port_name) {
                sr_set_error(session, path, "Failed to get port name by ID at '%s'.", node->xpath);
                tsn_port_close(bridge_hdl);
                return SR_ERR_OPERATION_FAILED;
            }
            tsn_port_close(bridge_hdl);

            if (EXIT_SUCCESS == get_q_bridge_port_pvid(pvid_array, BASE_MAX_VID, BASE_FIVE, port_name)) {
                /* This condition is changed because of applying startup
                 * configuration. When vlan-registration-entry with vids = 1 is
                 * added, because it is in startup datastore, do not return
                 * error message, that pvid is in vids continue with callback
                 * (continue adding other correct vlan-registration-entries).
                 * Error message is returned only if pvid is inside vids and if
                 * plugin_init == 2. That means that user is adding wrong
                 * vlan-registration-entry.
                 */
                if ((0 != is_pvid_in_vids_string(pvid_array[0], vids)) &&
                    (plugin_init == 2)) {
                    sr_set_error(session, path, ERR_BRIDGE_PVID_IN_VIDS_XPATH_STR, path);
                    return SR_ERR_OPERATION_FAILED;
                }

            } else {
                sr_set_error(session, path, ERR_BRIDGE_NO_PVID_XPATH_STR, path);
                return SR_ERR_OPERATION_FAILED;
            }

            i = 0;
            /* If pvid is not inside vids, that add or delete vlan-registration-entry, depends from operation. */
            if (0 == is_pvid_in_vids_string(pvid_array[0], vids)) {
                while ((i < BASE_MAX_VID) &&
                       (0 != strncmp(vids_array[i], "", BASE_TEN))) {
                    if (op == SR_OP_DELETED) {
                        if (EXIT_SUCCESS != bridge_vlan_mod_dev_vid(port_name, vids_array[i],
                                                                    Q_BRIDGE_DEL_DEV_VID_STR, "")) {
                            sr_set_error(session, path,
                                         ERR_SET_FUNC_FAILED_XPATH_STR, path);
                            return SR_ERR_OPERATION_FAILED;
                        }
                    }
                    if (op == SR_OP_CREATED) {
                        if (EXIT_SUCCESS != bridge_vlan_mod_dev_vid(port_name, vids_array[i],
                                                                    Q_BRIDGE_ADD_DEV_VID_STR,
                                                                    vlan_transmitted->data.enum_val)) {
                            sr_set_error(session, path,
                                         ERR_SET_FUNC_FAILED_XPATH_STR, path);
                            return SR_ERR_OPERATION_FAILED;
                        }
                    }
                    i++;
                }
            }
        }
    }

    sr_free_val(old_value);
    sr_free_val(new_value);
    sr_free_val(vlan_transmitted);
    sr_free_change_iter(iter);
    node = NULL;
    iter = NULL;

    return SR_ERR_OK;
}

/**
 * @brief This function finds node whose name is element_name, using start_node.
 *
 * @param[in]   start_node      Pointer to struct sr_node_t that represents
 *                              start node from which the desired node will be
 *                              looked for.
 * @param[in]   element_name    Name of the node that we are searching.
 * @return      Pointer to struct sr_node_t, that represents node that we are
 *              looking for. If element is not found, NULL is returned.
 */
struct lyd_node *find_lyd_node_element_by_name(struct lyd_node *start_node,
                                               char *element_name)
{
    struct lyd_node *tmp = NULL;
    struct lyd_node *ret_value = NULL;

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    if (NULL != start_node) {
        for (tmp=start_node; tmp!=NULL; tmp=tmp->next) {
            /* we need id to identify right component */
            if (0 == strncmp(((struct lys_node_leaf*)tmp)->name, element_name,
                             MAX_STR_LEN)) {
                ret_value = tmp;
                break;
            }
        }
    }

    /* Note: ret_value should be used, because if node with element_name is not
     * found, then return value (ret_value) is going go be NULL
     */
    return ret_value;
}

/**
 * @brief Function that fills up structure filtering_entry_t that is used
 * inside filtering-entry port-map callback.
 *
 * @param[out]  filtering_entry_s   Structure filtering_entry_t.
 * @param[in]   session             Automatically-created session that can be
 *                                  used for obtaining changed data (e.g. by
 *                                  sr_get_changes_iter call or sr_get_item
 *                                  -like calls). Do not stop this session.
 * @param[in]   bridge_name         Name of the bridge.
 * @param[in]   database_id         Database id of bridge.
 * @param[in]   vids                Vids of bridge.
 * @param[in]   address             Bridge address.
 * @param[in]   port_ref            Bridge port-ref value.
 * @return      Error code (SR_ERR_OK on success).
 */
static int fill_filtering_entry(filtering_entry_t *filtering_entry_s,
                                sr_session_ctx_t *session, char *bridge_name,
                                char *database_id, char *vids, char *address,
                                char *port_ref)
{
    char database[MAX_STR_LEN] = "";
    char path[MAX_STR_LEN] = "";

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    /* find and fill leaf database-id from list filtering-entry */
    fill_xpath(path, BRIDGE_FILTERING_ENTRY_DATABASE_ID_XPATH, bridge_name, bridge_name, database_id, vids, address);
    if (SR_ERR_OK != sr_get_item(session, path, 0, &filtering_entry_s->database_id)) {
        sr_set_error(session, path, ERR_MISSING_ELEMENT_XPATH_STR, path);
        return SR_ERR_OPERATION_FAILED;
    }

    snprintf(database, MAX_STR_LEN, "%d", filtering_entry_s->database_id->data.uint32_val);

    if (0 != strncmp(database, BASE_ONE_STR, 1)) {
        sr_set_error(session, path, ERR_BRIDGE_DATABE_ID_SUPPORETD_XPATH_STR, path);
        return SR_ERR_OPERATION_FAILED;
    }

    /* find and fill leaf address from list filtering-entry */
    fill_xpath(path, BRIDGE_FILTERING_ENTRY_ADDRESS_XPATH, bridge_name, bridge_name, database_id, vids, address);
    if (SR_ERR_OK != sr_get_item(session, path, 0, &filtering_entry_s->address)) {
        sr_set_error(session, path, ERR_MISSING_ELEMENT_XPATH_STR, path);
        return SR_ERR_OPERATION_FAILED;
    }

    /* find and fill leaf vids from list filtering-entry */
    fill_xpath(path, BRIDGE_FILTERING_ENTRY_VIDS_XPATH, bridge_name, bridge_name, database_id, vids, address);
    if (SR_ERR_OK != sr_get_item(session, path, 0, &filtering_entry_s->vids)) {
        sr_set_error(session, path, ERR_MISSING_ELEMENT_XPATH_STR, path);
        return SR_ERR_OPERATION_FAILED;
    }

    /* Checking valid vid range is not covered by sysrepo, so we need to do it. */
    if (does_vids_string_contains_descending_interval(filtering_entry_s->vids->data.string_val)) {
        sr_set_error(session, path, ERR_BRIDGE_DESCENDING_VIDS_XPATH_STR, vids, path);
        return SR_ERR_OPERATION_FAILED;
    }

    /* Checking valid vid range is not covered by sysrepo, so we need to do it. */
    if (does_vids_string_contains_invalid_vid(filtering_entry_s->vids->data.string_val)) {
        sr_set_error(session, path, ERR_BRIDGE_INVALID_RANGE_VIDS_XPATH_STR, vids, path);
        return SR_ERR_OPERATION_FAILED;
    }

    /* find and fill leaf entry-type from list filtering-entry */
    fill_xpath(path, BRIDGE_FILTERING_ENTRY_ENTRY_TYPE_XPATH, bridge_name, bridge_name, database_id, vids, address);
    if (SR_ERR_OK != sr_get_item(session, path, 0, &filtering_entry_s->entry_type)) {
        sr_set_error(session, path, ERR_MISSING_ELEMENT_XPATH_STR, path);
        return SR_ERR_OPERATION_FAILED;
    }

    if (0 != strncmp(filtering_entry_s->entry_type->data.enum_val, Q_BRIDGE_STATIC, MAX_STR_LEN)) {
        sr_set_error(session, path, "Only 'static' is supported for '%s'.", path);
        return SR_ERR_OPERATION_FAILED;
    }

    /* find and fill leaf name from list bridge */
    fill_xpath(path, BRIDGE_NAME_XPATH, bridge_name);
    if (SR_ERR_OK != sr_get_item(session, path, 0, &filtering_entry_s->bridge_name)) {
        sr_set_error(session, path, ERR_MISSING_ELEMENT_XPATH_STR, path);
        return SR_ERR_OPERATION_FAILED;
    }

    /* find and fill leaf control-element from list port-map */
    fill_xpath(path, BRIDGE_FILTERING_ENTRY_CONTROL_ELEMENT_XPATH, bridge_name, bridge_name, database_id, vids, address, port_ref);
    if (SR_ERR_OK != sr_get_item(session, path, 0, &filtering_entry_s->control_el)) {
        sr_set_error(session, path, ERR_MISSING_ELEMENT_XPATH_STR, path);
        return SR_ERR_OPERATION_FAILED;
    }

    if ((0 == strncmp(Q_BRIDGE_FILTER, filtering_entry_s->control_el->data.enum_val, MAX_STR_LEN)) ||
       (0 == strncmp(Q_BRIDGE_FORWARD_FILTER, filtering_entry_s->control_el->data.enum_val, MAX_STR_LEN)))
    {
        sr_set_error(session, path, Q_BRIDGE_CTRL_EL_MSG_ERR);
        return SR_ERR_OPERATION_FAILED;
    }

    return SR_ERR_OK;
}

/**
 * @brief Callback to be called by the event of changing content of list
 * filtering-entry inside container filtering-database.
 *
 * Subscribe to it by sr_module_change_subscribe call.
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
static int bridge_filtering_entry_port_map_abort_cb(sr_session_ctx_t *session,
                                                    const char *module_name,
                                                    const char *xpath,
                                                    sr_event_t event,
                                                    uint32_t request_id,
                                                    void *private_data)
{
    (void)module_name;
    (void)request_id;
    (void)private_data;

    int rc = SR_ERR_OK;
    sr_change_oper_t op = {0};
    sr_xpath_ctx_t st = {0};
    sr_change_iter_t *iter = NULL;
    sr_val_t *old_value = NULL;
    sr_val_t *new_value = NULL;
    sr_val_t *node = NULL;
    char *port_name = NULL;
    char bridge_name[MAX_STR_LEN] = "";
    char vids[MAX_STR_LEN] = "";
    char database_id[MAX_STR_LEN] = "";
    char address[MAX_STR_LEN] = "";
    char port_ref[MAX_STR_LEN] = "";
    struct port_hdl *bridge = NULL;

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    if ((SR_EV_DONE == event) || (0 == plugin_init)) {
        SRP_LOG_DBG(DEBUG_MSG_WITH_TWO_PARAM, DBG_APPLYING_CHANGES_MSG, __func__);
        return SR_ERR_OK;
    }

    if (SR_EV_ABORT == event) {
        rc = sr_get_changes_iter(session, xpath, &iter);
        if (SR_ERR_OK != rc) {
            return rc;
        }

        while (SR_ERR_OK == sr_get_change_next(session, iter, &op,
                                               &old_value, &new_value)) {
            node = (op == SR_OP_DELETED) ? old_value : new_value;
            /* if port-map entry is deleted or added inside filtering-entry list */
            if (op == SR_OP_DELETED) {
                /* Get bridge name value. That is a key inside bridge and component list */
                if (EXIT_SUCCESS != get_key_value(session, node->xpath,
                                                  Q_BRIDGE_BRIDGE_ENTRY,
                                                  Q_BRIDGE_NAME_STR,
                                                  &st, bridge_name)) {
                    return SR_ERR_OPERATION_FAILED;
                }

                /* Get database-id value. That is a key inside filtering-entry list */
                if (EXIT_SUCCESS != get_key_value(session, node->xpath,
                                                  Q_BRIDGE_FILTERING_ENTRY,
                                                  Q_BRIDGE_DATABASE_ID,
                                                  &st, database_id)) {
                    return SR_ERR_OPERATION_FAILED;
                }

                /* Get vids value. That is a key inside filtering-entry list */
                if (EXIT_SUCCESS != get_key_value(session, node->xpath,
                                                  Q_BRIDGE_FILTERING_ENTRY,
                                                  Q_BRIDGE_VIDS_STR,
                                                  &st, vids)) {
                    return SR_ERR_OPERATION_FAILED;
                }

                /* Get address value. That is a key inside filtering-entry list */
                if (EXIT_SUCCESS != get_key_value(session, node->xpath,
                                                  Q_BRIDGE_FILTERING_ENTRY,
                                                  Q_BRIDGE_ADDRESS_STR,
                                                  &st, address)) {
                    return SR_ERR_OPERATION_FAILED;
                }

                /* Get address value. That is a key inside port-map list */
                if (EXIT_SUCCESS != get_key_value(session, node->xpath,
                                                  Q_BRIDGE_PORT_MAP,
                                                  Q_BRIDGE_PORT_REF_STR,
                                                  &st, port_ref)) {
                    return SR_ERR_OPERATION_FAILED;
                }

                /* After bridge name is found, open port to get current number
                 * of used VLANs
                 */
                bridge = tsn_port_open_by_name(bridge_name);
                if (NULL == bridge) {
                    sr_set_error(session, node->xpath,
                                 ERR_PORT_NOT_OPEN_XPATH_STR, bridge_name);
                    return SR_ERR_OPERATION_FAILED;
                }

                port_name = tsn_get_name_by_id(bridge, (uint32_t)strtoul(port_ref, NULL, 10));
                if (NULL == port_name) {
                    sr_set_error(session, node->xpath,
                                 "Failed to get port name by ID at '%s'.", node->xpath);
                    tsn_port_close(bridge);
                    return SR_ERR_OPERATION_FAILED;
                }

                if (EXIT_SUCCESS != bridge_fdb_rem_dev_filt_entry(port_name, address, vids)) {
                    sr_set_error(session, node->xpath,
                                 ERR_SET_FUNC_FAILED_XPATH_STR, node->xpath);
                    tsn_port_close(bridge);
                    return SR_ERR_OPERATION_FAILED;
                }
                tsn_port_close(bridge);
            }
        }
    }

    sr_free_val(old_value);
    sr_free_val(new_value);
    sr_free_change_iter(iter);
    node = NULL;
    iter = NULL;

    return SR_ERR_OK;
}

/**
 * @brief Callback to be called by the event of changing content of list
 * filtering-entry inside container filtering-database.
 *
 * Subscribe to it by sr_module_change_subscribe call.
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
static int bridge_filtering_entry_port_map_cb(sr_session_ctx_t *session,
                                              const char *module_name,
                                              const char *xpath,
                                              sr_event_t event,
                                              uint32_t request_id,
                                              void *private_data)
{
    (void)module_name;
    (void)request_id;
    (void)private_data;

    int rc = SR_ERR_OK;
    sr_change_oper_t op = {0};
    sr_xpath_ctx_t st = {0};
    sr_change_iter_t *iter = NULL;
    sr_val_t *old_value = NULL;
    sr_val_t *new_value = NULL;
    sr_val_t *node = NULL;
    char *port_name = NULL;
    char path[MAX_STR_LEN] = "";
    char bridge_name[MAX_STR_LEN] = "";
    char vids[MAX_STR_LEN] = "";
    char database_id[MAX_STR_LEN] = "";
    char address[MAX_STR_LEN] = "";
    char port_ref[MAX_STR_LEN] = "";
    char new_vids_array[BASE_MAX_VID][BASE_FIVE] = {0};
    char port_vlans[BASE_MAX_VID][BASE_TEN] = {0};
    char tmp_port_vlans[BASE_MAX_VID][BASE_TEN] = {0};
    struct bridge_attr_and_caps bridge_attr = {0};
    filtering_entry_t filtering_entry_s = {0};
    struct port_hdl *bridge = NULL;
    int i = 0;
    int j = 0;
    int k = 0;
    int new_f_entry_cnt = 0;
    int vid_exist = 0;
    sr_val_t *filtering_database = NULL;
    size_t counter = 0;
    bool check_done = false;

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    if ((SR_EV_DONE == event) || (0 == plugin_init)) {
        SRP_LOG_DBG(DEBUG_MSG_WITH_TWO_PARAM, DBG_APPLYING_CHANGES_MSG, __func__);
        return SR_ERR_OK;
    }

    rc = sr_get_changes_iter(session, xpath, &iter);
    if (SR_ERR_OK != rc) {
        return rc;
    }

    while (SR_ERR_OK == sr_get_change_next(session, iter, &op,
                                           &old_value, &new_value)) {
        node = (op == SR_OP_DELETED) ? old_value : new_value;

        /* if port-map entry is deleted or added inside filtering-entry list */
        if ((true == sr_xpath_node_name_eq(node->xpath, Q_BRIDGE_PORT_REF_STR)) &&
            ((op == SR_OP_CREATED) ||
             (op == SR_OP_DELETED))) {
            /* Get bridge name value. That is a key inside bridge and component
             * list
             */
            if (EXIT_SUCCESS != get_key_value(session, node->xpath,
                                              Q_BRIDGE_BRIDGE_ENTRY,
                                              Q_BRIDGE_NAME_STR,
                                              &st, bridge_name)) {
                return SR_ERR_OPERATION_FAILED;
            }

            /* Get database-id value. That is a key inside filtering-entry list */
            if (EXIT_SUCCESS != get_key_value(session, node->xpath,
                                              Q_BRIDGE_FILTERING_ENTRY,
                                              Q_BRIDGE_DATABASE_ID,
                                              &st, database_id)) {
                return SR_ERR_OPERATION_FAILED;
            }

            /* Get vids value. That is a key inside filtering-entry list */
            if (EXIT_SUCCESS != get_key_value(session, node->xpath,
                                              Q_BRIDGE_FILTERING_ENTRY,
                                              Q_BRIDGE_VIDS_STR, &st, vids)) {
                return SR_ERR_OPERATION_FAILED;
            }

            /* Get address value. That is a key inside filtering-entry list */
            if (EXIT_SUCCESS != get_key_value(session, node->xpath,
                                              Q_BRIDGE_FILTERING_ENTRY,
                                              Q_BRIDGE_ADDRESS_STR,
                                              &st, address)) {
                return SR_ERR_OPERATION_FAILED;
            }

            /* Get address value. That is a key inside port-map list */
            if (EXIT_SUCCESS != get_key_value(session, node->xpath,
                                              Q_BRIDGE_PORT_MAP,
                                              Q_BRIDGE_PORT_REF_STR,
                                              &st, port_ref)) {
                return SR_ERR_OPERATION_FAILED;
            }

            /* After bridge name is found, open port to get current number of
             * used VLANs
             */
            bridge = tsn_port_open_by_name(bridge_name);
            if (NULL == bridge) {
                sr_set_error(session, node->xpath,
                             ERR_PORT_NOT_OPEN_XPATH_STR, bridge_name);
                return SR_ERR_OPERATION_FAILED;
            }

            port_name = tsn_get_name_by_id(bridge, (uint32_t)strtoul(port_ref, NULL, 10));
            if (NULL == port_name) {
                sr_set_error(session, node->xpath,
                             "Failed to get port name by ID at '%s'.",
                             node->xpath);
                tsn_port_close(bridge);
                return SR_ERR_OPERATION_FAILED;
            }

            if (SR_OP_DELETED == op) {
                /* Here we are using port_ref, address and vids filled above,
                 * because if we have delete operation, than we can not use
                 * sr_get_subtree to retrieve filtering-entry and port-map,
                 * because they do not exit inside configuration that is sent.
                 */

                /* Part that checks if vids contains descending interval is not
                 * covered by sysrepo
                 */
                if (does_vids_string_contains_descending_interval(vids)) {
                    sr_set_error(session, node->xpath,
                                 ERR_BRIDGE_DESCENDING_VIDS_XPATH_STR,
                                 vids, node->xpath);
                    return SR_ERR_OPERATION_FAILED;
                }

                /* Checking valid vid range is not covered by sysrepo, so we
                 * need to do it.
                 */
                if (does_vids_string_contains_invalid_vid(vids)) {
                    sr_set_error(session, node->xpath,
                                 ERR_BRIDGE_INVALID_RANGE_VIDS_XPATH_STR,
                                 vids, node->xpath);
                    return SR_ERR_OPERATION_FAILED;
                }

                if (EXIT_SUCCESS != bridge_fdb_rem_dev_filt_entry(port_name, address, vids)) {
                    sr_set_error(session, node->xpath,
                                 ERR_SET_FUNC_FAILED_XPATH_STR, node->xpath);
                    return SR_ERR_OPERATION_FAILED;
                }
            }

            if (op == SR_OP_CREATED) /* MODIFY operation is not supported */
            {
                if (SR_ERR_OK != fill_filtering_entry(&filtering_entry_s, session, bridge_name,
                                                      database_id, vids, address, port_ref)) {
                    return SR_ERR_OPERATION_FAILED;
                }

                if (false == check_done) {
                    /* Get filtering-database so we can iterate trough all
                     * filtering-entries
                     */
                    fill_xpath(path, BRIDGE_FILTERING_DATABASE_XPATH, bridge_name, bridge_name);
                    if (SR_ERR_OK != sr_get_items(session, path, 0, 0,
                                                  &filtering_database, &counter)) {
                        sr_set_error(session, path,
                                     ERR_FAILED_GET_SUBTREE_STR, path);
                        return SR_ERR_OPERATION_FAILED;
                    }

                    i = 0;
                    for (i = 0; i < (int)counter; i++) {
                        /* if vids insde filtering-databse is found */
                        if ((strstr(filtering_database[i].xpath, Q_BRIDGE_FILTERING_ENTRY) != NULL) &&
                            (strstr(filtering_database[i].xpath, "/vids") != NULL)) {
                            assemble_vids_as_array(new_vids_array, BASE_MAX_VID,
                                                   BASE_FIVE, filtering_database[i].data.string_val);

                            /* Count the sent vids which do not exist on the board */
                            k = 0;

                            while ((k < BASE_MAX_VID) &&
                                   (0 != strncmp(new_vids_array[k], "", BASE_FIVE))) {
                                if (EXIT_FAILURE == does_static_fdb_entry_exist(filtering_database[i].data.string_val,
                                                                                new_vids_array[k])) {
                                    new_f_entry_cnt++;
                                }
                                k++;
                            }
                        }
                    }
                    sr_free_values(filtering_database, counter);
                    filtering_database = NULL;
                    counter = 0;

                    if (0 == tsn_bridge_attr(bridge, &bridge_attr)) {
                        if (bridge_attr.FdbSize < (new_f_entry_cnt + bridge_attr.FdbStaticEntries)) {
                            sr_set_error(session, filtering_entry_s.bridge_name->xpath,
                                         Q_BRIDGE_MSG_ERR_TO_MANY_S_F_ENTRIES);
                            tsn_port_close(bridge);
                            return SR_ERR_OPERATION_FAILED;
                        }
                    }
                    new_f_entry_cnt = 0;

                }
                check_done = true;
                tsn_port_close(bridge);

                assemble_vids_as_array(tmp_port_vlans, BASE_MAX_VID,
                                       BASE_FIVE, filtering_entry_s.vids->data.string_val);
                /* Get port vlan ids */
                if (EXIT_FAILURE == get_port_vlans_by_name(port_vlans, BASE_MAX_VID,
                                                           BASE_TEN, 0, port_name)) {
                    // 0 is for uncompressed vlan ids
                    sr_set_error(session, node->xpath,
                                 ERR_GET_FUNC_FAILED_XPATH_STR, node->xpath);
                    return SR_ERR_OPERATION_FAILED;
                }
                i = 0;
                while ((i < BASE_MAX_VID) &&
                       (0 != strncmp(tmp_port_vlans[i], "", BASE_TEN))) {
                    j = 0;
                    while ((j < BASE_MAX_VID) &&
                           (0 != strncmp(port_vlans[j], "", BASE_TEN))) {
                        if (0 == strncmp(tmp_port_vlans[i], port_vlans[j], BASE_TEN)) {
                            vid_exist++;
                            break;
                        }
                        j++;
                    }
                    if (vid_exist) {
                        vid_exist = 0;
                    } else {
                        sr_set_error(session, node->xpath,
                                     ERR_BRIDGE_NO_PVID_XPATH_STR, node->xpath);
                        return SR_ERR_OPERATION_FAILED;
                    }
                    i++;
                }

                if (EXIT_SUCCESS != bridge_fdb_add_dev_filt_entry(port_name, filtering_entry_s.address->data.string_val,
                                                                  filtering_entry_s.vids->data.string_val)) {
                    sr_set_error(session, node->xpath,
                                 ERR_SET_FUNC_FAILED_XPATH_STR, node->xpath);
                    return SR_ERR_OPERATION_FAILED;
                }
            }
        }
    }

    sr_free_val(old_value);
    sr_free_val(new_value);
    sr_free_change_iter(iter);
    node = NULL;
    iter = NULL;

    /* Pointers fill_filtering_entry function */
    sr_free_val(filtering_entry_s.bridge_name);
    sr_free_val(filtering_entry_s.address);
    sr_free_val(filtering_entry_s.vids);
    sr_free_val(filtering_entry_s.database_id);
    sr_free_val(filtering_entry_s.entry_type);
    sr_free_val(filtering_entry_s.control_el);
    filtering_entry_s.bridge_name = NULL;
    filtering_entry_s.address = NULL;
    filtering_entry_s.vids = NULL;
    filtering_entry_s.database_id = NULL;
    filtering_entry_s.entry_type = NULL;
    filtering_entry_s.control_el = NULL;

    return SR_ERR_OK;
}

/**
 * @brief Callback to be called by the event of changing content of list
 * vid-to-fid inside container bridge-vlan.
 *
 * Subscribe to it by sr_module_change_subscribe call.
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
static int bridge_bridge_vlan_vid_to_fid_cb(sr_session_ctx_t *session,
                                            const char *module_name,
                                            const char *xpath,
                                            sr_event_t event,
                                            uint32_t request_id,
                                            void *private_data)
{
    (void)module_name;
    (void)request_id;
    (void)private_data;

    int rc = SR_ERR_OK;
    sr_change_oper_t op = {0};
    sr_xpath_ctx_t st = {0};
    sr_change_iter_t *iter = NULL;
    sr_val_t *old_value = NULL;
    sr_val_t *new_value = NULL;
    sr_val_t *node = NULL;
    char path[MAX_STR_LEN] = "";
    char vid[MAX_STR_LEN] = "";
    char fid_string[MAX_STR_LEN] = "";
    char bridge_name[MAX_STR_LEN] = "";
    sr_val_t *fid = NULL;

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    if ((SR_EV_DONE == event) || (0 == plugin_init)) {
        SRP_LOG_DBG(DEBUG_MSG_WITH_TWO_PARAM, DBG_APPLYING_CHANGES_MSG, __func__);
        return SR_ERR_OK;
    }

    rc = sr_get_changes_iter(session, xpath, &iter);
    if (SR_ERR_OK != rc) {
        return rc;
    }

    while (SR_ERR_OK == sr_get_change_next(session, iter, &op,
                                           &old_value, &new_value)) {
        node = (op == SR_OP_DELETED) ? old_value : new_value;

        /* if vlan entry is deleted or added inside vlan list */
        if ((true == sr_xpath_node_name_eq(node->xpath, Q_BRIDGE_VID_STR)) &&
            ((op == SR_OP_CREATED) ||
             (op == SR_OP_DELETED))) {
            /* Get all necessary data for delete and add operation */

            /* Get bridge name value. That is a key inside bridge and component
             * list
             */
            if (EXIT_SUCCESS != get_key_value(session, node->xpath,
                                              Q_BRIDGE_BRIDGE_ENTRY,
                                              Q_BRIDGE_NAME_STR,
                                              &st, bridge_name)) {
                return SR_ERR_OPERATION_FAILED;
            }

            /* Get vid value. That is a key inside vlan list */
            if (EXIT_SUCCESS != get_key_value(session, node->xpath,
                                              Q_BRIDGE_VID2FID_STR,
                                              Q_BRIDGE_VID_STR, &st, vid)) {
                return SR_ERR_OPERATION_FAILED;
            }

            if (op == SR_OP_CREATED) {
                /* get leaf vid from list vid-to-fid */
                fill_xpath(path, BRIDGE_VID_TO_FID_FID_XPATH,
                           bridge_name, bridge_name, vid);
                /* get vid-to-fid entry subtree */
                if (SR_ERR_OK != sr_get_item(session, path, 0, &fid)) {
                    sr_set_error(session, path, ERR_FAILED_GET_OBJ_STR, path);
                    return SR_ERR_OPERATION_FAILED;
                }
                /* TO DO:
                 * We may have to add here additional check, that exist on
                 * netopeer1 and it is commented. The check is inside
                 * callback_component_vid2fid function
                 */

                /* Convert fid from unit32 to string, because
                 * mstpctl_setvid2fid takes char
                 */
                snprintf(fid_string, MAX_STR_LEN, "%d", fid->data.uint32_val);
                if (EXIT_SUCCESS != mstpctl_setvid2fid(bridge_name, fid_string, vid)) {
                    sr_set_error(session, node->xpath,
                                 ERR_SET_FUNC_FAILED_XPATH_STR, node->xpath);
                    return SR_ERR_OPERATION_FAILED;
                }
            }

            if (op == SR_OP_DELETED) {
                if (EXIT_SUCCESS != mstpctl_setvid2fid(bridge_name, BASE_ZERO_STR, vid)) {
                    sr_set_error(session, node->xpath,
                                 ERR_SET_FUNC_FAILED_XPATH_STR, node->xpath);
                    return SR_ERR_OPERATION_FAILED;
                }
            }
        }
    }
    sr_free_val(old_value);
    sr_free_val(new_value);
    sr_free_change_iter(iter);
    sr_free_val(fid);
    node = NULL;
    iter = NULL;

    return SR_ERR_OK;
}

/**
 * @brief This callback handles fid value update.
 *
 * Subscribe to it by sr_module_change_subscribe call.
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
static int bridge_bridge_vlan_vid_to_fid_fid_cb(sr_session_ctx_t *session,
                                                const char *module_name,
                                                const char *xpath,
                                                sr_event_t event,
                                                uint32_t request_id,
                                                void *private_data)
{
    (void)module_name;
    (void)request_id;
    (void)private_data;

    sr_change_oper_t op = {0};
    sr_xpath_ctx_t ctx = {0};
    sr_change_iter_t *iter = NULL;
    sr_val_t *old_value = NULL;
    sr_val_t *new_value = NULL;
    sr_val_t *node = NULL;
    sr_val_t *tmp_node = NULL;
    char path[MAX_STR_LEN] = "";
    char vid[MAX_STR_LEN] = "";
    char fid_string[MAX_STR_LEN] = "";
    char bridge_name[MAX_STR_LEN] = "";

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    if ((SR_EV_DONE == event) || (0 == plugin_init)) {
        SRP_LOG_DBG(DEBUG_MSG_WITH_TWO_PARAM, DBG_APPLYING_CHANGES_MSG, __func__);
        return SR_ERR_OK;
    }

    if (SR_ERR_OK != sr_get_changes_iter(session, xpath, &iter)) {
        return SR_ERR_OPERATION_FAILED;
    }

    while (SR_ERR_OK == sr_get_change_next(session, iter, &op,
                                           &old_value, &new_value)) {

        node = (op == SR_OP_DELETED) ? old_value : new_value;

        /* Get bridge name value. That is a key inside bridge and component list */
        if (EXIT_SUCCESS != get_key_value(session, node->xpath,
                                          Q_BRIDGE_BRIDGE_ENTRY,
                                          Q_BRIDGE_NAME_STR,
                                          &ctx, bridge_name)) {
            return SR_ERR_OPERATION_FAILED;
        }

        /* Get vid value. That is a key inside vlan list */
        if (EXIT_SUCCESS != get_key_value(session, node->xpath,
                                          Q_BRIDGE_VID2FID_STR,
                                          Q_BRIDGE_VID_STR, &ctx, vid)) {
            return SR_ERR_OPERATION_FAILED;
        }

        /* Update fid value of existing vid-to-fid list entry */
        if (SR_OP_MODIFIED == op) {
            /* Convert fid to string */
            snprintf(fid_string, MAX_STR_LEN, "%d", node->data.uint32_val);

            /* Add new vid-to-fid entry */
            if (EXIT_SUCCESS != mstpctl_setvid2fid(bridge_name, fid_string, vid)) {
                sr_set_error(session, node->xpath,
                             ERR_SET_FUNC_FAILED_XPATH_STR, node->xpath);
                return SR_ERR_OPERATION_FAILED;
            }
        }

        /* Fid deletion is allowed only if entire vid-to-fid entry is deleted */
        if (SR_OP_DELETED == op) {

            fill_xpath(path, BRIDGE_COMPONENT__XPATH, bridge_name, bridge_name, vid);

            if (SR_ERR_OK == sr_get_item(session, path, 0, &tmp_node)) {
                sr_free_val(tmp_node);
                sr_set_error(session, path, ERR_FAILED_GET_OBJ_STR, path);
                return SR_ERR_OPERATION_FAILED;
            }
        }
    }

    /* free resources */
    sr_free_val(old_value);
    sr_free_val(new_value);
    sr_free_change_iter(iter);
    sr_free_val(tmp_node);
    node = NULL;
    iter = NULL;

    return SR_ERR_OK;
}

/**
 * @brief Function that checks if added vids inside vid-to-fid-allocation list
 * is valid.
 *
 * @param[in]   bridge_name     Name of the bridge.
 * @param[in]   vids            New added vids.
 * @return      EXIT_SUCCESS (EXIT_FAILURE if failed).
 */
int check_vids(char *bridge_name, char *vids)
{

    char result_array[BASE_MAX_VID][BASE_TEN] = {0};

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    /* if function could not get fid for desired vids then return error */
    if (EXIT_SUCCESS != get_mstp_fid_by_vids(result_array, BASE_MAX_VID,
                                             BASE_TEN, bridge_name, vids)) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

/**
 * @brief Function that checks if added fid inside fid-to-vid-allocation list
 * is valid.
 *
 * @param[in]   bridge_name     Name of the bridge.
 * @param[in]   fid             New added fid.
 * @return      EXIT_SUCCESS (EXIT_FAILURE if failed).
 */
int check_fid(char * bridge_name, char *fid)
{
    char result_array[BASE_ONE][MAX_STR_LEN] = {0};

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    /* if function could not get vids for desired fid then return error */
    if (EXIT_SUCCESS != get_mstp_vids_by_fid(result_array, BASE_ONE,
                                             MAX_STR_LEN, bridge_name, fid)) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

/**
 * @brief Function that checks if added vid inside vlan list is valid.
 *
 * @param[in]   bridge_name     Name of the bridge.
 * @return      EXIT_SUCCESS (EXIT_FAILURE if failed).
 */
int check_vlan_vid(char *vid)
{
    char bridge_vlans[BASE_MAX_VID][BASE_TEN] = {0};
    int i = 0;

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    /* if function could find desired vlan inside available vlans on board then
     * return error
     */
    if (EXIT_SUCCESS == get_bridge_vlans(bridge_vlans, BASE_MAX_VID, BASE_TEN, 0)) {
        // 0 is for uncompressed vlan ids
        while ((i < BASE_MAX_VID) &&
               (0 != strncmp(bridge_vlans[i], "", BASE_TEN))) {
            if (0 == strncmp(bridge_vlans[i], vid, MAX_STR_LEN)) {
                return EXIT_SUCCESS;
            }
            i++;
        }
    }

    return EXIT_FAILURE;
}

/**
 * @brief Callback to be called by the event of changing content of list vlan
 * inside container bridge-vlan.
 *
 * Subscribe to it by sr_module_change_subscribe call.
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
static int bridge_bridge_vlan_vlan_cb(sr_session_ctx_t *session,
                                      const char *module_name,
                                      const char *xpath,
                                      sr_event_t event,
                                      uint32_t request_id,
                                      void *private_data)
{
    (void)module_name;
    (void)request_id;
    (void)private_data;

    int rc = SR_ERR_OK;
    sr_change_oper_t op = {0};
    sr_xpath_ctx_t st = {0};
    sr_change_iter_t *iter = NULL;
    sr_val_t *old_value = NULL;
    sr_val_t *new_value = NULL;
    sr_val_t *node = NULL;
    char vid[MAX_STR_LEN] = "";
    char tmp_vid[MAX_STR_LEN] = "";
    char path[MAX_STR_LEN] = "";
    char bridge_name[MAX_STR_LEN] = "";
    sr_val_t *items = NULL;
    size_t counter = 0;
    int i = 0;

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    if ((SR_EV_DONE == event) || (0 == plugin_init)) {
        SRP_LOG_DBG(DEBUG_MSG_WITH_TWO_PARAM, DBG_APPLYING_CHANGES_MSG, __func__);
        return SR_ERR_OK;
    }

    rc = sr_get_changes_iter(session, xpath, &iter);
    if (SR_ERR_OK != rc) {
        return rc;
    }

    while (SR_ERR_OK == sr_get_change_next(session, iter, &op,
                                           &old_value, &new_value)) {
        node = (op == SR_OP_DELETED) ? old_value : new_value;
        /* if entry is deleted or added inside list */
        if ((true == sr_xpath_node_name_eq(node->xpath, Q_BRIDGE_VID_STR)) &&
            ((op == SR_OP_CREATED) ||
             (op == SR_OP_DELETED))) {
            /* Get all necessary data for delete and add operation */

            /* Get bridge name value. That is a key inside bridge and component
             * list
             */
            if (EXIT_SUCCESS != get_key_value(session, node->xpath,
                                              Q_BRIDGE_BRIDGE_ENTRY,
                                              Q_BRIDGE_NAME_STR,
                                              &st, bridge_name)) {
                return SR_ERR_OPERATION_FAILED;
            }

            /* Get vid value. That is a key inside vlan list */
            if (EXIT_SUCCESS != get_key_value(session, node->xpath,
                                              Q_BRIDGE_COMP_VLAN,
                                              Q_BRIDGE_VID_STR, &st, vid)) {
                return SR_ERR_OPERATION_FAILED;
            }

            /* free memory of vlan_array every time someone changes vlan list */
            if (vlan_array.vid_array != NULL) {
                for (i = 0; i < vlan_array.count; i++) {
                    free(vlan_array.vid_array[i] );
                }
                free(vlan_array.vid_array);
                vlan_array.vid_array = NULL;
                vlan_array.count = 0;
            }

            if (op == SR_OP_CREATED) {
                /* container bridge-vlan, list vlan */
                fill_xpath(path, BRIDGE_VLAN_LIST_XPATH, bridge_name, bridge_name);
                rc = sr_get_items(session, path, 0, 0, &items, &counter);
                if ((SR_ERR_OK != rc) && (SR_ERR_NOT_FOUND != rc)) {
                    // if item is not found the count is 0
                    sr_set_error(session, node->xpath,
                                 ERR_BRIDGE_FAILED_TO_GET_VLAN_LIST_STR,
                                 node->xpath);
                    return SR_ERR_OPERATION_FAILED;
                } else {
                    /* allocate memory and set count to number of retrieved
                     * vlan entries from configuration
                     */
                    vlan_array.count = (int)counter;
                    vlan_array.vid_array = malloc(sizeof(char*)*(int)counter);
                    if (vlan_array.vid_array != NULL) {
                        for (i = 0; i < ((int)counter); i++) {
                            vlan_array.vid_array[i] = malloc(MAX_STR_LEN);
                        }
                        i = 0;
                        for (i = 0; i < (int)counter; i++) {
                            if (sr_xpath_node_name_eq(items[i].xpath, Q_BRIDGE_VID_STR)) {
                                int ret = snprintf(tmp_vid, MAX_STR_LEN, "%u",
                                                   items[i].data.uint32_val);
                                memcpy(vlan_array.vid_array[i], tmp_vid, ret+1);
                            }
                        }
                    } else {
                        SRP_LOG_ERR(ERR_BRIDGE_FAILED_TO_ALLOCATE_MEMORY_STR,
                                    Q_BRIDGE_COMP_VLAN, __func__);
                        sr_set_error(session, node->xpath,
                                     ERR_BRIDGE_FAILED_TO_SET_VLAN_VID_STR,
                                     node->xpath);
                        return SR_ERR_OPERATION_FAILED;
                    }
                }
                sr_free_values(items, counter);

                if (EXIT_FAILURE == check_vlan_vid(vid)) {
                    sr_set_error(session, node->xpath,
                                 ERR_BRIDGE_FAILED_TO_SET_VLAN_VID_STR,
                                 node->xpath);
                    return SR_ERR_OPERATION_FAILED;
                }
            }
            /* if operation is SR_OP_DELETED, then nothing happens, entry is
             * deleted from configuration so no operation data for that entry
             * will be available in next get request
             */
        }
    }
    sr_free_val(old_value);
    sr_free_val(new_value);
    sr_free_change_iter(iter);
    node = NULL;
    iter = NULL;

    return SR_ERR_OK;
}

/**
 * @brief Callback to be called by the event of changing content of list
 * vid-to-fid-allocation inside container bridge-vlan.
 *
 * Subscribe to it by sr_module_change_subscribe call.
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
static int bridge_bridge_vlan_vid_to_fid_allocation_cb(
        sr_session_ctx_t *session, const char *module_name,
        const char *xpath, sr_event_t event,
        uint32_t request_id, void *private_data)
{
    (void)module_name;
    (void)request_id;
    (void)private_data;

    int rc = SR_ERR_OK;
    sr_change_oper_t op = {0};
    sr_xpath_ctx_t st = {0};
    sr_change_iter_t *iter = NULL;
    sr_val_t *old_value = NULL;
    sr_val_t *new_value = NULL;
    sr_val_t *node = NULL;
    char vids[MAX_STR_LEN] = "";
    char path[MAX_STR_LEN] = "";
    char bridge_name[MAX_STR_LEN] = "";
    sr_val_t *items = NULL;
    size_t counter = 0;
    int i = 0;

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    if ((SR_EV_DONE == event) || (0 == plugin_init)) {
        SRP_LOG_DBG(DEBUG_MSG_WITH_TWO_PARAM, DBG_APPLYING_CHANGES_MSG, __func__);
        return SR_ERR_OK;
    }

    rc = sr_get_changes_iter(session, xpath, &iter);
    if (SR_ERR_OK != rc) {
        return rc;
    }

    while (SR_ERR_OK == sr_get_change_next(session, iter, &op,
                                           &old_value, &new_value)) {
        node = (op == SR_OP_DELETED) ? old_value : new_value;
        /* if entry is deleted or added inside list */
        if ((true == sr_xpath_node_name_eq(node->xpath, Q_BRIDGE_VIDS_STR)) &&
            ((op == SR_OP_CREATED) ||
             (op == SR_OP_DELETED))) {
            /* Get all necessary data for delete and add operation */

            /* Get bridge name value. That is a key inside bridge and component
             * list
             */
            if (EXIT_SUCCESS != get_key_value(session, node->xpath,
                                              Q_BRIDGE_BRIDGE_ENTRY,
                                              Q_BRIDGE_NAME_STR,
                                              &st, bridge_name)) {
                return SR_ERR_OPERATION_FAILED;
            }

            /* Get vids value. That is a key inside vid-to-fid-allocation list */
            if (EXIT_SUCCESS != get_key_value(session, node->xpath,
                                              Q_BRIDGE_VID2FID_ALLOC_STR,
                                              Q_BRIDGE_VIDS_STR, &st, vids)) {
                return SR_ERR_OPERATION_FAILED;
            }

            /* Check vids notation (ascending) */
            if (does_vids_string_contains_descending_interval(vids)) {
                sr_set_error(session, node->xpath,
                             ERR_BRIDGE_DESCENDING_VIDS_XPATH_STR, vids, node->xpath);
                return SR_ERR_OPERATION_FAILED;
            }

            /* free memory of vid_to_fid_allocation_array every time someone
             * changes vid-to-fid-allocation list
             */
            if (vid_to_fid_allocation_array.vids_array != NULL) {
                for (i = 0; i < vid_to_fid_allocation_array.count; i++) {
                    free(vid_to_fid_allocation_array.vids_array[i]);
                    vid_to_fid_allocation_array.vids_array[i] = NULL;
                }
                free(vid_to_fid_allocation_array.vids_array);
                vid_to_fid_allocation_array.vids_array = NULL;
                vid_to_fid_allocation_array.count = 0;
            }

            if (op == SR_OP_CREATED || op == SR_OP_DELETED) {
                /* container bridge-vlan, list vid-to-fid-allocation */
                fill_xpath(path, BRIDGE_VID2FID_ALLOCATION_LIST_XPATH,
                           bridge_name, bridge_name);
                rc = sr_get_items(session, path, 0, 0, &items, &counter);
                if ((SR_ERR_OK != rc) && (SR_ERR_NOT_FOUND != rc)) {
                    // if item is not found the count is 0
                    sr_set_error(session, node->xpath,
                                 ERR_BRIDGE_FAILED_TO_GET_VID2FID_LIST_STR, node->xpath);
                    return SR_ERR_OPERATION_FAILED;
                } else {
                    /* allocate memory and set count to number of retrieved
                     * vid-to-fid-allocation entries from configuration
                     */
                    vid_to_fid_allocation_array.count = (int)counter;
                    vid_to_fid_allocation_array.vids_array = malloc(sizeof(char*)*(int)counter);
                    if (vid_to_fid_allocation_array.vids_array != NULL) {
                        for (i = 0; i < ((int)counter); i++) {
                            vid_to_fid_allocation_array.vids_array[i] = calloc(MAX_STR_LEN, 1);
                        }
                        i = 0;
                        for (i = 0; i < (int)counter; i++) {
                            if (sr_xpath_node_name_eq(items[i].xpath, Q_BRIDGE_VIDS_STR)) {
                                strncpy(vid_to_fid_allocation_array.vids_array[i],
                                        items[i].data.string_val,
                                        (MAX_STR_LEN-1));
                            }
                        }
                    } else {
                        SRP_LOG_ERR(ERR_BRIDGE_FAILED_TO_ALLOCATE_MEMORY_STR,
                                    Q_BRIDGE_VID2FID_ALLOC_STR, __func__);
                        sr_set_error(session, node->xpath,
                                     ERR_BRIDGE_FAILED_TO_SET_VID2FID_VIDS_STR,
                                     node->xpath);
                        return SR_ERR_OPERATION_FAILED;
                    }
                }
                sr_free_values(items, counter);
            }
        }
    }
    sr_free_val(old_value);
    sr_free_val(new_value);
    sr_free_change_iter(iter);
    node = NULL;
    iter = NULL;

    return SR_ERR_OK;
}

/**
 * @brief Callback to be called by the event of changing content of list
 * fid-to-vid-allocation inside container bridge-vlan.
 *
 * Subscribe to it by sr_module_change_subscribe call.
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
static int bridge_bridge_vlan_fid_to_vid_allocation_cb(
        sr_session_ctx_t *session, const char *module_name,
        const char *xpath, sr_event_t event,
        uint32_t request_id, void *private_data)
{
    (void)module_name;
    (void)request_id;
    (void)private_data;

    int rc = SR_ERR_OK;
    sr_change_oper_t op = {0};
    sr_xpath_ctx_t st = {0};
    sr_change_iter_t *iter = NULL;
    sr_val_t *old_value = NULL;
    sr_val_t *new_value = NULL;
    sr_val_t *node = NULL;
    char fid[MAX_STR_LEN] = "";
    char tmp_fid[MAX_STR_LEN] = "";
    char path[MAX_STR_LEN] = "";
    char bridge_name[MAX_STR_LEN] = "";
    sr_val_t *items = NULL;
    size_t counter = 0;
    int i = 0;

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    if ((SR_EV_DONE == event) || (0 == plugin_init)) {
        SRP_LOG_DBG(DEBUG_MSG_WITH_TWO_PARAM, DBG_APPLYING_CHANGES_MSG, __func__);
        return SR_ERR_OK;
    }

    rc = sr_get_changes_iter(session, xpath, &iter);
    if (SR_ERR_OK != rc) {
        return rc;
    }

    while (SR_ERR_OK == sr_get_change_next(session, iter, &op,
                                           &old_value, &new_value)) {
        node = (op == SR_OP_DELETED) ? old_value : new_value;
        /* if entry is deleted or added inside list */
        if ((true == sr_xpath_node_name_eq(node->xpath, Q_BRIDGE_FID_STR)) &&
            ((op == SR_OP_CREATED) ||
             (op == SR_OP_DELETED))) {
            /* Get all necessary data for delete and add operation */

            /* Get bridge name value. That is a key inside bridge and component
             * list
             */
            if (EXIT_SUCCESS != get_key_value(session, node->xpath,
                                              Q_BRIDGE_BRIDGE_ENTRY,
                                              Q_BRIDGE_NAME_STR,
                                              &st, bridge_name)) {
                return SR_ERR_OPERATION_FAILED;
            }

            /* Get fid value. That is a key inside fid-to-vid-allocation list */
            if (EXIT_SUCCESS != get_key_value(session, node->xpath,
                                              Q_BRIDGE_FID2VID_ALLOC_STR,
                                              Q_BRIDGE_FID_STR, &st, fid)) {
                return SR_ERR_OPERATION_FAILED;
            }

            /* free memory of fid_to_vid_allocation_array every time someone
             * changes fid-to-vid-allocation list
             */
            if (fid_to_vid_allocation_array.fid_array != NULL) {
                for (i = 0; i < fid_to_vid_allocation_array.count; i++) {
                    free(fid_to_vid_allocation_array.fid_array[i]);
                    fid_to_vid_allocation_array.fid_array[i] = NULL;
                }
                free(fid_to_vid_allocation_array.fid_array);
                fid_to_vid_allocation_array.fid_array = NULL;
                fid_to_vid_allocation_array.count = 0;
            }

            if (op == SR_OP_CREATED || op == SR_OP_DELETED) {
                /* container bridge-vlan, list fid-to-vid-allocation */
                fill_xpath(path, BRIDGE_FID2VID_ALLOCATION_LIST_XPATH,
                           bridge_name, bridge_name);
                rc = sr_get_items(session, path, 0, 0, &items, &counter);
                if ((SR_ERR_OK != rc) && (SR_ERR_NOT_FOUND != rc)) {
                    // if item is not found the count is 0
                    sr_set_error(session, node->xpath,
                                 ERR_BRIDGE_FAILED_TO_GET_FID2VID_LIST_STR,
                                 node->xpath);
                    return SR_ERR_OPERATION_FAILED;
                } else {
                    /* allocate memory and set count to number of retrieved
                     * fid-to-vid-allocation entries from configuration
                     */
                    fid_to_vid_allocation_array.count = (int)counter;
                    fid_to_vid_allocation_array.fid_array = malloc(sizeof(char*)*(int)counter);
                    if (fid_to_vid_allocation_array.fid_array != NULL) {
                        for (i = 0; i < ((int)counter); i++) {
                            fid_to_vid_allocation_array.fid_array[i] = calloc(MAX_STR_LEN, 1);
                        }
                        i = 0;
                        for (i = 0; i < (int)counter; i++) {
                            if (sr_xpath_node_name_eq(items[i].xpath, Q_BRIDGE_FID_STR)) {
                                int ret = snprintf(tmp_fid, MAX_STR_LEN, "%u",
                                                   items[i].data.uint32_val);
                                memcpy(fid_to_vid_allocation_array.fid_array[i],
                                       tmp_fid, ret+1);
                            }
                        }
                    } else {
                        SRP_LOG_ERR(ERR_BRIDGE_FAILED_TO_ALLOCATE_MEMORY_STR,
                                    Q_BRIDGE_FID2VID_ALLOC_STR, __func__);
                        sr_set_error(session, node->xpath,
                                     ERR_BRIDGE_FAILED_TO_SET_FID2VID_FID_STR,
                                     node->xpath);
                        return SR_ERR_OPERATION_FAILED;
                    }
                }
                sr_free_values(items, counter);

                /* FID 0 is default entry in fid table */
                if (0 == strncmp("0", fid, MAX_STR_LEN)) {
                    sr_set_error(session, node->xpath,
                                 ERR_BRIDGE_FAILED_TO_SET_FID2VID_FID_STR,
                                 node->xpath);
                    return SR_ERR_OPERATION_FAILED;
                }
            }
        }
    }
    sr_free_val(old_value);
    sr_free_val(new_value);
    sr_free_change_iter(iter);
    node = NULL;
    iter = NULL;

    return SR_ERR_OK;
}

/**
 * @brief Callback to be called by the event of changing content of node with
 * next xpath: "/ieee802-dot1q-bridge:bridges/bridge/address"
 * "/ieee802-dot1q-bridge:bridges/bridge/component/address
 *
 * Subscribe to it by sr_module_change_subscribe call.
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
static int bridge_address_cb(sr_session_ctx_t *session, const char *module_name,
                             const char *xpath, sr_event_t event,
                             uint32_t request_id, void *private_data)
{
    (void)module_name;
    (void)request_id;
    (void)private_data;

    int rc = SR_ERR_OK;
    sr_change_oper_t op = {0};
    sr_xpath_ctx_t st = {0};
    sr_change_iter_t *iter = NULL;
    sr_val_t *node = NULL;
    sr_val_t *old_value = NULL;
    sr_val_t *new_value = NULL;
    char path[MAX_STR_LEN] = "";
    char bridge_name[MAX_STR_LEN] = "";
    /*
     * Guards used to secure that address will be changed only once (client can
     * change both of the addresses in one edit which would result in address
     * swap). In case that both of addresses are changed, precedence is given
     * to bridge address (via higher cb priority).
     *
     * NOTE: Following approach works only if system contains one bridge instance.
     */
    static bool up_addr_changed = false;
    static bool chg_addr_changed = false;

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    if ((SR_EV_DONE == event) || (0 == plugin_init)) {
        SRP_LOG_DBG(DEBUG_MSG_WITH_TWO_PARAM, DBG_APPLYING_CHANGES_MSG, __func__);
        up_addr_changed = false;
        chg_addr_changed = false;
        return SR_ERR_OK;
    }

    if (SR_EV_UPDATE == event && !up_addr_changed) {
        rc = sr_get_changes_iter(session, xpath, &iter);
        if (SR_ERR_OK != rc) {
            return rc;
        }

        while (SR_ERR_OK == sr_get_change_next(session, iter, &op,
                                               &old_value, &new_value)) {

            if (SR_OP_MODIFIED == op) {
                /* get bridge name from xpath */
                if (EXIT_SUCCESS != get_key_value(session, new_value->xpath,
                                                  Q_BRIDGE_BRIDGE_ENTRY,
                                                  Q_BRIDGE_NAME_STR,
                                                  &st, bridge_name)) {
                    sr_set_error(session, xpath, ERR_KEY_VALUE_FAILED_XPATH_STR,
                                 new_value->xpath);
                    rc = SR_ERR_OPERATION_FAILED;
                    goto up_free_chg_iter;
                }

                if (0 == strncmp(xpath, "/ieee802-dot1q-bridge:bridges/bridge/address",
                                 MAX_STR_LEN)) {
                    /* get component address leaf */
                    fill_xpath(path, BRIDGE_COMPONENT_ADDRESS_XPATH, bridge_name, bridge_name);
                    if (SR_ERR_OK != sr_get_item(session, path, 0, &node)) {
                        sr_set_error(session, xpath,
                                     ERR_FAILED_TO_GET_ITEM_XPATH_STR, path);
                        rc = SR_ERR_OPERATION_FAILED;
                        goto up_free_chg_iter;
                    }
                }
                else if (0 == strncmp(xpath, "/ieee802-dot1q-bridge:bridges/bridge/component/address",
                                      MAX_STR_LEN)) {
                    /* get bridge address leaf */
                    fill_xpath(path, BRIDGE_ADDRESS_XPATH, bridge_name);
                    if (SR_ERR_OK != sr_get_item(session, path, 0, &node)) {
                        sr_set_error(session, xpath,
                                     ERR_FAILED_TO_GET_ITEM_XPATH_STR, path);
                        rc = SR_ERR_OPERATION_FAILED;
                        goto up_free_chg_iter;
                    }
                } else {
                    sr_set_error(session, xpath, ERR_UNSUPPORTED_CASE, xpath);
                    rc = SR_ERR_OPERATION_FAILED;
                    goto up_free_chg_iter;
                }

                /* align bridge and component adresses if they differ */
                if (0 != strncmp(new_value->data.string_val, node->data.string_val, MAX_STR_LEN)) {
                    if (SR_ERR_OK != sr_set_item_str(session, path,
                                                     new_value->data.string_val,
                                                     NULL, 0)) {
                        sr_set_error(session, xpath,
                                     ERR_FAILED_SET_OBJ_STR, path);
                        rc = SR_ERR_OPERATION_FAILED;
                        goto up_free_value;
                    }

                    up_addr_changed = true;
                }

                /* free node */
                sr_free_val(node);
                node = NULL;
            }
        }

up_free_value:
        if (NULL != node) {
            sr_free_val(node);
            node = NULL;
        }

up_free_chg_iter:
        if (NULL != iter) {
            sr_free_change_iter(iter);
            iter = NULL;
        }
    }

    if (SR_EV_CHANGE == event && !chg_addr_changed) {
        rc = sr_get_changes_iter(session, xpath, &iter);

        if (SR_ERR_OK != rc) {
            return rc;
        }

        while (SR_ERR_OK == sr_get_change_next(session, iter, &op,
                                               &old_value, &new_value)) {
            if ((op == SR_OP_MODIFIED)) {
                /* get bridge name from xpath */
                if (EXIT_SUCCESS != get_key_value(session, new_value->xpath,
                                                  Q_BRIDGE_BRIDGE_ENTRY,
                                                  Q_BRIDGE_NAME_STR,
                                                  &st, bridge_name)) {
                    sr_set_error(session, xpath,
                                 ERR_KEY_VALUE_FAILED_XPATH_STR, new_value->xpath);
                    rc = SR_ERR_OPERATION_FAILED;
                    goto chg_free_chg_iter;
                }

                if (EXIT_SUCCESS != set_interface_address(bridge_name, new_value->data.string_val)) {
                    sr_set_error(session, new_value->xpath,
                                 ERR_SET_FUNC_FAILED_XPATH_STR, new_value->xpath);
                    rc = SR_ERR_OPERATION_FAILED;
                    goto chg_free_chg_iter;
                }

                chg_addr_changed = true;
            }
            /* NOTE: ADD and DELETE operations are covered with
             * bridge_list_bridge_cb
             */
        }

chg_free_chg_iter:
        if (NULL != iter) {
            sr_free_change_iter(iter);
        }
    }

    return rc;
}

/**
 * @brief Creates a new entry for list 'component' inside list bridge inside
 * ieee802-dot1q-bridge module.
 *
 * This function is for state data from list 'bridge'.
 *
 * @param[in]   parent          Pointer to an existing parent of the requested
 *                              nodes. Is NULL for top-level nodes. Caller is
 *                              supposed to append the requested nodes to this
 *                              data subtree and return either the original
 *                              parent or a top-level node.
 * @param[in]   bridge_name     Bridge name.
 * @param[in]   caps            Contains the bridge capabilities and attributes
 *                              data.
 * @return      Error code (SR_ERR_OK on success).
 */
static int new_bridge_state_capabilities(struct lyd_node **parent,
                                         char *bridge_name,
                                         struct bridge_attr_and_caps caps)
{
    char tmp_value[MAX_STR_LEN] = "";
    char path[MAX_STR_LEN] = "";

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    /* leaf extended-filtering */
    snprintf(tmp_value, MAX_STR_LEN, "%s",
             (caps.ExtendedFilteringServices == true) ? BASE_TRUE : BASE_FALSE);
    fill_xpath(path, BRIDGE_COMPONENT_EXTENDED_FILTERING_XPATH, bridge_name, bridge_name);
    if (NULL == lyd_new_path(*parent, NULL, path, tmp_value, 0, 0)) {
        SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                    Q_BRDIGE_EXT_FILTERING, ERR_NOT_CREATED_ELEMENT_STR);
    }

    /* leaf traffic-classes */
    snprintf(tmp_value, MAX_STR_LEN, "%s",
             (caps.TrafficClasses == true) ? BASE_TRUE : BASE_FALSE);
    fill_xpath(path, BRIDGE_COMPONENT_TRAFFIC_CLASS_XPATH, bridge_name, bridge_name);
    if (NULL == lyd_new_path(*parent, NULL, path, tmp_value, 0, 0)) {
        SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                    Q_BRDIGE_TRAFFIC_CLASSES, ERR_NOT_CREATED_ELEMENT_STR);
    }

    /* leaf static-entry-individual-port */
    snprintf(tmp_value, MAX_STR_LEN, "%s",
             (caps.StaticEntryIndividualPort == true) ? BASE_TRUE : BASE_FALSE);
    fill_xpath(path, BRIDGE_COMPONENT_STAT_ENTRY_IND_PORT_XPATH, bridge_name, bridge_name);
    if (NULL == lyd_new_path(*parent, NULL, path, tmp_value, 0, 0)) {
        SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                    Q_BRDIGE_STATIC_ENTRY_IND_PORTS, ERR_NOT_CREATED_ELEMENT_STR);
    }

    /* leaf ivl-capable */
    snprintf(tmp_value, MAX_STR_LEN, "%s",
             (caps.IVLCapable == true) ? BASE_TRUE : BASE_FALSE);
    fill_xpath(path, BRIDGE_COMPONENT_IVL_CAPABLE_XPATH, bridge_name, bridge_name);
    if (NULL == lyd_new_path(*parent, NULL, path, tmp_value, 0, 0)) {
        SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                    Q_BRDIGE_IVL_CAPABLE, ERR_NOT_CREATED_ELEMENT_STR);
    }

    /* leaf svl-capable */
    snprintf(tmp_value, MAX_STR_LEN, "%s",
             (caps.SVLCapable == true) ? BASE_TRUE : BASE_FALSE);
    fill_xpath(path, BRIDGE_COMPONENT_SVL_CAPABLE_XPATH, bridge_name, bridge_name);
    if (NULL == lyd_new_path(*parent, NULL, path, tmp_value, 0, 0)) {
        SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                    Q_BRDIGE_SVL_CAPABLE, ERR_NOT_CREATED_ELEMENT_STR);
    }

    /* leaf hybrid-capable */
    snprintf(tmp_value, MAX_STR_LEN, "%s",
             (caps.HybridCapable == true) ? BASE_TRUE : BASE_FALSE);
    fill_xpath(path, BRIDGE_COMPONENT_HYBRID_CAPABLE_XPATH, bridge_name, bridge_name);
    if (NULL == lyd_new_path(*parent, NULL, path, tmp_value, 0, 0)) {
        SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                    Q_BRDIGE_HYBRID_CAPABLE, ERR_NOT_CREATED_ELEMENT_STR);
    }

    /* leaf configurable-pvid-tagging */
    snprintf(tmp_value, MAX_STR_LEN, "%s",
             (caps.ConfigurablePvidTagging == true) ? BASE_TRUE : BASE_FALSE);
    fill_xpath(path, BRIDGE_COMPONENT_CONF_PVID_TAGGING_XPATH, bridge_name, bridge_name);
    if (NULL == lyd_new_path(*parent, NULL, path, tmp_value, 0, 0)) {
        SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                    Q_BRDIGE_CONF_PVID_TAGGING, ERR_NOT_CREATED_ELEMENT_STR);
    }

    /* leaf local-vlan-capable */
    snprintf(tmp_value, MAX_STR_LEN, "%s",
             (caps.LocalVlanCapable == true) ? BASE_TRUE : BASE_FALSE);
    fill_xpath(path, BRIDGE_COMPONENT_LOCAL_VLAN_CAPABLE_XPATH, bridge_name, bridge_name);
    if (NULL == lyd_new_path(*parent, NULL, path, tmp_value, 0, 0)) {
        SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                    Q_BRDIGE_LOCAL_VLAN_CAPABLE, ERR_NOT_CREATED_ELEMENT_STR);
    }

    return SR_ERR_OK;
}

/**
 * @brief Creates a new container filtering-database inside list component
 * inside ieee802-dot1q-bridge module.
 *
 * This function is for state data from list 'bridge'.
 *
 * @param[in]   parent          Pointer to an existing parent of the requested
 *                              nodes. Is NULL for top-level nodes. Caller is
 *                              supposed to append the requested nodes to this
 *                              data subtree and return either the original
 *                              parent or a top-level node.
 * @param[in]   bridge_name     Bridge name.
 * @param[in]   caps            Contains the bridge capabilities and attributes
 *                              data.
 * @return      Error code (SR_ERR_OK on success).
 */
static int new_state_filtering_database(struct lyd_node **parent,
                                        char *bridge_name,
                                        struct bridge_attr_and_caps caps)
{
    char tmp_value[MAX_STR_LEN] = "";
    char path[MAX_STR_LEN] = "";
    char result_array[MAX_NUM_OF_INTERFACES][MAX_STR_LEN] = {0};
    struct port_hdl *port = NULL;
    uint32_t tmp_int = 0;

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    /* list filtering-database, leaf size */
    snprintf(tmp_value, MAX_STR_LEN, "%d", caps.FdbSize);
    fill_xpath(path, BRIDGE_COMPONENT_SIZE_XPATH, bridge_name, bridge_name);
    if (NULL == lyd_new_path(*parent, NULL, path, tmp_value, 0, 0)) {
        SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                    Q_BRIDGE_LEAF_SIZE, ERR_NOT_CREATED_ELEMENT_STR);
    }

    /* list filtering-database, leaf static-entries */
    snprintf(tmp_value, MAX_STR_LEN, "%d", caps.FdbStaticEntries);
    fill_xpath(path, BRIDGE_COMPONENT_STATIC_ENTRIES_XPATH,
               bridge_name, bridge_name);
    if (NULL == lyd_new_path(*parent, NULL, path, tmp_value, 0, 0)) {
        SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                    Q_BRIDGE_STATIC_ENTRIES_STR, ERR_NOT_CREATED_ELEMENT_STR);
    }

    /* list filtering-database, leaf dynamic-entries */
    if (EXIT_SUCCESS == get_dynamic_entries(result_array, MAX_NUM_OF_INTERFACES,
                                            MAX_STR_LEN, bridge_name)) {
        fill_xpath(path, BRIDGE_COMPONENT_DYNAMIC_ENTRIES_XPATH,
                   bridge_name, bridge_name);
        if (NULL == lyd_new_path(*parent, NULL, path, result_array[0], 0, 0)) {
            SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                        Q_BRIDGE_DYNAMIC_ENTRIES_STR,
                        ERR_NOT_CREATED_ELEMENT_STR);
        }
    }

    /* open bridge */
    port = tsn_port_open_by_name(bridge_name);
    if (NULL == port) {
        SRP_LOG_ERR(ERROR_MSG_MOD_FUNC_EL_MSG, Q_BRIDGE_MODULE_NAME, __func__,
                    bridge_name, ERR_PORT_NOT_OPEN_STR);
        return SR_ERR_OPERATION_FAILED;
    }

    /* list filtering-database, leaf static-vlan-registration-entries */
    if (0 == tsn_qbridge_get_num_vlans(port, &tmp_int)) {
        snprintf(tmp_value, MAX_STR_LEN, "%u", tmp_int);
        fill_xpath(path, BRIDGE_COMPONENT_STAT_VLAN_REG_ENTRIES_XPATH,
                   bridge_name, bridge_name);
        if (NULL == lyd_new_path(*parent, NULL, path, tmp_value, 0, 0)) {
            SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                        Q_BRIDGE_STATIC_VLAN_REG_ENTRY_STR,
                        ERR_NOT_CREATED_ELEMENT_STR);
        }
    }
    tsn_port_close(port);

    /* list filtering-database, leaf dynamic-vlan-registration-entries */
    if (EXIT_SUCCESS == get_dynamic_vlan_entries(result_array, MAX_NUM_OF_INTERFACES,
                                                 MAX_STR_LEN, bridge_name)) {
        fill_xpath(path, BRIDGE_COMPONENT_DYNAM_VLAN_REG_ENTRIES_XPATH,
                   bridge_name, bridge_name);
        if (NULL == lyd_new_path(*parent, NULL, path, result_array[0], 0, 0)) {
            SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                        Q_BRIDGE_DYNAMIC_VLAN_REG_ENTRY_STR,
                        ERR_NOT_CREATED_ELEMENT_STR);
        }
    }

#if 0
    /* list filtering-database, leaf mac-address-registration-entries */
    if (EXIT_SUCCESS == get_mac_address_entries(result_array, MAX_NUM_OF_INTERFACES,
                                                MAX_STR_LEN, bridge_name)) {
        fill_xpath(path, BRIDGE_COMPONENT_MAC_ADDRESS_REG_ENTRIES_XPATH,
                   bridge_name, bridge_name);
        if (NULL == lyd_new_path(*parent, NULL, path, result_array[0], 0, 0)) {
            SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                        Q_BRIDGE_MAC_ADDR_REG_ENTRY_STR, ERR_NOT_CREATED_ELEMENT_STR);
        }
    }
#endif

    return SR_ERR_OK;
}

/**
 * @brief Creates a new entry for list vlan inside container bridge-vlan inside
 * ieee802-dot1q-bridge module.
 *
 * This function is for state data from list 'bridge'.
 *
 * @param[in]   parent          Pointer to an existing parent of the requested
 *                              nodes. Is NULL for top-level nodes. Caller is
 *                              supposed to append the requested nodes to this
 *                              data subtree and return either the original
 *                              parent or a top-level node.
 * @param[in]   bridge_name     Bridge name.
 * @param[in]   bridge_vlan     Contains the value of the bridge vid.
 * @return      Error code (SR_ERR_OK on success).
 */
static int new_state_vlan_entry(struct lyd_node **parent,
                                char *bridge_name, char *bridge_vlan)
{
    char path[MAX_STR_LEN] = "";
    char message[MAX_STR_LEN] = "";
    char port_names[MAX_NUM_OF_INTERFACES][MAX_STR_LEN] = {0};
    char result_array[MAX_NUM_OF_INTERFACES][MAX_STR_LEN] = {0};
    int i = 0;

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    /* list vlan, leaf name */
    fill_xpath(path, BRIDGE_VLAN_NAME_XPATH, bridge_name, bridge_name, bridge_vlan);
    if (NULL == lyd_new_path(*parent, NULL, path, bridge_name, 0, 0)) {
        SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                    Q_BRIDGE_NAME_STR, ERR_NOT_CREATED_ELEMENT_STR);
    }

    if (EXIT_FAILURE == get_bridge_port_names(port_names, MAX_NUM_OF_INTERFACES,
                                              MAX_STR_LEN)) {
        snprintf(message, MAX_STR_LEN, ERROR_MSG_FUN_AND_MSG, __func__,
                 ERR_BRIDE_PORT_NAMES_FAILED_STR);
        SRP_LOG_INF(ERROR_MSG_FUN_AND_MSG, __func__,
                    message);
        return SR_ERR_OK;
    }

    /* list vlan, leaf-list untagged-ports */
    while ((i < MAX_NUM_OF_INTERFACES) &&
           (0 != strncmp(port_names[i], "", MAX_STR_LEN))) {
        if (EXIT_SUCCESS == get_untagged_port_names(result_array, MAX_NUM_OF_INTERFACES,
                                                    MAX_STR_LEN, port_names[i], bridge_vlan)) {
            fill_xpath(path, BRIDGE_VLAN_UNTAGGED_PORTS_XPATH,
                       bridge_name, bridge_name, bridge_vlan);
            if (NULL == lyd_new_path(*parent, NULL, path, port_names[i], 0, 0)) {
                SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                            Q_BIDGE_UNTAGGED_PORTS_STR, ERR_NOT_CREATED_ELEMENT_STR);
            }
        }
        i++;
    }
    i = 0;
    /* list vlan, leaf-list egress-ports */
    while ((i < MAX_NUM_OF_INTERFACES) &&
           (0 != strncmp(port_names[i], "", MAX_STR_LEN))) {
        if (EXIT_SUCCESS == get_egress_port_names(result_array, MAX_NUM_OF_INTERFACES,
                                                  MAX_STR_LEN, port_names[i], bridge_vlan)) {
            fill_xpath(path, BRIDGE_VLAN_EGRESS_PORTS_XPATH,
                       bridge_name, bridge_name, bridge_vlan);
            if (NULL == lyd_new_path(*parent, NULL, path, port_names[i], 0, 0)) {
                SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                            Q_BIDGE_EGRESS_PORTS_STR, ERR_NOT_CREATED_ELEMENT_STR);
            }
        }
        i++;
    }

    return SR_ERR_OK;
}

/**
 * @brief Returns FID for given VIDs.
 *
 * This function is for state data from list 'bridge'.
 *
 * @param[in]   fid_str_len     FID string length.
 * @param[out]  fid_str         FID value for given VIDs.
 * @param[in]   query_vids_str  A list of VIDs, or non overlapping VIDs ranges
 *                              in ascending order.
 * @param[in]   bridge_name_str Bridge name.
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
static int get_mstp_fid_by_vid_range(size_t fid_str_len, char *fid_str,
                                     char *query_vids_str,
                                     char *bridge_name_str)
{
    char result_arr[BASE_MAX_VID][MAX_STR_LEN] = {0};
    char query_vids_arr[BASE_MAX_VID][BASE_FIVE] = {0};
    char sys_vids_arr[BASE_MAX_VID][BASE_FIVE] = {0};
    char sys_vids_str[MAX_STR_LEN] = "";
    int fid_entry_found = 0;
    int vid = 0;
    int cur_vid = 0;
    int fid_index = 0;
    int i = 0;
    int j = 0;

    /* Extract vids from string */
    assemble_vids_as_array(query_vids_arr, BASE_MAX_VID, BASE_FIVE, query_vids_str);

    /* Get all fid table entries via tool */
    if (EXIT_FAILURE == get_bridge_mstp_vids(result_arr, BASE_MAX_VID,
                                             MAX_STR_LEN, bridge_name_str)) {
        SRP_LOG_ERR(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                    Q_BRIDGE_VID2FID_ALLOC_STR, ERR_BRIDE_MSTP_VIDS_FAILED_STR);
        return EXIT_FAILURE;
    }

    /* Find fid table entry using first vid specified in query */
    vid = atoi(query_vids_arr[0]);
    fid_index = 1; // fid 0 entries are ignored
    while ((0 == fid_entry_found) &&
           (BASE_MAX_VID > fid_index) &&
           (0 != strncmp(result_arr[fid_index], "", MAX_STR_LEN))) {
        // get current fid table entry
        assemble_vids_as_array(sys_vids_arr, BASE_MAX_VID, BASE_FIVE,
                               result_arr[fid_index]);
        i = 0;
        while ((0 == fid_entry_found) &&
               (BASE_MAX_VID > i) &&
               (0 != strncmp(sys_vids_arr[i], "", BASE_FIVE))) {
            cur_vid = atoi(sys_vids_arr[i]);

            if (vid < cur_vid) {
                // vids are sorted in ascending order
                break;
            }

            if (cur_vid == vid) {
                fid_entry_found = 1;
                memcpy(sys_vids_str, result_arr[fid_index],
                       strlen(result_arr[fid_index]));
            }
            i++;
        }
        fid_index++;
    }

    /* if fid entry is found, check if given range is its subrange */
    if (1 == fid_entry_found) {
        fid_index--;
        i--;

        while ((BASE_MAX_VID > i) &&
               (0 != strncmp(sys_vids_arr[i], "", BASE_FIVE)) &&
               (0 != strncmp(query_vids_arr[j], "", BASE_FIVE))) {
            if (0 == strncmp(sys_vids_arr[i], query_vids_arr[j], BASE_FIVE)) {
                j++;
            }
            i++;
        }

        if (0 == strncmp(query_vids_arr[j], "", BASE_FIVE)) {
            if (EXIT_FAILURE == get_mstp_fid_by_vids(result_arr, BASE_MAX_VID, MAX_STR_LEN,
                                                     bridge_name_str, sys_vids_str)) {
                SRP_LOG_ERR(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                            Q_BRIDGE_VID2FID_ALLOC_STR,
                            ERR_BRIDE_MSTP_VIDS_FAILED_STR);
            }
            size_t entry_len = strlen(result_arr[0]);
            if (entry_len >= fid_str_len) {
                return EXIT_FAILURE;
            }
            memcpy(fid_str, result_arr[0], entry_len+1);
        } else {
            return EXIT_FAILURE;
        }
    } else {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

/**
 * @brief Creates a new entry for list vid-to-fid-allocation inside container
 * bridge-vlan inside ieee802-dot1q-bridge module.
 *
 * This function is for state data from list 'bridge'.
 *
 * @param[in]   parent          Pointer to an existing parent of the requested
 *                              nodes. Is NULL for top-level nodes. Caller is
 *                              supposed to append the requested nodes to this
 *                              data subtree and return either the original
 *                              parent or a top-level node.
 * @param[in]   bridge_name     Bridge name.
 * @param[in]   bridge_vlan     Contains the value of the bridge vid.
 * @return      Error code (SR_ERR_OK on success).
 */
static int new_vid2fid_alloc_entry(struct lyd_node **parent,
                                   char *bridge_name, char *vids_str)
{
    char path[MAX_STR_LEN] = "";
    char fid_str[BASE_FIVE] = "";

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    /* list vid-to-fid-allocation, leaf fid */
    if (EXIT_SUCCESS == get_mstp_fid_by_vid_range(BASE_FIVE, fid_str, vids_str,
                                                  bridge_name)) {
        fill_xpath(path, BRIDGE_VID2FID_ALLOCATION_FID_XPATH, bridge_name,
                   bridge_name, vids_str);

        if (NULL == lyd_new_path(*parent, NULL, path, fid_str, 0, 0)) {
            SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                        Q_BRIDGE_FID_STR, ERR_NOT_CREATED_ELEMENT_STR);
        }
    }

    return SR_ERR_OK;
}

/**
 * @brief Creates a new entry for list fid-to-vid-allocation inside container
 * bridge-vlan inside ieee802-dot1q-bridge module.
 *
 * This function is for state data from list 'bridge'.
 *
 * @param[in]   parent          Pointer to an existing parent of the requested
 *                              nodes. Is NULL for top-level nodes. Caller is
 *                              supposed to append the requested nodes to this
 *                              data subtree and return either the original
 *                              parent or a top-level node.
 * @param[in]   bridge_name     Bridge name.
 * @param[in]   bridge_vlan     Contains the value of the bridge vid.
 * @return      Error code (SR_ERR_OK on success).
 */
static int new_fid2vid_alloc_entry(struct lyd_node **parent,
                                   char *bridge_name, char *bridge_vlan)
{
    char path[MAX_STR_LEN] = "";
    char result_array[BASE_ONE][MAX_STR_LEN] = {0};
    char vids_array[BASE_MAX_VID][BASE_TEN] = {0};
    int i = 0;

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    /* list vid-to-fid-allocation, leaf fid */
    if (EXIT_SUCCESS == get_mstp_vids_by_fid(result_array, BASE_ONE, MAX_STR_LEN,
                                             bridge_name, bridge_vlan)) {
        assemble_vids_as_array(vids_array, BASE_MAX_VID, BASE_TEN, result_array[0]);
        while ((i < BASE_MAX_VID) && (0 != strncmp(vids_array[i], "", BASE_TEN))) {
            fill_xpath(path, BRIDGE_FID2VID_ALLOCATION_VID_XPATH,
                       bridge_name, bridge_name, bridge_vlan);
            if (NULL == lyd_new_path(*parent, NULL, path, vids_array[i], 0, 0)) {
                SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                            Q_BRIDGE_VID_STR, ERR_NOT_CREATED_ELEMENT_STR);
            }
            i++;
        }
    }

    return SR_ERR_OK;
}

/**
 * @brief Creates a new container bridge-vlan inside list component inside
 * ieee802-dot1q-bridge module.
 *
 * This function is for state data from list 'bridge'.
 *
 * @param[in]   parent          Pointer to an existing parent of the requested
 *                              nodes. Is NULL for top-level nodes. Caller is
 *                              supposed to append the requested nodes to this
 *                              data subtree and return either the original
 *                              parent or a top-level node.
 * @param[in]   bridge_name     Bridge name.
 * @param[in]   caps            Contains the bridge capabilities and attributes
 *                              data.
 * @return      Error code (SR_ERR_OK on success).
 */
static int new_state_bridge_vlan(struct lyd_node **parent,
                                 char *bridge_name,
                                 struct bridge_attr_and_caps caps)
{
    char path[MAX_STR_LEN] = "";
    struct port_hdl *bridge = NULL;
    char tmp_value[MAX_STR_LEN] = "";
    uint32_t tmp_int = 0;
    int i = 0;

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    /* tsn_port_open_by_name */
    bridge = tsn_port_open_by_name(bridge_name);
    if (NULL == bridge) {
        SRP_LOG_ERR(ERROR_MSG_MOD_FUNC_EL_MSG, Q_BRIDGE_NAME_STR, __func__,
                    bridge_name, ERR_PORT_NOT_OPEN_STR);
        return SR_ERR_OPERATION_FAILED;
    }

    /* container bridge-vlan, leaf max-vids */
    if (0 == tsn_qbridge_get_max_supported_vlans(bridge, &tmp_int)) {
        snprintf(tmp_value, MAX_STR_LEN, "%u", tmp_int);
        fill_xpath(path, BRIDGE_MAX_VIDS_XPATH, bridge_name, bridge_name);
        if (NULL == lyd_new_path(*parent, NULL, path, tmp_value, 0, 0)) {
            SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                        Q_BRIDGE_MAX_VIDS_STR, ERR_NOT_CREATED_ELEMENT_STR);
        }
    }

    /* container bridge-vlan, leaf max-vid-value */
    if (0 == tsn_qbridge_get_max_vlan_id(bridge, &tmp_int)) {
        snprintf(tmp_value, MAX_STR_LEN, "%u", tmp_int);
        fill_xpath(path, BRIDGE_MAX_VID_VAL_XPATH, bridge_name, bridge_name);
        if (NULL == lyd_new_path(*parent, NULL, path, tmp_value, 0, 0)) {
            SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                        BRIDGE_MAX_VID_VAL_XPATH, ERR_NOT_CREATED_ELEMENT_STR);
        }
    }

    /* container bridge-vlan, leaf override-default-pvid */
    snprintf(tmp_value, MAX_STR_LEN, "%s",
             (caps.ConfigurablePvidTagging == true) ? BASE_TRUE : BASE_FALSE);
    fill_xpath(path, BRIDGE_OVERRIDE_DEFAULT_PVID_XPATH, bridge_name, bridge_name);
    if (NULL == lyd_new_path(*parent, NULL, path, tmp_value, 0, 0)) {
        SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                    "override-default-pvid", ERR_NOT_CREATED_ELEMENT_STR);
    }

    /* container bridge-vlan, leaf max-msti */
    if (0 == tsn_mstp_get_max_msti(bridge, &tmp_int)) {
        snprintf(tmp_value, MAX_STR_LEN, "%u", tmp_int);
        fill_xpath(path, BRIDGE_MAX_MSTI_XPATH, bridge_name, bridge_name);
        if (NULL == lyd_new_path(*parent, NULL, path, tmp_value, 0, 0)) {
            SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                        "max-msti", ERR_NOT_CREATED_ELEMENT_STR);
        }
    }
    tsn_port_close(bridge);

    /* container bridge-vlan, list vlan */
    for (i = 0; i < vlan_array.count; i++) {
        if (SR_ERR_OK != new_state_vlan_entry(parent, bridge_name,
                                              vlan_array.vid_array[i])) {
            SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                        Q_BRIDGE_COMP_VLAN, ERR_NOT_CREATED_ELEMENT_STR);
        }
    }

    /* container bridge-vlan, list vid-to-fid-allocation */
    for (i = 0; i < vid_to_fid_allocation_array.count; i++) {
        if (SR_ERR_OK != new_vid2fid_alloc_entry(parent, bridge_name,
                                                 vid_to_fid_allocation_array.vids_array[i])) {
            SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                        Q_BRIDGE_VID2FID_ALLOC_STR,
                        ERR_NOT_CREATED_ELEMENT_STR);
        }
    }

    /* container bridge-vlan, list fid-to-vid-allocation */

    for (i = 0; i < fid_to_vid_allocation_array.count; i++) {
        /* fid with value '0' will be skipped only if fid-to-vid-allocation
         * entry with fid '0' is not added inside configuration
         */
        if (SR_ERR_OK != new_fid2vid_alloc_entry(parent, bridge_name,
                                                 fid_to_vid_allocation_array.fid_array[i])) {
            SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                        Q_BRIDGE_FID2VID_ALLOC_STR,
                        ERR_NOT_CREATED_ELEMENT_STR);
        }
    }

    return SR_ERR_OK;
}

/**
 * @brief Creates a new list entry for list 'component' inside list bridge
 * inside ieee802-dot1q-bridge module.
 *
 * This function is for state data from list 'bridge'.
 *
 * @param[in]   parent          Pointer to an existing parent of the requested
 *                              nodes. Is NULL for top-level nodes. Caller is
 *                              supposed to append the requested nodes to this
 *                              data subtree and return either the original
 *                              parent or a top-level node.
 * @param[in]   bridge_name     Bridge name.
 * @param[in]   caps            Contains the bridge capabilities and attributes
 *                              data.
 * @return      Error code (SR_ERR_OK on success).
 */
static int new_bridge_state_component(struct lyd_node **parent,
                                      char *bridge_name,
                                      struct bridge_attr_and_caps caps)
{
    char tmp_value[MAX_STR_LEN] = "";
    char path[MAX_STR_LEN] = "";
    char result_array[MAX_NUM_OF_INTERFACES][MAX_STR_LEN] = {0};
    int i =0;

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    /* list component, leaf ports */
    snprintf(tmp_value, MAX_STR_LEN, "%d", caps.ComponentNumPorts);
    fill_xpath(path, BRIDGE_COMPONENT_PORTS_XPATH, bridge_name, bridge_name);
    if (NULL == lyd_new_path(*parent, NULL, path, tmp_value, 0, 0)) {
        SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                    Q_BRIDGE_PORTS_STR, ERR_NOT_CREATED_ELEMENT_STR);
    }

    /* list component, leaf-list bridge-port */
    if (EXIT_SUCCESS == get_bridge_port_names(result_array, MAX_NUM_OF_INTERFACES,
                                              MAX_STR_LEN)) {
        while ((i < MAX_NUM_OF_INTERFACES) &&
               (0 != strncmp(result_array[i], "", MAX_STR_LEN))) {
            fill_xpath(path, BRIDGE_COMPONENT_BRIDGE_PORT_XPATH,
                       bridge_name, bridge_name);
            if (NULL == lyd_new_path(*parent, NULL, path, result_array[i], 0, 0)) {
                SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                            Q_BRIDGE_BRIDGE_PORTS, ERR_NOT_CREATED_ELEMENT_STR);
            }
            i++;
        }
    }

    /* list component, container capabilities */
    if (SR_ERR_OK != new_bridge_state_capabilities(parent, bridge_name, caps)) {
        SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                    Q_BRIDGE_CAPABILITIES_STR, ERR_NOT_CREATED_ELEMENT_STR);
    }

    /* container filtering-database,
     * exist only if when 'bridge-type != 'two-port-mac-relay-bridge'
     */
    /* check if bridge-type is not 'two-port-mac-relay-bridge' */

    if (EXIT_SUCCESS == get_bridge_type(result_array, MAX_NUM_OF_INTERFACES,
                                        MAX_STR_LEN, bridge_name)) {
        if ((0 != strncmp(Q_BRIDGE_TWO_PORT_MAC_RELAY, result_array[0], MAX_STR_LEN))) {
            /* container filtering-database */
            if (SR_ERR_OK != new_state_filtering_database(parent, bridge_name, caps)) {
                SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                            Q_BRIDGE_FILTERING_DATABASE,
                            ERR_NOT_CREATED_ELEMENT_STR);
            }

            /* container bridge-vlan */
            if (SR_ERR_OK != new_state_bridge_vlan(parent, bridge_name, caps)) {
                SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                            Q_BRIDGE_VLAN_STR, ERR_NOT_CREATED_ELEMENT_STR);
            }
        }
    }

    /* sfsg state data */
    if (SR_ERR_OK != get_sfsg_state_data(parent, bridge_name)) {
        SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                    Q_BRIDGE_FILTERING_DATABASE, ERR_NOT_CREATED_ELEMENT_STR);
    }

    return SR_ERR_OK;
}

/**
 * @brief Creates a new list entry for list 'bridge' inside container bridges
 * inside ieee802-dot1q-bridge module.
 *
 * This function is for state data from list 'bridge'.
 *
 * @param[in]   parent          Pointer to an existing parent of the requested
 *                              nodes. Is NULL for top-level nodes. Caller is
 *                              supposed to append the requested nodes to this
 *                              data subtree and return either the original
 *                              parent or a top-level node.
 * @param[in]   bridge_name     Bridge name.
 * @return      Error code (SR_ERR_OK on success).
 */
static int new_bridge_state_node(struct lyd_node **parent, char *bridge_name)
{
    char tmp_value[MAX_STR_LEN] = "";
    char path[MAX_STR_LEN] = "";
    char result_array[MAX_NUM_OF_INTERFACES][MAX_STR_LEN] = {0};
    struct port_hdl *port = NULL;
    struct bridge_attr_and_caps caps = {0};

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    /* open bridge */
    port = tsn_port_open_by_name(bridge_name);
    if (NULL == port) {
        SRP_LOG_ERR(ERROR_MSG_MOD_FUNC_EL_MSG, Q_BRIDGE_MODULE_NAME, __func__,
                    bridge_name, ERR_PORT_NOT_OPEN_STR);
        return SR_ERR_OPERATION_FAILED;
    }
    if (0 != tsn_bridge_attr(port, &caps)) {
        SRP_LOG_ERR(ERROR_MSG_MOD_FUNC_EL_MSG, Q_BRIDGE_MODULE_NAME, __func__,
                    bridge_name,
                    "Failed to get bridge capabilities and attributes.");
        tsn_port_close(port);
        return SR_ERR_OPERATION_FAILED;
    }
    tsn_port_close(port);

    /* list bridge, leaf ports */
    snprintf(tmp_value, MAX_STR_LEN, "%d", caps.BridgeNumPorts);
    fill_xpath(path, BRIDGE_PORTS_XPATH, bridge_name);
    if (NULL == lyd_new_path(*parent, NULL, path, tmp_value, 0, 0)) {
        SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                    Q_BRIDGE_PORTS_STR, ERR_NOT_CREATED_ELEMENT_STR);
    }

    /* list bridge, leaf up-time */
    if (EXIT_SUCCESS == get_bridge_uptime(result_array, MAX_NUM_OF_INTERFACES,
                                          MAX_STR_LEN)) {
        fill_xpath(path, BRIDGE_UP_TIME_XPATH, bridge_name);
        if (NULL == lyd_new_path(*parent, NULL, path, result_array[0], 0, 0)) {
            SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                        Q_BRIDGE_UPTIME_STR, ERR_NOT_CREATED_ELEMENT_STR);
        }
    }

    /* list bridge, leaf components */
    fill_xpath(path, BRIDGE_COMPONENTS_XPATH, bridge_name);
    if (NULL == lyd_new_path(*parent, NULL, path, BASE_ONE_STR, 0, 0)) {
        SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                    Q_BRIDGE_COMPONENTS, ERR_NOT_CREATED_ELEMENT_STR);
    }

    /* list component */
    if (SR_ERR_OK != new_bridge_state_component(parent, bridge_name, caps)) {
        SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                    Q_BRIDGE_COMPONENT, ERR_NOT_CREATED_ELEMENT_STR);
    }

    return SR_ERR_OK;
}

/**
 * @brief Callback to be called when operational data of module
 * ieee802-dot1q-bridge is requested.
 *
 * Subscribe to it by sr_oper_get_items_subscribe call.
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
static int bridge_state_cb(sr_session_ctx_t *session, const char *module_name,
                           const char *path, const char *request_xpath,
                           uint32_t request_id, struct lyd_node **parent,
                           void *private_data)
{
    (void)session;
    (void)module_name;
    (void)request_xpath;
    (void)request_id;
    (void)private_data;
    (void)path;

    char bridge_names[MAX_NUM_OF_INTERFACES][MAX_STR_LEN] = {0};
    int i = 0;

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    if (EXIT_FAILURE == get_bridge_names(bridge_names, MAX_NUM_OF_INTERFACES,
                                         MAX_STR_LEN)) {
        return SR_ERR_OPERATION_FAILED;
    }

    /* go through the list and process all bridges */
    while ((i < MAX_NUM_OF_INTERFACES) &&
           (0 != strncmp(bridge_names[i], "", MAX_STR_LEN))) {
        if (EXIT_FAILURE == new_bridge_state_node(parent, bridge_names[i])) {
            SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                        Q_BRIDGE_BRIDGE_ENTRY, ERR_NOT_CREATED_ELEMENT_STR);
            return SR_ERR_OPERATION_FAILED;
        }
        i++;
    }
    return SR_ERR_OK;
}

/**
 * @brief Callback to be called by the event of changing content of leaf-list
 * mstid inside container bridge-mst.
 *
 * Subscribe to it by sr_module_change_subscribe call.
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
static int bridge_bridge_mst_mstid_cb(sr_session_ctx_t *session,
                                      const char *module_name,
                                      const char *xpath,
                                      sr_event_t event,
                                      uint32_t request_id,
                                      void *private_data)
{
    (void)module_name;
    (void)request_id;
    (void)private_data;

    int rc = SR_ERR_OK;
    sr_change_oper_t op = {0};
    sr_xpath_ctx_t st = {0};
    sr_change_iter_t *iter = NULL;
    sr_val_t *old_value = NULL;
    sr_val_t *new_value = NULL;
    sr_val_t *node = NULL;
    sr_val_t *component_id = NULL;
    char path[MAX_STR_LEN] = "";
    char id[MAX_STR_LEN] = "";
    char mstid[MAX_STR_LEN] = "";
    char bridge_name[MAX_STR_LEN] = "";
    char result_array[MAX_NUM_OF_INTERFACES][MAX_STR_LEN] = {0};

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    if ((SR_EV_DONE == event) || (0 == plugin_init)) {
        SRP_LOG_DBG(DEBUG_MSG_WITH_TWO_PARAM, DBG_APPLYING_CHANGES_MSG, __func__);
        return SR_ERR_OK;
    }

    rc = sr_get_changes_iter(session, xpath, &iter);
    if (SR_ERR_OK != rc) {
        return rc;
    }

    while (SR_ERR_OK == sr_get_change_next(session, iter, &op,
                                           &old_value, &new_value)) {
        node = (op == SR_OP_DELETED) ? old_value : new_value;

        if (SR_OP_MODIFIED == op) {
            sr_set_error(session, node->xpath,
                         ERR_MOD_OP_NOT_SUPPORTED_XPATH_STR, node->xpath);
            return SR_ERR_OPERATION_FAILED;
        }
        if ((SR_OP_CREATED == op) || (SR_OP_DELETED == op)) {
            /* Get bridge name value. That is a key inside bridge and component
             * list.
             */
            if (EXIT_SUCCESS != get_key_value(session, node->xpath,
                                              Q_BRIDGE_BRIDGE_ENTRY,
                                              Q_BRIDGE_NAME_STR,
                                              &st, bridge_name)) {
                return SR_ERR_OPERATION_FAILED;
            }

            /* Get leaf id from list component. */
            fill_xpath(path, BRIDGE_COMPONENT_ID_XPATH, bridge_name, bridge_name);
            if (SR_ERR_OK != sr_get_item(session, path, 0, &component_id)) {
                sr_set_error(session, path, ERR_FAILED_GET_OBJ_STR, path);
                return SR_ERR_OPERATION_FAILED;
            }

            snprintf(id, MAX_STR_LEN, "%d", component_id->data.uint32_val);
            snprintf(mstid, MAX_STR_LEN, "%d", node->data.uint32_val);

            if (SR_OP_CREATED == op) {
                if (EXIT_SUCCESS != add_mstp_entry(id, mstid)) {
                    sr_set_error(session, node->xpath,
                                 ERR_SET_FUNC_FAILED_XPATH_STR, node->xpath);
                    return SR_ERR_OPERATION_FAILED;
                }
            }

            if (SR_OP_DELETED == op) {
                if (EXIT_SUCCESS == get_bridge_fid_by_mstid(result_array, MAX_NUM_OF_INTERFACES,
                                                            MAX_STR_LEN, bridge_name, mstid)) {
                    if (EXIT_SUCCESS != mstpctl_delete_fids(bridge_name, result_array[0])) {
                        sr_set_error(session, node->xpath,
                                     ERR_SET_FUNC_FAILED_XPATH_STR, node->xpath);
                        return SR_ERR_OPERATION_FAILED;
                    }
                }

                if (EXIT_SUCCESS != remove_mstp_entry(id, mstid)) {
                    sr_set_error(session, node->xpath,
                                 ERR_SET_FUNC_FAILED_XPATH_STR, node->xpath);
                    return SR_ERR_OPERATION_FAILED;
                }
            }
            /* Reload Running must happen here! */
        }
    }

    sr_free_val(new_value);
    sr_free_val(old_value);
    sr_free_val(component_id);
    sr_free_change_iter(iter);
    node = NULL;

    return SR_ERR_OK;
}

/**
 * @brief Callback to be called by the event of changing content of list
 * fid-to-mstid inside container bridge-mst.
 *
 * Subscribe to it by sr_module_change_subscribe call.
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
static int bridge_bridge_mst_fid2mstid_cb(sr_session_ctx_t *session,
                                          const char *module_name,
                                          const char *xpath,
                                          sr_event_t event,
                                          uint32_t request_id,
                                          void *private_data)
{
    (void)module_name;
    (void)request_id;
    (void)private_data;

    int rc = SR_ERR_OK;
    sr_change_oper_t op = {0};
    sr_change_iter_t *iter = NULL;
    sr_val_t *old_value = NULL;
    sr_val_t *new_value = NULL;
    sr_val_t *node = NULL;

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    if ((SR_EV_DONE == event) || (0 == plugin_init)) {
        SRP_LOG_DBG(DEBUG_MSG_WITH_TWO_PARAM, DBG_APPLYING_CHANGES_MSG, __func__);
        return SR_ERR_OK;
    }
    if (SR_EV_ABORT  == event) {
        SRP_LOG_ERR(DEBUG_MSG_WITH_TWO_PARAM,
                    ERR_APPLYING_CHANGES_FAILED_MSG, __func__);
        return SR_ERR_OPERATION_FAILED;
    }

    rc = sr_get_changes_iter(session, xpath, &iter);
    if (SR_ERR_OK != rc) {
        return rc;
    }

    while (SR_ERR_OK == sr_get_change_next(session, iter, &op,
                                           &old_value, &new_value)) {
        node = (op == SR_OP_DELETED) ? old_value : new_value;

        if ((true == sr_xpath_node_name_eq(node->xpath, Q_BRIDGE_FID_STR)) &&
            ((SR_OP_MODIFIED == op) ||
             (SR_OP_CREATED == op) ||
             (SR_OP_DELETED == op))) {
            sr_set_error(session, node->xpath,
                         ERR_MOD_OP_NOT_SUPPORTED_XPATH_STR, node->xpath);
            return SR_ERR_OPERATION_FAILED;
        }
    }

    sr_free_val(new_value);
    sr_free_val(old_value);
    sr_free_change_iter(iter);
    node = NULL;

    return SR_ERR_OK;
}

/**
 * @brief Callback to be called by the event of changing content of list
 * fid-to-mstid-allocation inside container bridge-mst.
 *
 * Subscribe to it by sr_module_change_subscribe call.
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
static int bridge_bridge_mst_fid2mstid_allocation_cb(sr_session_ctx_t *session,
                                                     const char *module_name,
                                                     const char *xpath,
                                                     sr_event_t event,
                                                     uint32_t request_id,
                                                     void *private_data)
{
    (void)module_name;
    (void)request_id;
    (void)private_data;

    int rc = SR_ERR_OK;
    sr_change_oper_t op = {0};
    sr_xpath_ctx_t st = {0};
    sr_change_iter_t *iter = NULL;
    sr_val_t *old_value = NULL;
    sr_val_t *new_value = NULL;
    sr_val_t *node = NULL;
    sr_val_t *component_id = NULL;
    sr_val_t *mstid = NULL;
    char bridge_name[MAX_STR_LEN] = "";
    char mstid_string[MAX_STR_LEN] = "";
    char id[MAX_STR_LEN] = "";
    char fids[MAX_STR_LEN] = "";
    char path[MAX_STR_LEN] = "";
    char bridge_msti[MAX_NUM_OF_INTERFACES][MAX_STR_LEN] = {0};
    int i = 0;
    int msti_exist = 0;

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    if ((SR_EV_DONE == event) || (0 == plugin_init)) {
        SRP_LOG_DBG(DEBUG_MSG_WITH_TWO_PARAM, DBG_APPLYING_CHANGES_MSG, __func__);
        return SR_ERR_OK;
    }

    rc = sr_get_changes_iter(session, xpath, &iter);
    if (SR_ERR_OK != rc) {
        return rc;
    }

    while (SR_ERR_OK == sr_get_change_next(session, iter, &op,
                                           &old_value, &new_value)) {
        node = (op == SR_OP_DELETED) ? old_value : new_value;

        /* SR_OP_MODIFIED operation is covered inside callback for leaf mstid. */
        if ((true == sr_xpath_node_name_eq(node->xpath, Q_BRIDGE_FIDS_STR)) &&
            ((SR_OP_CREATED == op) || (SR_OP_DELETED == op))) {
            /* Get bridge name value. That is a key inside bridge and component
             * list.
             */
            if (EXIT_SUCCESS != get_key_value(session, node->xpath,
                                              Q_BRIDGE_BRIDGE_ENTRY,
                                              Q_BRIDGE_NAME_STR,
                                              &st, bridge_name)) {
                return SR_ERR_OPERATION_FAILED;
            }

            /* Get leaf id from list component. */
            fill_xpath(path, BRIDGE_COMPONENT_ID_XPATH, bridge_name, bridge_name);
            if (SR_ERR_OK != sr_get_item(session, path, 0, &component_id)) {
                sr_set_error(session, path, ERR_FAILED_GET_OBJ_STR, path);
                return SR_ERR_OPERATION_FAILED;
            }

            snprintf(id, MAX_STR_LEN, "%d", component_id->data.uint32_val);

            /* Get fids value. That is a key inside fid-to-mstid-allocation
             * list.
             */
            if (EXIT_SUCCESS != get_key_value(session, node->xpath,
                                              Q_BRIDGE_FID2MSTID_ALLOC_STR,
                                              Q_BRIDGE_FIDS_STR, &st, fids)) {
                return SR_ERR_OPERATION_FAILED;
            }

            /* check does this msti exist */
            if (EXIT_SUCCESS != get_bridge_mstpi(bridge_msti, MAX_NUM_OF_INTERFACES,
                                                 MAX_STR_LEN, bridge_name)) {
                sr_set_error(session, node->xpath,
                             ERR_GET_FUNC_FAILED_XPATH_STR, node->xpath);
                return SR_ERR_OPERATION_FAILED;
            }

            if (SR_OP_CREATED == op) {
                /* Get leaf mstid from list fid-to-mstid-allocation. */
                /* We are getting leaf mstid here, because we can not get it if
                 * there is a delete operation. He is already deleted from
                 * configuration. Also there is no need to check does msti
                 * exist on system if there is a delete operation.
                 */

                fill_xpath(path, BRIDGE_FID2MSTID_ALLOCATION_MSTID_XPATH,
                           bridge_name, bridge_name, fids);
                if (SR_ERR_OK != sr_get_item(session, path, 0, &mstid)) {
                    sr_set_error(session, path, ERR_FAILED_GET_OBJ_STR, path);
                    return SR_ERR_OPERATION_FAILED;
                }

                /* Convert data from unit_32 to string. */
                snprintf(mstid_string, MAX_STR_LEN, "%d", mstid->data.uint32_val);

                while ((i < MAX_NUM_OF_INTERFACES) &&
                       (0 != strncmp(bridge_msti[i], "", MAX_STR_LEN))) {
                    if (0 == strncmp(bridge_msti[i], mstid_string, MAX_STR_LEN)) {
                        msti_exist = 1;
                    }
                    i++;
                }

                if (0 == msti_exist) {
                    if (EXIT_SUCCESS != add_mstp_entry(id, mstid_string)) {
                        sr_set_error(session, node->xpath,
                                     ERR_SET_FUNC_FAILED_XPATH_STR, node->xpath);
                        return SR_ERR_OPERATION_FAILED;
                    }
                }

                if (EXIT_SUCCESS != mstpctl_setfid2mstid(bridge_name, mstid_string, fids)) {
                    sr_set_error(session, mstid->xpath,
                                 ERR_SET_FUNC_FAILED_XPATH_STR, mstid->xpath);
                    return SR_ERR_OPERATION_FAILED;
                }

                /* Free mstid here. Do not free it at the end, because we do
                 * not use it in DELETE operation.
                 */
                sr_free_val(mstid);
            }
            if (SR_OP_DELETED == op) {
                if (EXIT_SUCCESS != mstpctl_setfid2mstid(bridge_name, BASE_ZERO_STR, fids)) {
                    sr_set_error(session, node->xpath,
                                 ERR_SET_FUNC_FAILED_XPATH_STR, node->xpath);
                    return SR_ERR_OPERATION_FAILED;
                }
            }
        }
    }

    sr_free_val(new_value);
    sr_free_val(old_value);
    sr_free_val(component_id);
    sr_free_change_iter(iter);
    node = NULL;

    return SR_ERR_OK;
}

/**
 * @brief Callback to be called by the event of changing content of leaf mstid
 * inside list fid-to-mstid-allocation inside container bridge-mst.
 *
 * Subscribe to it by sr_module_change_subscribe call.
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
static int bridge_bridge_mst_fid2mstid_allocation_mstid_cb(
        sr_session_ctx_t *session, const char *module_name, const char *xpath,
        sr_event_t event, uint32_t request_id, void *private_data)
{
    (void)module_name;
    (void)request_id;
    (void)private_data;

    int rc = SR_ERR_OK;
    sr_change_oper_t op = {0};
    sr_change_iter_t *iter = NULL;
    sr_val_t *old_value = NULL;
    sr_val_t *new_value = NULL;
    sr_val_t *node = NULL;

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    if ((SR_EV_DONE == event) || (0 == plugin_init)) {
        SRP_LOG_DBG(DEBUG_MSG_WITH_TWO_PARAM, DBG_APPLYING_CHANGES_MSG, __func__);
        return SR_ERR_OK;
    }

    rc = sr_get_changes_iter(session, xpath, &iter);
    if (SR_ERR_OK != rc) {
        return rc;
    }

    while (SR_ERR_OK == sr_get_change_next(session, iter, &op,
                                           &old_value, &new_value)) {
        node = (op == SR_OP_DELETED) ? old_value : new_value;

        /* SR_OP_CREATED SR_OP_DELETED operations are covered inside callback
         * for list fid-to-mstid-allocation.
         */
        if ((true == sr_xpath_node_name_eq(node->xpath, Q_BRIDGE_MSTID_STR)) &&
            (SR_OP_MODIFIED == op)) {
            sr_set_error(session, node->xpath,
                         ERR_MOD_OP_NOT_SUPPORTED_XPATH_STR, node->xpath);
            return SR_ERR_OPERATION_FAILED;
        }
    }

    sr_free_val(new_value);
    sr_free_val(old_value);
    sr_free_change_iter(iter);
    node = NULL;

    return SR_ERR_OK;
}

/**
 * @brief Callback to be called by the event of changing content of leaf
 * control-element inside static-filtering-entries container inside
 * ieee802-dot1q-bridge yang module.
 *
 * Subscribe to it by sr_module_change_subscribe call.
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
int bridge_filtering_entry_control_element_cb(sr_session_ctx_t *session,
                                              const char *module_name,
                                              const char *xpath,
                                              sr_event_t event,
                                              uint32_t request_id,
                                              void *private_data)
{
    (void)module_name;
    (void)event;
    (void)request_id;
    (void)private_data;

    int rc = SR_ERR_OK;
    sr_change_oper_t op = {0};
    sr_change_iter_t *iter = NULL;
    sr_val_t *old_value = NULL;
    sr_val_t *new_value = NULL;
    sr_val_t *node = NULL;

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    if ((SR_EV_DONE == event) || (0 == plugin_init)) {
        SRP_LOG_DBG(DEBUG_MSG_WITH_TWO_PARAM, DBG_APPLYING_CHANGES_MSG, __func__);
        return SR_ERR_OK;
    }

    rc = sr_get_changes_iter(session, xpath, &iter);
    if (SR_ERR_OK != rc) {
        return rc;
    }

    while (SR_ERR_OK == sr_get_change_next(session, iter, &op,
                                           &old_value, &new_value)) {
        node = (op == SR_OP_DELETED) ? old_value : new_value;

        if ((op == SR_OP_MODIFIED) || (op == SR_OP_CREATED)) {
            /* only forward value od control-element is supported */
            if (0 != strncmp(node->data.enum_val, Q_BRIDGE_FORWARD, MAX_STR_LEN)) {
                SRP_LOG_ERR(ERROR_MSG_FUN_XML_EL_AND_MSG, __func__,
                            sr_xpath_node_name(xpath),
                            ERR_BRIDE_PORT_CONTROL_ELEMENT_STR);
                sr_set_error(session, node->xpath,
                             ERR_BRIDE_PORT_CONTROL_ELEMENT_STR);
                return SR_ERR_OPERATION_FAILED;
            }
        }
        /* delete operation is suppoerted */
    }

    sr_free_val(old_value);
    sr_free_val(new_value);
    sr_free_change_iter(iter);
    node = NULL;

    return SR_ERR_OK;
}

/**
 * @brief Sysrepo plugin initialization callback.
 *
 * @param[in]   session         Sysrepo session that can be used for any API
 *                              calls needed for plugin initialization (mainly
 *                              for reading of startup configuration and
 *                              subscribing for notifications).
 * @param[in]   private_data    Private context as passed in sr_plugin_init_cb.
 * @return      Error code (SR_ERR_OK on success). If an error is returned,
 *              plugin will be considered as uninitialized.
 */
int sr_plugin_init_cb(sr_session_ctx_t *session, void **private_data)
{
    (void)private_data;

    int rc = SR_ERR_OK;
    char *path = NULL;

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    rc = config_change_helper_start();
    if (rc) {
        SRP_LOG_ERR(ERR_MODULE_INIT_FAILED_STR, Q_BRIDGE_MODULE_NAME,
                    sr_strerror(rc));
        return rc;
    }

    struct config_change_list *reset_list = config_change_list_new();
    if (!reset_list) {
        SRP_LOG_ERR(ERR_MODULE_INIT_FAILED_STR, Q_BRIDGE_MODULE_NAME,
                    sr_strerror(SR_ERR_NOMEM));
        config_change_helper_stop();
        return SR_ERR_NOMEM;
    }

    /* Set plugin_init to "1" so callbacks are triggered when startup configuration is copied to running. */
    if (0 != sr_path_check_startup_done_file(BRIDGE_MODULE_NAME, &path)) {
        /* Startup datastore has already been copied to running datastore.
         * Apply config-change reset to both datastores
         * before subscriptions apply configuration to system.
         */
        rc = psfp_fix_datastore(session, SR_DS_STARTUP);
        if (SR_ERR_OK == rc)
            rc = psfp_fix_datastore(session, SR_DS_RUNNING);
        if (SR_ERR_OK != rc) {
            free(reset_list);
            config_change_helper_stop();
            return rc;
        }

        plugin_init = 1;
    }

    do {
        /* subscribe for /ieee802-dot1q-bridge module changes */
        rc = sr_module_change_subscribe(session, Q_BRIDGE_MODULE_NAME, NULL,
                                        module_change_cb, NULL, 0, SR_SUBSCR_ENABLED, &subscription);
        if (SR_ERR_OK != rc) break;

        /* subscribe as state data provider for the /ieee802-dot1q-bridge:bridges/bridge */
        rc = sr_oper_get_items_subscribe(session, Q_BRIDGE_MODULE_NAME, "/ieee802-dot1q-bridge:bridges/bridge",
                                         bridge_state_cb, NULL, SR_SUBSCR_DEFAULT, &subscription);
        if (rc != SR_ERR_OK) break;

        /* subscribe for /ieee802-dot1q-bridge:bridges/bridge changes */
        rc = sr_module_change_subscribe(session, Q_BRIDGE_MODULE_NAME, "/ieee802-dot1q-bridge:bridges/bridge",
                                        bridge_list_mod_not_supported_cb, NULL, 7, SR_SUBSCR_ENABLED, &subscription);
        if (rc != SR_ERR_OK) break;

        /* subscribe for /ieee802-dot1q-bridge:bridges/bridge/address changes */
        rc = sr_module_change_subscribe(session, Q_BRIDGE_MODULE_NAME, "/ieee802-dot1q-bridge:bridges/bridge/address",
                                        bridge_address_cb, NULL, 2, SR_SUBSCR_UPDATE, &subscription);
        if (rc != SR_ERR_OK) break;

        /* subscribe for /ieee802-dot1q-bridge:bridges/bridge/bridge-type changes */
        rc = sr_module_change_subscribe(session, Q_BRIDGE_MODULE_NAME, "/ieee802-dot1q-bridge:bridges/bridge/bridge-type",
                                        mod_not_supported_cb, NULL, 1, SR_SUBSCR_ENABLED, &subscription);
        if (rc != SR_ERR_OK) break;

        /* subscribe for /ieee802-dot1q-bridge:bridges/bridge/component changes */
        rc = sr_module_change_subscribe(session, Q_BRIDGE_MODULE_NAME, "/ieee802-dot1q-bridge:bridges/bridge/component",
                                        bridge_list_mod_not_supported_cb, NULL, 7, SR_SUBSCR_ENABLED, &subscription);
        if (rc != SR_ERR_OK) break;

        /* subscribe for /ieee802-dot1q-bridge:bridges/bridge/component/id changes */
        rc = sr_module_change_subscribe(session, Q_BRIDGE_MODULE_NAME, "/ieee802-dot1q-bridge:bridges/bridge/component/id",
                                        mod_not_supported_cb, NULL, 1, SR_SUBSCR_ENABLED, &subscription);
        if (rc != SR_ERR_OK) break;

        /* subscribe for /ieee802-dot1q-bridge:bridges/bridge/component/type changes */
        rc = sr_module_change_subscribe(session, Q_BRIDGE_MODULE_NAME, "/ieee802-dot1q-bridge:bridges/bridge/component/type",
                                        mod_not_supported_cb, NULL, 1, SR_SUBSCR_ENABLED, &subscription);
        if (rc != SR_ERR_OK) break;

        /* subscribe for /ieee802-dot1q-bridge:bridges/bridge/component/address changes */
        rc = sr_module_change_subscribe(session, Q_BRIDGE_MODULE_NAME, "/ieee802-dot1q-bridge:bridges/bridge/component/address",
                                        bridge_address_cb, NULL, 1, SR_SUBSCR_UPDATE, &subscription);
        if (rc != SR_ERR_OK) break;

        /* subscribe for /ieee802-dot1q-bridge:bridges/bridge/component/traffic-class-enabled changes */
        rc = sr_module_change_subscribe(session, Q_BRIDGE_MODULE_NAME, "/ieee802-dot1q-bridge:bridges/bridge/component/traffic-class-enabled",
                                        mod_not_supported_cb, NULL, 1, SR_SUBSCR_ENABLED, &subscription);
        if (rc != SR_ERR_OK) break;

        /* subscribe for /ieee802-dot1q-bridge:bridges/bridge/component/filtering-database/vlan-registration-entry changes */
        rc = sr_module_change_subscribe(session, Q_BRIDGE_MODULE_NAME, "/ieee802-dot1q-bridge:bridges/bridge/component/filtering-database/vlan-registration-entry/vids",
                                        bridge_vlan_reg_entry_cb, NULL, 7, SR_SUBSCR_ENABLED, &subscription);
        if (rc != SR_ERR_OK) break;

        /* subscribe for /ieee802-dot1q-bridge:bridges/bridge/component/filtering-database/vlan-registration-entry/entry-type changes */
        rc = sr_module_change_subscribe(session, Q_BRIDGE_MODULE_NAME, "/ieee802-dot1q-bridge:bridges/bridge/component/filtering-database/vlan-registration-entry/entry-type",
                                        bridge_vlan_entry_type_cb, NULL, 6, SR_SUBSCR_ENABLED, &subscription);
        if (rc != SR_ERR_OK) break;

        /* subscribe for /ieee802-dot1q-bridge:bridges/bridge/component/filtering-database/vlan-registration-entry/port-map changes */
        rc = sr_module_change_subscribe(session, Q_BRIDGE_MODULE_NAME, "/ieee802-dot1q-bridge:bridges/bridge/component/filtering-database/vlan-registration-entry/port-map/port-ref",
                                        bridge_vlan_entry_port_map_cb, NULL, 5, SR_SUBSCR_ENABLED, &subscription);
        if (rc != SR_ERR_OK) break;

        /* subscribe for /ieee802-dot1q-bridge:bridges/bridge/component/filtering-database/filtering-entry/entry-type changes */
        rc = sr_module_change_subscribe(session, Q_BRIDGE_MODULE_NAME, "/ieee802-dot1q-bridge:bridges/bridge/component/filtering-database/filtering-entry/entry-type",
                                        mod_not_supported_cb, NULL, 4, SR_SUBSCR_ENABLED, &subscription);
        if (rc != SR_ERR_OK) break;

        /* subscribe for /ieee802-dot1q-bridge:bridges/bridge/component/filtering-database/filtering-entry/port-map/static-filtering-entries/control-element changes */
        rc = sr_module_change_subscribe(session, Q_BRIDGE_MODULE_NAME, "/ieee802-dot1q-bridge:bridges/bridge/component/filtering-database/filtering-entry/port-map/static-filtering-entries/control-element",
                                        bridge_filtering_entry_control_element_cb, NULL, 3, SR_SUBSCR_ENABLED, &subscription);
        if (rc != SR_ERR_OK) break;

        /* subscribe for /ieee802-dot1q-bridge:bridges/bridge/component/filtering-database/filtering-entry/port-map changes */
        rc = sr_module_change_subscribe(session, Q_BRIDGE_MODULE_NAME, "/ieee802-dot1q-bridge:bridges/bridge/component/filtering-database/filtering-entry/port-map/port-ref",
                                        bridge_filtering_entry_port_map_abort_cb, NULL, 4, SR_SUBSCR_ENABLED, &subscription);
        if (rc != SR_ERR_OK) break;

        /* subscribe for /ieee802-dot1q-bridge:bridges/bridge/component/filtering-database/filtering-entry/port-map changes */
        rc = sr_module_change_subscribe(session, Q_BRIDGE_MODULE_NAME, "/ieee802-dot1q-bridge:bridges/bridge/component/filtering-database/filtering-entry/port-map/port-ref",
                                        bridge_filtering_entry_port_map_cb, NULL, 3, SR_SUBSCR_ENABLED, &subscription);
        if (rc != SR_ERR_OK) break;

        /* subscribe for /ieee802-dot1q-bridge:bridges/bridge/component/bridge-vlan/vid-to-fid changes */
        rc = sr_module_change_subscribe(session, Q_BRIDGE_MODULE_NAME, "/ieee802-dot1q-bridge:bridges/bridge/component/bridge-vlan/vid-to-fid/vid",
                                        bridge_bridge_vlan_vid_to_fid_cb, NULL, 3, SR_SUBSCR_ENABLED, &subscription);
        if (rc != SR_ERR_OK) break;

        /* subscribe for /ieee802-dot1q-bridge:bridges/bridge/component/bridge-vlan/vid-to-fid/fid changes */
        rc = sr_module_change_subscribe(session, Q_BRIDGE_MODULE_NAME, "/ieee802-dot1q-bridge:bridges/bridge/component/bridge-vlan/vid-to-fid/fid",
                                        bridge_bridge_vlan_vid_to_fid_fid_cb, NULL, 2, SR_SUBSCR_ENABLED, &subscription);
        if (rc != SR_ERR_OK) break;

        /* subscribe for /ieee802-dot1q-bridge:bridges/bridge/component/bridge-vlan/vlan/vid changes */
        rc = sr_module_change_subscribe(session, Q_BRIDGE_MODULE_NAME, "/ieee802-dot1q-bridge:bridges/bridge/component/bridge-vlan/vlan/vid",
                                        bridge_bridge_vlan_vlan_cb, NULL, 2, SR_SUBSCR_ENABLED, &subscription);
        if (rc != SR_ERR_OK) break;

        /* subscribe for /ieee802-dot1q-bridge:bridges/bridge/component/bridge-vlan/vlan/name changes */
        rc = sr_module_change_subscribe(session, Q_BRIDGE_MODULE_NAME, "/ieee802-dot1q-bridge:bridges/bridge/component/bridge-vlan/vlan/name",
                                        add_rem_mod_not_supported_cb, NULL, 2, SR_SUBSCR_ENABLED, &subscription);
        if (rc != SR_ERR_OK) break;

        /* subscribe for /ieee802-dot1q-bridge:bridges/bridge/component/bridge-vlan/vid-to-fid-allocation/vids changes */
        rc = sr_module_change_subscribe(session, Q_BRIDGE_MODULE_NAME, "/ieee802-dot1q-bridge:bridges/bridge/component/bridge-vlan/vid-to-fid-allocation/vids",
                                        bridge_bridge_vlan_vid_to_fid_allocation_cb, NULL, 2, SR_SUBSCR_ENABLED, &subscription);
        if (rc != SR_ERR_OK) break;

        /* subscribe for /ieee802-dot1q-bridge:bridges/bridge/component/bridge-vlan/fid-to-vid-allocation/vids changes */
        rc = sr_module_change_subscribe(session, Q_BRIDGE_MODULE_NAME, "/ieee802-dot1q-bridge:bridges/bridge/component/bridge-vlan/fid-to-vid-allocation/fid",
                                        bridge_bridge_vlan_fid_to_vid_allocation_cb, NULL, 2, SR_SUBSCR_ENABLED, &subscription);
        if (rc != SR_ERR_OK) break;

        /* subscribe for /ieee802-dot1q-bridge:bridges/bridge/component/bridge-mst/mstid changes */
        rc = sr_module_change_subscribe(session, Q_BRIDGE_MODULE_NAME, "/ieee802-dot1q-bridge:bridges/bridge/component/bridge-mst/mstid",
                                        bridge_bridge_mst_mstid_cb, NULL, 2, SR_SUBSCR_ENABLED, &subscription);
        if (rc != SR_ERR_OK) break;

        /* subscribe for /ieee802-dot1q-bridge:bridges/bridge/component/bridge-mst/fid-to-mstid changes */
        rc = sr_module_change_subscribe(session, Q_BRIDGE_MODULE_NAME, "/ieee802-dot1q-bridge:bridges/bridge/component/bridge-mst/fid-to-mstid/fid",
                                        bridge_bridge_mst_fid2mstid_cb, NULL, 2, SR_SUBSCR_ENABLED, &subscription);
        if (rc != SR_ERR_OK) break;

        /* subscribe for /ieee802-dot1q-bridge:bridges/bridge/component/bridge-mst/fid-to-mstid-allocation changes */
        rc = sr_module_change_subscribe(session, Q_BRIDGE_MODULE_NAME, "/ieee802-dot1q-bridge:bridges/bridge/component/bridge-mst/fid-to-mstid-allocation/fids",
                                        bridge_bridge_mst_fid2mstid_allocation_cb, NULL, 3, SR_SUBSCR_ENABLED, &subscription);
        if (rc != SR_ERR_OK) break;

        /* subscribe for /ieee802-dot1q-bridge:bridges/bridge/component/bridge-mst/fid-to-mstid-allocation/mstid changes */
        rc = sr_module_change_subscribe(session, Q_BRIDGE_MODULE_NAME, "/ieee802-dot1q-bridge:bridges/bridge/component/bridge-mst/fid-to-mstid-allocation/mstid",
                                        bridge_bridge_mst_fid2mstid_allocation_mstid_cb, NULL, 2, SR_SUBSCR_ENABLED, &subscription);
        if (rc != SR_ERR_OK) break;

        /* subscribe for /ieee802-dot1q-bridge:bridges/bridge/component/ieee802-dot1q-stream-filters-gates:stream-gates/ieee802-dot1q-psfp:supported-list-max changes */
        rc = sr_module_change_subscribe(session, Q_BRIDGE_MODULE_NAME, "/ieee802-dot1q-bridge:bridges/bridge/component/ieee802-dot1q-stream-filters-gates:stream-gates/ieee802-dot1q-psfp:supported-list-max",
                                        mod_not_supported_cb, NULL, 3, SR_SUBSCR_ENABLED, &subscription);
        if (rc != SR_ERR_OK) break;

        /* subscribe for /ieee802-dot1q-bridge:bridges/bridge/component/ieee802-dot1q-stream-filters-gates:stream-gates/ieee802-dot1q-psfp:supported-cycle-max/numerator changes */
        rc = sr_module_change_subscribe(session, Q_BRIDGE_MODULE_NAME, "/ieee802-dot1q-bridge:bridges/bridge/component/ieee802-dot1q-stream-filters-gates:stream-gates/ieee802-dot1q-psfp:supported-cycle-max/numerator",
                                        mod_not_supported_cb, NULL, 3, SR_SUBSCR_ENABLED, &subscription);
        if (rc != SR_ERR_OK) break;

        /* subscribe for /ieee802-dot1q-bridge:bridges/bridge/component/ieee802-dot1q-stream-filters-gates:stream-gates/ieee802-dot1q-psfp:supported-cycle-max/denominator changes */
        rc = sr_module_change_subscribe(session, Q_BRIDGE_MODULE_NAME, "/ieee802-dot1q-bridge:bridges/bridge/component/ieee802-dot1q-stream-filters-gates:stream-gates/ieee802-dot1q-psfp:supported-cycle-max/denominator",
                                        mod_not_supported_cb, NULL, 3, SR_SUBSCR_ENABLED, &subscription);
        if (rc != SR_ERR_OK) break;

        /* subscribe for /ieee802-dot1q-bridge:bridges/bridge/component/ieee802-dot1q-stream-filters-gates:stream-gates/ieee802-dot1q-psfp:supported-interval-max changes */
        rc = sr_module_change_subscribe(session, Q_BRIDGE_MODULE_NAME, "/ieee802-dot1q-bridge:bridges/bridge/component/ieee802-dot1q-stream-filters-gates:stream-gates/ieee802-dot1q-psfp:supported-interval-max",
                                        mod_not_supported_cb, NULL, 3, SR_SUBSCR_ENABLED, &subscription);
        if (rc != SR_ERR_OK) break;

        /* subscribe for /ieee802-dot1q-bridge:bridges/bridge/component/ieee802-dot1q-stream-filters-gates:stream-gates/stream-gate-instance-table changes */
        rc = sr_module_change_subscribe(session, Q_BRIDGE_MODULE_NAME, "/ieee802-dot1q-bridge:bridges/bridge/component/ieee802-dot1q-stream-filters-gates:stream-gates/stream-gate-instance-table/admin-gate-states",
                                        sfg_stream_gates_mod_cb, NULL, 6, SR_SUBSCR_ENABLED, &subscription);
        if (rc != SR_ERR_OK) break;

        /* subscribe for /ieee802-dot1q-bridge:bridges/bridge/component/ieee802-dot1q-stream-filters-gates:stream-gates/stream-gate-instance-table changes */
        rc = sr_module_change_subscribe(session, Q_BRIDGE_MODULE_NAME, "/ieee802-dot1q-bridge:bridges/bridge/component/ieee802-dot1q-stream-filters-gates:stream-gates/stream-gate-instance-table/ieee802-dot1q-psfp:gate-closed-due-to-invalid-rx-enable",
                                        sfg_stream_gates_mod_cb, NULL, 5, SR_SUBSCR_ENABLED, &subscription);
        if (rc != SR_ERR_OK) break;

        /* subscribe for /ieee802-dot1q-bridge:bridges/bridge/component/ieee802-dot1q-stream-filters-gates:stream-gates/stream-gate-instance-table changes */
        rc = sr_module_change_subscribe(session, Q_BRIDGE_MODULE_NAME, "/ieee802-dot1q-bridge:bridges/bridge/component/ieee802-dot1q-stream-filters-gates:stream-gates/stream-gate-instance-table/ieee802-dot1q-psfp:gate-closed-due-to-invalid-rx",
                                        sfg_stream_gates_mod_cb, NULL, 4, SR_SUBSCR_ENABLED, &subscription);
        if (rc != SR_ERR_OK) break;

        /* subscribe for /ieee802-dot1q-bridge:bridges/bridge/component/ieee802-dot1q-stream-filters-gates:stream-gates/stream-gate-instance-table changes */
        rc = sr_module_change_subscribe(session, Q_BRIDGE_MODULE_NAME, "/ieee802-dot1q-bridge:bridges/bridge/component/ieee802-dot1q-stream-filters-gates:stream-gates/stream-gate-instance-table/admin-ipv",
                                        sfg_stream_gates_mod_cb, NULL, 3, SR_SUBSCR_ENABLED, &subscription);
        if (rc != SR_ERR_OK) break;

        /* subscribe for /ieee802-dot1q-bridge:bridges/bridge/component/ieee802-dot1q-stream-filters-gates:stream-filters/stream-filter-instance-table changes */
        rc = sr_module_change_subscribe(session, Q_BRIDGE_MODULE_NAME, "/ieee802-dot1q-bridge:bridges/bridge/component/ieee802-dot1q-stream-filters-gates:stream-filters/stream-filter-instance-table/stream-handle",
                                        sfg_stream_filters_mod_cb, NULL, 9, SR_SUBSCR_ENABLED, &subscription);
        if (rc != SR_ERR_OK) break;

        /* subscribe for /ieee802-dot1q-bridge:bridges/bridge/component/ieee802-dot1q-stream-filters-gates:stream-filters/stream-filter-instance-table changes */
        rc = sr_module_change_subscribe(session, Q_BRIDGE_MODULE_NAME, "/ieee802-dot1q-bridge:bridges/bridge/component/ieee802-dot1q-stream-filters-gates:stream-filters/stream-filter-instance-table/priority-spec",
                                        sfg_stream_filters_mod_cb, NULL, 8, SR_SUBSCR_ENABLED, &subscription);
        if (rc != SR_ERR_OK) break;

        /* subscribe for /ieee802-dot1q-bridge:bridges/bridge/component/ieee802-dot1q-stream-filters-gates:stream-filters/stream-filter-instance-table changes */
        rc = sr_module_change_subscribe(session, Q_BRIDGE_MODULE_NAME, "/ieee802-dot1q-bridge:bridges/bridge/component/ieee802-dot1q-stream-filters-gates:stream-filters/stream-filter-instance-table/max-sdu-size",
                                        sfg_stream_filters_mod_cb, NULL, 7, SR_SUBSCR_ENABLED, &subscription);
        if (rc != SR_ERR_OK) break;

        /* subscribe for /ieee802-dot1q-bridge:bridges/bridge/component/ieee802-dot1q-stream-filters-gates:stream-filters/stream-filter-instance-table changes */
        rc = sr_module_change_subscribe(session, Q_BRIDGE_MODULE_NAME, "/ieee802-dot1q-bridge:bridges/bridge/component/ieee802-dot1q-stream-filters-gates:stream-filters/stream-filter-instance-table/stream-blocked-due-to-oversize-frame-enabled",
                                        sfg_stream_filters_mod_cb, NULL, 6, SR_SUBSCR_ENABLED, &subscription);
        if (rc != SR_ERR_OK) break;

        /* subscribe for /ieee802-dot1q-bridge:bridges/bridge/component/ieee802-dot1q-stream-filters-gates:stream-filters/stream-filter-instance-table changes */
        rc = sr_module_change_subscribe(session, Q_BRIDGE_MODULE_NAME, "/ieee802-dot1q-bridge:bridges/bridge/component/ieee802-dot1q-stream-filters-gates:stream-filters/stream-filter-instance-table/stream-blocked-due-to-oversize-frame",
                                        sfg_stream_filters_mod_cb, NULL, 5, SR_SUBSCR_ENABLED, &subscription);
        if (rc != SR_ERR_OK) break;

        /* subscribe for /ieee802-dot1q-bridge:bridges/bridge/component/ieee802-dot1q-stream-filters-gates:stream-filters/stream-filter-instance-table changes */
        rc = sr_module_change_subscribe(session, Q_BRIDGE_MODULE_NAME, "/ieee802-dot1q-bridge:bridges/bridge/component/ieee802-dot1q-stream-filters-gates:stream-filters/stream-filter-instance-table/stream-gate-ref",
                                        sfg_stream_filters_mod_cb, NULL, 4, SR_SUBSCR_ENABLED, &subscription);
        if (rc != SR_ERR_OK) break;

        /* subscribe for /ieee802-dot1q-bridge:bridges/bridge/component/ieee802-dot1q-stream-filters-gates:stream-filters/stream-filter-instance-table changes */
        rc = sr_module_change_subscribe(session, Q_BRIDGE_MODULE_NAME, "/ieee802-dot1q-bridge:bridges/bridge/component/ieee802-dot1q-stream-filters-gates:stream-filters/stream-filter-instance-table/stream-filter-instance-id",
                                        sfg_stream_filters_cb, NULL, 3, SR_SUBSCR_ENABLED, &subscription);
        if (rc != SR_ERR_OK) break;

        /* subscribe for /ieee802-dot1q-bridge:bridges/bridge/component/ieee802-dot1q-stream-filters-gates:stream-gates/stream-gate-instance-table/ieee802-dot1q-psfp:admin-control-list changes */
        rc = sr_module_change_subscribe(session, Q_BRIDGE_MODULE_NAME, "/ieee802-dot1q-bridge:bridges/bridge/component/ieee802-dot1q-stream-filters-gates:stream-gates/stream-gate-instance-table/ieee802-dot1q-psfp:admin-control-list/gate-control-entry/operation-name",
                                        psfp_acl_mod_cb, NULL, 9, SR_SUBSCR_ENABLED, &subscription);
        if (SR_ERR_OK != rc) break;

        /* subscribe for /ieee802-dot1q-bridge:bridges/bridge/component/ieee802-dot1q-stream-filters-gates:stream-gates/stream-gate-instance-table/ieee802-dot1q-psfp:admin-control-list changes */
        rc = sr_module_change_subscribe(session, Q_BRIDGE_MODULE_NAME, "/ieee802-dot1q-bridge:bridges/bridge/component/ieee802-dot1q-stream-filters-gates:stream-gates/stream-gate-instance-table/ieee802-dot1q-psfp:admin-control-list/gate-control-entry/time-interval-value",
                                        psfp_acl_mod_cb, NULL, 8, SR_SUBSCR_ENABLED, &subscription);
        if (SR_ERR_OK != rc) break;

        /* subscribe for /ieee802-dot1q-bridge:bridges/bridge/component/ieee802-dot1q-stream-filters-gates:stream-gates/stream-gate-instance-table/ieee802-dot1q-psfp:admin-control-list changes */
        rc = sr_module_change_subscribe(session, Q_BRIDGE_MODULE_NAME, "/ieee802-dot1q-bridge:bridges/bridge/component/ieee802-dot1q-stream-filters-gates:stream-gates/stream-gate-instance-table/ieee802-dot1q-psfp:admin-control-list/gate-control-entry/gate-state-value",
                                        psfp_acl_mod_cb, NULL, 7, SR_SUBSCR_ENABLED, &subscription);
        if (SR_ERR_OK != rc) break;

        /* subscribe for /ieee802-dot1q-bridge:bridges/bridge/component/ieee802-dot1q-stream-filters-gates:stream-gates/stream-gate-instance-table/ieee802-dot1q-psfp:admin-control-list changes */
        rc = sr_module_change_subscribe(session, Q_BRIDGE_MODULE_NAME, "/ieee802-dot1q-bridge:bridges/bridge/component/ieee802-dot1q-stream-filters-gates:stream-gates/stream-gate-instance-table/ieee802-dot1q-psfp:admin-control-list/gate-control-entry/ipv-spec",
                                        psfp_acl_mod_cb, NULL, 6, SR_SUBSCR_ENABLED, &subscription);
        if (SR_ERR_OK != rc) break;

        /* subscribe for /ieee802-dot1q-bridge:bridges/bridge/component/ieee802-dot1q-stream-filters-gates:stream-gates/stream-gate-instance-table/ieee802-dot1q-psfp:admin-control-list changes */
        rc = sr_module_change_subscribe(session, Q_BRIDGE_MODULE_NAME, "/ieee802-dot1q-bridge:bridges/bridge/component/ieee802-dot1q-stream-filters-gates:stream-gates/stream-gate-instance-table/ieee802-dot1q-psfp:admin-control-list/gate-control-entry/index",
                                        psfp_acl_cb, NULL, 5, SR_SUBSCR_ENABLED, &subscription);
        if (SR_ERR_OK != rc) break;

        /* subscribe for /ieee802-dot1q-bridge:bridges/bridge/component/ieee802-dot1q-stream-filters-gates:stream-gates/stream-gate-instance-table/ieee802-dot1q-psfp:admin-control-list changes */
        rc = sr_module_change_subscribe(session, Q_BRIDGE_MODULE_NAME, "/ieee802-dot1q-bridge:bridges/bridge/component/ieee802-dot1q-stream-filters-gates:stream-gates/stream-gate-instance-table/ieee802-dot1q-psfp:admin-cycle-time/nominator",
                                        psfp_stream_gates_cb, NULL, 4, SR_SUBSCR_ENABLED, &subscription);
        if (SR_ERR_OK != rc) break;

        /* subscribe for /ieee802-dot1q-bridge:bridges/bridge/component/ieee802-dot1q-stream-filters-gates:stream-gates/stream-gate-instance-table/ieee802-dot1q-psfp:admin-control-list changes */
        rc = sr_module_change_subscribe(session, Q_BRIDGE_MODULE_NAME, "/ieee802-dot1q-bridge:bridges/bridge/component/ieee802-dot1q-stream-filters-gates:stream-gates/stream-gate-instance-table/ieee802-dot1q-psfp:admin-cycle-time/denominator",
                                        psfp_stream_gates_cb, NULL, 4, SR_SUBSCR_ENABLED, &subscription);
        if (SR_ERR_OK != rc) break;

        /* subscribe for /ieee802-dot1q-bridge:bridges/bridge/component/ieee802-dot1q-stream-filters-gates:stream-gates/stream-gate-instance-table/ieee802-dot1q-psfp:admin-control-list changes */
        rc = sr_module_change_subscribe(session, Q_BRIDGE_MODULE_NAME, "/ieee802-dot1q-bridge:bridges/bridge/component/ieee802-dot1q-stream-filters-gates:stream-gates/stream-gate-instance-table/ieee802-dot1q-psfp:admin-cycle-extension",
                                        psfp_stream_gates_cb, NULL, 4, SR_SUBSCR_ENABLED, &subscription);
        if (SR_ERR_OK != rc) break;

        /* subscribe for /ieee802-dot1q-bridge:bridges/bridge/component/ieee802-dot1q-stream-filters-gates:stream-gates/stream-gate-instance-table/ieee802-dot1q-psfp:admin-control-list changes */
        rc = sr_module_change_subscribe(session, Q_BRIDGE_MODULE_NAME, "/ieee802-dot1q-bridge:bridges/bridge/component/ieee802-dot1q-stream-filters-gates:stream-gates/stream-gate-instance-table/ieee802-dot1q-psfp:admin-base-time/seconds",
                                        psfp_stream_gates_cb, NULL, 4, SR_SUBSCR_ENABLED, &subscription);
        if (SR_ERR_OK != rc) break;

        /* subscribe for /ieee802-dot1q-bridge:bridges/bridge/component/ieee802-dot1q-stream-filters-gates:stream-gates/stream-gate-instance-table/ieee802-dot1q-psfp:admin-control-list changes */
        rc = sr_module_change_subscribe(session, Q_BRIDGE_MODULE_NAME, "/ieee802-dot1q-bridge:bridges/bridge/component/ieee802-dot1q-stream-filters-gates:stream-gates/stream-gate-instance-table/ieee802-dot1q-psfp:admin-base-time/nanoseconds",
                                        psfp_stream_gates_cb, NULL, 4, SR_SUBSCR_ENABLED, &subscription);
        if (SR_ERR_OK != rc) break;

        /* subscribe for /ieee802-dot1q-bridge:bridges/bridge/component/ieee802-dot1q-stream-filters-gates:stream-gates/stream-gate-instance-table/ieee802-dot1q-psfp:admin-control-list changes */
        rc = sr_module_change_subscribe(session, Q_BRIDGE_MODULE_NAME,
                                        "/ieee802-dot1q-bridge:bridges/bridge/component/ieee802-dot1q-stream-filters-gates:stream-gates/stream-gate-instance-table/ieee802-dot1q-psfp:config-change" "|"
                                        "/ieee802-dot1q-bridge:bridges/bridge/component/ieee802-dot1q-stream-filters-gates:stream-gates/stream-gate-instance-table/gate-enable",
                                        psfp_config_change_cb, reset_list, 3, SR_SUBSCR_ENABLED, &subscription);
        if (SR_ERR_OK != rc) break;

    } while (0);

    if (SR_ERR_OK != rc) {
        SRP_LOG_ERR(ERR_MODULE_INIT_FAILED_STR, Q_BRIDGE_MODULE_NAME,
                    sr_strerror(rc));
        sr_unsubscribe(subscription);
        config_change_helper_stop();
        free(reset_list);
        return rc;
    }

    if (EXIT_FAILURE == bridge_fill_datastore(session)) {
        SRP_LOG_ERR(ERROR_MSG_MOD_FUNC_MSG, Q_BRIDGE_MODULE_NAME, __func__,
                    ERR_MSG_EMPTY_STARTUP_DS_STR);
        config_change_helper_stop();
        free(reset_list);
        return SR_ERR_OPERATION_FAILED;
    }

    *private_data = reset_list;

    plugin_init = 2;

    SRP_LOG_INF(INF_MODULE_INIT_SUCCESS_STR, Q_BRIDGE_MODULE_NAME);

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

    sr_unsubscribe(subscription);
    config_change_helper_stop();
    free(private_data);
    SRP_LOG_INF(INF_MODULE_CLEANUP_STR, Q_BRIDGE_MODULE_NAME);
}
