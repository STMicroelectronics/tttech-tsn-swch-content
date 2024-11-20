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
from lxml import etree
from tools.netconf_service import NetconfService
from tools.yang_service import YangService
from tools.input_service import InputService
from utils.constants import EDIT_TYPE, EXPECTED_TYPE, NEW_ENTRIES_TYPE, USE_CASE_TYPE, IMPLEMENTED_TYPE, IMPLEMENTED_FILTER_TYPE, TEST_TYPES,  \
                            NS_DICT, MODEL_DICT, NEGATIVE_TYPE, NEGATIVE_USE_CASE_TYPE
from utils.utils import get_filter_by_module_name

def pytest_addoption(parser):
    parser.addoption("--target", action="append", default=[],
        help="")

@pytest.fixture(scope="session", autouse=True)
def input_service(request):
    target_name = request.config.getoption('--target')[0]
    input_service = InputService.getInputServiceInstance(target_name)
    yield input_service
    
@pytest.fixture(scope="session", autouse=True)
def netconf_service(input_service):
#     print("\nConnecting to netconf...")
    target_configuration = input_service.get_target_configuration()
    netconf_service = NetconfService(None, target_configuration['ip-address'], target_configuration["username"],
                                     target_configuration["password"])
    netconf_service.create_connection_ssh()
    yield netconf_service
#     print("\nClose connection...")
    netconf_service.close_connection_ssh()

@pytest.fixture(scope="session", autouse=True)
def repeat_test(input_service):
    target_configuration = input_service.get_target_configuration()
    yield target_configuration['repeat']

@pytest.fixture(scope="session", autouse=True)
def yang_service():
    yang_service = YangService.getYangServiceInstance()
    return yang_service

@pytest.fixture(scope="module")
def yang_service_clean_all_data(yang_service):
    yield
    yang_service.clean()

@pytest.fixture(scope="session")
def yang_module(yang_service):
    yang_module = {}
    for model in MODEL_DICT:
        yang_module[model]= yang_service.yang_module_objects[model]
    yield yang_module
  
@pytest.fixture(scope="session")      
def config_data_from_board(netconf_service):
    config_data_from_board = {}
    for model in MODEL_DICT:
        config_data_from_board [model] = netconf_service.get_config_data('running', get_filter_by_module_name(model))
    yield config_data_from_board
 
@pytest.fixture(scope="session")  
def data_from_board_all(netconf_service):
    data_from_board = {}
    for model in MODEL_DICT:
        data_from_board[model] = etree.fromstring(netconf_service.get_data(NS_DICT[model]), parser= etree.XMLParser(encoding='utf-8'))
    yield data_from_board
    
@pytest.fixture(scope="session")   
def implemented_obj(input_service):
    yield input_service.all_input_obj[IMPLEMENTED_TYPE]

@pytest.fixture(scope="session")
def repeat_configs(input_service):
    yield input_service.all_repeat_obj

def pytest_report_teststatus(report):
    category, short, verbose = '', '', ''
    if hasattr(report, 'wasxfail'):
        if report.skipped:
            category = 'xfailed'
            verbose = 'xfail'
        elif report.passed:
            category = 'xpassed'
            verbose = ('XPASS', {'yellow': True})
        return (category, short, verbose)
    elif report.when in ('setup', 'teardown'):
        if report.failed:
            category = 'error'
            verbose = 'ERROR'
        elif report.skipped:
            category = 'skipped'
            verbose = 'SKIPPED'
        return (category, short, verbose)
    category = report.outcome
    verbose = category.upper()
    return (category, short, verbose)

    
def pytest_generate_tests(metafunc):
    input_service = InputService.getInputServiceInstance(metafunc.config.option.target[0])
    yang_service = YangService.getYangServiceInstance()
    validation_model = input_service.get_model_test_per_type(IMPLEMENTED_TYPE)
    validation_filter_model = input_service.get_model_test_per_type(IMPLEMENTED_FILTER_TYPE)
    expected_model = input_service.get_model_test_per_type(EXPECTED_TYPE)
    edit_model = input_service.get_model_test_per_type(EDIT_TYPE)
    new_entry_model = input_service.get_model_test_per_type(NEW_ENTRIES_TYPE)
    use_case_model = input_service.get_model_test_per_type(USE_CASE_TYPE)
    negative_model = input_service.get_model_test_per_type(NEGATIVE_TYPE)
    negative_use_case_model = input_service.get_model_test_per_type(NEGATIVE_USE_CASE_TYPE)


    implemented = {}
    yang_objects = {}     
    if 'implemented_object' in metafunc.fixturenames:
        for model in validation_model:
            #print(input_service.get_implemented_objects(model))
            implemented[model] = input_service.prepare_input_objects(input_service.get_implemented_objects(model), IMPLEMENTED_TYPE).items()
        metafunc.parametrize(('model_name_impl','implemented_object'), 
                             [(model, implemented_object) for model in validation_model 
                              for implemented_object in implemented[model]],
                             ids= [item[0] for model in validation_model for item in implemented[model]], 
                             scope='session') 
        
    if 'implemented_object_filter' in metafunc.fixturenames:
        for model in validation_filter_model:
            yang_objects[model] = yang_service.yang_module_objects[model].get_all_data()
        metafunc.parametrize(('model_name_impl_filter','implemented_object_filter'), 
                             [(model, yang_object) for model in validation_filter_model 
                              for yang_object in yang_objects[model]],
                             ids= [item.path for model in validation_filter_model for item in yang_objects[model]], 
                             scope='session')    
    # Array for parameterization for ieee802_dot1q_bridge
    expected_data = {}
    if 'expected_data' in metafunc.fixturenames:
        for model in expected_model:
            expected_data[model] = input_service.prepare_input_objects(input_service.get_expected_values_for_objects(model), EXPECTED_TYPE).items()
        metafunc.parametrize(('model_name_exp', 'expected_data'), 
                              [(model, expected_value) for model in expected_model
                               for expected_value in expected_data[model]],
                               ids= [item[0] for model in expected_model for item in expected_data[model]], 
                               scope='session')
        
    input_edit_data = {}
    if 'edited_data' in metafunc.fixturenames:
        for model in edit_model:    
            input_edit_data[model] = input_service.prepare_input_objects(input_service.get_edit_config_values(model), EDIT_TYPE).items()
        
        metafunc.parametrize(('model_name_edit','edited_data'),
                             [(model, edited_data) for model in edit_model 
                             for edited_data in input_edit_data[model]], 
                             ids= [item[0] for model in edit_model for item in input_edit_data[model]],  
                             scope='session')

    # Parametrize tests for new entries
    new_entry_data = {}
    if 'new_entry_data' in metafunc.fixturenames:
        for model in new_entry_model:
            new_entry_data[model] = input_service.prepare_input_objects(input_service.get_new_entries(model), NEW_ENTRIES_TYPE).items()
        metafunc.parametrize(('model_name_new','new_entry_data'),
                             [(model, new_entry) for model in new_entry_model 
                             for new_entry in new_entry_data[model]], 
                             ids= [item[0] for model in new_entry_model for item in new_entry_data[model]],  
                             scope='session')
        
    # Parametrize use-case tests    
    use_case_data = {}
    if 'use_case_data' in metafunc.fixturenames:
        for model in use_case_model:
            use_case_data[model] = input_service.get_use_cases(model).items()
        metafunc.parametrize(('model_name_use_case','use_case_data'),
                             [(model, use_case) for model in use_case_model 
                             for use_case in use_case_data[model]], 
                             ids= [item[0] for model in use_case_model for item in use_case_data[model]],  
                             scope='session')
        
    negative_edit_data = {}
    if 'negative_edit_data' in metafunc.fixturenames:
        for model in negative_model:
            negative_edit_data[model] = input_service.prepare_input_objects(input_service.get_negative_values(model), NEGATIVE_TYPE).items()
        metafunc.parametrize(('model_name_negative','negative_edit_data'),
                             [(model, negative_edit) for model in negative_model 
                             for negative_edit in negative_edit_data[model]], 
                             ids= [item[0] for model in negative_model for item in negative_edit_data[model]],  
                             scope='session')

     # Parametrize use-case tests    
    negative_use_case_data = {}
    if 'negative_use_case_data' in metafunc.fixturenames:
        for model in negative_use_case_model:
            negative_use_case_data[model] = input_service.get_negative_use_cases(model).items()
        metafunc.parametrize(('model_name_negative_use_case','negative_use_case_data'),
                             [(model, use_case) for model in negative_use_case_model 
                             for use_case in negative_use_case_data[model]], 
                             ids= [item[0] for model in negative_use_case_model for item in negative_use_case_data[model]],  
                             scope='session')
