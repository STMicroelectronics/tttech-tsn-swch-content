/*
 * TTTech ietf-interfaces-yang-module
 * Copyright(c) 2023 TTTech Computertechnik AG.
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
 * Contact: https://tttech.com * support@tttech.com
 * TTTech Computertechnik AG, Schoenbrunnerstrasse 7, 1040 Vienna, Austria
 */

#ifndef CONFIG_CHANGE_H_
#define CONFIG_CHANGE_H_

#include <sysrepo.h>

#include <stdbool.h>

/**
 * @brief Start config-change reset helper thread.
 *
 * @return      Error code (SR_ERR_OK on success).
 */
int config_change_helper_start(void);

/**
 * @brief Stop config-change reset helper thread.
 */
void config_change_helper_stop(void);

/**
 * @brief Opaque type for a single config-change reset request.
 *
 * This carries also boolean trigger value so that combined
 * gate-enabled and config-change handler can avoid applying changes
 * to system twice.
 */
struct config_change_reset;

/**
 * @brief List of config-change reset requests.
 *
 * List itself is an item in config-change reset queue.
 */
struct config_change_list {
    struct config_change_list *next_list;
    struct config_change_reset *first;
    struct config_change_reset *last;
};

/**
 * @brief Get new list for collecting config-change reset requests.
 *
 * List must eventually be freed by free().
 *
 * @return      New list or NULL on failure.
 */
struct config_change_list *config_change_list_new(void);

/**
 * @brief Append new config-change reset request to list.
 *
 * This is completely local until config_change_list_enqueue() is called.
 * List contents can also be abandoned by config_change_list_clear() instead.
 *
 * @param[out]  list            List to add reset request to.
 * @param[in]   xpath           XPath of the config-change item to reset.
 * @param[in]   value           Whether to actually reset xpath or not.
 * @return      Error code (SR_ERR_OK on success).
 */
int config_change_list_push(struct config_change_list *list,
                            const char *xpath, bool value);

/**
 * @brief Check if config-change XPath already exists in list.
 *
 * @param[in]   list            List to check.
 * @param[in]   xpath           XPath of the config-change item to check.
 * @return      True if xpath already is in the list.
 */
bool config_change_list_exists(const struct config_change_list *list,
                               const char *xpath);

/**
 * @param Remove all config-change reset requests from list.
 *
 * @param[out]  list            List to clear from all reset requests.
 */
void config_change_list_clear(struct config_change_list *list);

/**
 * @brief Send config-change reset list to helper thread for processing.
 *
 * @param[in]   list            List to send. List is empty after call.
 */
void config_change_list_enqueue(struct config_change_list *list);

/**
 * @brief Process config-change reset list using given session.
 *
 * This is used internally by the helper thread, but can also be used
 * explicitly by plugins at plugin initialization time,
 * before subscriptions.
 *
 * @param[in]   session         Session to use for applying changes to current
 *                              session datastore.
 * @param[in]   list            List of config-change reset requests.
 *                              List is empty after call.
 */
void config_change_list_process(sr_session_ctx_t *session,
                                struct config_change_list *list);

#endif
