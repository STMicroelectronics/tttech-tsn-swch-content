Test suite for tsntool

The tests expect CE01 to be not connected and CE02 to be connected to a Qbv / Qbu device.

To execute the tests:
- make sure that pytest is installed on the target (if the target is connected
    to the network: pip install pytest)
- copy the complete directory tsntool to the target
- connect to the target and change to the directory where the test suite is located
- run 'pytest -v tsntool'

Negative tests:
The definition for negative tests is done in the file negative-tests.json.
To add a new test case, just edit the json file. 
Each negative test case follows the same scheme:
{
        "comment": " Slope out of range (too high)",
        "error_code": 255,
        "error_message": "Cannot set admin idle slope - 22 (Invalid argument)",
        "command": [
            "tsntool",
            "fqtss",
            "slope",
            "7",
            "10000000000",
            "CE01"
        ]
    }
    comment: Short description of the test case
    error_code: expected error code for the failing command
    error_message: error message expected for command. Limited to one line.
    command: The command for the test, each parameter is given between quote
    
Functional test:
When using the function ok_command, the test will check that the command returns
 nothing in stderr and return the error_code 0. Any deviation will fail the test.
 
Test suite for libtsn

To execute the tests:
- make sure that pytest is installed on the target (if the target is connected
    to the network: pip install pytest)
- copy the complete directory test to the target
- connect to the target and change to the directory where the test suite is located
- run 'pytest -v tsnlib'