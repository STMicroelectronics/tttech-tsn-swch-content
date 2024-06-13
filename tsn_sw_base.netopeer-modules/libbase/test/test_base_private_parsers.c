/*
 * TTTech libbase
 * Copyright(c) 2018 TTTech Computertechnik AG.
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

#include "unity.h"
#include "base_private_parsers.h"

#include "mock_base_prints.h"
#include "mock_base_private_syscalls.h"
#include "mock_base_linux_tool_helpers.h"

void setUp(void)
{
    base_print_error_CMockIgnore();
}


////*****TESTS for find_on_line*****////
//TEST - input_str is same as match case
void  test_001_find_on_line(void)
{
    module_index_t module_index = MSTP;//valid
    int obj_index    = MSTP_PORT_NAMES;//valid
    int comb_index   = 1;//valid, 1 is from configuration
    int parser_index = 0;//valid, 0 is from configuration
    char* input_str = NULL; //invalid, unused
    char result_array[MAX_NUM_OF_INTERFACES][MAX_STR_LEN];//irelevant for this test case
    sys_call_spec_t cmock_to_return;

    //sys_call_spec_t need to be set manually
    memset(&cmock_to_return, 0, sizeof(sys_call_spec_t));//clear, somehow testing framework keeps old ????
    cmock_to_return.sys_call = NULL;       //irelevant for this test
    cmock_to_return.sys_call_command = ""; //irelevant for this test
    cmock_to_return.num_of_combinators = 2;//from configuration
    cmock_to_return.combinator = NULL;     //irelevant for this test
    int mstp_num_of_cb_tmp[2] = {0, 2};    //taken from configuration
    memcpy(cmock_to_return.num_of_cb, mstp_num_of_cb_tmp, sizeof(int)*2*2);
    memcpy(cmock_to_return.parse_cb[1][0].match_case, CIST_INFO_STR, MAX_STR_LEN);

    //sys_call_spec_t setting is done
    input_str = (char*) malloc(sizeof(char)*MAX_STR_LEN);
    strncpy(input_str, CIST_INFO_STR, MAX_STR_LEN);

    //mocks
    assemble_sys_call_structure_CMockIgnoreAndReturn(0, cmock_to_return);

    base_status_t a = find_on_line(module_index, obj_index, comb_index, parser_index, input_str, 0, NULL, 0, 0);
    TEST_ASSERT_EQUAL_INT_MESSAGE(BASE_PARSING_SUCCEEDED, a, "===>>> [input_str]= CIST_INFO_STR, [match_case] = CIST_INFO_STR ===>>> SHOULD RETURN: BASE_PARSING_SUCCEEDED, because input_str is same as match case ");
}

//TEST - input_str is not same as match case
void  test_002_find_on_line(void)
{
    module_index_t module_index = MSTP;//valid
    int obj_index    = MSTP_PORT_NAMES;//valid
    int comb_index   = 1;//valid, 1 is from configuraion
    int parser_index = 0;//valid, 0 is from configuration
    char* input_str = NULL; //invalid, unused
    char result_array[MAX_NUM_OF_INTERFACES][MAX_STR_LEN];//irelevant for this test case
    sys_call_spec_t cmock_to_return;

    //sys_call_spec_t need to be set manually
    memset(&cmock_to_return, 0, sizeof(sys_call_spec_t));//clear, somehow testing framework keeps old ????
    cmock_to_return.sys_call = NULL;       //irelevant for this test
    cmock_to_return.sys_call_command = ""; //irelevant for this test
    cmock_to_return.num_of_combinators = 2;//from configuration
    cmock_to_return.combinator = NULL;     //irelevant for this test
    int mstp_num_of_cb_tmp[2] = {0, 2};    //taken from configuration
    memcpy(cmock_to_return.num_of_cb, mstp_num_of_cb_tmp, sizeof(int)*4);
    memcpy(cmock_to_return.parse_cb[comb_index][parser_index].match_case, CIST_INFO_STR, MAX_STR_LEN);
    //sys_call_spec_t setting is done
    input_str = (char*) malloc(sizeof(char)*MAX_STR_LEN);
    strncpy(input_str, "nothing", MAX_STR_LEN);//different than match case

    //mocks
    assemble_sys_call_structure_CMockIgnoreAndReturn(0, cmock_to_return);

    base_status_t a = find_on_line(module_index, obj_index, comb_index, parser_index, input_str, 0, NULL, 0, 0);
    TEST_ASSERT_EQUAL_INT_MESSAGE(BASE_PARSING_FAILED, a, "===>>> [input_str]= \"nothing\", [match_case] = CIST_INFO_STR ===>>> SHOULD RETURN: BASE_PARSING_FAILED, because input_str is not same as match case ");
}

//TEST - assemble_sys_call_structure returns structure filled with 0s
void  test_003_find_on_line(void)
{
    module_index_t module_index = MSTP;//valid
    int obj_index    = MSTP_PORT_NAMES;//valid
    int comb_index   = 1;//valid, 1 is from configuraion
    int parser_index = 0;//valid, 0 is from configuration
    char* input_str = NULL; //invalid, unused
    char result_array[MAX_NUM_OF_INTERFACES][MAX_STR_LEN];//irelevant for this test case
    sys_call_spec_t cmock_to_return;

    //sys_call_spec_t need to be set manually
    memset(&cmock_to_return, 0, sizeof(cmock_to_return));
    //sys_call_spec_t setting is done
    input_str = (char*) malloc(sizeof(char)*MAX_STR_LEN);
    strncpy(input_str, CIST_INFO_STR, MAX_STR_LEN);//different than match case

    //mocks
    assemble_sys_call_structure_CMockIgnoreAndReturn(0, cmock_to_return);

    base_status_t a = find_on_line(module_index, obj_index, comb_index, parser_index, input_str, 0, NULL, 0, 0);
    TEST_ASSERT_EQUAL_INT_MESSAGE(BASE_PARSING_FAILED, a, "===>>> [input_str]= CIST_INFO_STR ===>>> SHOULD RETURN: BASE_PARSING_FAILED, because assemble_sys_call_structure returns structure filled with 0s ");
}

//TEST - match case is ""
void  test_004_find_on_line(void)
{
    module_index_t module_index = MSTP;//valid
    int obj_index    = MSTP_PORT_NAMES;//valid
    int comb_index   = 1;//valid, 1 is from configuraion
    int parser_index = 0;//valid, 0 is from configuration
    char* input_str = NULL; //invalid, unused
    char result_array[MAX_NUM_OF_INTERFACES][MAX_STR_LEN];//irelevant for this test case
    sys_call_spec_t cmock_to_return;

    //sys_call_spec_t need to be set manually
    memset(&cmock_to_return, 0, sizeof(sys_call_spec_t));//clear, somehow testing framework keeps old ????
    cmock_to_return.sys_call = NULL;       //irelevant for this test
    cmock_to_return.sys_call_command = ""; //irelevant for this test
    cmock_to_return.num_of_combinators = 2;//from configuration
    cmock_to_return.combinator = NULL;     //irelevant for this test
    int mstp_num_of_cb_tmp[2] = {0, 2};    //taken from configuration
    memcpy(cmock_to_return.num_of_cb, mstp_num_of_cb_tmp, sizeof(int)*4);
    memcpy(cmock_to_return.parse_cb[comb_index][parser_index].match_case, "", MAX_STR_LEN);
    //sys_call_spec_t setting is done
    input_str = (char*) malloc(sizeof(char)*MAX_STR_LEN);
    strncpy(input_str, CIST_INFO_STR, MAX_STR_LEN);//different than match case

    //mocks
    assemble_sys_call_structure_CMockIgnoreAndReturn(0, cmock_to_return);

    base_status_t a = find_on_line(module_index, obj_index, comb_index, parser_index, input_str, 0, NULL, 0, 0);
    TEST_ASSERT_EQUAL_INT_MESSAGE(BASE_PARSING_FAILED, a, "===>>> [input_str]= CIST_INFO_STR, [match_case] = \"\" ===>>> SHOULD RETURN: BASE_PARSING_FAILED, because match case is \"\"");
}

//TEST - input_str is empty string
void  test_005_find_on_line(void)
{
    module_index_t module_index = MSTP;//valid
    int obj_index    = MSTP_PORT_NAMES;//valid
    int comb_index   = 1;//valid, 1 is from configuraion
    int parser_index = 0;//valid, 0 is from configuration
    char* input_str = NULL; //invalid, unused
    char result_array[MAX_NUM_OF_INTERFACES][MAX_STR_LEN];//irelevant for this test case
    sys_call_spec_t cmock_to_return;

    //sys_call_spec_t need to be set manually
    memset(&cmock_to_return, 0, sizeof(sys_call_spec_t));//clear, somehow testing framework keeps old ????
    cmock_to_return.sys_call = NULL;       //irelevant for this test
    cmock_to_return.sys_call_command = ""; //irelevant for this test
    cmock_to_return.num_of_combinators = 2;//from configuration
    cmock_to_return.combinator = NULL;     //irelevant for this test
    int mstp_num_of_cb_tmp[2] = {0, 2};    //taken from configuration
    memcpy(cmock_to_return.num_of_cb, mstp_num_of_cb_tmp, sizeof(int)*4);
    memcpy(cmock_to_return.parse_cb[comb_index][parser_index].match_case, CIST_INFO_STR, MAX_STR_LEN);
    //sys_call_spec_t setting is done
    input_str = (char*) malloc(sizeof(char)*MAX_STR_LEN);
    strncpy(input_str, "", MAX_STR_LEN);//different than match case

    //mocks
    assemble_sys_call_structure_CMockIgnoreAndReturn(0, cmock_to_return);

    base_status_t a = find_on_line(module_index, obj_index, comb_index, parser_index, input_str, 0, NULL, 0, 0);
    TEST_ASSERT_EQUAL_INT_MESSAGE(BASE_PARSING_FAILED, a, "===>>> [input_str]= \"\", [match_case] = CIST_INFO_STR ===>>> SHOULD RETURN: BASE_PARSING_FAILED, because input_str is empty string");
}
////*****END OF TESTING find_on_line*****////


////*****TESTS for is_not_on_line*****////
//TEST - input_str is same as match case
void  test_001_is_not_on_line(void)
{
    module_index_t module_index = MSTP;//valid
    int obj_index    = MSTP_PORT_NAMES;//valid
    int comb_index   = 1;//valid, 1 is from configuration
    int parser_index = 0;//valid, 0 is from configuration
    char* input_str = NULL; //invalid, unused
    char result_array[MAX_NUM_OF_INTERFACES][MAX_STR_LEN];//irelevant for this test case
    sys_call_spec_t cmock_to_return;

    //sys_call_spec_t need to be set manually
    memset(&cmock_to_return, 0, sizeof(sys_call_spec_t));//clear, somehow testing framework keeps old ????
    cmock_to_return.sys_call = NULL;       //irelevant for this test
    cmock_to_return.sys_call_command = ""; //irelevant for this test
    cmock_to_return.num_of_combinators = 2;//from configuration
    cmock_to_return.combinator = NULL;     //irelevant for this test
    int mstp_num_of_cb_tmp[2] = {0, 2};    //taken from configuration
    memcpy(cmock_to_return.num_of_cb, mstp_num_of_cb_tmp, sizeof(int)*2*2);
    memcpy(cmock_to_return.parse_cb[comb_index][parser_index].match_case, CIST_INFO_STR, MAX_STR_LEN);

    //sys_call_spec_t setting is done
    input_str = (char*) malloc(sizeof(char)*MAX_STR_LEN);
    strncpy(input_str, CIST_INFO_STR, MAX_STR_LEN);

    //mocks
    assemble_sys_call_structure_CMockIgnoreAndReturn(0, cmock_to_return);

    base_status_t a = is_not_on_line(module_index, obj_index, comb_index, parser_index, input_str, 0, NULL, 0, 0);
    TEST_ASSERT_EQUAL_INT_MESSAGE(BASE_PARSING_FAILED, a, "===>>> [input_str]= CIST_INFO_STR, [match_case] = CIST_INFO_STR ===>>> SHOULD RETURN: BASE_PARSING_SUCCEEDED, because input_str is same as match case ");
}

//TEST - input_str is not same as match case
void  test_002_is_not_on_line(void)
{
    module_index_t module_index = MSTP;//valid
    int obj_index    = MSTP_PORT_NAMES;//valid
    int comb_index   = 1;//valid, 1 is from configuraion
    int parser_index = 0;//valid, 0 is from configuration
    char* input_str = NULL; //invalid, unused
    char result_array[MAX_NUM_OF_INTERFACES][MAX_STR_LEN];//irelevant for this test casee
    sys_call_spec_t cmock_to_return;

    //sys_call_spec_t need to be set manually
    memset(&cmock_to_return, 0, sizeof(sys_call_spec_t));//clear, somehow testing framework keeps old ????
    cmock_to_return.sys_call = NULL;       //irelevant for this test
    cmock_to_return.sys_call_command = ""; //irelevant for this test
    cmock_to_return.num_of_combinators = 2;//from configuration
    cmock_to_return.combinator = NULL;     //irelevant for this test
    int mstp_num_of_cb_tmp[2] = {0, 2};    //taken from configuration
    memcpy(cmock_to_return.num_of_cb, mstp_num_of_cb_tmp, sizeof(int)*4);
    memcpy(cmock_to_return.parse_cb[1][0].match_case, CIST_INFO_STR, MAX_STR_LEN);
    //sys_call_spec_t setting is done
    input_str = (char*) malloc(sizeof(char)*MAX_STR_LEN);
    strncpy(input_str, "nothing", MAX_STR_LEN);//different than match case

    //mocks
    assemble_sys_call_structure_CMockIgnoreAndReturn(0, cmock_to_return);

    base_status_t a = is_not_on_line(module_index, obj_index, comb_index, parser_index, input_str, 0, NULL, 0, 0);
    TEST_ASSERT_EQUAL_INT_MESSAGE(BASE_PARSING_SUCCEEDED, a, "===>>> [input_str]= \"nothing\", [match_case] = CIST_INFO_STR ===>>> SHOULD RETURN: BASE_PARSING_FAILED, because input_str is not same as match case ");
}

//TEST - assemble_sys_call_structure returns structure filled with 0s
void  test_003_is_not_on_line(void)
{
    module_index_t module_index = MSTP;//valid
    int obj_index    = MSTP_PORT_NAMES;//valid
    int comb_index   = 1;//valid, 1 is from configuraion
    int parser_index = 0;//valid, 0 is from configuration
    char* input_str = NULL; //invalid, unused
    char result_array[MAX_NUM_OF_INTERFACES][MAX_STR_LEN];//irelevant for this test case
    sys_call_spec_t cmock_to_return;

    //sys_call_spec_t need to be set manually
    memset(&cmock_to_return, 0, sizeof(cmock_to_return));
    //sys_call_spec_t setting is done
    input_str = (char*) malloc(sizeof(char)*MAX_STR_LEN);
    strncpy(input_str, CIST_INFO_STR, MAX_STR_LEN);//different than match case

    //mocks
    assemble_sys_call_structure_CMockIgnoreAndReturn(0, cmock_to_return);

    base_status_t a = is_not_on_line(module_index, obj_index, comb_index, parser_index, input_str, 0, NULL, 0, 0);
    TEST_ASSERT_EQUAL_INT_MESSAGE(BASE_PARSING_SUCCEEDED, a, "===>>> [input_str]= CIST_INFO_STR ===>>> SHOULD RETURN: BASE_PARSING_FAILED, because assemble_sys_call_structure returns structure filled with 0s ");
}

//TEST - match case is ""
void  test_004_is_not_on_line(void)
{
    module_index_t module_index = MSTP;//valid
    int obj_index    = MSTP_PORT_NAMES;//valid
    int comb_index   = 1;//valid, 1 is from configuraion
    int parser_index = 0;//valid, 0 is from configuration
    char* input_str = NULL; //invalid, unused
    char result_array[MAX_NUM_OF_INTERFACES][MAX_STR_LEN];//irelevant for this test case
    sys_call_spec_t cmock_to_return;

    //sys_call_spec_t need to be set manually
    memset(&cmock_to_return, 0, sizeof(sys_call_spec_t));//clear, somehow testing framework keeps old ????
    cmock_to_return.sys_call = NULL;       //irelevant for this test
    cmock_to_return.sys_call_command = ""; //irelevant for this test
    cmock_to_return.num_of_combinators = 2;//from configuration
    cmock_to_return.combinator = NULL;     //irelevant for this test
    int mstp_num_of_cb_tmp[2] = {0, 2};    //taken from configuration
    memcpy(cmock_to_return.num_of_cb, mstp_num_of_cb_tmp, sizeof(int)*4);
    memcpy(cmock_to_return.parse_cb[comb_index][parser_index].match_case, "", MAX_STR_LEN);
    //sys_call_spec_t setting is done
    input_str = (char*) malloc(sizeof(char)*MAX_STR_LEN);
    strncpy(input_str, CIST_INFO_STR, MAX_STR_LEN);//different than match case

    //mocks
    assemble_sys_call_structure_CMockIgnoreAndReturn(0, cmock_to_return);

    base_status_t a = is_not_on_line(module_index, obj_index, comb_index, parser_index, input_str, 0, NULL, 0, 0);
    TEST_ASSERT_EQUAL_INT_MESSAGE(BASE_PARSING_SUCCEEDED, a, "===>>> [input_str]= CIST_INFO_STR, [match_case] = \"\" ===>>> SHOULD RETURN: BASE_PARSING_FAILED, because match case is \"\"");
}

//TEST - input_str is empty string
void  test_005_is_not_on_line(void)
{
    module_index_t module_index = MSTP;//valid
    int obj_index    = MSTP_PORT_NAMES;//valid
    int comb_index   = 1;//valid, 1 is from configuraion
    int parser_index = 0;//valid, 0 is from configuration
    char* input_str = NULL; //invalid, unused
    char result_array[MAX_NUM_OF_INTERFACES][MAX_STR_LEN];//irelevant for this test case
    sys_call_spec_t cmock_to_return;

    //sys_call_spec_t need to be set manually
    memset(&cmock_to_return, 0, sizeof(sys_call_spec_t));//clear, somehow testing framework keeps old ????
    cmock_to_return.sys_call = NULL;       //irelevant for this test
    cmock_to_return.sys_call_command = ""; //irelevant for this test
    cmock_to_return.num_of_combinators = 2;//from configuration
    cmock_to_return.combinator = NULL;     //irelevant for this test
    int mstp_num_of_cb_tmp[2] = {0, 2};    //taken from configuration
    memcpy(cmock_to_return.num_of_cb, mstp_num_of_cb_tmp, sizeof(int)*4);
    memcpy(cmock_to_return.parse_cb[comb_index][parser_index].match_case, CIST_INFO_STR, MAX_STR_LEN);
    //sys_call_spec_t setting is done
    input_str = (char*) malloc(sizeof(char)*MAX_STR_LEN);
    strncpy(input_str, "", MAX_STR_LEN);//different than match case

    //mocks
    assemble_sys_call_structure_CMockIgnoreAndReturn(0, cmock_to_return);

    base_status_t a = is_not_on_line(module_index, obj_index, comb_index, parser_index, input_str, 0, NULL, 0, 0);
    TEST_ASSERT_EQUAL_INT_MESSAGE(BASE_PARSING_SUCCEEDED, a, "===>>> [input_str]= \"\", [match_case] = CIST_INFO_STR ===>>> SHOULD RETURN: BASE_PARSING_FAILED, because input_str is empty string");
}
////*****END OF TESTING is_not_on_line*****////

////*****TESTS for find_on_line_and_fill_result_array*****////
//TEST - find_on_line_helper returns BASE_PARSING_SUCCEEDED
void  test_001_find_on_line_and_fill_result_array(void)
{
	module_index_t module_index = IETF_INTERFACES;//valid
	int obj_index    = IF_ENABLED;//valid
	int comb_index   = 1;//valid, 1 is from configuraion
	int parser_index = 0;//valid, 0 is from configuration
	char* input_str = NULL; //invalid, unused
	char result_array[MAX_NUM_OF_INTERFACES][MAX_STR_LEN];//irelevant for this test case
    sys_call_spec_t cmock_to_return;

    //sys_call_spec_t need to be set manually
    memset(&cmock_to_return, 0, sizeof(sys_call_spec_t));//clear, somehow testing framework keeps old ????
    cmock_to_return.sys_call = NULL;       //irelevant for this test
    cmock_to_return.sys_call_command = ""; //irelevant for this test
    cmock_to_return.num_of_combinators = 5;//from configuration
    cmock_to_return.combinator = NULL;     //irelevant for this test
    int mstp_num_of_cb_tmp[5] = {0, 2, 1, 1, 1};    //taken from configuration
    memcpy(cmock_to_return.num_of_cb, mstp_num_of_cb_tmp, sizeof(int)*5);
    memcpy(cmock_to_return.parse_cb[comb_index][parser_index].match_case, BASE_LOWER_UP, MAX_STR_LEN);
    //sys_call_spec_t setting is done
    input_str = (char*) malloc(sizeof(char)*MAX_STR_LEN);
    strncpy(input_str, "LOWER_UP and more", MAX_STR_LEN);
    memset(&result_array, 0, sizeof(char)*MAX_NUM_OF_INTERFACES*MAX_STR_LEN);//clear result_array, somehow testing framework keeps old ????

    //mocks
    assemble_sys_call_structure_CMockIgnoreAndReturn(0, cmock_to_return);
    assemble_sys_call_structure_CMockIgnoreAndReturn(1, cmock_to_return);// second call

    base_status_t a = find_on_line_and_fill_result_array(module_index, obj_index, comb_index, parser_index, input_str, 0, result_array, MAX_NUM_OF_INTERFACES, MAX_STR_LEN);
    //check result_array[0]
    if(0 != strncmp(BASE_LOWER_UP, result_array[0], MAX_STR_LEN))
    {
        a = BASE_PARSING_FAILED;//direct fail if result is not good
    }
    TEST_ASSERT_EQUAL_INT_MESSAGE(BASE_PARSING_SUCCEEDED, a, "===>>> [find_on_line_helper returns BASE_PARSING_SUCCEEDED] = BASE_LOWER_UP ===>>> SHOULD RETURN: BASE_PARSING_FAILED, because mocking of functions in same file is not possible");
}

//TEST - find_on_line_helper returns BASE_PARSING_FAILED
void  test_002_find_on_line_and_fill_result_array(void)
{
	module_index_t module_index = IETF_INTERFACES;//valid
	int obj_index    = IF_ENABLED;//valid
	int comb_index   = 1;//valid, 1 is from configuraion
	int parser_index = 0;//valid, 0 is from configuration
	char* input_str = NULL; //invalid, unused
    char result_array[MAX_NUM_OF_INTERFACES][MAX_STR_LEN];//irelevant for this test case
    sys_call_spec_t cmock_to_return;

    //sys_call_spec_t need to be set manually
    memset(&cmock_to_return, 0, sizeof(sys_call_spec_t));//clear, somehow testing framework keeps old ????
    cmock_to_return.sys_call = NULL;       //irelevant for this test
    cmock_to_return.sys_call_command = ""; //irelevant for this test
    cmock_to_return.num_of_combinators = 5;//from configuration
    cmock_to_return.combinator = NULL;     //irelevant for this test
    int mstp_num_of_cb_tmp[5] = {0, 2, 1, 1, 1};    //taken from configuration
    memcpy(cmock_to_return.num_of_cb, mstp_num_of_cb_tmp, sizeof(int)*5);
    memcpy(cmock_to_return.parse_cb[comb_index][parser_index].match_case, BASE_LOWER_UP, MAX_STR_LEN);
    //sys_call_spec_t setting is done
    input_str = (char*) malloc(sizeof(char)*MAX_STR_LEN);
    strncpy(input_str, "without match case", MAX_STR_LEN);
    memset(&result_array, 0, sizeof(char)*MAX_NUM_OF_INTERFACES*MAX_STR_LEN);//clear result_array, somehow testing framework keeps old ????

    //mocks
    assemble_sys_call_structure_CMockIgnoreAndReturn(0, cmock_to_return);
    assemble_sys_call_structure_CMockIgnoreAndReturn(1, cmock_to_return);// second call

    base_status_t a = find_on_line_and_fill_result_array(module_index, obj_index, comb_index, parser_index, input_str, 0, result_array, MAX_NUM_OF_INTERFACES, MAX_STR_LEN);
    //check result_array[0]
    if(0 != strncmp("", result_array[0], MAX_STR_LEN))
    {
        a = BASE_PARSING_FAILED;//direct fail if result is not good
    }
    TEST_ASSERT_EQUAL_INT_MESSAGE(BASE_PARSING_FAILED, a, "===>>> [find_on_line_helper returns BASE_PARSING_FAILED] ===>>> SHOULD RETURN: BASE_PARSING_SUCCEEDED, because input_str is not same as match case ");
}
//TEST - assemble_sys_call_structure returns structure filled with 0s
void  test_003_find_on_line_and_fill_result_array(void)
{
	module_index_t module_index = IETF_INTERFACES;//valid
	int obj_index    = IF_ENABLED;//valid
	int comb_index   = 1;//valid, 1 is from configuraion
	int parser_index = 0;//valid, 0 is from configuration
	char* input_str = NULL; //invalid, unused
    char result_array[MAX_NUM_OF_INTERFACES][MAX_STR_LEN];//irelevant for this test case
    sys_call_spec_t cmock_to_return;

    //sys_call_spec_t need to be set manually
    memset(&cmock_to_return, 0, sizeof(cmock_to_return));
    //sys_call_spec_t setting is done
    input_str = (char*) malloc(sizeof(char)*MAX_STR_LEN);
    strncpy(input_str, "test test", MAX_STR_LEN);//different than match case

    //mocks
    assemble_sys_call_structure_CMockIgnoreAndReturn(0, cmock_to_return);
    assemble_sys_call_structure_CMockIgnoreAndReturn(1, cmock_to_return);// second call

    base_status_t a = find_on_line_and_fill_result_array(module_index, obj_index, comb_index, parser_index, input_str, 0, NULL, 0, 0);
    TEST_ASSERT_EQUAL_INT_MESSAGE(BASE_PARSING_FAILED, a, "===>>> [input_str]= CIST_INFO_STR ===>>> SHOULD RETURN: BASE_PARSING_FAILED, because assemble_sys_call_structure returns structure filled with 0s ");
}

//TEST - match case is ""
void  test_004_find_on_line_and_fill_result_array(void)
{
	module_index_t module_index = IETF_INTERFACES;//valid
	int obj_index    = IF_ENABLED;//valid
	int comb_index   = 1;//valid, 1 is from configuraion
	int parser_index = 0;//valid, 0 is from configuration
	char* input_str = NULL; //invalid, unused
    char result_array[MAX_NUM_OF_INTERFACES][MAX_STR_LEN];//irelevant for this test case
    sys_call_spec_t cmock_to_return;

    //sys_call_spec_t need to be set manually
    memset(&cmock_to_return, 0, sizeof(sys_call_spec_t));//clear, somehow testing framework keeps old ????
    cmock_to_return.sys_call = NULL;       //irelevant for this test
    cmock_to_return.sys_call_command = ""; //irelevant for this test
    cmock_to_return.num_of_combinators = 5;//from configuration
    cmock_to_return.combinator = NULL;     //irelevant for this test
    int mstp_num_of_cb_tmp[5] = {0, 2, 1, 1, 1};    //taken from configuration
    memcpy(cmock_to_return.num_of_cb, mstp_num_of_cb_tmp, sizeof(int)*5);
    memcpy(cmock_to_return.parse_cb[comb_index][parser_index].match_case, "", MAX_STR_LEN);
    //sys_call_spec_t setting is done
    input_str = (char*) malloc(sizeof(char)*MAX_STR_LEN);
    strncpy(input_str, CIST_INFO_STR, MAX_STR_LEN);//different than match case

    //mocks
    assemble_sys_call_structure_CMockIgnoreAndReturn(0, cmock_to_return);
    assemble_sys_call_structure_CMockIgnoreAndReturn(1, cmock_to_return);// second call

    base_status_t a = find_on_line_and_fill_result_array(module_index, obj_index, comb_index, parser_index, input_str, 0, NULL, 0, 0);
    TEST_ASSERT_EQUAL_INT_MESSAGE(BASE_PARSING_FAILED, a, "===>>> [input_str]= CIST_INFO_STR, [match_case] = \"\" ===>>> SHOULD RETURN: BASE_PARSING_FAILED, because match case is \"\"");
}

//TEST - input_str is empty string
void  test_005_find_on_line_and_fill_result_array(void)
{
	module_index_t module_index = IETF_INTERFACES;//valid
	int obj_index    = IF_ENABLED;//valid
	int comb_index   = 1;//valid, 1 is from configuraion
	int parser_index = 0;//valid, 0 is from configuration
	char* input_str = NULL; //invalid, unused
    char result_array[MAX_NUM_OF_INTERFACES][MAX_STR_LEN];//irelevant for this test case
    sys_call_spec_t cmock_to_return;

    //sys_call_spec_t need to be set manually
    memset(&cmock_to_return, 0, sizeof(sys_call_spec_t));//clear, somehow testing framework keeps old ????
    cmock_to_return.sys_call = NULL;       //irelevant for this test
    cmock_to_return.sys_call_command = ""; //irelevant for this test
    cmock_to_return.num_of_combinators = 5;//from configuration
    cmock_to_return.combinator = NULL;     //irelevant for this test
    int mstp_num_of_cb_tmp[5] = {0, 2, 1, 1, 1};    //taken from configuration
    memcpy(cmock_to_return.num_of_cb, mstp_num_of_cb_tmp, sizeof(int)*5);
    memcpy(cmock_to_return.parse_cb[comb_index][parser_index].match_case, BASE_LOWER_UP, MAX_STR_LEN);
    //sys_call_spec_t setting is done
    input_str = (char*) malloc(sizeof(char)*MAX_STR_LEN);
    strncpy(input_str, "", MAX_STR_LEN);//different than match case

    //mocks
    assemble_sys_call_structure_CMockIgnoreAndReturn(0, cmock_to_return);
    assemble_sys_call_structure_CMockIgnoreAndReturn(1, cmock_to_return);// second call

    base_status_t a = find_on_line_and_fill_result_array(module_index, obj_index, comb_index, parser_index, input_str, 0, NULL, 0, 0);
    TEST_ASSERT_EQUAL_INT_MESSAGE(BASE_PARSING_FAILED, a, "===>>> [input_str]= \"\", [match_case] = CIST_INFO_STR ===>>> SHOULD RETURN: BASE_PARSING_FAILED, because input_str is empty string");
}
////*****END OF TESTING find_on_line_and_fill_result_arra*****////

////*****TESTS for find_and_get_next_word*****////
//TEST - input_str contains match case 
void  test_001_find_and_get_next_word(void)
{
    module_index_t module_index = MSTP;//valid
    int obj_index    = MSTP_PORT_NAMES;//valid
    int comb_index   = 1;//valid, 1 is from configuration
    int parser_index = 0;//valid, 0 is from configuration
    char* input_str = NULL; //invalid, unused
    char result_array[MAX_NUM_OF_INTERFACES][MAX_STR_LEN];//irelevant for this test case
    sys_call_spec_t cmock_to_return;
    base_status_t a;

    //sys_call_spec_t need to be set manually
    memset(&cmock_to_return, 0, sizeof(sys_call_spec_t));//clear, somehow testing framework keeps old ????
    cmock_to_return.sys_call = NULL;       //irelevant for this test
    cmock_to_return.sys_call_command = ""; //irelevant for this test
    cmock_to_return.num_of_combinators = 5;//from configuration
    cmock_to_return.combinator = NULL;     //irelevant for this test
    int mstp_num_of_cb_tmp[5] = {0, 1, 1, 1, 1};    //taken from configuration
    memcpy(cmock_to_return.num_of_cb, mstp_num_of_cb_tmp, sizeof(int)*5);
    memcpy(cmock_to_return.parse_cb[comb_index][parser_index].match_case, BRIDGE_ID_STR, MAX_STR_LEN);
    //sys_call_spec_t setting is done
    input_str = (char*) malloc(sizeof(char)*MAX_STR_LEN);
    strncpy(input_str, "bridge id 1", MAX_STR_LEN);//contains "bridge id" and something after it
    memset(&result_array, 0, MAX_NUM_OF_INTERFACES*MAX_STR_LEN);//clear result_array, somehow testing framework keeps old ????

    //mocks
    assemble_sys_call_structure_CMockIgnoreAndReturn(0, cmock_to_return);

    a = find_and_get_next_word(module_index, obj_index, comb_index, parser_index, input_str, 0, result_array, MAX_NUM_OF_INTERFACES, MAX_STR_LEN);
    //check result_array[0]        
    if(0 != strncmp("1", result_array[0], MAX_STR_LEN))
    {
        a = BASE_PARSING_FAILED;//direct fail if result is not good
    }
    TEST_ASSERT_EQUAL_INT_MESSAGE(BASE_PARSING_SUCCEEDED, a, "===>>> [input_str]= \"bridge id 1\" ===>>> SHOULD RETURN: BASE_PARSING_SUCCEEDED, because input_str contains match case ");    
}

//TEST - input_str is not same as match case 
void  test_002_find_and_get_next_word(void)
{
    module_index_t module_index = MSTP;//valid
    int obj_index    = MSTP_BRIDGE_PRIORITY;//valid
    int comb_index   = 1;//valid, 1 is from configuraion
    int parser_index = 0;//valid, 1 is from configuration
    char* input_str = NULL; //invalid, unused
    char result_array[MAX_NUM_OF_INTERFACES][MAX_STR_LEN];//irelevant for this test case
    sys_call_spec_t cmock_to_return;
    base_status_t a;

    //sys_call_spec_t need to be set manually
    memset(&cmock_to_return, 0, sizeof(sys_call_spec_t));//clear, somehow testing framework keeps old ????
    cmock_to_return.sys_call = NULL;       //irelevant for this test
    cmock_to_return.sys_call_command = ""; //irelevant for this test
    cmock_to_return.num_of_combinators = 5;//from configuration
    cmock_to_return.combinator = NULL;     //irelevant for this test
    int mstp_num_of_cb_tmp[5] = {0, 1, 1, 1, 1};    //taken from configuration
    memcpy(cmock_to_return.num_of_cb, mstp_num_of_cb_tmp, sizeof(int)*5);
    memcpy(cmock_to_return.parse_cb[comb_index][parser_index].match_case, BRIDGE_ID_STR, MAX_STR_LEN);
    //sys_call_spec_t setting is done
    input_str = (char*) malloc(sizeof(char)*MAX_STR_LEN);
    strncpy(input_str, "nothing", MAX_STR_LEN);//different than match case
    memset(&result_array, 0, MAX_NUM_OF_INTERFACES*MAX_STR_LEN);//clear result_array, somehow testing framework keeps old ????

    //mocks
    assemble_sys_call_structure_CMockIgnoreAndReturn(0, cmock_to_return);

    a = find_and_get_next_word(module_index, obj_index, comb_index, parser_index, input_str, 0, result_array, MAX_NUM_OF_INTERFACES, MAX_STR_LEN);
    //check result_array[0]
    if(0 != strncmp("", result_array[0], MAX_STR_LEN))
    {
        a = BASE_PARSING_FAILED;//direct fail if result is not good
    }
    TEST_ASSERT_EQUAL_INT_MESSAGE(BASE_PARSING_FAILED, a, "===>>> [input_str]= \"nothing\", [match_case] = BRIDGE_ID_STR ===>>> SHOULD RETURN: BASE_PARSING_FAILED, because input_str is not same as match case ");
}

//TEST - assemble_sys_call_structure returns structure filled with 0s 
void  test_003_find_and_get_next_word(void)
{
    module_index_t module_index = MSTP;//valid
    int obj_index    = MSTP_BRIDGE_PRIORITY;//valid
    int comb_index   = 1;//valid, 1 is from configuraion
    int parser_index = 0;//valid, 1 is from configuration
    char* input_str = NULL; //invalid, unused
    char result_array[MAX_NUM_OF_INTERFACES][MAX_STR_LEN];//irelevant for this test case
    sys_call_spec_t cmock_to_return;
    base_status_t a;
    
    //sys_call_spec_t need to be set manually
    memset(&cmock_to_return, 0, sizeof(cmock_to_return));
    //sys_call_spec_t setting is done
    input_str = (char*) malloc(sizeof(char)*MAX_STR_LEN);
    strncpy(input_str, "bridge id 1", MAX_STR_LEN);//different than match case
    memset(&result_array, 0, MAX_NUM_OF_INTERFACES*MAX_STR_LEN);//clear result_array, somehow testing framework keeps old ????

    //mocks
    assemble_sys_call_structure_CMockIgnoreAndReturn(0, cmock_to_return);

    a = find_and_get_next_word(module_index, obj_index, comb_index, parser_index, input_str, 0, result_array, MAX_NUM_OF_INTERFACES, MAX_STR_LEN);
    //check result_array[0]
    if(0 != strncmp("", result_array[0], MAX_STR_LEN))
    {
        a = BASE_PARSING_FAILED;//direct fail if result is not good
    }
    TEST_ASSERT_EQUAL_INT_MESSAGE(BASE_PARSING_FAILED, a, "===>>> [input_str]= \"bridge id 1\" ===>>> SHOULD RETURN: BASE_PARSING_FAILED, because assemble_sys_call_structure returns structure filled with 0s ");    
}

//TEST - match case is "" 
void  test_004_find_and_get_next_word(void)
{
    module_index_t module_index = MSTP;//valid
    int obj_index    = MSTP_BRIDGE_PRIORITY;//valid
    int comb_index   = 1;//valid, 1 is from configuraion
    int parser_index = 0;//valid, 1 is from configuration
    char* input_str = NULL; //invalid, unused
    char result_array[MAX_NUM_OF_INTERFACES][MAX_STR_LEN];//irelevant for this test case
    sys_call_spec_t cmock_to_return;
    base_status_t a;

    //sys_call_spec_t need to be set manually
    memset(&cmock_to_return, 0, sizeof(sys_call_spec_t));//clear, somehow testing framework keeps old ????
    cmock_to_return.sys_call = NULL;       //irelevant for this test
    cmock_to_return.sys_call_command = ""; //irelevant for this test
    cmock_to_return.num_of_combinators = 5;//from configuration
    cmock_to_return.combinator = NULL;     //irelevant for this test
    int mstp_num_of_cb_tmp[5] = {0, 1, 1, 1, 1};    //taken from configuration
    memcpy(cmock_to_return.num_of_cb, mstp_num_of_cb_tmp, sizeof(int)*5);
    memcpy(cmock_to_return.parse_cb[comb_index][parser_index].match_case, "", MAX_STR_LEN);
    //sys_call_spec_t setting is done
    input_str = (char*) malloc(sizeof(char)*MAX_STR_LEN);
    strncpy(input_str, "bridge id 1", MAX_STR_LEN);//different than match case
    memset(&result_array, 0, MAX_NUM_OF_INTERFACES*MAX_STR_LEN);//clear result_array, somehow testing framework keeps old ????

    //mocks
    assemble_sys_call_structure_CMockIgnoreAndReturn(0, cmock_to_return);

    a = find_and_get_next_word(module_index, obj_index, comb_index, parser_index, input_str, 0, result_array, MAX_NUM_OF_INTERFACES, MAX_STR_LEN);
    //check result_array[0]
    if(0 != strncmp("", result_array[0], MAX_STR_LEN))
    {
        a = BASE_PARSING_FAILED;//direct fail if result is not good
    }
    TEST_ASSERT_EQUAL_INT_MESSAGE(BASE_PARSING_FAILED, a, "===>>> [input_str]= \"bridge id 1\", [match_case] = \"\" ===>>> SHOULD RETURN: BASE_PARSING_FAILED, because match case is \"\"");
}

//TEST - input_str is empty string
void  test_005_find_and_get_next_word(void)
{
    module_index_t module_index = MSTP;//valid
    int obj_index    = MSTP_BRIDGE_PRIORITY;//valid
    int comb_index   = 1;//valid, 1 is from configuraion
    int parser_index = 0;//valid, 1 is from configuration
    char* input_str = NULL; //invalid, unused
    char result_array[MAX_NUM_OF_INTERFACES][MAX_STR_LEN];//irelevant for this test case
    sys_call_spec_t cmock_to_return;
    base_status_t a;

    //sys_call_spec_t need to be set manually
    memset(&cmock_to_return, 0, sizeof(sys_call_spec_t));//clear, somehow testing framework keeps old ????
    cmock_to_return.sys_call = NULL;       //irelevant for this test
    cmock_to_return.sys_call_command = ""; //irelevant for this test
    cmock_to_return.num_of_combinators = 5;//from configuration
    cmock_to_return.combinator = NULL;     //irelevant for this test
    int mstp_num_of_cb_tmp[5] = {0, 1, 1, 1, 1};    //taken from configuration
    memcpy(cmock_to_return.num_of_cb, mstp_num_of_cb_tmp, sizeof(int)*5);
    memcpy(cmock_to_return.parse_cb[comb_index][parser_index].match_case, BRIDGE_ID_STR, MAX_STR_LEN);
    //sys_call_spec_t setting is done
    input_str = (char*) malloc(sizeof(char)*MAX_STR_LEN);
    strncpy(input_str, "", MAX_STR_LEN);//different than match case
    memset(&result_array, 0, MAX_NUM_OF_INTERFACES*MAX_STR_LEN);//clear result_array, somehow testing framework keeps old ????

    //mocks
    assemble_sys_call_structure_CMockIgnoreAndReturn(0, cmock_to_return);

    a = find_and_get_next_word(module_index, obj_index, comb_index, parser_index, input_str, 0, result_array, MAX_NUM_OF_INTERFACES, MAX_STR_LEN);
    //check result_array[0]
    if(0 != strncmp("", result_array[0], MAX_STR_LEN))
    {
        a = BASE_PARSING_FAILED;//direct fail if result is not good
    }
    TEST_ASSERT_EQUAL_INT_MESSAGE(BASE_PARSING_FAILED, a, "===>>> [input_str]= \"\", [match_case] = BRIDGE_ID_STR ===>>> SHOULD RETURN: BASE_PARSING_FAILED, because input_str is empty string");
}
////*****END OF TESTING find_and_get_next_word*****////


////*****TESTS for find_and_get_previous_word*****////
//TEST - input_str contains match case 
void  test_001_find_and_get_previous_word(void)
{
    module_index_t module_index = Q_BRIDGE;//valid
    int obj_index    = Q_BRIDGE_PORT_PVID;//valid
    int comb_index   = 1;//valid, 1 is from configuration
    int parser_index = 0;//valid, 0 is from configuration
    char* input_str = NULL; //invalid, unused
    char result_array[MAX_NUM_OF_INTERFACES][MAX_STR_LEN];//irelevant for this test case
    sys_call_spec_t cmock_to_return;
    base_status_t a;

    //sys_call_spec_t need to be set manually
    memset(&cmock_to_return, 0, sizeof(sys_call_spec_t));//clear, somehow testing framework keeps old ????
    cmock_to_return.sys_call = NULL;       //irelevant for this test
    cmock_to_return.sys_call_command = ""; //irelevant for this test
    cmock_to_return.num_of_combinators = 4;//from configuration
    cmock_to_return.combinator = NULL;     //irelevant for this test
    int mstp_num_of_cb_tmp[4] = {0, 1, 1, 1};    //taken from configuration
    memcpy(cmock_to_return.num_of_cb, mstp_num_of_cb_tmp, sizeof(int)*4);
    memcpy(cmock_to_return.parse_cb[comb_index][parser_index].match_case, "PVID", MAX_STR_LEN);
    //sys_call_spec_t setting is done
    input_str = (char*) malloc(sizeof(char)*MAX_STR_LEN);
    strncpy(input_str, "CE01     1 PVID Egress Untagged", MAX_STR_LEN);//contains "PVID" and something after it
    memset(&result_array, 0, MAX_NUM_OF_INTERFACES*MAX_STR_LEN);//clear result_array, somehow testing framework keeps old ????

    //mocks
    assemble_sys_call_structure_CMockIgnoreAndReturn(0, cmock_to_return);

    a = find_and_get_previous_word(module_index, obj_index, comb_index, parser_index, input_str, 0, result_array, MAX_NUM_OF_INTERFACES, MAX_STR_LEN);
    //check result_array[0]        
    if(0 != strncmp("1", result_array[0], MAX_STR_LEN))
    {
        a = BASE_PARSING_FAILED;//direct fail if result is not good
    }
    TEST_ASSERT_EQUAL_INT_MESSAGE(BASE_PARSING_SUCCEEDED, a, "===>>> [input_str]= \"CE01     1 PVID Egress Untagged\" ===>>> SHOULD RETURN: BASE_PARSING_SUCCEEDED, because input_str contains match case ");    
}

//TEST - input_str is not same as match case 
void  test_002_find_and_get_previous_word(void)
{
    module_index_t module_index = Q_BRIDGE;//valid
    int obj_index    = Q_BRIDGE_PORT_PVID;//valid
    int comb_index   = 1;//valid, 1 is from configuraion
    int parser_index = 0;//valid, 1 is from configuration
    char* input_str = NULL; //invalid, unused
    char result_array[MAX_NUM_OF_INTERFACES][MAX_STR_LEN];//irelevant for this test case
    sys_call_spec_t cmock_to_return;
    base_status_t a;

    //sys_call_spec_t need to be set manually
    memset(&cmock_to_return, 0, sizeof(sys_call_spec_t));//clear, somehow testing framework keeps old ????
    cmock_to_return.sys_call = NULL;       //irelevant for this test
    cmock_to_return.sys_call_command = ""; //irelevant for this test
    cmock_to_return.num_of_combinators = 4;//from configuration
    cmock_to_return.combinator = NULL;     //irelevant for this test
    int mstp_num_of_cb_tmp[4] = {0, 1, 1, 1};    //taken from configuration
    memcpy(cmock_to_return.num_of_cb, mstp_num_of_cb_tmp, sizeof(int)*4);
    memcpy(cmock_to_return.parse_cb[comb_index][parser_index].match_case, "PVID", MAX_STR_LEN);
    //sys_call_spec_t setting is done
    input_str = (char*) malloc(sizeof(char)*MAX_STR_LEN);
    strncpy(input_str, "nothing", MAX_STR_LEN);//different than match case
    memset(&result_array, 0, MAX_NUM_OF_INTERFACES*MAX_STR_LEN);//clear result_array, somehow testing framework keeps old ????

    //mocks
    assemble_sys_call_structure_CMockIgnoreAndReturn(0, cmock_to_return);

    a = find_and_get_previous_word(module_index, obj_index, comb_index, parser_index, input_str, 0, result_array, MAX_NUM_OF_INTERFACES, MAX_STR_LEN);
    //check result_array[0]
    if(0 != strncmp("", result_array[0], MAX_STR_LEN))
    {
        a = BASE_PARSING_FAILED;//direct fail if result is not good
    }
    TEST_ASSERT_EQUAL_INT_MESSAGE(BASE_PARSING_FAILED, a, "===>>> [input_str]= \"nothing\", [match_case] = PVID ===>>> SHOULD RETURN: BASE_PARSING_FAILED, because input_str is not same as match case ");
}

//TEST - input_str is same as match case, bud value before it is missing
void  test_003_find_and_get_previous_word(void)
{
    module_index_t module_index = Q_BRIDGE;//valid
    int obj_index    = Q_BRIDGE_PORT_PVID;//valid
    int comb_index   = 1;//valid, 1 is from configuraion
    int parser_index = 0;//valid, 1 is from configuration
    char* input_str = NULL; //invalid, unused
    char result_array[MAX_NUM_OF_INTERFACES][MAX_STR_LEN];//irelevant for this test case
    sys_call_spec_t cmock_to_return;
    base_status_t a;

    //sys_call_spec_t need to be set manually
    memset(&cmock_to_return, 0, sizeof(sys_call_spec_t));//clear, somehow testing framework keeps old ????
    cmock_to_return.sys_call = NULL;       //irelevant for this test
    cmock_to_return.sys_call_command = ""; //irelevant for this test
    cmock_to_return.num_of_combinators = 4;//from configuration
    cmock_to_return.combinator = NULL;     //irelevant for this test
    int mstp_num_of_cb_tmp[4] = {0, 1, 1, 1};    //taken from configuration
    memcpy(cmock_to_return.num_of_cb, mstp_num_of_cb_tmp, sizeof(int)*4);
    memcpy(cmock_to_return.parse_cb[comb_index][parser_index].match_case, "PVID", MAX_STR_LEN);
    //sys_call_spec_t setting is done
    input_str = (char*) malloc(sizeof(char)*MAX_STR_LEN);
    strncpy(input_str, "PVID", MAX_STR_LEN);//different than match case
    memset(&result_array, 0, MAX_NUM_OF_INTERFACES*MAX_STR_LEN);//clear result_array, somehow testing framework keeps old ????

    //mocks
    assemble_sys_call_structure_CMockIgnoreAndReturn(0, cmock_to_return);

    a = find_and_get_previous_word(module_index, obj_index, comb_index, parser_index, input_str, 0, result_array, MAX_NUM_OF_INTERFACES, MAX_STR_LEN);
    //check result_array[0]
    if(0 != strncmp("", result_array[0], MAX_STR_LEN))
    {
        a = BASE_PARSING_FAILED;//direct fail if result is not good
    }
    TEST_ASSERT_EQUAL_INT_MESSAGE(BASE_PARSING_FAILED, a, "===>>> [input_str]= \"PVID\", [match_case] = PVID ===>>> SHOULD RETURN: BASE_PARSING_FAILED, input_str is same as match case, bud value before it is missing");
}

//TEST - assemble_sys_call_structure returns structure filled with 0s 
void  test_004_find_and_get_previous_word(void)
{
    module_index_t module_index = Q_BRIDGE;//valid
    int obj_index    = Q_BRIDGE_PORT_PVID;//valid
    int comb_index   = 1;//valid, 1 is from configuraion
    int parser_index = 0;//valid, 1 is from configuration
    char* input_str = NULL; //invalid, unused
    char result_array[MAX_NUM_OF_INTERFACES][MAX_STR_LEN];//irelevant for this test case
    sys_call_spec_t cmock_to_return;
    base_status_t a;
    
    //sys_call_spec_t need to be set manually
    memset(&cmock_to_return, 0, sizeof(cmock_to_return));
    //sys_call_spec_t setting is done
    input_str = (char*) malloc(sizeof(char)*MAX_STR_LEN);
    strncpy(input_str, "CE01     1 PVID Egress Untagged", MAX_STR_LEN);//different than match case
    memset(&result_array, 0, MAX_NUM_OF_INTERFACES*MAX_STR_LEN);//clear result_array, somehow testing framework keeps old ????

    //mocks
    assemble_sys_call_structure_CMockIgnoreAndReturn(0, cmock_to_return);

    a = find_and_get_previous_word(module_index, obj_index, comb_index, parser_index, input_str, 0, result_array, MAX_NUM_OF_INTERFACES, MAX_STR_LEN);
    //check result_array[0]
    if(0 != strncmp("", result_array[0], MAX_STR_LEN))
    {
        a = BASE_PARSING_FAILED;//direct fail if result is not good
    }
    TEST_ASSERT_EQUAL_INT_MESSAGE(BASE_PARSING_FAILED, a, "===>>> [input_str]= \"CE01     1 PVID Egress Untagged\" ===>>> SHOULD RETURN: BASE_PARSING_FAILED, because assemble_sys_call_structure returns structure filled with 0s ");    
}

//TEST - match case is "" 
void  test_005_find_and_get_previous_word(void)
{
    module_index_t module_index = Q_BRIDGE;//valid
    int obj_index    = Q_BRIDGE_PORT_PVID;//valid
    int comb_index   = 1;//valid, 1 is from configuraion
    int parser_index = 0;//valid, 1 is from configuration
    char* input_str = NULL; //invalid, unused
    char result_array[MAX_NUM_OF_INTERFACES][MAX_STR_LEN];//irelevant for this test case
    sys_call_spec_t cmock_to_return;
    base_status_t a;

    //sys_call_spec_t need to be set manually
    memset(&cmock_to_return, 0, sizeof(sys_call_spec_t));//clear, somehow testing framework keeps old ????
    cmock_to_return.sys_call = NULL;       //irelevant for this test
    cmock_to_return.sys_call_command = ""; //irelevant for this test
    cmock_to_return.num_of_combinators = 4;//from configuration
    cmock_to_return.combinator = NULL;     //irelevant for this test
    int mstp_num_of_cb_tmp[4] = {0, 1, 1, 1};    //taken from configuration
    memcpy(cmock_to_return.num_of_cb, mstp_num_of_cb_tmp, sizeof(int)*4);
    memcpy(cmock_to_return.parse_cb[comb_index][parser_index].match_case, "", MAX_STR_LEN);
    //sys_call_spec_t setting is done
    input_str = (char*) malloc(sizeof(char)*MAX_STR_LEN);
    strncpy(input_str, "CE01     1 PVID Egress Untagged", MAX_STR_LEN);//different than match case
    memset(&result_array, 0, MAX_NUM_OF_INTERFACES*MAX_STR_LEN);//clear result_array, somehow testing framework keeps old ????

    //mocks
    assemble_sys_call_structure_CMockIgnoreAndReturn(0, cmock_to_return);

    a = find_and_get_previous_word(module_index, obj_index, comb_index, parser_index, input_str, 0, result_array, MAX_NUM_OF_INTERFACES, MAX_STR_LEN);
    //check result_array[0]
    if(0 != strncmp("", result_array[0], MAX_STR_LEN))
    {
        a = BASE_PARSING_FAILED;//direct fail if result is not good
    }
    TEST_ASSERT_EQUAL_INT_MESSAGE(BASE_PARSING_FAILED, a, "===>>> [input_str]= \"CE01     1 PVID Egress Untagged\", [match_case] = \"\" ===>>> SHOULD RETURN: BASE_PARSING_FAILED, because match case is \"\"");
}

//TEST - input_str is empty string
void  test_006_find_and_get_previous_word(void)
{
    module_index_t module_index = Q_BRIDGE;//valid
    int obj_index    = Q_BRIDGE_PORT_PVID;//valid
    int comb_index   = 1;//valid, 1 is from configuraion
    int parser_index = 0;//valid, 1 is from configuration
    char* input_str = NULL; //invalid, unused
    char result_array[MAX_NUM_OF_INTERFACES][MAX_STR_LEN];//irelevant for this test case
    sys_call_spec_t cmock_to_return;
    base_status_t a;

    //sys_call_spec_t need to be set manually
    memset(&cmock_to_return, 0, sizeof(sys_call_spec_t));//clear, somehow testing framework keeps old ????
    cmock_to_return.sys_call = NULL;       //irelevant for this test
    cmock_to_return.sys_call_command = ""; //irelevant for this test
    cmock_to_return.num_of_combinators = 4;//from configuration
    cmock_to_return.combinator = NULL;     //irelevant for this test
    int mstp_num_of_cb_tmp[4] = {0, 1, 1, 1};    //taken from configuration
    memcpy(cmock_to_return.num_of_cb, mstp_num_of_cb_tmp, sizeof(int)*4);
    memcpy(cmock_to_return.parse_cb[comb_index][parser_index].match_case, "PVID", MAX_STR_LEN);
    //sys_call_spec_t setting is done
    input_str = (char*) malloc(sizeof(char)*MAX_STR_LEN);
    strncpy(input_str, "", MAX_STR_LEN);//different than match case
    memset(&result_array, 0, MAX_NUM_OF_INTERFACES*MAX_STR_LEN);//clear result_array, somehow testing framework keeps old ????

    //mocks
    assemble_sys_call_structure_CMockIgnoreAndReturn(0, cmock_to_return);

    a = find_and_get_previous_word(module_index, obj_index, comb_index, parser_index, input_str, 0, result_array, MAX_NUM_OF_INTERFACES, MAX_STR_LEN);
    //check result_array[0]
    if(0 != strncmp("", result_array[0], MAX_STR_LEN))
    {
        a = BASE_PARSING_FAILED;//direct fail if result is not good
    }
    TEST_ASSERT_EQUAL_INT_MESSAGE(BASE_PARSING_FAILED, a, "===>>> [input_str]= \"\", [match_case] = \"PVID\" ===>>> SHOULD RETURN: BASE_PARSING_FAILED, because input_str is empty string");
}
////*****END OF TESTING find_and_get_previous_word*****////

////*****TESTS for get_first_word*****////
//TEST - input_str contains match case 
void  test_001_get_first_word(void)
{
    module_index_t module_index = MSTP;//irelevant for this test case
    int obj_index    = MSTP_CIST_PORT_DSGN_ROOT;//irelevant for this test case
    int comb_index   = 1;//irelevant for this test case
    int parser_index = 0;//irelevant for this test case
    char* input_str = NULL;
    char result_array[MAX_NUM_OF_INTERFACES][MAX_STR_LEN];
    base_status_t a;
    //set input_str
    input_str = (char*) malloc(sizeof(char)*MAX_STR_LEN);
    strncpy(input_str, "word", MAX_STR_LEN);//contains ":" and something after it
    memset(&result_array, 0, MAX_NUM_OF_INTERFACES*MAX_STR_LEN);//clear result_array, somehow testing framework keeps old ????

    a = get_first_word(module_index, obj_index, comb_index, parser_index, input_str, 0, result_array, MAX_NUM_OF_INTERFACES, MAX_STR_LEN);
    
    //check result_array[0]
    if(0 != strncmp("word", result_array[0], MAX_STR_LEN))
    {
        a = BASE_PARSING_FAILED;//direct fail if result is not good
    }
    TEST_ASSERT_EQUAL_INT_MESSAGE(BASE_PARSING_SUCCEEDED, a, "===>>> [input_str]= \"word\" ===>>> SHOULD RETURN: BASE_PARSING_SUCCEEDED, because input_str contains match case ");    
}

//TEST - input_str contains two words 
void  test_002_get_first_word(void)
{
    module_index_t module_index = MSTP;//irelevant for this test case
    int obj_index    = MSTP_CIST_PORT_DSGN_ROOT;//irelevant for this test case
    int comb_index   = 1;//irelevant for this test case
    int parser_index = 0;//irelevant for this test case
    char* input_str = NULL;
    char result_array[MAX_NUM_OF_INTERFACES][MAX_STR_LEN];
    base_status_t a;
    //set input_str
    input_str = (char*) malloc(sizeof(char)*MAX_STR_LEN);
    strncpy(input_str, "two words", MAX_STR_LEN);//contains ":" and something after it
    memset(&result_array, 0, MAX_NUM_OF_INTERFACES*MAX_STR_LEN);//clear result_array, somehow testing framework keeps old ????
    
    a = get_first_word(module_index, obj_index, comb_index, parser_index, input_str, 0, result_array, MAX_NUM_OF_INTERFACES, MAX_STR_LEN);
    
    //check result_array[0]
    if(0 != strncmp("two", result_array[0], MAX_STR_LEN))
    {
        a = BASE_PARSING_FAILED;//direct fail if result is not good
    }
    TEST_ASSERT_EQUAL_INT_MESSAGE(BASE_PARSING_SUCCEEDED, a, "===>>> [input_str]= \"two words\" ===>>> SHOULD RETURN: BASE_PARSING_SUCCEEDED, because input_str contains two words");    
}

//TEST - input_str contains word with punction 
void  test_003_get_first_word(void)
{
    module_index_t module_index = MSTP;//irelevant for this test case
    int obj_index    = MSTP_CIST_PORT_DSGN_ROOT;//irelevant for this test case
    int comb_index   = 1;//irelevant for this test case
    int parser_index = 0;//irelevant for this test case
    char* input_str = NULL;
    char result_array[MAX_NUM_OF_INTERFACES][MAX_STR_LEN];
    base_status_t a;
    //set input_str
    input_str = (char*) malloc(sizeof(char)*MAX_STR_LEN);
    strncpy(input_str, "two.words", MAX_STR_LEN);//contains ":" and something after it
    memset(&result_array, 0, MAX_NUM_OF_INTERFACES*MAX_STR_LEN);//clear result_array, somehow testing framework keeps old ????

    a = get_first_word(module_index, obj_index, comb_index, parser_index, input_str, 0, result_array, MAX_NUM_OF_INTERFACES, MAX_STR_LEN);
    
    //check result_array[0]
    if(0 != strncmp("two.words", result_array[0], MAX_STR_LEN))
    {
        a = BASE_PARSING_FAILED;//direct fail if result is not good
    }
    TEST_ASSERT_EQUAL_INT_MESSAGE(BASE_PARSING_SUCCEEDED, a, "===>>> [input_str]= \"two.words\" ===>>> SHOULD RETURN: BASE_PARSING_SUCCEEDED, because input_str contains word with punction");    
}

//TEST - input_str is empty string
void  test_004_get_first_word(void)
{
    module_index_t module_index = MSTP;//irelevant for this test case
    int obj_index    = MSTP_CIST_PORT_DSGN_ROOT;//irelevant for this test case
    int comb_index   = 1;//irelevant for this test case
    int parser_index = 0;//irelevant for this test case
    char* input_str = NULL;
    char result_array[MAX_NUM_OF_INTERFACES][MAX_STR_LEN];
    base_status_t a;
    //set input_str
    input_str = (char*) malloc(sizeof(char)*MAX_STR_LEN);
    strncpy(input_str, "", MAX_STR_LEN);//contains ":" and something after it
    memset(&result_array, 0, MAX_NUM_OF_INTERFACES*MAX_STR_LEN);//clear result_array, somehow testing framework keeps old ????

    a = get_first_word(module_index, obj_index, comb_index, parser_index, input_str, 0, result_array, MAX_NUM_OF_INTERFACES, MAX_STR_LEN);
    
    //check result_array[0]
    if(0 != strncmp("", result_array[0], MAX_STR_LEN))
    {
        a = BASE_PARSING_FAILED;//direct fail if result is not good
    }
    TEST_ASSERT_EQUAL_INT_MESSAGE(BASE_PARSING_FAILED, a, "===>>> [input_str]= \"word\" ===>>> SHOULD RETURN: BASE_PARSING_FAILED, because input_str is empty");    
}
////*****END OF TESTING get_first_word*****////


////*****TESTS for parse_dec_values_separated_with_spaces*****////
//TEST - input_str contains match case 
void  test_001_parse_dec_values_separated_with_spaces(void)
{
    module_index_t module_index = MSTP;//irelevant for this test case
    int obj_index    = MSTP_CIST_PORT_DSGN_ROOT;//irelevant for this test case
    int comb_index   = 1;//irelevant for this test case
    int parser_index = 0;//irelevant for this test case
    char* input_str = NULL;
    char result_array[MAX_NUM_OF_INTERFACES][MAX_STR_LEN];
    base_status_t a;
    //set input_str
    input_str = (char*) malloc(sizeof(char)*MAX_STR_LEN);
    strncpy(input_str, "10", MAX_STR_LEN);//
    memset(&result_array, 0, MAX_NUM_OF_INTERFACES*MAX_STR_LEN);//clear result_array, somehow testing framework keeps old ????

    a = parse_dec_values_separated_with_spaces(module_index, obj_index, comb_index, parser_index, input_str, 0, result_array, MAX_NUM_OF_INTERFACES, MAX_STR_LEN);

    //check result_array[0]
    if(0 != strncmp("10", result_array[0], MAX_STR_LEN))
    {
        a = BASE_PARSING_FAILED;//direct fail if result is not good
    }
    TEST_ASSERT_EQUAL_INT_MESSAGE(BASE_PARSING_SUCCEEDED, a, "===>>> [input_str]= \"10\" ===>>> SHOULD RETURN: BASE_PARSING_SUCCEEDED, because input_str contains match case ");    
}

//TEST - input_str contains two words 
void  test_002_parse_dec_values_separated_with_spaces(void)
{
    module_index_t module_index = MSTP;//irelevant for this test case
    int obj_index    = MSTP_CIST_PORT_DSGN_ROOT;//irelevant for this test case
    int comb_index   = 1;//irelevant for this test case
    int parser_index = 0;//irelevant for this test case
    char* input_str = NULL;
    char result_array[MAX_NUM_OF_INTERFACES][MAX_STR_LEN];
    base_status_t a;
    //set input_str
    input_str = (char*) malloc(sizeof(char)*MAX_STR_LEN);
    strncpy(input_str, " 10 11 ", MAX_STR_LEN);//contains ":" and something after it
    memset(&result_array, 0, MAX_NUM_OF_INTERFACES*MAX_STR_LEN);//clear result_array, somehow testing framework keeps old ????
    
    a = parse_dec_values_separated_with_spaces(module_index, obj_index, comb_index, parser_index, input_str, 0, result_array, MAX_NUM_OF_INTERFACES, MAX_STR_LEN);
        
    //check result_array[0]
    if((0 != strncmp("10", result_array[0], MAX_STR_LEN)) || (0 != strncmp("11", result_array[1], MAX_STR_LEN)))
    {
        a = BASE_PARSING_FAILED;//direct fail if result is not good
    }
    TEST_ASSERT_EQUAL_INT_MESSAGE(BASE_PARSING_SUCCEEDED, a, "===>>> [input_str]= \"10 11\" ===>>> SHOULD RETURN: BASE_PARSING_SUCCEEDED, because input_str contains two words");    
}

//TEST - input_str contains value greater than kong int max 
void  test_003_parse_dec_values_separated_with_spaces(void)
{
    module_index_t module_index = MSTP;//irelevant for this test case
    int obj_index    = MSTP_CIST_PORT_DSGN_ROOT;//irelevant for this test case
    int comb_index   = 1;//irelevant for this test case
    int parser_index = 0;//irelevant for this test case
    char* input_str = NULL;
    char result_array[MAX_NUM_OF_INTERFACES][MAX_STR_LEN];
    base_status_t a;
    //set input_str
    input_str = (char*) malloc(sizeof(char)*MAX_STR_LEN);
    strncpy(input_str, "4294967296", MAX_STR_LEN);//(UINT_MAX) 4294967295 + 1
    memset(&result_array, 0, MAX_NUM_OF_INTERFACES*MAX_STR_LEN);//clear result_array, somehow testing framework keeps old ????
    
    a = parse_dec_values_separated_with_spaces(module_index, obj_index, comb_index, parser_index, input_str, 0, result_array, MAX_NUM_OF_INTERFACES, MAX_STR_LEN);
        
    //check result_array[0]
    // this part of the code is not reached
    TEST_ASSERT_EQUAL_INT_MESSAGE(BASE_PARSING_FAILED, a, "===>>> [input_str]= \"4294967296\" ===>>> SHOULD RETURN: BASE_PARSING_FAILED, because input_str too big number");    
}

//TEST - input_str contains word with punction 
void  test_004_parse_dec_values_separated_with_spaces(void)
{
    module_index_t module_index = MSTP;//irelevant for this test case
    int obj_index    = MSTP_CIST_PORT_DSGN_ROOT;//irelevant for this test case
    int comb_index   = 1;//irelevant for this test case
    int parser_index = 0;//irelevant for this test case
    char* input_str = NULL;
    char result_array[MAX_NUM_OF_INTERFACES][MAX_STR_LEN];
    base_status_t a;
    //set input_str
    input_str = (char*) malloc(sizeof(char)*MAX_STR_LEN);
    strncpy(input_str, "10.11 abc", MAX_STR_LEN);//contains ":" and something after it
    memset(&result_array, 0, MAX_NUM_OF_INTERFACES*MAX_STR_LEN);//clear result_array, somehow testing framework keeps old ????

    a = parse_dec_values_separated_with_spaces(module_index, obj_index, comb_index, parser_index, input_str, 0, result_array, MAX_NUM_OF_INTERFACES, MAX_STR_LEN);
    
    //check result_array[0]
    if(0 != strncmp("", result_array[0], MAX_STR_LEN))
    {
        a = BASE_PARSING_FAILED;//direct fail if result is not good
    }
    TEST_ASSERT_EQUAL_INT_MESSAGE(BASE_PARSING_FAILED, a, "===>>> [input_str]= \"10.11 abc\" ===>>> SHOULD RETURN: BASE_PARSING_FAILED, because input_str contains word with punction");    
}

//TEST - input_str is empty string
void  test_005_parse_dec_values_separated_with_spaces(void)
{
    module_index_t module_index = MSTP;//irelevant for this test case
    int obj_index    = MSTP_CIST_PORT_DSGN_ROOT;//irelevant for this test case
    int comb_index   = 1;//irelevant for this test case
    int parser_index = 0;//irelevant for this test case
    char* input_str = NULL;
    char result_array[MAX_NUM_OF_INTERFACES][MAX_STR_LEN];
    base_status_t a;
    //set input_str
    input_str = (char*) malloc(sizeof(char)*MAX_STR_LEN);
    strncpy(input_str, "", MAX_STR_LEN);//contains ":" and something after it
    memset(&result_array, 0, MAX_NUM_OF_INTERFACES*MAX_STR_LEN);//clear result_array, somehow testing framework keeps old ????

    a = parse_dec_values_separated_with_spaces(module_index, obj_index, comb_index, parser_index, input_str, 0, result_array, MAX_NUM_OF_INTERFACES, MAX_STR_LEN);

    //check result_array[0]
    if(0 != strncmp("", result_array[0], MAX_STR_LEN))
    {
        a = BASE_PARSING_FAILED;//direct fail if result is not good
    }
    TEST_ASSERT_EQUAL_INT_MESSAGE(BASE_PARSING_FAILED, a, "===>>> [input_str]= \"\" ===>>> SHOULD RETURN: BASE_PARSING_FAILED, because input_str is empty");    
}
////*****END OF TESTING parse_dec_values_separated_with_spaces*****////


// BEGINING
////*****TESTS for array_take_value_after*****////
//TEST - there is no match case in result_array[0]
void  test_001_array_take_value_after(void)
{
    module_index_t module_index = MSTP;//valid
    int obj_index    = MSTP_PORT_NUM;//valid
    int comb_index   = 2;//from configuraion
    int parser_index = 0;//from configuration
    char* input_str = NULL; //irelevant for this test case (unused parameter)
    char result_array[MAX_NUM_OF_INTERFACES][MAX_STR_LEN];
    sys_call_spec_t cmock_to_return;
    base_status_t a;

    //sys_call_spec_t need to be set manually
    memset(&cmock_to_return, 0, sizeof(sys_call_spec_t));//clear, somehow testing framework keeps old ????
    cmock_to_return.sys_call = NULL;       //irelevant for this test
    cmock_to_return.sys_call_command = ""; //irelevant for this test
    cmock_to_return.num_of_combinators = 4;//from configuration
    cmock_to_return.combinator = NULL;     //irelevant for this test
    int mstp_num_of_cb_tmp[4] = {0, 1, 1, 1};    //taken from configuration
    memcpy(cmock_to_return.num_of_cb, mstp_num_of_cb_tmp, sizeof(int)*4);
    memcpy(cmock_to_return.parse_cb[comb_index][parser_index].match_case, POINT_STR, MAX_STR_LEN);
    //sys_call_spec_t setting is done
    memset(&result_array, 0, MAX_NUM_OF_INTERFACES*MAX_STR_LEN);//clear result_array, somehow testing framework keeps old ????
    strncpy(result_array[0], "nothing", MAX_STR_LEN);//different than match case
    
    //mocks
    assemble_sys_call_structure_CMockIgnoreAndReturn(0, cmock_to_return);

    a = array_take_value_after(module_index, obj_index, comb_index, parser_index, input_str, 0, result_array, MAX_NUM_OF_INTERFACES, MAX_STR_LEN);
    //check result_array[0]
    if(0 != strncmp("", result_array[0], MAX_STR_LEN))
    {
        a = BASE_PARSING_SUCCEEDED;//direct fail if result is not good
    }//in normal execution we except FAILED, so in previous block logic is changed that tet can fail
    TEST_ASSERT_EQUAL_INT_MESSAGE(BASE_PARSING_FAILED, a, "===>>> [there is no match case in result_array[0]] ===>>> SHOULD RETURN: BASE_PARSING_FAILED, because there is no match case in result_array[0]");    
}

//TEST - assemble_sys_call_structure returns structure filled with 0s 
void  test_002_array_take_value_after(void)
{
    module_index_t module_index = MSTP;//valid
    int obj_index    = MSTP_PORT_NAMES;//valid
    int comb_index   = 2;//valid, 2 is from configuraion
    int parser_index = 0;//valid, 0 is from configuration
    char* input_str = NULL; //invalid, unused
    char result_array[MAX_NUM_OF_INTERFACES][MAX_STR_LEN];//irelevant for this test case
    sys_call_spec_t cmock_to_return;
    base_status_t a;
    
    //sys_call_spec_t need to be set manually
    memset(&cmock_to_return, 0, sizeof(cmock_to_return));
    //sys_call_spec_t setting is done
    memset(&result_array, 0, MAX_NUM_OF_INTERFACES*MAX_STR_LEN);//clear result_array, somehow testing framework keeps old ????
    strncpy(result_array[0], "10.11", MAX_STR_LEN);//different than match case
    
    //mocks
    assemble_sys_call_structure_CMockIgnoreAndReturn(0, cmock_to_return);

    a = array_take_value_after(module_index, obj_index, comb_index, parser_index, input_str, 0, result_array, MAX_NUM_OF_INTERFACES, MAX_STR_LEN);
    //check result_array[0]
    //NO NEED, part with this manipulation is not REACHED
    TEST_ASSERT_EQUAL_INT_MESSAGE(BASE_PARSING_FAILED, a, "===>>> [assemble_sys_call_structure returns structure filled with 0s] ===>>> SHOULD RETURN: BASE_PARSING_FAILED, because assemble_sys_call_structure returns structure filled with 0s ");    
}

//TEST - match case is "" 
void  test_003_array_take_value_after(void)
{
    module_index_t module_index = MSTP;//valid
    int obj_index    = MSTP_PORT_NUM;//valid
    int comb_index   = 2;//from configuraion
    int parser_index = 0;//from configuration
    char* input_str = NULL; //irelevant for this test case (unused parameter)
    char result_array[MAX_NUM_OF_INTERFACES][MAX_STR_LEN];
    sys_call_spec_t cmock_to_return;
    base_status_t a;

    //sys_call_spec_t need to be set manually
    memset(&cmock_to_return, 0, sizeof(sys_call_spec_t));//clear, somehow testing framework keeps old ????
    cmock_to_return.sys_call = NULL;       //irelevant for this test
    cmock_to_return.sys_call_command = ""; //irelevant for this test
    cmock_to_return.num_of_combinators = 4;//from configuration
    cmock_to_return.combinator = NULL;     //irelevant for this test
    int mstp_num_of_cb_tmp[4] = {0, 1, 1, 1};    //taken from configuration
    memcpy(cmock_to_return.num_of_cb, mstp_num_of_cb_tmp, sizeof(int)*4);
    memcpy(cmock_to_return.parse_cb[comb_index][parser_index].match_case, "", MAX_STR_LEN);
    //sys_call_spec_t setting is done
    memset(&result_array, 0, MAX_NUM_OF_INTERFACES*MAX_STR_LEN);//clear result_array, somehow testing framework keeps old ????
    strncpy(result_array[0], "10.11", MAX_STR_LEN);//different than match case
    
    //mocks
    assemble_sys_call_structure_CMockIgnoreAndReturn(0, cmock_to_return);


    a = array_take_value_after(module_index, obj_index, comb_index, parser_index, input_str, 0, result_array, MAX_NUM_OF_INTERFACES, MAX_STR_LEN);
    //check result_array[0]
    //NO NEED, part with this manipulation is not REACHED
    TEST_ASSERT_EQUAL_INT_MESSAGE(BASE_PARSING_FAILED, a, "===>>> [match case is \"\"] ===>>> SHOULD RETURN: BASE_PARSING_FAILED, because match case is \"\"");    
}

//TEST - result_array[0] is empty string
void  test_004_array_take_value_after(void)
{
    module_index_t module_index = MSTP;//valid
    int obj_index    = MSTP_PORT_NUM;//valid
    int comb_index   = 2;//from configuraion
    int parser_index = 0;//from configuration
    char* input_str = NULL; //irelevant for this test case (unused parameter)
    char result_array[MAX_NUM_OF_INTERFACES][MAX_STR_LEN];
    sys_call_spec_t cmock_to_return;
    base_status_t a;

    //sys_call_spec_t need to be set manually
    memset(&cmock_to_return, 0, sizeof(sys_call_spec_t));//clear, somehow testing framework keeps old ????
    cmock_to_return.sys_call = NULL;       //irelevant for this test
    cmock_to_return.sys_call_command = ""; //irelevant for this test
    cmock_to_return.num_of_combinators = 4;//from configuration
    cmock_to_return.combinator = NULL;     //irelevant for this test
    int mstp_num_of_cb_tmp[4] = {0, 1, 1, 1};    //taken from configuration
    memcpy(cmock_to_return.num_of_cb, mstp_num_of_cb_tmp, sizeof(int)*4);
    memcpy(cmock_to_return.parse_cb[comb_index][parser_index].match_case, POINT_STR, MAX_STR_LEN);
    //sys_call_spec_t setting is done
    input_str = (char*) malloc(sizeof(char)*MAX_STR_LEN);
    strncpy(input_str, "", MAX_STR_LEN);//different than match case
    memset(&result_array, 0, MAX_NUM_OF_INTERFACES*MAX_STR_LEN);//clear result_array, somehow testing framework keeps old ????

    //mocks
    assemble_sys_call_structure_CMockIgnoreAndReturn(0, cmock_to_return);

    a = array_take_value_after(module_index, obj_index, comb_index, parser_index, input_str, 0, result_array, MAX_NUM_OF_INTERFACES, MAX_STR_LEN);
    //check result_array[0]
    if(0 != strncmp("", result_array[0], MAX_STR_LEN))
    {
        a = BASE_PARSING_SUCCEEDED;//direct fail if result is not good
    }
    TEST_ASSERT_EQUAL_INT_MESSAGE(BASE_PARSING_FAILED, a, "===>>> [result_array[0] is empty string] ===>>> SHOULD RETURN: BASE_PARSING_FAILED, because result_array[0] is empty string");    
}
////*****END OF TESTING array_take_value_after*****////


// BEGINING
////*****TESTS for array_drop_punctuation*****////
//TEST - there is no match case in result_array[0]
void  test_001_array_drop_punctuation(void)
{
    module_index_t module_index = MSTP;//irelevant for this test case (unused parameter)
    int obj_index    = MSTP_PORT_NUM;//irelevant for this test case (unused parameter)
    int comb_index   = 2;//irelevant for this test case (unused parameter)
    int parser_index = 0;//irelevant for this test case (unused parameter)
    char* input_str = NULL; //irelevant for this test case (unused parameter)
    char result_array[MAX_NUM_OF_INTERFACES][MAX_STR_LEN];
    base_status_t a;
    
    memset(&result_array, 0, MAX_NUM_OF_INTERFACES*MAX_STR_LEN);//clear result_array, somehow testing framework keeps old ????
    strncpy(result_array[0], "nothing", MAX_STR_LEN);//different than match case

    a = array_drop_punctuation(module_index, obj_index, comb_index, parser_index, input_str, 0, result_array, MAX_NUM_OF_INTERFACES, MAX_STR_LEN);
    //check result_array[0]
    if(0 != strncmp("nothing", result_array[0], MAX_STR_LEN))
    {
        a = BASE_PARSING_FAILED;//direct fail if result is not good
    }//in normal execution we except FAILED, so in previous block logic is changed that tet can fail
    TEST_ASSERT_EQUAL_INT_MESSAGE(BASE_PARSING_SUCCEEDED, a, "===>>> [there is no match case in result_array[0]] ===>>> SHOULD RETURN: BASE_PARSING_SUCCEEDED, because there is no match case in result_array[0] (still valid for this parser)");    
}

//TEST complicate result_array[0]
void  test_002_array_drop_punctuation(void)
{
    module_index_t module_index = MSTP;//irelevant for this test case (unused parameter)
    int obj_index    = MSTP_PORT_NUM;//irelevant for this test case (unused parameter)
    int comb_index   = 2;//irelevant for this test case (unused parameter)
    int parser_index = 0;//irelevant for this test case (unused parameter)
    char* input_str = NULL; //irelevant for this test case (unused parameter)
    char result_array[MAX_NUM_OF_INTERFACES][MAX_STR_LEN];
    base_status_t a;
    
    memset(&result_array, 0, MAX_NUM_OF_INTERFACES*MAX_STR_LEN);//clear result_array, somehow testing framework keeps old ????
    strncpy(result_array[0], "!10.11+", MAX_STR_LEN);//different than match case

    a = array_drop_punctuation(module_index, obj_index, comb_index, parser_index, input_str, 0, result_array, MAX_NUM_OF_INTERFACES, MAX_STR_LEN);
    //check result_array[0]
    if(0 != strncmp("1011", result_array[0], MAX_STR_LEN))
    {
        a = BASE_PARSING_FAILED;//direct fail if result is not good
    }//in normal execution we except FAILED, so in previous block logic is changed that tet can fail
    TEST_ASSERT_EQUAL_INT_MESSAGE(BASE_PARSING_SUCCEEDED, a, "===>>> [result_array[0] = \"!10.11+\"] ===>>> SHOULD RETURN: BASE_PARSING_SUCCEEDED, because match case is \"!10.11+\"");    
}

//TEST - result_array[0] is empty string
void  test_003_array_drop_punctuation(void)
{
    module_index_t module_index = MSTP;//irelevant for this test case (unused parameter)
    int obj_index    = MSTP_PORT_NUM;//irelevant for this test case (unused parameter)
    int comb_index   = 2;//irelevant for this test case (unused parameter)
    int parser_index = 0;//irelevant for this test case (unused parameter)
    char* input_str = NULL; //irelevant for this test case (unused parameter)
    char result_array[MAX_NUM_OF_INTERFACES][MAX_STR_LEN];
    base_status_t a;

    memset(&result_array, 0, MAX_NUM_OF_INTERFACES*MAX_STR_LEN);//clear result_array, somehow testing framework keeps old ????

    a = array_drop_punctuation(module_index, obj_index, comb_index, parser_index, input_str, 0, result_array, MAX_NUM_OF_INTERFACES, MAX_STR_LEN);
    //check result_array[0]
    if(0 != strncmp("", result_array[0], MAX_STR_LEN))
    {
        a = BASE_PARSING_SUCCEEDED;//direct fail if result is not good
    }
    TEST_ASSERT_EQUAL_INT_MESSAGE(BASE_PARSING_FAILED, a, "===>>> [result_array[0] is empty string] ===>>> SHOULD RETURN: BASE_PARSING_FAILED, because result_array[0] is empty string");    
}
////*****END OF TESTING array_drop_punctuation*****////


// BEGINING
////*****TESTS for array_take_value_after*****////
//TEST - there is no match case in result_array[0]
void  test_001_array_delete_element(void)
{
    module_index_t module_index = MSTP;//valid
    int obj_index    = MSTP_ID;//valid
    int comb_index   = 2;//from configuraion
    int parser_index = 0;//from configuration
    char* input_str = NULL; //irelevant for this test case (unused parameter)
    char result_array[MAX_NUM_OF_INTERFACES][MAX_STR_LEN];
    sys_call_spec_t cmock_to_return;
    base_status_t a;

    //sys_call_spec_t need to be set manually
    memset(&cmock_to_return, 0, sizeof(sys_call_spec_t));//clear, somehow testing framework keeps old ????
    cmock_to_return.sys_call = NULL;       //irelevant for this test
    cmock_to_return.sys_call_command = ""; //irelevant for this test
    cmock_to_return.num_of_combinators = 3;//from configuration
    cmock_to_return.combinator = NULL;     //irelevant for this test
    int mstp_num_of_cb_tmp[3] = {0, 1, 1};    //taken from configuration
    memcpy(cmock_to_return.num_of_cb, mstp_num_of_cb_tmp, sizeof(int)*3);
    memcpy(cmock_to_return.parse_cb[comb_index][parser_index].match_case, BASE_ZERO_STR, MAX_STR_LEN);
    //sys_call_spec_t setting is done
    memset(&result_array, 0, MAX_NUM_OF_INTERFACES*MAX_STR_LEN);//clear result_array, somehow testing framework keeps old ????
    strncpy(result_array[0], "nothing", MAX_STR_LEN);//different than match case
    
    //mocks
    assemble_sys_call_structure_CMockIgnoreAndReturn(0, cmock_to_return);

    a = array_delete_element(module_index, obj_index, comb_index, parser_index, input_str, 0, result_array, MAX_NUM_OF_INTERFACES, MAX_STR_LEN);
    //check result_array[0]
    if(0 != strncmp("nothing", result_array[0], MAX_STR_LEN))
    {
        a = BASE_PARSING_FAILED;//direct fail if result is not good
    }//in normal execution we except FAILED, so in previous block logic is changed that tet can fail
    TEST_ASSERT_EQUAL_INT_MESSAGE(BASE_PARSING_SUCCEEDED, a, "===>>> [there is no match case in result_array[0]] ===>>> SHOULD RETURN: BASE_PARSING_SUCCEEDED, because this parser just deletes elements");    
}

//TEST - assemble_sys_call_structure returns structure filled with 0s 
void  test_002_array_delete_element(void)
{
    module_index_t module_index = MSTP;//valid
    int obj_index    = MSTP_ID;//valid
    int comb_index   = 2;//from configuraion
    int parser_index = 0;//from configuration
    char* input_str = NULL; //irelevant for this test case (unused parameter)
    char result_array[MAX_NUM_OF_INTERFACES][MAX_STR_LEN];
    sys_call_spec_t cmock_to_return;
    base_status_t a;
    
    //sys_call_spec_t need to be set manually
    memset(&cmock_to_return, 0, sizeof(cmock_to_return));
    //sys_call_spec_t setting is done
    memset(&result_array, 0, MAX_NUM_OF_INTERFACES*MAX_STR_LEN);//clear result_array, somehow testing framework keeps old ????
    strncpy(result_array[0], "something", MAX_STR_LEN);//different than match case
    
    //mocks
    assemble_sys_call_structure_CMockIgnoreAndReturn(0, cmock_to_return);

    a = array_delete_element(module_index, obj_index, comb_index, parser_index, input_str, 0, result_array, MAX_NUM_OF_INTERFACES, MAX_STR_LEN);
    //check result_array[0]
    //NO NEED, part with this manipulation is not REACHED
    TEST_ASSERT_EQUAL_INT_MESSAGE(BASE_PARSING_SUCCEEDED, a, "===>>> [assemble_sys_call_structure returns structure filled with 0s] ===>>> SHOULD RETURN: BASE_PARSING_SUCCEEDED, because this function does not care ");    
}

//TEST - match case is "" 
void  test_003_array_delete_element(void)
{
    module_index_t module_index = MSTP;//valid
    int obj_index    = MSTP_ID;//valid
    int comb_index   = 2;//from configuraion
    int parser_index = 0;//from configuration
    char* input_str = NULL; //irelevant for this test case (unused parameter)
    char result_array[MAX_NUM_OF_INTERFACES][MAX_STR_LEN];
    sys_call_spec_t cmock_to_return;
    base_status_t a;

    //sys_call_spec_t need to be set manually
    memset(&cmock_to_return, 0, sizeof(sys_call_spec_t));//clear, somehow testing framework keeps old ????
    cmock_to_return.sys_call = NULL;       //irelevant for this test
    cmock_to_return.sys_call_command = ""; //irelevant for this test
    cmock_to_return.num_of_combinators = 3;//from configuration
    cmock_to_return.combinator = NULL;     //irelevant for this test
    int mstp_num_of_cb_tmp[3] = {0, 1, 1};    //taken from configuration
    memcpy(cmock_to_return.num_of_cb, mstp_num_of_cb_tmp, sizeof(int)*3);
    memcpy(cmock_to_return.parse_cb[comb_index][parser_index].match_case, "", MAX_STR_LEN);
    //sys_call_spec_t setting is done
    memset(&result_array, 0, MAX_NUM_OF_INTERFACES*MAX_STR_LEN);//clear result_array, somehow testing framework keeps old ????
    strncpy(result_array[0], "something", MAX_STR_LEN);//different than match case
    
    //mocks
    assemble_sys_call_structure_CMockIgnoreAndReturn(0, cmock_to_return);

    a = array_delete_element(module_index, obj_index, comb_index, parser_index, input_str, 0, result_array, MAX_NUM_OF_INTERFACES, MAX_STR_LEN);
    //check result_array[0]
    if(0 != strncmp("something", result_array[0], MAX_STR_LEN))
    {
        a = BASE_PARSING_FAILED;//direct fail if result is not good
    }//in normal execution we except FAILED, so in previous block logic is changed that tet can fail
    TEST_ASSERT_EQUAL_INT_MESSAGE(BASE_PARSING_SUCCEEDED, a, "===>>> [match case is \"\"] ===>>> SHOULD RETURN: BASE_PARSING_SUCCEEDED, because this function does not care");    
}

//TEST - result_array[0] is empty string
void  test_004_array_delete_element(void)
{
    module_index_t module_index = MSTP;//valid
    int obj_index    = MSTP_ID;//valid
    int comb_index   = 2;//from configuraion
    int parser_index = 0;//from configuration
    char* input_str = NULL; //irelevant for this test case (unused parameter)
    char result_array[MAX_NUM_OF_INTERFACES][MAX_STR_LEN];
    sys_call_spec_t cmock_to_return;
    base_status_t a;

    //sys_call_spec_t need to be set manually
    memset(&cmock_to_return, 0, sizeof(sys_call_spec_t));//clear, somehow testing framework keeps old ????
    cmock_to_return.sys_call = NULL;       //irelevant for this test
    cmock_to_return.sys_call_command = ""; //irelevant for this test
    cmock_to_return.num_of_combinators = 3;//from configuration
    cmock_to_return.combinator = NULL;     //irelevant for this test
    int mstp_num_of_cb_tmp[3] = {0, 1, 1};    //taken from configuration
    memcpy(cmock_to_return.num_of_cb, mstp_num_of_cb_tmp, sizeof(int)*3);
    memcpy(cmock_to_return.parse_cb[comb_index][parser_index].match_case, BASE_ZERO_STR, MAX_STR_LEN);
    //sys_call_spec_t setting is done
    memset(&result_array, 0, MAX_NUM_OF_INTERFACES*MAX_STR_LEN);//clear result_array, somehow testing framework keeps old ????
    strncpy(result_array[0], "", MAX_STR_LEN);

    //mocks
    assemble_sys_call_structure_CMockIgnoreAndReturn(0, cmock_to_return);

    a = array_delete_element(module_index, obj_index, comb_index, parser_index, input_str, 0, result_array, MAX_NUM_OF_INTERFACES, MAX_STR_LEN);
    //check result_array[0]
    if(0 != strncmp("", result_array[0], MAX_STR_LEN))
    {
        a = BASE_PARSING_FAILED;//direct fail if result is not good
    }
    TEST_ASSERT_EQUAL_INT_MESSAGE(BASE_PARSING_SUCCEEDED, a, "===>>> [result_array[0] is empty string] ===>>> SHOULD RETURN: BASE_PARSING_SUCCEEDED, because this function does not care");
}

//TEST - result_array[1] is same as match case, need to be deleted from array
void  test_005_array_delete_element(void)
{
    module_index_t module_index = MSTP;//valid
    int obj_index    = MSTP_ID;//valid
    int comb_index   = 2;//from configuraion
    int parser_index = 0;//from configuration
    char* input_str = NULL; //irelevant for this test case (unused parameter)
    char result_array[MAX_NUM_OF_INTERFACES][MAX_STR_LEN];
    sys_call_spec_t cmock_to_return;
    base_status_t a;

    //sys_call_spec_t need to be set manually
    memset(&cmock_to_return, 0, sizeof(sys_call_spec_t));//clear, somehow testing framework keeps old ????
    cmock_to_return.sys_call = NULL;       //irelevant for this test
    cmock_to_return.sys_call_command = ""; //irelevant for this test
    cmock_to_return.num_of_combinators = 3;//from configuration
    cmock_to_return.combinator = NULL;     //irelevant for this test
    int mstp_num_of_cb_tmp[3] = {0, 1, 1};    //taken from configuration
    memcpy(cmock_to_return.num_of_cb, mstp_num_of_cb_tmp, sizeof(int)*3);
    memcpy(cmock_to_return.parse_cb[comb_index][parser_index].match_case, "drop", MAX_STR_LEN);
    //sys_call_spec_t setting is done
    memset(&result_array, 0, MAX_NUM_OF_INTERFACES*MAX_STR_LEN*sizeof(char));//clear result_array, somehow testing framework keeps old ????
    strncpy(result_array[0], "something", MAX_STR_LEN);
    strncpy(result_array[1], "drop", MAX_STR_LEN);
    
    //mocks
    assemble_sys_call_structure_CMockIgnoreAndReturn(0, cmock_to_return);

    a = array_delete_element(module_index, obj_index, comb_index, parser_index, input_str, 0, result_array, MAX_NUM_OF_INTERFACES, MAX_STR_LEN);
    //check result_array[0]
    if((0 != strncmp("something", result_array[0], MAX_STR_LEN)) || (0 != strncmp("", result_array[1], MAX_STR_LEN)))
    {
        a = BASE_PARSING_FAILED;//direct fail if result is not good
    }
    TEST_ASSERT_EQUAL_INT_MESSAGE(BASE_PARSING_SUCCEEDED, a, "===>>> [result_array[1] is same as match case, need to be deleted from array] ===>>> SHOULD RETURN: BASE_PARSING_SUCCEEDED, because this function does not care");
}
////*****END OF TESTING array_delete_element*****////


// BEGINING
////*****TESTS for array_bitwise_AND_mask*****////
//TEST - result_array[0] = "FF", [match_case] = 01
void  test_001_array_bitwise_AND_mask(void)
{
    module_index_t module_index = MSTP;//valid
    int obj_index    = MSTP_PORT_NUM;//valid
    int comb_index   = 3;//from configuraion
    int parser_index = 0;//from configuration
    char* input_str = NULL; //irelevant for this test case (unused parameter)
    char result_array[MAX_NUM_OF_INTERFACES][MAX_STR_LEN];
    sys_call_spec_t cmock_to_return;
    base_status_t a;

    //sys_call_spec_t need to be set manually
    memset(&cmock_to_return, 0, sizeof(sys_call_spec_t));//clear, somehow testing framework keeps old ????
    cmock_to_return.sys_call = NULL;       //irelevant for this test
    cmock_to_return.sys_call_command = ""; //irelevant for this test
    cmock_to_return.num_of_combinators = 4;//from configuration
    cmock_to_return.combinator = NULL;     //irelevant for this test
    int mstp_num_of_cb_tmp[4] = {0, 1, 1, 1};    //taken from configuration
    memcpy(cmock_to_return.num_of_cb, mstp_num_of_cb_tmp, sizeof(int)*4);
    memcpy(cmock_to_return.parse_cb[comb_index][parser_index].match_case, "FF", MAX_STR_LEN);
    //sys_call_spec_t setting is done
    memset(&result_array, 0, MAX_NUM_OF_INTERFACES*MAX_STR_LEN);//clear result_array, somehow testing framework keeps old ????
    strncpy(result_array[0], "01", MAX_STR_LEN);//different than match case

    //mocks
    assemble_sys_call_structure_CMockIgnoreAndReturn(0, cmock_to_return);
    a = array_bitwise_AND_mask(module_index, obj_index, comb_index, parser_index, input_str, 0, result_array, MAX_NUM_OF_INTERFACES, MAX_STR_LEN);
    //check result_array[0]
    if(0 != strncmp("1", result_array[0], MAX_STR_LEN))
    {
        a = BASE_PARSING_FAILED;//direct fail if result is not good
    }
    TEST_ASSERT_EQUAL_INT_MESSAGE(BASE_PARSING_SUCCEEDED, a, "===>>> //TEST - result_array[0] = \"FF\", [match_case] = \"01\" ===>>> SHOULD RETURN: BASE_PARSING_FAILED, because there is no match case in result_array[0]");
}

//TEST - invalid match case
void  test_002_array_bitwise_AND_mask(void)
{
    module_index_t module_index = MSTP;//valid
    int obj_index    = MSTP_PORT_NUM;//valid
    int comb_index   = 3;//from configuraion
    int parser_index = 0;//from configuration
    char* input_str = NULL; //irelevant for this test case (unused parameter)
    char result_array[MAX_NUM_OF_INTERFACES][MAX_STR_LEN];
    sys_call_spec_t cmock_to_return;
    base_status_t a;

    //sys_call_spec_t need to be set manually
    memset(&cmock_to_return, 0, sizeof(sys_call_spec_t));//clear, somehow testing framework keeps old ????
    cmock_to_return.sys_call = NULL;       //irelevant for this test
    cmock_to_return.sys_call_command = ""; //irelevant for this test
    cmock_to_return.num_of_combinators = 4;//from configuration
    cmock_to_return.combinator = NULL;     //irelevant for this test
    int mstp_num_of_cb_tmp[4] = {0, 1, 1, 1};    //taken from configuration
    memcpy(cmock_to_return.num_of_cb, mstp_num_of_cb_tmp, sizeof(int)*4);
    memcpy(cmock_to_return.parse_cb[comb_index][parser_index].match_case, "gg", MAX_STR_LEN);
    //sys_call_spec_t setting is done
    memset(&result_array, 0, MAX_NUM_OF_INTERFACES*MAX_STR_LEN);//clear result_array, somehow testing framework keeps old ????
    strncpy(result_array[0], "0F", MAX_STR_LEN);//different than match case
    
    //mocks
    assemble_sys_call_structure_CMockIgnoreAndReturn(0, cmock_to_return);
    a = array_bitwise_AND_mask(module_index, obj_index, comb_index, parser_index, input_str, 0, result_array, MAX_NUM_OF_INTERFACES, MAX_STR_LEN);

    TEST_ASSERT_EQUAL_INT_MESSAGE(BASE_PARSING_FAILED, a, "===>>> [there is no match case in result_array[0]] ===>>> SHOULD RETURN: BASE_PARSING_FAILED, because there is no match case in result_array[0]");    
}

//TEST - assemble_sys_call_structure returns structure filled with 0s 
void  test_003_array_bitwise_AND_mask(void)
{
    module_index_t module_index = MSTP;//valid
    int obj_index    = MSTP_PORT_NAMES;//valid
    int comb_index   = 3;//valid, 2 is from configuraion
    int parser_index = 0;//valid, 0 is from configuration
    char* input_str = NULL; //invalid, unused
    char result_array[MAX_NUM_OF_INTERFACES][MAX_STR_LEN];//irelevant for this test case
    sys_call_spec_t cmock_to_return;
    base_status_t a;
    
    //sys_call_spec_t need to be set manually
    memset(&cmock_to_return, 0, sizeof(cmock_to_return));
    //sys_call_spec_t setting is done
    memset(&result_array, 0, MAX_NUM_OF_INTERFACES*MAX_STR_LEN);//clear result_array, somehow testing framework keeps old ????
    strncpy(result_array[0], "10.11", MAX_STR_LEN);//different than match case
    
    //mocks
    assemble_sys_call_structure_CMockIgnoreAndReturn(0, cmock_to_return);

    a = array_bitwise_AND_mask(module_index, obj_index, comb_index, parser_index, input_str, 0, result_array, MAX_NUM_OF_INTERFACES, MAX_STR_LEN);
    //check result_array[0]
    //NO NEED, part with this manipulation is not REACHED
    TEST_ASSERT_EQUAL_INT_MESSAGE(BASE_PARSING_FAILED, a, "===>>> [assemble_sys_call_structure returns structure filled with 0s] ===>>> SHOULD RETURN: BASE_PARSING_FAILED, because assemble_sys_call_structure returns structure filled with 0s ");    
}

//TEST - match case is "" 
void  test_004_array_bitwise_AND_mask(void)
{
    module_index_t module_index = MSTP;//valid
    int obj_index    = MSTP_PORT_NUM;//valid
    int comb_index   = 3;//from configuraion
    int parser_index = 0;//from configuration
    char* input_str = NULL; //irelevant for this test case (unused parameter)
    char result_array[MAX_NUM_OF_INTERFACES][MAX_STR_LEN];
    sys_call_spec_t cmock_to_return;
    base_status_t a;

    //sys_call_spec_t need to be set manually
    memset(&cmock_to_return, 0, sizeof(sys_call_spec_t));//clear, somehow testing framework keeps old ????
    cmock_to_return.sys_call = NULL;       //irelevant for this test
    cmock_to_return.sys_call_command = ""; //irelevant for this test
    cmock_to_return.num_of_combinators = 4;//from configuration
    cmock_to_return.combinator = NULL;     //irelevant for this test
    int mstp_num_of_cb_tmp[4] = {0, 1, 1, 1};    //taken from configuration
    memcpy(cmock_to_return.num_of_cb, mstp_num_of_cb_tmp, sizeof(int)*4);
    memcpy(cmock_to_return.parse_cb[comb_index][parser_index].match_case, "", MAX_STR_LEN);
    //sys_call_spec_t setting is done
    memset(&result_array, 0, MAX_NUM_OF_INTERFACES*MAX_STR_LEN);//clear result_array, somehow testing framework keeps old ????
    strncpy(result_array[0], "0F", MAX_STR_LEN);//different than match case
    
    //mocks
    assemble_sys_call_structure_CMockIgnoreAndReturn(0, cmock_to_return);

    a = array_bitwise_AND_mask(module_index, obj_index, comb_index, parser_index, input_str, 0, result_array, MAX_NUM_OF_INTERFACES, MAX_STR_LEN);
    //check result_array[0]
    //NO NEED, part with this manipulation is not REACHED
    TEST_ASSERT_EQUAL_INT_MESSAGE(BASE_PARSING_FAILED, a, "===>>> [match case is \"\"] ===>>> SHOULD RETURN: BASE_PARSING_FAILED, because match case is \"\"");    
}

//TEST - result_array[0] is empty string
void  test_005_array_bitwise_AND_mask(void)
{
    module_index_t module_index = MSTP;//valid
    int obj_index    = MSTP_PORT_NUM;//valid
    int comb_index   = 3;//from configuraion
    int parser_index = 0;//from configuration
    char* input_str = NULL; //irelevant for this test case (unused parameter)
    char result_array[MAX_NUM_OF_INTERFACES][MAX_STR_LEN];
    sys_call_spec_t cmock_to_return;
    base_status_t a;

    //sys_call_spec_t need to be set manually
    memset(&cmock_to_return, 0, sizeof(sys_call_spec_t));//clear, somehow testing framework keeps old ????
    cmock_to_return.sys_call = NULL;       //irelevant for this test
    cmock_to_return.sys_call_command = ""; //irelevant for this test
    cmock_to_return.num_of_combinators = 4;//from configuration
    cmock_to_return.combinator = NULL;     //irelevant for this test
    int mstp_num_of_cb_tmp[4] = {0, 1, 1, 1};    //taken from configuration
    memcpy(cmock_to_return.num_of_cb, mstp_num_of_cb_tmp, sizeof(int)*4);
    parser_spec_t mstp_parsing_cb_tmp[4][2]={//taken from configuration
        [0] = {{NULL, ""}}, //irelevant
        [1] = {{NULL, ""}},
        [2] = {{NULL, POINT_STR}},
        [3] = {{NULL, X0F_MASK_STR}}
    };
    memcpy(cmock_to_return.parse_cb, mstp_parsing_cb_tmp, sizeof(parser_spec_t)*4*2);
    //sys_call_spec_t setting is done
    memset(&result_array, 0, MAX_NUM_OF_INTERFACES*MAX_STR_LEN);//clear result_array, somehow testing framework keeps old ????
    strncpy(result_array[0], "", MAX_STR_LEN);
    //mocks
    assemble_sys_call_structure_CMockIgnoreAndReturn(0, cmock_to_return);

    a = array_bitwise_AND_mask(module_index, obj_index, comb_index, parser_index, input_str, 0, result_array, MAX_NUM_OF_INTERFACES, MAX_STR_LEN);
    //check result_array[0]
    if(0 != strncmp("", result_array[0], MAX_STR_LEN))
    {
        a = BASE_PARSING_SUCCEEDED;//direct fail if result is not good
    }
    TEST_ASSERT_EQUAL_INT_MESSAGE(BASE_PARSING_FAILED, a, "===>>> [result_array[0] is empty string] ===>>> SHOULD RETURN: BASE_PARSING_FAILED, because result_array[0] is empty string");    
}
////*****END OF TESTING array_bitwise_AND_mask*****////


// BEGINING
////*****TESTS for array_multiply_long_int_with*****////
//TEST - match_case and result_array[0] are valid
void  test_001_array_multiply_long_int_with(void)
{
    module_index_t module_index = MSTP;//valid
    int obj_index    = MSTP_CIST_PORT_HELLO_TIME;//valid
    int comb_index   = 2;//from configuraion
    int parser_index = 0;//from configuration
    char* input_str = NULL; //irelevant for this test case (unused parameter)
    char result_array[MAX_NUM_OF_INTERFACES][MAX_STR_LEN];
    sys_call_spec_t cmock_to_return;
    base_status_t a;

    //sys_call_spec_t need to be set manually
    memset(&cmock_to_return, 0, sizeof(sys_call_spec_t));//clear, somehow testing framework keeps old ????
    cmock_to_return.sys_call = NULL;       //irelevant for this test
    cmock_to_return.sys_call_command = ""; //irelevant for this test
    cmock_to_return.num_of_combinators = 3;//from configuration
    cmock_to_return.combinator = NULL;     //irelevant for this test
    int mstp_num_of_cb_tmp[3] = {0, 2, 1};    //taken from configuration
    memcpy(cmock_to_return.num_of_cb, mstp_num_of_cb_tmp, sizeof(int)*3);
    memcpy(cmock_to_return.parse_cb[comb_index][parser_index].match_case, BASE_ONE_HUNDRED_STR, MAX_STR_LEN);// valid
    //sys_call_spec_t setting is done
    memset(&result_array, 0, MAX_NUM_OF_INTERFACES*MAX_STR_LEN);//clear result_array, somehow testing framework keeps old ????
    strncpy(result_array[0], "10", MAX_STR_LEN);//valid
    
    //mocks
    assemble_sys_call_structure_CMockIgnoreAndReturn(0, cmock_to_return);

    a = array_multiply_long_int_with(module_index, obj_index, comb_index, parser_index, input_str, 0, result_array, MAX_NUM_OF_INTERFACES, MAX_STR_LEN);
    //check result_array[0]
    if(0 != strncmp("1000", result_array[0], MAX_STR_LEN))
    {
        a = BASE_PARSING_FAILED;//direct fail if result is not good
    }//in normal execution we except FAILED, so in previous block logic is changed that tet can fail
    TEST_ASSERT_EQUAL_INT_MESSAGE(BASE_PARSING_SUCCEEDED, a, "===>>> [match_case and result_array[0] are valid] ===>>> SHOULD RETURN: BASE_PARSING_SUCCEEDED, because match_case and result_array[0] are valid");
}

//TEST - assemble_sys_call_structure returns structure filled with 0s 
void  test_002_array_multiply_long_int_with(void)
{
    module_index_t module_index = MSTP;//valid
    int obj_index    = MSTP_CIST_PORT_HELLO_TIME;//valid
    int comb_index   = 2;//from configuraion
    int parser_index = 0;//from configuration
    char* input_str = NULL; //irelevant for this test case (unused parameter)
    char result_array[MAX_NUM_OF_INTERFACES][MAX_STR_LEN];
    sys_call_spec_t cmock_to_return;
    base_status_t a;
    
    //sys_call_spec_t need to be set manually
    memset(&cmock_to_return, 0, sizeof(cmock_to_return));
    //sys_call_spec_t setting is done
    memset(&result_array, 0, MAX_NUM_OF_INTERFACES*MAX_STR_LEN);//clear result_array, somehow testing framework keeps old ????
    strncpy(result_array[0], "10", MAX_STR_LEN);
    
    //mocks
    assemble_sys_call_structure_CMockIgnoreAndReturn(0, cmock_to_return);

    a = array_multiply_long_int_with(module_index, obj_index, comb_index, parser_index, input_str, 0, result_array, MAX_NUM_OF_INTERFACES, MAX_STR_LEN);
    //check result_array[0]
    //NO NEED, part with this manipulation is not REACHED
    TEST_ASSERT_EQUAL_INT_MESSAGE(BASE_PARSING_FAILED, a, "===>>> [assemble_sys_call_structure returns structure filled with 0s] ===>>> SHOULD RETURN: BASE_PARSING_FAILED, because assemble_sys_call_structure returns structure filled with 0s ");    
}

//TEST - match case is "" 
void  test_003_array_multiply_long_int_with(void)
{
    module_index_t module_index = MSTP;//valid
    int obj_index    = MSTP_CIST_PORT_HELLO_TIME;//valid
    int comb_index   = 2;//from configuraion
    int parser_index = 0;//from configuration
    char* input_str = NULL; //irelevant for this test case (unused parameter)
    char result_array[MAX_NUM_OF_INTERFACES][MAX_STR_LEN];
    sys_call_spec_t cmock_to_return;
    base_status_t a;

    //sys_call_spec_t need to be set manually
    memset(&cmock_to_return, 0, sizeof(sys_call_spec_t));//clear, somehow testing framework keeps old ????
    cmock_to_return.sys_call = NULL;       //irelevant for this test
    cmock_to_return.sys_call_command = ""; //irelevant for this test
    cmock_to_return.num_of_combinators = 3;//from configuration
    cmock_to_return.combinator = NULL;     //irelevant for this test
    int mstp_num_of_cb_tmp[3] = {0, 2, 1};    //taken from configuration
    memcpy(cmock_to_return.num_of_cb, mstp_num_of_cb_tmp, sizeof(int)*3);
    memcpy(cmock_to_return.parse_cb[comb_index][parser_index].match_case, "", MAX_STR_LEN);// valid
    //sys_call_spec_t setting is done
    memset(&result_array, 0, MAX_NUM_OF_INTERFACES*MAX_STR_LEN);//clear result_array, somehow testing framework keeps old ????
    strncpy(result_array[0], "10", MAX_STR_LEN);//different than match case
    
    //mocks
    assemble_sys_call_structure_CMockIgnoreAndReturn(0, cmock_to_return);

    a = array_multiply_long_int_with(module_index, obj_index, comb_index, parser_index, input_str, 0, result_array, MAX_NUM_OF_INTERFACES, MAX_STR_LEN);
    //check result_array[0]
    //NO NEED, part with this manipulation is not REACHED
    TEST_ASSERT_EQUAL_INT_MESSAGE(BASE_PARSING_FAILED, a, "===>>> [match case is \"\"] ===>>> SHOULD RETURN: BASE_PARSING_FAILED, because match case is \"\"");    
}

//TEST - result_array[0] is empty string
void  test_004_array_multiply_long_int_with(void)
{
    module_index_t module_index = MSTP;//valid
    int obj_index    = MSTP_CIST_PORT_HELLO_TIME;//valid
    int comb_index   = 2;//from configuraion
    int parser_index = 0;//from configuration
    char* input_str = NULL; //irelevant for this test case (unused parameter)
    char result_array[MAX_NUM_OF_INTERFACES][MAX_STR_LEN];
    sys_call_spec_t cmock_to_return;
    base_status_t a;

    //sys_call_spec_t need to be set manually
    memset(&cmock_to_return, 0, sizeof(sys_call_spec_t));//clear, somehow testing framework keeps old ????
    cmock_to_return.sys_call = NULL;       //irelevant for this test
    cmock_to_return.sys_call_command = ""; //irelevant for this test
    cmock_to_return.num_of_combinators = 3;//from configuration
    cmock_to_return.combinator = NULL;     //irelevant for this test
    int mstp_num_of_cb_tmp[3] = {0, 2, 1};    //taken from configuration
    memcpy(cmock_to_return.num_of_cb, mstp_num_of_cb_tmp, sizeof(int)*3);
    memcpy(cmock_to_return.parse_cb[comb_index][parser_index].match_case, BASE_ONE_HUNDRED_STR, MAX_STR_LEN);// valid
    //sys_call_spec_t setting is done
    memset(&result_array, 0, MAX_NUM_OF_INTERFACES*MAX_STR_LEN);//clear result_array, somehow testing framework keeps old ????
    strncpy(result_array[0], "", MAX_STR_LEN);//different than match case

    //mocks
    assemble_sys_call_structure_CMockIgnoreAndReturn(0, cmock_to_return);

    a = array_multiply_long_int_with(module_index, obj_index, comb_index, parser_index, input_str, 0, result_array, MAX_NUM_OF_INTERFACES, MAX_STR_LEN);
    //check result_array[0]
    if(0 != strncmp("", result_array[0], MAX_STR_LEN))
    {
        a = BASE_PARSING_SUCCEEDED;//direct fail if result is not good
    }
    TEST_ASSERT_EQUAL_INT_MESSAGE(BASE_PARSING_FAILED, a, "===>>> [result_array[0] is empty string] ===>>> SHOULD RETURN: BASE_PARSING_FAILED, because result_array[0] is empty string");    
}
////*****END OF TESTING array_multiply_long_int_with*****////


// BEGINING
////*****TESTS for array_shift_right*****////
//TEST - match_case and result_array[0] are valid
void  test_001_array_shift_right(void)
{
    module_index_t module_index = MSTP;//valid
    int obj_index    = MSTP_PORT_NUM;//valid
    int comb_index   = 3;//from configuraion
    int parser_index = 0;//from configuration
    char* input_str = NULL; //irelevant for this test case (unused parameter)
    char result_array[MAX_NUM_OF_INTERFACES][MAX_STR_LEN];
    sys_call_spec_t cmock_to_return;
    base_status_t a;

    //sys_call_spec_t need to be set manually
    memset(&cmock_to_return, 0, sizeof(sys_call_spec_t));//clear, somehow testing framework keeps old ????
    cmock_to_return.sys_call = NULL;       //irelevant for this test
    cmock_to_return.sys_call_command = ""; //irelevant for this test
    cmock_to_return.num_of_combinators = 4;//from configuration
    cmock_to_return.combinator = NULL;     //irelevant for this test
    int mstp_num_of_cb_tmp[4] = {0, 1, 1, 1};    //taken from configuration
    memcpy(cmock_to_return.num_of_cb, mstp_num_of_cb_tmp, sizeof(int)*4);
    memcpy(cmock_to_return.parse_cb[comb_index][parser_index].match_case, BASE_EIGHT_STR, MAX_STR_LEN);// valid
    //sys_call_spec_t setting is done
    memset(&result_array, 0, MAX_NUM_OF_INTERFACES*MAX_STR_LEN);//clear result_array, somehow testing framework keeps old ????
    strncpy(result_array[0], "FFF00000", MAX_STR_LEN);//different than match case
    
    //mocks
    assemble_sys_call_structure_CMockIgnoreAndReturn(0, cmock_to_return);

    a = array_shift_right(module_index, obj_index, comb_index, parser_index, input_str, 0, result_array, MAX_NUM_OF_INTERFACES, MAX_STR_LEN);
    //check result_array[0]
    if(0 != strncmp("16773120", result_array[0], MAX_STR_LEN))// FFF000 is 16773120 as int
    {
        a = BASE_PARSING_FAILED;//direct fail if result is not good
    }//in normal execution we except FAILED, so in previous block logic is changed that tet can fail
    TEST_ASSERT_EQUAL_INT_MESSAGE(BASE_PARSING_SUCCEEDED, a, "===>>> [match_case and result_array[0] are valid] ===>>> SHOULD RETURN: BASE_PARSING_SUCCEEDED");
}

//TEST - assemble_sys_call_structure returns structure filled with 0s 
void  test_002_array_shift_right(void)
{
    module_index_t module_index = MSTP;//valid
    int obj_index    = MSTP_PORT_NAMES;//valid
    int comb_index   = 3;//valid, 2 is from configuraion
    int parser_index = 0;//valid, 0 is from configuration
    char* input_str = NULL; //invalid, unused
    char result_array[MAX_NUM_OF_INTERFACES][MAX_STR_LEN];//irelevant for this test case
    sys_call_spec_t cmock_to_return;
    base_status_t a;
    
    //sys_call_spec_t need to be set manually
    memset(&cmock_to_return, 0, sizeof(cmock_to_return));
    //sys_call_spec_t setting is done
    memset(&result_array, 0, MAX_NUM_OF_INTERFACES*MAX_STR_LEN);//clear result_array, somehow testing framework keeps old ????
    strncpy(result_array[0], "FFFF", MAX_STR_LEN);//different than match case
    
    //mocks
    assemble_sys_call_structure_CMockIgnoreAndReturn(0, cmock_to_return);

    a = array_shift_right(module_index, obj_index, comb_index, parser_index, input_str, 0, result_array, MAX_NUM_OF_INTERFACES, MAX_STR_LEN);
    //check result_array[0]
    //NO NEED, part with this manipulation is not REACHED
    TEST_ASSERT_EQUAL_INT_MESSAGE(BASE_PARSING_FAILED, a, "===>>> [assemble_sys_call_structure returns structure filled with 0s] ===>>> SHOULD RETURN: BASE_PARSING_FAILED, because assemble_sys_call_structure returns structure filled with 0s ");    
}

//TEST - match case is "" 
void  test_003_array_shift_right(void)
{
    module_index_t module_index = MSTP;//valid
    int obj_index    = MSTP_PORT_NUM;//valid
    int comb_index   = 3;//from configuraion
    int parser_index = 0;//from configuration
    char* input_str = NULL; //irelevant for this test case (unused parameter)
    char result_array[MAX_NUM_OF_INTERFACES][MAX_STR_LEN];
    sys_call_spec_t cmock_to_return;
    base_status_t a;

    //sys_call_spec_t need to be set manually
    memset(&cmock_to_return, 0, sizeof(sys_call_spec_t));//clear, somehow testing framework keeps old ????
    cmock_to_return.sys_call = NULL;       //irelevant for this test
    cmock_to_return.sys_call_command = ""; //irelevant for this test
    cmock_to_return.num_of_combinators = 4;//from configuration
    cmock_to_return.combinator = NULL;     //irelevant for this test
    int mstp_num_of_cb_tmp[4] = {0, 1, 1, 1};    //taken from configuration
    memcpy(cmock_to_return.num_of_cb, mstp_num_of_cb_tmp, sizeof(int)*4);
    memcpy(cmock_to_return.parse_cb[comb_index][parser_index].match_case, "", MAX_STR_LEN);// valid
    //sys_call_spec_t setting is done
    memset(&result_array, 0, MAX_NUM_OF_INTERFACES*MAX_STR_LEN);//clear result_array, somehow testing framework keeps old ????
    strncpy(result_array[0], "FFFF", MAX_STR_LEN);//different than match case
    
    //mocks
    assemble_sys_call_structure_CMockIgnoreAndReturn(0, cmock_to_return);

    a = array_shift_right(module_index, obj_index, comb_index, parser_index, input_str, 0, result_array, MAX_NUM_OF_INTERFACES, MAX_STR_LEN);
    //check result_array[0]
    //NO NEED, part with this manipulation is not REACHED
    TEST_ASSERT_EQUAL_INT_MESSAGE(BASE_PARSING_FAILED, a, "===>>> [match case is \"\"] ===>>> SHOULD RETURN: BASE_PARSING_FAILED, because match case is \"\"");    
}

//TEST - result_array[0] is empty string
void  test_004_array_shift_right(void)
{
    module_index_t module_index = MSTP;//valid
    int obj_index    = MSTP_PORT_NUM;//valid
    int comb_index   = 3;//from configuraion
    int parser_index = 0;//from configuration
    char* input_str = NULL; //irelevant for this test case (unused parameter)
    char result_array[MAX_NUM_OF_INTERFACES][MAX_STR_LEN];
    sys_call_spec_t cmock_to_return;
    base_status_t a;

    //sys_call_spec_t need to be set manually
    memset(&cmock_to_return, 0, sizeof(sys_call_spec_t));//clear, somehow testing framework keeps old ????
    cmock_to_return.sys_call = NULL;       //irelevant for this test
    cmock_to_return.sys_call_command = ""; //irelevant for this test
    cmock_to_return.num_of_combinators = 4;//from configuration
    cmock_to_return.combinator = NULL;     //irelevant for this test
    int mstp_num_of_cb_tmp[4] = {0, 1, 1, 1};    //taken from configuration
    memcpy(cmock_to_return.num_of_cb, mstp_num_of_cb_tmp, sizeof(int)*4);
    memcpy(cmock_to_return.parse_cb[comb_index][parser_index].match_case, BASE_EIGHT_STR, MAX_STR_LEN);// valid
    //sys_call_spec_t setting is done
    memset(&result_array, 0, MAX_NUM_OF_INTERFACES*MAX_STR_LEN);//clear result_array, somehow testing framework keeps old ????
    strncpy(result_array[0], "", MAX_STR_LEN);
    //mocks
    assemble_sys_call_structure_CMockIgnoreAndReturn(0, cmock_to_return);

    a = array_shift_right(module_index, obj_index, comb_index, parser_index, input_str, 0, result_array, MAX_NUM_OF_INTERFACES, MAX_STR_LEN);
    //check result_array[0]
    if(0 != strncmp("", result_array[0], MAX_STR_LEN))
    {
        a = BASE_PARSING_SUCCEEDED;//direct fail if result is not good
    }
    TEST_ASSERT_EQUAL_INT_MESSAGE(BASE_PARSING_FAILED, a, "===>>> [result_array[0] is empty string] ===>>> SHOULD RETURN: BASE_PARSING_FAILED, because result_array[0] is empty string");    
}
////*****END OF TESTING array_shift_right*****////


////*****TESTS for array_convert_hex_string_to_long_int*****////
//TEST - there is no match case in result_array[0]
void  test_001_array_convert_hex_string_to_long_int(void)
{
    module_index_t module_index = MSTP;//valid
    int obj_index    = MSTP_BRIDGE_PRIORITY;//valid
    int comb_index   = 4;//from configuraion
    int parser_index = 0;//from configuration
    char* input_str = NULL; //irelevant for this test case (unused parameter)
    char result_array[MAX_NUM_OF_INTERFACES][MAX_STR_LEN];
    sys_call_spec_t cmock_to_return;
    base_status_t a;
    
    memset(&result_array, 0, MAX_NUM_OF_INTERFACES*MAX_STR_LEN);//clear result_array, somehow testing framework keeps old ????
    strncpy(result_array[0], "YUV", MAX_STR_LEN);//different than match case

    a = array_convert_hex_string_to_long_int(module_index, obj_index, comb_index, parser_index, input_str, 0, result_array, MAX_NUM_OF_INTERFACES, MAX_STR_LEN);

    TEST_ASSERT_EQUAL_INT_MESSAGE(BASE_PARSING_FAILED, a, "===>>> [result_array[0] = \"YUV\"] ===>>> SHOULD RETURN: BASE_PARSING_SUCCEEDED, because result_array[0] is invalid");
}

//TEST complicate result_array[0]
void  test_002_array_convert_hex_string_to_long_int(void)
{
    module_index_t module_index = MSTP;//valid
    int obj_index    = MSTP_BRIDGE_PRIORITY;//valid
    int comb_index   = 4;//from configuraion
    int parser_index = 0;//from configuration
    char* input_str = NULL; //irelevant for this test case (unused parameter)
    char result_array[MAX_NUM_OF_INTERFACES][MAX_STR_LEN];
    sys_call_spec_t cmock_to_return;
    base_status_t a;
    
    memset(&result_array, 0, MAX_NUM_OF_INTERFACES*MAX_STR_LEN);//clear result_array, somehow testing framework keeps old ????
    strncpy(result_array[0], "F0", MAX_STR_LEN);//different than match case

    a = array_convert_hex_string_to_long_int(module_index, obj_index, comb_index, parser_index, input_str, 0, result_array, MAX_NUM_OF_INTERFACES, MAX_STR_LEN);
    //check result_array[0]
    if(0 != strncmp("240", result_array[0], MAX_STR_LEN))
    {
        a = BASE_PARSING_FAILED;//direct fail if result is not good
    }//in normal execution we except FAILED, so in previous block logic is changed that tet can fail
    TEST_ASSERT_EQUAL_INT_MESSAGE(BASE_PARSING_SUCCEEDED, a, "===>>> [result_array[0] = \"F0\"] ===>>> SHOULD RETURN: BASE_PARSING_SUCCEEDED, because match case is \"!10.11+\"");
}

//TEST - result_array[0] is empty string
void  test_003_array_convert_hex_string_to_long_int(void)
{
    module_index_t module_index = MSTP;//valid
    int obj_index    = MSTP_BRIDGE_PRIORITY;//valid
    int comb_index   = 4;//from configuraion
    int parser_index = 0;//from configuration
    char* input_str = NULL; //irelevant for this test case (unused parameter)
    char result_array[MAX_NUM_OF_INTERFACES][MAX_STR_LEN];
    sys_call_spec_t cmock_to_return;
    base_status_t a;

    memset(&result_array, 0, MAX_NUM_OF_INTERFACES*MAX_STR_LEN);//clear result_array, somehow testing framework keeps old ????

    a = array_convert_hex_string_to_long_int(module_index, obj_index, comb_index, parser_index, input_str, 0, result_array, MAX_NUM_OF_INTERFACES, MAX_STR_LEN);
    //check result_array[0]
    if(0 != strncmp("", result_array[0], MAX_STR_LEN))
    {
        a = BASE_PARSING_SUCCEEDED;//direct fail if result is not good
    }
    TEST_ASSERT_EQUAL_INT_MESSAGE(BASE_PARSING_FAILED, a, "===>>> [result_array[0] is empty string] ===>>> SHOULD RETURN: BASE_PARSING_FAILED, because result_array[0] is empty string");    
}
////*****END OF TESTING array_convert_hex_string_to_long_int*****////


////*****TESTS for array_shrink_string*****////
//TEST - result_array[0] length is greater than match case
void  test_001_array_shrink_string(void)
{
    module_index_t module_index = MSTP;//valid
    int obj_index    = MSTP_BRIDGE_PRIORITY;//valid
    int comb_index   = 3;//from configuraion
    int parser_index = 0;//from configuration
    char* input_str = NULL; //irelevant for this test case (unused parameter)
    char result_array[MAX_NUM_OF_INTERFACES][MAX_STR_LEN];
    sys_call_spec_t cmock_to_return;
    base_status_t a;

    //sys_call_spec_t need to be set manually
    memset(&cmock_to_return, 0, sizeof(sys_call_spec_t));//clear, somehow testing framework keeps old ????
    cmock_to_return.sys_call = NULL;       //irelevant for this test
    cmock_to_return.sys_call_command = ""; //irelevant for this test
    cmock_to_return.num_of_combinators = 5;//from configuration
    cmock_to_return.combinator = NULL;     //irelevant for this test
    int mstp_num_of_cb_tmp[5] = {0, 1, 1, 1, 1};    //taken from configuration
    memcpy(cmock_to_return.num_of_cb, mstp_num_of_cb_tmp, sizeof(int)*5);
    memcpy(cmock_to_return.parse_cb[comb_index][parser_index].match_case, BASE_FOUR_STR, MAX_STR_LEN);// valid
    //sys_call_spec_t setting is done
    memset(&result_array, 0, MAX_NUM_OF_INTERFACES*MAX_STR_LEN);//clear result_array, somehow testing framework keeps old ????
    strncpy(result_array[0], "abcabc", MAX_STR_LEN);//different than match case

    //mocks
    assemble_sys_call_structure_CMockIgnoreAndReturn(0, cmock_to_return);

    a = array_shrink_string(module_index, obj_index, comb_index, parser_index, input_str, 0, result_array, MAX_NUM_OF_INTERFACES, MAX_STR_LEN);
    //check result_array[0]
    if(0 != strncmp("abca", result_array[0], MAX_STR_LEN))
    {
        a = BASE_PARSING_FAILED;//direct fail if result is not good
    }//in normal execution we except FAILED, so in previous block logic is changed that tet can fail
    TEST_ASSERT_EQUAL_INT_MESSAGE(BASE_PARSING_SUCCEEDED, a, "===>>> [result_array[0] length is greater than match case] ===>>> SHOULD RETURN: BASE_PARSING_SUCCEEDED");
}

//TEST - result_array[0] length is less than match case
void  test_002_array_shrink_string(void)
{
    module_index_t module_index = MSTP;//valid
    int obj_index    = MSTP_BRIDGE_PRIORITY;//valid
    int comb_index   = 3;//from configuraion
    int parser_index = 0;//from configuration
    char* input_str = NULL; //irelevant for this test case (unused parameter)
    char result_array[MAX_NUM_OF_INTERFACES][MAX_STR_LEN];
    sys_call_spec_t cmock_to_return;
    base_status_t a;

    //sys_call_spec_t need to be set manually
    memset(&cmock_to_return, 0, sizeof(sys_call_spec_t));//clear, somehow testing framework keeps old ????
    cmock_to_return.sys_call = NULL;       //irelevant for this test
    cmock_to_return.sys_call_command = ""; //irelevant for this test
    cmock_to_return.num_of_combinators = 5;//from configuration
    cmock_to_return.combinator = NULL;     //irelevant for this test
    int mstp_num_of_cb_tmp[5] = {0, 1, 1, 1, 1};    //taken from configuration
    memcpy(cmock_to_return.num_of_cb, mstp_num_of_cb_tmp, sizeof(int)*5);
    memcpy(cmock_to_return.parse_cb[comb_index][parser_index].match_case, BASE_FOUR_STR, MAX_STR_LEN);// valid
    //sys_call_spec_t setting is done
    memset(&result_array, 0, MAX_NUM_OF_INTERFACES*MAX_STR_LEN);//clear result_array, somehow testing framework keeps old ????
    strncpy(result_array[0], "abc", MAX_STR_LEN);//different than match case
    
    //mocks
    assemble_sys_call_structure_CMockIgnoreAndReturn(0, cmock_to_return);

    a = array_shrink_string(module_index, obj_index, comb_index, parser_index, input_str, 0, result_array, MAX_NUM_OF_INTERFACES, MAX_STR_LEN);
    //check result_array[0]
    if(0 != strncmp("abc", result_array[0], MAX_STR_LEN))
    {
        a = BASE_PARSING_FAILED;//direct fail if result is not good
    }//in normal execution we except FAILED, so in previous block logic is changed that tet can fail
    TEST_ASSERT_EQUAL_INT_MESSAGE(BASE_PARSING_SUCCEEDED, a, "===>>> [result_array[0] length is less than match case] ===>>> SHOULD RETURN: BASE_PARSING_SUCCEEDED");
}

//TEST - assemble_sys_call_structure returns structure filled with 0s 
void  test_003_array_shrink_string(void)
{
    module_index_t module_index = MSTP;//valid
    int obj_index    = MSTP_BRIDGE_PRIORITY;//valid
    int comb_index   = 3;//from configuraion
    int parser_index = 0;//from configuration
    char* input_str = NULL; //irelevant for this test case (unused parameter)
    char result_array[MAX_NUM_OF_INTERFACES][MAX_STR_LEN];
    sys_call_spec_t cmock_to_return;
    base_status_t a;
    
    //sys_call_spec_t need to be set manually
    memset(&cmock_to_return, 0, sizeof(cmock_to_return));
    //sys_call_spec_t setting is done
    memset(&result_array, 0, MAX_NUM_OF_INTERFACES*MAX_STR_LEN);//clear result_array, somehow testing framework keeps old ????
    strncpy(result_array[0], "abcdef", MAX_STR_LEN);//different than match case
    
    //mocks
    assemble_sys_call_structure_CMockIgnoreAndReturn(0, cmock_to_return);

    a = array_shrink_string(module_index, obj_index, comb_index, parser_index, input_str, 0, result_array, MAX_NUM_OF_INTERFACES, MAX_STR_LEN);
    //check result_array[0]
    //NO NEED, part with this manipulation is not REACHED
    TEST_ASSERT_EQUAL_INT_MESSAGE(BASE_PARSING_FAILED, a, "===>>> [assemble_sys_call_structure returns structure filled with 0s] ===>>> SHOULD RETURN: BASE_PARSING_FAILED, because assemble_sys_call_structure returns structure filled with 0s ");    
}

//TEST - match case is "" 
void  test_004_array_shrink_string(void)
{
    module_index_t module_index = MSTP;//valid
    int obj_index    = MSTP_BRIDGE_PRIORITY;//valid
    int comb_index   = 3;//from configuraion
    int parser_index = 0;//from configuration
    char* input_str = NULL; //irelevant for this test case (unused parameter)
    char result_array[MAX_NUM_OF_INTERFACES][MAX_STR_LEN];
    sys_call_spec_t cmock_to_return;
    base_status_t a;

    //sys_call_spec_t need to be set manually
    memset(&cmock_to_return, 0, sizeof(sys_call_spec_t));//clear, somehow testing framework keeps old ????
    cmock_to_return.sys_call = NULL;       //irelevant for this test
    cmock_to_return.sys_call_command = ""; //irelevant for this test
    cmock_to_return.num_of_combinators = 5;//from configuration
    cmock_to_return.combinator = NULL;     //irelevant for this test
    int mstp_num_of_cb_tmp[5] = {0, 1, 1, 1, 1};    //taken from configuration
    memcpy(cmock_to_return.num_of_cb, mstp_num_of_cb_tmp, sizeof(int)*5);
    memcpy(cmock_to_return.parse_cb[comb_index][parser_index].match_case, "", MAX_STR_LEN);// valid
    //sys_call_spec_t setting is done
    memset(&result_array, 0, MAX_NUM_OF_INTERFACES*MAX_STR_LEN);//clear result_array, somehow testing framework keeps old ????
    strncpy(result_array[0], "abcdef", MAX_STR_LEN);//different than match case
    
    //mocks
    assemble_sys_call_structure_CMockIgnoreAndReturn(0, cmock_to_return);

    a = array_shrink_string(module_index, obj_index, comb_index, parser_index, input_str, 0, result_array, MAX_NUM_OF_INTERFACES, MAX_STR_LEN);
    //check result_array[0]
    //NO NEED, part with this manipulation is not REACHED
    TEST_ASSERT_EQUAL_INT_MESSAGE(BASE_PARSING_FAILED, a, "===>>> [match case is \"\"] ===>>> SHOULD RETURN: BASE_PARSING_FAILED, because match case is \"\"");    
}

//TEST - result_array[0] is empty string
void  test_005_array_shrink_string(void)
{
    module_index_t module_index = MSTP;//valid
    int obj_index    = MSTP_BRIDGE_PRIORITY;//valid
    int comb_index   = 3;//from configuraion
    int parser_index = 0;//from configuration
    char* input_str = NULL; //irelevant for this test case (unused parameter)
    char result_array[MAX_NUM_OF_INTERFACES][MAX_STR_LEN];
    sys_call_spec_t cmock_to_return;
    base_status_t a;

    //sys_call_spec_t need to be set manually
    memset(&cmock_to_return, 0, sizeof(sys_call_spec_t));//clear, somehow testing framework keeps old ????
    cmock_to_return.sys_call = NULL;       //irelevant for this test
    cmock_to_return.sys_call_command = ""; //irelevant for this test
    cmock_to_return.num_of_combinators = 5;//from configuration
    cmock_to_return.combinator = NULL;     //irelevant for this test
    int mstp_num_of_cb_tmp[5] = {0, 1, 1, 1, 1};    //taken from configuration
    memcpy(cmock_to_return.num_of_cb, mstp_num_of_cb_tmp, sizeof(int)*5);
    memcpy(cmock_to_return.parse_cb[comb_index][parser_index].match_case, BASE_FOUR_STR, MAX_STR_LEN);// valid
    //sys_call_spec_t setting is done
    memset(&result_array, 0, MAX_NUM_OF_INTERFACES*MAX_STR_LEN);//clear result_array, somehow testing framework keeps old ????
    strncpy(result_array[0], "", MAX_STR_LEN);
    //mocks
    assemble_sys_call_structure_CMockIgnoreAndReturn(0, cmock_to_return);

    a = array_shrink_string(module_index, obj_index, comb_index, parser_index, input_str, 0, result_array, MAX_NUM_OF_INTERFACES, MAX_STR_LEN);
    //check result_array[0]
    if(0 != strncmp("", result_array[0], MAX_STR_LEN))
    {
        a = BASE_PARSING_SUCCEEDED;//direct fail if result is not good
    }
    TEST_ASSERT_EQUAL_INT_MESSAGE(BASE_PARSING_FAILED, a, "===>>> [result_array[0] is empty string] ===>>> SHOULD RETURN: BASE_PARSING_FAILED, because result_array[0] is empty string");    
}
////*****END OF TESTING array_shrink_string*****////


////*****TESTS for array_uniq_elements*****////
//TEST - result_array[0] and result_array[1] contains same string
void  test_001_array_uniq_elements(void)
{
    module_index_t module_index = MSTP;//valid
    int obj_index    = MSTP_BRIDGE_PRIORITY;//valid
    int comb_index   = 4;//from configuraion
    int parser_index = 0;//from configuration
    char* input_str = NULL; //irelevant for this test case (unused parameter)
    char result_array[MAX_NUM_OF_INTERFACES][MAX_STR_LEN];
    sys_call_spec_t cmock_to_return;
    base_status_t a;

    memset(&result_array, 0, MAX_NUM_OF_INTERFACES*MAX_STR_LEN);//clear result_array, somehow testing framework keeps old ????
    strncpy(result_array[0], "test", MAX_STR_LEN);//different than match case
    strncpy(result_array[1], "test", MAX_STR_LEN);//different than match case

    a = array_uniq_elements(module_index, obj_index, comb_index, parser_index, input_str, 0, result_array, MAX_NUM_OF_INTERFACES, MAX_STR_LEN);
    //check result_array[0]
	if((0 != strncmp("test", result_array[0], MAX_STR_LEN)) && (0 != strncmp("", result_array[1], MAX_STR_LEN)))
	{
		a = BASE_PARSING_FAILED;//direct fail if result is not good
	}
    TEST_ASSERT_EQUAL_INT_MESSAGE(BASE_PARSING_SUCCEEDED, a, "===>>> [result_array[0] and result_array[1] contains same string] ===>>> SHOULD RETURN: BASE_PARSING_SUCCEEDED");
}

//TEST - result_array[0]= test, result_array[1] = \"\"
void  test_002_array_uniq_elements(void)
{
    module_index_t module_index = MSTP;//valid
    int obj_index    = MSTP_BRIDGE_PRIORITY;//valid
    int comb_index   = 4;//from configuraion
    int parser_index = 0;//from configuration
    char* input_str = NULL; //irelevant for this test case (unused parameter)
    char result_array[MAX_NUM_OF_INTERFACES][MAX_STR_LEN];
    sys_call_spec_t cmock_to_return;
    base_status_t a;

    memset(&result_array, 0, MAX_NUM_OF_INTERFACES*MAX_STR_LEN);//clear result_array, somehow testing framework keeps old ????
    strncpy(result_array[0], "test", MAX_STR_LEN);//different than match case
    strncpy(result_array[1], "", MAX_STR_LEN);//different than match case

    a = array_uniq_elements(module_index, obj_index, comb_index, parser_index, input_str, 0, result_array, MAX_NUM_OF_INTERFACES, MAX_STR_LEN);
    //check result_array[0]
    if((0 != strncmp("test", result_array[0], MAX_STR_LEN)) && (0 != strncmp("", result_array[1], MAX_STR_LEN)))
    {
        a = BASE_PARSING_FAILED;//direct fail if result is not good
    }//in normal execution we except FAILED, so in previous block logic is changed that tet can fail
    TEST_ASSERT_EQUAL_INT_MESSAGE(BASE_PARSING_SUCCEEDED, a, "===>>> [result_array[0]= test, result_array[1] = \"\"] ===>>> SHOULD RETURN: BASE_PARSING_SUCCEEDED, because array contains unique strings");
}

//TEST - result_array[0]= \"\", result_array[1] = test
void  test_003_array_uniq_elements(void)
{
    module_index_t module_index = MSTP;//valid
    int obj_index    = MSTP_BRIDGE_PRIORITY;//valid
    int comb_index   = 4;//from configuraion
    int parser_index = 0;//from configuration
    char* input_str = NULL; //irelevant for this test case (unused parameter)
    char result_array[MAX_NUM_OF_INTERFACES][MAX_STR_LEN];
    sys_call_spec_t cmock_to_return;
    base_status_t a;

    memset(&result_array, 0, MAX_NUM_OF_INTERFACES*MAX_STR_LEN);//clear result_array, somehow testing framework keeps old ????

    a = array_uniq_elements(module_index, obj_index, comb_index, parser_index, input_str, 0, result_array, MAX_NUM_OF_INTERFACES, MAX_STR_LEN);
    //check result_array[0]
    if((0 != strncmp("", result_array[0], MAX_STR_LEN)) && (0 != strncmp("test", result_array[1], MAX_STR_LEN)))
    {
        a = BASE_PARSING_FAILED;//in this case result_array is cleared
    }
    TEST_ASSERT_EQUAL_INT_MESSAGE(BASE_PARSING_SUCCEEDED, a, "===>>> [result_array[0]= \"\", result_array[1] = test] ===>>> SHOULD RETURN: BASE_PARSING_SUCCEEDED, because in this case result_array is cleared");
}
////*****END OF TESTING array_convert_hex_string_to_long_int*****////

////*****TESTS for array_count_elements*****////
//TEST - result_array[0] = test1, result_array[1] = test2
void  test_001_array_count_elements(void)
{
    module_index_t module_index = MSTP;//valid
    int obj_index    = MSTP_BRIDGE_PRIORITY;//valid
    int comb_index   = 4;//from configuraion
    int parser_index = 0;//from configuration
    char* input_str = NULL; //irelevant for this test case (unused parameter)
    char result_array[MAX_NUM_OF_INTERFACES][MAX_STR_LEN];
    sys_call_spec_t cmock_to_return;
    base_status_t a;

    memset(&result_array, 0, MAX_NUM_OF_INTERFACES*MAX_STR_LEN);//clear result_array, somehow testing framework keeps old ????
    strncpy(result_array[0], "test1", MAX_STR_LEN);//different than match case
    strncpy(result_array[1], "test2", MAX_STR_LEN);//different than match case

    a = array_count_elements(module_index, obj_index, comb_index, parser_index, input_str, 0, result_array, MAX_NUM_OF_INTERFACES, MAX_STR_LEN);
    //check result_array[0]
	if(0 != strncmp("2", result_array[0], MAX_STR_LEN))
	{
		a = BASE_PARSING_FAILED;//direct fail if result is not good
	}
    TEST_ASSERT_EQUAL_INT_MESSAGE(BASE_PARSING_SUCCEEDED, a, "===>>> [result_array[0] = test1, result_array[1] = test2] ===>>> SHOULD RETURN: BASE_PARSING_SUCCEEDED");
}

//TEST - result_array[0]= "\"\", result_array[1] = \"\"
void  test_002_array_count_elements(void)
{
    module_index_t module_index = MSTP;//valid
    int obj_index    = MSTP_BRIDGE_PRIORITY;//valid
    int comb_index   = 4;//from configuraion
    int parser_index = 0;//from configuration
    char* input_str = NULL; //irelevant for this test case (unused parameter)
    char result_array[MAX_NUM_OF_INTERFACES][MAX_STR_LEN];
    sys_call_spec_t cmock_to_return;
    base_status_t a;

    memset(&result_array, 0, MAX_NUM_OF_INTERFACES*MAX_STR_LEN);//clear result_array, somehow testing framework keeps old ????
    strncpy(result_array[0], "", MAX_STR_LEN);//different than match case
    strncpy(result_array[1], "", MAX_STR_LEN);//different than match case

    a = array_count_elements(module_index, obj_index, comb_index, parser_index, input_str, 0, result_array, MAX_NUM_OF_INTERFACES, MAX_STR_LEN);
    //check result_array[0]
    if(0 != strncmp("0", result_array[0], MAX_STR_LEN))
    {
        a = BASE_PARSING_FAILED;//direct fail if result is not good
    }//in normal execution we except FAILED, so in previous block logic is changed that tet can fail
    TEST_ASSERT_EQUAL_INT_MESSAGE(BASE_PARSING_SUCCEEDED, a, "===>>> [result_array[0]= \"\", result_array[1] = \"\"] ===>>> SHOULD RETURN: BASE_PARSING_SUCCEEDED");
}

//TEST - result_array[0]= \"\", result_array[1] = test
void  test_003_array_count_elements(void)
{
    module_index_t module_index = MSTP;//valid
    int obj_index    = MSTP_BRIDGE_PRIORITY;//valid
    int comb_index   = 4;//from configuraion
    int parser_index = 0;//from configuration
    char* input_str = NULL; //irelevant for this test case (unused parameter)
    char result_array[MAX_NUM_OF_INTERFACES][MAX_STR_LEN];
    sys_call_spec_t cmock_to_return;
    base_status_t a;

    memset(&result_array, 0, MAX_NUM_OF_INTERFACES*MAX_STR_LEN);//clear result_array, somehow testing framework keeps old ????

    a = array_count_elements(module_index, obj_index, comb_index, parser_index, input_str, 0, result_array, MAX_NUM_OF_INTERFACES, MAX_STR_LEN);
    //check result_array[0]
    if(0 != strncmp("0", result_array[0], MAX_STR_LEN))
    {
        a = BASE_PARSING_FAILED;//in this case result_array is cleared
    }
    TEST_ASSERT_EQUAL_INT_MESSAGE(BASE_PARSING_SUCCEEDED, a, "===>>> [result_array[0]= \"\", result_array[1] = test] ===>>> SHOULD RETURN: BASE_PARSING_SUCCEEDED, because in this case result_array is cleared");
}
////*****END OF TESTING array_convert_hex_string_to_long_int*****////


////*****TESTS for array_until_match_case*****////
//TEST - result_array[0] contains match case
void  test_001_array_until_match_case(void)
{
    module_index_t module_index = Q_BRIDGE;//valid
    int obj_index    = Q_BRIDGE_NAMES;//valid
    int comb_index   = 2;//from configuraion
    int parser_index = 0;//from configuration
    char* input_str = NULL; //irelevant for this test case (unused parameter)
    char result_array[MAX_NUM_OF_INTERFACES][MAX_STR_LEN];
    sys_call_spec_t cmock_to_return;
    base_status_t a;

    //sys_call_spec_t need to be set manually
    memset(&cmock_to_return, 0, sizeof(sys_call_spec_t));//clear, somehow testing framework keeps old ????
    cmock_to_return.sys_call = NULL;       //irelevant for this test
    cmock_to_return.sys_call_command = ""; //irelevant for this test
    cmock_to_return.num_of_combinators = 5;//from configuration
    cmock_to_return.combinator = NULL;     //irelevant for this test
    int mstp_num_of_cb_tmp[5] = {0, 1, 1, 1, 1};    //taken from configuration
    memcpy(cmock_to_return.num_of_cb, mstp_num_of_cb_tmp, sizeof(int)*5);
    memcpy(cmock_to_return.parse_cb[comb_index][parser_index].match_case, COLON_STR, MAX_STR_LEN);// valid
    //sys_call_spec_t setting is done
    memset(&result_array, 0, MAX_NUM_OF_INTERFACES*MAX_STR_LEN);//clear result_array, somehow testing framework keeps old ????
    strncpy(result_array[0], "abc:abc", MAX_STR_LEN);//different than match case

    //mocks
    assemble_sys_call_structure_CMockIgnoreAndReturn(0, cmock_to_return);

    a = array_until_match_case(module_index, obj_index, comb_index, parser_index, input_str, 0, result_array, MAX_NUM_OF_INTERFACES, MAX_STR_LEN);
    //check result_array[0]
    if(0 != strncmp("abc", result_array[0], MAX_STR_LEN))
    {
        a = BASE_PARSING_FAILED;//direct fail if result is not good
    }//in normal execution we except FAILED, so in previous block logic is changed that tet can fail
    TEST_ASSERT_EQUAL_INT_MESSAGE(BASE_PARSING_SUCCEEDED, a, "===>>> [result_array[0] = abc:abc, match_case = , ] ===>>> SHOULD RETURN: BASE_PARSING_SUCCEEDED, because result_array[0] contains match case");
}

//TEST - result_array[0] do not contains match case
void  test_002_array_until_match_case(void)
{
    module_index_t module_index = Q_BRIDGE;//valid
    int obj_index    = Q_BRIDGE_NAMES;//valid
    int comb_index   = 2;//from configuraion
    int parser_index = 0;//from configuration
    char* input_str = NULL; //irelevant for this test case (unused parameter)
    char result_array[MAX_NUM_OF_INTERFACES][MAX_STR_LEN];
    sys_call_spec_t cmock_to_return;
    base_status_t a;

    //sys_call_spec_t need to be set manually
    memset(&cmock_to_return, 0, sizeof(sys_call_spec_t));//clear, somehow testing framework keeps old ????
    cmock_to_return.sys_call = NULL;       //irelevant for this test
    cmock_to_return.sys_call_command = ""; //irelevant for this test
    cmock_to_return.num_of_combinators = 5;//from configuration
    cmock_to_return.combinator = NULL;     //irelevant for this test
    int mstp_num_of_cb_tmp[5] = {0, 1, 1, 1, 1};    //taken from configuration
    memcpy(cmock_to_return.num_of_cb, mstp_num_of_cb_tmp, sizeof(int)*5);
    memcpy(cmock_to_return.parse_cb[comb_index][parser_index].match_case, COLON_STR, MAX_STR_LEN);// valid
    //sys_call_spec_t setting is done
    memset(&result_array, 0, MAX_NUM_OF_INTERFACES*MAX_STR_LEN);//clear result_array, somehow testing framework keeps old ????
    strncpy(result_array[0], "abc", MAX_STR_LEN);//different than match case

    //mocks
    assemble_sys_call_structure_CMockIgnoreAndReturn(0, cmock_to_return);

    a = array_until_match_case(module_index, obj_index, comb_index, parser_index, input_str, 0, result_array, MAX_NUM_OF_INTERFACES, MAX_STR_LEN);
    //check result_array[0]
    if(0 != strncmp("abc", result_array[0], MAX_STR_LEN))
    {
        a = BASE_PARSING_FAILED;//direct fail if result is not good
    }//in normal execution we except FAILED, so in previous block logic is changed that tet can fail
    TEST_ASSERT_EQUAL_INT_MESSAGE(BASE_PARSING_SUCCEEDED, a, "===>>> [result_array[0] length is less than match case] ===>>> SHOULD RETURN: BASE_PARSING_SUCCEEDED");
}

//TEST - assemble_sys_call_structure returns structure filled with 0s
void  test_003_array_until_match_case(void)
{
    module_index_t module_index = Q_BRIDGE;//valid
    int obj_index    = Q_BRIDGE_NAMES;//valid
    int comb_index   = 2;//from configuraion
    int parser_index = 0;//from configuration
    char* input_str = NULL; //irelevant for this test case (unused parameter)
    char result_array[MAX_NUM_OF_INTERFACES][MAX_STR_LEN];
    sys_call_spec_t cmock_to_return;
    base_status_t a;

    //sys_call_spec_t need to be set manually
    memset(&cmock_to_return, 0, sizeof(cmock_to_return));
    //sys_call_spec_t setting is done
    memset(&result_array, 0, MAX_NUM_OF_INTERFACES*MAX_STR_LEN);//clear result_array, somehow testing framework keeps old ????
    strncpy(result_array[0], "abcdef", MAX_STR_LEN);//different than match case

    //mocks
    assemble_sys_call_structure_CMockIgnoreAndReturn(0, cmock_to_return);

    a = array_until_match_case(module_index, obj_index, comb_index, parser_index, input_str, 0, result_array, MAX_NUM_OF_INTERFACES, MAX_STR_LEN);
    //check result_array[0]
    //NO NEED, part with this manipulation is not REACHED
    TEST_ASSERT_EQUAL_INT_MESSAGE(BASE_PARSING_FAILED, a, "===>>> [assemble_sys_call_structure returns structure filled with 0s] ===>>> SHOULD RETURN: BASE_PARSING_FAILED, because assemble_sys_call_structure returns structure filled with 0s ");
}

//TEST - match case is ""
void  test_004_array_until_match_case(void)
{
    module_index_t module_index = Q_BRIDGE;//valid
    int obj_index    = Q_BRIDGE_NAMES;//valid
    int comb_index   = 2;//from configuraion
    int parser_index = 0;//from configuration
    char* input_str = NULL; //irelevant for this test case (unused parameter)
    char result_array[MAX_NUM_OF_INTERFACES][MAX_STR_LEN];
    sys_call_spec_t cmock_to_return;
    base_status_t a;

    //sys_call_spec_t need to be set manually
    memset(&cmock_to_return, 0, sizeof(sys_call_spec_t));//clear, somehow testing framework keeps old ????
    cmock_to_return.sys_call = NULL;       //irelevant for this test
    cmock_to_return.sys_call_command = ""; //irelevant for this test
    cmock_to_return.num_of_combinators = 5;//from configuration
    cmock_to_return.combinator = NULL;     //irelevant for this test
    int mstp_num_of_cb_tmp[5] = {0, 1, 1, 1, 1};    //taken from configuration
    memcpy(cmock_to_return.num_of_cb, mstp_num_of_cb_tmp, sizeof(int)*5);
    memcpy(cmock_to_return.parse_cb[comb_index][parser_index].match_case, "", MAX_STR_LEN);// valid
    //sys_call_spec_t setting is done
    memset(&result_array, 0, MAX_NUM_OF_INTERFACES*MAX_STR_LEN);//clear result_array, somehow testing framework keeps old ????
    strncpy(result_array[0], "abcdef", MAX_STR_LEN);//different than match case

    //mocks
    assemble_sys_call_structure_CMockIgnoreAndReturn(0, cmock_to_return);

    a = array_until_match_case(module_index, obj_index, comb_index, parser_index, input_str, 0, result_array, MAX_NUM_OF_INTERFACES, MAX_STR_LEN);
    //check result_array[0]
    //NO NEED, part with this manipulation is not REACHED
    TEST_ASSERT_EQUAL_INT_MESSAGE(BASE_PARSING_FAILED, a, "===>>> [match case is \"\"] ===>>> SHOULD RETURN: BASE_PARSING_FAILED, because match case is \"\"");
}

//TEST - result_array[0] is empty string
void  test_005_array_until_match_case(void)
{
    module_index_t module_index = Q_BRIDGE;//valid
    int obj_index    = Q_BRIDGE_NAMES;//valid
    int comb_index   = 2;//from configuraion
    int parser_index = 0;//from configuration
    char* input_str = NULL; //irelevant for this test case (unused parameter)
    char result_array[MAX_NUM_OF_INTERFACES][MAX_STR_LEN];
    sys_call_spec_t cmock_to_return;
    base_status_t a;

    //sys_call_spec_t need to be set manually
    memset(&cmock_to_return, 0, sizeof(sys_call_spec_t));//clear, somehow testing framework keeps old ????
    cmock_to_return.sys_call = NULL;       //irelevant for this test
    cmock_to_return.sys_call_command = ""; //irelevant for this test
    cmock_to_return.num_of_combinators = 5;//from configuration
    cmock_to_return.combinator = NULL;     //irelevant for this test
    int mstp_num_of_cb_tmp[5] = {0, 1, 1, 1, 1};    //taken from configuration
    memcpy(cmock_to_return.num_of_cb, mstp_num_of_cb_tmp, sizeof(int)*5);
    memcpy(cmock_to_return.parse_cb[comb_index][parser_index].match_case, COLON_STR, MAX_STR_LEN);// valid
    //sys_call_spec_t setting is done
    memset(&result_array, 0, MAX_NUM_OF_INTERFACES*MAX_STR_LEN);//clear result_array, somehow testing framework keeps old ????
    strncpy(result_array[0], "", MAX_STR_LEN);
    //mocks
    assemble_sys_call_structure_CMockIgnoreAndReturn(0, cmock_to_return);

    a = array_until_match_case(module_index, obj_index, comb_index, parser_index, input_str, 0, result_array, MAX_NUM_OF_INTERFACES, MAX_STR_LEN);
    //check result_array[0]
    if(0 != strncmp("", result_array[0], MAX_STR_LEN))
    {
        a = BASE_PARSING_FAILED;//direct fail if result is not good
    }
    TEST_ASSERT_EQUAL_INT_MESSAGE(BASE_PARSING_SUCCEEDED, a, "===>>> [result_array[0] is empty string] ===>>> SHOULD RETURN: BASE_PARSING_SUCCEEDED, because empty result_array is allowed");
}
////*****END OF TESTING array_separate_with*****////

void tearDown(void)
{
}
