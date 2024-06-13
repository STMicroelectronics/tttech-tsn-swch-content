/*
 * TTTech ieee802-dot1q-bridge-yang-module
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
#include <pthread.h>
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
#include "libbase/base_q_bridge_linux_tools.h"
/* libtsn includes */
#include "libtsn/tsn.h"
#include "libtsn/tsn_sid.h"
#include "libtsn/tsn_frer.h"
#include "libtsn/tsn_def.h"
/* module specific includes */
#include "frer_defines.h"

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

enum _seqenc_active {
    ENC_PASSIVE = 0,
    ENC_ACTIVE = 1,
    ENC_UNKNOWN = 2,    // used when entry is added but active not set yet
    ENC_UNDEFINED = 3,  // used at init, entry doesnt exist
};

static char active_map[2][2][MAX_STR_LEN] = {
    // 2 pairs
    { "Active",     BASE_TRUE },
    { "Passive",    BASE_FALSE },
};

static char algorithm_map[2][2][MAX_STR_LEN] = {
    // 2 pairs
    { FRER_VECTOR_ALG_STR,  BASE_ZERO_STR },
    { FRER_MATCH_ALG_STR,   BASE_ONE_STR },
};

/**
 * @brief Function that converts uint8_t to string. Function is used to convert
 * leaf active.
 *
 * @param[in]   active  uint8_t that represents leaf active.
 * @param[out]  str     String value of leaf active.
 */
void seqenc_set_active_str(uint8_t active, char *str)
{
    if (active == ENC_ACTIVE)
        strncpy(str, "Active", 7);
    else if (active == ENC_PASSIVE)
        strncpy(str, "Passive", 8);
    else
        strncpy(str, "Unknown", 8);
}

/**
 * @brief This function checks if port with deleted stream-list exists on board
 * inside sequence-identification-list list.
 *
 * @param[in]   session         Implicit session (do not stop) with information
 *                              about the changed data (retrieved by
 *                              sr_get_changes_iter) the event originator
 *                              session IDs.
 * @param[in]   node            Pointer to sr_val_t structure representing node
 *                              that is changed.
 * @return      Error code (EXIT_SUCCESS on success).
 */
int check_stream_indentification_list(sr_session_ctx_t *session, sr_val_t *node)
{
    sr_xpath_ctx_t st = {0};
    char port_name[MAX_STR_LEN] = "";
    struct port_hdl* port = NULL;
    uint32_t active;
    char str[] = "ENC_UNDEFINED";

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    /* get port value, key of list sequence-identification-list list */
    if (EXIT_SUCCESS != get_key_value(session, node->xpath, FRER_SEQ_ID_LIST_STR,
                                      FRER_PORT_STR, &st, port_name)) {
        SRP_LOG_ERR("%s: %s (%s)", __func__,
                    ERR_MISSING_ELEMENT_STR, FRER_PORT_STR);
        return EXIT_FAILURE;
    }

    /* open port */
    port = tsn_port_open_by_name(port_name);
    if (NULL == port) {
        SRP_LOG_ERR(ERROR_MSG_MOD_FUNC_EL_MSG, FRER_MODULE_NAME_STR, __func__,
                    port_name, ERR_PORT_NOT_OPEN_STR);
        return EXIT_FAILURE;
    }

    /* if leaf active is found */
    if (0 == tsn_frer_seqenc_getactive(port, &active)) {
        /* this function converts uint32_t to string (Active, Passive, Unknown) */
        seqenc_set_active_str(active, str);

        /* If leaf active exists and it does not have Unknown value, that means
         * that entry exist on board otherwise entry does not exist on board.
         */
        if (EXIT_SUCCESS != translate_single_string_using_map(str, active_map)) {
            /* close port */
            tsn_port_close(port);
            return EXIT_FAILURE;
        }
    } else {
        SRP_LOG_ERR(ERROR_MSG_FUN_AND_MSG, __func__, ERR_FRER_PORT2STR_GET_STR);
        tsn_port_close(port);
        return EXIT_FAILURE;
    }

    /* close port */
    tsn_port_close(port);

    return EXIT_SUCCESS;
}

#if 0
/**
 * @brief Subroutine that is executed by the thread.
 *
 * @param[in]   callback_value  Pointer to void that contains the arguments of
 *                              the function address_thread.
 * @return      NULL.
  */
void *port_list_thread(void *callback_value)
{
    sr_val_t *node = (sr_val_t*)callback_value;
    sr_conn_ctx_t *refresh_connection = NULL;
    sr_session_ctx_t *refresh_session = NULL;
    sr_xpath_ctx_t st = {0};
    char fn_index[MAX_STR_LEN] = "";
    char path[MAX_STR_LEN] = "";

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    if (NULL == node) {
        SRP_LOG_ERR(ERR_ARGUMENT_OF_ROUTINE_STR,  __func__);
        return NULL;
    }

    /* Creating new connection */
    if (SR_ERR_OK  != sr_connect(0, &refresh_connection)) {
        SRP_LOG_ERR(ERR_NEW_CONNECTION_ROUTINE_STR, __func__);
        sr_free_val(node);
        return NULL;
    }

    /*  Staring new session with RUNNING datastore */
    if (SR_ERR_OK  != sr_session_start(refresh_connection, SR_DS_RUNNING, &refresh_session)) {
        SRP_LOG_ERR(ERR_NEW_SESSION_FAILED_STR, __func__);
        sr_free_val(node);
        return NULL;
    }

    /* get index value, key of list sequence-recovery */
    if (EXIT_SUCCESS != get_key_value(NULL, (char*)node->xpath,
                                      FRER_SEQ_RECOVERY_LIST_STR, FRER_INDEX_STR, &st, fn_index)) {
        SRP_LOG_ERR("%s: %s (%s)", __func__,
                    ERR_MISSING_ELEMENT_STR, FRER_INDEX_STR);
        sr_free_val(node);
        return NULL;
    }

    /* Check if the changed node is port-list. */
    if (NULL != strstr(node->xpath, FRER_PORT_LIST_STR)) {
        fill_xpath(path, FRER_SEQ_REC_LIST_XPATH, fn_index);
        if (SR_ERR_OK != sr_delete_item(refresh_session, path, SR_EDIT_STRICT)) {
            SRP_LOG_ERR(ERR_DELETING_ENTRY_STR, FRER_SEQ_RECOVERY_LIST_STR, __func__);
            sr_free_val(node);
            return NULL;
        }
    }

    if (SR_ERR_OK != sr_apply_changes(refresh_session, 0, 0)) {
        SRP_LOG_ERR(ERR_APPLYING_CHANGES_FAILED_STR, __func__);
        sr_free_val(node);
        sr_session_stop(refresh_session);
        sr_disconnect(refresh_connection);
        return NULL;
    }

    sr_free_val(node);
    sr_session_stop(refresh_session);
    sr_disconnect(refresh_connection);
    refresh_session = NULL;
    refresh_connection = NULL;

    return NULL;
}
#endif

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

    rc = sr_get_items(session, "/ieee802-dot1cb-frer:ieee802-dot1cb-frer//*",
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
 * @brief Creates a new entry for sequence-generation list inside container
 * frer inside ieee802-dot1cb-frer yang module.
 *
 * It is used for config data.
 *
 * @param[in]   parent          Pointer to struct lyd_node which is going to be
 *                              parent to created entry.
 * @param[in]   module          Main schema node structure representing YANG
 *                              module.
 * @param[in]   port            Poiner to struct port_hdl struct, that
 *                              represents opened port.
 * @param[in]   fn_index        Leaf direction, key of rer-seg-gen-entry list.
 * @return  struct lyd_node* or NULL if element not created.
 */
struct lyd_node *new_sequence_generation_list_entry(
        struct lyd_node *parent, const struct lys_module *module,
        struct port_hdl *port, uint16_t fn_index)
{

    struct lyd_node *frer_seq_gen_entry = NULL;
    char tmp[MAX_STR_LEN] = "";
    char message[MAX_STR_LEN] = "";
    uint32_t max_streams = 0;
    uint32_t ret = 0u;
    uint32_t i = 0;
    uint16_t *strlist = NULL;
    uint8_t dir = 0;

    SRP_LOG_DBG(DEBUG_MSG_FUN_CALLED, __func__);

    /* new sequence-generation entry */
    if (EXIT_FAILURE == new_container(parent, &frer_seq_gen_entry, module,
                                      FRER_SEQ_GEN_LIST_STR)) {
        return NULL;
    }

    /* leaf index, key of the list */
    snprintf(tmp, MAX_STR_LEN, "%u", fn_index);
    if (EXIT_FAILURE == new_node(frer_seq_gen_entry, module,
                                 FRER_INDEX_STR, tmp)) {
        return NULL;
    }

    ret = tsn_sid_get_max_sup(port, &max_streams);
    if (ret) {
        SRP_LOG_ERR(ERROR_MSG_FUN_AND_MSG, __func__, ERR_FRER_STREAM_LIST_STR);
        return NULL;
    }

    strlist =  malloc(sizeof(*strlist)*max_streams);
    if (NULL == strlist) {
        SRP_LOG_ERR(ERROR_MSG_FUN_AND_MSG, __func__,
                    ERR_FRER_STREAM_LIST_ALLOCATE_STR);
        return NULL;
    }

    if (!ret) {
        if (0 == tsn_frer_seqgen_fn2sid(port, fn_index, strlist, max_streams)) {
            /* list sequence-generation, leaf 'direction' */
            if (0 == tsn_frer_seqgen_fn2dir(port, fn_index, &dir)) {
                snprintf(tmp, MAX_STR_LEN, "%s", (dir ? "true" : "false"));
                if (EXIT_FAILURE == new_node(frer_seq_gen_entry, module,
                                             FRER_DIRECTION_STR, tmp)) {
                    return NULL;
                }
            } else {
                snprintf(message, MAX_STR_LEN, ERR_FRER_SEQ_GEN_LIST_DIRECTION_STR, fn_index);
                SRP_LOG_ERR(ERROR_MSG_FUN_AND_MSG, __func__, message);
                return NULL;
            }

            /* list sequence-generation, leaf-list stream-list */
            for (i = 0; i < max_streams; i++) {
                if (65535 == strlist[i]) {
                    break;
                }
                snprintf(tmp, MAX_STR_LEN, "%u", strlist[i]);
                if (EXIT_FAILURE == new_node(frer_seq_gen_entry, module,
                                             FRER_STREAM_LIST_STR, tmp)) {
                    return NULL;
                }
            }
        }
    }
    free(strlist);

    return frer_seq_gen_entry;
}

/**
 * @brief Creates a new leaf-list port-list inside list sequence-recovery from
 * ieee802-dot1cb-frer yang module.
 *
 * It is used for config data.
 *
 * @param[in]   parent          Pointer to struct lyd_node which is going to be
 *                              parent to created entry.
 * @param[in]   module          Main schema node structure representing YANG
 *                              module.
 * @param[in]   port            Poiner to struct port_hdl struct, that
 *                              represents opened port.
 * @param[in]   fn_index        Leaf index, key of sequence-recovery list.
 * @return      struct lyd_node* or NULL if element not created.
 */
struct lyd_node *leaf_list_port_list(struct lyd_node *parent,
                                     const struct lys_module *module,
                                     struct port_hdl *port, uint16_t fn_index)
{
    uint16_t val16 = 0;
    uint8_t i = 0;
    char tmp[MAX_STR_LEN] = "";

    SRP_LOG_DBG(DEBUG_MSG_FUN_CALLED, __func__);

    if (0 == tsn_frer_seqrec_get_ptmask(port, fn_index, &val16)) {
        for (i = 1; i < 16; i++) {
            if ((val16 & (1 << i))) {
                /* list sequence-recovery, leaf-list port-list */
                snprintf(tmp, MAX_STR_LEN, "sw0p%d", i + 1);
                if (EXIT_FAILURE == new_node(parent, module,
                                             FRER_PORT_LIST_STR, tmp)) {
                    return NULL;
                }
            }
        }
    } else {
        SRP_LOG_ERR(ERROR_MSG_FUN_AND_MSG, __func__, ERR_FRER_PORT_LIST_STR);
        return NULL;
    }

    return parent;
}

/**
 * @brief Creates a new leaf-list stream-list inside list sequence-recovery
 * from ieee802-dot1cb-frer yang module.
 *
 * It is used for config data.
 *
 * @param[in]   parent          Pointer to struct lyd_node which is going to be
 *                              parent to created entry.
 * @param[in]   module          Main schema node structure representing YANG
 *                              module.
 * @param[in]   port            Poiner to struct port_hdl struct, that
 *                              represents opened port.
 * @param[in]   fn_index        Leaf index, key of sequence-recovery list.
 * @return  struct lyd_node* or NULL if element not created.
 */
struct lyd_node *leaf_list_stream_list(struct lyd_node *parent,
                                       const struct lys_module *module,
                                       struct port_hdl* port, uint16_t fn_index)
{
    uint16_t *strlist = NULL;
    uint32_t max_streams = 0;
    uint32_t i = 0;
    char tmp[MAX_STR_LEN] = "";
    char message[MAX_STR_LEN] = "";

    SRP_LOG_DBG(DEBUG_MSG_FUN_CALLED, __func__);

    if (0 == tsn_sid_get_max_sup(port, &max_streams)) {
        strlist =  malloc(sizeof(*strlist) * max_streams);
        if (NULL == strlist) {
            SRP_LOG_ERR(ERROR_MSG_FUN_AND_MSG, __func__,
                        ERR_FRER_SEQ_REC_LIST_STREAM_LIST_ALLOCATE_STR);
            return NULL;
        }

        if (0 == tsn_frer_seqrec_fn2sid(port, fn_index, strlist, max_streams)) {
            for (i = 0; i < max_streams; i++) {
                if (strlist[i] == _SEQGEN_INVALID) {
                    break;
                }
                /* list sequence-recovery, leaf-list stream-list */
                snprintf(tmp, MAX_STR_LEN, "%d", strlist[i]);
                if (EXIT_FAILURE == new_node(parent, module, FRER_STREAM_LIST_STR, tmp)) {
                    return NULL;
                }
            }
            free(strlist);
        } else {
            snprintf(message, MAX_STR_LEN, ERR_FRER_GET_STREAM_LIST_STR, fn_index);
            SRP_LOG_ERR(ERROR_MSG_FUN_AND_MSG, __func__, message);
            return NULL;
        }
    } else {
        SRP_LOG_ERR(ERROR_MSG_FUN_AND_MSG, __func__, ERR_FRER_STREAM_LIST_STR);
        return NULL;
    }

    return parent;
}

/**
 * @brief Creates a new entry for sequence-recovery list inside container frer
 * inside ieee802-dot1cb-frer yang module.
 *
 * It is used for config data.
 *
 * @param[in]   parent          Pointer to struct lyd_node which is going to be
 *                              parent to created entry.
 * @param[in]   module          Main schema node structure representing YANG
 *                              module.
 * @param[in]   port            Poiner to struct port_hdl struct, that
 *                              represents opened port.
 * @param[in]   fn_index        Leaf index, key of sequence-recovery list.
 * @return      struct lyd_node* or NULL if element not created.
 */
struct lyd_node *new_sequence_recovery_list_entry(
        struct lyd_node *parent, const struct lys_module *module,
        struct port_hdl *port, uint16_t seqrec_index)
{
    struct lyd_node *frer_seqrec_fn_list = NULL;
    struct lyd_node *frer_seqrec_alg = NULL;
    struct lyd_node *tmp_node = NULL;
    char tmp[MAX_STR_LEN] = "";
    uint32_t val32 = 0;
    uint8_t val8 = 0;
    uint16_t val16 = 0;

    SRP_LOG_DBG(DEBUG_MSG_FUN_CALLED, __func__);

    /* new sequence-recovery container */
    if (EXIT_FAILURE == new_container(parent, &frer_seqrec_fn_list, module,
                                      FRER_SEQ_RECOVERY_LIST_STR)) {
        return NULL;
    }

    /* leaf index, list sequence-recovery */
    snprintf(tmp, MAX_STR_LEN, "%u", seqrec_index);
    if (EXIT_FAILURE == new_node(frer_seqrec_fn_list, module,
                                 FRER_INDEX_STR, tmp)) {
        return NULL;
    }

    /* leaf direction, list sequence-recovery (it has value "0", standard
     * deviations document seas so.
     */
    /* hard coded value */
    if (EXIT_FAILURE == new_node(frer_seqrec_fn_list, module,
                                 FRER_DIRECTION_STR, "false")) {
        return NULL;
    }

    /* container algorithm, list sequence-recovery */
    if (EXIT_FAILURE == new_container(frer_seqrec_fn_list, &frer_seqrec_alg, module,
                                      FRER_ALG_CONTAINER_STR)) {
        return NULL;
    }

    if (0 == tsn_frer_seqrec_get_alg(port, seqrec_index, &val32)) {
        if (val32 != _SEQREC_ALG_UNKOWN) {
            if (val32) {
                if (EXIT_FAILURE == new_container(frer_seqrec_alg, &tmp_node, module,
                                                  FRER_ALG_MATCH_STR)) {
                    return NULL;
                }
            } else {
                if (EXIT_FAILURE == new_container(frer_seqrec_alg, &tmp_node, module,
                                                  FRER_VECTOR_ALG_STR)) {
                    return NULL;
                }
            }
        }
    }

    /* leaf history-length, list sequence-recovery */
    if (0 == tsn_frer_seqrec_get_histlen(port, seqrec_index, &val8)) {
        if (val8) {
            snprintf(tmp, MAX_STR_LEN, "%u", val8);
            if (EXIT_FAILURE == new_node(frer_seqrec_fn_list, module,
                                         FRER_HISTORY_LENGTH_STR, tmp)) {
                return NULL;
            }
        }
    }

    /* leaf reset-timeout, list sequence-recovery */
    if (0 == tsn_frer_seqrec_get_recreset(port, seqrec_index, &val16)) {
        if (val32 != _SEQREC_INV_U16) {
            snprintf(tmp, MAX_STR_LEN, "%u", val16);
            if (EXIT_FAILURE == new_node(frer_seqrec_fn_list, module,
                                         FRER_RESET_TIMEOUT_STR, tmp)) {
                return NULL;
            }
        }
    }

    /* leaf take-no-sequence, list sequence-recovery */
    if (0 == tsn_frer_seqrec_get_takenoseq(port, seqrec_index, &val8)) {
        if (val8 != _SEQREC_INV_BOOL) {
            snprintf(tmp, MAX_STR_LEN, "%s", (val8 ? "true" : "false"));
            if (EXIT_FAILURE == new_node(frer_seqrec_fn_list, module,
                                         FRER_TAKE_NO_SEQ_STR, tmp)) {
                return NULL;
            }
        }
    }

    /* leaf individual-recovery, list sequence-recovery */
    if (0 == tsn_frer_seqrec_get_indrcv(port, seqrec_index, &val8)) {
        if (val8 != _SEQREC_INV_BOOL) {
            snprintf(tmp, MAX_STR_LEN, "%s", (val8 ? "true" : "false"));
            if (EXIT_FAILURE == new_node(frer_seqrec_fn_list, module,
                                         FRER_INVALID_RECOVERY_STR, tmp)) {
                return NULL;
            }
        }
    }

    /* leaf latent-error-detection, list sequence-recovery */
    if (0 == tsn_frer_seqrec_get_laterr(port, seqrec_index, &val8)) {
        if (val8 != _SEQREC_INV_BOOL) {
            snprintf(tmp, MAX_STR_LEN, "%s", (val8 ? "true" : "false"));
            if (EXIT_FAILURE == new_node(frer_seqrec_fn_list, module,
                                         FRER_LATENT_ERROR_DETECION_STR, tmp)) {
                return NULL;
            }
        }
    }

    if (NULL == leaf_list_port_list(frer_seqrec_fn_list, module,
                                    port, seqrec_index)) {
        return NULL;
    }

    if (NULL == leaf_list_stream_list(frer_seqrec_fn_list, module,
                                      port, seqrec_index)) {
        return NULL;
    }

    return frer_seqrec_fn_list;
}

/**
 * @brief Creates a new entry for list sequence-identification-list inside
 * ieee802-dot1cb-frer yang module.
 *
 * It is used for config data.
 *
 * @param[in]   parent          Pointer to struct lyd_node which is going to be
 *                              parent to created entry.
 * @param[in]   module          Main schema node structure representing YANG
 *                              module.
 * @param[in]   port            Poiner to struct port_hdl struct, that
 *                              represents opened port.
 * @param[in]   brdev           Poiner to struct port_hdl struct, that
 *                              represents opened bridge.
 * @param[in]   port_str        Poiner to char that represents name of the port.
 * @return      struct lyd_node* or NULL if element not created.
 */
struct lyd_node *new_sequence_idenitification_entry(
        struct lyd_node *parent, const struct lys_module *module,
        struct port_hdl *port, struct port_hdl *brdev, char *port_str)
{
    struct lyd_node *sequence_identification_list_entry = NULL;
    struct lyd_node *node_encapsulation = NULL;
    struct lyd_node *node_rtag = NULL;

    uint16_t *strlist = NULL;
    uint32_t max_streams = 0;
    uint32_t i = 0;
    uint32_t active;
    char str[] = "ENC_UNDEFINED";
    char tmp[MAX_STR_LEN] = "";

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    if (0 == tsn_sid_get_max_sup(brdev, &max_streams)) {
        strlist =  malloc(sizeof(*strlist) * max_streams);
        if (strlist) {
            if (0 == tsn_frer_seqenc_port2str (port, max_streams, strlist)) {
                /* if leaf active is found */
                if (0 == tsn_frer_seqenc_getactive(port, &active)) {
                    /* this function converts uint32_t to string (Active,
                     * Passive, Unknown)
                     */
                    seqenc_set_active_str(active, str);
                    /* Convert Active to true and Passive to false. If value of
                     * str is Unknown, then do not create leaf. Also if there
                     * is no leaf Active that means that entry does not exist,
                     * so it it not created.
                     */
                    if (EXIT_SUCCESS == translate_single_string_using_map(str, active_map)) {
                        /* new sequence-identification-list entry */
                        if (EXIT_FAILURE == new_container(parent, &sequence_identification_list_entry,
                                                          module, FRER_SEQ_ID_LIST_STR)) {
                            return NULL;
                        }

                        /* new encapsulation entry */
                        if (EXIT_FAILURE == new_container(sequence_identification_list_entry, &node_encapsulation,
                                                          module, FRER_ENC_STR)) {
                            return NULL;
                        }

                        /* for now, only r-tag encapsulation is supported */
                        if (EXIT_FAILURE == new_container(node_encapsulation, &node_rtag,
                                                          module, FRER_ENC_TYPE_R_TAG_STR)) {
                            return NULL;
                        }

                        /* leaf port, key of list sequence-identification-list */
                        if (EXIT_FAILURE == new_node(sequence_identification_list_entry,
                                                     module, FRER_PORT_STR, port_str)) {
                            return NULL;
                        }

                        /* leaf direction, key of list
                         * sequence-identification-list (default value is "1",
                         * because you can only create list entry if direction
                         * is "1" using tsntool)
                         */
                        if (EXIT_FAILURE == new_node(sequence_identification_list_entry,
                                                     module, FRER_DIRECTION_STR, "true")) {
                            return NULL;
                        }

                        /* leaf active */
                        if (EXIT_FAILURE == new_node(sequence_identification_list_entry,
                                                     module, FRER_ACTIVE_STR, str)) {
                            return NULL;
                        }

                        for (i = 0; i < max_streams; i++) {
                            if (strlist[i] != _SEQENC_INVALID) {
                                /* leaf-list stream-list */
                                snprintf(tmp, MAX_STR_LEN, "%u", strlist[i]);
                                if (EXIT_FAILURE == new_node(sequence_identification_list_entry,
                                                             module, FRER_STREAM_LIST_STR, tmp)) {
                                    return NULL;
                                }
                            }
                        }
                    } else {
                        return NULL;
                    }
                } else {
                    SRP_LOG_ERR(ERROR_MSG_FUN_AND_MSG, __func__, ERR_FRER_ACTIVE_GET_STR);
                    return NULL;
                }
            } else {
                SRP_LOG_ERR(ERROR_MSG_FUN_AND_MSG, __func__, ERR_FRER_PORT2STR_GET_STR);
                return NULL;
            }
        } else {
            SRP_LOG_ERR(ERROR_MSG_FUN_AND_MSG, __func__, ERR_FRER_STREAM_LIST_ALLOCATE_STR);
            return NULL;
        }
    } else {
        SRP_LOG_ERR(ERROR_MSG_FUN_AND_MSG, __func__, ERR_FRER_STREAM_LIST_STR);
        return NULL;
    }

    return sequence_identification_list_entry;
}

/**
 * @brief Creates a new container frer inside ieee802-dot1cb-frer yang module.
 *
 * This function is for configuration data from list component.
 *
 * @param[out]  parent          Pointer to struct lyd_node which is going to be
 *                              parent to created entry.
 * @param[in]   module          Main schema node structure representing YANG
 *                              module.
 * @param[in]   bridge_name     Contains the name of the bridge.
 * @return      struct lyd_node* or NULL if element not created.
 */
struct lyd_node *frame_replication_and_elimination(
        struct lyd_node *parent, const struct lys_module *module,
        char *bridge_name)
{
    struct port_hdl *port = NULL;
    struct port_hdl *brdev = NULL;
    uint32_t ret = 0u;
    uint32_t i = 0;
    uint16_t cnt = 0;
    uint16_t *fn_list = NULL;
    uint16_t *seqrec_fn_list = NULL;
    uint32_t numofports = 0;
    char *port_name = NULL;

    SRP_LOG_DBG(DEBUG_MSG_FUN_CALLED_WITH_ONE_PARAM, __func__, bridge_name);

    /* open br0 */
    brdev = tsn_port_open_by_name(bridge_name);
    if (NULL == brdev) {
        SRP_LOG_ERR(ERROR_MSG_MOD_FUNC_EL_MSG, FRER_MODULE_NAME_STR, __func__,
                    bridge_name, ERR_PORT_NOT_OPEN_STR);
        return NULL;
    }

    /* list sequence-generation */
    tsn_frer_seqgen_fn_cnt(brdev, &cnt);
    if (0 == cnt) {
        SRP_LOG_INF(ERROR_MSG_FUN_AND_MSG, __func__,
                    ERR_FRER_SEQ_GEN_LIST_EMPTY_STR);
    }

    /* get number of sequence-generation entries and malloc memory for them */
    if (0 != cnt) {
        fn_list = malloc(sizeof(uint16_t)*cnt);
        if (NULL != fn_list) {
            if (0 == tsn_frer_seqgen_list_fn(brdev, cnt, fn_list)) {
                for (i = 0; i < cnt; i++) {
                    if (NULL == new_sequence_generation_list_entry(parent, module,
                                                                   brdev, fn_list[i])) {
                        return NULL;
                    }
                }
                free(fn_list);
            } else {
                SRP_LOG_ERR(ERROR_MSG_MOD_FUNC_EL_MSG, FRER_MODULE_NAME_STR, __func__,
                            FRER_SEQ_GEN_LIST_STR, ERR_NOT_CREATED_ELEMENT_STR);
            }
        } else {
            SRP_LOG_ERR(ERROR_MSG_FUN_AND_MSG, __func__,
                        ERR_FRER_SEQ_GENERATION_LIST_ALLOCATE_STR);
        }
    }

    /* list sequence-recovery */
    cnt = 0;
    tsn_frer_seqrec_fn_cnt(brdev, &cnt);
    if (0 == cnt) {
        SRP_LOG_INF(ERROR_MSG_FUN_AND_MSG, __func__,
                    ERR_FRER_SEQ_REC_LIST_EMPTY_STR);
    }

    /* get number of sequence-recovery entries and malloc memory for
     * sequence-recovery list
     */
    if (0 != cnt) {
        seqrec_fn_list = malloc(sizeof(uint16_t) * cnt);
        if (NULL != seqrec_fn_list) {
            if (0 == tsn_frer_seqrec_list_fn(brdev, cnt, seqrec_fn_list)) {
                for (i = 0; i < cnt; i++) {
                    if (NULL == new_sequence_recovery_list_entry(parent, module,
                                                                 brdev, seqrec_fn_list[i])) {
                        return NULL;
                    }
                }
            } else {
                SRP_LOG_ERR(ERROR_MSG_MOD_FUNC_EL_MSG, FRER_MODULE_NAME_STR, __func__,
                            FRER_SEQ_RECOVERY_LIST_STR, ERR_NOT_CREATED_ELEMENT_STR);
            }
            free(seqrec_fn_list);
        } else {
            SRP_LOG_ERR(ERROR_MSG_FUN_AND_MSG, __func__,
                        ERR_FRER_SEQ_RECOVERY_LIST_ALLOCATE_STR);
        }
    }

    i = 0;
    /* list sequence-identification-list */
    ret = tsn_frer_get_max_ports(brdev, &numofports);
    if (ret) {
        SRP_LOG_ERR(ERROR_MSG_FUN_AND_MSG, __func__,
                    ERR_FRER_SEQ_IDENT_LIST_SEQ_ENC_DEC_STR);
    }

    for (i = 1; i <= numofports; i++) {
        port_name = tsn_get_name_by_id(brdev, i);
        port = tsn_port_open_by_name(port_name);
        if (NULL == port) {
            SRP_LOG_ERR(ERROR_MSG_MOD_FUNC_EL_MSG, FRER_MODULE_NAME_STR, __func__,
                        port_name, ERR_PORT_NOT_OPEN_STR);
            return NULL;
        }

        if (NULL == new_sequence_idenitification_entry(parent, module,
                                                       port, brdev, port_name)) {
            SRP_LOG_INF(ERROR_MSG_FUN_AND_MSG, __func__,
                        ERR_FRER_SEQ_IDENT_LIST_EMPTY_STR);
        }

        tsn_port_close(port);
    }

    /* close bridge br0 */
    tsn_port_close(brdev);

    return parent;
}

/**
 * @brief Creates a startup datastore for ieee802-dot1cb-frer module. Function
 * is going to be called once, due to sysrepo-plugind startup.
 *
 * @param[in]   session         Sysrepo session that can be used for any API
 *                              calls needed for plugin cleanup (mainly for
 *                              unsubscribing of subscriptions initialized in
 *                              sr_plugin_init_cb).
 * @return      EXIT_SUCCESS or EXIT_FAILURE.
 */
static int frer_fill_startup_datastore(sr_session_ctx_t *session)
{
    const struct ly_ctx *ctx = NULL;
    const struct lys_module *module = NULL;
    struct lyd_node *root = NULL;
    char bridge_names[MAX_NUM_OF_INTERFACES][MAX_STR_LEN]= {0};
    int fd = -1;
    char *path = NULL;

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    /* this means that startup is empty */
    if (0 == sr_path_check_startup_done_file(FRER_MODULE_NAME_STR, &path)) {
        ctx = sr_get_context(sr_session_get_connection(session));
        if (NULL == ctx) {
            SRP_LOG_ERR(ERROR_MSG_FUN_AND_MSG, __func__,
                        ERR_SESSION_CTX_FAILED_STR);
            free(path);
            return EXIT_FAILURE;
        }

        /* create ieee802-dot1cb-frer. root element (container frer) */
        root = lyd_new_path(NULL, ctx, "/ieee802-dot1cb-frer:frer", NULL, 0, 0);
        if (NULL == root) {
            SRP_LOG_ERR(ERROR_MSG_FUN_AND_MSG, __func__,
                        ERR_NOT_CREATED_ROOT_ELEMENT_STR);
            return EXIT_FAILURE;
        }

        if (EXIT_FAILURE == get_bridge_names(bridge_names, MAX_NUM_OF_INTERFACES,
                                             MAX_STR_LEN)) {
            SRP_LOG_ERR(ERR_BRIDE_NAMES_FAILED_STR, __func__);
            return EXIT_SUCCESS;
        }

        /* container frer */
        if (NULL == frame_replication_and_elimination(root, module, bridge_names[0])) {
            SRP_LOG_WRN(ERROR_MSG_MOD_FUNC_MSG, FRER_MODULE_NAME_STR, __func__,
                        ERR_MSG_EMPTY_STARTUP_DS_STR);
            return EXIT_FAILURE;
        }

        /******** NOTE ******/
        /* The startup to running datastore copy is done before the plugin is started.
         * So, here we will replace the current startup and running datastores with the
         * subtree called 'root' in this function.
         * */

        /* switch to running datastore */
        if (SR_ERR_OK != sr_session_switch_ds(session, SR_DS_RUNNING)) {
            SRP_LOG_ERR(ERROR_MSG_FUN_AND_MSG, __func__, ERR_SWITCH_DATASTORE_FAILED_STR);
            free(path);
            return EXIT_FAILURE;
        }

        /* Replace current running configuration with created 'root' subtree */
        if (SR_ERR_OK != sr_replace_config(session, FRER_MODULE_NAME_STR, root, 0, 0)) {
            SRP_LOG_ERR(ERROR_MSG_FUN_AND_MSG, __func__, ERR_REPLACE_CONFIG_FAILED_STR);
            free(path);
            return EXIT_FAILURE;
        }

        /* switch back to startup datastore */
        if (SR_ERR_OK != sr_session_switch_ds(session, SR_DS_STARTUP)) {
            SRP_LOG_ERR(ERROR_MSG_FUN_AND_MSG, __func__, ERR_SWITCH_DATASTORE_FAILED_STR);
            free(path);
            return EXIT_FAILURE;
        }

        /* copy config from running to startup datastore */
        if (SR_ERR_OK != sr_copy_config(session, FRER_MODULE_NAME_STR, SR_DS_RUNNING, 0, 0)) {
            SRP_LOG_ERR(ERROR_MSG_FUN_AND_MSG, __func__, ERR_COPY_DATASTORE_FAILED_STR);
            free(path);
            return EXIT_FAILURE;
        }

        /* switch back to running datastore */
        if (SR_ERR_OK != sr_session_switch_ds(session, SR_DS_RUNNING)) {
            SRP_LOG_ERR(ERROR_MSG_FUN_AND_MSG, __func__, ERR_SWITCH_DATASTORE_FAILED_STR);
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
 * @brief This function checks if port with deleted stream-list or port-list
 * exists on board inside sequence-recovery list.
 *
 * @param[in]   session         Implicit session (do not stop) with information
 *                              about the changed data (retrieved by
 *                              sr_get_changes_iter) the event originator
 *                              session IDs.
 * @param[in]   node            Pointer to sr_val_t structure representing node
 *                              that is changed.
 * @return      Error code (EXIT_SUCCESS on success).
 */
int check_sequence_recovery_list(sr_session_ctx_t *session, sr_val_t *node)
{
    sr_xpath_ctx_t st = {0};
    char *bridge_name = "br0";
    char fn_index[MAX_STR_LEN] = "";
    struct port_hdl* brdev = NULL;
    uint16_t cnt = 0;
    uint16_t *seqrec_fn_list = NULL;
    uint16_t fnidx = 0;
    int i = 0;

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    /* get index value, key of list sequence-recovery */
    if (EXIT_SUCCESS != get_key_value(session, (char*)node->xpath,
                                      FRER_SEQ_RECOVERY_LIST_STR, FRER_INDEX_STR,
                                      &st, fn_index)) {
        SRP_LOG_ERR("%s: %s (%s)", __func__,
                    ERR_MISSING_ELEMENT_STR, FRER_INDEX_STR);
        return EXIT_FAILURE;
    }
    fnidx = strtoul(fn_index, NULL, 10);

    /* open br0 */
    brdev = tsn_port_open_by_name(bridge_name);
    if (NULL == brdev) {
        SRP_LOG_ERR(ERROR_MSG_MOD_FUNC_EL_MSG, FRER_MODULE_NAME_STR, __func__,
                    bridge_name, ERR_PORT_NOT_OPEN_STR);
        return EXIT_FAILURE;
    }

    /* list sequence-recovery */
    cnt = 0;
    tsn_frer_seqrec_fn_cnt(brdev, &cnt);
    if (0 == cnt) {
        SRP_LOG_INF(ERROR_MSG_FUN_AND_MSG, __func__,
                    ERR_FRER_SEQ_REC_LIST_EMPTY_STR);
        tsn_port_close(brdev);
        return EXIT_FAILURE;
    }

    /* get number of sequence-recovery entries and malloc memory for
     * sequence-recovery list
     */
    if (0 != cnt) {
        seqrec_fn_list = malloc(sizeof(uint16_t) * cnt);
        if (NULL != seqrec_fn_list) {
            if (0 == tsn_frer_seqrec_list_fn(brdev, cnt, seqrec_fn_list)) {
                for (i = 0; i < cnt; i++) {
                    /* check if entry with fnidx exist on board */
                    if (seqrec_fn_list[i] == fnidx) {
                        tsn_port_close(brdev);
                        free(seqrec_fn_list);
                        return EXIT_SUCCESS;
                    }
                }
            } else {
                 SRP_LOG_ERR(ERROR_MSG_MOD_FUNC_EL_MSG, FRER_MODULE_NAME_STR, __func__,
                             FRER_SEQ_RECOVERY_LIST_STR, "failed to get list");
            }
            free(seqrec_fn_list);
        } else {
             SRP_LOG_ERR(ERROR_MSG_FUN_AND_MSG, __func__,
                         ERR_FRER_SEQ_RECOVERY_LIST_ALLOCATE_STR);
        }
    }

    tsn_port_close(brdev);

    return EXIT_FAILURE;
}

/**
 * @brief This function checks if port with deleted stream-list exists on board
 * inside sequence-generation list.
 *
 * @param[in]   session         Implicit session (do not stop) with information
 *                              about the changed data (retrieved by
 *                              sr_get_changes_iter) the event originator
 *                              session IDs.
 * @param[in]   node            Pointer to sr_val_t structure representing node
 *                              that is changed.
 * @return      Error code (EXIT_SUCCESS on success).
 */
int check_sequence_generation_list(sr_session_ctx_t *session, sr_val_t *node)
{
    sr_xpath_ctx_t st = {0};
    char *bridge_name = "br0";
    char fn_index[MAX_STR_LEN] = "";
    struct port_hdl* brdev = NULL;
    uint16_t cnt = 0;
    uint16_t *fn_list = NULL;
    uint16_t fnidx = 0;
    int i = 0;

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    /* get index value, key of list frer-seg-gen-entry list */
    if (EXIT_SUCCESS != get_key_value(session, (char*)node->xpath, FRER_SEQ_GEN_LIST_STR,
                                      FRER_INDEX_STR, &st, fn_index)) {
        SRP_LOG_ERR("%s: %s (%s)", __func__,
                    ERR_MISSING_ELEMENT_STR, FRER_INDEX_STR);
        return EXIT_FAILURE;
    }
    fnidx = strtoul(fn_index, NULL, 10);

    /* open br0 */
    brdev = tsn_port_open_by_name(bridge_name);
    if (NULL == brdev) {
        SRP_LOG_ERR(ERROR_MSG_MOD_FUNC_EL_MSG, FRER_MODULE_NAME_STR, __func__,
                    bridge_name, ERR_PORT_NOT_OPEN_STR);
        return EXIT_FAILURE;
    }

    /* list frer-seg-gen-entry */
    tsn_frer_seqgen_fn_cnt(brdev, &cnt);
    if (0 == cnt) {
        SRP_LOG_INF(ERROR_MSG_FUN_AND_MSG, __func__,
                    ERR_FRER_SEQ_GEN_LIST_EMPTY_STR);
        tsn_port_close(brdev);
        return EXIT_FAILURE;
    }

    /* get number of frer-seg-gen-entries and malloc memory for
     * frer-seg-gen-entry list
     */
    if (0 != cnt) {
        fn_list = malloc(sizeof(uint16_t)*cnt);
        if (NULL != fn_list) {
            if (0 == tsn_frer_seqgen_list_fn(brdev, cnt, fn_list)) {
                for (i = 0; i < cnt; i++) {
                    /* check if entry with fnidx exist on board */
                    if (fn_list[i] == fnidx) {
                        tsn_port_close(brdev);
                        free(fn_list);
                        return EXIT_SUCCESS;
                    }
                }
                free(fn_list);
            } else {
                SRP_LOG_ERR(ERROR_MSG_MOD_FUNC_EL_MSG, FRER_MODULE_NAME_STR, __func__,
                            FRER_SEQ_GEN_LIST_STR, "failed to get list");
            }
        } else {
            SRP_LOG_ERR(ERROR_MSG_FUN_AND_MSG, __func__,
                        ERR_FRER_SEQ_GENERATION_LIST_ALLOCATE_STR);
        }
    }

    tsn_port_close(brdev);

    return EXIT_FAILURE;
}

/**
 * @brief This function checks if there is more than one leaf-list port-list
 * inside sequence-recovery-entry.
 *
 * @param[in]   session         Implicit session (do not stop) with information
 *                              about the changed data (retrieved by
 *                              sr_get_changes_iter) the event originator
 *                              session IDs.
 * @param[in]   node            Pointer to sr_val_t structure representing node
 *                              that is changed.
 * @return      Error code (EXIT_SUCCESS on success).
 */
int check_existing_port_lists(sr_session_ctx_t *session, sr_val_t *node)
{
    sr_xpath_ctx_t st = {0};
    char fn_index[MAX_STR_LEN] = "";
    char path[MAX_STR_LEN] = "";
    sr_val_t *port_list_entries = NULL;
    size_t val_cnt = 0;
    int i = 0;
    int br = 0;

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    /* get index value, key of list sequence-recovery */
    if (EXIT_SUCCESS != get_key_value(NULL, (char*)node->xpath, FRER_SEQ_RECOVERY_LIST_STR,
                                      FRER_INDEX_STR, &st, fn_index)) {
        SRP_LOG_ERR("%s: %s (%s)", __func__,
                    ERR_MISSING_ELEMENT_STR, FRER_INDEX_STR);
        return EXIT_FAILURE;
    }

    fill_xpath(path, FRER_SEQ_REC_LIST_PORT_LIST_XPATH, fn_index);
    if (SR_ERR_OK != sr_get_items(session, path, 0, 0, &port_list_entries, &val_cnt)) {
        SRP_LOG_ERR(ERR_FAILED_GET_OBJ_STR, path);
        return EXIT_FAILURE;
    }

    for (i = 0; i < (int)val_cnt; i++) {
        if (strstr(port_list_entries[i].xpath, FRER_PORT_LIST_STR)) {
            br++;
        }
    }

    sr_free_values(port_list_entries, val_cnt);

    /* If counter is different from 0, then there are more than one port-list
     * leaf-lists.
     */
    if (0 != br) {
        return EXIT_FAILURE;
    }

    /* If there is no port-list entries left, return EXIT_SUCCESS. */
    return EXIT_SUCCESS;
}

/**
 * @brief Callback to be called by the event of changing content of
 * sequence-recovery ieee802-dot1cb-frer yang module.
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
int frer_sequence_recovery_list_cb(sr_session_ctx_t *session,
                                   const char *module_name, const char *xpath,
                                   sr_event_t event, uint32_t request_id,
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
    char *bridge_name = "br0";
    char err_msg[MAX_STR_LEN] = "";
    struct port_hdl *port = NULL;
    uint32_t ret = 0u;
    uint16_t fnidx = 0;

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    if ((SR_EV_DONE == event) || (0 == plugin_init)) {
        SRP_LOG_DBG(DEBUG_MSG_WITH_TWO_PARAM, DBG_APPLYING_CHANGES_MSG, __func__);
        return SR_ERR_OK;
    }

    rc = sr_get_changes_iter(session, xpath, &iter);
    if (SR_ERR_OK != rc) {
        return rc;
    }

    while (SR_ERR_OK == sr_get_change_next(session, iter, &op, &old_value, &new_value)) {
        node = (op == SR_OP_DELETED) ? old_value : new_value;

        /* if sequence-recovery-list entry is added or deleted */
        if ((SR_OP_CREATED == op) ||
            (SR_OP_DELETED == op) ||
            (SR_OP_MODIFIED == op)) {
            /* open bridge */
            port = tsn_port_open_by_name(bridge_name);
            if (NULL == port) {
                snprintf(err_msg, MAX_STR_LEN, ERROR_MSG_FUN_AND_MSG,
                         ERR_PORT_NOT_OPEN_STR, bridge_name);
                sr_set_error(session, node->xpath, err_msg);
                return SR_ERR_OPERATION_FAILED;
            }

            fnidx = node->data.uint32_val;

            if ((SR_OP_CREATED == op) || (op == SR_OP_MODIFIED)) {
                ret = tsn_frer_seqrec_create(port, fnidx);
                if (ret) {
                    sr_set_error(session, node->xpath,
                                 ERR_FRER_SEQUENCE_RECOVERY_LIST_ADD_STR, node->xpath);
                    tsn_port_close(port);
                    return SR_ERR_OPERATION_FAILED;
                }
            }
            /* This additional check is added because of thread. If port-list is deleted, then entire
             * entry is deleted from board. If entry exist on board then delete it, otherwise do nothing. */
            if ((SR_OP_DELETED == op) &&
                (EXIT_SUCCESS == check_sequence_recovery_list(session, node))) {
                ret = tsn_frer_seqrec_delfn(port, fnidx);
                if (ret) {
                    sr_set_error(session, node->xpath,
                                 ERR_FRER_SEQUENCE_RECOVERY_LIST_DELETE_STR,
                                 node->xpath);
                    tsn_port_close(port);
                    return SR_ERR_OPERATION_FAILED;
                }
            }
            tsn_port_close(port);
        }
    }

    sr_free_val(new_value);
    sr_free_val(old_value);
    sr_free_change_iter(iter);
    node = NULL;

    return SR_ERR_OK;
}

/**
 * @brief Callback to be called by the event of changing content of leafs
 * inside sequence-recovery-list inside ieee802-dot1cb-frer yang module.
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
int frer_sequence_recovery_list_element_cb(sr_session_ctx_t *session,
                                           const char *module_name,
                                           const char *xpath, sr_event_t event,
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
    sr_val_t *tmp_node = NULL;
    char *bridge_name = "br0";
    char tmp[MAX_STR_LEN] = "";
    char fn_index[MAX_STR_LEN] = "";
    char err_msg[MAX_STR_LEN] = "";
    struct port_hdl *port = NULL;
    int16_t ch_alg_nodes_cnt = 0;
    uint32_t algorithm = 0u;
    uint32_t ret = 0u;
    uint16_t fnidx = 0;
    uint16_t reset_msec = 0;
    uint8_t history_length = 0;
    uint8_t direction = 0;
    uint8_t take_no_sequence = 0;
    uint8_t individual_recovery = 0;
    uint8_t latent_error_detection = 0;

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

        /* if sequence-recovery-list element is added or modified */
        if ((SR_OP_CREATED == op) || (SR_OP_MODIFIED == op)) {
            /* get index value, key of list sequence-recovery-list */
            if (EXIT_SUCCESS != get_key_value(session, (char*)node->xpath, FRER_SEQ_RECOVERY_LIST_STR,
                                              FRER_INDEX_STR, &st, fn_index)) {
                SRP_LOG_ERR("%s: %s (%s)", __func__,
                            ERR_MISSING_ELEMENT_STR, FRER_INDEX_STR);
                return SR_ERR_OPERATION_FAILED;
            }
            fnidx = strtoul(fn_index, NULL, 10);

            /* open bridge */
            port = tsn_port_open_by_name(bridge_name);
            if (NULL == port) {
                snprintf(err_msg, MAX_STR_LEN, ERROR_MSG_FUN_AND_MSG,
                         ERR_PORT_NOT_OPEN_STR, bridge_name);
                sr_set_error(session, node->xpath, err_msg);
                return SR_ERR_OPERATION_FAILED;
            }

            /* if leaf algorithm was changed */
            if (true == sr_xpath_node_name_eq(node->xpath, FRER_ALG_MATCH_STR) ||
                true == sr_xpath_node_name_eq(node->xpath, FRER_ALG_VECTOR_STR)) {
                snprintf(tmp, MAX_STR_LEN, "%s", sr_xpath_node_name(node->xpath));
                if (EXIT_FAILURE == translate_single_string_using_map(tmp, algorithm_map)) {
                    sr_set_error(session, node->xpath, ERR_MSG_INVALID_VALUE);
                    tsn_port_close(port);
                    return SR_ERR_OPERATION_FAILED;
                }

                algorithm = strtoul(tmp, NULL, 10);
                ret = tsn_frer_seqrec_set_alg(port, fnidx, algorithm);
                if (ret) {
                    sr_set_error(session, node->xpath,
                                 ERR_FRER_SEQ_REC_ENTRY_DATA_STR, node->xpath);
                    tsn_port_close(port);
                    return SR_ERR_OPERATION_FAILED;
                }

                ch_alg_nodes_cnt++;
            }

            /* if leaf direction was changed */
            if (true == sr_xpath_node_name_eq(node->xpath, FRER_DIRECTION_STR)) {
                direction = node->data.bool_val;
                ret = tsn_frer_seqrec_set_dir(port, fnidx, direction);
                if (ret) {
                    sr_set_error(session, node->xpath,
                                 ERR_FRER_SEQ_REC_ENTRY_DATA_STR, node->xpath);
                    tsn_port_close(port);
                    return SR_ERR_OPERATION_FAILED;
                }
            }

            /* if leaf history-length was changed */
            if (true == sr_xpath_node_name_eq(node->xpath, FRER_HISTORY_LENGTH_STR)) {
                history_length = node->data.int32_val;
                ret = tsn_frer_seqrec_set_histlen(port, fnidx, history_length);
                if (ret) {
                    sr_set_error(session, node->xpath,
                                 ERR_FRER_SEQ_REC_ENTRY_DATA_STR, node->xpath);
                    tsn_port_close(port);
                    return SR_ERR_OPERATION_FAILED;
                }
            }

            /* if reset-timeout was changed */
            if (true == sr_xpath_node_name_eq(node->xpath, FRER_RESET_TIMEOUT_STR)) {
                reset_msec = node->data.uint32_val;
                ret = tsn_frer_seqrec_set_recreset(port, fnidx, reset_msec);
                if (ret) {
                    sr_set_error(session, node->xpath,
                                 ERR_FRER_SEQ_REC_ENTRY_DATA_STR, node->xpath);
                    tsn_port_close(port);
                    return SR_ERR_OPERATION_FAILED;
                }
            }

            /* if take-no-sequence was changed */
            if (true == sr_xpath_node_name_eq(node->xpath, FRER_TAKE_NO_SEQ_STR)) {
                take_no_sequence = node->data.bool_val;
                ret = tsn_frer_seqrec_set_takenoseq(port, fnidx, take_no_sequence);
                if (ret) {
                    sr_set_error(session, node->xpath,
                                 ERR_FRER_SEQ_REC_ENTRY_DATA_STR, node->xpath);
                    tsn_port_close(port);
                    return SR_ERR_OPERATION_FAILED;
                }
            }

            /* if individual-recovery changed */
            if (true == sr_xpath_node_name_eq(node->xpath, FRER_INVALID_RECOVERY_STR)) {
                individual_recovery = node->data.bool_val;
                ret = tsn_frer_seqrec_set_indrcv(port, fnidx, individual_recovery);
                if (ret) {
                    sr_set_error(session, node->xpath,
                                 ERR_FRER_SEQ_REC_ENTRY_DATA_STR, node->xpath);
                    tsn_port_close(port);
                    return SR_ERR_OPERATION_FAILED;
                }
            }

            /* if latent-error-detection changed */
            if (true == sr_xpath_node_name_eq(node->xpath, FRER_LATENT_ERROR_DETECION_STR)) {
                latent_error_detection = node->data.bool_val;
                ret = tsn_frer_seqrec_set_laterr(port, fnidx, latent_error_detection);
                if (ret) {
                    sr_set_error(session, node->xpath,
                                 ERR_FRER_SEQ_REC_ENTRY_DATA_STR, node->xpath);
                    tsn_port_close(port);
                    return SR_ERR_OPERATION_FAILED;
                }
            }
            tsn_port_close(port);
        }

        if (SR_OP_DELETED == op) {
            if (true == sr_xpath_node_name_eq(node->xpath, FRER_ALG_MATCH_STR) ||
                true == sr_xpath_node_name_eq(node->xpath, FRER_ALG_VECTOR_STR)) {
                /* get port value, key of list sequence-identification-list list */
                if (EXIT_SUCCESS != get_key_value(session, node->xpath,
                                                  FRER_SEQ_RECOVERY_LIST_STR,
                                                  FRER_INDEX_STR, &st, fn_index)) {
                    SRP_LOG_ERR("%s: %s (%s)", __func__,
                                ERR_MISSING_ELEMENT_STR, FRER_PORT_STR);
                    return SR_ERR_OPERATION_FAILED;
                }

                fill_xpath(tmp, FRER_SEQ_REC_LIST_XPATH, fn_index);
                if (SR_ERR_OK == sr_get_item(session, tmp, 0, &tmp_node)) {
                    ch_alg_nodes_cnt--;
                }

            }
        }

        /* SR_OP_DELETED operation is allowed and there is nothing to be done.
         * SR_OP_DELETED operation is allowed because entry of
         * sequence-recovery-list cat be fully deleted.
         */
    }

    if (0 > ch_alg_nodes_cnt) {
        tsn_port_close(port);
        SRP_LOG_ERR("%s: %s (%s)", __func__,
                    ERR_FRER_DEL_SEQ_REC_ENTRY_DATA_STR, FRER_ALG_CONTAINER_STR);
        sr_set_error(session, xpath, ERR_FRER_DEL_SEQ_REC_ENTRY_DATA_STR,
                     FRER_ALG_STR);
        return SR_ERR_OPERATION_FAILED;
    }

    sr_free_val(new_value);
    sr_free_val(old_value);
    sr_free_change_iter(iter);
    node = NULL;

    return SR_ERR_OK;
}

/**
 * @brief Callback to be called by the event of changing content of leaf-lists
 * port-list and stream inside sequence-recovery-list inside
 * ieee802-dot1cb-frer yang module.
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
int frer_sequence_recovery_leaf_list_cb(sr_session_ctx_t *session,
                                        const char *module_name,
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
    char *bridge_name = "br0";
    char port_name[MAX_STR_LEN] = "";
    char fn_index[MAX_STR_LEN] = "";
    char err_msg[MAX_STR_LEN] = "";
    struct port_hdl *bridge = NULL;
    struct port_hdl *port = NULL;
    uint32_t ret = 0u;
    uint16_t fnidx = 0;
    uint16_t stream_list = 0;

    /* Pointer to an unsigned integer value that returns the thread id of the
     * thread created.
     */
    // pthread_t thread;

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    if (0 == plugin_init) {
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

        /* get index value, key of list sequence-recovery-list */
        if (EXIT_SUCCESS != get_key_value(session, (char*)node->xpath,
                                          FRER_SEQ_RECOVERY_LIST_STR,
                                          FRER_INDEX_STR, &st, fn_index)) {
            SRP_LOG_ERR("%s: %s (%s)", __func__,
                        ERR_MISSING_ELEMENT_STR, FRER_INDEX_STR);
            return SR_ERR_OPERATION_FAILED;
        }

        fnidx = strtoul(fn_index, NULL, 10);

        /* if leaf-list is added, deleted or modified */
        if ((SR_OP_CREATED == op) ||
            (SR_OP_MODIFIED == op) ||
            (SR_OP_DELETED == op)) {
            /* if leaf-list stream was changed */
            if (strstr(node->xpath, FRER_STREAM_LIST_STR)) {
                stream_list = node->data.uint64_val;

                /* open bridge */
                bridge = tsn_port_open_by_name(bridge_name);
                if (NULL == bridge) {
                    snprintf(err_msg, MAX_STR_LEN, ERROR_MSG_FUN_AND_MSG,
                             ERR_PORT_NOT_OPEN_STR, bridge_name);
                    sr_set_error(session, node->xpath, err_msg);
                    return SR_ERR_OPERATION_FAILED;
                }

                if ((SR_OP_CREATED == op) || (SR_OP_MODIFIED == op)) {
                    if (event != SR_EV_DONE) {
                        ret = tsn_frer_seqrec_addstr(bridge, fnidx, stream_list);
                        if (ret) {
                            sr_set_error(session, node->xpath,
                                         ERR_FRER_ADD_STREAM_LIST_STR,
                                         node->xpath);
                            tsn_port_close(bridge);
                            return SR_ERR_OPERATION_FAILED;
                        }
                    }
                }

                if ((SR_OP_DELETED == op) &&
                    (EXIT_SUCCESS == check_sequence_recovery_list(session, node))) {
                    if (event != SR_EV_DONE) {
                        ret = tsn_frer_seqrec_delstr(bridge, fnidx, stream_list);
                        if (ret) {
                            sr_set_error(session, node->xpath,
                                         ERR_FRER_DEL_SEQ_REC_ENTRY_DATA_STR,
                                         node->xpath);
                            tsn_port_close(bridge);
                            return SR_ERR_OPERATION_FAILED;
                        }
                    }
                }
                tsn_port_close(bridge);
            }

            /* if leaf-list port-list was changed */
            if (strstr(node->xpath, FRER_PORT_LIST_STR)) {
                snprintf(port_name, MAX_STR_LEN, "%s", node->data.string_val);

                /* open port */
                port = tsn_port_open_by_name(port_name);
                if (NULL == port) {
                    snprintf(err_msg, MAX_STR_LEN, ERROR_MSG_FUN_AND_MSG,
                             ERR_PORT_NOT_OPEN_STR, port_name);
                    sr_set_error(session, node->xpath, err_msg);
                    return SR_ERR_OPERATION_FAILED;
                }

                if ((SR_OP_CREATED == op) || (SR_OP_MODIFIED == op)) {
                    if (event != SR_EV_DONE) {
                        ret = tsn_frer_seqrec_addport(port, fnidx);
                        if (ret) {
                            sr_set_error(session, node->xpath,
                                         ERR_FRER_ADD_PORT_LIST_STR,
                                         node->xpath);
                            tsn_port_close(bridge);
                            return SR_ERR_OPERATION_FAILED;
                        }
                    }
                }

                /* No need to check if entry exists, because if operation is delete and event CHANGE then, entry will be deleted from board.
                 * So when event is DONE check would not pass, because entry does not exist. */
                if (SR_OP_DELETED == op) {
                    if ((event != SR_EV_DONE) &&
                        (EXIT_SUCCESS == check_sequence_recovery_list(session, node))) {
                        ret = tsn_frer_seqrec_delport(port, fnidx);
                        if (ret) {
                            sr_set_error(session, node->xpath,
                                         ERR_FRER_DEL_SEQ_REC_ENTRY_DATA_STR,
                                         node->xpath);
                            tsn_port_close(bridge);
                            return SR_ERR_OPERATION_FAILED;
                        }
                    }

#if 0
                    /* create thread to delete entire entry */
                    if ((event == SR_EV_DONE) &&
                        (EXIT_SUCCESS == check_existing_port_lists(session, old_value))) {
                        rc = pthread_create((pthread_t *)&thread, NULL,
                                            port_list_thread, old_value);
                        if (rc != SR_ERR_OK) {
                            sr_set_error(session, node->xpath,
                                         ERR_CREATING_THREAD_FAILED_XPATH_STR,
                                         node->xpath);
                            return SR_ERR_OPERATION_FAILED;
                        }
                    }
#endif
                }
                tsn_port_close(port);
            }
        }
    }

    sr_free_val(new_value);
    sr_free_change_iter(iter);
    node = NULL;

    return SR_ERR_OK;
}

/**
 * @brief Callback to be called by the event of changing content
 * sequence-generation list inside ieee802-dot1cb-frer yang module.
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
int frer_sequence_generation_list_cb(sr_session_ctx_t *session,
                                     const char *module_name, const char *xpath,
                                     sr_event_t event, uint32_t request_id,
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
    sr_val_t *direction = NULL;
    char *bridge_name = "br0";
    char path[MAX_STR_LEN] = "";
    char fn_index[MAX_STR_LEN] = "";
    struct port_hdl *brdev = NULL;
    uint32_t ret = 0u;
    uint16_t fnidx = 0;
    uint8_t dir = 2;
    /* Put direction default value to 2, because possible values are 1 and 0,
     * so callback should fail id direction has wrong value.
     */

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

        /* get index value, key of sequence-generation */
        if (EXIT_SUCCESS != get_key_value(session, (char*)node->xpath,
                                          FRER_SEQ_GEN_LIST_STR, FRER_INDEX_STR,
                                          &st, fn_index)) {
            SRP_LOG_ERR("%s: %s (%s)", __func__,
                        ERR_MISSING_ELEMENT_STR, FRER_INDEX_STR);
            return SR_ERR_OPERATION_FAILED;
        }
        fnidx = strtoul(fn_index, NULL, 10);

        if ((SR_OP_CREATED == op) || (SR_OP_MODIFIED == op)) {
            /* open bridge */
            brdev = tsn_port_open_by_name(bridge_name);
            if (NULL == brdev) {
                sr_set_error(session, node->xpath,
                             ERR_PORT_NOT_OPEN_XPATH_STR, bridge_name);
                return SR_ERR_OPERATION_FAILED;
            }

            /* get leaf direction */
            fill_xpath(path, FRER_SEQ_GEN_LIST_LEAF_DIRECTION_XPATH, fn_index);
            if (SR_ERR_OK != sr_get_item(session, path, 0, &direction)) {
                sr_set_error(session, path, ERR_FAILED_GET_OBJ_STR, path);
                return SR_ERR_OPERATION_FAILED;
            }

            dir = direction->data.bool_val;

            ret = tsn_frer_seqgen_create (brdev, fnidx);

            if (!ret) {
                ret = tsn_frer_seqgen_add_dir(brdev, fnidx, dir);
                if (ret) {
                    tsn_frer_seqgen_del_fn(brdev, fnidx);
                }
            }
            if (ret) {
                sr_set_error(session, node->xpath,
                             ERR_FRER_SEQ_GEN_LIST_CREATE_STR, node->xpath);
                tsn_port_close(brdev);
                return SR_ERR_OPERATION_FAILED;
            }
            tsn_port_close(brdev);
        }
        /* This additional check is added because of thread. If stream is
         * deleted, then entire entry is deleted from board. If entry exist on
         * board then delete it, otherwise do nothing.
         */
        if ((SR_OP_DELETED == op) &&
            (EXIT_SUCCESS == check_sequence_generation_list(session, node))) {
            /* open bridge */
            brdev = tsn_port_open_by_name(bridge_name);
            if (NULL == brdev) {
                sr_set_error(session, node->xpath,
                             ERR_PORT_NOT_OPEN_XPATH_STR, bridge_name);
                return SR_ERR_OPERATION_FAILED;
            }

            ret = tsn_frer_seqgen_del_fn (brdev, fnidx);

            if (ret) {
                sr_set_error(session, node->xpath,
                             ERR_FRER_SEQ_GEN_LIST_CREATE_STR, node->xpath);
                tsn_port_close(brdev);
                return SR_ERR_OPERATION_FAILED;
            }
            tsn_port_close(brdev);
        }
    }

    sr_free_val(new_value);
    sr_free_val(old_value);
    sr_free_change_iter(iter);
    node = NULL;

    return SR_ERR_OK;
}

/**
 * @brief Subroutine that is executed by the thread.
 *
 * @param[in]   callback_value  Pointer to void that contains the arguments of
 *                              the function address_thread.
 * @return      NULL.
 */
void *seq_gen_stream_list_thread(void *callback_value)
{
    sr_val_t *node = (sr_val_t*)callback_value;
    sr_conn_ctx_t *refresh_connection = NULL;
    sr_session_ctx_t *refresh_session = NULL;
    sr_xpath_ctx_t st = {0};
    char fn_index[MAX_STR_LEN] = {0};
    char path[MAX_STR_LEN] = {0};

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    if (NULL == node) {
        SRP_LOG_ERR(ERR_ARGUMENT_OF_ROUTINE_STR,  __func__);
        return NULL;
    }

    /* Creating new connection */
    if (SR_ERR_OK  != sr_connect(0, &refresh_connection)) {
        SRP_LOG_ERR(ERR_NEW_CONNECTION_ROUTINE_STR, __func__);
        sr_free_val(node);
        return NULL;
    }

    /*  Staring new session with RUNNING datastore */
    if (SR_ERR_OK  != sr_session_start(refresh_connection, SR_DS_RUNNING,
                                       &refresh_session)) {
        SRP_LOG_ERR(ERR_NEW_SESSION_FAILED_STR, __func__);
        sr_free_val(node);
        return NULL;
    }

    /* get index value, key of list sequence-recovery-list */
    if (EXIT_SUCCESS != get_key_value(NULL, (char*)node->xpath,
                                      FRER_SEQ_GEN_LIST_STR, FRER_INDEX_STR,
                                      &st, fn_index)) {
        SRP_LOG_ERR("%s: %s (%s)", __func__,
                    ERR_MISSING_ELEMENT_STR, FRER_INDEX_STR);
        sr_free_val(node);
        return NULL;
    }

    /* Check if the changed node is port-list. */
    if (NULL != strstr(node->xpath, FRER_STREAM_LIST_STR)) {
        fill_xpath(path, FRER_SEQ_GEN_LIST_XPATH, fn_index);
        if (SR_ERR_OK != sr_delete_item(refresh_session, path, SR_EDIT_STRICT)) {
            SRP_LOG_ERR(ERR_DELETING_ENTRY_STR, FRER_SEQ_RECOVERY_LIST_STR, __func__);
            sr_free_val(node);
            return NULL;
        }
    }

    if (SR_ERR_OK != sr_apply_changes(refresh_session, 0, 0)) {
        SRP_LOG_ERR(ERR_APPLYING_CHANGES_FAILED_STR, __func__);
        sr_free_val(node);
        sr_session_stop(refresh_session);
        sr_disconnect(refresh_connection);
        return NULL;
    }

    sr_free_val(node);
    sr_session_stop(refresh_session);
    sr_disconnect(refresh_connection);
    refresh_session = NULL;
    refresh_connection = NULL;

    return NULL;
}

/**
 * @brief This function checks if there is more than one leaf-list stream
 * inside sequence-generation entry.
 *
 * @param[in]   session         Implicit session (do not stop) with information
 *                              about the changed data (retrieved by
 *                              sr_get_changes_iter) the event originator
 *                              session IDs.
 * @param[in]   node            Pointer to sr_val_t structure representing node
 *                              that is changed.
 * @return      Error code (EXIT_SUCCESS on success).
 */
int check_existing_seq_gen_stream_lists(sr_session_ctx_t *session,
                                        sr_val_t *node)
{
    sr_xpath_ctx_t st = {0};
    char fn_index[MAX_STR_LEN] = "";
    char path[MAX_STR_LEN] = "";
    sr_val_t *stream_list_entries = NULL;
    size_t val_cnt = 0;
    int i = 0;
    int br = 0;

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    /* get index value, key of list sequence-generation */
    if (EXIT_SUCCESS != get_key_value(NULL, (char*)node->xpath,
                                      FRER_SEQ_GEN_LIST_STR, FRER_INDEX_STR,
                                      &st, fn_index)) {
        SRP_LOG_ERR("%s: %s (%s)", __func__,
                    ERR_MISSING_ELEMENT_STR, FRER_INDEX_STR);
        return EXIT_FAILURE;
    }

    fill_xpath(path, FRER_SEQ_GEN_LIST_STREAM_LIST_XPATH, fn_index);
    if (SR_ERR_OK != sr_get_items(session, path, 0, 0,
                                  &stream_list_entries, &val_cnt)) {
        SRP_LOG_ERR(ERR_FAILED_GET_OBJ_STR, path);
        return EXIT_FAILURE;
    }

    for (i = 0; i < (int)val_cnt; i++) {
        sr_print_val(&stream_list_entries[i]);
        if (strstr(stream_list_entries[i].xpath, FRER_STREAM_LIST_STR)) {
            br++;
        }
    }

    sr_free_values(stream_list_entries, val_cnt);

    /* If counter is different from 0, then there are more than one stream
     * leaf-lists.
     */
    if (0 != br) {
        return EXIT_FAILURE;
    }

    /* If there is no stream entries left, return EXIT_SUCCESS. */
    return EXIT_SUCCESS;
}

/**
 * @brief Callback to be called by the event of changing content of leaf-list
 * stream inside sequence-generation list inside ieee802-dot1cb-frer yang
 * module.
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
 * @param[in]   private_data   Private context opaque to sysrepo, as passed to
 *                              sr_module_change_subscribe call.
 * @return      Error code (SR_ERR_OK on success).
  */
int frer_sequence_generation_list_leaf_list_cb(sr_session_ctx_t *session,
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
    char *bridge_name = "br0";
    char fn_index[MAX_STR_LEN] = "";
    char err_msg[MAX_STR_LEN] = "";
    struct port_hdl *bridge = NULL;
    uint32_t ret = 0u;
    uint16_t fnidx = 0;
    uint16_t stream_list = 0;

    /* Pointer to an unsigned integer value that returns the thread id of the thread created. */
    // pthread_t thread;

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    if (0 == plugin_init) {
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

        /* if leaf-list stream or stream is added, deleted or modified */
        if ((SR_OP_CREATED == op) ||
            (SR_OP_MODIFIED == op) ||
            (SR_OP_DELETED == op)) {

            /* get index value, key of sequence-generation */
            if (EXIT_SUCCESS != get_key_value(session, (char*)node->xpath,
                                              FRER_SEQ_GEN_LIST_STR, FRER_INDEX_STR,
                                              &st, fn_index)) {
                SRP_LOG_ERR("%s: %s (%s)", __func__,
                            ERR_MISSING_ELEMENT_STR, FRER_INDEX_STR);
                return SR_ERR_OPERATION_FAILED;
            }
            fnidx = strtoul(fn_index, NULL, 10);

            stream_list = node->data.uint64_val;

            /* open bridge */
            bridge = tsn_port_open_by_name(bridge_name);
            if (NULL == bridge) {
                snprintf(err_msg, MAX_STR_LEN, ERROR_MSG_FUN_AND_MSG,
                         ERR_PORT_NOT_OPEN_STR, bridge_name);
                sr_set_error(session, node->xpath, err_msg);
                return SR_ERR_OPERATION_FAILED;
            }

            if ((SR_OP_CREATED == op) || (SR_OP_MODIFIED == op)) {
                ret = tsn_frer_seqgen_add_strhdl(bridge, fnidx, stream_list);
                if (ret) {
                    sr_set_error(session, node->xpath,
                                 ERR_FRER_SEQ_GEN_LIST_ADD_STREAM_LIST_STR,
                                 node->xpath);
                    tsn_port_close(bridge);
                    return SR_ERR_OPERATION_FAILED;
                }
            }

            if (SR_OP_DELETED == op) {
                if ((event != SR_EV_DONE) &&
                    (EXIT_SUCCESS == check_sequence_generation_list(session, node))) {
                    ret = tsn_frer_seqgen_del_strhdl(bridge, stream_list);
                    if (ret) {
                        sr_set_error(session, node->xpath,
                                     ERR_FRER_SEQ_GEN_LIST_ENTRY_DATA_STR,
                                     node->xpath);
                        tsn_port_close(bridge);
                        return SR_ERR_OPERATION_FAILED;
                    }
                }

#if 0
                /* create thread to delete entire entry */
                if ((event == SR_EV_DONE) &&
                    (EXIT_SUCCESS == check_existing_seq_gen_stream_lists(session, old_value))) {
                    rc = pthread_create((pthread_t *)&thread, NULL,
                                        seq_gen_stream_list_thread, old_value);
                    if (rc != SR_ERR_OK) {
                        sr_set_error(session, node->xpath,
                                     ERR_CREATING_THREAD_FAILED_XPATH_STR,
                                     node->xpath);
                        return SR_ERR_OPERATION_FAILED;
                    }
                }
#endif
            }
            tsn_port_close(bridge);
        }
    }

    sr_free_val(new_value);
    sr_free_change_iter(iter);
    node = NULL;

    return SR_ERR_OK;
}

/**
 * @brief Callback to be called by the event of changing content of
 * sequence-identification-list list inside ieee802-dot1cb-frer yang module.
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
int frer_sequence_identification_list_cb(sr_session_ctx_t *session,
                                         const char *module_name,
                                         const char *xpath, sr_event_t event,
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
    sr_val_t *direction = NULL;
    sr_val_t *active = NULL;
    char path[MAX_STR_LEN] = "";
    struct port_hdl *port_dev = NULL;
    uint32_t ret = 0u;
    uint8_t dir = 2;
    /* Put direction default value to 2, because possible values are 1 and 0,
     * so callback should fail if direction has wrong value.
     */

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

        /* open port */
        port_dev = tsn_port_open_by_name(node->data.string_val);
        if (NULL == port_dev) {
            sr_set_error(session, node->xpath,
                         ERR_PORT_NOT_OPEN_XPATH_STR, node->data.string_val);
            return SR_ERR_OPERATION_FAILED;
        }

        if ((SR_OP_CREATED == op) || (SR_OP_MODIFIED == op)) {
            /* get leaf active */
            fill_xpath(path, FRER_SEQ_IDENT_LIST_LEAF_ACTIVE_XPATH, node->data.string_val);
            if (SR_ERR_OK != sr_get_item(session, path, 0, &active)) {
                sr_set_error(session, path, ERR_FAILED_GET_OBJ_STR, path);
                return SR_ERR_OPERATION_FAILED;
            }

            /* get leaf direction */
            fill_xpath(path, FRER_SEQ_IDENT_LIST_LEAF_DIRECTION_XPATH, node->data.string_val);
            if (SR_ERR_OK != sr_get_item(session, path, 0, &direction)) {
                sr_set_error(session, path, ERR_FAILED_GET_OBJ_STR, path);
                return SR_ERR_OPERATION_FAILED;
            }

            dir = direction->data.bool_val;

            ret = tsn_frer_seqenc_create(port_dev, dir);

            if (!ret) {
                ret = tsn_frer_seqenc_setactive(port_dev, active->data.bool_val);
                if (ret) {
                    tsn_frer_seqenc_delport(port_dev);
                    sr_set_error(session, node->xpath,
                                 ERR_FRER_SEQ_IDENT_LIST_ACTIVE_STR, node->xpath);
                    tsn_port_close(port_dev);
                    return SR_ERR_OPERATION_FAILED;
                }
            }
            if (ret) {
                sr_set_error(session, node->xpath,
                             ERR_FRER_SEQ_IDENT_LIST_CREATE_STR, node->xpath);
                tsn_port_close(port_dev);
                return SR_ERR_OPERATION_FAILED;
            }
        }
        if (SR_OP_DELETED == op) {
            ret = tsn_frer_seqenc_delport(port_dev);

            if (ret) {
                sr_set_error(session, node->xpath,
                             ERR_FRER_SEQ_IDENT_LIST_ENTRY_DEL_STR, node->xpath);
                tsn_port_close(port_dev);
                return SR_ERR_OPERATION_FAILED;
            }
        }
    }
    tsn_port_close(port_dev);

    sr_free_val(new_value);
    sr_free_val(old_value);
    sr_free_change_iter(iter);
    node = NULL;

    return SR_ERR_OK;
}

/**
 * @brief Callback to be called by the event of changing content of leaf active
 * inside sequence-identification-list list inside ieee802-dot1cb-frer yang
 * module.
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
int frer_sequence_identification_list_active_cb(sr_session_ctx_t *session,
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
    char leaf_port[MAX_STR_LEN] = "";
    struct port_hdl *port_dev = NULL;
    uint32_t ret = 0u;

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

        /* get port value, key of list sequence-identification-list list */
        if (EXIT_SUCCESS != get_key_value(session, node->xpath,
                                          FRER_SEQ_ID_LIST_STR, FRER_PORT_STR,
                                          &st, leaf_port)) {
            SRP_LOG_ERR("%s: %s (%s)", __func__,
                        ERR_MISSING_ELEMENT_STR, FRER_PORT_STR);
            return SR_ERR_OPERATION_FAILED;
        }

        /* Add operation is covered by callback on list, because list entry can
         * not be created if leaf active does not exist
         */
        if (SR_OP_MODIFIED == op) {
            /* open bridge */
            port_dev = tsn_port_open_by_name(leaf_port);
            if (NULL == port_dev) {
                sr_set_error(session, node->xpath,
                             ERR_PORT_NOT_OPEN_XPATH_STR, leaf_port);
                return SR_ERR_OPERATION_FAILED;
            }

            ret = tsn_frer_seqenc_setactive(port_dev, node->data.bool_val);
            if (ret) {
                sr_set_error(session, node->xpath,
                             ERR_FRER_SEQ_IDENT_LIST_ACTIVE_STR, node->xpath);
                tsn_port_close(port_dev);
                return SR_ERR_OPERATION_FAILED;
            }
            tsn_port_close(port_dev);
        }
    }

    sr_free_val(new_value);
    sr_free_val(old_value);
    sr_free_change_iter(iter);
    node = NULL;

    return SR_ERR_OK;
}

/**
 * @brief Callback to be called by the event of changing content of leaf-list
 * stream inside ieee802-dot1cb-frer yang module.
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
int frer_sequence_identification_list_leaf_list_cb(sr_session_ctx_t *session,
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
    sr_val_t *type = NULL;
    char path[MAX_STR_LEN] = "";
    char leaf_port[MAX_STR_LEN] = "";
    char err_msg[MAX_STR_LEN] = "";
    struct port_hdl *port = NULL;
    uint32_t ret = 0u;
    uint16_t stream_list = 0;
    uint8_t type_value = 4;
    /* Put direction encapsulation value to 4, because possible values are 1, 2
     * and 3, so callback should fail if encapsulation-type has wrong value.
     */

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

        /* if leaf-list stream or stream is added or deleted */
        if ((SR_OP_CREATED == op) ||
            (SR_OP_MODIFIED == op) ||
            (SR_OP_DELETED == op)) {
            stream_list = node->data.uint16_val;

            /* get port value, key of list sequence-identification-list list */
            if (EXIT_SUCCESS != get_key_value(session, node->xpath,
                                              FRER_SEQ_ID_LIST_STR, FRER_PORT_STR,
                                              &st, leaf_port)) {
                SRP_LOG_ERR("%s: %s (%s)", __func__,
                            ERR_MISSING_ELEMENT_STR, FRER_PORT_STR);
                return SR_ERR_OPERATION_FAILED;
            }

            /* open port */
            port = tsn_port_open_by_name(leaf_port);
            if (NULL == port) {
                snprintf(err_msg, MAX_STR_LEN, ERROR_MSG_FUN_AND_MSG,
                         ERR_PORT_NOT_OPEN_STR, leaf_port);
                sr_set_error(session, node->xpath, err_msg);
                return SR_ERR_OPERATION_FAILED;
            }

            if ((SR_OP_CREATED == op) || (SR_OP_MODIFIED == op)) {
                fill_xpath(path, FRER_SEQ_IDENT_LIST_ENC_R_TAG_XPATH, leaf_port);
                if (SR_ERR_OK == sr_get_item(session, path, 0, &type)) {
                    type_value = 1;
                }

                if (4 == type_value) {
                    fill_xpath(path, FRER_SEQ_IDENT_LIST_ENC_HSR_SEQ_TAG_XPATH, leaf_port);
                    if (SR_ERR_OK == sr_get_item(session, path, 0, &type)) {
                        type_value = 2;
                    }
                }

                if (4 == type_value) {
                    fill_xpath(path, FRER_SEQ_IDENT_LIST_ENC_PRP_SEQ_TRAILER_XPATH, leaf_port);
                    if (SR_ERR_OK == sr_get_item(session, path, 0, &type)) {
                        type_value = 3;
                    }
                }

                ret = tsn_frer_seqenc_add_str(port, stream_list);
                if (ret) {
                    sr_set_error(session, node->xpath,
                                 ERR_FRER_SEQ_IDENT_LIST_ADD_STREAM_LIST_STR,
                                 node->xpath);
                    tsn_port_close(port);
                    return SR_ERR_OPERATION_FAILED;
                }

                ret = tsn_frer_seqenc_settype(port, type_value);
                if (ret) {
                    sr_set_error(session, node->xpath,
                                 ERR_FRER_SEQ_IDENT_LIST_ADD_STREAM_LIST_STR,
                                 node->xpath);
                    tsn_port_close(port);
                    return SR_ERR_OPERATION_FAILED;
                }
            }

            if ((SR_OP_DELETED == op) &&
                (EXIT_SUCCESS == check_stream_indentification_list(session, node))) {
                ret = tsn_frer_seqenc_delstr(port, stream_list);
                if (ret) {
                    sr_set_error(session, node->xpath,
                                 ERR_FRER_SEQ_IDENT_LIST_ENTRY_DATA_STR,
                                 node->xpath);
                    tsn_port_close(port);
                    return SR_ERR_OPERATION_FAILED;
                }
            }
            tsn_port_close(port);
        }
    }

    sr_free_val(new_value);
    sr_free_val(old_value);
    sr_free_change_iter(iter);
    node = NULL;

    return SR_ERR_OK;
}

/**
 * @brief Callback to be called by the event of deleting r-tag presence
 * container of sequence identification list entry.
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
int frer_sequence_identification_list_enc_rtag_cb(sr_session_ctx_t *session,
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
    sr_val_t *iter_node = NULL;
    sr_val_t *tmp_node = NULL;
    char port_name[MAX_STR_LEN] = "";
    char path[MAX_STR_LEN] = "";

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    if ((SR_EV_DONE == event) || (0 == plugin_init)) {
        SRP_LOG_DBG(DEBUG_MSG_WITH_TWO_PARAM, DBG_APPLYING_CHANGES_MSG, __func__);
        return SR_ERR_OK;
    }

    rc = sr_get_changes_iter(session, xpath, &iter);
    if (SR_ERR_OK != rc) {
        return rc;
    }

    while (SR_ERR_OK == sr_get_change_next(session, iter, &op, &old_value, &new_value)) {
        iter_node = (op == SR_OP_DELETED) ? old_value : new_value;

        if (SR_OP_DELETED == op) {
            /* get port value, key of list sequence-identification-list list */
            if (EXIT_SUCCESS != get_key_value(session, iter_node->xpath,
                                              FRER_SEQ_ID_LIST_STR, FRER_PORT_STR,
                                              &st, port_name)) {
                SRP_LOG_ERR("%s: %s (%s)", __func__,
                            ERR_MISSING_ELEMENT_STR, FRER_PORT_STR);
                return SR_ERR_OPERATION_FAILED;
            }

            fill_xpath(path, FRER_SEQ_IDENT_LIST_LEAF_PORT_XPATH, port_name);
            if (SR_ERR_OK == sr_get_item(session, path, 0, &tmp_node)) {
                sr_set_error(session, iter_node->xpath,
                             ERR_FRER_SEQ_IDENT_LIST_ENTRY_GET_STR,
                             iter_node->xpath);
                return SR_ERR_OPERATION_FAILED;
            }
        }
    }

    sr_free_val(new_value);
    sr_free_val(old_value);
    sr_free_change_iter(iter);

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

    /* Set plugin_init to "1" so callbacks are triggered when startup
     * configuration is copied to running.
     */
    if (0 != sr_path_check_startup_done_file(FRER_MODULE_NAME_STR, &path)) {
        plugin_init = 1;
    }

    do {
        /* subscribe for /ieee802-dot1cb-frer module changes */
        rc = sr_module_change_subscribe(session, FRER_MODULE_NAME_STR, NULL,
                module_change_cb, NULL, 0, SR_SUBSCR_ENABLED, &subscription);
         if (SR_ERR_OK != rc) break;

        /* subscribe for /ieee802-dot1cb-frer:frer/sequence-generation/index changes */
        rc = sr_module_change_subscribe(session, FRER_MODULE_NAME_STR, "/ieee802-dot1cb-frer:frer/sequence-generation/index",
                frer_sequence_generation_list_cb, NULL, 5, SR_SUBSCR_ENABLED, &subscription);
        if (rc != SR_ERR_OK) break;

        /* subscribe for /ieee802-dot1cb-frer:frer/sequence-generation/direction changes */
        rc = sr_module_change_subscribe(session, FRER_MODULE_NAME_STR, "/ieee802-dot1cb-frer:frer/sequence-generation/direction-out-facing",
                mod_not_supported_cb, NULL, 3, SR_SUBSCR_ENABLED, &subscription);
        if (rc != SR_ERR_OK) break;

        /* subscribe for /ieee802-dot1cb-frer:frer/sequence-generation/stream changes */
        rc = sr_module_change_subscribe(session, FRER_MODULE_NAME_STR, "/ieee802-dot1cb-frer:frer/sequence-generation/stream",
                frer_sequence_generation_list_leaf_list_cb, NULL, 4, SR_SUBSCR_ENABLED, &subscription);
        if (rc != SR_ERR_OK) break;

        /* subscribe for /ieee802-dot1cb-frer:frer/sequence-recovery/index changes */
        rc = sr_module_change_subscribe(session, FRER_MODULE_NAME_STR, "/ieee802-dot1cb-frer:frer/sequence-recovery/index",
                frer_sequence_recovery_list_cb, NULL, 5, SR_SUBSCR_ENABLED, &subscription);
        if (rc != SR_ERR_OK) break;

        /* subscribe for /ieee802-dot1cb-frer:frer/sequence-recovery/direction changes */
        rc = sr_module_change_subscribe(session, FRER_MODULE_NAME_STR, "/ieee802-dot1cb-frer:frer/sequence-recovery/direction-out-facing",
                frer_sequence_recovery_list_element_cb, NULL, 3, SR_SUBSCR_ENABLED, &subscription);
        if (rc != SR_ERR_OK) break;

        /* subscribe for /ieee802-dot1cb-frer:frer/sequence-recovery/algorithm changes */
        rc = sr_module_change_subscribe(session, FRER_MODULE_NAME_STR, "/ieee802-dot1cb-frer:frer/sequence-recovery/algorithm/*",
                frer_sequence_recovery_list_element_cb, NULL, 3, SR_SUBSCR_ENABLED, &subscription);
        if (rc != SR_ERR_OK) break;

        /* subscribe for /ieee802-dot1cb-frer:frer/sequence-recovery/history-length changes */
        rc = sr_module_change_subscribe(session, FRER_MODULE_NAME_STR, "/ieee802-dot1cb-frer:frer/sequence-recovery/history-length",
                frer_sequence_recovery_list_element_cb, NULL, 3, SR_SUBSCR_ENABLED, &subscription);
        if (rc != SR_ERR_OK) break;

        /* subscribe for /ieee802-dot1cb-frer:frer/sequence-recovery/reset-timeout changes */
        rc = sr_module_change_subscribe(session, FRER_MODULE_NAME_STR, "/ieee802-dot1cb-frer:frer/sequence-recovery/reset-timeout",
                frer_sequence_recovery_list_element_cb, NULL, 3, SR_SUBSCR_ENABLED, &subscription);
        if (rc != SR_ERR_OK) break;

        /* subscribe for /ieee802-dot1cb-frer:frer/sequence-recovery/take-no-sequence changes */
        rc = sr_module_change_subscribe(session, FRER_MODULE_NAME_STR, "/ieee802-dot1cb-frer:frer/sequence-recovery/take-no-sequence",
                frer_sequence_recovery_list_element_cb, NULL, 3, SR_SUBSCR_ENABLED, &subscription);
        if (rc != SR_ERR_OK) break;

        /* subscribe for /ieee802-dot1q-bridge:bridges/bridge/component/ieee802-dot1CB:frer/sequence-recovery/individual-recovery changes */
        rc = sr_module_change_subscribe(session, FRER_MODULE_NAME_STR, "/ieee802-dot1cb-frer:frer/sequence-recovery/individual-recovery",
                frer_sequence_recovery_list_element_cb, NULL, 3, SR_SUBSCR_ENABLED, &subscription);
        if (rc != SR_ERR_OK) break;

        /* subscribe for /ieee802-dot1cb-frer:frer/sequence-recovery/latent-error-detectio changes */
        rc = sr_module_change_subscribe(session, FRER_MODULE_NAME_STR, "/ieee802-dot1cb-frer:frer/sequence-recovery/latent-error-detection",
                frer_sequence_recovery_list_element_cb, NULL, 3, SR_SUBSCR_ENABLED, &subscription);
        if (rc != SR_ERR_OK) break;

        /* subscribe for /ieee802-dot1cb-frer:frer/sequence-recovery/stream changes */
        rc = sr_module_change_subscribe(session, FRER_MODULE_NAME_STR, "/ieee802-dot1cb-frer:frer/sequence-recovery/stream",
                frer_sequence_recovery_leaf_list_cb, NULL, 4, SR_SUBSCR_ENABLED, &subscription);
        if (rc != SR_ERR_OK) break;

        /* subscribe for /ieee802-dot1cb-frer:frer/sequence-recovery/port-list changes */
        rc = sr_module_change_subscribe(session, FRER_MODULE_NAME_STR, "/ieee802-dot1cb-frer:frer/sequence-recovery/port",
                frer_sequence_recovery_leaf_list_cb, NULL, 4, SR_SUBSCR_ENABLED, &subscription);
        if (rc != SR_ERR_OK) break;

        /* subscribe for /ieee802-dot1cb-frer:frer/sequence-identification-list/portchanges */
        rc = sr_module_change_subscribe(session, FRER_MODULE_NAME_STR, "/ieee802-dot1cb-frer:frer/sequence-identification/port",
                frer_sequence_identification_list_cb, NULL, 5, SR_SUBSCR_ENABLED, &subscription);
        if (rc != SR_ERR_OK) break;

        /* subscribe for /ieee802-dot1cb-frer:frer/sequence-identification-list/active changes */
        rc = sr_module_change_subscribe(session, FRER_MODULE_NAME_STR, "/ieee802-dot1cb-frer:frer/sequence-identification/active",
                frer_sequence_identification_list_active_cb, NULL, 3, SR_SUBSCR_ENABLED, &subscription);
        if (rc != SR_ERR_OK) break;

        /* subscribe for /ieee802-dot1cb-frer:frer/sequence-identification-list/direction changes */
        rc = sr_module_change_subscribe(session, FRER_MODULE_NAME_STR, "/ieee802-dot1cb-frer:frer/sequence-identification/direction-out-facing",
                mod_not_supported_cb, NULL, 3, SR_SUBSCR_ENABLED, &subscription);
        if (rc != SR_ERR_OK) break;

        /* subscribe for /ieee802-dot1cb-frer:frer/sequence-identification-list/encapsulation changes */
        rc = sr_module_change_subscribe(session, FRER_MODULE_NAME_STR, "/ieee802-dot1cb-frer:frer/sequence-identification/encapsulation/r-tag",
                frer_sequence_identification_list_enc_rtag_cb, NULL, 3, SR_SUBSCR_ENABLED, &subscription);
        if (rc != SR_ERR_OK) break;

        /* subscribe for /ieee802-dot1cb-frer:frer/sequence-identification-list/stream changes */
        rc = sr_module_change_subscribe(session, FRER_MODULE_NAME_STR, "/ieee802-dot1cb-frer:frer/sequence-identification/stream",
                frer_sequence_identification_list_leaf_list_cb, NULL, 4, SR_SUBSCR_ENABLED, &subscription);
        if (rc != SR_ERR_OK) break;

    } while (0);

    if (SR_ERR_OK != rc) {
        SRP_LOG_ERR(ERR_MODULE_INIT_FAILED_STR, FRER_MODULE_NAME_STR, sr_strerror(rc));
        sr_unsubscribe(subscription);
        return rc;
    }

    if (EXIT_FAILURE == frer_fill_startup_datastore(session)) {
        SRP_LOG_ERR(ERROR_MSG_MOD_FUNC_MSG, FRER_MODULE_NAME_STR, __func__,
                    ERR_MSG_EMPTY_STARTUP_DS_STR);
        return SR_ERR_OPERATION_FAILED;
    }

    plugin_init = 2;

    SRP_LOG_INF(INF_MODULE_INIT_SUCCESS_STR, FRER_MODULE_NAME_STR);
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
    SRP_LOG_INF(INF_MODULE_CLEANUP_STR, FRER_MODULE_NAME_STR);
}
