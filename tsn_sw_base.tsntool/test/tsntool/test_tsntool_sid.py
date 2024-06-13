from utils import nok_command, ok_command, ok_command_stdout

def test_tsntool_sid_showmax():
    ''' Check that the max number of stream is reported '''
    expected_stdout = ["Max supported streams = 128"]
    ok_command_stdout(['tsntool', 'sid', 'showmax', 'br0'], expected_stdout)

#def test_tsntool_sid_create_stream():
#    ''' Create a new stream '''
#    ok_command(['tsntool', 'sid', 'create', '96', 'NULL', '01:10:5e:01:01:05', '1', '3', 'out-fac-in', 'CE01', 'br0'])
#    expected_stdout = ["Stream handle = 96",
#                       "Null Stream Identification",
#                       "OutFacInputPortList = CE01", 
#                       "Identification parameters:",
#                       "MAC = 01:10:5e:01:01:05, Tagged, VLAN ID = 3"]
#    ok_command_stdout(['tsntool', 'sid', 'show', 'br0'], expected_stdout)
#    ok_command_stdout(['tsntool', 'sid', 'show', 'br0', '96'], expected_stdout)
#    ok_command(['tsntool', 'sid', 'delete', '96', 'br0'])
#    ok_command(['tsntool', 'sid', 'show', 'br0'])
#    nok_command(['tsntool', 'sid', 'show', 'br0', '96'],"ERROR: Reading SID database; check Stream Handle value", 255)
#
#def test_tsntool_sid_add_stream():
#    ''' Create a new stream and add a port '''
#    ok_command(['tsntool', 'sid', 'create', '96', 'NULL', '01:10:5e:01:01:05', '1', '3', 'out-fac-in', 'CE01', 'br0'])
#    ok_command(['tsntool', 'sid', 'add', '96', 'out-fac-in', 'CE02', 'br0'])
#    expected_stdout = ["Stream handle = 96",
#                       "Null Stream Identification",
#                       "OutFacInputPortList = CE01 CE02", 
#                       "Identification parameters:",
#                       "MAC = 01:10:5e:01:01:05, Tagged, VLAN ID = 3"]
#   ok_command_stdout(['tsntool', 'sid', 'show', 'br0'], expected_stdout)
#    ok_command_stdout(['tsntool', 'sid', 'show', 'br0', '96'], expected_stdout)
#    ok_command(['tsntool', 'sid', 'delport', '96', 'out-fac-in', 'CE02', 'br0'])
#    expected_stdout = ["Stream handle = 96",
#                       "Null Stream Identification",
#                       "OutFacInputPortList = CE01", 
#                       "Identification parameters:",
#                       "MAC = 01:10:5e:01:01:05, Tagged, VLAN ID = 3"]
#    ok_command(['tsntool', 'sid', 'delete', '96', 'br0'])
#    ok_command(['tsntool', 'sid', 'show', 'br0'])
#    nok_command(['tsntool', 'sid', 'show', 'br0', '96'],"ERROR: Reading SID database; check Stream Handle value", 255)
