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
#include "base_private_validators.h"

#include "mock_base_prints.h"
#include "mock_base_private_syscalls.h"
#include "mock_base_linux_tool_helpers.h"

module_index_t module_index;
int obj_index;
int comb_index;
int parser_index;
char* input_str;
char result_array[MAX_NUM_OF_INTERFACES][MAX_STR_LEN];

void setUp(void)
{
    base_print_error_CMockIgnore();
}

////*****TESTS for validate_uint_32_number*****////
// valid number
void  test_001_validate_uint_32_number (void) {	
	strncpy(result_array[0], "1", MAX_STR_LEN);
	base_status_t a = validate_uint_32_number(module_index, obj_index, comb_index, parser_index, input_str, 0, result_array, MAX_NUM_OF_INTERFACES, MAX_STR_LEN);
	TEST_ASSERT_EQUAL_INT_MESSAGE(BASE_VALIDATION_SUCCEEDED, a, "===>>> [result_array[0]]= \"1\" ===>>> SHOULD RETURN: BASE_VALIDATION_SUCCEEDED,  because input is number");	
}

// empty string
void test_002_validate_uint_32_number(void) {
	//TEST ON SIMPLE CHARACTER
	strncpy(result_array[0], "", MAX_STR_LEN);
	base_status_t a = validate_uint_32_number(module_index, obj_index, comb_index, parser_index, input_str, 0, result_array, MAX_NUM_OF_INTERFACES, MAX_STR_LEN);
	TEST_ASSERT_EQUAL_INT_MESSAGE(BASE_VALIDATION_FAILED, a, "===>>> [result_array[0]]=\"\" ===>>> SHOULD RETURN: BASE_VALIDATION_FAILED because there is nothing in input");
}

//TEST - to big value for uint 32
void test_003_validate_uint_32_number(void) {
	strncpy(result_array[0], "42949672961", MAX_STR_LEN);
	base_status_t a = validate_uint_32_number(module_index, obj_index, comb_index, parser_index, input_str, 0, result_array, MAX_NUM_OF_INTERFACES, MAX_STR_LEN);
	TEST_ASSERT_EQUAL_INT_MESSAGE(BASE_VALIDATION_FAILED, a, "===>>> [result_array[0]]=\"42949672961\" ===>>>  SHOULD RETURN: BASE_VALIDATION_FAILED because string is long more than 10 characters");
}

//TEST - negative value 
void test_004_validate_uint_32_number(void) {
	strncpy(result_array[0], "-429", MAX_STR_LEN);
	base_status_t a = validate_uint_32_number(module_index, obj_index, comb_index, parser_index, input_str, 0, result_array, MAX_NUM_OF_INTERFACES, MAX_STR_LEN);
	TEST_ASSERT_EQUAL_INT_MESSAGE(BASE_VALIDATION_FAILED, a, "===>>> [result_array[0]]=\"-429\" ===>>>  SHOULD RETURN: BASE_VALIDATION_FAILED because string contains negativ value");
}

//TEST - invalid parameters which are not used in the function 
void test_005_validate_uint_32_number(void) {
	module_index = 10000;//invalid
	obj_index    = 10000;//invalid
	comb_index   = 10000;//invalid
	parser_index = 10000;//invalid
	input_str    = NULL; //invalid
	strncpy(result_array[0], "1", MAX_STR_LEN);//valid
	base_status_t a = validate_uint_32_number(module_index, obj_index, comb_index, parser_index, input_str, 0, result_array, MAX_NUM_OF_INTERFACES, MAX_STR_LEN);
	TEST_ASSERT_EQUAL_INT_MESSAGE(BASE_VALIDATION_SUCCEEDED, a, "===>>> [all unused parameters are invalid] ===>>>  SHOULD RETURN: BASE_VALIDATION_SUCCEEDED because all parameters which are not used in the function are invalid - parameters are not used, so theoretically they can be invalid");
}

//TEST - punction in string 
void test_007_validate_uint_32_number(void) {
	strncpy(result_array[0], "1!2", MAX_STR_LEN);
	base_status_t a = validate_uint_32_number(module_index, obj_index, comb_index, parser_index, input_str, 0, result_array, MAX_NUM_OF_INTERFACES, MAX_STR_LEN);
	TEST_ASSERT_EQUAL_INT_MESSAGE(BASE_VALIDATION_FAILED, a, "===>>> [result_array[0]]=\"1!2\" ===>>>  SHOULD RETURN: BASE_VALIDATION_FAILED because string contains punction");
}

//TEST - letter in string
void test_008_validate_uint_32_number(void) {
	strncpy(result_array[0], "1a2", MAX_STR_LEN);
	base_status_t a = validate_uint_32_number(module_index, obj_index, comb_index, parser_index, input_str, 0, result_array, MAX_NUM_OF_INTERFACES, MAX_STR_LEN);
	TEST_ASSERT_EQUAL_INT_MESSAGE(BASE_VALIDATION_FAILED, a, "===>>> [result_array[0]]=\"1a2\" ===>>>  SHOULD RETURN: BASE_VALIDATION_FAILED because string contains letter");
}
////*****END OF TESTING validate_uint_32_number*****////

////*****TESTS for validate_string_content*****////
//TEST - valid parameters
void test_001_validate_string_content(void) {
	module_index = MSTP;//valid
	obj_index    = MSTP_CIST_PORT_TOP_CHANGE_ACK;//valid
	comb_index   = 2;//valid, 2 is from configuraion
	parser_index = 0;//valid, 0 is from configuration
	input_str    = NULL; //invalid, unused
	//sys_call_spec_t need to be set manually
	sys_call_spec_t cmock_to_return;
	cmock_to_return.sys_call = NULL;//irelevant for this test
	cmock_to_return.sys_call_command = "";//irelevant for this test
	cmock_to_return.num_of_combinators = 4;//from configuration
	cmock_to_return.combinator = NULL;//irelevant for this test
    int mstp_num_of_cb_tmp[4] = {0, 1, 1, 1};//taken from configuration
	memcpy(cmock_to_return.num_of_cb, mstp_num_of_cb_tmp, sizeof(int)*4);
    memcpy(cmock_to_return.parse_cb[comb_index][parser_index].match_case, BASE_YES, MAX_STR_LEN);
        //sys_call_spec_t setting is done
	strncpy(result_array[0], BASE_YES, MAX_STR_LEN);//valid

	//mocks
	assemble_sys_call_structure_CMockIgnoreAndReturn(0, cmock_to_return);
	are_object_indices_valid_CMockIgnoreAndReturn(0, 1);//this means that indices are valid
	
	base_status_t a = validate_string_content(module_index, obj_index, comb_index, parser_index, input_str, 0, result_array, MAX_NUM_OF_INTERFACES, MAX_STR_LEN);
	TEST_ASSERT_EQUAL_INT_MESSAGE(BASE_VALIDATION_SUCCEEDED, a, "===>>> [all parameters are valid] ===>>>  SHOULD RETURN: BASE_VALIDATION_SUCCEEDED because parameters are taken from configuration");
}

//TEST - invalid string
void test_002_validate_string_content(void) {
	module_index = MSTP;//valid
	obj_index    = MSTP_CIST_PORT_TOP_CHANGE_ACK;//valid
	comb_index   = 2;//valid, 2 is from configuraion
	parser_index = 0;//valid, 0 is from configuration
	input_str    = NULL; //invalid, unused
        //sys_call_spec_t need to be set manually
        sys_call_spec_t cmock_to_return;
	cmock_to_return.sys_call = NULL;//irelevant for this test
	cmock_to_return.sys_call_command = "";//irelevant for this test
	cmock_to_return.num_of_combinators = 4;//from configuration
	cmock_to_return.combinator = NULL;//irelevant for this test
    	int mstp_num_of_cb_tmp[4] = {0, 1, 1, 1};//taken from configuration
	memcpy(cmock_to_return.num_of_cb, mstp_num_of_cb_tmp, sizeof(int)*4);
	parser_spec_t mstp_parsing_cb_tmp[4][2]={//taken from configuration
		[0] = {{NULL, ""}},
	    	[1] = {{NULL, ""}},
		[2] = {{NULL, BASE_YES}},//"yes"
		[3] = {{NULL, BASE_NO}}
	};
	memcpy(cmock_to_return.parse_cb, mstp_parsing_cb_tmp, sizeof(parser_spec_t)*4*2);
        //sys_call_spec_t setting is done
	strncpy(result_array[0], "y", MAX_STR_LEN);//invalid (eecepted valid is BASE_YES)

	//mocks
	assemble_sys_call_structure_CMockIgnoreAndReturn(0, cmock_to_return);
	are_object_indices_valid_CMockIgnoreAndReturn(0, 1);//this means that indices are valid
	
	base_status_t a = validate_string_content(module_index, obj_index, comb_index, parser_index, input_str, 0, result_array, MAX_NUM_OF_INTERFACES, MAX_STR_LEN);
	TEST_ASSERT_EQUAL_INT_MESSAGE(BASE_VALIDATION_FAILED, a, "===>>> [result_array[0]]=\"y\" [match case]=\"yes\" ===>>>  SHOULD RETURN: BASE_VALIDATION_FAILED because input string is different that match case");
}

//TEST - valid string in mach case length 
void test_003_validate_string_content(void) {
	module_index = MSTP;//valid
	obj_index    = MSTP_CIST_PORT_TOP_CHANGE_ACK;//valid
	comb_index   = 2;//valid, 2 is from configuraion
	parser_index = 0;//valid, 0 is from configuration
	input_str    = NULL; //invalid, unused
        //sys_call_spec_t need to be set manually
        sys_call_spec_t cmock_to_return;
	cmock_to_return.sys_call = NULL;//irelevant for this test
	cmock_to_return.sys_call_command = "";//irelevant for this test
	cmock_to_return.num_of_combinators = 4;//from configuration
	cmock_to_return.combinator = NULL;//irelevant for this test
    	int mstp_num_of_cb_tmp[4] = {0, 1, 1, 1};//taken from configuration
	memcpy(cmock_to_return.num_of_cb, mstp_num_of_cb_tmp, sizeof(int)*4);
	parser_spec_t mstp_parsing_cb_tmp[4][2]={//taken from configuration
		[0] = {{NULL, ""}},
	    	[1] = {{NULL, ""}},
		[2] = {{NULL, BASE_YES}},
		[3] = {{NULL, BASE_NO}}
	};
	memcpy(cmock_to_return.parse_cb, mstp_parsing_cb_tmp, sizeof(parser_spec_t)*4*2);
        //sys_call_spec_t setting is done
	strncpy(result_array[0], "yes1", MAX_STR_LEN);//invalid (eecepted valid is BASE_YES)

	//mocks
	assemble_sys_call_structure_CMockIgnoreAndReturn(0, cmock_to_return);
	are_object_indices_valid_CMockIgnoreAndReturn(0, 1);//this means that indices are valid

	base_status_t a = validate_string_content(module_index, obj_index, comb_index, parser_index, input_str, 0, result_array, MAX_NUM_OF_INTERFACES, MAX_STR_LEN);
	TEST_ASSERT_EQUAL_INT_MESSAGE(BASE_VALIDATION_FAILED, a, "===>>> [result_array[0]]=\"yes1\" [match case]=\"yes\" ===>>>  SHOULD RETURN: BASE_VALIDATION_FAILED because input string is valid in mach case length ");
}

//TEST - valid string in mach case length 
void test_004_validate_string_content(void) {
	module_index = MSTP;//valid
	obj_index    = MSTP_CIST_PORT_TOP_CHANGE_ACK;//valid
	comb_index   = 2;//valid, 2 is from configuraion
	parser_index = 0;//valid, 0 is from configuration
	input_str    = NULL; //invalid, unused
        //sys_call_spec_t need to be set manually
        sys_call_spec_t cmock_to_return;
	cmock_to_return.sys_call = NULL;//irelevant for this test
	cmock_to_return.sys_call_command = "";//irelevant for this test
	cmock_to_return.num_of_combinators = 4;//from configuration
	cmock_to_return.combinator = NULL;//irelevant for this test
    	int mstp_num_of_cb_tmp[4] = {0, 1, 1, 1};//taken from configuration
	memcpy(cmock_to_return.num_of_cb, mstp_num_of_cb_tmp, sizeof(int)*4);
	parser_spec_t mstp_parsing_cb_tmp[4][2]={//taken from configuration
		[0] = {{NULL, ""}},
	    	[1] = {{NULL, ""}},
		[2] = {{NULL, BASE_YES}},
		[3] = {{NULL, BASE_NO}}
	};
	memcpy(cmock_to_return.parse_cb, mstp_parsing_cb_tmp, sizeof(parser_spec_t)*4*2);
        //sys_call_spec_t setting is done
	strncpy(result_array[0], "yes ", MAX_STR_LEN);//invalid (eecepted valid is BASE_YES)

	//mocks
	assemble_sys_call_structure_CMockIgnoreAndReturn(0, cmock_to_return);
	are_object_indices_valid_CMockIgnoreAndReturn(0, 1);//this means that indices are valid

	base_status_t a = validate_string_content(module_index, obj_index, comb_index, parser_index, input_str, 0, result_array, MAX_NUM_OF_INTERFACES, MAX_STR_LEN);
	TEST_ASSERT_EQUAL_INT_MESSAGE(BASE_VALIDATION_FAILED, a, "===>>> [result_array[0]]=\"yes \" [match case]=\"yes\"  ===>>>  SHOULD RETURN: BASE_VALIDATION_FAILED because input string is valid in mach case length ");
}

//TEST - invalid match case 
void test_005_validate_string_content(void) {
	module_index = MSTP;//valid
	obj_index    = MSTP_CIST_PORT_TOP_CHANGE_ACK;//valid
	comb_index   = 2;//valid, 2 is from configuraion
	parser_index = 0;//valid, 0 is from configuration
	input_str    = NULL; //invalid, unused
    //sys_call_spec_t need to be set manually
    sys_call_spec_t cmock_to_return;
	cmock_to_return.sys_call = NULL;//irelevant for this test
	cmock_to_return.sys_call_command = "";//irelevant for this test
	cmock_to_return.num_of_combinators = 4;//from configuration
	cmock_to_return.combinator = NULL;//irelevant for this test
    int mstp_num_of_cb_tmp[4] = {0, 1, 1, 1};//taken from configuration
	memcpy(cmock_to_return.num_of_cb, mstp_num_of_cb_tmp, sizeof(int)*4);
    memcpy(cmock_to_return.parse_cb[comb_index][parser_index].match_case, "", MAX_STR_LEN);
    //sys_call_spec_t setting is done
	strncpy(result_array[0], "yes", MAX_STR_LEN);//invalid (eecepted valid is BASE_YES)

	//mocks
	assemble_sys_call_structure_CMockIgnoreAndReturn(0, cmock_to_return);
	are_object_indices_valid_CMockIgnoreAndReturn(0, 1);//this means that indices are valid

	base_status_t a = validate_string_content(module_index, obj_index, comb_index, parser_index, input_str, 0, result_array, MAX_NUM_OF_INTERFACES, MAX_STR_LEN);
	TEST_ASSERT_EQUAL_INT_MESSAGE(BASE_VALIDATION_FAILED, a, "===>>> [result_array[0]]=\"yes\" [match case]=\"\"  ===>>>  SHOULD RETURN: BASE_VALIDATION_FAILED because match case is invalid");
}

//TEST - invalid string
void test_006_validate_string_content(void) {
	module_index = MSTP;//valid
	obj_index    = MSTP_CIST_PORT_TOP_CHANGE_ACK;//valid
	comb_index   = 2;//valid, 2 is from configuraion
	parser_index = 0;//valid, 0 is from configuration
	input_str    = NULL; //invalid, unused
    //sys_call_spec_t need to be set manually
    sys_call_spec_t cmock_to_return;
	cmock_to_return.sys_call = NULL;//irelevant for this test
	cmock_to_return.sys_call_command = "";//irelevant for this test
	cmock_to_return.num_of_combinators = 4;//from configuration
	cmock_to_return.combinator = NULL;//irelevant for this test
    int mstp_num_of_cb_tmp[4] = {0, 1, 1, 1};//taken from configuration
	memcpy(cmock_to_return.num_of_cb, mstp_num_of_cb_tmp, sizeof(int)*4);
    memcpy(cmock_to_return.parse_cb[comb_index][parser_index].match_case, BASE_YES, MAX_STR_LEN);
        //sys_call_spec_t setting is done
	result_array[0][0] = '\0';//invalid

	//mocks
	assemble_sys_call_structure_CMockIgnoreAndReturn(0, cmock_to_return);
	are_object_indices_valid_CMockIgnoreAndReturn(0, 1);//this means that indices are valid
	
	base_status_t a = validate_string_content(module_index, obj_index, comb_index, parser_index, input_str, 0, result_array, MAX_NUM_OF_INTERFACES, MAX_STR_LEN);
	TEST_ASSERT_EQUAL_INT_MESSAGE(BASE_VALIDATION_FAILED, a, "===>>> [result_array[0][0]]=\'\\0\' [match case]=\"yes\" ===>>>  SHOULD RETURN: BASE_VALIDATION_FAILED because input string starts with '\0'.");
}
////*****END OF TESTING validate_string_content*****////

////*****TESTS for array_validate_string_length*****////
//TEST - valid parameters
void test_001_array_validate_string_length(void) {
	module_index = MSTP;//valid
	obj_index    = MSTP_CONF_ID_CONF_NAME;//valid
	comb_index   = 1;//valid, 1 is from configuraion
	parser_index = 1;//valid, 0 is from configuration
	input_str    = NULL; //invalid, unused
    //sys_call_spec_t need to be set manually
    sys_call_spec_t cmock_to_return;
	cmock_to_return.sys_call = NULL;//irelevant for this test
	cmock_to_return.sys_call_command = "";//irelevant for this test
	cmock_to_return.num_of_combinators = 2;//from configuration
	cmock_to_return.combinator = NULL;//irelevant for this test
    int mstp_num_of_cb_tmp[2] = {0, 2};//taken from configuration
	memcpy(cmock_to_return.num_of_cb, mstp_num_of_cb_tmp, sizeof(int)*2);
    memcpy(cmock_to_return.parse_cb[comb_index][parser_index].match_case, "3", MAX_STR_LEN);
    //sys_call_spec_t setting is done
	strncpy(result_array[0], "yes", MAX_STR_LEN);//valid

	//mocks
	assemble_sys_call_structure_CMockIgnoreAndReturn(0, cmock_to_return);
	are_object_indices_valid_CMockIgnoreAndReturn(0, 1);//this means that indices are valid
	
	base_status_t a = array_validate_string_length(module_index, obj_index, comb_index, parser_index, input_str, 0, result_array, MAX_NUM_OF_INTERFACES, MAX_STR_LEN);
	TEST_ASSERT_EQUAL_INT_MESSAGE(BASE_VALIDATION_SUCCEEDED, a, "===>>> [result_array[0]]=\"yes\" [match case]=\"3\"  ===>>>  SHOULD RETURN: BASE_VALIDATION_SUCCEEDED because parameters are taken from configuration");
}

//TEST - match case is invalid
void test_002_array_validate_string_length(void) {
	module_index = MSTP;//valid
	obj_index    = MSTP_CONF_ID_CONF_NAME;//valid
	comb_index   = 1;//valid, 1 is from configuraion
	parser_index = 1;//valid, 0 is from configuration
	input_str    = NULL; //invalid, unused
        //sys_call_spec_t need to be set manually
        sys_call_spec_t cmock_to_return;
	cmock_to_return.sys_call = NULL;//irelevant for this test
	cmock_to_return.sys_call_command = "";//irelevant for this test
	cmock_to_return.num_of_combinators = 2;//from configuration
	cmock_to_return.combinator = NULL;//irelevant for this test
    	int mstp_num_of_cb_tmp[2] = {0, 2};//taken from configuration
	memcpy(cmock_to_return.num_of_cb, mstp_num_of_cb_tmp, sizeof(int)*2);
    memcpy(cmock_to_return.parse_cb[comb_index][parser_index].match_case, "2", MAX_STR_LEN);
        //sys_call_spec_t setting is done
	strncpy(result_array[0], "yes", MAX_STR_LEN);//valid

	//mocks
	assemble_sys_call_structure_CMockIgnoreAndReturn(0, cmock_to_return);
	are_object_indices_valid_CMockIgnoreAndReturn(0, 1);//this means that indices are valid
	
	base_status_t a = array_validate_string_length(module_index, obj_index, comb_index, parser_index, input_str, 0, result_array, MAX_NUM_OF_INTERFACES, MAX_STR_LEN);
	TEST_ASSERT_EQUAL_INT_MESSAGE(BASE_VALIDATION_FAILED, a, "===>>> [result_array[0]]=\"yes\" [match case]=\"2\"  ===>>>  SHOULD RETURN: BASE_VALIDATION_FAILED because match case is invalid");
}

//TEST - empty string and 0 as length
void test_003_array_validate_string_length(void) {
	module_index = MSTP;//valid
	obj_index    = MSTP_CONF_ID_CONF_NAME;//valid
	comb_index   = 1;//valid, 1 is from configuraion
	parser_index = 1;//valid, 0 is from configuration
	input_str    = NULL; //invalid, unused
        //sys_call_spec_t need to be set manually
        sys_call_spec_t cmock_to_return;
	cmock_to_return.sys_call = NULL;//irelevant for this test
	cmock_to_return.sys_call_command = "";//irelevant for this test
	cmock_to_return.num_of_combinators = 2;//from configuration
	cmock_to_return.combinator = NULL;//irelevant for this test
    	int mstp_num_of_cb_tmp[2] = {0, 2};//taken from configuration
	memcpy(cmock_to_return.num_of_cb, mstp_num_of_cb_tmp, sizeof(int)*2);
	parser_spec_t mstp_parsing_cb_tmp[2][2]={//taken from configuration
		[0] = {{NULL, ""}, {NULL, ""}},//irelevant
		[1] = {{NULL, CONFIGURATION_NAME_STR}, {NULL, "0"}}
	};
	memcpy(cmock_to_return.parse_cb, mstp_parsing_cb_tmp, sizeof(parser_spec_t)*2*2);
        //sys_call_spec_t setting is done
	strncpy(result_array[0], "", MAX_STR_LEN);//this case is invalid by the code (covered by the code)

	//mocks
	assemble_sys_call_structure_CMockIgnoreAndReturn(0, cmock_to_return);
	are_object_indices_valid_CMockIgnoreAndReturn(0, 1);//this means that indices are valid
	
	base_status_t a = array_validate_string_length(module_index, obj_index, comb_index, parser_index, input_str, 0, result_array, MAX_NUM_OF_INTERFACES, MAX_STR_LEN);
	TEST_ASSERT_EQUAL_INT_MESSAGE(BASE_VALIDATION_FAILED, a, "===>>> [result_array[0]]=\"\" [match case]=\"0\"  ===>>>  SHOULD RETURN: BASE_VALIDATION_FAILED because input string is empty");
}

//TEST - input sting starts with '\0'
void test_004_array_validate_string_length(void) {
	module_index = MSTP;//valid
	obj_index    = MSTP_CONF_ID_CONF_NAME;//valid
	comb_index   = 1;//valid, 1 is from configuraion
	parser_index = 1;//valid, 0 is from configuration
	input_str    = NULL; //invalid, unused
        //sys_call_spec_t need to be set manually
        sys_call_spec_t cmock_to_return;
	cmock_to_return.sys_call = NULL;//irelevant for this test
	cmock_to_return.sys_call_command = "";//irelevant for this test
	cmock_to_return.num_of_combinators = 2;//from configuration
	cmock_to_return.combinator = NULL;//irelevant for this test
    	int mstp_num_of_cb_tmp[2] = {0, 2};//taken from configuration
	memcpy(cmock_to_return.num_of_cb, mstp_num_of_cb_tmp, sizeof(int)*2);
	parser_spec_t mstp_parsing_cb_tmp[2][2]={//taken from configuration
		[0] = {{NULL, ""}, {NULL, ""}},//irelevant
		[1] = {{NULL, CONFIGURATION_NAME_STR}, {NULL, "3"}}
	};
	memcpy(cmock_to_return.parse_cb, mstp_parsing_cb_tmp, sizeof(parser_spec_t)*2*2);
        //sys_call_spec_t setting is done
	strncpy(result_array[0], "", MAX_STR_LEN);//valid
	result_array[0][0] = '\0';//invalid

	//mocks
	assemble_sys_call_structure_CMockIgnoreAndReturn(0, cmock_to_return);
	are_object_indices_valid_CMockIgnoreAndReturn(0, 1);//this means that indices are valid
	
	base_status_t a = array_validate_string_length(module_index, obj_index, comb_index, parser_index, input_str, 0, result_array, MAX_NUM_OF_INTERFACES, MAX_STR_LEN);
	TEST_ASSERT_EQUAL_INT_MESSAGE(BASE_VALIDATION_FAILED, a, "===>>> [result_array[0]]=\'\\0\' [match case]=\"3\"  ===>>>  SHOULD RETURN: BASE_VALIDATION_FAILED because input string is empty.");
}
////*****END OF TESTING array_validate_string_length*****////

void tearDown(void)
{
}
