from utils import nok_command, ok_command, ok_command_stdout

def test_tsntool_qbridge_show():
    ''' Check the result of qbridge show '''
    expected_stdout = {}
    expected_stdout = ["Bridge 'br0':",
  					   "Maximum VLAN Id: 4094",
   					   "Used VLANs:      1 (out of 64)"]
    value = ok_command_stdout(['tsntool', 'qbridge', 'show', 'br0'], expected_stdout)
 
def test_tsntool_qbridge_show_vid_1():
    ''' Check the result of qbridge show '''
    expected_stdout = {}
    expected_stdout = ["Allocated FID: 0)"]
    value = ok_command_stdout(['tsntool', 'qbridge', 'vid', 'show', '1', 'br0'], expected_stdout)

def test_tsntool_qbridge_show_fid_0():
    ''' Check the result of qbridge show '''
    expected_stdout = {}
    expected_stdout = ["FID\t \t allocated VIDs",
                       "0\t \t 1"]
    value = ok_command_stdout(['tsntool', 'qbridge', 'fid', 'show', '0', 'br0'], expected_stdout)

def test_tsntool_qbridge_show_all():
    ''' Check the result of qbridge show '''
    expected_stdout = {}
    expected_stdout = ["FID\t \t allocated VIDs",
                       "0\t \t 1"]
    value = ok_command_stdout(['tsntool', 'qbridge', 'fid', 'showall', 'br0'], expected_stdout)

def test_tsntool_mstp_mstilist_show():
    ''' Check the result of mstp mstilist show '''
    expected_stdout = {}
    expected_stdout = ["Allocated MSTIDs list: 0 4094"]
    value = ok_command_stdout(['tsntool', 'mstp', 'mstilist', 'show', 'br0'], expected_stdout)

def test_tsntool_mstp_fid2mstid_show():
    ''' Check the result of mstp mstilist show '''
    expected_stdout = {}
    expected_stdout = ["FID 0 (allocated to MSTID 0)"]
    value = ok_command_stdout(['tsntool', 'mstp', 'fid2mstid', 'show', '0', 'br0'], expected_stdout)

def test_tsntool_mstp_portstate_show_blocking():
    ''' Check the result of portstate for blocking port '''
    expected_stdout = {}
    expected_stdout = ["Port state is blocking for MSTID 0"]
    value = ok_command_stdout(['tsntool', 'mstp', 'portstate', 'show', '0', 'sw0p2'], expected_stdout)

def test_tsntool_mstp_portstate_show_forwarding():
    ''' Check the result of mstp portstate for blocking port '''
    expected_stdout = {}
    expected_stdout = ["Port state is forwarding for MSTID 0"]
    value = ok_command_stdout(['tsntool', 'mstp', 'portstate', 'show', '0', 'sw0p3'], expected_stdout)