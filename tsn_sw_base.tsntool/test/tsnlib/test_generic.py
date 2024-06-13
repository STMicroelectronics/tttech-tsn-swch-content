''' This module contains the generic test functions.
    The parametrization of the tests is done in conftest.py '''
import ctypes
from ctypes import *
from util import check_single_value, check_single_value_index

libbsd = ctypes.CDLL("/usr/lib/libbsd.so", mode=ctypes.RTLD_GLOBAL)
libtsn = cdll.LoadLibrary("libtsn.so.1.4")

def test_null_point_port(function_null, null_pointer_error):
    ''' Null pointer for port handler'''
    tc = c_uint8()
    error = function_null(None, c_uint8(7), byref(tc))
    assert error == null_pointer_error
    error = 0

def test_param1_out(function_param1_out, port, index, type, index_error):
    ''' Get function with index out of range'''
    output = type
    port_h = libtsn.tsn_port_open_by_name(c_char_p(port))
    error = function_param1_out(port_h, index, byref(output))
    assert error == index_error
    libtsn.tsn_port_close(port_h)

def test_first_index_set_out(function_first_index_set_out, port1, out_index_value1, index1, param_error1):
    ''' Set with out of range parameter '''
    port_h = libtsn.tsn_port_open_by_name(c_char_p(port1))
    error = function_first_index_set_out(port_h, out_index_value1, index1)
    assert error == param_error1
    libtsn.tsn_port_close(port_h)

def test_second_index_set_out(function_second_index_set_out, port2, index2, out_index_value2, param_error2):
    ''' Set with out of range parameter '''
    port_h = libtsn.tsn_port_open_by_name(c_char_p(port2))
    error = function_second_index_set_out(port_h, index2, out_index_value2)
    assert error == param_error2
    libtsn.tsn_port_close(port_h)


def test_test_set_out(function_set_out, port, set_error, out_value):
    ''' Set with single out of range value'''
    port_h = libtsn.tsn_port_open_by_name(c_char_p(port))
    error = function_set_out(port_h, out_value)
    assert error == set_error
    error = 0
    libtsn.tsn_port_close(port_h)

def test_get_nok(function_nok, port, nok_error):
    ''' Test getter function failing '''
    port_h = libtsn.tsn_port_open_by_name(c_char_p(port))
    out_value = c_uint32() 
    error = function_nok(port_h, byref(out_value))
    assert error == nok_error
    error = 0
    libtsn.tsn_port_close(port_h)

def test_single_value(function_single, port_single, expected_value, value_type):
        ''' Check the return value of a single function '''
        check_single_value(function_single, port_single, expected_value, value_type)
