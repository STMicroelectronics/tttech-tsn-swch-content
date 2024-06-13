''' This module contains common functions for all tests '''

import ctypes
from ctypes import *

libbsd = ctypes.CDLL("/usr/lib/libbsd.so", mode=ctypes.RTLD_GLOBAL)
libtsn = cdll.LoadLibrary("libtsn.so.1.4")

def check_table_default_uint(function, port, input_values, expected_values):
    ''' Iterate over a table, call function and check result '''
    port_h = libtsn.tsn_port_open_by_name(c_char_p(port))
    output = c_int64()
    print ('Testing: ', function.__name__, 'Port:', port)
    for input in input_values:
        error = function(port_h,c_uint(input),byref(output))
        print('Index', input, 'Expected', expected_values[input], 'from target', output.value)
        print('Return_code:', error)
        assert error == 0
        assert output.value == expected_values[input]
    libtsn.tsn_port_close(port_h)

def set_table_uint(function, port, input_values, set_values, type):
    ''' Set all elements in a table '''
    port_h = libtsn.tsn_port_open_by_name(c_char_p(port))
    print ('Testing: ', function.__name__, 'Port:', port)
    for index in input_values:
        error = function(port_h,c_uint(index),type(set_values[index]))
        assert error == 0
        print('Index', index, 'Set', set_values[index])
        print('Return_code:', error)
    libtsn.tsn_port_close(port_h)

def check_single_value(function, port, expected_value, type):
    ''' Call a getter function and check the returned value '''
    return_value = type
    port_h = libtsn.tsn_port_open_by_name(c_char_p(port))
    error = function(port_h, byref(return_value))
    print('Expected', expected_value, 'from target', return_value.value, 'Code', error)
    assert error == 0
    assert return_value.value == expected_value
    libtsn.tsn_port_close(port_h)

def check_single_value_index(function, port, index, expected_value, type):
    ''' Check the default value returned by the function for the passed index (uint32)'''
    return_value = type
    port_h = libtsn.tsn_port_open_by_name(c_char_p(port))
    error = function(port_h, index, byref(return_value))
    print('Expected', expected_value, 'from target', return_value.value, 'Code', error)
    assert error == 0
    assert return_value.value == expected_value
    libtsn.tsn_port_close(port_h)

def set_single_value(function, port, set_value, type):
    ''' Call a setter function to set a value '''
    pass_value = type
    pass_value.value = set_value
    port_h = libtsn.tsn_port_open_by_name(c_char_p(port))
    error = function(port_h, pass_value)
    assert error == 0
    libtsn.tsn_port_close(port_h)

def set_single_value_index(function, port, index, set_value, type):
    pass_value = type
    pass_value.value = set_value
    print('index', index, set_value)
    port_h = libtsn.tsn_port_open_by_name(c_char_p(port))
    error = function(port_h, index, pass_value)
    assert error == 0
    libtsn.tsn_port_close(port_h)

def set_and_restore(get_function, set_function, port, set_value, type):
    ''' Uses a pair of getter / setter functions to read the default value
        set a new value, read-back and finally restore the default value''' 
    default_value = type
    port_h = libtsn.tsn_port_open_by_name(c_char_p(port))
    error = get_function(port_h, byref(default_value))
    default = default_value.value  # Necessary to prevent an overwrite of the value
    print('default value before', default)    
    assert error == 0
    set_single_value(set_function, port, set_value, type)
    check_single_value(get_function, port, set_value, type)
    print('default value after', default)    
    set_single_value(set_function, port, default, type)
    check_single_value(get_function, port, default, type)
    libtsn.tsn_port_close(port_h)
