from utils import ok_command

def test_tsntool_edge_show(tsn_port):
    value = ok_command(['tsntool', 'edge', 'show', tsn_port])
    assert value.splitlines()[1].lstrip() == "Management Traffic Class: : 1"
    assert value.splitlines()[2].lstrip() == "Cut-through                 OFF"
    assert value.splitlines()[3].lstrip() == "Port-Mirroring              OFF"
