Used python version: 2.7
Used IDE: Eclipse

***********************************

Running tests:

Position yourself inside YangVerify folder and execute following command:

python main.py -t [target-name]

Generate JUnit XML report:

python main.py -t [target-name] -r /path/to/report.xml

For running the tests additional setup needs to be done. In configuration file for target, under ./input/[target-name]/[target-name]_configuration.json, it is necessary to specify what tests will be executed by adding key words. These specifications can be added to an arrays that are assigned to yang module name. 

Following tests can be executed:

	- for testing expected data (key word: "expected")
	- for testing if objects from the MIB are implemented and if data from the board have correct data type (key word: "implemented")

If all arrays are empty no tests will be executed

***********************************
Description of software architecture and created python modules:

input package - Contains all input files (MIB modules that needs to be parsed)

tests package - all test cases are placed here. Test cases will be distributed by modules that are named by appropriate MIB

tools package - all tools that will be used by test functions.
		
	- delayed_assert.py - used to catch exception and allow some tests to fail.
	
	- input_service.py - used for parsing and loading the test cases 
	
utils package - all utility functions and constants that will be used. 


