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

#include "config_change.h"

#include <sysrepo.h>
#include <sysrepo/values.h>
#include <sysrepo/xpath.h>

#include <string.h>
#include <pthread.h>

struct config_change_reset {
    struct config_change_reset *next;
    bool value;
    char xpath[];
};

/**
 * @brief Queue type of config-change reset request lists.
 *
 * This queue type is used to transfer config-change resets from subscriptions
 * to helper thread in a list for actual processing.
 */
struct config_change_queue {
    struct config_change_list *first;
    struct config_change_list *last;
};

/**
 * @brief Private config-change reset helper context.
 */
struct config_change_helper {
    pthread_t       thread;     ///< helper thread
    pthread_mutex_t lock;       ///< synchronize queue and termination request
    pthread_cond_t  cond;       ///< inform of new queue items or termination

    unsigned int users;         ///< number of active users
    struct config_change_queue queue;   ///< config-change reset request queue

    sr_conn_ctx_t       *conn;          ///< connection to sysrepo
    sr_session_ctx_t    *session;       ///< session to use by helper thread
};

static struct config_change_helper config_change_helper = {
    .lock = PTHREAD_MUTEX_INITIALIZER,
    .cond = PTHREAD_COND_INITIALIZER,
};

/**
 * @brief Get access to helper context singleton.
 *
 * @return      Helper context instance.
 */
static struct config_change_helper *config_change_helper_get(void)
{
    return &config_change_helper;
}

struct config_change_list *config_change_list_new(void)
{
    struct config_change_list *new;

    new = malloc(sizeof(*new));
    if (!new) {
        SRP_LOG_ERR("%s() failed to allocate new list", __func__);
        return NULL;
    }

    *new = (struct config_change_list){ .first = NULL };
    return new;
}

int config_change_list_push(struct config_change_list *list,
                            const char *xpath, bool value)
{
    size_t xpath_len = strlen(xpath) + 1;
    struct config_change_reset *new;

    SRP_LOG_DBG("%s() append %s value %d", __func__, xpath, value);

    new = malloc(sizeof(*new) + xpath_len);
    if (!new) {
        SRP_LOG_ERR("%s() failed to allocate new list item", __func__);
        return SR_ERR_NOMEM;
    }

    new->next = NULL;
    new->value = value;
    memcpy(new->xpath, xpath, xpath_len);

    if (list->last)
        list->last->next = new;
    else
        list->first = new;
    list->last = new;

    return SR_ERR_OK;
}

/**
 * @brief Helper to check if config-change reset request list is empty.
 *
 * @param[in]   list            List to check.
 * @return      True if list is empty.
 */
static inline bool config_change_list_empty(
        const struct config_change_list *list)
{
    return !list->first;
}

bool config_change_list_exists(const struct config_change_list *list,
                               const char *xpath)
{
    struct config_change_reset *item;

    for (item = list->first; item; item = item->next)
        if (strcmp(xpath, item->xpath) == 0)
            return true;
    return false;
}

/**
 * @brief Pop oldest config-change reset request from list.
 *
 * @param[in]   list            List to pop.
 * @return      Reset request or NULL if list is empty.
 *              Returned item must be freed by free().
 */
static struct config_change_reset *config_change_list_pop(
        struct config_change_list *list)
{
    struct config_change_reset *item = list->first;

    if (!item)
        return NULL;

    list->first = item->next;
    if (!list->first)
        list->last = NULL;

    item->next = NULL;

    SRP_LOG_DBG("%s() xpath %s value %d", __func__, item->xpath, item->value);
    return item;
}

void config_change_list_clear(struct config_change_list *list)
{
    struct config_change_reset *next;

    SRP_LOG_DBG("%s()", __func__);

    for (;;) {
        next = config_change_list_pop(list);
        if (!next)
            break;
        free(next);
    }
}

/**
 * @brief Helper to check if config-change reset list queue is empty.
 *
 * @param[in]   queue           Queue to check.
 * @return      True if queue is empty.
 */
static inline bool config_change_queue_empty(
        const struct config_change_queue *queue)
{
    return !queue->first;
}

/**
 * @brief Move config-change reset requests to helper thread queue.
 *
 * @param[out]  queue           Queue to push requests to. Internally new list
 *                              is created, reset requests are moved to that
 *                              list, and then the new list is added to the
 *                              queue.
 * @param[in]   list            List whose reset requests to add to queue.
 *                              List is empty after call.
 * @return      Error code (SR_ERR_OK on success).
 */
static int config_change_queue_push(struct config_change_queue *queue,
                                    struct config_change_list *list)
{
    struct config_change_list *new;

    SRP_LOG_DBG("%s()", __func__);

    new = config_change_list_new();
    if (!new)
        return SR_ERR_NOMEM;
    *new = *list;
    list->first = NULL;
    list->last = NULL;

    if (queue->last)
        queue->last->next_list = new;
    else
        queue->first = new;
    queue->last = new;

    return SR_ERR_OK;
}

/**
 * @brief Pop oldest config-change reset list from queue.
 *
 * @param[in]   queue           Queue to pop.
 * @return      List or NULL if queue was empty.
 */
static struct config_change_list *config_change_queue_pop(
        struct config_change_queue *queue)
{
    struct config_change_list *item = queue->first;

    if (!item)
        return NULL;

    queue->first = item->next_list;
    if (!queue->first)
        queue->last = NULL;

    item->next_list = NULL;
    return item;
}

/**
 * @brief Clear config-change reset list queue.
 *
 * This removes and frees all config-change reset lists from queue.
 *
 * @param[in]   queue           Queue to empty.
 */
static void config_change_queue_clear(struct config_change_queue *queue)
{
    struct config_change_list *next;

    for (;;) {
        next = config_change_queue_pop(queue);
        if (!next)
            break;
        free(next);
    }
}

void config_change_list_enqueue(struct config_change_list *list)
{
    SRP_LOG_DBG("%s() check if list contains anything to do", __func__);

    if (config_change_list_empty(list))
        return;

    bool trigger_any = false;
    struct config_change_reset *item;
    for (item = list->first; item; item = item->next) {
        if (item->value) {
            trigger_any = true;
            break;
        }
    }

    SRP_LOG_DBG("%s() list %s triggers", __func__,
                trigger_any ? "contains" : "does not contain");

    if (!trigger_any) {
        config_change_list_clear(list);
        return;
    }

    struct config_change_helper *helper = config_change_helper_get();

    pthread_mutex_lock(&helper->lock);

    int ret = config_change_queue_push(&helper->queue, list);
    if (ret == SR_ERR_OK)
        pthread_cond_signal(&helper->cond);

    pthread_mutex_unlock(&helper->lock);

    if (ret != SR_ERR_OK)
        SRP_LOG_WRN("%s() failed to inform helper thread", __func__);
}

static void config_change_list_process_one(sr_session_ctx_t *session,
                                           const char *xpath)
{
    SRP_LOG_DBG("%s() reset %s", __func__, xpath);

    sr_val_t val = {
        .xpath = NULL,
        .type = SR_BOOL_T,
        .data.bool_val = false,
    };

    int ret = sr_set_item(session, xpath, &val, SR_EDIT_NON_RECURSIVE);
    if (ret != SR_ERR_OK) {
        SRP_LOG_ERR("%s() sr_set_item failed for %s with %d %s",
                    __func__, xpath, ret, sr_strerror(ret));
    }
}

void config_change_list_process(sr_session_ctx_t *session,
                                struct config_change_list *list)
{
    bool apply_needed = false;
    struct config_change_reset *item;
    int ret;

    SRP_LOG_DBG("%s() start processing new list", __func__);

    for (;;) {
        item = config_change_list_pop(list);
        if (!item)
            break;

        if (item->value) {
            config_change_list_process_one(session, item->xpath);
            apply_needed = true;
        }
        free(item);
    }

    if (apply_needed) {
        SRP_LOG_DBG("%s() apply config-change reset changes", __func__);
        ret = sr_apply_changes(session, 0, 0);
        if (ret != SR_ERR_OK) {
            SRP_LOG_WRN("%s() failed to apply config-change resets to datastore",
                        __func__);
        }
    }

    SRP_LOG_DBG("%s() list processed", __func__);
}

static void config_change_process(struct config_change_helper *helper)
{
    struct config_change_list *list;

    SRP_LOG_DBG("%s()", __func__);

    for (;;) {
        list = config_change_queue_pop(&helper->queue);
        if (!list)
            break;

        pthread_mutex_unlock(&helper->lock);

        config_change_list_process(helper->session, list);
        free(list);

        pthread_mutex_lock(&helper->lock);
    }
}

static void *config_change_helper_thread(void *arg)
{
    struct config_change_helper *helper = arg;

    SRP_LOG_DBG("%s() started", __func__);

    pthread_mutex_lock(&helper->lock);

    for (;;) {
        pthread_cond_wait(&helper->cond, &helper->lock);
        SRP_LOG_DBG("%s() thread wakeup", __func__);
        if (helper->users == 0)
            break;

        if (!config_change_queue_empty(&helper->queue))
            config_change_process(helper);
    }

    config_change_queue_clear(&helper->queue);

    pthread_mutex_unlock(&helper->lock);

    SRP_LOG_DBG("%s() terminating", __func__);

    return NULL;
}

int config_change_helper_start(void)
{
    struct config_change_helper *helper = config_change_helper_get();

    SRP_LOG_DBG("%s() start", __func__);

    pthread_mutex_lock(&helper->lock);

    if (helper->users++ > 0)
        goto done;

    int ret = sr_connect(SR_CONN_DEFAULT | SR_CONN_AVOID_DEADLOCK,
                         &helper->conn);
    if (ret != SR_ERR_OK) {
        SRP_LOG_ERR("%s() sr_connect failed with %d %s",
                    __func__, ret, sr_strerror(ret));
        goto err_connect;
    }

    ret = sr_session_start(helper->conn, SR_DS_RUNNING, &helper->session);
    if (ret != SR_ERR_OK) {
        SRP_LOG_ERR("%s() sr_session_start failed with %d %s",
                    __func__, ret, sr_strerror(ret));
        goto err_session;
    }

    ret = pthread_create(&helper->thread, NULL,
                         &config_change_helper_thread, helper);
    if (ret) {
        SRP_LOG_ERR("%s() failed to start thread", __func__);
        goto err_thread;
    }

done:
    pthread_mutex_unlock(&helper->lock);
    return SR_ERR_OK;

err_thread:
    sr_session_stop(helper->session);
    helper->session = NULL;
err_session:
    sr_disconnect(helper->conn);
    helper->conn = NULL;
err_connect:
    helper->users--;
    pthread_mutex_unlock(&helper->lock);
    return ret;
}

void config_change_helper_stop(void)
{
    struct config_change_helper *helper = config_change_helper_get();

    SRP_LOG_DBG("%s() stop", __func__);

    pthread_mutex_lock(&helper->lock);

    if (helper->users == 0 || --helper->users > 0) {
        pthread_mutex_unlock(&helper->lock);
        return;
    }

    pthread_cond_signal(&helper->cond);
    pthread_mutex_unlock(&helper->lock);

    pthread_join(helper->thread, NULL);

    sr_session_stop(helper->session);
    helper->session = NULL;

    sr_disconnect(helper->conn);
    helper->conn = NULL;
}
