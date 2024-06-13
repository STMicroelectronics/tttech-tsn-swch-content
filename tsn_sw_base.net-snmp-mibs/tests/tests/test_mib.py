'''
Created on Jan 17, 2019

@author: tijanak
'''
from tests import test_service


def test_implemented_data(snmp_service, mib, implemented_object):
    test_service.validate_implemented_data( snmp_service, mib, implemented_object)
    
def test_expected_data(snmp_service, mib_expected, expected_object):
    test_service.validate_expected_data( snmp_service, mib_expected, expected_object)

def test_edit_data(snmp_service, mib, edit_object):
    test_service.validate_edit_data( snmp_service, mib, edit_object)