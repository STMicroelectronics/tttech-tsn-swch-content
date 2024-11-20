/*
 * TTTech ieee8021-mstp-yang-module
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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
/* common includes */
#include "common_defines.h"
#include "common.h"
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
#include "libbase/base_mstp_linux_tools.h"
/* module specific includes */
#include "mstp_defines.h"

/* structure holding all the subscriptions */
sr_subscription_ctx_t *subscription;

/* Variable to detect is callback triggered for the first time The startup
 * datastore iy copied to the running before this plugin is initialized. So, at
 * first time plugin started we need just to copy data to startup and running.
 * It is not necessary to trigger setter function. "plugin_init" will be '0' if
 * plugin is not initialized. Set "plugin_init" to '1' after is initialized.
 * The will be after fill startup datastore function.
 */
int plugin_init = 0;

/**
 * @brief Callback to be called by the event of changing content of node with next xpath:
 * "/ieee8021-mstp:ieee8021-mstp/ieee8021MstpCistTable/ieee8021MstpCistEntry/ieee8021MstpCistMaxHops"
 * "/ieee8021-mstp:ieee8021-mstp/ieee8021MstpConfigIdTable/ieee8021MstpConfigIdEntry/ieee8021MstpConfigIdFormatSelector",
 * "/ieee8021-mstp:ieee8021-mstp/ieee8021MstpConfigIdTable/ieee8021MstpConfigIdEntry/ieee8021MstpConfigurationName",
 * "/ieee8021-mstp:ieee8021-mstp/ieee8021MstpConfigIdTable/ieee8021MstpConfigIdEntry/ieee8021MstpRevisionLevel",
 * "/ieee8021-mstp:ieee8021-mstp/ieee8021MstpPortTable/ieee8021MstpPortEntry/ieee8021MstpPortPriority",
 * "/ieee8021-mstp:ieee8021-mstp/ieee8021MstpPortTable/ieee8021MstpPortEntry/ieee8021MstpPortPathCost".
 *
 * Subscribe to it by sr_module_change_subscribe call.
 *
 * @param[in]   session         Automatically-created session that can be used
 *                              for obtaining changed data (e.g. by
 *                              sr_get_changes_iter call or sr_get_item -like
 *                              calls). Do not stop this session.
 * @param[in]   xpath           Data Path of the subtree where the change has
 *                              occurred.
 * @param[in]   event           Type of the notification event that has
 *                              occurred.
 * @param[in]   private_ctx     Private context opaque to sysrepo, as passed to
 *                              sr_module_change_subscribe call.
 * @return      Error code (SR_ERR_OK on success).
 */
static int mstp_element_cb(sr_session_ctx_t *session, const char *module_name,
                           const char *xpath, sr_event_t event,
                           uint32_t request_id, void *private_data)
{
    (void)private_data;
    (void)module_name;
    (void)request_id;

    int rc = SR_ERR_OK;
    sr_change_oper_t op = {0};
    sr_xpath_ctx_t st = {0};
    sr_change_iter_t *iter = NULL;
    sr_val_t *old_value = NULL;
    sr_val_t *new_value = NULL;
    sr_val_t *node = NULL;
    char mstp_comp_id[MAX_STR_LEN] = "";
    char mstp_id[MAX_STR_LEN] = "";
    char port_id[MAX_STR_LEN] = "";
    char mstp_cist_max_hops[MAX_STR_LEN] = "";
    char mstp_revision_level[MAX_STR_LEN] = "";
    char bridge_piority[MAX_STR_LEN] = "";
    char tmp_str[MAX_STR_LEN] = "";

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

        if ((SR_OP_CREATED == op) || (SR_OP_MODIFIED == op)) {
            /* if leaf from list ieee8021MstpConfigIdEntry is changed */
            if (strstr(node->xpath, MSTP_CONF_ID_ENTRY)) {
                /* get ieee8021MstpConfigIdComponentId from xpath */
                if (EXIT_SUCCESS != get_key_value(session, node->xpath,
                                                  MSTP_CONF_ID_ENTRY, MSTP_CONF_ID_COMP_ID,
                                                  &st, mstp_comp_id)) {
                    return SR_ERR_OPERATION_FAILED;
                }

                /* leaf ieee8021MstpConfigurationName */
                if (sr_xpath_node_name_eq(node->xpath, MSTP_CONF_ID_CONF_NAME_STR)) {
                    if (EXIT_SUCCESS != set_mstp_conf_name(mstp_comp_id, node->data.string_val)) {
                        sr_set_error(session, node->xpath,
                                     ERR_SET_FUNC_FAILED_XPATH_STR, node->xpath);
                        return SR_ERR_OPERATION_FAILED;
                    }
                }

                /* leaf ieee8021MstpRevisionLevel */
                if (sr_xpath_node_name_eq(node->xpath, MSTP_CONF_ID_REVISION_STR)) {
                    /* convert uint_32 to string */
                    snprintf(mstp_revision_level, MAX_STR_LEN, "%d", node->data.int32_val);
                    if (EXIT_SUCCESS != set_mstp_revision_level(mstp_comp_id, mstp_revision_level)) {
                        sr_set_error(session, node->xpath,
                                     ERR_SET_FUNC_FAILED_XPATH_STR, node->xpath);
                        return SR_ERR_OPERATION_FAILED;
                    }
                }
            }
            /* if leaf from list ieee8021MstpCistEntry is changed */
            else if (strstr(node->xpath, MSTP_CIST_ENTRY)) {
                /* leaf ieee8021MstpCistMaxHops */
                if (sr_xpath_node_name_eq(node->xpath, MSTP_CIST_MAX_HOPS_STR)) {
                    /* get ieee8021MstpCistComponentId from xpath */
                    if (EXIT_SUCCESS != get_key_value(session, node->xpath,
                                                      MSTP_CIST_ENTRY, MSTP_CIST_COMP_ID,
                                                      &st, mstp_comp_id)) {
                        return SR_ERR_OPERATION_FAILED;
                    }
                    /* convert uint_32 to string */
                    snprintf(mstp_cist_max_hops, MAX_STR_LEN, "%d", node->data.int32_val);
                    if (EXIT_SUCCESS != set_mstp_cist_max_hops(mstp_comp_id, mstp_cist_max_hops)) {
                        sr_set_error(session, node->xpath,
                                     ERR_SET_FUNC_FAILED_XPATH_STR, node->xpath);
                        return SR_ERR_OPERATION_FAILED;
                    }
                }
            }
            /* if leaf from list ieee8021MstpEntry is changed */
            else if (strstr(node->xpath, MSTP_ENTRY_STR)) {
                /* get ieee8021MstpComponentId from xpath */
                if (EXIT_SUCCESS != get_key_value(session, node->xpath,
                                                  MSTP_ENTRY_STR, MSTP_COMP_ID,
                                                  &st, mstp_comp_id)) {
                    return SR_ERR_OPERATION_FAILED;
                }

                /* get ieee8021MstpId from xpath */
                if (EXIT_SUCCESS != get_key_value(session, node->xpath,
                                                  MSTP_ENTRY_STR, MSTP_ID_STR,
                                                  &st, mstp_id)) {
                    return SR_ERR_OPERATION_FAILED;
                }

                /* convert int_32 to string */
                snprintf(bridge_piority, MAX_STR_LEN, "%d",
                         node->data.int32_val);

                /* leaf ieee8021MstpBridgePriority */
                if (sr_xpath_node_name_eq(node->xpath, MSTP_BRIDGE_PRIORITY_STR)) {
                    if (EXIT_SUCCESS != set_mstp_bridge_priority(mstp_comp_id, mstp_id, bridge_piority)) {
                        sr_set_error(session, node->xpath,
                                     ERR_SET_FUNC_FAILED_XPATH_STR, node->xpath);
                        return SR_ERR_OPERATION_FAILED;
                    }
                }
            }
            /* if leaf from list ieee8021MstpCistPortEntry is changed */
            else if (strstr(node->xpath, MSTP_CIST_PORT_ENTRY)) {
                /* get ieee8021MstpCistPortComponentId from xpath */
                if (EXIT_SUCCESS != get_key_value(session, node->xpath,
                                                  MSTP_CIST_PORT_ENTRY, MSTP_CIST_PORT_COMP_ID,
                                                  &st, mstp_comp_id)) {
                    return SR_ERR_OPERATION_FAILED;
                }

                /* get ieee8021MstpCistPortNum from xpath */
                if (EXIT_SUCCESS != get_key_value(session, node->xpath,
                                                  MSTP_CIST_PORT_ENTRY, MSTP_CIST_PORT_NUM_STR,
                                                  &st, port_id)) {
                    return SR_ERR_OPERATION_FAILED;
                }

                /* leaf ieee8021MstpCistPortAdminPathCost */
                if (sr_xpath_node_name_eq(node->xpath, MSTP_CIST_PORT_ADMIN_PATH_COST_STR)) {
                    /* convert admin path cost to string */
                    snprintf(tmp_str, MAX_STR_LEN, "%d", node->data.int32_val);

                    if (EXIT_SUCCESS != set_mstp_cist_port_admin_path_cost(mstp_comp_id, port_id, tmp_str)) {
                        sr_set_error(session, node->xpath,
                                     ERR_SET_FUNC_FAILED_XPATH_STR, node->xpath);
                        return SR_ERR_OPERATION_FAILED;
                    }
                }
                /* leaf ieee8021MstpCistPortAdminEdgePort */
                if (sr_xpath_node_name_eq(node->xpath, MSTP_CIST_PORT_ADMIN_EDGE_PORT_STR)) {
                    /* convert edge port to string */
                    snprintf(tmp_str, MAX_STR_LEN, "%d", node->data.bool_val);

                    if (EXIT_FAILURE == translate_single_string_using_map(tmp_str, true_false_map)) {
                        sr_set_error(session, node->xpath,
                                     ERR_INVALID_VALUE_XPATH_STR, node->xpath);
                        return SR_ERR_OPERATION_FAILED;
                    }
                    if (EXIT_SUCCESS != set_mstp_cist_port_admin_edge_port(mstp_comp_id, port_id, tmp_str)) {
                        sr_set_error(session, node->xpath,
                                     ERR_SET_FUNC_FAILED_XPATH_STR, node->xpath);
                        return SR_ERR_OPERATION_FAILED;
                    }
                }
                /* leaf ieee8021MstpCistPortMacEnabled */
                if (sr_xpath_node_name_eq(node->xpath, MSTP_CIST_PORT_MAC_ENABLED_STR)) {
                    /* convert mac enabled to string */
                    snprintf(tmp_str, MAX_STR_LEN, "%d", node->data.bool_val);
                    if (EXIT_FAILURE == translate_single_string_using_map(tmp_str, true_false_map)) {
                        sr_set_error(session, node->xpath,
                                     ERR_INVALID_VALUE_XPATH_STR, node->xpath);
                        return SR_ERR_OPERATION_FAILED;
                    }
                    if (EXIT_SUCCESS != set_mstp_cist_port_mac_enabled(mstp_comp_id, port_id, tmp_str)) {
                        sr_set_error(session, node->xpath,
                                     ERR_SET_FUNC_FAILED_XPATH_STR, node->xpath);
                        return SR_ERR_OPERATION_FAILED;
                    }
                }
                /* leaf ieee8021MstpCistPortRestrictedRole */
                if (sr_xpath_node_name_eq(node->xpath, MSTP_CIST_PORT_RESTR_ROLE_STR)) {
                    /* convert restriced role to string */
                    snprintf(tmp_str, MAX_STR_LEN, "%d", node->data.bool_val);

                    if (EXIT_FAILURE == translate_single_string_using_map(tmp_str, true_false_map)) {
                        sr_set_error(session, node->xpath,
                                     ERR_INVALID_VALUE_XPATH_STR, node->xpath);
                        return SR_ERR_OPERATION_FAILED;
                    }
                    if (EXIT_SUCCESS != set_mstp_cist_port_restricted_role(mstp_comp_id, port_id, tmp_str)) {
                        sr_set_error(session, node->xpath,
                                     ERR_SET_FUNC_FAILED_XPATH_STR, node->xpath);
                        return SR_ERR_OPERATION_FAILED;
                    }
                }
                /* leaf ieee8021MstpCistPortRestrictedTcn */
                if (sr_xpath_node_name_eq(node->xpath, MSTP_CIST_PORT_RESTR_TCN_STR)) {
                    /* convert restriced tcn to string */
                    snprintf(tmp_str, MAX_STR_LEN, "%d", node->data.bool_val);
                    if (EXIT_FAILURE == translate_single_string_using_map(tmp_str, true_false_map)) {
                        sr_set_error(session, node->xpath,
                                     ERR_INVALID_VALUE_XPATH_STR, node->xpath);
                        return SR_ERR_OPERATION_FAILED;
                    }
                    if (EXIT_SUCCESS != set_mstp_cist_port_restricted_tcn(mstp_comp_id, port_id, tmp_str)) {
                        sr_set_error(session, node->xpath,
                                     ERR_SET_FUNC_FAILED_XPATH_STR, node->xpath);
                        return SR_ERR_OPERATION_FAILED;
                    }
                }
            }
            /* if leaf from list ieee8021MstpPortEntry is changed */
            else if (strstr(node->xpath, MSTP_PORT_ENTRY)) {
                /* get ieee8021MstpPortComponentId from xpath */
                if (EXIT_SUCCESS != get_key_value(session, node->xpath,
                                                  MSTP_PORT_ENTRY, MSTP_PORT_COMP_ID,
                                                  &st, mstp_comp_id)) {
                    return SR_ERR_OPERATION_FAILED;
                }

                /* get ieee8021MstpPortMstId from xpath */
                if (EXIT_SUCCESS != get_key_value(session, node->xpath,
                                                  MSTP_PORT_ENTRY, MSTP_PORT_MSTP_ID,
                                                  &st, mstp_id)) {
                    return SR_ERR_OPERATION_FAILED;
                }

                /* get ieee8021MstpPortNum from xpath */
                if (EXIT_SUCCESS != get_key_value(session, node->xpath,
                                                  MSTP_PORT_ENTRY, MSTP_PORT_NUM_STR,
                                                  &st, port_id)) {
                    return SR_ERR_OPERATION_FAILED;
                }

                /* leaf ieee8021MstpPortPriority */
                if (sr_xpath_node_name_eq(node->xpath, MSTP_PORT_PRIORITY_STR)) {
                    /* convert priority to string */
                    snprintf(tmp_str, MAX_STR_LEN, "%d", node->data.int32_val);
                    if (EXIT_SUCCESS != set_mstp_port_priority(mstp_comp_id, port_id, mstp_id, tmp_str)) {
                        sr_set_error(session, node->xpath,
                                     ERR_SET_FUNC_FAILED_XPATH_STR, node->xpath);
                        return SR_ERR_OPERATION_FAILED;
                    }
                }

                /* leaf ieee8021MstpPortPathCost */
                if (sr_xpath_node_name_eq(node->xpath, MSTP_PORT_PATH_COST_STR)) {
                    /* convert path cost to string */
                    snprintf(tmp_str, MAX_STR_LEN, "%d", node->data.int32_val);
                    if (EXIT_SUCCESS != set_mstp_port_path_cost(mstp_comp_id, port_id, mstp_id, tmp_str)) {
                        sr_set_error(session, node->xpath,
                                     ERR_SET_FUNC_FAILED_XPATH_STR, node->xpath);
                        return SR_ERR_OPERATION_FAILED;
                    }
                }
            }
        }

        /* SR_OP_DELETED operation is not supported no matter what leaf is deleted */
        if (SR_OP_DELETED == op) {
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
 * @brief Callback to be called by the event of changing any running datastore
 * content within the module.
 *
 * @param[in]   session         Automatically-created session that can be used
 *                              for obtaining changed data (e.g. by
 *                              sr_get_changes_iter call ot sr_get_item -like
 *                              calls). Do not stop this session.
 * @param[in]   module_name     Name of the module where the change has occurred.
 * @param[in]   event           Type of the notification event that has occurred.
 * @param[in]   private_ctx     Private context opaque to sysrepo, as passed to
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
}

/**
 * @brief Creates a new entry for list ieee8021MstpConfigIdEntry from
 * ieee8021-mstp yang module.
 *
 * @param[out]  root            Pointer to struct lyd_node which is going to be
 *                              parent to created entry.
 * @param[in]   module          Main schema node structure representing YANG
 *                              module.
 * @param[in]   bridge_name     Contains the name of the bridge.
 * @return      struct lyd_node* or NULL if element not created.
 */
struct lyd_node *new_mstp_config_id_entry(struct lyd_node *root,
                                          const struct lys_module *module,
                                          char bridge_name[MAX_STR_LEN])
{
    struct lyd_node *ieee8021MstpConfigIdEntry = NULL;
    char tmp_array[MAX_NUM_OF_INTERFACES][MAX_STR_LEN];
    char encoded[MAX_STR_LEN] = "";

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    /* new ieee8021MstpCistEntry in ieee8021MstpConfigIdTable container */
    if (EXIT_FAILURE == new_container(root, &ieee8021MstpConfigIdEntry, module,
                                      MSTP_CONF_ID_ENTRY)) {
        return NULL;
    }
    /* list ieee8021MstpConfigIdEntry, leaf ieee8021MstpConfigIdComponentId, key */
    if (EXIT_FAILURE == new_node(ieee8021MstpConfigIdEntry, module,
                                 MSTP_CONF_ID_COMP_ID, BASE_ONE_STR)) {
        return NULL;
    }

    /* list ieee8021MstpConfigIdEntry, leaf ieee8021MstpConfigIdFormatSelector */
    if (EXIT_SUCCESS == get_mstp_config_id_format_selector(tmp_array, MAX_NUM_OF_INTERFACES,
                                                           MAX_STR_LEN, bridge_name)) {
        if (EXIT_FAILURE == new_node(ieee8021MstpConfigIdEntry, module,
                                     MSTP_CONF_ID_SELECTOR_STR, tmp_array[0])) {
            return NULL;
        }
    }

    /* list ieee8021MstpConfigIdEntry, leaf ieee8021MstpConfigurationName */
    if (EXIT_SUCCESS == get_mstp_configuration_name(tmp_array, MAX_NUM_OF_INTERFACES,
                                                    MAX_STR_LEN, bridge_name)) {
        if (EXIT_FAILURE == new_node(ieee8021MstpConfigIdEntry, module,
                                     MSTP_CONF_ID_CONF_NAME_STR, tmp_array[0])) {
            return NULL;
        }
    }

    /* list ieee8021MstpConfigIdEntry, leaf ieee8021MstpRevisionLevel */
    if (EXIT_SUCCESS == get_mstp_revision_level(tmp_array, MAX_NUM_OF_INTERFACES,
                                                MAX_STR_LEN, bridge_name)) {
        if (EXIT_FAILURE == new_node(ieee8021MstpConfigIdEntry, module,
                                     MSTP_CONF_ID_REVISION_STR, tmp_array[0])) {
            return NULL;
        }
    }

    /* list ieee8021MstpConfigIdEntry, leaf ieee8021MstpConfigurationDigest */
    if (EXIT_SUCCESS == get_mstp_configuration_digest(tmp_array, MAX_NUM_OF_INTERFACES,
                                                      MAX_STR_LEN, bridge_name)) {
        if (EXIT_SUCCESS == base64_encode((unsigned char*)tmp_array[0], IF_STAT_LEN, encoded)) {
            if (EXIT_FAILURE == new_node(ieee8021MstpConfigIdEntry, module,
                                         MSTP_CONF_DIGEST_STR, encoded)) {
                return NULL;
            }
        }
    }

    return ieee8021MstpConfigIdEntry;
}

/**
 * @brief Creates container ieee8021MstpConfigIdTable inside ieee8021-mstp yang
 * module.
 *
 * @param[out]  root            Pointer to struct lyd_node which is going to be
 *                              parent to created entry.
 * @param[in]   module          Main schema node structure representing YANG
 *                              module.
 * @param[in]   bridge_names    Contains the name of the bridges.
 * @return      struct lyd_node* or NULL if element not created.
 */
struct lyd_node *new_mstp_config_id_table(
        struct lyd_node *root,
        const struct lys_module *module,
        char bridge_names[MAX_NUM_OF_INTERFACES][MAX_STR_LEN])
{
    struct lyd_node *ieee8021MstpConfigIdTable = NULL;

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

     /* Container ieee8021MstpConfigIdTable */
    if (EXIT_FAILURE == new_container(root, &ieee8021MstpConfigIdTable, module,
                                      MSTP_CONF_ID_TABLE_STR)) {
        return NULL;
    }

    /* List ieee8021MstpConfigIdEntry* */
    if (NULL == new_mstp_config_id_entry(ieee8021MstpConfigIdTable, module,
                                         bridge_names[0])) {
        return NULL;
    }

    return ieee8021MstpConfigIdTable;
}

/**
 * @brief Creates a new entry for list ieee8021MstpCistEntry from ieee8021-mstp
 * yang module.
 *
 * @param[out]  root            Pointer to struct lyd_node which is going to be
 *                              parent to created entry.
 * @param[in]   module          Main schema node structure representing YANG
 *                              module.
 * @param[in]   bridge_name     Contains the name of the bridge.
 * @return      struct lyd_node* or NULL if element not created.
 */
struct lyd_node *new_mstp_cist_entry(struct lyd_node *root,
                                     const struct lys_module *module,
                                     char bridge_name[MAX_STR_LEN])
{
    struct lyd_node *ieee8021MstpCistEntry = NULL;
    char tmp_array[MAX_NUM_OF_INTERFACES][MAX_STR_LEN];

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    /* new ieee8021MstpCistEntry in ieee8021MstpCistTable container */
    if (EXIT_FAILURE == new_container(root, &ieee8021MstpCistEntry, module,
                                      MSTP_CIST_ENTRY)) {
        return NULL;
    }
    /* list ieee8021MstpCistEntry, leaf ieee8021MstpCistComponentId, key */
    if (EXIT_FAILURE == new_node(ieee8021MstpCistEntry, module,
                                 MSTP_CIST_COMP_ID, BASE_ONE_STR)) {
        return NULL;
    }
    /* list ieee8021MstpEntry, leaf ieee8021MstpBridgePriority */
    if (EXIT_SUCCESS == get_cist_bridge_max_hops(tmp_array, MAX_NUM_OF_INTERFACES,
                                                 MAX_STR_LEN, bridge_name)) {
        if (EXIT_FAILURE == new_node(ieee8021MstpCistEntry, module,
                                     MSTP_CIST_MAX_HOPS_STR, tmp_array[0])) {
            return NULL;
        }
    }

    return ieee8021MstpCistEntry;
}

/**
 * @brief Creates container ieee8021MstpCistTable inside ieee8021-mstp yang
 * module.
 *
 * @param[out]  root            Pointer to struct lyd_node which is going to be
 *                              parent to created entry.
 * @param[in]   module          Main schema node structure representing YANG
 *                              module.
 * @param[in]   bridge_names    Contains the name of the bridges.
 * @return      struct lyd_node* or NULL if element not created.
 */
struct lyd_node *new_mstp_cist_table(
        struct lyd_node *root,
        const struct lys_module *module,
        char bridge_names[MAX_NUM_OF_INTERFACES][MAX_STR_LEN])
{
    struct lyd_node *ieee8021MstpCistTable = NULL;

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

     /* Container ieee8021MstpCistTable */
    if (EXIT_FAILURE == new_container(root, &ieee8021MstpCistTable, module,
                                      MSTP_CIST_TABLE_STR)) {
        return NULL;
    }

    /* List ieee8021MstpCistEntry */
    if (NULL == new_mstp_cist_entry(ieee8021MstpCistTable, module,
                                    bridge_names[0])) {
        return NULL;
    }

    return ieee8021MstpCistTable;
}

/**
 * @brief Creates a new entry for list ieee8021MstpEntry from ieee8021-mstp
 * yang module.
 *
 * @param[out]  root            Pointer to struct lyd_node which is going to be
 *                              parent to created entry.
 * @param[in]   module          Main schema node structure representing YANG
 *                              module.
 * @param[in]   bridge_name     Contains the name of the bridge.
 * @param[in]   current_mstp_id Contains the value of the bridge MSTI.
 * @return      struct lyd_node* or NULL if element not created.
 */
struct lyd_node *new_mstp_entry(struct lyd_node *root,
                                const struct lys_module *module,
                                char bridge_name[MAX_STR_LEN],
                                char current_mstp_id[MAX_STR_LEN])
{
    struct lyd_node *ieee8021MstpEntry = NULL;
    char tmp_array[MAX_NUM_OF_INTERFACES][MAX_STR_LEN];

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    /* new ieee8021MstpEntry in ieee8021MstpTable container */
    if (EXIT_FAILURE == new_container(root, &ieee8021MstpEntry, module,
                                      MSTP_ENTRY_STR)) {
        return NULL;
    }

    /* list ieee8021MstpEntry, leaf id, key */
    if (EXIT_FAILURE == new_node(ieee8021MstpEntry, module,
                                 MSTP_COMP_ID, BASE_ONE_STR)) {
        return NULL;
    }

    /* list ieee8021MstpEntry, leaf ieee8021MstpId, key */
    if (EXIT_FAILURE == new_node(ieee8021MstpEntry, module,
                                 MSTP_ID_STR, current_mstp_id)) {
        return NULL;
    }

    /* list ieee8021MstpEntry, leaf ieee8021MstpBridgePriority */
    if (EXIT_SUCCESS == get_mstp_bridge_priority(tmp_array, MAX_NUM_OF_INTERFACES,
                                                 MAX_STR_LEN, bridge_name, current_mstp_id)) {
        if (EXIT_FAILURE == new_node(ieee8021MstpEntry, module,
                                     MSTP_BRIDGE_PRIORITY_STR, tmp_array[0])) {
            return NULL;
        }
    }

    return ieee8021MstpEntry;
}

/**
 * @brief Creates container ieee8021MstpTable inside ieee8021-mstp yang module.
 *
 * @param[out]  root            Pointer to struct lyd_node which is going to be
 *                              parent to created entry.
 * @param[in]   module          Main schema node structure representing YANG
 *                              module.
 * @param[in]   bridge_names    Contains the name of the bridges.
 * @return      struct lyd_node* or NULL if element not created.
 */
struct lyd_node *new_mstp_table(struct lyd_node *root,
                                const struct lys_module *module,
                                char bridge_names[MAX_NUM_OF_INTERFACES][MAX_STR_LEN])
{
    struct lyd_node *ieee8021MstpTable = NULL;
    char mstp_ids[BASE_MAX_VID][BASE_TEN];
    int i = 0;

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

     /* Container ieee8021MstpTable */
    if (EXIT_FAILURE == new_container(root, &ieee8021MstpTable, module, MSTP_TABLE_STR)) {
        return NULL;
    }

    if (EXIT_FAILURE == get_bridge_mstpi(mstp_ids, BASE_MAX_VID, BASE_TEN, bridge_names[0])) {
        return NULL;
    }

    /* List ieee8021MstpEntry */
    while ((i < BASE_MAX_VID) && (0 != strncmp(mstp_ids[i], "", BASE_TEN))) {
        if (NULL == new_mstp_entry(ieee8021MstpTable, module,
                                   bridge_names[0], mstp_ids[i])) {
            return NULL;
        }
        i++;
    }

    return ieee8021MstpTable;
}

/**
 * @brief Creates a new entry for list ieee8021MstpCistPortEntry from
 * ieee8021-mstp yang module.
 *
 * @param[out]  root            Pointer to struct lyd_node which is going to be
 *                              parent to created entry.
 * @param[in]   module          Main schema node structure representing YANG
 *                              module.
 * @param[in]   bridge_name     Contains the name of the bridge.
 * @param[in]   port_name       Contains the name of the port.
 * @return      struct lyd_node* or NULL if element not created.
 */
struct lyd_node *new_mstp_cist_port_entry(struct lyd_node *root,
                                          const struct lys_module *module,
                                          char bridge_name[MAX_STR_LEN],
                                          char port_name[MAX_STR_LEN])
{
    struct lyd_node *ieee8021MstpCistPortEntry = NULL;
    char tmp_array[MAX_NUM_OF_INTERFACES][MAX_STR_LEN];

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    /* new ieee8021MstpCistPortEntry in ieee8021MstpCistTable container */
    if (EXIT_FAILURE == new_container(root, &ieee8021MstpCistPortEntry, module,
                                      MSTP_CIST_PORT_ENTRY)) {
        return NULL;
    }

    /* list ieee8021MstpCistPortEntry, leaf ieee8021MstpCistPortComponentId, key */
    if (EXIT_FAILURE == new_node(ieee8021MstpCistPortEntry, module,
                                 MSTP_CIST_PORT_COMP_ID, BASE_ONE_STR)) {
        return NULL;
    }

    /* list ieee8021MstpCistPortEntry, leaf ieee8021MstpCistPortNum, key */
    if (EXIT_SUCCESS == get_mstp_port_id(tmp_array, MAX_NUM_OF_INTERFACES,
                                         MAX_STR_LEN, bridge_name, port_name)) {
        if (EXIT_FAILURE == new_node(ieee8021MstpCistPortEntry, module,
                                     MSTP_CIST_PORT_NUM_STR, tmp_array[0])) {
            return NULL;
        }
    }

    /* list ieee8021MstpCistPortEntry, leaf ieee8021MstpCistPortAdminPathCost */
    if (EXIT_SUCCESS == get_cist_port_admin_path_cost(tmp_array, MAX_NUM_OF_INTERFACES,
                                                      MAX_STR_LEN, bridge_name, port_name)) {
        if (EXIT_FAILURE == new_node(ieee8021MstpCistPortEntry, module,
                                     MSTP_CIST_PORT_ADMIN_PATH_COST_STR, tmp_array[0])) {
            return NULL;
        }
    }

    /* list ieee8021MstpCistPortEntry, leaf ieee8021MstpCistPortAdminEdgePort */
    if (EXIT_SUCCESS == get_cist_port_admin_edge_port(tmp_array, MAX_NUM_OF_INTERFACES,
                                                      MAX_STR_LEN, bridge_name, port_name)) {
        if (EXIT_FAILURE == translate_strings_using_map(tmp_array, yes_no_map)) {
            return NULL;
        }
        if (EXIT_FAILURE == new_node(ieee8021MstpCistPortEntry, module,
                                     MSTP_CIST_PORT_ADMIN_EDGE_PORT_STR, (char*)tmp_array)) {
            return NULL;
        }
    }

    /* list ieee8021MstpCistPortEntry, leaf ieee8021MstpCistPortMacEnabled */
    if (EXIT_SUCCESS == get_cist_port_mac_enabled(tmp_array, MAX_NUM_OF_INTERFACES,
                                                  MAX_STR_LEN, bridge_name, port_name)) {
        if (EXIT_FAILURE == translate_strings_using_map(tmp_array, yes_no_map)) {
            return NULL;
        }
        if (EXIT_FAILURE == new_node(ieee8021MstpCistPortEntry, module,
                                     MSTP_CIST_PORT_MAC_ENABLED_STR, (char*)tmp_array)) {
            return NULL;
        }
    }

    /* list ieee8021MstpCistPortEntry, leaf ieee8021MstpCistPortRestrictedRole */
    if (EXIT_SUCCESS == get_cist_port_restricted_role(tmp_array, MAX_NUM_OF_INTERFACES,
                                                      MAX_STR_LEN, bridge_name, port_name)) {
        if (EXIT_FAILURE == translate_strings_using_map(tmp_array, yes_no_map)) {
            return NULL;
        }
        if (EXIT_FAILURE == new_node(ieee8021MstpCistPortEntry, module,
                                     MSTP_CIST_PORT_RESTR_ROLE_STR, (char*)tmp_array)) {
            return NULL;
        }
    }

    /* list ieee8021MstpCistPortEntry, leaf ieee8021MstpCistPortRestrictedTcn */
    if (EXIT_SUCCESS == get_cist_port_restricted_tcn(tmp_array, MAX_NUM_OF_INTERFACES,
                                                     MAX_STR_LEN, bridge_name, port_name)) {
        if (EXIT_FAILURE == translate_strings_using_map(tmp_array, yes_no_map)) {
            return NULL;
        }
        if (EXIT_FAILURE == new_node(ieee8021MstpCistPortEntry, module,
                                     MSTP_CIST_PORT_RESTR_TCN_STR, (char*)tmp_array)) {
            return NULL;
        }
    }

    return ieee8021MstpCistPortEntry;
}

/**
 * @brief Creates container ieee8021MstpCistPortTable inside ieee8021-mstp yang
 * module.
 *
 * @param[out]  root            Pointer to struct lyd_node which is going to be
 *                              parent to created entry.
 * @param[in]   module          Main schema node structure representing YANG
 *                              module.
 * @param[in]   bridge_names    Contains the name of the bridges.
 * @return      struct lyd_node* or NULL if element not created.
 */
struct lyd_node *new_mstp_cist_port_table(
        struct lyd_node *root,
        const struct lys_module *module,
        char bridge_names[MAX_NUM_OF_INTERFACES][MAX_STR_LEN])
{
    struct lyd_node *ieee8021MstpCistPortTable = NULL;
    char port_names[MAX_NUM_OF_INTERFACES][MAX_STR_LEN];
    int i = 0;

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

     /* Container ieee8021MstpCistPortTable */
    if (EXIT_FAILURE == new_container(root, &ieee8021MstpCistPortTable, module,
                                      MSTP_CIST_PORT_TABLE_STR)) {
        return NULL;
    }

    if (EXIT_FAILURE == get_mstp_bridge_port_names(port_names, MAX_NUM_OF_INTERFACES,
                                                   MAX_STR_LEN, bridge_names[0])) {
        return NULL;
    }

    /* List ieee8021MstpCistPortEntry */
    while ((i < MAX_NUM_OF_INTERFACES) &&
           (0 != strncmp("", port_names[i], MAX_STR_LEN))) {
        if (NULL == new_mstp_cist_port_entry(ieee8021MstpCistPortTable, module,
                                             bridge_names[0], port_names[i])) {
            return NULL;
        }
        i++;
    }

    return ieee8021MstpCistPortTable;
}

/**
 * @brief Creates a new entry for list ieee8021MstpPortEntry from ieee8021-mstp
 * yang module.
 *
 * @param[out]  root            Pointer to struct lyd_node which is going to be
 *                              parent to created entry.
 * @param[in]   module          Main schema node structure representing YANG
 *                              module.
 * @param[in]   bridge_name     Contains the name of the bridge.
 * @param[in]   current_mstp_id Contains the value of the bridge MSTI.
 * @param[in]   port_name       Contains the name of the port.
 * @return      struct lyd_node* or NULL if element not created.
 */
struct lyd_node *new_mstp_port_entry(struct lyd_node *root,
                                     const struct lys_module *module,
                                     char bridge_name[MAX_STR_LEN],
                                     char current_mstp_id[MAX_STR_LEN],
                                     char port_name[MAX_STR_LEN])
{
    struct lyd_node *ieee8021MstpPortEntry = NULL;
    char tmp_array[MAX_NUM_OF_INTERFACES][MAX_STR_LEN];

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    /* new ieee8021MstpPortEntry in ieee8021MstpPortTable container */
    if (EXIT_FAILURE == new_container(root, &ieee8021MstpPortEntry, module,
                                      MSTP_PORT_ENTRY)) {
        return NULL;
    }

    /* list ieee8021MstpPortEntry, leaf ieee8021MstpPortComponentId, key */
    if (EXIT_FAILURE == new_node(ieee8021MstpPortEntry, module,
                                 MSTP_PORT_COMP_ID, BASE_ONE_STR)) {
        return NULL;
    }

    /* list ieee8021MstpPortEntry, leaf ieee8021MstpPortMstId, key */
    if (EXIT_FAILURE == new_node(ieee8021MstpPortEntry, module,
                                 MSTP_PORT_MSTP_ID, current_mstp_id)) {
        return NULL;
    }

    /* list ieee8021MstpPortEntry, leaf ieee8021MstpPortNum, key */
    if (EXIT_SUCCESS == get_mstp_port_id(tmp_array, MAX_NUM_OF_INTERFACES,
                                         MAX_STR_LEN, bridge_name, port_name)) {
        if (EXIT_FAILURE == new_node(ieee8021MstpPortEntry, module,
                                     MSTP_PORT_NUM_STR, tmp_array[0])) {
            return NULL;
        }
    }

    /* list ieee8021MstpPortEntry, leaf ieee8021MstpPortPriority */
    if (EXIT_SUCCESS == get_mstp_port_prioriy(tmp_array, MAX_NUM_OF_INTERFACES,
                                              MAX_STR_LEN, bridge_name, port_name, current_mstp_id)) {
        if (EXIT_FAILURE == new_node(ieee8021MstpPortEntry, module,
                                     MSTP_PORT_PRIORITY_STR, tmp_array[0])) {
            return NULL;
        }
    }

    /* list ieee8021MstpPortEntry, leaf ieee8021MstpPortPathCost */
    if (EXIT_SUCCESS == get_mstp_port_path_cost(tmp_array, MAX_NUM_OF_INTERFACES,
                                                MAX_STR_LEN, bridge_name, port_name, current_mstp_id)) {
        if (EXIT_FAILURE == new_node(ieee8021MstpPortEntry, module,
                                     MSTP_PORT_PATH_COST_STR, tmp_array[0])) {
            return NULL;
        }
    }

    return ieee8021MstpPortEntry;
}

/**
 * @brief Creates container ieee8021MstpPortTable inside ieee8021-mstp yang
 * module.
 *
 * @param[out]  root            Pointer to struct lyd_node which is going to be
 *                              parent to created entry.
 * @param[in]   module          Main schema node structure representing YANG
 *                              module.
 * @param[in]   bridge_names    Contains the name of the bridges.
 * @return      struct lyd_node* or NULL if element not created.
 */
struct lyd_node *new_mstp_port_table(
        struct lyd_node *root, const struct lys_module *module,
        char bridge_names[MAX_NUM_OF_INTERFACES][MAX_STR_LEN])
{
    struct lyd_node *ieee8021MstpPortTable = NULL;
    char mstp_ids[BASE_MAX_VID][BASE_TEN] = {0};
    char port_names[MAX_NUM_OF_INTERFACES][MAX_STR_LEN] = {0};
    int i = 0;
    int j = 0;

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

     /* Container ieee8021MstpPortTable */
    if (EXIT_FAILURE == new_container(root, &ieee8021MstpPortTable, module,
                                      MSTP_PORT_TABLE_STR)) {
        return NULL;
    }

    if (EXIT_FAILURE == get_bridge_mstpi(mstp_ids, BASE_MAX_VID, BASE_TEN,
                                         bridge_names[0])) {
        return NULL;
    }

    if (EXIT_FAILURE == get_mstp_bridge_port_names(port_names, MAX_NUM_OF_INTERFACES,
                                                   MAX_STR_LEN, bridge_names[0])) {
        return NULL;
    }

    /* List ieee8021MstpPortEntry */
    while ((i < BASE_MAX_VID) && (0 != strncmp("", mstp_ids[i], BASE_TEN))) {
        while ((j < MAX_NUM_OF_INTERFACES) && (0 != strncmp("", port_names[j], MAX_STR_LEN))) {
            if (NULL == new_mstp_port_entry(ieee8021MstpPortTable, module,
                                            bridge_names[0], mstp_ids[i],
                                            port_names[j])) {
                return NULL;
            }
            j++;
        }
        j = 0;
        i++;
    }

    return ieee8021MstpPortTable;
}

/**
 * @brief Creates a startup datastore for ieee802-dot1q-bridge module.
 *
 * Function is going to be called once, due to sysrepo-plugind startup.
 *
 * @param[in]   session         Sysrepo session that can be used for any API
 *                              calls needed for plugin initialization (mainly
 *                              for reading of startup configuration and
 *                              subscribing for notifications).
 *
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
static int mstp_fill_datastore(sr_session_ctx_t *session)
{
    const struct ly_ctx *ctx = NULL;
    const struct lys_module *module = NULL;
    struct lyd_node *root = NULL;
    char bridge_names[MAX_NUM_OF_INTERFACES][MAX_STR_LEN] = {0};
    int fd = -1;
    char *path = NULL;

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    if (0 == sr_path_check_startup_done_file(MSTP_MODULE_NAME, &path)) {

        ctx = sr_get_context(sr_session_get_connection(session));
        if (NULL == ctx) {
            SRP_LOG_ERR(ERROR_MSG_FUN_AND_MSG, __func__,
                        ERR_SESSION_CTX_FAILED_STR);
            free(path);
            return EXIT_FAILURE;
        }

        root = lyd_new_path(NULL, ctx, "/ieee8021-mstp:ieee8021-mstp", NULL, 0, 0);
        if (NULL == root) {
            SRP_LOG_ERR(ERROR_MSG_FUN_AND_MSG, __func__,
                        ERR_NOT_CREATED_ROOT_ELEMENT_STR);
            free(path);
            return EXIT_FAILURE;
        }

        if (EXIT_FAILURE == get_mstp_bridge_names(bridge_names, MAX_NUM_OF_INTERFACES,
                                                  MAX_STR_LEN)) {
            SRP_LOG_ERR(ERROR_MSG_FUN_AND_MSG, __func__, ERR_MSG_NO_BRIDGES);
            free(path);
            return EXIT_FAILURE;
        }
        // if system supports more than one bridge print out error message
        if ((0 != strncmp(bridge_names[0], "", MAX_STR_LEN)) &&
            (0 != strncmp(bridge_names[1], "", MAX_STR_LEN))) {
            SRP_LOG_ERR(ERROR_MSG_FUN_AND_MSG, __func__,
                        ERR_MSG_MORE_BRIDGES);
            free(path);
            return EXIT_FAILURE;
        }

        /* Container ieee8021MstpCistTable from ieee8021-mstp */
        if (NULL == new_mstp_cist_table(root, module, bridge_names)) {
            SRP_LOG_INF(ERROR_MSG_FUN_XML_EL_AND_MSG, __func__,
                        MSTP_CIST_TABLE_STR, ERR_NOT_CREATED_ELEMENT_STR);
        }

        /* Container ieee8021MstpTable from ieee8021-mstp */
        if (NULL == new_mstp_table(root, module, bridge_names)) {
            SRP_LOG_INF(ERROR_MSG_FUN_XML_EL_AND_MSG, __func__,
                        MSTP_TABLE_STR, ERR_NOT_CREATED_ELEMENT_STR);
        }

        /* Container ieee8021MstpCistPortTable from ieee8021-mstp */
        if (NULL == new_mstp_cist_port_table(root, module, bridge_names)) {
            SRP_LOG_INF(ERROR_MSG_FUN_XML_EL_AND_MSG, __func__,
                        MSTP_CIST_PORT_TABLE_STR, ERR_NOT_CREATED_ELEMENT_STR);
        }

        /* Container ieee8021MstpPortTable from ieee8021-mstp */
        if (NULL == new_mstp_port_table(root, module, bridge_names)) {
            SRP_LOG_INF(ERROR_MSG_FUN_XML_EL_AND_MSG, __func__,
                        MSTP_PORT_TABLE_STR, ERR_NOT_CREATED_ELEMENT_STR);
        }

        /* Container ieee8021MstpConfigIdTable from ieee8021-mstp */
        if (NULL == new_mstp_config_id_table(root, module, bridge_names)) {
            SRP_LOG_INF(ERROR_MSG_FUN_XML_EL_AND_MSG, __func__,
                        MSTP_CONF_ID_TABLE_STR, ERR_NOT_CREATED_ELEMENT_STR);
        }

        /******** NOTE *******/
        /* The startup to running datastore copy is done before the plugin is
         * started. So, here we will replace the current startup and running
         * datastores with the subtree called 'root' in this function.
         */
        if (SR_ERR_OK != sr_session_switch_ds(session, SR_DS_STARTUP)) {
            SRP_LOG_ERR(ERROR_MSG_FUN_AND_MSG, __func__,
                        ERR_SWITCH_DATASTORE_FAILED_STR);
            free(path);
            return EXIT_FAILURE;
        }

        /* Replace current running configuration with created 'root' subtree */
        if (SR_ERR_OK != sr_replace_config(session, MSTP_NAME, root, 0, 0)) {
            SRP_LOG_ERR(ERROR_MSG_FUN_AND_MSG, __func__,
                        ERR_REPLACE_CONFIG_FAILED_STR);
            free(path);
            return EXIT_FAILURE;
        }

        if (SR_ERR_OK != sr_session_switch_ds(session, SR_DS_RUNNING)) {
            SRP_LOG_ERR(ERROR_MSG_FUN_AND_MSG, __func__, ERR_SWITCH_DATASTORE_FAILED_STR);
            free(path);
            return EXIT_FAILURE;
        }

        if (SR_ERR_OK != sr_copy_config(session, MSTP_NAME, SR_DS_STARTUP, 0, 0)) {
            SRP_LOG_ERR(ERROR_MSG_FUN_AND_MSG, __func__, ERR_COPY_DATASTORE_FAILED_STR);
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

static int mstp_state_cb(sr_session_ctx_t *session, const char *module_name,
                         const char *path, const char *request_xpath,
                         uint32_t request_id, struct lyd_node **parent,
                         void *private_data)
{
    (void)session;
    (void)request_id;
    (void)module_name;
    (void)request_xpath;
    (void)request_id;
    (void)private_data;
    (void)path;

    const struct ly_ctx *ctx = NULL;
    char bridge_names[MAX_NUM_OF_INTERFACES][MAX_STR_LEN] = {0};
    char tmp_array[MAX_NUM_OF_INTERFACES][MAX_STR_LEN] = {0};
    char tmp_xpath[2*MAX_STR_LEN] = "";
    char encoded[MAX_STR_LEN] = "";
    char mstp_ids[BASE_MAX_VID][BASE_TEN] = {0};
    char port_names[MAX_NUM_OF_INTERFACES][MAX_STR_LEN] = {0};
    char port_id[MAX_NUM_OF_INTERFACES][MAX_STR_LEN] = {0};
    int i = 0;
    int j = 0;

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    /* get MSTP bridge names */
    if (EXIT_FAILURE == get_mstp_bridge_names(bridge_names, MAX_NUM_OF_INTERFACES, MAX_STR_LEN)) {
        return SR_ERR_OPERATION_FAILED;
    }
    // if system supports more than one bridge print out error message
    if ((0 != strncmp(bridge_names[0], "", MAX_STR_LEN)) &&
        (0 != strncmp(bridge_names[1], "", MAX_STR_LEN))) {
        SRP_LOG_ERR(ERROR_MSG_FUN_AND_MSG, __func__, ERR_MSG_MORE_BRIDGES);
        return SR_ERR_OPERATION_FAILED;
    }

    ctx = sr_get_context(sr_session_get_connection(session));
    if (NULL == ctx) {
        SRP_LOG_ERR(ERROR_MSG_FUN_AND_MSG, __func__, ERR_SESSION_CTX_FAILED_STR);
        return EXIT_FAILURE;
    }

    *parent = lyd_new_path(NULL, ctx, "/ieee8021-mstp:ieee8021-mstp", NULL, 0, 0);
    if (NULL == *parent) {
        return SR_ERR_OPERATION_FAILED;
    }

    /* STATE DATA FOR CONTAINER "ieee8021MstpCistTable" */
    /* list ieee8021MstpCistEntry, leaf ieee8021MstpCistBridgeIdentifier */
    if (EXIT_SUCCESS == get_cist_bridge_comp_id(tmp_array, MAX_NUM_OF_INTERFACES,
                                                MAX_STR_LEN, bridge_names[0])) {
        if (EXIT_SUCCESS == base64_encode((unsigned char*)tmp_array[0], BASE_EIGHT, encoded)) {
            fill_xpath(tmp_xpath, MSTP_CIST_ENTRY_XPATH, BASE_ONE_STR, MSTP_CIST_BRIDGE_ID_STR);
            if (NULL == lyd_new_path(*parent, NULL, tmp_xpath, encoded, 0, 0)) {
                SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                            MSTP_CIST_BRIDGE_ID_STR, ERR_NOT_CREATED_ELEMENT_STR);
            }
        }
    }

    /* list ieee8021MstpCistEntry, ieee8021MstpCistTopologyChange */
    if (EXIT_SUCCESS == get_cist_bridge_topology_change(tmp_array, MAX_NUM_OF_INTERFACES,
                                                        MAX_STR_LEN, bridge_names[0])) {
        if (EXIT_SUCCESS == translate_strings_using_map(tmp_array, yes_no_map)) {
            fill_xpath(tmp_xpath, MSTP_CIST_ENTRY_XPATH, BASE_ONE_STR, MSTP_CIST_TOP_CHANGE_STR);
            if (NULL == lyd_new_path(*parent, NULL, tmp_xpath, tmp_array[0], 0, 0)) {
                SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                            MSTP_CIST_TOP_CHANGE_STR, ERR_NOT_CREATED_ELEMENT_STR);
            }
        }
    }

    /* list ieee8021MstpCistEntry, ieee8021MstpCistRegionalRootIdentifier */
    if (EXIT_SUCCESS == get_cist_bridge_reg_root_identifier(tmp_array, MAX_NUM_OF_INTERFACES,
                                                            MAX_STR_LEN, bridge_names[0])) {
        if (EXIT_SUCCESS == base64_encode((unsigned char*)tmp_array[0], BASE_EIGHT, encoded)) {
            fill_xpath(tmp_xpath, MSTP_CIST_ENTRY_XPATH, BASE_ONE_STR, MSTP_CIST_REG_ROOT_ID_STR);
            if (NULL == lyd_new_path(*parent, NULL, tmp_xpath, encoded, 0, 0)) {
                SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                            MSTP_CIST_REG_ROOT_ID_STR, ERR_NOT_CREATED_ELEMENT_STR);
            }
        }
    }

    /* list ieee8021MstpCistEntry, ieee8021MstpCistPathCost */
    if (EXIT_SUCCESS == get_cist_bridge_path_cost(tmp_array, MAX_NUM_OF_INTERFACES,
                                                  MAX_STR_LEN, bridge_names[0])) {
        fill_xpath(tmp_xpath, MSTP_CIST_ENTRY_XPATH, BASE_ONE_STR, MSTP_CIST_PATH_COST_STR);
        if (NULL == lyd_new_path(*parent, NULL, tmp_xpath, tmp_array[0], 0, 0)) {
            SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                        MSTP_CIST_PATH_COST_STR, ERR_NOT_CREATED_ELEMENT_STR);
        }
    }

    /* list ieee8021MstpCistEntry, ieee8021MstpCistBridgePriority */
    if (EXIT_SUCCESS == get_cist_bridge_priority(tmp_array, MAX_NUM_OF_INTERFACES,
                                                 MAX_STR_LEN, bridge_names[0])) {
        fill_xpath(tmp_xpath, MSTP_CIST_ENTRY_XPATH, BASE_ONE_STR, MSTP_CIST_BRIDGE_PRIO_STR);
        if (NULL == lyd_new_path(*parent, NULL, tmp_xpath, tmp_array[0], 0, 0)) {
            SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                        MSTP_CIST_BRIDGE_PRIO_STR, ERR_NOT_CREATED_ELEMENT_STR);
        }
    }

    /* STATE DATA FOR CONTAINER "ieee8021MstpTable" */
    if (EXIT_FAILURE == get_bridge_mstpi(mstp_ids, BASE_MAX_VID, BASE_TEN, bridge_names[0])) {
        return SR_ERR_OPERATION_FAILED;
    }

    /* List ieee8021MstpEntry */
    while ((i < BASE_MAX_VID) && (0 != strncmp(mstp_ids[i], "", BASE_TEN))) {
        /* list ieee8021MstpEntry, leaf ieee8021MstpBridgeId */
        if (EXIT_SUCCESS == get_mstp_bridge_id(tmp_array, MAX_NUM_OF_INTERFACES,
                                               MAX_STR_LEN, bridge_names[0], mstp_ids[i])) {
            if (EXIT_SUCCESS == base64_encode((unsigned char*)tmp_array[0], BASE_EIGHT, encoded)) {
                fill_xpath(tmp_xpath, MSTP_ENTRY_XPATH, BASE_ONE_STR, mstp_ids[i], MSTP_BRIDGE_ID_STR);
                if (NULL == lyd_new_path(*parent, NULL, tmp_xpath, encoded, 0, 0)) {
                    SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                                MSTP_BRIDGE_ID_STR, ERR_NOT_CREATED_ELEMENT_STR);
                }
            }
        }

        /* list ieee8021MstpEntry, leaf ieee8021MstpTimeSinceTopologyChange */
        if (EXIT_SUCCESS == get_mstp_time_since_top_change(tmp_array, MAX_NUM_OF_INTERFACES,
                                                           MAX_STR_LEN, bridge_names[0], mstp_ids[i])) {
            fill_xpath(tmp_xpath, MSTP_ENTRY_XPATH, BASE_ONE_STR, mstp_ids[i], MSTP_SINCE_TOP_CHANGE_STR);
            if (NULL == lyd_new_path(*parent, NULL, tmp_xpath, tmp_array[0], 0, 0)) {
                SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                            MSTP_SINCE_TOP_CHANGE_STR, ERR_NOT_CREATED_ELEMENT_STR);
            }
        }

        /* list ieee8021MstpEntry, leaf ieee8021MstpTopologyChanges */
        if (EXIT_SUCCESS == get_mstp_top_changes(tmp_array, MAX_NUM_OF_INTERFACES,
                                                 MAX_STR_LEN, bridge_names[0], mstp_ids[i])) {
            fill_xpath(tmp_xpath, MSTP_ENTRY_XPATH, BASE_ONE_STR, mstp_ids[i], MSTP_TOP_CHANGES_STR);
            if (NULL == lyd_new_path(*parent, NULL, tmp_xpath, tmp_array[0], 0, 0)) {
                SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                            MSTP_TOP_CHANGES_STR, ERR_NOT_CREATED_ELEMENT_STR);
            }
        }

        /* list ieee8021MstpEntry, leaf ieee8021MstpDesignatedRoot */
        if (EXIT_SUCCESS == get_mstp_designated_root(tmp_array, MAX_NUM_OF_INTERFACES,
                                                     MAX_STR_LEN, bridge_names[0])) {
            if (EXIT_SUCCESS == base64_encode((unsigned char*)tmp_array[0], BASE_EIGHT, encoded)) {
                fill_xpath(tmp_xpath, MSTP_ENTRY_XPATH, BASE_ONE_STR, mstp_ids[i], MSTP_DSGN_ROOT_STR);
                if (NULL == lyd_new_path(*parent, NULL, tmp_xpath, encoded, 0, 0)) {
                    SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                                MSTP_DSGN_ROOT_STR, ERR_NOT_CREATED_ELEMENT_STR);
                }
            }
        }

        /* list ieee8021MstpEntry, leaf ieee8021MstpRootPathCost */
        if (EXIT_SUCCESS == get_mstp_root_path_cost(tmp_array, MAX_NUM_OF_INTERFACES,
                                                    MAX_STR_LEN, bridge_names[0], mstp_ids[i])) {
            fill_xpath(tmp_xpath, MSTP_ENTRY_XPATH, BASE_ONE_STR, mstp_ids[i], MSTP_ROOT_PATH_COST_STR);
            if (NULL == lyd_new_path(*parent, NULL, tmp_xpath, tmp_array[0], 0, 0)) {
                SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                            MSTP_ROOT_PATH_COST_STR, ERR_NOT_CREATED_ELEMENT_STR);
            }
        }

        /* list ieee8021MstpEntry, leaf ieee8021MstpRootPort */
        if (EXIT_SUCCESS == get_mstp_root_port(tmp_array, MAX_NUM_OF_INTERFACES,
                                               MAX_STR_LEN, bridge_names[0], mstp_ids[i])) {
            /* NOTE: function get_mstp_root_port fills up tmp_array[0] with
             * string "none" (That is the only value we can get from board).
             * That is correct value on board, but leaf ieee8021MstpRootPort is
             * unit_32 whose value range is "1..65535". strtol function return
             * 0, because she can not convert string "none" to int. So if
             * tmp_array[0] is different than "none", that convert it to int
             * and create leaf, because than RootPort exist. If tmp_array[0] is
             * "none", then leaf ieee8021MstpRootPort will not be created.
             */
            if (strncmp(tmp_array[0], "none", MAX_STR_LEN)) {
                fill_xpath(tmp_xpath, MSTP_ENTRY_XPATH, BASE_ONE_STR, mstp_ids[i], MSTP_ROOT_PORT_STR);
                if (NULL == lyd_new_path(*parent, NULL, tmp_xpath, tmp_array[0], 0, 0)) {
                    SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                                MSTP_ROOT_PORT_STR, ERR_NOT_CREATED_ELEMENT_STR);
                }
            }
        }
        i++;
    }

    /* STATE DATA FOR CONTAINER "ieee8021MstpCistPortTable" */
    if (EXIT_FAILURE == get_mstp_bridge_port_names(port_names, MAX_NUM_OF_INTERFACES,
                                                   MAX_STR_LEN, bridge_names[0])) {
        return SR_ERR_OPERATION_FAILED;
    }

    i = 0;
    /* List ieee8021MstpCistPortEntry */
    while ((i < MAX_NUM_OF_INTERFACES) &&
           (0 != strncmp("", port_names[i], MAX_STR_LEN))) {
        /* list ieee8021MstpCistPortEntry get port id by port name */
        if (EXIT_SUCCESS == get_mstp_port_id(port_id, MAX_NUM_OF_INTERFACES,
                                             MAX_STR_LEN, bridge_names[0], port_names[i])) {
            /* list ieee8021MstpCistPortEntry, leaf ieee8021MstpCistPortDesignatedRoot */
            if (EXIT_SUCCESS == get_cist_port_designated_root(tmp_array, MAX_NUM_OF_INTERFACES,
                                                              MAX_STR_LEN, bridge_names[0], port_names[i])) {
                if (EXIT_SUCCESS == base64_encode((unsigned char*)tmp_array[0], BASE_EIGHT, encoded)) {
                    fill_xpath(tmp_xpath, MSTP_CIST_PORT_ENTRY_XPATH,
                               BASE_ONE_STR, port_id[0], MSTP_CIST_PORT_DSGN_ROOT_STR);
                    if (NULL == lyd_new_path(*parent, NULL, tmp_xpath, encoded, 0, 0)) {
                        SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                                    MSTP_CIST_PORT_DSGN_ROOT_STR, ERR_NOT_CREATED_ELEMENT_STR);
                    }
                }
            }
            /* list ieee8021MstpCistPortEntry, leaf ieee8021MstpCistPortTopologyChangeAck */
            if (EXIT_SUCCESS == get_cist_port_top_change_ack(tmp_array, MAX_NUM_OF_INTERFACES,
                                                             MAX_STR_LEN, bridge_names[0], port_names[i])) {
                if (EXIT_SUCCESS == translate_strings_using_map(tmp_array, yes_no_map)) {
                    fill_xpath(tmp_xpath, MSTP_CIST_PORT_ENTRY_XPATH, BASE_ONE_STR, port_id[0], MSTP_CIST_PORT_TOP_CHANGE_ACK_STR);
                    if (NULL == lyd_new_path(*parent, NULL, tmp_xpath, tmp_array[0], 0, 0)) {
                        SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                                    MSTP_CIST_PORT_TOP_CHANGE_ACK_STR, ERR_NOT_CREATED_ELEMENT_STR);
                    }
                }
            }
            /* list ieee8021MstpCistPortEntry, leaf ieee8021MstpCistPortHelloTime */
            if (EXIT_SUCCESS == get_cist_port_hello_time(tmp_array, MAX_NUM_OF_INTERFACES,
                                                         MAX_STR_LEN, bridge_names[0], port_names[i])) {
                fill_xpath(tmp_xpath, MSTP_CIST_PORT_ENTRY_XPATH, BASE_ONE_STR, port_id[0], MSTP_CIST_PORT_HELLO_TIME_STR);
                if (NULL == lyd_new_path(*parent, NULL, tmp_xpath, tmp_array[0], 0, 0)) {
                    SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                                MSTP_CIST_PORT_HELLO_TIME_STR, ERR_NOT_CREATED_ELEMENT_STR);
                }
            }
            /* list ieee8021MstpCistPortEntry, leaf ieee8021MstpCistPortOperEdgePort */
            if (EXIT_SUCCESS == get_cist_port_oper_edge_port(tmp_array, MAX_NUM_OF_INTERFACES,
                                                             MAX_STR_LEN, bridge_names[0], port_names[i])) {
                if (EXIT_SUCCESS == translate_strings_using_map(tmp_array, yes_no_map)) {
                    fill_xpath(tmp_xpath, MSTP_CIST_PORT_ENTRY_XPATH, BASE_ONE_STR, port_id[0], MSTP_CIST_PORT_OPER_EDGE_PORT_STR);
                    if (NULL == lyd_new_path(*parent, NULL, tmp_xpath, tmp_array[0], 0, 0)) {
                        SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                                    MSTP_CIST_PORT_OPER_EDGE_PORT_STR, ERR_NOT_CREATED_ELEMENT_STR);
                    }
                }
            }
            /* list ieee8021MstpCistPortEntry, leaf ieee8021MstpCistPortRole */
            if (EXIT_SUCCESS == get_cist_port_role(tmp_array, MAX_NUM_OF_INTERFACES,
                                                   MAX_STR_LEN, bridge_names[0], port_names[i])) {
                if (EXIT_SUCCESS == translate_strings_using_map(tmp_array, port_role_map)) {
                    fill_xpath(tmp_xpath, MSTP_CIST_PORT_ENTRY_XPATH, BASE_ONE_STR, port_id[0], MSTP_CIST_PORT_PORT_ROLE_STR);
                    if (NULL == lyd_new_path(*parent, NULL, tmp_xpath, tmp_array[0], 0, 0)) {
                        SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                                    MSTP_CIST_PORT_PORT_ROLE_STR, ERR_NOT_CREATED_ELEMENT_STR);
                    }
                }
            }
            /* list ieee8021MstpCistPortEntry, leaf ieee8021MstpCistPortDisputed */
            if (EXIT_SUCCESS == get_cist_port_disputed(tmp_array, MAX_NUM_OF_INTERFACES,
                                                       MAX_STR_LEN, bridge_names[0], port_names[i])) {
                if (EXIT_SUCCESS == translate_strings_using_map(tmp_array, yes_no_map)) {
                    fill_xpath(tmp_xpath, MSTP_CIST_PORT_ENTRY_XPATH, BASE_ONE_STR, port_id[0], MSTP_CIST_PORT_DISPUTED_STR);
                    if (NULL == lyd_new_path(*parent, NULL, tmp_xpath, tmp_array[0], 0, 0)) {
                        SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                                    MSTP_CIST_PORT_DISPUTED_STR, ERR_NOT_CREATED_ELEMENT_STR);
                    }
                }
            }
            /* list ieee8021MstpCistPortEntry, leaf ieee8021MstpCistPortCistRegionalRootId */
            if (EXIT_SUCCESS == get_cist_port_reg_root_id(tmp_array, MAX_NUM_OF_INTERFACES,
                                                          MAX_STR_LEN, bridge_names[0], port_names[i])) {
                if (EXIT_SUCCESS == base64_encode((unsigned char*)tmp_array[0], BASE_EIGHT, encoded)) {
                    fill_xpath(tmp_xpath, MSTP_CIST_PORT_ENTRY_XPATH, BASE_ONE_STR, port_id[0], MSTP_CIST_PORT_REG_ROOT_ID_STR);
                    if (NULL == lyd_new_path(*parent, NULL, tmp_xpath, encoded, 0, 0)) {
                        SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                                    MSTP_CIST_PORT_REG_ROOT_ID_STR, ERR_NOT_CREATED_ELEMENT_STR);
                    }
                }
            }
            /* list ieee8021MstpCistPortEntry, leaf ieee8021MstpCistPortCistPathCost */
            if (EXIT_SUCCESS == get_cist_port_path_cost(tmp_array, MAX_NUM_OF_INTERFACES,
                                                        MAX_STR_LEN, bridge_names[0], port_names[i])) {
                fill_xpath(tmp_xpath, MSTP_CIST_PORT_ENTRY_XPATH, BASE_ONE_STR, port_id[0], MSTP_CIST_PORT_PATH_COST_STR);
                if (NULL == lyd_new_path(*parent, NULL, tmp_xpath, tmp_array[0], 0, 0)) {
                    SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                                MSTP_CIST_PORT_PATH_COST_STR, ERR_NOT_CREATED_ELEMENT_STR);
                }
            }
        }
        i++;
    }

    /* List ieee8021MstpPortEntry */
    i = 0;
    while ((i < BASE_MAX_VID) && (0 != strncmp("", mstp_ids[i], BASE_TEN))) {
        while ((j < MAX_NUM_OF_INTERFACES) &&
               (0 != strncmp("", port_names[j], MAX_STR_LEN))) {
            /* list ieee8021MstpPortEntry get port id by port name */
            if (EXIT_SUCCESS == get_mstp_port_id(port_id, MAX_NUM_OF_INTERFACES,
                                                 MAX_STR_LEN, bridge_names[0], port_names[j])) {
                /* list ieee8021MstpPortEntry, leaf ieee8021MstpPortState */
                if (EXIT_SUCCESS == get_mstp_port_state(tmp_array, MAX_NUM_OF_INTERFACES,
                                                        MAX_STR_LEN, bridge_names[0], port_names[j], mstp_ids[i])) {
                    fill_xpath(tmp_xpath, MSTP_PORT_ENTRY_XPATH, BASE_ONE_STR, mstp_ids[i], port_id[0], MSTP_PORT_STATE_STR);
                    if (NULL == lyd_new_path(*parent, NULL, tmp_xpath, tmp_array[0], 0, 0)) {
                        SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                                    MSTP_PORT_STATE_STR, ERR_NOT_CREATED_ELEMENT_STR);
                    }
                }
                /* list ieee8021MstpPortEntry, leaf ieee8021MstpPortDesignatedRoot */
                if (EXIT_SUCCESS == get_mstp_port_dsgn_root(tmp_array, MAX_NUM_OF_INTERFACES,
                                                            MAX_STR_LEN, bridge_names[0], port_names[j], mstp_ids[i])) {
                    if (EXIT_SUCCESS == base64_encode((unsigned char*)tmp_array[0], BASE_EIGHT, encoded)) {
                        fill_xpath(tmp_xpath, MSTP_PORT_ENTRY_XPATH, BASE_ONE_STR, mstp_ids[i], port_id[0], MSTP_PORT_DSGN_ROOT_STR);
                        if (NULL == lyd_new_path(*parent, NULL, tmp_xpath, encoded, 0, 0)) {
                            SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                                        MSTP_PORT_DSGN_ROOT_STR, ERR_NOT_CREATED_ELEMENT_STR);
                        }
                    }
                }
                /* list ieee8021MstpPortEntry, leaf ieee8021MstpPortDesignatedCost */
                if (EXIT_SUCCESS == get_mstp_port_dsgn_cost(tmp_array, MAX_NUM_OF_INTERFACES,
                                                            MAX_STR_LEN, bridge_names[0], port_names[j], mstp_ids[i])) {
                    fill_xpath(tmp_xpath, MSTP_PORT_ENTRY_XPATH, BASE_ONE_STR, mstp_ids[i], port_id[0], MSTP_PORT_DSGN_COST_STR);
                    if (NULL == lyd_new_path(*parent, NULL, tmp_xpath, tmp_array[0], 0, 0)) {
                        SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                                    MSTP_PORT_DSGN_COST_STR, ERR_NOT_CREATED_ELEMENT_STR);
                    }
                }
                /* list ieee8021MstpPortEntry, leaf ieee8021MstpPortDesignatedBridge */
                if (EXIT_SUCCESS == get_mstp_port_dsgn_bridge(tmp_array, MAX_NUM_OF_INTERFACES,
                                                              MAX_STR_LEN, bridge_names[0], port_names[j], mstp_ids[i])) {
                    if (EXIT_SUCCESS == base64_encode((unsigned char*)tmp_array[0], BASE_EIGHT, encoded)) {
                        fill_xpath(tmp_xpath, MSTP_PORT_ENTRY_XPATH, BASE_ONE_STR, mstp_ids[i], port_id[0], MSTP_PORT_DSGN_BRIDGE_STR);
                        if (NULL == lyd_new_path(*parent, NULL, tmp_xpath, encoded, 0, 0)) {
                            SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                                        MSTP_PORT_DSGN_BRIDGE_STR, ERR_NOT_CREATED_ELEMENT_STR);
                        }
                    }
                }
                /* list ieee8021MstpPortEntry, leaf ieee8021MstpPortDesignatedPort */
                if (EXIT_SUCCESS == get_mstp_port_dsgn_port(tmp_array, MAX_NUM_OF_INTERFACES,
                                                            MAX_STR_LEN, bridge_names[0], port_names[j], mstp_ids[i])) {
                    /* NOTE: function get_mstp_port_dsgn_port fills up tmp_array[0] with designated port value.
                     * That value on board is 0.000, so function will return string "0". Returned string will be last zero after ".".
                     * This leaf is IEEE8021BridgePortNumber type, and his range is from 1 to 65535. So if function returns "0"
                     * then we do not need to fill up leaf. */
                    if (strncmp(tmp_array[0], "0", MAX_STR_LEN)) {
                        fill_xpath(tmp_xpath, MSTP_PORT_ENTRY_XPATH, BASE_ONE_STR, mstp_ids[i], port_id[0], MSTP_PORT_DSGN_PORT_STR);
                        if (NULL == lyd_new_path(*parent, NULL, tmp_xpath, tmp_array[0], 0, 0)) {
                            SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                                        MSTP_PORT_DSGN_PORT_STR, ERR_NOT_CREATED_ELEMENT_STR);
                        }
                    }
                }
                /* list ieee8021MstpPortEntry, leaf ieee8021MstpPortRole */
                if (EXIT_SUCCESS == get_mstp_port_role(tmp_array, MAX_NUM_OF_INTERFACES,
                                                       MAX_STR_LEN, bridge_names[0], port_names[j], mstp_ids[i])) {
                    if (EXIT_SUCCESS == translate_strings_using_map(tmp_array, port_role_map)) {
                        fill_xpath(tmp_xpath, MSTP_PORT_ENTRY_XPATH, BASE_ONE_STR, mstp_ids[i], port_id[0], MSTP_PORT_ROLE_STR);
                        if (NULL == lyd_new_path(*parent, NULL, tmp_xpath, tmp_array[0], 0, 0)) {
                            SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                                        MSTP_PORT_ROLE_STR, ERR_NOT_CREATED_ELEMENT_STR);
                        }
                    }
                }
                /* list ieee8021MstpPortEntry, leaf ieee8021MstpPortDisputed */
                if (EXIT_SUCCESS == get_mstp_port_disputed(tmp_array, MAX_NUM_OF_INTERFACES,
                                                           MAX_STR_LEN, bridge_names[0], port_names[j], mstp_ids[i])) {
                    if (EXIT_SUCCESS == translate_strings_using_map(tmp_array, yes_no_map)) {
                        fill_xpath(tmp_xpath, MSTP_PORT_ENTRY_XPATH, BASE_ONE_STR, mstp_ids[i], port_id[0], MSTP_PORT_DISPUTED_STR);
                        if (NULL == lyd_new_path(*parent, NULL, tmp_xpath, tmp_array[0], 0, 0)) {
                            SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                                        MSTP_PORT_DISPUTED_STR, ERR_NOT_CREATED_ELEMENT_STR);
                        }
                    }
                }
            }
            j++;
        }
        j = 0;
        i++;
    }

    return SR_ERR_OK;
}

/**
 * @brief Sysrepo plugin initialization callback.
 *
 * @param[in]   session         Sysrepo session that can be used for any API
 *                              calls needed for plugin initialization (mainly
 *                              for reading of startup configuration and
 *                              subscribing for notifications).
 * @param[in]   private_ctx     Private context (opaque to sysrepo) that will
 *                              be passed to sr_plugin_cleanup_cb when plugin
 *                              cleanup is requested.
 * @return      Error code (SR_ERR_OK on success). If an error is returned,
 *              plugin will be considered as uninitialized.
 */
int sr_plugin_init_cb(sr_session_ctx_t *session, void **private_ctx)
{
    (void)private_ctx;
    int rc = SR_ERR_OK;
    char *path = NULL;

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    /* Set plugin_init to "1" so callbacks are triggered when startup
     * configuration is copied to running.
     */
    if (0 != sr_path_check_startup_done_file(MSTP_MODULE_NAME, &path)) {
        plugin_init = 1;
    }

    do {
        /* subscribe for ieee8021-mstp module changes */
        rc = sr_module_change_subscribe(session, MSTP_NAME, NULL, module_change_cb, NULL, 0,
                SR_SUBSCR_ENABLED, &subscription);
        if (SR_ERR_OK != rc) break;

        /* subscribe for "/ieee8021-mstp:ieee8021-mstp/ieee8021MstpCistTable/ieee8021MstpCistEntry/ieee8021MstpCistMaxHops" changes */
        rc = sr_module_change_subscribe(session, MSTP_NAME,
                "/ieee8021-mstp:ieee8021-mstp/ieee8021MstpCistTable/ieee8021MstpCistEntry/ieee8021MstpCistMaxHops",
                mstp_element_cb, NULL, 1, SR_SUBSCR_ENABLED, &subscription);
        if (SR_ERR_OK != rc) break;

        /* subscribe for "/ieee8021-mstp:ieee8021-mstp/ieee8021MstpConfigIdTable/ieee8021MstpConfigIdEntry/ieee8021MstpConfigIdFormatSelector" changes */
        rc = sr_module_change_subscribe(session, MSTP_NAME,
                "/ieee8021-mstp:ieee8021-mstp/ieee8021MstpConfigIdTable/ieee8021MstpConfigIdEntry/ieee8021MstpConfigIdFormatSelector",
                mod_not_supported_cb, NULL, 1, SR_SUBSCR_ENABLED, &subscription);
        if (SR_ERR_OK != rc) break;

        /* subscribe for "/ieee8021-mstp:ieee8021-mstp/ieee8021MstpConfigIdTable/ieee8021MstpConfigIdEntry/ieee8021MstpConfigurationName" changes */
        rc = sr_module_change_subscribe(session, MSTP_NAME,
                "/ieee8021-mstp:ieee8021-mstp/ieee8021MstpConfigIdTable/ieee8021MstpConfigIdEntry/ieee8021MstpConfigurationName",
                mstp_element_cb, NULL, 1, SR_SUBSCR_ENABLED, &subscription);
        if (SR_ERR_OK != rc) break;

        /* subscribe for "/ieee8021-mstp:ieee8021-mstp/ieee8021MstpConfigIdTable/ieee8021MstpConfigIdEntry/ieee8021MstpRevisionLevel" changes */
        rc = sr_module_change_subscribe(session, MSTP_NAME,
                "/ieee8021-mstp:ieee8021-mstp/ieee8021MstpConfigIdTable/ieee8021MstpConfigIdEntry/ieee8021MstpRevisionLevel",
                mstp_element_cb, NULL, 1, SR_SUBSCR_ENABLED, &subscription);
        if (SR_ERR_OK != rc) break;

        /* subscribe for the /ieee8021-mstp:ieee8021-mstp/ieee8021MstpConfigIdTable/ieee8021MstpConfigIdEntry/ieee8021MstpConfigurationDigest */
        rc = sr_module_change_subscribe(session, MSTP_NAME,
                "/ieee8021-mstp:ieee8021-mstp/ieee8021MstpConfigIdTable/ieee8021MstpConfigIdEntry/ieee8021MstpConfigurationDigest",
                mod_not_supported_cb, NULL, 1, SR_SUBSCR_ENABLED, &subscription);
        if (SR_ERR_OK != rc) break;

        /* subscribe for "/ieee8021-mstp:ieee8021-mstp/ieee8021MstpTable/ieee8021MstpEntry/ieee8021MstpBridgePriority" changes */
        rc = sr_module_change_subscribe(session, MSTP_NAME,
                "/ieee8021-mstp:ieee8021-mstp/ieee8021MstpTable/ieee8021MstpEntry/ieee8021MstpBridgePriority",
                mstp_element_cb, NULL, 1, SR_SUBSCR_ENABLED, &subscription);
        if (SR_ERR_OK != rc) break;

        /* subscribe for "/ieee8021-mstp:ieee8021-mstp/ieee8021MstpCistPortTable/ieee8021MstpCistPortEntry/ieee8021MstpCistPortAdminPathCost" changes */
        rc = sr_module_change_subscribe(session, MSTP_NAME,
                "/ieee8021-mstp:ieee8021-mstp/ieee8021MstpCistPortTable/ieee8021MstpCistPortEntry/ieee8021MstpCistPortAdminPathCost",
                mstp_element_cb, NULL, 1, SR_SUBSCR_ENABLED, &subscription);
        if (SR_ERR_OK != rc) break;

        /* subscribe for "/ieee8021-mstp:ieee8021-mstp/ieee8021MstpCistPortTable/ieee8021MstpCistPortEntry/ieee8021MstpCistPortAdminPathCost" changes */
        rc = sr_module_change_subscribe(session, MSTP_NAME,
                "/ieee8021-mstp:ieee8021-mstp/ieee8021MstpCistPortTable/ieee8021MstpCistPortEntry/ieee8021MstpCistPortAdminEdgePort",
                mstp_element_cb, NULL, 1, SR_SUBSCR_ENABLED, &subscription);
        if (SR_ERR_OK != rc) break;

        /* subscribe for "/ieee8021-mstp:ieee8021-mstp/ieee8021MstpCistPortTable/ieee8021MstpCistPortEntry/ieee8021MstpCistPortAdminPathCost" changes */
        rc = sr_module_change_subscribe(session, MSTP_NAME,
                "/ieee8021-mstp:ieee8021-mstp/ieee8021MstpCistPortTable/ieee8021MstpCistPortEntry/ieee8021MstpCistPortMacEnabled",
                mstp_element_cb, NULL, 1, SR_SUBSCR_ENABLED, &subscription);
        if (SR_ERR_OK != rc) break;

        /* subscribe for "/ieee8021-mstp:ieee8021-mstp/ieee8021MstpCistPortTable/ieee8021MstpCistPortEntry/ieee8021MstpCistPortAdminPathCost" changes */
        rc = sr_module_change_subscribe(session, MSTP_NAME,
                "/ieee8021-mstp:ieee8021-mstp/ieee8021MstpCistPortTable/ieee8021MstpCistPortEntry/ieee8021MstpCistPortRestrictedRole",
                mstp_element_cb, NULL, 1, SR_SUBSCR_ENABLED, &subscription);
        if (SR_ERR_OK != rc) break;

        /* subscribe for "/ieee8021-mstp:ieee8021-mstp/ieee8021MstpCistPortTable/ieee8021MstpCistPortEntry/ieee8021MstpCistPortAdminPathCost" changes */
        rc = sr_module_change_subscribe(session, MSTP_NAME,
                "/ieee8021-mstp:ieee8021-mstp/ieee8021MstpCistPortTable/ieee8021MstpCistPortEntry/ieee8021MstpCistPortRestrictedTcn",
                mstp_element_cb, NULL, 1, SR_SUBSCR_ENABLED, &subscription);
        if (SR_ERR_OK != rc) break;

        /* subscribe for "/ieee8021-mstp:ieee8021-mstp/ieee8021MstpPortTable/ieee8021MstpPortEntry/ieee8021MstpCistPortAdminPathCost" changes */
        rc = sr_module_change_subscribe(session, MSTP_NAME,
                "/ieee8021-mstp:ieee8021-mstp/ieee8021MstpPortTable/ieee8021MstpPortEntry/ieee8021MstpPortPriority",
                mstp_element_cb, NULL, 1, SR_SUBSCR_ENABLED, &subscription);
        if (SR_ERR_OK != rc) break;

        /* subscribe for "/ieee8021-mstp:ieee8021-mstp/ieee8021MstpPortTable/ieee8021MstpPortEntry/ieee8021MstpCistPortAdminPathCost" changes */
        rc = sr_module_change_subscribe(session, MSTP_NAME,
                "/ieee8021-mstp:ieee8021-mstp/ieee8021MstpPortTable/ieee8021MstpPortEntry/ieee8021MstpPortPathCost",
                mstp_element_cb, NULL, 1, SR_SUBSCR_ENABLED, &subscription);
        if (SR_ERR_OK != rc) break;

        /* subscribe as state data provider for the /ieee8021-mstp:ieee8021-mstp/ */
        rc = sr_oper_get_items_subscribe(session, MSTP_NAME, "/ieee8021-mstp:ieee8021-mstp",
                mstp_state_cb, NULL, SR_SUBSCR_DEFAULT, &subscription);
        if (SR_ERR_OK != rc) break;

    } while (0);

    if (SR_ERR_OK != rc) {
        SRP_LOG_ERR(ERR_MODULE_INIT_FAILED_STR, MSTP_NAME, sr_strerror(rc));
        sr_unsubscribe(subscription);
        return rc;
    }

    if (EXIT_FAILURE == mstp_fill_datastore(session)) {
        SRP_LOG_ERR(ERROR_MSG_MOD_FUNC_MSG, MSTP_NAME, __func__,
                    ERR_MSG_EMPTY_STARTUP_DS_STR);
        return SR_ERR_OPERATION_FAILED;
    }

    plugin_init = 1;

    SRP_LOG_INF(INF_MODULE_INIT_SUCCESS_STR, MSTP_NAME);

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
    (void)session;
    (void)private_data;

    /* nothing to cleanup except freeing the subscriptions */
    sr_unsubscribe(subscription);
    SRP_LOG_INF(INF_MODULE_CLEANUP_STR, MSTP_NAME);
}
