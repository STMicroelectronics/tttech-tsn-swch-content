'''
Created on Feb 25, 2019

@author: tijanak
'''
import os
import sys
from tools.delayed_assert import expect, assert_expectations
from tools.input_service import JSONParseException
from pysnmp.hlapi import *
from pysnmp.smi.view import MibViewController
from pysnmp.smi import builder, view, compiler, rfc1902
from pyasn1.type import univ

def resolve_oid(mib,object):
    os.chdir(os.path.dirname(__file__))
    print(os.getcwd())
    se = SnmpEngine()
    mibBuilder = builder.MibBuilder()
    mibViewController = view.MibViewController(mibBuilder)
    compiler.addMibCompiler(mibBuilder, sources = ['/code/MIB_repository/'])
    mibBuilder.addMibSources(builder.DirMibSource('/code/MIB_repository/'))
    mibBuilder.loadModules(mib)
    mvc = se.getUserContext('mibViewController')
    
    # Split object name and instance
    object_name, object_instance = object.split('.',1)
    # Retrieve base OID of the object
    object_oid = ObjectIdentity(mib, object_name).resolveWithMib(mibViewController)
    #print(str(object_oid)+'.'+object_instance)
    # Create OID of the exact instance to retrieve
    object_oid=ObjectType(ObjectIdentity(str(object_oid)+'.'+object_instance))
    return object_oid

def resolve_oid_min(mib,object):
    se = SnmpEngine()
    mibBuilder = builder.MibBuilder()
    mibViewController = view.MibViewController(mibBuilder)
    compiler.addMibCompiler(mibBuilder, sources = ['/code/MIB_repository/'])
    mibBuilder.addMibSources(builder.DirMibSource('/code/MIB_repository/'))
    mibBuilder.loadModules(mib)
    mvc = se.getUserContext('mibViewController')
    
    # Split object name and instance
    object_name, object_instance = object.split('.',1)
    # Retrieve base OID of the object
    object_oid = ObjectIdentity(mib, object_name).resolveWithMib(mibViewController)
    #print(str(object_oid)+'.'+object_instance)
    # Create OID of the exact instance to retrieve
    return ObjectIdentity(str(object_oid)+'.'+object_instance)

def get_snmp(snmp_service, object_oid):
    g = getCmd(SnmpEngine(),
                          UsmUserData(snmp_service['username']),
                          UdpTransportTarget((snmp_service['ip-address'], 161)), ContextData(), object_oid)
    return g

def set_snmp(snmp_service, mib, object_oid, value):
    object_oid_new= ObjectType(object_oid,20)
    
    g = setCmd(SnmpEngine(),
                          UsmUserData(snmp_service['username']),
                          UdpTransportTarget((snmp_service['ip-address'], 161)), ContextData(), object_oid_new)
    return g

def validate_implemented_data(snmp_service, mib_implemented, object):  
    # Load the MIB from the text file
    object_oid=resolve_oid(mib_implemented,object[0])
    if object[1]['read']=='true':  
        g = get_snmp(snmp_service, object_oid) 
        errorIndication, errorStatus, errorIndex, varBinds = next(g)
        expect (errorIndication==None, "Object does not exist")
        expect (errorStatus==0, "Object does not exist")
        expect (varBinds[0][1]!='', "Object does not exist")
    assert_expectations()
    
def validate_expected_data(snmp_service, mib_expected, object): 
    object_oid=resolve_oid(mib_expected,object[0])
    validate_expected_data_by_oid(snmp_service, mib_expected, object_oid, object[1]['expected_value'] )
    assert_expectations()
    
def validate_expected_data_by_oid(snmp_service, mib, object, value): 
    g = get_snmp(snmp_service, object) 
    errorIndication, errorStatus, errorIndex, varBinds = next(g)

    expect (errorIndication==None, "Object does not exist")
    expect (errorStatus==0, "Object does not exist")
    expect (str(varBinds[0][1])==value, "Value do not match")
    print('From board: ', str(varBinds[0][1]) , 'Expected: ',str(value))
    
def validate_edit_data(snmp_service, mib, object): 
    object_oid=resolve_oid(mib,object[0])
    # Get the original value to restore it at the end of the test
    g = get_snmp(snmp_service, object_oid) 
    errorIndication, errorStatus, errorIndex, varBinds = next(g)
    old_value = varBinds[0][1]
    # Set the value
    new_value=object[1]['new_value']
    object_oid_min=resolve_oid_min(mib,object[0])
    s = set_snmp(snmp_service, mib, object_oid_min, new_value)
    errorIndication, errorStatus, errorIndex, varBinds = next(s)
    expect (errorIndication==None, "Object is not writeable")
    expect (errorStatus==0, "Object is not writeable")
    # Read back the value to check if it's properly applied
    validate_expected_data_by_oid(snmp_service, mib, object_oid, new_value)
    # Restore original value
    s = set_snmp(snmp_service, mib, object_oid, old_value)
    errorIndication, errorStatus, errorIndex, varBinds = next(s)
    
    assert_expectations()
