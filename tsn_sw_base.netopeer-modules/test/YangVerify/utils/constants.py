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


# Names of modules
IEEE802_DOT1Q_BRIDGE = 'ieee802-dot1q-bridge'
IETF_INTERFACES = 'ietf-interfaces'
IEEE8021_MSTP = 'ieee8021-mstp'
IEEE802_DOT1AB_LLDP = 'ieee802-dot1ab-lldp'
IEEE802_DOT1Q_FQTSS = 'ieee802-dot1q-fqtss'
IETF_PTP = 'ietf-ptp'
MODEL_DICT = [IEEE802_DOT1Q_BRIDGE, IETF_INTERFACES, IEEE8021_MSTP, IEEE802_DOT1AB_LLDP, IEEE802_DOT1Q_FQTSS, IETF_PTP]

# Filters for whole modules
IEEE802_DOT1Q_BRIDGE_MODULE = '<bridges xmlns="urn:ieee:std:802.1Q:yang:ieee802-dot1q-bridge"/>'
IETF_INTERFACES_MODULE = '<interfaces xmlns="urn:ietf:params:xml:ns:yang:ietf-interfaces"/>'
IEEE8021_MSTP_MODULE = '<ieee8021-mstp xmlns="urn:ietf:params:xml:ns:yang:smiv2:ieee8021-mstp"/>'
IEEE8021_SPANNING_TREE_MODULE = '<ieee8021-spanning-tree xmlns="urn:ietf:params:xml:ns:yang:smiv2:ieee8021-spanning-tree"/>'
IEEE802_DOT1AB_LLDP_MODULE = '<lldp xmlns="urn:ieee:std:802.1AB:yang:ieee802-dot1ab-lldp"/>'
IEEE802_DOT1Q_FQTSS_MODULE = '<ieee802-dot1q-fqtss xmlns="urn:ietf:params:xml:ns:yang:smiv2:ieee802-dot1q-fqtss"/>'
IETF_PTP_MODULE = '<ptp xmlns="urn:ietf:params:xml:ns:yang:ietf-ptp"/>'


NS_DICT = {IEEE802_DOT1Q_BRIDGE : IEEE802_DOT1Q_BRIDGE_MODULE,
           IETF_INTERFACES : IETF_INTERFACES_MODULE,
           IEEE8021_MSTP : IEEE8021_MSTP_MODULE,
           IEEE802_DOT1AB_LLDP : IEEE802_DOT1AB_LLDP_MODULE,
           IEEE802_DOT1Q_FQTSS : IEEE802_DOT1Q_FQTSS_MODULE,
           IETF_PTP : IETF_PTP_MODULE }

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
NEGATIVE_TYPE = 'negative'
NEGATIVE_USE_CASE_TYPE = 'negative-use-case'
TEST_TYPES =  {EXPECTED_TYPE : EXPECTED_TYPE, 
               EDIT_TYPE : EDIT_TYPE,
               NEW_ENTRIES_TYPE : NEW_ENTRIES_TYPE,
               USE_CASE_TYPE : USE_CASE_TYPE,
               IMPLEMENTED_TYPE : IMPLEMENTED_TYPE,
               IMPLEMENTED_FILTER_TYPE : IMPLEMENTED_FILTER_TYPE,
               NEGATIVE_TYPE : NEGATIVE_TYPE,
               NEGATIVE_USE_CASE_TYPE : NEGATIVE_USE_CASE_TYPE}


'''Error messages'''
OBJ_NOT_FOUND_ERR = "Object with following path {} is not found in data from the board{}!\n"
OBJ_NOT_FOUND_ERR_OR_KEY = "Object with following path {} is not found in data from the board{} or key is not unique!\n"
NUM_OBJ_ERR = "For object with following path {}, number of values entered in input file ({}) and number of objects found on the board ({}) do not match{}!\n"
OBJ_NOT_FOUND_YANG_ERR = "Object with path {} does not exist in following yang module: {}!\n"
EXPECTED_ERR = "{}: Expected value ({}) and value from board ({}) do not match{}!\n"
NEGATIVE_EDIT_ERR = "Editing object with following path {}, using value {} is passed, but expected to fail{}!\n"
EDIT_LIST_OBJ_ERR = "For object with following path {}, it is not possible to edit list of objects with following single value {}.{}!\n"
NEW_ENTRY_ERR = "Entry {} not added under container {}{}!"
UNIQUE_KEYS_ERR = ". Unique keys: "




