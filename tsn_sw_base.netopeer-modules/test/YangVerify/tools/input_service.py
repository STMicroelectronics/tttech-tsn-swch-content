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

import os
import json
import sys
from collections import OrderedDict
from utils.constants import INPUT_TARGET_CONFIG, NEGATIVE_TYPE, \
    EDIT_TYPE, EXPECTED_TYPE, NEW_ENTRIES_TYPE, USE_CASE_TYPE, IMPLEMENTED_TYPE, NEGATIVE_USE_CASE_TYPE, \
    INPUT_ROOT_DIR

from pyang.translators import yang

class InputService():
    
    inputServiceInstance = None
    
    @classmethod
    def getInputServiceInstance(cls, target_name):
        if cls.inputServiceInstance is not None:
            return cls.inputServiceInstance
        else:
            cls.inputServiceInstance = InputService(target_name)
            return cls.inputServiceInstance
    
    def __init__(self, target_name=None, root_folder="./input"):
        self.target_name = target_name
        self.target_config_file = INPUT_TARGET_CONFIG.format(self.target_name, self.target_name)
        self.target_configuration = {}
        self.root_folder = root_folder
        self.test_list_per_type = {}
        self.all_input_obj = {}
        self.all_repeat_obj = {}
        self.input_root_dir = INPUT_ROOT_DIR


        self.init_config_file()
        if self.target_configuration["repeat"]:
            repeat_name = self.input_root_dir.format(target_name, "repeat")
            self.init_get_repeat(self.all_repeat_obj, repeat_name)
            
        for test_type in self.test_list_per_type.keys():
            target_name = self.target_name
            if ":" in test_type:
                test_type_seg = test_type.split(":")
                test_type = test_type_seg[0]
                target_name = test_type_seg[1]
            if test_type.startswith("implemented"):
                test_type = IMPLEMENTED_TYPE
            self.init_get_input(self.all_input_obj, self.input_root_dir.format(target_name, test_type), test_type, self.test_list_per_type[test_type])

    
    def init_config_file(self):
        ''' Read JSON file for configuration '''
        try:
            configuration_file = open(self.target_config_file)
            configuration_file_str = configuration_file.read()
            configuration_dict = json.loads(configuration_file_str)
            if "target" in configuration_dict.keys():
                if "ip-address" in configuration_dict["target"].keys():
                    self.target_configuration["ip-address"] = configuration_dict["target"]['ip-address']
                else:
                    raise JSONParseException("'ip-address' parameter is missing in JSON file!\n")
                if "password" in configuration_dict["target"].keys():
                    self.target_configuration["password"] = configuration_dict["target"]["password"]
                else:
                    raise JSONParseException("'password' parameter is missing in JSON file!\n")
                if "username" in configuration_dict["target"].keys():
                    self.target_configuration["username"] = configuration_dict["target"]["username"]
                else:
                    raise JSONParseException("'username' parameter is missing in JSON file!\n") 
                if "repeat" in configuration_dict["target"].keys():
                    self.target_configuration["repeat"] = configuration_dict["target"]["repeat"]
                else:
                    raise JSONParseException("'repeat' parameter is missing in JSON file!\n") 
            else:
                raise JSONParseException("'target' parameter is missing in JSON file!\n")
            if "tests" in configuration_dict.keys():
                for model in configuration_dict["tests"].keys():
                    if len (configuration_dict["tests"][model])>0:
                        for test in configuration_dict["tests"][model]:
                            if test in self.test_list_per_type.keys():
                                self.test_list_per_type[test].append(model)
                            else:
                                self.test_list_per_type[test]= [model]
            else:
                raise JSONParseException("tests are not defined in JSON file!\n")
        except IOError as error:
            sys.stderr.write(str(error))
        except ValueError:
            sys.stderr.write("JSON file {} is not in the correct format!\n".format(self.target_config_file))
        except JSONParseException as error:
            sys.stderr.write(str(error))
        else:
            configuration_file.close()
    
    
    def init_get_repeat(self, target_dict, repeat_path):
        ''' Read JSON file for testing '''
        if os.path.exists(repeat_path):
            for sub_dir_name in os.listdir(repeat_path):
                sub_dir_path = os.path.join(repeat_path, sub_dir_name)
                if os.path.exists(sub_dir_path):
                    target_dict[sub_dir_name] = {}
                    for filename in os.listdir(sub_dir_path):
                        model_name = os.path.splitext(filename)[0]
                        try:
                            file_path = os.path.join(sub_dir_path, filename)
                            _file_dict = self.load_json_file(file_path)
                            target_dict[sub_dir_name][model_name]= {}
                            target_dict[sub_dir_name][model_name] = self.prepare_input_objects(_file_dict, NEW_ENTRIES_TYPE)
                        except IOError as error:
                            sys.stderr.write(str(error))
                        except ValueError:
                            sys.stderr.write("JSON file {} is not in the correct format!\n".format(file_path))
                else:
                    sys.stderr.write("{} path does not exist!\n".format(sub_dir_path))

    def init_get_input(self, target_dict, path, identifier, yang_model_list):
        ''' Read JSON file for testing '''
        target_dict[identifier] = {}
        if identifier == USE_CASE_TYPE or identifier == NEGATIVE_USE_CASE_TYPE :
            for dir_name in yang_model_list:
                target_dict[identifier][dir_name] = {}
                dir_path = os.path.join(path, dir_name)
                if os.path.exists(dir_path):
                    for sub_dir_name in os.listdir(dir_path):
                        sub_dir_path = os.path.join(dir_path, sub_dir_name)
                        target_dict[identifier][dir_name][sub_dir_name] = {}
                        if os.path.exists(sub_dir_path):
                            for filename in os.listdir(sub_dir_path):
                                try:
                                    file_path = os.path.join(sub_dir_path, filename)
                                    _file_dict = self.load_json_file(file_path)
                                    filename_seg = filename.split(".")[0]
                                    if EDIT_TYPE in filename_seg:
                                        target_dict[identifier][dir_name][sub_dir_name][filename_seg] = self.prepare_input_objects(_file_dict, EDIT_TYPE)
                                    elif EXPECTED_TYPE in filename_seg:
                                        target_dict[identifier][dir_name][sub_dir_name][filename_seg] = self.prepare_input_objects(_file_dict, EXPECTED_TYPE)
                                    else:
                                        target_dict[identifier][dir_name][sub_dir_name][filename_seg] = self.prepare_input_objects(_file_dict, NEW_ENTRIES_TYPE)
                                except IOError as error:
                                    sys.stderr.write(str(error))
                                except ValueError:
                                    sys.stderr.write("JSON file {} is not in the correct format!\n".format(file_path))
                        else:
                            sys.stderr.write("{} path does not exist!\n".format(sub_dir_path))
                else:
                    sys.stderr.write("{} path does not exist!\n".format(dir_path))
        else:
            files_to_be_loaded = [identifier+"_"+yang_model+".json" for yang_model in yang_model_list]
            for filename in files_to_be_loaded:
                file_path = os.path.join(path, filename)
                try:
                    filename_segments = filename.split('_')
                    module_name = filename_segments[1].split(".")[0]
                    _file_dict = self.load_json_file(file_path)
                    target_dict[identifier][module_name] = _file_dict
                except IOError as error:
                    sys.stderr.write(str(error)+"\n")
                except ValueError:
                    sys.stderr.write("JSON file {} is not in the correct format!\n".format(file_path))

    def get_model_test_per_type(self, test_type):
        if test_type in self.test_list_per_type.keys():
            return self.test_list_per_type[test_type]
        else:
            return {}                
    
    def get_implemented_objects(self, yang_module_name):
        if yang_module_name in self.all_input_obj[IMPLEMENTED_TYPE].keys():
            return self.all_input_obj[IMPLEMENTED_TYPE][yang_module_name]
        else:
            return {}
    
    def get_expected_values_for_objects(self, yang_module_name):
        if yang_module_name in self.all_input_obj[EXPECTED_TYPE].keys():
            return self.all_input_obj[EXPECTED_TYPE][yang_module_name]
        else:
            return {}
    
    def get_target_configuration(self):
        return self.target_configuration
    
    def get_edit_config_values(self, yang_module_name):
        if yang_module_name in self.all_input_obj[EDIT_TYPE].keys():
            return self.all_input_obj[EDIT_TYPE][yang_module_name]
        else:
            return {}
 
    def get_use_cases(self, yang_module_name):
        if yang_module_name in self.all_input_obj[USE_CASE_TYPE].keys():
            return self.all_input_obj[USE_CASE_TYPE][yang_module_name]
        else:
            return {}
        
    def get_negative_values(self, yang_module_name):
        if yang_module_name in self.all_input_obj[NEGATIVE_TYPE].keys():
            return self.all_input_obj[NEGATIVE_TYPE][yang_module_name]
        else:
            return {}
    def get_negative_use_cases(self, yang_module_name):
        if yang_module_name in self.all_input_obj[NEGATIVE_USE_CASE_TYPE].keys():
            return self.all_input_obj[NEGATIVE_USE_CASE_TYPE][yang_module_name]
        else:
            return {}
    
    def get_repeat_config(self):
        return self.all_repeat_obj

    def load_json_file(self,file_path):
        ''' Open a json file and returns the content in a dictionary '''
        obj_file = open(file_path)
        obj_file_str = obj_file.read()
        obj_file.close()
        return json.loads(obj_file_str, object_pairs_hook=OrderedDict)

    def prepare_input_objects(self, input_data, _type):
        '''Convert JSON input file to be possible to parameterize test functions with yang objects'''
        expected_values_per_obj_dict = {}
        try:
            ''' Process simple objects (not in a list) '''
            for key,value in input_data.items():
                if key != "unique_keys":
                    expected_values_per_obj_dict[key] = value
            ''' Process the objects belonging to a list '''
            if "unique_keys" in input_data.keys():
                for key, unique_key_item in input_data['unique_keys'].items():
                    ''' Extract dict for all unique keys '''
                    unique_key = self.get_key_or_exception('key',unique_key_item, "'key' parameter is missing in JSON file!\n")
                    ''' Extract the value of each unique key '''
                    unique_key_values = self.get_key_or_exception('key_values',unique_key_item, "'key_values' parameter is missing in JSON file!\n")
                    ''' Process all objects in the list matching the unique key '''
                    for unique_key_id, unique_key_values_item in unique_key_values.items():
                        unique_key_value = self.get_key_or_exception('value', unique_key_values_item, "'value' parameter is missing in JSON file!\n")
                        unique_key_value_objects = self.get_key_or_exception('objects', unique_key_values_item, "'objects' parameter is missing in JSON file!\n")
                        ''' Prepare dictionary for unique keys to be added on each object '''
                        unique_key_dict = {}
                        for key, value in unique_key.items():
                            unique_key_dict[value] = {}
                            unique_key_dict[value]['key_value'] = unique_key_value[int(key) - 1]
                        ''' Process all objects in the list '''
                        for object_path, object_item in unique_key_value_objects.items():
                            if object_path not in expected_values_per_obj_dict.keys():
                                expected_values_per_obj_dict[object_path] = {}
                                expected_values_per_obj_dict[object_path]['unique_keys'] = {}
                            expected_values_per_obj_dict[object_path]['unique_keys'][unique_key_id] = {}
                            expected_values_per_obj_dict[object_path]['unique_keys'][unique_key_id]["paths"] = {}
                            expected_values_per_obj_dict[object_path]['unique_keys'][unique_key_id]["paths"] = unique_key_dict
                            ''' Allow to mix different type of test in one json file '''
                            if 'new_entries' in object_item.keys():
                                type_value = 'new_entries'
                            elif 'expected_value' in object_item.keys():
                                type_value = 'expected_value'
                            elif 'new_value' in object_item.keys():
                                type_value = 'new_value'
                            expected_values_per_obj_dict[object_path]['unique_keys'][unique_key_id][type_value] = object_item[type_value]
  
#             print(expected_values_per_obj_dict)
        except JSONParseException as error:
            sys.stderr.write(str(error))
        except KeyError as error:
            exc_type, exc_obj, exc_tb = sys.exc_info()
            fname = os.path.split(exc_tb.tb_frame.f_code.co_filename)[1]
            sys.stderr.write("Key Error {} in file {} on line {}!\n".format(str(error), str(fname), exc_tb.tb_lineno))
#         print expected_values_per_obj_dict
        return expected_values_per_obj_dict

    def get_key_or_exception(self,key, obj_dict, error_message):
        ''' Extract the data for the given key or throws an exception '''
        if key in obj_dict.keys():
            return obj_dict[key]
        else:
            raise JSONParseException(error_message)

class JSONParseException(Exception):
    
    def __init__(self, message):
        super(JSONParseException, self).__init__(message)

    
        
        
        
        