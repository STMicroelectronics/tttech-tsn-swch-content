import ctypes
from ctypes import *
from time import sleep
from util import check_table_default_uint, set_table_uint, check_single_value, set_single_value, set_and_restore


class TestPreempt(object):
    def setup(self):
        self.libbsd = ctypes.CDLL("/usr/lib/libbsd.so", mode=ctypes.RTLD_GLOBAL)
        self.libtsn = cdll.LoadLibrary("libtsn.so.1.4")
        self.port = self.libtsn.tsn_port_open_by_name(c_char_p('sw0p3'))

    def activate_preemption(self):
        ''' Activate preemption and disable verification '''
        error = self.libtsn.tsn_preemp_set_verify_tx_disable(self.port, c_bool(True))
        assert error == 0
        error = self.libtsn.tsn_preemp_set_verify_time(self.port, c_uint32(10))
        assert error == 0
        error = self.libtsn.tsn_preemp_set_tx_enable(self.port, c_bool(True))
        assert error == 0

    def desactivate_preemption(self):
        error = self.libtsn.tsn_preemp_set_tx_enable(self.port, c_bool(False))
        assert error == 0

    def test_preempt_status_table_default(self):
        ''' Check the default values of preemption for each traffic class'''
        index = [0,1,2,3,4,5,6,7]
        expected_values= [1,1,1,1,1,1,1,1] # Expect only express TC
        check_table_default_uint(self.libtsn.tsn_preemp_get_status_table, 'sw0p2', index, expected_values)

    def test_preempt_status_table_set(self):
        ''' Set, read-back and restore all preemption status '''
        index = [0,1,2,3,4,5,6,7]
        set_values = [1,1,2,2,2,2,2,2] # Only higher TC are set to avoid losing connectivity
        expected_values= [1,1,1,1,1,1,1,1]
        # The test must use a connected port
        set_table_uint(self.libtsn.tsn_preemp_set_status_table, 'sw0p3', index, set_values, c_uint8)
        self.activate_preemption()
        check_table_default_uint(self.libtsn.tsn_preemp_get_status_table, 'sw0p3', index, set_values)
        set_table_uint(self.libtsn.tsn_preemp_set_status_table, 'sw0p3', index, expected_values, c_uint8)
        self.desactivate_preemption()

    def test_hold_advance(self):
        ''' Check the values for hold/advance on connected and not connected port '''
        self.activate_preemption()
        check_single_value(self.libtsn.tsn_preemp_get_hold_advance, 'sw0p2', 0, c_uint32())
        check_single_value(self.libtsn.tsn_preemp_get_hold_advance, 'sw0p3', 2568, c_uint32())
        check_single_value(self.libtsn.tsn_preemp_get_hold_request, 'sw0p2', 1, c_uint32())
        check_single_value(self.libtsn.tsn_preemp_get_hold_request, 'sw0p3', 1, c_uint32())
        check_single_value(self.libtsn.tsn_preemp_get_release_advance, 'sw0p2', 0, c_uint32())
        check_single_value(self.libtsn.tsn_preemp_get_release_advance, 'sw0p3', 804, c_uint32())
        self.desactivate_preemption()

    def test_preempt_active(self):
        ''' Check the flag for preemption active '''
        check_single_value(self.libtsn.tsn_preemp_get_preemption_active, 'sw0p3', 1, c_uint8())
        self.activate_preemption()
        check_single_value(self.libtsn.tsn_preemp_get_preemption_active, 'sw0p3', 2, c_uint8())
        self.desactivate_preemption()

    def test_get_set_fragsize(self):
        ''' Check default value, set, recheck and restore fragment size'''
        set_and_restore(self.libtsn.tsn_preemp_get_frag_size, 
                        self.libtsn.tsn_preemp_set_frag_size,
                        'sw0p2', 3, c_int8())

    def test_get_set_verifytime(self):
        ''' Check default value, set, recheck and restore default verification time '''
        set_and_restore(self.libtsn.tsn_preemp_get_verify_time, 
                        self.libtsn.tsn_preemp_set_verify_time, 
                        'sw0p2', 127, c_uint32())

    def test_preemption_control(self):
        ''' Check the preemption functions and flags before and after activation '''
        check_single_value(self.libtsn.tsn_preemp_get_support, 'sw0p3', True, c_bool())
        check_single_value(self.libtsn.tsn_preemp_get_verify_status, 'sw0p3', 6, c_uint8())
        check_single_value(self.libtsn.tsn_preemp_get_tx_enabled, 'sw0p3', False, c_bool())
        check_single_value(self.libtsn.tsn_preemp_get_verify_tx_disabled, 'sw0p3', True, c_bool())
        # Activate preemption
        set_single_value(self.libtsn.tsn_preemp_set_verify_tx_disable, 'sw0p3', False, c_bool())
        set_single_value(self.libtsn.tsn_preemp_set_tx_enable, 'sw0p3', True, c_bool())
        check_single_value(self.libtsn.tsn_preemp_get_tx_enabled, 'sw0p3', True, c_bool())
        check_single_value(self.libtsn.tsn_preemp_get_verify_status, 'sw0p3', 2, c_uint8())
        sleep(1)
        check_single_value(self.libtsn.tsn_preemp_get_verify_status, 'sw0p3', 4, c_uint8())
        check_single_value(self.libtsn.tsn_preemp_get_status_tx, 'sw0p3', 1, c_uint8())
        set_single_value(self.libtsn.tsn_preemp_set_tx_enable, 'sw0p3', False, c_bool())
        check_single_value(self.libtsn.tsn_preemp_get_tx_enabled, 'sw0p3', False, c_bool())
        set_single_value(self.libtsn.tsn_preemp_set_verify_tx_disable, 'sw0p3', False, c_bool())
        check_single_value(self.libtsn.tsn_preemp_get_status_tx, 'sw0p3', 1, c_uint8())
