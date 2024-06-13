from utils import nok_command, ok_command

def test_tsntool_help(command_help):
    ok_command(['tsntool', command_help, 'help'])

def test_tsntool_nok(negative_test):
    nok_command(negative_test['command'],                        
                negative_test['error_message'],
                negative_test['error_code'])
