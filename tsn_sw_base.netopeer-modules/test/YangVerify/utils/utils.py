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

from utils.constants import NS_DICT
import re
from xml.etree.ElementTree import tostring, Element
from tools import settings
import itertools
from lxml import etree

def create_xpath(node_name, path_dict, namespace, prefix_map):
    _xpath = '.'
    _namespace = '{' + namespace + '}'
    namespace_aug = None
    separator = '/'
    for item in path_dict:
        for key in item.keys():
            if key == 'module':
                continue
            elif key == 'augment':
                if item[key] is not None:
                    aug_path = item[key]
                    aug_path_list = aug_path.split('/')
                    for _el in aug_path_list:
                        if _el is not None and _el != '':
                            aug_path_items = _el.split(':')
                            for  aug_path_item in aug_path_items:
                                if aug_path_item in prefix_map.keys():
                                    namespace_aug = '{'+prefix_map[aug_path_item]['namespace']+"}"
                            for  aug_path_item in aug_path_items:
                                if aug_path_item not in prefix_map.keys():
                                        _xpath+=separator+""+namespace_aug+""+ aug_path_item
                else:
                    break
            else:
                if(item[key] != None):
                    _xpath+=separator+""+ item[key]
    _xpath+=separator+""+_namespace+""+node_name
    return _xpath

def create_path(node_name, path_dict, prefix_map):
    path = ''
    separator = '/'
    for item in path_dict:
        for key in item.keys():
            if key == 'module':
                continue
            elif key == 'augment':
                if item[key] is not None:
                    aug_path = item[key]
                    aug_path_list = aug_path.split('/')
                    for _el in aug_path_list:
                        if _el is not None and _el != '':
                            aug_path_items = _el.split(':')
                            for  aug_path_item in aug_path_items:
                                if aug_path_item not in prefix_map.keys():
                                    path+=separator+""+ aug_path_item
#                     _xpath+=separator+""+namespace_aug+""+node_name
                else:
                    break
            else:
                if(item[key] != None):
                    path+=separator+""+ item[key]
    path+=separator+""+node_name
    create_filter_from_path(path)
    return path

def create_filter_from_path(path):
    path_segments = path.split('/')
    child_elem = Element(path_segments[-1])
    path_segments.pop()
    path_segments.reverse()
    for index, path_segment in enumerate(path_segments):
        if path_segment != '':
            new_element = Element(path_segment)
            new_element.append(child_elem)
            child_elem = new_element
    root_element = child_elem
    _filter = tostring(root_element).decode()
#     print(_filter)
    return _filter

def create_filter_from_xpath(xpath_dict):
    path_segments = xpath_dict['xpath'].split('/')
    namespace_dict = xpath_dict['namespaces']
    child_elem = create_new_element(path_segments[-1], namespace_dict)
    path_segments.pop()
    path_segments.reverse()
    for index, path_segment in enumerate(path_segments):
        if path_segment != '.':
            if '[' in path_segment:
                # Extract only the path using split
                path =  path_segment.split('[',1)[0]
                new_element = create_new_element(path, namespace_dict)
                children = get_substring_between_str(path_segment,'\[','\]')
                children_list = children.split('and')
                for child in children_list:
                    if child:
                        child = get_substring_between_str(child,'child::','')
                        namespace, new_object = child.split(':')
                        tag_name = new_object.split('[',1)[0]
                        tag_value = get_substring_between_str(child, "\[text\(\)='", "'\]")
                        key_element = Element(tag_name)
                        key_element.text = tag_value
                        key_element.set("xmlns", namespace_dict[namespace])
                        new_element.append(key_element)
                new_element.append(child_elem)                    
            else:
                new_element = create_new_element(path_segment, namespace_dict)
                new_element.append(child_elem)
            # Move up in the filter chain
            child_elem = new_element
            root_element = child_elem
    _filter = tostring(root_element).decode()
    return _filter

def create_new_element(el_string, namespace_dict):
    namespace, object = el_string.split(':')
    new_element = Element(object)
    new_element.set("xmlns", namespace_dict[namespace])
    return new_element

def create_lxml_xpath(xpath):
    result = {}
    prefix_map = settings.prefix_map
    namespaces = {}
    for prefix, namespace_dict in prefix_map.items():
        if namespace_dict['namespace'] in xpath:
            xpath = xpath.replace("{"+namespace_dict['namespace']+"}", prefix+":")
            namespaces[prefix] = namespace_dict['namespace']
    result['namespaces'] = namespaces
    result['xpath'] = xpath
    return result
    
def create_xpath_for_filtering(xpath, unique_key_dict, yang_module):
    result = {}
    key_paths = unique_key_dict["paths"].keys()
    prefix_map = settings.prefix_map
    namespaces = {}
    for prefix, namespace_dict in prefix_map.items():
        if namespace_dict['namespace'] in xpath:
            xpath = xpath.replace("{"+namespace_dict['namespace']+"}", prefix+":")
            namespaces[prefix] = namespace_dict['namespace']
    
    key_path_root_dict = {}
    for key_path in key_paths:
        xpath_filter = ''
        filter = "child::{}[text()='{}']"
        object_by_path = yang_module.get_object_by_path(key_path)
        key_xpath = object_by_path._xpath
        for prefix, namespace_dict in prefix_map.items():
            if namespace_dict['namespace'] in key_xpath:
#                 print prefix
                key_xpath = key_xpath.replace("{"+namespace_dict['namespace']+"}", prefix+":")
        key_xpath_seg = key_xpath.split('/')
        key_xpath_seg_last_two = key_xpath_seg[-2:]
        first = key_xpath_seg_last_two[0]
        second = key_xpath_seg_last_two[1]
        xpath_filter += filter.format(second, unique_key_dict["paths"][key_path]['key_value'])
        key_path_root = ''
        for item in itertools.islice(key_xpath_seg, 0, len(key_xpath_seg)-1):
            if item != ".":
                key_path_root += "/" + item
        if key_path_root not in key_path_root_dict.keys():
            key_path_root_dict[key_path_root] = []
            key_path_root_dict[key_path_root].append(xpath_filter)
        else:
            key_path_root_dict[key_path_root].append(xpath_filter)
    for key in key_path_root_dict.keys():
        filters = key_path_root_dict[key]
        temp_filter = ""
        for item in itertools.islice(filters, 0, len(filters)-1):
            temp_filter += item + " and "
        temp_filter += filters[-1]
        root_key_xpath_seg = key.split('/')
        root_key_xpath_seg_last = root_key_xpath_seg[-1]
        new_filter = root_key_xpath_seg_last+ "["+temp_filter+']'
        xpath = xpath.replace("/"+root_key_xpath_seg_last+'/', "/"+new_filter+'/')
    result['namespaces'] = namespaces
    result['xpath'] = xpath
    return result
    
    
def get_filter_by_module_name(yang_module_name):
    if yang_module_name in NS_DICT:
        return NS_DICT[yang_module_name]
    else:
        print("Module does not exists")
        
def get_xpath_for_container(container_path, yang_module_obj):
    container_xpath = None
    for obj in yang_module_obj:
        if obj.path.startswith(container_path):
            obj_xpath = obj._xpath
            container_path_last_el = container_path.split('/')[-1]
            container_xpath = obj_xpath.split("}" + container_path_last_el)[0] +  "}" + container_path_last_el
            break
    return container_xpath


def clean_dict_attr(dict_to_clean):
    for key in dict_to_clean.keys():
        if '@xmlns' in dict_to_clean[key].keys():
            del dict_to_clean[key]['@xmlns']
        if '@xmlns:nc' in dict_to_clean[key].keys():
            del dict_to_clean[key]['@xmlns:nc']
    return dict_to_clean
            
            
def get_substring_between_str(_str, sub_str1, sub_str2):
    result = re.search(sub_str1+'(.*)'+sub_str2, _str)
    return result.group(1)


def remove_tag(xml_element, tag_name):
    tag_elements = xml_element.xpath('.//' + tag_name)
    parent_item = None
    grandparent_item = None
    for item in tag_elements:
        parent_item = item.getparent()
        new_parent = etree.Element(parent_item.tag)
        grandparent_item = parent_item.getparent()
        child_items = item.getchildren()
        parent_item.remove(item)
        for child_item in child_items:
            new_parent.append(child_item)
        grandparent_item.append(new_parent)
    grandparent_item.remove(parent_item)
    return xml_element

    
