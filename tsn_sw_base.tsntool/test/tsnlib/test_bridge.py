import ctypes
from ctypes import *
from util import check_table_default_uint, set_table_uint, check_single_value, set_single_value

class PortAttr(Structure):
    #_packed_ = 1
    _fields_ = [("portExternal", c_uint32),
                ("portType", c_uint32),
                ("portAcceptableFrameTypes", c_uint32),
                ("portEnableIngressFiltering", c_bool),
                ("portTaggingCapable", c_bool),
                ("portConfigurableAcceptableFrameTypes", c_bool),
                ("portIngressFilteringCapable", c_bool),
                ("portTypeCapCustomerVlan", c_bool),
                ("portTypeCapProviderNetwork", c_bool),
                ("portTypeCapCustomerNetwork", c_bool),
                ("portTypeCapCustomerEdge", c_bool),
                ("portTypeCapCustomerBackbone", c_bool),
                ("portTypeCapVirtualInstance", c_bool),
                ("portTypeCapDBridge", c_bool),
                ("portTypeCapRemoteCustomerAccess", c_bool),
                ("portTypeCapStationFacing", c_bool),
                ("portTypeCapUplinkAccess", c_bool),
                ("portTypeCapUplinkRelay", c_bool)]

class BridgeAttr(Structure):
    #_packed_ = 1
    _fields_ = [("BridgeType", c_int),
                ("BridgeNumPorts", c_int),
                ("BridgeComponents", c_bool),
                ("BridgeTrafficClassesEnabled", c_bool),
                ("ComponentId", c_uint16),
                ("ComponentType", c_uint16),
                ("ComponentNumPorts", c_uint16),
                ("FdbSize", c_uint16),
                ("FdbStaticEntries", c_uint16),
                ("ExtendedFilteringServices", c_bool),
                ("TrafficClasses", c_bool),
                ("StaticEntryIndividualPort", c_bool),
                ("IVLCapable", c_bool),
                ("SVLCapable", c_bool),
                ("HybridCapable", c_bool),
                ("ConfigurablePvidTagging", c_bool),
                ("LocalVlanCapable", c_bool)
                ]


class TestBridge(object):
    def setup(self):
        self.libbsd = ctypes.CDLL("/usr/lib/libbsd.so", mode=ctypes.RTLD_GLOBAL)
        self.libtsn = cdll.LoadLibrary("libtsn.so.1.4")

    def test_tsn_bridge_get_traffic_class_table_default(self):
        ''' Check the default TC of each priority'''
        input_values = [0,1,2,3,4,5,6,7]
        expected_values= [1,0,2,3,4,5,6,7]
        check_table_default_uint(self.libtsn.tsn_fqtss_get_tc_table, 'sw0p2', input_values, expected_values)
    
    def test_tsn_bridge_get_traffic_class_table_set(self):
        ''' Check the default TC of each priority'''
        index = [0,1,2,3,4,5,6,7]
        set_values= [7,6,5,4,3,2,1,0]
        default_values = [1,0,2,3,4,5,6,7]
        set_table_uint(self.libtsn.tsn_fqtss_set_tc_table, 'sw0p2', index, set_values, c_uint)
        check_table_default_uint(self.libtsn.tsn_fqtss_get_tc_table, 'sw0p2', index, set_values)
        set_table_uint(self.libtsn.tsn_fqtss_set_tc_table, 'sw0p2', index, default_values, c_uint)

    def test_tsn_bridge_get_prioregen_table_default(self):
        ''' Set the TC table, check and restore '''
        input_values = [0,1,2,3,4,5,6,7]
        expected_values= [0,1,2,3,4,5,6,7]
        check_table_default_uint(self.libtsn.tsn_fqtss_get_prio_regen_table, 'sw0p2', input_values, expected_values)

    def test_tsn_bridge_prioregen_table_set(self):
        ''' Set the priority regeneration table, check and restore '''
        index = [0,1,2,3,4,5,6,7]
        set_values= [7,6,5,4,3,2,1,0]
        default_values = [0,1,2,3,4,5,6,7]
        set_table_uint(self.libtsn.tsn_fqtss_set_prio_regen_table, 'sw0p2', index, set_values, c_uint)
        check_table_default_uint(self.libtsn.tsn_fqtss_get_prio_regen_table, 'sw0p2', index, set_values)
        set_table_uint(self.libtsn.tsn_fqtss_set_prio_regen_table, 'sw0p2', index, default_values, c_uint)

    def test_tsn_bridge_get_num_traffic_classes(self):
        ''' Check that 8 traffic classes are available '''
        check_single_value(self.libtsn.tsn_bridge_get_num_traffic_classes, 'sw0p2', 8, c_uint32())

    def test_tsn_bridge_port_id(self):
        ''' Check that 8 traffic classes are available '''
        check_single_value(self.libtsn.tsn_bridge_port_id, 'sw0p2', 2, c_uint32())

    def test_default_prio(self):
        ''' Check default value, set, recheck and restore default port priority '''
        check_single_value(self.libtsn.tsn_bridge_get_default_prio, 'sw0p2', 0, c_uint32())
        set_single_value(self.libtsn.tsn_bridge_set_default_prio, 'sw0p2', 7, c_int32())
        check_single_value(self.libtsn.tsn_bridge_get_default_prio, 'sw0p2', 7, c_uint32())
        set_single_value(self.libtsn.tsn_bridge_set_default_prio, 'sw0p2', 0, c_int32())
    

"""
    def test_bridge_attr(self):
        bridge_attr = BridgeAttr()
        bridge_hl = self.libtsn.tsn_port_open_by_name(c_char_p('deipce0'))
        error = self.libtsn.tsn_bridge_attr(bridge_hl, byref(bridge_attr))
        self.libtsn.tsn_port_close(bridge_hl)
        assert error == 0
        assert bridge_attr.BridgeType == 3
        assert bridge_attr.BridgeNumPorts == 5
        assert bridge_attr.BridgeTrafficClassesEnabled == True
        assert bridge_attr.ComponentId == 1
        assert bridge_attr.ComponentType == 3
        assert bridge_attr.ComponentNumPorts == 5
        assert bridge_attr.FdbSize == 512
        assert bridge_attr.FdbStaticEntries == 0
        assert bridge_attr.ExtendedFilteringServices == False
        assert bridge_attr.TrafficClasses == True
        assert bridge_attr.StaticEntryIndividualPort == True
        assert bridge_attr.IVLCapable == True
        assert bridge_attr.SVLCapable == True
        assert bridge_attr.HybridCapable == False
        assert bridge_attr.ConfigurablePvidTagging == False
        assert bridge_attr.LocalVlanCapable == False

    def test_port_attr(self):
        port_attr = PortAttr()
        port_hl = self.libtsn.tsn_port_open_by_name(c_char_p('sw0p5'))
        error = self.libtsn.tsn_bridge_port_attr(port_hl, byref(port_attr))
        print(port_attr.portType)
        self.libtsn.tsn_port_close(port_hl)
        assert error == 0
        assert port_attr.portExternal == 1
        assert port_attr.portType == 3
        assert port_attr.portAcceptableFrameTypes == 1
        assert port_attr.portEnableIngressFiltering == False
        assert port_attr.portTaggingCapable == True
        assert port_attr.portConfigurableAcceptableFrameTypes == False
        assert port_attr.portIngressFilteringCapable == False
        assert port_attr.portTypeCapCustomerVlan == True
        assert port_attr.portTypeCapProviderNetwork == False
        assert port_attr.portTypeCapCustomerNetwork == True
        assert port_attr.portTypeCapCustomerEdge == False
        assert port_attr.portTypeCapCustomerBackbone == False
        assert port_attr.portTypeCapVirtualInstance == True
        assert port_attr.portTypeCapDBridge == False
        assert port_attr.portTypeCapRemoteCustomerAccess == False
        assert port_attr.portTypeCapStationFacing == False
        assert port_attr.portTypeCapUplinkAccess == False
        assert port_attr.portTypeCapUplinkRelay == False
"""