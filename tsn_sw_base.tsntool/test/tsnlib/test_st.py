import ctypes
from ctypes import *
from time import sleep, time
from util import check_table_default_uint, set_table_uint, check_single_value, set_single_value, set_and_restore

SGS = c_uint8(0)
SHM = c_uint8(1)
SRM = c_uint8(2)
MAX_ACL = 255

class PtpTime(Structure):
    _fields_ = [("sec", c_uint64),
                ("nsec", c_uint32)]

class Rational(Structure):
    _fields_ = [("numerator", c_uint32),
                ("denominator", c_uint32)]

class GateEntry(Structure):
    _packed_ = 1
    _fields_ = [("interval", c_uint32),
                ("op", c_uint8),
                ("gsv", c_uint8),
                ("padding", c_uint16)]

admin_list = (GateEntry * 4)()
admin_list[0].interval = c_uint32(25000)
admin_list[1].interval = c_uint32(25000)
admin_list[2].interval = c_uint32(25000)
admin_list[3].interval = c_uint32(25000)
admin_list[0].op = SGS
admin_list[1].op = SGS
admin_list[2].op = SGS
admin_list[3].op = SGS
admin_list[0].gsv = c_uint8(255)
admin_list[1].gsv = c_uint8(3)
admin_list[2].gsv = c_uint8(255)
admin_list[3].gsv = c_uint8(3)

default_list = (GateEntry * 4)()
default_list[0].interval = c_uint32(0)
default_list[1].interval = c_uint32(0)
default_list[2].interval = c_uint32(0)
default_list[3].interval = c_uint32(0)
default_list[0].op = SGS
default_list[1].op = SGS
default_list[2].op = SGS
default_list[3].op = SGS
default_list[0].gsv = c_uint8(0)
default_list[1].gsv = c_uint8(0)
default_list[2].gsv = c_uint8(0)
default_list[3].gsv = c_uint8(0)

class TestST(object):
    def setup(self):
        self.libbsd = ctypes.CDLL("/usr/lib/libbsd.so", mode=ctypes.RTLD_GLOBAL)
        self.libtsn = cdll.LoadLibrary("libtsn.so.1.4")
        self.port = self.libtsn.tsn_port_open_by_name(c_char_p('sw0p3'))

    def clean_acl(self):
        acl_default = (GateEntry * MAX_ACL)()
        for index in range(MAX_ACL):
            acl_default[index].interval = 0
            acl_default[index].op = 0
            acl_default[index].gsv = 0
        error = self.libtsn.tsn_st_set_admin_control_list(self.port, c_uint32(MAX_ACL), pointer(acl_default))
    
    def check_time(self, get_time_function, second, nanosec):
        read_time = PtpTime()
        error = get_time_function(self.port, byref(read_time))
        assert error == 0
        assert read_time.sec == second
        assert read_time.nsec == nanosec

    def set_cycle(self, numerator, denominator):
        current_cycle = Rational()
        current_cycle.numerator = numerator
        current_cycle.denominator = denominator
        error = self.libtsn.tsn_st_set_admin_cycle_time(self.port, byref(current_cycle))
        assert error == 0
    
    def check_cycle(self, get_cycle_function, numerator, denominator):
        read_cycle = Rational()
        error = self.libtsn.tsn_st_get_admin_cycle_time(self.port, byref(read_cycle))
        print('numerator', read_cycle.numerator, 'denominator', read_cycle.denominator)
        assert error == 0
        assert read_cycle.numerator == numerator
        assert read_cycle.denominator == denominator

    def check_list(self, get_list_function, nb_entries, ref_list):
        list_read = (GateEntry * nb_entries)()
        error = get_list_function(self.port, c_uint32(nb_entries), pointer(list_read))
        #assert error == 0
        for index in range(nb_entries):
            assert list_read[index].interval == ref_list[index].interval
            assert list_read[index].op == ref_list[index].op
            assert list_read[index].gsv == ref_list[index].gsv
            
    def test_admin_gate_state(self):
        ''' Check default value, set, recheck and restore default admin gate state'''
        set_and_restore(self.libtsn.tsn_st_get_admin_gate_states, 
                        self.libtsn.tsn_st_set_admin_gate_states, 
                        'sw0p3', 0xFF, c_uint8())
    def test_admin_control_length(self):
        ''' Check default value, set, recheck and restore default admin-control-list-length'''
        set_and_restore(self.libtsn.tsn_st_get_admin_control_list_length, 
                        self.libtsn.tsn_st_set_admin_control_list_length, 
                        'sw0p3', 255, c_uint32())
    def test_cycle_time_extension(self):
        ''' Check default value, set, recheck and restore default cycle_time_extension '''
        set_and_restore(self.libtsn.tsn_st_get_admin_cycle_time_extension, 
                        self.libtsn.tsn_st_set_admin_cycle_time_extension, 
                        'sw0p3', 10000, c_uint32())

    def test_current_time(self):
        current_time = PtpTime()
        error = self.libtsn.tsn_st_get_current_time(self.port, byref(current_time))
        print('second', current_time.sec, 'nsec', current_time.nsec)
        assert error == 0
        assert (current_time.sec == round(time())) or (current_time.sec == round(time())-1)

    def test_admin_base_time_default(self):
        self.check_time(self.libtsn.tsn_st_get_admin_base_time, 0, 0)

    def test_admin_cycle_time_default(self):
        self.check_cycle(self.libtsn.tsn_st_get_admin_cycle_time, 0, 1)

    def test_acl_read_default(self):
        self.check_list(self.libtsn.tsn_st_get_admin_control_list, 4, default_list)

    def test_acl_load(self):
        error = self.libtsn.tsn_st_set_admin_control_list(self.port, c_uint32(4), pointer(admin_list))
        assert error == 0
        self.check_list(self.libtsn.tsn_st_get_admin_control_list, 4, admin_list)
        self.clean_acl()

    def test_acl_load_too_long(self):
        error = self.libtsn.tsn_st_set_admin_control_list(self.port, c_uint32(257), pointer(admin_list))
        assert error == 24<<8 #E_TSN_LIB_ST_OP_ERANGE, cause it will fail while validating the ops as the list is not filled to 257 positions

    def test_admin_cycle_time_set(self):
        self.set_cycle(1, 100)
        self.check_cycle(self.libtsn.tsn_st_get_admin_cycle_time, 1, 100)
        self.set_cycle(0, 1)
        self.check_cycle(self.libtsn.tsn_st_get_admin_cycle_time, 0, 1)

    def test_load_activate_config(self):
        self.set_cycle(1, 100)
        error = self.libtsn.tsn_st_set_admin_control_list(self.port, c_uint32(4), pointer(admin_list))
        assert error == 0
        set_single_value(self.libtsn.tsn_st_set_admin_control_list_length, 'sw0p3', 4, c_uint32())
        base_time = PtpTime()
        base_time.sec = int(round(time()) +1)
        ref_time = base_time.sec   # Avoid overwrite
        error = self.libtsn.tsn_st_set_admin_base_time(self.port, byref(base_time))
        set_single_value(self.libtsn.tsn_st_set_admin_cycle_time_extension, 'sw0p3', 0, c_uint32())
        set_single_value(self.libtsn.tsn_st_set_config_change, 'sw0p3', True, c_bool())
        set_single_value(self.libtsn.tsn_st_set_gate_enabled, 'sw0p3', True, c_bool())
        # Check config pending
        check_single_value(self.libtsn.tsn_st_get_config_pending, 'sw0p3', True, c_bool())
        sleep(2)
        check_single_value(self.libtsn.tsn_st_get_config_pending, 'sw0p3', False, c_bool())
        check_single_value(self.libtsn.tsn_st_get_oper_cycle_time_extension, 'sw0p3', 0, c_uint32())
        #check_single_value(self.libtsn.tsn_st_get_oper_gate_states, 'sw0p3', 0, c_uint8())
        self.check_time(self.libtsn.tsn_st_get_config_change_time, ref_time, 0)
        self.check_time(self.libtsn.tsn_st_get_admin_base_time, ref_time, 0)
        self.check_time(self.libtsn.tsn_st_get_oper_base_time, ref_time, 0)
        self.check_list(self.libtsn.tsn_st_get_oper_control_list, 4, admin_list)
        self.check_cycle(self.libtsn.tsn_st_get_oper_cycle_time, 1, 100)
        # Stop and clean
        set_single_value(self.libtsn.tsn_st_set_gate_enabled, 'sw0p3', False, c_bool())
        set_single_value(self.libtsn.tsn_st_set_admin_control_list_length, 'sw0p3', 0, c_uint32())
        self.set_cycle(0, 1)
        base_time = PtpTime()
        base_time.sec = 0
        base_time.nsec = 0
        error = self.libtsn.tsn_st_set_admin_base_time(self.port, byref(base_time))
        self.clean_acl()

        
 #   def test_acl_load_null(self):
 #       error = self.libtsn.tsn_st_set_admin_control_list(self.port, c_uint32(4), None)
 #       assert error == -1
        
