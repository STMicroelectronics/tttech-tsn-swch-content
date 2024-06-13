'''
Created on Jan 10, 2019

@author: tijanak
'''
from pysnmp.hlapi import *
import time


class SNMPService:

    def __init__(self, mng=None, host="192.168.0.1", username="snmp_access", password="",port="161"):
        self.mng = mng
        self.host = host
        self.username = username
        self.password = password
        self.port = port

    def create_connection(self): #add exception
        if self.mng is None:
            self.mng = getCmd(SnmpEngine(),
                          UsmUserData(self.username,self.password),
                          UdpTransportTarget((self.host, self.port)), ContextData())
        else:
            print("Connection already exists")

    def get_data(self, object):
        errorIndication, errorStatus, errorIndex, varBinds = self.mng.send(filter=(object))                                                        
        return varBinds

        