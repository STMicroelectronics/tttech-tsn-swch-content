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

from model.yang_data import YangObject
from utils.utils import create_xpath, create_path, get_substring_between_str 
from collections import OrderedDict
import tools.settings as settings
from utils.constants import OBJ_NOT_FOUND_YANG_ERR

class YangModule:
    
    def __init__(self, yang_module):
        self.name = yang_module['module']['@name']
        self.module_data = yang_module
        self.state_objects = []
        self.config_objects = []
        self.all_objects = []
        self.all_uses_grouping = {}
        self.grouping_nodes = []
        self.namespace = yang_module['module']['namespace']['@uri']
        self.prefix = yang_module['module']['prefix']['@value']
        self.create_state_config_objects()
        
        
    def create_state_config_objects(self):
        node_data = {
            'path': [],
            'path_namespace': [],
            'is_config': [],
            'key_path': {
                }
        }
        if self.module_data['module']['@name'] == "ieee802-dot1q-bridge":
            self.process_nested_uses(self.module_data)

        self.traverse_leafs(self.module_data, node_data, self.module_data['module']['namespace']['@uri'])
        self.get_all_leafs_from_augments(self.module_data)
        self.get_all_leafs_from_groups()

    def process_nested_uses(self, module):
        for grouping in settings.yang_modules['ieee802-dot1q-types']['module']['grouping']:
            if grouping['@name'] == "base-gate-control-entries":
                list = grouping['list']
        augment_of_gate_control_entry = settings.yang_modules['ieee802-dot1q-psfp']['module']['grouping']['uses']['augment']['leaf']
        for leaf in augment_of_gate_control_entry:
            list['leaf'].append(leaf)
        settings.yang_modules['ieee802-dot1q-psfp']['module']['grouping']['list'] = list

    def get_all_leafs_from_augments(self, yang_module):
        node_data = {
            'path': [],
            'path_namespace': [],
            'is_config': []
        }
        if yang_module['module']['@name'] in settings.module_related_augments.keys():
            for yang_module_ref in settings.module_related_augments[yang_module['module']['@name']].keys():
                paths = settings.module_related_augments[yang_module['module']['@name']][yang_module_ref]
                augments = self.get_augment_nodes_from_module(settings.yang_modules[yang_module_ref]['module'], paths, [])
                for augment in augments:
                    node_data['path'].append({'augment': augment['@target-node']})
                    node_data['path_namespace'].append({'augment': augment['@target-node']})
                    self.traverse_leafs(augment, node_data, settings.yang_modules[yang_module_ref]['module']['namespace']['@uri'], settings.yang_modules[yang_module_ref]['module']['prefix']['@value'])
    
    
    def get_all_leafs_from_groups(self):
        for grouping_node in self.grouping_nodes:
            for grouping_node_name in grouping_node.keys():
                node_data = grouping_node[grouping_node_name]['node-data']
                node = grouping_node[grouping_node_name]['node']
                container_node = node_data['path_namespace'][-1]
                for key, value in container_node.items():
                    container_node_val = value
                if (container_node_val):
                    container_node_namespace = get_substring_between_str(container_node_val, "{", "}")
                    if (container_node_namespace):
                        self.traverse_leafs(node, node_data, container_node_namespace)
    
    def traverse_leafs(self, node, node_data, namespace, prefix=None):
        for key in node.keys():
            if 'leaf' == key:
                if type(node[key]) is list:
                    for item in node[key]:
                        if 'config' in item.keys():
                            if len(node_data['is_config']) > 0:
                                node_data['is_config'].pop()
                            node_data['is_config'].append(item['config']['@value'])  
                        self.create_yang_object(item, node_data, namespace)
                else:
                    if 'config' in node[key].keys():
                        if len(node_data['is_config']) > 0:
                            node_data['is_config'].pop()
                        node_data['is_config'].append(node[key]['config']['@value'])
                    self.create_yang_object(node[key], node_data, namespace)
            if key != 'augment' and key != 'grouping' and key != 'leaf':
                if type(node[key]) is OrderedDict or type(node[key]) is dict:
                    if len(node_data['is_config']) > 0:
                        node_data['is_config'].pop()
                    self.traverse_leafs_util(node[key], key, node_data, namespace, prefix)

                elif type(node[key]) is list:
                    list_data = node[key]
                    for index, item in enumerate(list_data):
                            
                        if type(item) is OrderedDict or type(item) is dict:
                            self.traverse_leafs_util(item, key, node_data, namespace, prefix)

        if len(node_data['path']) > 0:
            node_data['path'].pop()
        if len(node_data['path_namespace']) > 0:
            node_data['path_namespace'].pop()
            
    def traverse_leafs_util(self, subnode, key, node_data, namespace, prefix):
        all_keys = subnode.keys()
        path_element = {}
        path_element_namespace = {}
        path_element[key] = None
        path_element_namespace[key] = None
        node_data['path'].append(path_element)
        node_data['path_namespace'].append(path_element_namespace)
        if '@name' in all_keys and key != 'choice' and key != "case":
            _el = node_data['path'][-1]
            _el_namespace = node_data['path_namespace'][-1]
            _el_namespace[key] = "{}".format("{"+namespace+"}")+""+subnode['@name']
            _el[key] = subnode['@name']
        if 'config' in all_keys:
            node_data['is_config'].append(subnode['config']['@value'])  
        if 'leaf-list' in all_keys:
            if type(subnode['leaf-list']) is list:
                for item in subnode['leaf-list']:
                    if 'config' in item.keys():
                        if len(node_data['is_config']) > 0:
                            node_data['is_config'].pop()
                        node_data['is_config'].append(item['config']['@value'])
                                    
                    self.create_yang_object(item, node_data, namespace)
            else:
                if 'config' in subnode['leaf-list'].keys():
                    if len(node_data['is_config']) > 0:
                        node_data['is_config'].pop()
                    node_data['is_config'].append(subnode['leaf-list']['config']['@value'])
                self.create_yang_object(subnode['leaf-list'], node_data, namespace)
        if 'uses' in all_keys:
            grouping_node = {}
            grouping_node[subnode['uses']['@name']] = {}
    
            grouping_node[subnode['uses']['@name']]['node-data'] = {}
            grouping_node[subnode['uses']['@name']]['node-data']['path'] = []
            grouping_node[subnode['uses']['@name']]['node-data']['path_namespace'] = []
            grouping_node[subnode['uses']['@name']]['node-data']['is_config'] = []
            for path in node_data['path']:
                grouping_node[subnode['uses']['@name']]['node-data']['path'].append(path)
            for path_namespace in node_data['path_namespace']:
                grouping_node[subnode['uses']['@name']]['node-data']['path_namespace'].append(path_namespace)
            for is_cofig in node_data['is_config']:
                grouping_node[subnode['uses']['@name']]['node-data']['is_config'].append(is_cofig)
            if ':' in subnode['uses']['@name']:
                if subnode['uses']['@name'] not in settings.grouping_nodes:
                    name = subnode['uses']['@name'].split(":")[1]
                    for key, value in settings.grouping_nodes.items():
                        if name == value['@name']:
                            grouping_node[subnode['uses']['@name']]['node'] = value
                            break
                else:
                    grouping_node[subnode['uses']['@name']]['node'] = settings.grouping_nodes[subnode['uses']['@name']]
            else:
                if prefix is None:
                    grouping_prefix = self.prefix
                    if namespace =="urn:ieee:std:1588:yang:ieee1588-ptp":
                        grouping_prefix = "ptp1588"
                    grouping_node[subnode['uses']['@name']]['node'] = settings.grouping_nodes[grouping_prefix+":"+subnode['uses']['@name']]
                else:
                    grouping_node[subnode['uses']['@name']]['node'] = settings.grouping_nodes[prefix+":"+subnode['uses']['@name']]
            self.grouping_nodes.append(grouping_node)
        self.traverse_leafs(subnode, node_data, namespace, prefix)
    
    
    def get_augment_nodes_from_module(self, yang_module, paths, augments):
        for key in yang_module.keys():
            if key == 'augment':
                if type(yang_module[key]) is list:
                    for item in yang_module[key]:
                        if item['@target-node'] in paths:
                            augments.append(item)
                else:
                    if yang_module[key]['@target-node'] in paths:
                        augments.append(yang_module[key])
        return augments
    
    
    def create_yang_object(self, node, node_data, namespace):
        _xpath = create_xpath(node['@name'], node_data['path_namespace'], namespace, settings.prefix_map)
        path = create_path(node['@name'], node_data['path'], settings.prefix_map)
        name = node['@name']
        type = node['type']
        if 'false' in node_data['is_config']:
            is_config = False
        else:
            is_config = True
        if is_config:
            self.config_objects.append(YangObject(self, name, type, _xpath, path, is_config))
        else:
            self.state_objects.append(YangObject(self, name, type, _xpath, path, is_config))
    
    def get_state_data(self):
        return self.state_objects
    
    def get_config_data(self):
        return self.config_objects
    
    def get_all_data(self):
        return self.config_objects + self.state_objects
    
    def get_config_object_by_path(self, _path):
        for config_obj in self.config_objects:
            if config_obj.path == _path:
                return config_obj
        return None
    
    def get_object_by_path(self, _path):
        ''' Retrieve object or throws an exception '''
        all_objects = self.config_objects + self.state_objects
        for _object in all_objects:
            if _object.path == _path:
                return _object
        ''' If no object is found, raise an exception '''
        raise TypeError(OBJ_NOT_FOUND_YANG_ERR.format(_path, self.name))
