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

import tools.settings as sett
from tools import settings
from tools.settings import current_yang_module

built_in_types = ['binary', 'bits', 'boolean', 'decimal64', 'empty', 'enumeration', 'identityref',
                  'instance-identifier', 'int8', 'int16', 'int32', 'int64', 'leafref', 'string',
                  'uint8', 'uint16', 'uint32', 'uint64', 'union']

built_in_types_int = {
    
    "int8": {
        "range":  [-128,127]
    },
    "int16": {
        "range": [-32768, 32767]
    },
    "int32": {
        "range": [-2147483648, 2147483647]
    },
    "int64": {
        "range": [-9223372036854775808, 9223372036854775807]
    },
    "uint8": {
        "range": [0, 255]
    },
    "uint16": {
        "range": [0, 65535]
    },
    "uint32": {
        "range": [0, 4294967295]
    },
    "uint64": {
        "range": [0, 18446744073709551615]
    }

}

def create_yang_type(yang_module, type_dict):
    type_name = type_dict['@name']
    type_obj = None 
    if type_name in built_in_types:
        type_obj = create_built_in_type(type_name, type_dict)
    elif ":" in type_name:
        if type_name in sett.yang_derived_types.keys():
            type_obj = create_built_in_type(type_name, sett.yang_derived_types[type_name])
    else:
        prefix = yang_module.prefix
        type_name_with_prefix = prefix + ":" + type_name
        if type_name_with_prefix in sett.yang_derived_types.keys():
            type_obj = create_built_in_type(type_name_with_prefix, sett.yang_derived_types[type_name_with_prefix])
    return type_obj
    

def create_built_in_type(type_name, type_dict):
    type_obj = None
    if type_name in built_in_types_int.keys():
        type_obj = create_int_type(type_name, type_dict)
    elif type_name == 'string':
        type_obj = create_str_type(type_name, type_dict)
    elif type_name == 'enumeration':
        type_obj = create_enum_type(type_name, type_dict)
    elif type_name == 'bits':
        type_obj = create_bit_type(type_name, type_dict)
    elif type_name == 'binary':
        type_obj = create_binary_type(type_name, type_dict)
    elif type_name == 'boolean':
        type_obj = create_bool_type(type_name, type_dict) 
    return type_obj


def create_int_type(type_name, type_dict):
    if 'range' in type_dict.keys():
        type_range = []
        type_range_str = type_dict['range']['@value']
        type_range_str_array = type_range_str.split('..')
        for item in type_range_str_array:
            if 'min' == item:
                type_range.append(int(built_in_types_int[type_name]['range'][0]))
            elif 'max' == item:
                type_range.append(int(built_in_types_int[type_name]['range'][1]))
            else:
                type_range.append(int(item))
    else:
        type_range = built_in_types_int[type_name]['range']
            
    return IntType(type_name, type_range)


def create_str_type(type_name, type_dict):
    type_pattern = None
    if 'pattern' in type_dict.keys():
        type_pattern = type_dict['pattern']['@value']
        
    return StrType(type_name, type_pattern)

def create_enum_type(type_name, type_dict):
    enum_values = []
    enum_values_obj = type_dict['enum']
    if type(enum_values_obj) is list:
        for item in enum_values_obj:
            enum_values.append(item['@name'])
    else:
        enum_values.append(enum_values_obj['@name'])
    return EnumType(type_name, enum_values)

def create_bit_type(type_name, type_dict):
    bit_values = []
    bit_obj = type_dict['bit']
    for item in bit_obj:
        bit_values.insert(int(item['position']['@value']), item['@name'])
#       bit_values[int(item['position']['@value'])] = item['@name']
    return BitType(type_name, bit_values)

def create_binary_type(type_name, type_dict):
    _length = None
    if 'length' in type_dict.keys():
        _length = type_dict['length']['@value']
    return BinaryType(type_name, _length)

def create_bool_type(type_name, type_dict):
    return BoolType(type_name)


class IntType():
    
    def __init__(self, name, _range=None):
        self.name = name
        self._range = _range
        
        
class StrType():
    
    def __init__(self, name, _pattern=None):
        self.name = name
        self._pattern = _pattern
        
        
class EnumType():
    
    def __init__(self, name, values=[]):
        self.name = name
        self.values = values
        
        
class BitType():
    
    def __init__(self, name, _bits=[]):
        self.name = name
        self._bits = _bits
        
        
class BinaryType():
    
    def __init__(self, name, _length=None):
        self.name = name
        self._length = _length
        
        
class BoolType():
    
    def __init__(self, name):
        self.name = name
    
        
    
    
    