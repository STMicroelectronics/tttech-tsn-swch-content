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

import os
import sys
import dicttoxml
import xmltodict
import time
import copy
import pytest
import json
from lxml import etree
from ncclient.operations.rpc import RPCError
from tools.input_service import JSONParseException
from tools.yang_service import YangService
from tools.netconf_service import NetconfService
from tools.delayed_assert import expect, assert_expectations
from tools.validation_service import validate_yang_type
from utils.utils import get_filter_by_module_name, create_filter_from_path, create_filter_from_xpath, create_xpath_for_filtering,\
    create_lxml_xpath, get_xpath_for_container, clean_dict_attr, remove_tag, create_new_element

from utils.constants import OBJ_NOT_FOUND_ERR, OBJ_NOT_FOUND_ERR_OR_KEY, OBJ_NOT_FOUND_YANG_ERR, NUM_OBJ_ERR, EXPECTED_ERR, \
NEGATIVE_EDIT_ERR, EDIT_LIST_OBJ_ERR, UNIQUE_KEYS_ERR, NEW_ENTRY_ERR


def validate_all_data(model_name, data_from_board, yang_service, implemented_obj):
    '''For every object that is marked as implemented in input files, checks if it exists on the target'''
    yang_objects = yang_service[model_name].get_all_data()
	
    yang_paths = [];
       
    for yang_object in yang_objects:
        yang_paths.append(yang_object.path)
        if (implemented_obj[0] == yang_object.path):
            implemented_obj_xpath = yang_object._xpath
            implemented_obj_path = yang_object.path
            yang_obj = yang_object
	    
    if implemented_obj[0] in yang_paths:
        if implemented_obj[1]['read'] == 'true':
            board_elements = data_from_board[model_name].findall(implemented_obj_xpath)
            if(len(board_elements) == 0):
                expect(len(board_elements) != 0, OBJ_NOT_FOUND_ERR.format(implemented_obj[0], ""))
            else:
                for board_el in board_elements:
                    validation_result = validate_yang_type(yang_obj, board_el.text)
                    if validation_result is not None:
                        expect(validation_result, "for {} type is not correct".format(yang_obj.path))
    else:
        #Path is not correct, does not exists
        expect(False, "The path "+implemented_obj[0]+" does not exists in the YANG "+model_name+" module.")
    assert_expectations()
    

def validate_all_data_with_filters(model_name, implemented_obj_all, yang_object, netconf_service, ):
    '''For every object that is marked as implemented in input files, checks if it exists on the target
    by using filter created for that object'''
    implemented_obj = implemented_obj_all[model_name]
    if yang_object.path in implemented_obj.keys():
        if implemented_obj[yang_object.path]['read'] == 'true':
            state_object_filter = create_filter_from_path(yang_object.path)
            state_obj_from_board_XML = netconf_service.get_data_etree(state_object_filter)
            state_obj_from_board_XML_children = state_obj_from_board_XML.findall(yang_object._xpath)
            if len(state_obj_from_board_XML_children) > 0:
                for _element in state_obj_from_board_XML_children:
                    validation_result = validate_yang_type(yang_object, _element.text)
                    if validation_result is not None:
                        expect(validation_result, "for {} type is not correct".format(yang_object.path))
    assert_expectations()
    
    
def validate_expected_data(model_name, data_from_board_all, expected_data, yang_module_all):
    '''Prepare expected data and create xpaths'''
    try:
        data_from_board = data_from_board_all[model_name]
        yang_module = yang_module_all[model_name]
        path_to_obj = expected_data[0]
        expected_data_dict = expected_data[1]
        obj_to_test = yang_module.get_object_by_path(path_to_obj)
        if 'expected_value' in expected_data_dict.keys():
            xpath_dict = create_lxml_xpath(obj_to_test._xpath)
            validate_expected_util(expected_data_dict['expected_value'], data_from_board, xpath_dict, obj_to_test, "")
        elif 'unique_keys' in expected_data_dict.keys():
            unique_keys_obj = expected_data_dict['unique_keys']
            for key, unique_key_dict in unique_keys_obj.items():
                unique_keys_str = create_unique_keys_str(unique_key_dict)
                expected_value = unique_key_dict['expected_value']
                xpath_dict_filter = create_xpath_for_filtering(obj_to_test._xpath, unique_key_dict, yang_module)
                validate_expected_util(expected_value, data_from_board, xpath_dict_filter, obj_to_test, unique_keys_str)
        else:
            raise JSONParseException("Input JSON file is not correctly formated!\n")
    except Exception as error:
        expect(False, error)
        exc_type, exc_obj, exc_tb = sys.exc_info()
        fname = os.path.split(exc_tb.tb_frame.f_code.co_filename)[1]
        sys.stderr.write(" {} ; file= {}; line= {}!\n".format(str(error), str(fname), exc_tb.tb_lineno))
    finally:
        assert_expectations()
        
        
def validate_expected_util(expected_data, data_from_board, xpath_dict, obj_to_test, uniqe_key_str):
    '''For every object that has defined expected value checks if that value match with value
    from the board'''
    ''' Force the data to be a list '''
    expected_values = cast_to_list(expected_data)
    values_from_board = get_element_list_bypath(data_from_board, xpath_dict)
    ''' Check that the number of expected values matches the number of values returned by the target '''
    expect(len(expected_values) == len(values_from_board), NUM_OBJ_ERR.format(obj_to_test.path, len(expected_values), len(values_from_board), uniqe_key_str))
    if len(expected_values) == len(values_from_board):
        for index, value in enumerate(values_from_board):
            expect(value.text == expected_values[index], EXPECTED_ERR.format(obj_to_test.path, expected_values[index], value.text, uniqe_key_str))
    
def validate_edit_config_data(model_name, data_from_board, edited_data, yang_module_all, netconf_service, is_poison):
    '''Prepare edit data and create xpaths'''
    data_from_board_copy = copy.deepcopy(data_from_board[model_name])
    yang_module = yang_module_all[model_name]
    #print(etree.tostring(data_from_board_copy, encoding='unicode', pretty_print=True))
    try:
        path_to_obj = edited_data[0]
        edited_data_dict = edited_data[1]
        obj_to_test = yang_module.get_object_by_path(path_to_obj)
        if 'new_value' in edited_data_dict.keys():
            xpath_dict = create_lxml_xpath(obj_to_test._xpath)
            validate_edit_config_util(netconf_service, yang_module, data_from_board_copy, obj_to_test, path_to_obj, edited_data_dict['new_value'], xpath_dict, is_poison, "")
        elif "unique_keys" in edited_data_dict.keys():
            unique_keys_obj = edited_data_dict['unique_keys']
            for key, unique_key_dict in unique_keys_obj.items():
                unique_keys_str = create_unique_keys_str(unique_key_dict)
                xpath_dict = create_xpath_for_filtering(obj_to_test._xpath, unique_key_dict, yang_module)
                validate_edit_config_util(netconf_service, yang_module, data_from_board_copy, obj_to_test, path_to_obj, unique_key_dict['new_value'], xpath_dict, is_poison, unique_keys_str)

    except RPCError as err:
        if not is_poison:
            print(err.xml)
            expect(False, err)
    except Exception as err:
        raise
        expect(False, err)
        exc_type, exc_obj, exc_tb = sys.exc_info()
        fname = os.path.split(exc_tb.tb_frame.f_code.co_filename)[1]
        sys.stderr.write(" {} ; file= {}; line= {}!\n".format(str(err), str(fname), exc_tb.tb_lineno))
    finally:
        assert_expectations()
        
        
def validate_edit_config_util(netconf_service, yang_module, config_el_xml, obj_to_test, path_to_obj, input_values, xpath_dict, is_poison, unique_keys_str):
    '''For every object that has defined new value, changes this value on the board and checks if the value is as
    expected; if is_poison is true, attempts to change object with an incorrect data and expects an exception
    to be thrown'''
    working_copy = copy.deepcopy(config_el_xml)
    object_to_edit = get_container_element_bypath(working_copy, xpath_dict)
    ''' Backup original value '''
    original_value = object_to_edit.text
    input_list = cast_to_list(input_values)
    ''' Iterate over all values in the list '''
    for input_val in input_list:
        object_to_edit = get_container_element_bypath(working_copy, xpath_dict)
        if is_poison:
            negative_single_value_test(netconf_service, yang_module, working_copy, object_to_edit, input_val, obj_to_test.path, unique_keys_str)
        else:
            positive_single_value_test(netconf_service, yang_module, object_to_edit, input_val, xpath_dict, path_to_obj, unique_keys_str)
    ''' Reset the configuration if any value has been edited '''
    edit_config_object(netconf_service, yang_module, config_el_xml, object_to_edit ,original_value)


def negative_single_value_test(netconf_service, yang_module, config_el_xml, object_to_edit, value_to_set, path, unique_keys_str):
    ''' Expects a RPC exception as the netconf server should not accept the object '''
    with pytest.raises(RPCError) as err:
        edit_config_object(netconf_service, yang_module, config_el_xml, object_to_edit, value_to_set)
        pytest.fail(NEGATIVE_EDIT_ERR.format(path, value_to_set, unique_keys_str))

def positive_single_value_test(netconf_service, yang_module, object_to_edit, value_to_set, xpath_dict, path_to_obj, unique_keys_str):
    ''' Apply a config change for one object, read it back and check the result '''
    edited_config_data = merge_config_object(netconf_service, yang_module, object_to_edit, value_to_set, xpath_dict, path_to_obj)
    edited_value = get_container_element_bypath(edited_config_data, xpath_dict).text
    expect(value_to_set == edited_value, EXPECTED_ERR.format(path_to_obj, value_to_set, edited_value, unique_keys_str))

def validate_use_cases(model_name, data_from_board_all, use_case, yang_module_all, netconf_service, is_poison):
    '''Prepares data from use cases and calls for appropriate operations defined in inputs'''
    yang_module= yang_module_all[model_name]
    ''' Backup data to restore at the end of the test '''
    backup_data = copy.deepcopy(data_from_board_all[model_name])
    edited_data = {}
    working_config = copy.deepcopy(backup_data)
    try:
        use_case_name = use_case[0]
        use_cases = use_case[1]
        if(len(use_cases.keys())):
            for key in sorted(use_cases.keys()):
                if "edit" in key:
                    edit_config_data_at_once(yang_module, working_config, use_cases[key])
                elif "new-entries" in key:
                    add_new_entries_at_once(yang_module, working_config, use_cases[key])
            if is_poison:
                with pytest.raises(RPCError) as err:
                    apply_config(netconf_service, working_config)
                    # Fail the test case if no exception is produced
                    pytest.fail(use_case_name)
            else:
                time.sleep(4)
                apply_config(netconf_service, working_config)
                time.sleep(5)
                edited_data[model_name] = netconf_service.get_data_etree(get_filter_by_module_name(yang_module.name))
                time.sleep(4)
                expected_data = use_cases[key]
                for item in expected_data.items():
                    validate_expected_data(model_name, edited_data, item, yang_module_all)
        else:
            sys.stderr.write("{}: No use cases defined for {}".format(yang_module.name, use_case_name))
    except RPCError as err:
        print(err.xml)
        expect(False, err)
    except Exception as err:
        expect(False, err)
        exc_type, exc_obj, exc_tb = sys.exc_info()
        fname = os.path.split(exc_tb.tb_frame.f_code.co_filename)[1]
        sys.stderr.write(" {} ; file= {}; line= {}!\n".format(str(err), str(fname), exc_tb.tb_lineno))
    finally:
        ''' Restore the complete configuration '''
        apply_config(netconf_service, backup_data)
        assert_expectations()

def repeat_config(iteration, repeat_configs, netconf_service):
    ''' Backup configuration before the test and prepare working copies '''
    config_backup = netconf_service.get_config_data_all('running')
    working_config = copy.deepcopy(config_backup)
    ''' Create Yang model instance '''
    yang_service = YangService.getYangServiceInstance()
    try:
        for current_iter in range(iteration):
            for config_id in repeat_configs.keys():
                ''' Start from a clean config '''
                working_config = copy.deepcopy(config_backup)
                for model_name in repeat_configs[config_id].keys():
                    yang_module = yang_service.yang_module_objects[model_name]
                    obj_list = repeat_configs[config_id][model_name]
                    for config_object in obj_list.items():
                        path_to_obj = config_object[0]
                        object = config_object[1]
                        if 'new_value' in object.keys():
                             obj_to_test = yang_module.get_object_by_path(path_to_obj)
                             xpath_dict = create_lxml_xpath(obj_to_test._xpath)
                             object_to_edit = get_container_element_bypath(working_config, xpath_dict)
                             object_to_edit.text = object['new_value']
                        elif 'new_entries' in object.keys():
                            container_xpath = get_xpath_for_container(path_to_obj, yang_module.get_all_data())
                            xpath_dict = create_lxml_xpath(obj_to_test._xpath)
                            add_multiple_config_element(object['new_entries'], working_config, xpath_dict)
                        elif "unique_keys" in object.keys():
                            unique_keys_obj = object['unique_keys']
                            for key, unique_key_dict in unique_keys_obj.items():
                                if 'new_entries' in unique_key_dict:
                                    container_xpath = get_xpath_for_container(path_to_obj, yang_module.get_all_data())
                                    xpath_dict = create_xpath_for_filtering(container_xpath, unique_key_dict, yang_module)
                                    add_multiple_config_element(unique_key_dict['new_entries'], working_config, xpath_dict)
                                elif 'new_value' in unique_key_dict:
                                    obj_to_test = yang_module.get_object_by_path(path_to_obj)
                                    xpath_dict = create_xpath_for_filtering(obj_to_test._xpath, unique_key_dict, yang_module)
                                    edit_multiple_config_element(working_config, xpath_dict, unique_key_dict['new_value'])
                apply_config(netconf_service, working_config)
        ''' Restore the complete configuration '''
        apply_config(netconf_service, config_backup)
        print(current_iter)
    except RPCError as err:
        print(err.xml)
        print('Iteration :', current_iter)
        expect(False, err)
    except Exception as err:
        expect(False, err)
        exc_type, exc_obj, exc_tb = sys.exc_info()
        fname = os.path.split(exc_tb.tb_frame.f_code.co_filename)[1]
        sys.stderr.write(" {} ; file= {}; line= {}!\n".format(str(err), str(fname), exc_tb.tb_lineno))
    finally:
        ''' Restore the complete configuration '''
        apply_config(netconf_service, config_backup)

def edit_config_data_at_once(yang_module, data_from_board, edited_data):
    '''Attempts to edit data defined in use cases at once, not objects one by one'''
    for path_to_obj, edited_data_dict in edited_data.items():
        obj_to_test = yang_module.get_object_by_path(path_to_obj)
        if 'new_value' in edited_data_dict.keys():
            xpath_dict = create_lxml_xpath(obj_to_test._xpath)
            edit_multiple_config_element(data_from_board,xpath_dict, edited_data_dict['new_value'])
        elif "unique_keys" in edited_data_dict.keys():
            unique_keys_obj = edited_data_dict['unique_keys']
            for key, unique_key_dict in unique_keys_obj.items():
                xpath_dict = create_xpath_for_filtering(obj_to_test._xpath, unique_key_dict, yang_module)
                edit_multiple_config_element(data_from_board,xpath_dict, unique_key_dict['new_value'])


def add_new_entries_at_once(yang_module, data_from_board, new_entries):
    '''Attempts to add all new entries defined in use cases at once, not objects one by one
       The variable data_from_board is modified                                              '''
    for container_path, new_entries_dict in new_entries.items():
        container_xpath = get_xpath_for_container(container_path, yang_module.get_all_data())
        if 'new_entries' in new_entries_dict.keys():
            xpath_dict = create_lxml_xpath(container_xpath)
            add_multiple_config_element(new_entries_dict['new_entries'], data_from_board, xpath_dict)
        elif "unique_keys" in new_entries_dict.keys():
            unique_keys_obj = new_entries_dict['unique_keys']
            for key, unique_key_dict in unique_keys_obj.items():
                xpath_dict = create_xpath_for_filtering(container_xpath, unique_key_dict, yang_module)
                add_multiple_config_element(unique_key_dict['new_entries'], data_from_board, xpath_dict)

def edit_config_object(netconf_service, yang_module, config_el_xml, obj_to_edit, new_values):
    '''Edits a single object and return the config as etree   '''
    ''' config_el_xml is the complete configuration           '''
    ''' obj_to_edit is a pointer to the object to edit        '''
    obj_to_edit.text = new_values
    apply_config(netconf_service, config_el_xml)
    edited_config_data = netconf_service.get_config_data('running', get_filter_by_module_name(yang_module.name))    
    return edited_config_data

def merge_config_object(netconf_service, yang_module, obj_to_edit, new_value, xpath_dict, path_to_obj):
    ''' Edits a single object and return the config as etree  '''
    ''' obj_to_edit is a pointer to the object to edit        '''
    new_config = create_merge_config_el_xml(xpath_dict)
    object_to_edit = get_container_element_bypath(new_config, xpath_dict)
    object_to_edit.text = new_value
    data_to_board = etree.tostring(new_config, encoding='unicode')
    netconf_service.edit_config_data('running', data_to_board, 'merge')
    edited_config_data = netconf_service.get_config_data('running', get_filter_by_module_name(yang_module.name))    
    return edited_config_data

def edit_multiple_config_element(data_to_edit, xpath_dict, new_values):
    ''' Edit multiple elements in data_to_edit '''
    object_to_edit = get_element_list_bypath(data_to_edit, xpath_dict)
    new_entries = cast_to_list(new_values)
    if(len(object_to_edit) > 0):
        if len(new_entries) <= len(object_to_edit):
           for index, new_val in enumerate(new_entries):
               object_to_edit[index].text = new_val

def add_multiple_config_element(new_entries, data_from_board, xpath_dict):
    ''' Add the new elements from new_entries in the root element '''
    if len(data_from_board.xpath(xpath_dict['xpath'], namespaces=xpath_dict['namespaces']))!=0:
        ''' Container exists '''
        root_element = get_container_element_bypath(data_from_board, xpath_dict)
    else:
        ''' Create container DIRTY HACK '''
        xpath_dict['xpath']="./dot1q:bridges/dot1q:bridge[child::dot1q:name[text()='br0']]/dot1q:component[child::dot1q:name[text()='br0']]"
        root_element = get_container_element_bypath(data_from_board, xpath_dict)
        bridge = etree.Element('bridge-vlan')
        bridge.set("xmlns", "urn:ieee:std:802.1Q:yang:ieee802-dot1q-bridge")
        root_element.append(bridge)
        root_element = bridge
        
    for new_entry in new_entries:
        new_el_str = dicttoxml.dicttoxml(new_entry, attr_type=False, root=False)
        new_el_xml = etree.fromstring(new_el_str)
        if b"item" in new_el_str:
            new_el_cln = remove_tag(new_el_xml, "item")
        else:
            new_el_cln = new_el_xml
        if root_element is not None:
            if new_el_cln.tag == "stream-identification": #If container stream-identification is created, then we need to add him namespace.
                # suggestion: The best idea is to add namespaces of augment container inside tests (.json files) and enable it inside framework.
                new_el_cln.set("xmlns", "urn:ieee:std:802.1Q:yang:ieee802-dot1CB")
            if new_el_cln.tag == "frer": #If container stream-identification is created, then we need to add him namespace.
                # suggestion: The best idea is to add namespaces of augment container inside tests (.json files) and enable it inside framework.
                new_el_cln.set("xmlns", "urn:ieee:std:802.1Q:yang:ieee802-dot1CB")
            root_element.append(new_el_cln)

def create_merge_config_el_xml(xpath_dict):
    '''Creates a xml filter for the configuration'''
    config_el = etree.Element('config')
    config_el.set("xmlns", "urn:ietf:params:xml:ns:netconf:base:1.0")
    path = create_filter_from_xpath(xpath_dict)
    data = etree.fromstring(path)
    config_el.append(data)
    return config_el

def create_config_el_xml(data_to_board):
    '''Creates <config> xml element'''
    config_el = etree.Element('config')
    config_el.set("xmlns", "urn:ietf:params:xml:ns:netconf:base:1.0")
    for config_model in data_to_board.getchildren():
        config_el.append(config_model)
    return config_el

def apply_config(netconf_service, data_to_board):
    '''Apply the config given as etree to the target '''
    ''' Create a copy of the data to avoid destruction when appending to the config '''
    data_copy = copy.deepcopy(data_to_board)
    config_el = create_config_el_xml(data_copy)  
    print(etree.tostring(config_el, encoding='unicode', pretty_print=True))
    netconf_service.edit_config_data('running', etree.tostring(config_el, encoding='unicode'), 'replace')
    
def create_unique_keys_str(unique_keys_dict):
    unique_keys_str = UNIQUE_KEYS_ERR
    for index, (key_path, value) in enumerate(unique_keys_dict['paths'].items()):
        if (index + 1) == len(unique_keys_dict['paths'].items()):
            unique_keys_str += key_path + " ({})".format(value['key_value'])
        else:
            unique_keys_str += key_path + " ({}), ".format(value['key_value'])
    return unique_keys_str
            
def get_element_list_bypath(data_element, xpath_dict):
    ''' Returns a list of element to edit '''
    return data_element.xpath(xpath_dict['xpath'], namespaces=xpath_dict['namespaces'])

def get_container_element_bypath(data_element, xpath_dict):
    ''' Returns one element based on the xpath '''
    #print(etree.tostring(data_element, encoding='unicode', pretty_print=True))
    return data_element.xpath(xpath_dict['xpath'], namespaces=xpath_dict['namespaces'])[0]


def cast_to_list(input_dict):
    if type(input_dict) is list:
        new_values = input_dict
    else:
        new_values = [input_dict]
    return new_values

