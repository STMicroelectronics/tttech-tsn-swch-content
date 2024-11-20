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
import subprocess
import xmltodict
from collections import OrderedDict
from model.yang_module import YangModule
from model.yang_data import YangObject
from utils.utils import create_xpath, create_path, get_substring_between_str
from . import settings



class YangService:
    
    yangServiceInstance = None
    
    @classmethod
    def getYangServiceInstance(cls):
        if cls.yangServiceInstance is not None:
            return cls.yangServiceInstance
        else:
            cls.yangServiceInstance = YangService()
            return cls.yangServiceInstance

    def __init__(self, root_folder="../../YANG_modules_repository"):
        self.yang_modules = {}
        self.yang_module_objects = {}
        self.root_folder = root_folder
        self.module_related_augments = {}
        self.module_target_nodes = {}
        self.prefix_map = {}
        self.grouping_nodes = {}
        self.yang_derived_types = {}
        
        self.load_and_traverse_yang_modules()
        self.create_module_related_augments()
        self.init_global_settings()
        self.create_yang_module_objects()

        
    def init_global_settings(self):
        settings.module_related_augments = self.module_related_augments
        settings.yang_derived_types = self.yang_derived_types
        settings.prefix_map = self.prefix_map
        settings.grouping_nodes = self.grouping_nodes
        settings.yang_modules = self.yang_modules
        
    
    def load_and_traverse_yang_modules(self):
        for root, subdir, files in os.walk(self.root_folder):
            for file in files:
                if file.endswith('.yang'):
                    file_path = os.path.join(root, file)
                    yin_module = subprocess.check_output(["pyang", "-f", "yin", "-p", root, file_path], stderr=subprocess.STDOUT)
                    yang_module_dict = xmltodict.parse(yin_module)
                    filename = file.split(".")[0]
                    self.yang_modules[filename] = yang_module_dict
                    prefix = yang_module_dict['module']['prefix']['@value']
                    name = yang_module_dict['module']['@name']
                    if name == 'ieee802-dot1q-types':
                        prefix = 'dot1qtypes'
                    if name == 'ieee1588-ptp':
                        prefix = 'ptp1588'
                    namespace = yang_module_dict['module']['namespace']['@uri']
                    self.prefix_map[prefix] = {}
                    self.prefix_map[prefix]['name'] = name
                    self.prefix_map[prefix]['namespace'] = namespace
                    self.traverse_data(filename, prefix, yang_module_dict)


    def traverse_data(self, module_name, prefix, node):
        for key in node.keys():
            if type(node[key]) is OrderedDict or type(node[key]) is dict:
                if 'typedef' in node[key].keys():
                    self.create_derived_datatype(node[key]['typedef'], prefix)
                if 'grouping' in node[key].keys():
                    self.create_grouping_nodes(node[key]['grouping'], prefix)
                if 'augment' in node[key].keys():
                    self.create_augment_target_nodes_for_modules(module_name, node[key]['augment'])
                self.traverse_data(module_name, prefix, node[key])
            elif type(node[key]) is list:
                list_data = node[key]
                for index, item in enumerate(list_data):
                    if type(item) is OrderedDict or type(item) is dict:
                        if 'typedef' in item.keys():
                            self.create_derived_datatype(item['typedef'], prefix)
                        if 'grouping' in item.keys():
                            self.create_grouping_nodes(node[key]['grouping'], prefix)
                        if 'augment' in item.keys():
                            self.create_augment_target_nodes_for_modules(module_name, item['augment'])
                        self.traverse_data(module_name, prefix, item)
    
    
    def create_augment_target_nodes_for_modules(self, module_name, augment_node):
        if module_name not in self.module_target_nodes:
            self.module_target_nodes[module_name] = []
        if type(augment_node) is list:
            for item in augment_node:
                target_node = item['@target-node']
                self.module_target_nodes[module_name].append(target_node)
        else:
            target_node = augment_node['@target-node']
            self.module_target_nodes[module_name].append(target_node)
                

    
    def create_derived_datatype(self, node, prefix):
        if type(node) is list:
            for item in node:
                self.yang_derived_types[prefix+":"+item['@name']] = item['type']
        else:
            self.yang_derived_types[prefix+":"+node['@name']] = node['type']
    
    def create_grouping_nodes(self, node, prefix):
        if type(node) is list:
            for item in node:
                self.grouping_nodes[prefix+":"+item['@name']] = item
        else:
            self.grouping_nodes[prefix+":"+node['@name']] = node
        
    
    def create_yang_module_objects(self):
        for module_name in self.yang_modules.keys():
            yang_module_object = YangModule(self.yang_modules[module_name])
            self.yang_module_objects[module_name] = yang_module_object
    
    def create_module_related_augments(self):
        for module_name in self.module_target_nodes.keys():
            for path in self.module_target_nodes[module_name]:
                path_segments = path.split('/')
                for path_segment in path_segments:
                    if path_segment != '':
                        prefix = path_segment.split(':')
                        prefix = prefix[0] if 2 == len(prefix) else '' # in case that augment is used to redefine type
                        if '' != prefix:
                            module_name_target = self.prefix_map[prefix]['name']
                            if module_name_target not in self.module_related_augments.keys():
                                self.module_related_augments[module_name_target] = {}
                            if module_name not in self.module_related_augments[module_name_target].keys():
                                self.module_related_augments[module_name_target][module_name] = []
                                self.module_related_augments[module_name_target][module_name].append(path)
                            else:
                                self.module_related_augments[module_name_target][module_name].append(path)
    
    def set_current_yang_module(self, yang_module):
        settings.current_yang_module = yang_module
        
    def get_derived_datatypes(self):
        return self.yang_derived_types
    
    def clean(self):
        self.state_objects = []
        self.config_objects = []
        self.all_leafs = []
        
                    
                