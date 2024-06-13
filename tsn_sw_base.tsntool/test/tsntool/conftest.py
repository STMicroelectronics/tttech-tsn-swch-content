import pytest
import json

def pytest_generate_tests(metafunc):

    command_help_list= ['brport', 'edge', 'fqtss', 'preempt', 'qbridge', 'st', 'mstp', 'sid']  
    ports = ['sw0p1', 'sw0p2', 'sw0p3', 'sw0p4', 'sw0p5']
    tsn_ports = [ 'sw0p2', 'sw0p3', 'sw0p4', 'sw0p5']  
    negative_tests={}

    
    with open('./tsntool/negative-tests.json', 'r') as outfile:
        negative_tests= json.load(outfile)
    
    if 'command_help' in metafunc.fixturenames:
     
        metafunc.parametrize(('command_help'), 
                             [(command) for command in command_help_list],
                             ids= [command for command in command_help_list ], 
                             scope='session')  
    
    if 'port' in metafunc.fixturenames:
     
        metafunc.parametrize(('port'), 
                             [(port) for port in ports],
                             scope='session')
    
    if 'tsn_port' in metafunc.fixturenames:
     
        metafunc.parametrize(('tsn_port'), 
                             [(port) for port in tsn_ports],
                             scope='session')  
        
    if 'negative_test' in metafunc.fixturenames:
        metafunc.parametrize(('negative_test'), 
                             [(test) for  test in negative_tests],
                             ids=[(test['comment']) for test in negative_tests],
                             scope='session')  