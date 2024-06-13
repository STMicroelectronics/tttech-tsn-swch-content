# TTTech YangVerify
# Copyright(c) 2019 TTTech Industrial Automation AG.
#
# ALL RIGHTS RESERVED.
# Usage of this software, including source code, netlists, documentation,
# is subject to restrictions and conditions of the applicable license
# agreement with TTTech Industrial Automation AG or its affiliates.
#
# All trademarks used are the property of their respective owners.
#
# TTTech Industrial Automation AG and its affiliates do not assume any liability
# arising out of the application or use of any product described or shown
# herein. TTTech Industrial Automation AG and its affiliates reserve the right to
# make changes, at any time, in order to improve reliability, function or
# design.
#
# Contact Information:
# support@tttech-industrial.com
# TTTech Industrial Automation AG, Schoenbrunnerstrasse 7, 1040 Vienna, Austria

from model.yang_types import IntType, StrType, EnumType, BitType, BoolType
import re

def validate_yang_type(yang_object, _value):
    if yang_object.type is not None:
        if isinstance(yang_object.type, IntType):
            _range = yang_object.type._range
            _value_int = int(_value)
            if len(_range) == 2:
                if _value_int >= _range[0] and _value_int <= _range[1]:
                    return True
                else:
                    return False
            elif len(_range) == 1:
                if _value_int == _range[0]:
                    return True
                else:
                    return False
            else:
                return False
        elif isinstance(yang_object.type, StrType):
            _pattern = yang_object.type._pattern
            if _pattern is not None:
                _pattern_regex = re.compile(_pattern)
                if _pattern_regex.match(_value):
                    return True
                else:
                    return False
        elif isinstance(yang_object.type, EnumType):
            if _value in yang_object.type.values:
                return True
            else:
                return False
        elif isinstance(yang_object.type, BoolType):
            if _value == 'true' or _value == 'false':
                return True
            else:
                return False
    else:
        return None
            