'''
Created on Feb 19, 2019

@author: tijanak
'''
import os
import json
import sys
from collections import OrderedDict
from utils.constants import INPUT_EXPECTED, INPUT_EDIT, INPUT_IMPLEMENTED_OBJ, INPUT_TARGET_CONFIG,\
    INPUT_NEW_ENTRIES, INPUT_USE_CASE, \
    EDIT_TYPE, EXPECTED_TYPE, NEW_ENTRIES_TYPE, USE_CASE_TYPE, IMPLEMENTED_TYPE, IMPLEMENTED_FILTER_TYPE, TEST_TYPES,\
    INPUT_ROOT_DIR

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
        self.input_root_dir = INPUT_ROOT_DIR


        self.init_config_file()
        for test_type in self.test_list_per_type.keys():
            if test_type.startswith("implemented"):
                test_type = IMPLEMENTED_TYPE
            self.init_get_input(self.all_input_obj, self.input_root_dir.format(self.target_name, test_type), test_type, self.test_list_per_type[test_type])

    
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
    
    
    def init_get_input(self, target_dict, path, identifier, yang_model_list):
        target_dict[identifier] = {}
        files_to_be_loaded = [identifier+"_"+yang_model+".json" for yang_model in yang_model_list]
        for filename in files_to_be_loaded:
            file_path = os.path.join(path, filename)
            try:
                    obj_file = open(file_path)
                    obj_file_str = obj_file.read()
                    obj_dict = json.loads(obj_file_str)
                    filename_segments = filename.split('_')
                    module_name = filename_segments[1].split(".")[0]
                    target_dict[identifier][module_name] = obj_dict
#                     print target_dict
            except IOError as error:
                    sys.stderr.write(str(error)+"\n")
            except ValueError:
                sys.stderr.write("JSON file {} is not in the correct format!\n".format(file_path))
            else:
                obj_file.close()
                    

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
    
    def get_new_entries(self, yang_module_name):
        if yang_module_name in self.all_input_obj[NEW_ENTRIES_TYPE].keys():
            return self.all_input_obj[NEW_ENTRIES_TYPE][yang_module_name]
        else:
            return {}
    
    def get_use_cases(self, yang_module_name):
        if yang_module_name in self.all_input_obj[USE_CASE_TYPE].keys():
            return self.all_input_obj[USE_CASE_TYPE][yang_module_name]
        else:
            return {}
    
    def prepare_input_objects(self, input_data, _type):
        if _type == 'edit':
            type_value = 'new_value'
        elif _type == 'expected':
            type_value = 'expected_value'
        else:
            type_value = 'new_entries'
        expected_values_per_obj_dict = {}
        try:
            for key,value in input_data.items():
                if key != "unique_keys":
                    expected_values_per_obj_dict[key] = value
            if "unique_keys" in input_data.keys():
                for key, value in input_data['unique_keys'].items():
                    unique_key_item = value
                    if "key" in unique_key_item.keys():
                        unique_key = unique_key_item['key']
                    else:
                        raise JSONParseException("'key' parameter is missing in JSON file!\n")
                    if "key_values" in unique_key_item.keys():
                        unique_key_values = unique_key_item['key_values']
                        for key1, value in unique_key_values.items():
                            unique_key_values_item = value
                            if "value" in unique_key_values_item.keys():
                                unique_key_value = unique_key_values_item['value']
                            else:
                                raise JSONParseException("'value' parameter is missing in JSON file!\n")
                            if 'objects' in unique_key_values_item.keys():
                                unique_key_value_objects = unique_key_values_item['objects']
                                for key, value in unique_key_value_objects.items():
                                    object_path = key
                                    object_item = value
                                    if object_path in expected_values_per_obj_dict.keys():
                                        expected_values_per_obj_dict[object_path]['unique_keys'][key1] = {}
                                        expected_values_per_obj_dict[object_path]['unique_keys'][key1]["paths"] = {}
                                        for key, value in unique_key.items():
                                            expected_values_per_obj_dict[object_path]['unique_keys'][key1]["paths"][value] = {}
                                            expected_values_per_obj_dict[object_path]['unique_keys'][key1]["paths"][value]['key_value'] = unique_key_value[int(key) - 1]
                                            expected_values_per_obj_dict[object_path]['unique_keys'][key1][type_value] = object_item[type_value]
                                    else:
                                        expected_values_per_obj_dict[object_path] = {}
                                        expected_values_per_obj_dict[object_path]['unique_keys'] = {}
                                        expected_values_per_obj_dict[object_path]['unique_keys'][key1] = {}
                                        expected_values_per_obj_dict[object_path]['unique_keys'][key1]["paths"] = {}
                                        for key, value in unique_key.items():
                                            expected_values_per_obj_dict[object_path]['unique_keys'][key1]["paths"][value] = {}
                                            expected_values_per_obj_dict[object_path]['unique_keys'][key1]["paths"][value]['key_value'] = unique_key_value[int(key) - 1]
                                            expected_values_per_obj_dict[object_path]['unique_keys'][key1][type_value] = object_item[type_value]
                                        
                                        
                            else:
                                raise JSONParseException("'objects' parameter is missing in JSON file!\n")
                    else:
                        raise JSONParseException("'key_values' parameter is missing in JSON file!\n")
#             print(expected_values_per_obj_dict)
        except JSONParseException as error:
            sys.stderr.write(str(error))
        except KeyError as error:
            exc_type, exc_obj, exc_tb = sys.exc_info()
            fname = os.path.split(exc_tb.tb_frame.f_code.co_filename)[1]
            sys.stderr.write("Key Error {} in file {} on line {}!\n".format(str(error), str(fname), exc_tb.tb_lineno))
#         print expected_values_per_obj_dict
        return expected_values_per_obj_dict
        
        
class JSONParseException(Exception):
    
    def __init__(self, message):
        super(JSONParseException, self).__init__(message)

    
        
        
        
        