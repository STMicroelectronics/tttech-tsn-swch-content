/*
 * TTTech sysrepo-plugins common defines
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

#include "common.h"

extern int plugin_init;

int get_key_value(sr_session_ctx_t *session, char *xpath, char *node_name,
                  char *key_name, sr_xpath_ctx_t *st, char *result)
{
    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    if (NULL == sr_xpath_key_value(xpath, node_name, key_name, st)) {
        sr_xpath_recover(st);
        SRP_LOG_ERR(ERR_KEY_VALUE_ERR_XPATH_STR, key_name, node_name, xpath);
        if (NULL != session) {
            sr_set_error(session, xpath, ERR_KEY_VALUE_ERR_XPATH_STR,
                         key_name, node_name, xpath);
        }
        sr_xpath_recover(st);
        return EXIT_FAILURE;
    }

    sr_xpath_recover(st);
    strncpy(result, sr_xpath_key_value(xpath, node_name, key_name, st),
            MAX_STR_LEN - 1);
    sr_xpath_recover(st);

    return EXIT_SUCCESS;
}

int new_node(struct lyd_node *parent, const struct lys_module *module,
             char *name, char *value)
{
    struct lyd_node *node = NULL;

    SRP_LOG_DBG(DBG_FUNCTION_CALL_MSG_STR, __func__, name);

    node = lyd_new_leaf(parent, module, name, value);
    if (NULL == node) {
        SRP_LOG_ERR(ERROR_MSG_FUN_XML_EL_AND_MSG, __func__,
                    name, ERR_NOT_CREATED_ELEMENT_STR);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int new_container(struct lyd_node *parent, struct lyd_node** container,
                  const struct lys_module *module, char *container_name)
{
    SRP_LOG_DBG(DBG_FUNCTION_CALL_MSG_STR, __func__, container_name);

    *container = lyd_new(parent, module, container_name);
    if (NULL == *container) {
        SRP_LOG_ERR(ERROR_MSG_FUN_XML_EL_AND_MSG, __func__,
                    container_name, ERR_NOT_CREATED_ROOT_ELEMENT_STR);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int create_root(struct ly_ctx **ctx, struct lyd_node **root,
                const struct lys_module **module, char *module_name,
                char *root_name, char *repository_path)
{
    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    *ctx = ly_ctx_new(repository_path, 0);
    if (NULL == *ctx) {
        SRP_LOG_ERR(ERROR_MSG_FUN_AND_MSG, __func__,
                    ERR_LIBYANG_CONTEXT_HANDLER_STR);
        return EXIT_FAILURE;
    }
    /* Print error only. If ctx is NULL then root will be NULL and error
     * message for root will print to stdout.
     */
    *module = ly_ctx_load_module(*ctx, module_name, NULL);

    *root = lyd_new(NULL, *module, root_name);
    if (NULL == *root) {
        SRP_LOG_ERR(ERROR_MSG_MOD_FUNC_MSG, module_name, __func__,
                    ERR_NOT_CREATED_ROOT_ELEMENT_STR);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int create_list_keys(struct lyd_node *parent, const struct lys_module *module,
                     char key_name[][MAX_STR_LEN],
                     char key_value[][MAX_STR_LEN], int key_num)
{
    int i = 0;

    for (i = 0; i < key_num; i++) {
        if (EXIT_FAILURE == new_node(parent, module,
                                     key_name[i], key_value[i])) {
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}

int check_file(char *path_to_file)
{
    FILE *fp = NULL;
    long int size = 0;

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    fp = fopen(path_to_file, "r");

    if (NULL != fp) {
        fseek(fp, 0, SEEK_END);
        size = ftell(fp);
        fclose(fp);
    } else {
        SRP_LOG_ERR(ERROR_MSG_FUN_AND_MSG, __func__,
                    ERR_MSG_FILE_OPEN_FAILED_STR);
        size = -1;
    }
    return size;
}

void fill_xpath(char *result, const char *format, ...)
{
    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    va_list args;
    va_start(args, format);
    vsnprintf(result, 2*MAX_STR_LEN, format, args);
    va_end(args);
}

int mod_not_supported_cb(sr_session_ctx_t *session, const char *module_name,
                         const char *xpath, sr_event_t event,
                         uint32_t request_id, void *private_data)
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
        if ((op == SR_OP_MODIFIED)) {
            SRP_LOG_ERR(ERR_MOD_OP_NOT_SUPPORTED_XPATH_STR, xpath);
            sr_set_error(session, xpath,
                         ERR_MOD_OP_NOT_SUPPORTED_XPATH_STR, xpath);
            return SR_ERR_OPERATION_FAILED;
        }
    }

    sr_free_val(old_value);
    sr_free_val(new_value);
    sr_free_change_iter(iter);

    return SR_ERR_OK;
}

int add_rem_mod_not_supported_cb(sr_session_ctx_t *session,
                                 const char *module_name, const char *xpath,
                                 sr_event_t event, uint32_t request_id,
                                 void *private_data)
{
    (void)module_name;
    (void)request_id;
    (void)private_data;

    int rc;
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

        if (((op == SR_OP_MODIFIED) ||
             (op == SR_OP_CREATED) ||
             (op == SR_OP_DELETED)) &&
            (plugin_init != 1)) {
            SRP_LOG_ERR(ERR_ADD_REM_MOD_OP_NOT_SUPPORTED_XPATH_STR,
                        node->xpath);
            sr_set_error(session, node->xpath,
                         ERR_ADD_REM_MOD_OP_NOT_SUPPORTED_XPATH_STR,
                         node->xpath);
            return SR_ERR_OPERATION_FAILED;
        }
    }

    sr_free_val(old_value);
    sr_free_val(new_value);
    sr_free_change_iter(iter);
    node = NULL;

    return SR_ERR_OK;
}

int get_item_value_cb(sr_session_ctx_t *session, sr_val_t **value,
                      const char *xpath_format, ...)
{
    char tmp_path[2*MAX_STR_LEN] = "";

    SRP_LOG_DBG(DBG_MSG_FUN_CALLED_STR, __func__);

    va_list args;
    va_start(args, xpath_format);
    vsnprintf(tmp_path, 2*MAX_STR_LEN, xpath_format, args);
    va_end(args);

    if (SR_ERR_OK != sr_get_item(session, tmp_path, 0, value)) {
        SRP_LOG_ERR(ERR_FAILED_TO_GET_ITEM_XPATH_STR, tmp_path);
        sr_set_error(session, tmp_path, ERR_FAILED_TO_GET_ITEM_XPATH_STR,
                     tmp_path);
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

int translate_string_to_bool(char value[MAX_STR_LEN],
                             char map[][2][MAX_STR_LEN], bool *bvalue)
{
    int ret_value = EXIT_SUCCESS;

    SRP_LOG_DBG(DEBUG_MSG_FUN_CALLED, __func__);

    if (EXIT_FAILURE == translate_single_string_using_map(value, map)) {
        return EXIT_FAILURE;
    }
    if (0 == strncmp(BASE_TRUE, value, MAX_STR_LEN)) {
        *bvalue = true;
    }

    return ret_value;
}

int translate_strings_using_map(char values[][MAX_STR_LEN],
                                char map[][2][MAX_STR_LEN])
{
    int ret_value = EXIT_SUCCESS;
    int i = 0;
    int map_pair = 0;
    int flag = 1;

    SRP_LOG_DBG(DEBUG_MSG_FUN_CALLED, __func__);

    while ((0 != strncmp(values[i], "", MAX_STR_LEN))) {
        while ((0 != strcmp(map[map_pair][0], "")) &&
               (0 != strncmp(map[map_pair][1], "", MAX_STR_LEN))) {
            if (0 == strncmp(values[i], map[map_pair][0], MAX_STR_LEN)) {
                // first string from the map is key
                strncpy(values[i], map[map_pair][1], MAX_STR_LEN);
                // second string is new mapped value
                flag = 0;
                break;
            }
            map_pair++;
        }
        if (1 == flag) {
            // last values element did not match any of map keys
            ret_value = EXIT_FAILURE;
            SRP_LOG_ERR("%s(): (%s) element did not match any of map keys.",
                        __func__, values[i]);
            break;
        }
        flag = 1; // reset flag for next values element
        i++; // next values element
    }

    return ret_value;
}

void stringify_mac_addr(uint8_t mac_addr_b[MAC_ADDR_B_LEN],
                        char *mac_addr_str, char delim)
{
    int i = 0;
    int step = 3;
    char format[6] = "";

    if ('\0' != delim) {
        step = 3;
        snprintf(format, 6, "%%02x%c", delim);
    } else {
        step = 2;
        strncpy(format, "%02x", 6);
    }

    for (i = 0; i < MAC_ADDR_B_LEN - 1; i++) {
        snprintf(mac_addr_str + step*i, 4, format, mac_addr_b[i]);
    }
    snprintf(mac_addr_str + step*i, 3, "%02x", mac_addr_b[i]);
}
