from utils import nok_command, ok_command


def test_tsntool_fqtss_show(tsn_port):
    ''' Check the result of fqtss show '''
    value = ok_command(['tsntool', 'fqtss', 'show', tsn_port])

    assert value.splitlines()[0].lstrip() == "Traffic Class        Algorithm            operIdleSlope"
    assert value.splitlines()[1].lstrip() == "0              Strict prio          n/a"
    assert value.splitlines()[7].lstrip() == "6              Strict prio          n/a"

def test_tsntool_fqtss_set_ok(tsn_port):
    ''' Set a CBS, check output and restore config '''
    value = ok_command(['tsntool', 'fqtss', 'slope', '7', '500000', tsn_port])
    value = ok_command(['tsntool', 'fqtss', 'tsa', '7', 'cbs', tsn_port])
    value = ok_command(['tsntool', 'fqtss', 'show', tsn_port])
    assert value.splitlines()[8].lstrip() == "7              CBS                  512695 bps"
    value =  ok_command(['tsntool', 'fqtss', 'tsa', '7', 'strict', tsn_port])
