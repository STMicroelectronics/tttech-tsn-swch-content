from utils import nok_command, ok_command, ok_command_stdout
from time import sleep

def test_tsntool_st_wrcl_no_link():
    ''' Check that a configuration is rejected when no link is present '''
    ok_command(['tsntool', 'st', 'wrcl', 'sw0p2', './tsntool/qbv.cfg'])
    expected_stdout = ["sgs  50000  0xFF",
                       "sgs  30000  0xF0",
                       "sgs  20000  0x0F",
                       "sgs  50000  0xFF",
                       "sgs  30000  0xF0",
                       "sgs  20000  0x0F",
                       "sgs  50000  0xFF",
                       "sgs  30000  0xF0",
                       "sgs  20000  0x0F"]

    ok_command_stdout(['tsntool', 'st', 'rdacl', 'sw0p2'], expected_stdout)
    nok_command(['tsntool', 'st', 'configure', '+10.0', '1/100', '1000', 'sw0p2'],"ERROR: Cannot trigger config change - tsnlib reported 10 (Failed writing a single value to driver, errno is reported in lower 8 bits for more information)\nERROR: Cannot trigger config change - driver reported 5 (Unknown link speed)", 5)

def test_tsntool_st_wrcl_link_ok():
    ''' Check that a configuration is accepted when a link is present '''
    ok_command(['tsntool', 'st', 'wrcl', 'sw0p3', './tsntool/qbv.cfg'])
    expected_stdout = ["sgs  50000  0xFF",
                       "sgs  30000  0xF0",
                       "sgs  20000  0x0F",
                       "sgs  50000  0xFF",
                       "sgs  30000  0xF0",
                       "sgs  20000  0x0F",
                       "sgs  50000  0xFF",
                       "sgs  30000  0xF0",
                       "sgs  20000  0x0F"]
    sleep(2)
    ok_command_stdout(['tsntool', 'st', 'rdacl', 'sw0p3'], expected_stdout)
    ok_command(['tsntool', 'st', 'configure', '+1.0', '1/100', '1000', 'sw0p3'])
    sleep(2)
    ok_command_stdout(['tsntool', 'st', 'rdocl', 'sw0p3'], expected_stdout)
    sleep(2)
    ok_command(['tsntool', 'st', 'stop', 'sw0p3'])
    
