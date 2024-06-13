import ctypes
from ctypes import *
from util import check_table_default_uint, set_table_uint

class TestFqtss(object):
    def setup(self):
        self.libbsd = ctypes.CDLL("/usr/lib/libbsd.so", mode=ctypes.RTLD_GLOBAL)
        self.libtsn = cdll.LoadLibrary("libtsn.so.1.4")
        self.port = self.libtsn.tsn_port_open_by_name(c_char_p('sw0p2'))

    def test_admin_slope_table_set_ok(self):
        ''' Set, read-back and restore all admin slope '''
        index = [0,1,2,3,4,5,6,7]
        set_values = [10000, 20000, 30000, 50000, 100000, 500000, 1000000, 1500000]
        expected_values = [24414, 24414, 48828, 73242, 122070, 512695, 1000976, 1513671]
        set_table_uint(self.libtsn.tsn_fqtss_set_admin_slope_table, 'sw0p2', index, set_values, c_uint64)
        check_table_default_uint(self.libtsn.tsn_fqtss_get_admin_slope_table, 'sw0p2', index, expected_values)
        check_table_default_uint(self.libtsn.tsn_fqtss_get_oper_slope_table, 'sw0p2', index, expected_values)
        set_values= [250000000,250000000,250000000,250000000,250000000,250000000,250000000,250000000]
        set_table_uint(self.libtsn.tsn_fqtss_set_admin_slope_table, 'sw0p2', index, set_values, c_uint64)

    def test_admin_slope_table_default(self):
        ''' Check the default values of AdminSlope '''
        input_values = [0,1,2,3,4,5,6, 7]
        expected_values= [250000000,250000000,250000000,250000000,250000000,250000000,250000000,250000000]
        check_table_default_uint(self.libtsn.tsn_fqtss_get_admin_slope_table, 'sw0p2', input_values, expected_values)

    def test_algo_table_default(self):
        ''' Check the default values of transmission selection algorithm '''
        index = [0,1,2,3,4,5,6,7]
        expected_values= [0,0,0,0,0,0,0,0]
        check_table_default_uint(self.libtsn.tsn_fqtss_get_tx_selection_table, 'sw0p2', index, expected_values)

    def test_algo_table_set(self):
        ''' Set, read-back and restore all transmission selection algorithm '''
        index = [0,1,2,3,4,5,6,7]
        set_values = [1,1,1,1,1,1,1,1]
        expected_values= [0,0,0,0,0,0,0,0]
        set_table_uint(self.libtsn.tsn_fqtss_set_tx_selection_table, 'sw0p2', index, set_values, c_uint64)
        check_table_default_uint(self.libtsn.tsn_fqtss_get_tx_selection_table, 'sw0p2', index, set_values)
        set_table_uint(self.libtsn.tsn_fqtss_set_tx_selection_table, 'sw0p2', index, expected_values, c_uint64)

    def __del__(self):
        self.libtsn.tsn_port_close(self.port)
