from utils import nok_command, ok_command

def test_tsntool_brport_show(tsn_port):
    value = ok_command(['tsntool', 'brport', 'show', tsn_port])
    assert value.splitlines()[1].lstrip() == "Number of traffic classes: 8"
    assert value.splitlines()[2].lstrip() == "Default priority:          0"


def test_tsntool_brport_priotbl(tsn_port):
    ''' Invert the priorities 6 and 7, read back the config '''
    ok_command(['tsntool', 'brport', 'wrpriotbl', '7', '6', tsn_port])
    ok_command(['tsntool', 'brport', 'wrpriotbl', '6', '7', tsn_port])
    value = ok_command(['tsntool', 'brport', 'rdpriotbl', tsn_port])
    assert value.splitlines()[7].lstrip() == "6            7 "
    assert value.splitlines()[8].lstrip() == "7            6 "
    ok_command(['tsntool', 'brport', 'wrpriotbl', '7', '7', tsn_port])
    ok_command(['tsntool', 'brport', 'wrpriotbl', '6', '6', tsn_port])

def test_tsntool_brport_tctbl(tsn_port):
    ''' Invert the priority / TC mapping 6 and 7, read back the config '''
    ok_command(['tsntool', 'brport', 'wrtctbl', '7', '6', tsn_port])
    ok_command(['tsntool', 'brport', 'wrtctbl', '6', '7', tsn_port])
    value = ok_command(['tsntool', 'brport', 'rdtctbl', tsn_port])                      
    assert value.splitlines()[7].lstrip() == "6               7 "
    assert value.splitlines()[8].lstrip() == "7               6 "
    ok_command(['tsntool', 'brport', 'wrtctbl', '7', '7', tsn_port])
    ok_command(['tsntool', 'brport', 'wrtctbl', '6', '6', tsn_port])


