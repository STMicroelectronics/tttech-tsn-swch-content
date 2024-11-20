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

from ncclient import manager
from ncclient.operations.rpc import RPCError
import time
from lxml import etree


class NetconfService:

    def __init__(self, mng=None, host="192.168.0.1", username="root", password="", port=830):
        self.mng = mng
        self.host = host
        self.username = username
        self.password = password
        self.port = port

    def create_connection_ssh(self): #add exception
        if self.mng is None:
            self.mng = manager.connect(host=self.host,
                                       port=self.port,
                                       username=self.username,
                                       password=self.password,
                                       hostkey_verify=False,
                                       allow_agent=True,
                                       timeout=10)
            self.mng.async_mode = True
            self.mng.raise_mode = 2
            self.mng.timeout = 9
        else:
            print("Connection already exists")

    def close_connection_ssh(self): 
        ''' Check if connection is closed '''
        self.mng.close_session()

    def get_data(self, _filter):
        data = self.mng.get(filter=('subtree', _filter))
        while not data.event.is_set():
            time.sleep(.1)      
        return data.reply.data_xml.encode("utf-8")

    def get_data_etree(self, _filter):
        data = self.mng.get(filter=('subtree', _filter))
        while not data.event.is_set():
            time.sleep(.1)      
        return etree.fromstring(data.reply.data_xml.encode("utf-8"))
    
    def get_config_data(self, target, _filter):
        try:
            config_data = self.mng.get_config(target, filter=('subtree', _filter))
            while not config_data.event.is_set():
                time.sleep(.1)      
            return etree.fromstring(config_data.reply.data_xml.encode("utf-8"))
        except RPCError:
            print(config_data.reply.errors)

    def get_config_data_all(self, target):
        try:
            config_data = self.mng.get_config(target)
            while not config_data.event.is_set():
                time.sleep(.1)      
            return etree.fromstring(config_data.reply.data_xml.encode("utf-8"))
        except RPCError:
            print(config_data.reply.errors)

    def edit_config_data(self, target, config_data, operation):
        ''' Edit the configuration data using the operation given as parameter '''
        edited_config_data = self.mng.edit_config(target=target, config=config_data, default_operation=operation)
#         print "Finished edit config"
        time_out = 200
        while (not edited_config_data.event.is_set()) and (time_out > 0):
            time.sleep(1)
            time_out = time_out - 1
        if time_out == 0:
            edited_config_data = None
        if edited_config_data.reply.error is not None:
            raise RPCError(edited_config_data.reply.errors)
        return edited_config_data
        