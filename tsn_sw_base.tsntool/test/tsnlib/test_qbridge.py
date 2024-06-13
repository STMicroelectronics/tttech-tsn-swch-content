import ctypes
from ctypes import *
from time import sleep
from util import check_table_default_uint, set_table_uint, check_single_value_index, set_single_value

class TestQbridge(object):
    def setup(self):
        self.libbsd = ctypes.CDLL("/usr/lib/libbsd.so", mode=ctypes.RTLD_GLOBAL)
        self.libtsn = cdll.LoadLibrary("libtsn.so.1.4")
        self.port = self.libtsn.tsn_port_open_by_name(c_char_p('br0'))


    def test_tsn_qbridge_get_vid2fid(self):
        ''' Check the flag for preemption active '''
        check_single_value_index(self.libtsn.tsn_qbridge_get_vid2fid, 'br0', c_uint32(1), 0, c_uint32())

    def test_msti_create_delete(self):
        ''' Check the flag for preemption active '''
        set_single_value(self.libtsn.tsn_mstp_create_msti, 'br0', 10, c_uint16())
        set_single_value(self.libtsn.tsn_mstp_delete_msti, 'br0', 10, c_uint16())
