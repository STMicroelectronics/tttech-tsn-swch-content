'''
Created on Jan 14, 2019

@author: tijanak
'''
# Names of modules
IF_MIB = 'IF-MIB'
IEEE8021_MSTP_MIB= 'IEEE8021-MSTP-MIB'
IEEE8021_QBRIDGE_MIB = 'IEEE8021-Q-BRIDGE-MIB'
IEEE8021_BRIDGE_MIB = 'IEEE8021-BRIDGE-MIB'

#paths
ROOT_INPUT = "./input"
INPUT_TARGET_CONFIG = "./input/{}/{}_configuration.json"
INPUT_EXPECTED = "./input/{}/expected"
INPUT_EDIT = "./input/{}/edit"
INPUT_IMPLEMENTED_OBJ = "./input/{}/implemented-objects"
INPUT_NEW_ENTRIES = "./input/{}/new-entries"
INPUT_USE_CASE = "./input/{}/use-case"
INPUT_ROOT_DIR = "./input/{}/{}"

#key constants
EXPECTED_TYPE = 'expected'
EDIT_TYPE = 'edit'
NEW_ENTRIES_TYPE = 'new-entries'
USE_CASE_TYPE = 'use-case'
IMPLEMENTED_TYPE = 'implemented'
IMPLEMENTED_FILTER_TYPE = 'implemented_filter'
TEST_TYPES =  [EXPECTED_TYPE, EDIT_TYPE, NEW_ENTRIES_TYPE, USE_CASE_TYPE, IMPLEMENTED_TYPE, IMPLEMENTED_FILTER_TYPE]