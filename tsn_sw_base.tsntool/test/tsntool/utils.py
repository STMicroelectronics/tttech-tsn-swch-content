import subprocess

def nok_command(arguments, error_message, error_code):
    value = subprocess.Popen(arguments,
                         stdout= subprocess.PIPE,
                         stderr = subprocess.PIPE,
                         shell = False)
    stdout, stderr = value.communicate()
    print('Command:', arguments)
    print('stdout: ', stdout)
    print("Error message from target: ", stderr.rstrip("\n"), ' from test: ', error_message)
    print("Error code from target:", value.returncode, ' from test: ', error_code)
    assert stderr.rstrip("\n") == error_message
    assert value.returncode == int(error_code)

def ok_command(arguments):
    value = subprocess.Popen(arguments,
                         stdout= subprocess.PIPE,
                         stderr = subprocess.PIPE,
                         shell = False)
    stdout, stderr = value.communicate()
    print('Command:', arguments)
    print('stderr:', stderr)
    print('error_code',value.returncode )
    assert stderr == ''
    assert value.returncode == 0
    return stdout

def ok_command_stdout(arguments, expected_stdout):
    value = subprocess.Popen(arguments,
                         stdout= subprocess.PIPE,
                         stderr = subprocess.PIPE,
                         shell = False)
    stdout, stderr = value.communicate()
    print('stderr:', stderr)
    print('error_code',value.returncode)
    assert stderr == ''
    assert value.returncode == 0
    i = 0
    print('Command:', arguments)
    for line in expected_stdout:
        print('Line: ', i, ' ', stdout.splitlines()[i].lstrip().rstrip(), ' ', line)
        assert stdout.splitlines()[i].lstrip().rstrip() == line
        i = i+1
    return stdout
