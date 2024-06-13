/*
 * TTTech sysrepo-module-versions-yang-module
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
#include <inttypes.h>
#include <sys/select.h>
#include <fcntl.h>
/* common includes */
#include "common_defines.h"
#include "common.h"
/* sysrepo includes */
#include <sysrepo.h>
#include <sysrepo/values.h>
#include <sysrepo/xpath.h>
/* libbase includes */
#include "libbase/base_prints.h"

/* structure holding all the subscriptions */
sr_subscription_ctx_t *subscription;

int plugin_init = 0;

/* module specific defines */
#define SR_MODULE_VERSIONS_NAME_STR             "sysrepo-module-versions"
#define SR_MOD_VER_YANG_FILE_PATH               "/etc/netopeer2/yang/sysrepo-module-versions.yang"
#define SR_MOD_VER_DATASTORE_FILE_PATH          "/etc/netopeer2/yang/sysrepo-module-versions.data.xml"

#define MOD_VER_NAME_STR                        "name"
#define MOD_VER_REVISION_STR                    "revision"
#define MOD_VER_SOURCE_STR                      "source"
#define MOD_VER_COMMIT_ID_STR                   "commit_id"
#define MOD_VER_INTERNAL_STR                    "internal_version"
#define MOD_VER_DEVELOPER_STR                   "developer_change"

/* error messages */
#define SR_MOD_VER_CTX_ERROR_STR                "Private context opaque to sysrepo is NULL."
#define SR_MOD_VER_DATASTORE_OPEN_ERROR_STR     "Unable to open the datastore file for %s module."
#define SR_MOD_VER_MODULE_ERROR_STR             "Main schema node structure 'module' representing %s YANG module is NULL."
#define SR_MOD_VER_TREE_LOAD_ERROR_STR          "Loading tree failed for sysrepo-module-versions"

/* leaf xpaths */
#define SR_MOD_VER_XPATH                        "/sysrepo-module-versions:sysrepo-module-versions/module[name='%s']"
#define SR_MOD_VER_REVISION_XPATH               SR_MOD_VER_XPATH "/revision"
#define SR_MOD_VER_SOURCE_XPATH                 SR_MOD_VER_XPATH "/source"
#define SR_MOD_VER_COMMIT_ID_XPATH              SR_MOD_VER_XPATH "/commit_id"
#define SR_MOD_VER_INT_VERSION_XPATH            SR_MOD_VER_XPATH "/internal_version"
#define SR_MOD_VER_DEV_CHANGE_XPATH             SR_MOD_VER_XPATH "/developer_change"

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
}

/**
 * @brief Callback to be called when operational data of
 * sysrepo-module-versions is requested.
 *
 * Subscribe to it by sr_dp_get_items_subscribe call.
 *
 * @param[in]   session         Implicit session (do not stop) with information
 *                              about the event originator session IDs.
 * @param[out   module_name     Name of the affected module.
 * @param[out   path            Path identifying the subtree that is supposed
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
static int sys_repo_module_versions_state_cb(sr_session_ctx_t *session,
                                             const char *module_name,
                                             const char *path,
                                             const char *request_xpath,
                                             uint32_t request_id,
                                             struct lyd_node **parent,
                                             void *private_data)
{
    (void)session;
    (void)module_name;
    (void)request_xpath;
    (void)request_id;
    (void)private_data;
    (void)path;

    int rc = SR_ERR_OK;
    struct ly_ctx *ctx = NULL;
    const struct lys_module *module = NULL;
    struct lyd_node *root = NULL;
    struct lyd_node *node = NULL;
    struct lyd_node *next = NULL;
    struct lyd_node_leaf_list *tmp = NULL;
    char node_xpath[MAX_STR_LEN] = "";
    char yang_module_name[MAX_STR_LEN] = "";
    int fd = -1;
    bool flag = false;

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    ctx = ly_ctx_new(NULL, 0);

    if (!ctx) {
        SRP_LOG_ERR(ERROR_MSG_FUN_AND_MSG, __func__, SR_MOD_VER_CTX_ERROR_STR);
        return SR_ERR_OPERATION_FAILED;
    }

    fd = open(SR_MOD_VER_DATASTORE_FILE_PATH, O_RDONLY);

    if (-1 == fd) {
        SRP_LOG_ERR(SR_MOD_VER_DATASTORE_OPEN_ERROR_STR,
                    SR_MODULE_VERSIONS_NAME_STR);
        close(fd);
        return SR_ERR_OPERATION_FAILED;
    }

    module = lys_parse_path(ctx, SR_MOD_VER_YANG_FILE_PATH, LYS_YANG);

    if (!module) {
        SRP_LOG_ERR(SR_MOD_VER_MODULE_ERROR_STR, SR_MODULE_VERSIONS_NAME_STR);
        return SR_ERR_OPERATION_FAILED;
    }

    root = lyd_parse_fd(ctx, fd, LYD_XML, LYD_OPT_DATA | LYD_OPT_TRUSTED);

    if (root) {
        /* fill up array of values */
        LY_TREE_FOR(root->child, node) {
            LY_TREE_FOR(node->child, next) {
                if (strncmp(next->schema->name, MOD_VER_NAME_STR, MAX_STR_LEN) == 0) {
                    tmp = (struct lyd_node_leaf_list *)next;
                    if (NULL != tmp->value_str) {
                        strncpy(yang_module_name, tmp->value_str, MAX_STR_LEN-1);
                        flag = true;
                    }
                }
                if ((strncmp(next->schema->name, MOD_VER_REVISION_STR, MAX_STR_LEN) == 0) && (flag = true)) {
                    fill_xpath(node_xpath, SR_MOD_VER_REVISION_XPATH, yang_module_name);
                    tmp = (struct lyd_node_leaf_list *)next;
                    if (NULL == lyd_new_path(*parent, NULL, node_xpath, (char*)tmp->value_str, 0, 0)) {
                        SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                                    MOD_VER_REVISION_STR, ERR_NOT_CREATED_ELEMENT_STR);
                    }
                }

                if ((strncmp(next->schema->name, MOD_VER_SOURCE_STR, MAX_STR_LEN) == 0) && (flag = true)) {
                    fill_xpath(node_xpath, SR_MOD_VER_SOURCE_XPATH, yang_module_name);
                    tmp = (struct lyd_node_leaf_list *)next;
                    if (NULL == lyd_new_path(*parent, NULL, node_xpath, (char*)tmp->value_str, 0, 0)) {
                        SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                                    MOD_VER_SOURCE_STR, ERR_NOT_CREATED_ELEMENT_STR);
                    }
                }

                if ((strncmp(next->schema->name, MOD_VER_COMMIT_ID_STR, MAX_STR_LEN) == 0) && (flag = true)) {
                    fill_xpath(node_xpath, SR_MOD_VER_COMMIT_ID_XPATH, yang_module_name);
                    tmp = (struct lyd_node_leaf_list *)next;
                    if (NULL == lyd_new_path(*parent, NULL, node_xpath, (char*)tmp->value_str, 0, 0)) {
                        SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                                    MOD_VER_COMMIT_ID_STR, ERR_NOT_CREATED_ELEMENT_STR);
                    }
                }

                if ((strncmp(next->schema->name, MOD_VER_INTERNAL_STR, MAX_STR_LEN) == 0) && (flag = true)) {
                    fill_xpath(node_xpath, SR_MOD_VER_INT_VERSION_XPATH, yang_module_name);
                    tmp = (struct lyd_node_leaf_list *)next;
                    if (NULL == lyd_new_path(*parent, NULL, node_xpath, (char*)tmp->value_str, 0, 0)) {
                        SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                                    MOD_VER_INTERNAL_STR, ERR_NOT_CREATED_ELEMENT_STR);
                    }
                }

                if ((strncmp(next->schema->name, MOD_VER_DEVELOPER_STR, MAX_STR_LEN) == 0) && (flag = true)) {
                    fill_xpath(node_xpath, SR_MOD_VER_DEV_CHANGE_XPATH, yang_module_name);
                    tmp = (struct lyd_node_leaf_list *)next;
                    if (NULL == lyd_new_path(*parent, NULL, node_xpath, (char*)tmp->value_str, 0, 0)) {
                        SRP_LOG_INF(ERROR_MSG_FUN_NODE_EL_AND_MSG_STR, __func__,
                                    MOD_VER_DEVELOPER_STR, ERR_NOT_CREATED_ELEMENT_STR);
                    }
                }
            }
            flag = false;
        }
    } else {
        SRP_LOG_ERR(ERROR_MSG_FUN_AND_MSG, __func__,
                    SR_MOD_VER_TREE_LOAD_ERROR_STR);
        return SR_ERR_OPERATION_FAILED;
    }

    if (root) {
        lyd_free(root);
        lyd_free(node);
        lyd_free(next);
        root = NULL;
        node = NULL;
        next = NULL;
    }

    if (ctx)
        ly_ctx_destroy(ctx, NULL);

    if (fd > 0)
        close(fd);

    return rc;
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

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    do {
        /* subscribe for /sysrepo-module-versions module changes */
        rc = sr_module_change_subscribe(session, SR_MODULE_VERSIONS_NAME_STR,
                                        NULL, module_change_cb, NULL, 0,
                                        SR_SUBSCR_DEFAULT, &subscription);
        if (SR_ERR_OK != rc) break;

        /* subscribe as state data provider for the /sysrepo-module-versions:sysrepo-module-versions/module */
        rc = sr_oper_get_items_subscribe(session, SR_MODULE_VERSIONS_NAME_STR,
                                         "/sysrepo-module-versions:sysrepo-module-versions/module",
                                         sys_repo_module_versions_state_cb, NULL,
                                         SR_SUBSCR_DEFAULT, &subscription);
        if (rc != SR_ERR_OK) break;

    } while (0);

    if (SR_ERR_OK != rc) {
        SRP_LOG_ERR(ERR_MODULE_INIT_FAILED_STR,
                    SR_MODULE_VERSIONS_NAME_STR, sr_strerror(rc));
        sr_unsubscribe(subscription);
        return rc;
    }

    SRP_LOG_INF(INF_MODULE_INIT_SUCCESS_STR, SR_MODULE_VERSIONS_NAME_STR);

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
    SRP_LOG_INF(INF_MODULE_CLEANUP_STR, SR_MODULE_VERSIONS_NAME_STR);
}
