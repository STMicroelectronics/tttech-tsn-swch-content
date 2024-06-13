'''
Created on Jan 10, 2019

@author: tijanak
'''
import pytest
from tools.snmp_service import SNMPService
from tools.input_service import InputService
from utils.constants import EDIT_TYPE, EXPECTED_TYPE, NEW_ENTRIES_TYPE, USE_CASE_TYPE, IMPLEMENTED_TYPE, IMPLEMENTED_FILTER_TYPE, TEST_TYPES
                            
from pysnmp.hlapi import *
from pysnmp.smi.rfc1902 import ObjectIdentity
from pysnmp.hlapi import varbinds
from pysnmp.smi import builder, view, rfc1902, error
from pysnmp.smi.view import MibViewController

def pytest_addoption(parser):
    parser.addoption("--target", action="append", default=[],
        help="")

@pytest.fixture(scope="session", autouse=True)
def input_service(request):
    target_name = request.config.getoption('--target')[0]
    input_service = InputService.getInputServiceInstance(target_name)
    yield input_service
    
@pytest.fixture(scope="session", autouse=True)
def snmp_service(input_service):
    target_configuration = input_service.get_target_configuration()
    yield target_configuration

@pytest.fixture(scope="session")   
def mib():
    yield 

@pytest.fixture(scope="session")   
def object():
    yield 
@pytest.fixture(scope="session")   
def instance():
    yield 


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
    implemented_model = input_service.get_model_test_per_type(IMPLEMENTED_TYPE)
    expected_model =  input_service.get_model_test_per_type(EXPECTED_TYPE)
    edit_model =  input_service.get_model_test_per_type(EDIT_TYPE)
    implemented_objects={}
    expected_objects = {}
    edit_objects = {}
       
    if 'implemented_object' in metafunc.fixturenames:
        for model in implemented_model:
            implemented_objects[model]=input_service.prepare_input_objects(input_service.get_implemented_objects(model), IMPLEMENTED_TYPE).items()
        metafunc.parametrize(('mib', 'implemented_object'),[(mib,test)  for mib in implemented_model for test in implemented_objects[mib]],
                                                ids= [mib+'::'+item[0] for mib in implemented_model for item in implemented_objects[mib]],scope='session')    
    if 'expected_object' in metafunc.fixturenames:
        for mib in expected_model:
            expected_objects[mib]=input_service.prepare_input_objects(input_service.get_expected_values_for_objects(mib), EXPECTED_TYPE).items()
        metafunc.parametrize(('mib_expected', 'expected_object'),[(mib,test)  for mib in expected_model for test in expected_objects[mib]],
                                                ids= [mib+'::'+item[0] for mib in expected_model for item in expected_objects[mib]],scope='session')   
    if 'edit_object' in metafunc.fixturenames:
        for model in edit_model:
            edit_objects[model]=input_service.prepare_input_objects(input_service.get_edit_config_values(model), EDIT_TYPE).items()
        metafunc.parametrize(('mib', 'edit_object'),[(mib,test)  for mib in edit_model for test in edit_objects[mib]],
                                                ids= [item[0] for model in edit_model for item in edit_objects[model]],scope='session')   
