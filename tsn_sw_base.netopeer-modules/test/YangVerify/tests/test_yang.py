# TTTech YangVerify
# Copyright(c) 2019 TTTech Computertechnik AG.
#
# ALL RIGHTS RESERVED.
# Usage of this software, including source code, netlists, documentation,
# is subject to restrictions and conditions of the applicable license
# agreement with TTTech Computertechnik AG or its affiliates.
#
# All trademarks used are the property of their respective owners.
#
# TTTech Computertechnik AG and its affiliates do not assume any liability
# arising out of the application or use of any product described or shown
# herein. TTTech Computertechnik AG and its affiliates reserve the right to
# make changes, at any time, in order to improve reliability, function or
# design.
#
# Contact Information:
# support@4me.tttech-dependablenetworks.com
# TTTech Computertechnik AG, Schoenbrunnerstrasse 7, 1040 Vienna, Austria

import pytest
from ncclient.operations.rpc import RPCError
from . import test_service
from tools.delayed_assert import expect, assert_expectations


def test_all_data(model_name_impl, data_from_board_all, yang_module, implemented_object):
    test_service.validate_all_data(model_name_impl, data_from_board_all, yang_module, implemented_object)

def test_all_data_with_filters(model_name_impl_filter, implemented_obj, implemented_object_filter, netconf_service):
    test_service.validate_all_data_with_filters(model_name_impl_filter, implemented_obj, implemented_object_filter, netconf_service)

def test_data_with_expected_values(model_name_exp, data_from_board_all, expected_data, yang_module):
    test_service.validate_expected_data(model_name_exp, data_from_board_all, expected_data, yang_module)
 
def test_edit_config_data(model_name_edit, config_data_from_board, edited_data, yang_module, netconf_service ):
    test_service.validate_edit_config_data(model_name_edit, config_data_from_board, edited_data, yang_module, netconf_service, False)
    
def test_negative_edit_config_data(model_name_negative, config_data_from_board, negative_edit_data, yang_module, netconf_service ):
    test_service.validate_edit_config_data(model_name_negative, config_data_from_board, negative_edit_data, yang_module, netconf_service, True)

def test_use_cases(model_name_use_case, config_data_from_board, use_case_data, yang_module, netconf_service):
    try:
        test_service.validate_use_cases(model_name_use_case, config_data_from_board, use_case_data,yang_module, netconf_service, False)
    except RPCError as err:
        expect(False, err)
    finally:
        assert_expectations()
        
def test_negative_use_cases(model_name_negative_use_case, config_data_from_board, negative_use_case_data, yang_module, netconf_service):
    try:
        test_service.validate_use_cases(model_name_negative_use_case, config_data_from_board, negative_use_case_data,yang_module, netconf_service, True)
    except RPCError as err:
        expect(False, err)
    finally:
        assert_expectations()


def test_stability(repeat_configs, netconf_service, repeat_test):
    if repeat_test:
        test_service.repeat_config(100, repeat_configs, netconf_service)
    else:
         pytest.skip('Stability test skipped')