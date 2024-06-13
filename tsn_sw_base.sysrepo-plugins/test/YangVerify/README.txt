Used python version: 3.6
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
	- for testing edit-config operation (key word: "edit")
	- for testing edit-config operation with wrong values (key word: "negative")
	- for testing adding new entries to the lists (key word: "new_entry")
	- for testing use-case (e.g. Qbv, VLANs) (key word: "use-case")
	- for testing negative use-case (e.g. wrong configuration) (key word: "negative-use-case")	
	- for testing if objects from yang modules are implemented and if data from the board have correct data type (key word: "validation")
	- for testing if objects from yang modules are implemented and if data from the board have correct data type using filters for each individual object (key word: "validation_filter")
	

If all arrays are empty no tests will be executed
***********************************
Running the test in docker

Build the image for testing:
docker build -t netconf-test .

Run the test for DE-Eval:
docker run  --env target=IPSOL-DE-Eval-Board -v ${WORKSPACE}/sysrepo-plugins/YANG_modules_repository:/YANG_modules_repository  --rm netconf-test

***********************************
Description of software architecture and created python modules:

input package - Contains all input files (Yang modules that needs to be parsed)

model package - Contains python classes and methods for manipulating data from yang modules (for example in yang_types.py python classes for yang types are created)

tests package - all test cases are placed here. Test cases will be distributed by modules that are named by appropriate yang module.

tools package - all tools that will be used by test functions.
	
	- fixtures.py - fixtures are specific for pytest framework. Contains all preparations that needs to be executed before tests executions. Currently there are two fixtures. First is used for creating netconf connection and second is for loading all yang modules from input directory and adding them to one dictionary.
	
	- netconf_service.py - will be used for all comunication with the board. All netconf operations will be placed here (For now there is only get oparation)

	- validation_service.py - used for validating data from the board with yang data types.
	
	- yang_service.py - used for parsing and traversing yang modules and extracting needed data. Currently there are recursive functions for extracting state objects and augments from all yang modules. Also, creating xpath for leafs is implemented. With this xpath it is possible to access every object in xml after it is returned from the board.

utils package - all utility functions and constants that will be used. For example for creating xpaths and subtree filters from xpath.

***********************************

***Filling Data for Different Types of Tests***


Expected, Edit, New-entries and Negative


To execute following types of tests: expected, edit, new-entries, negative it is necessary to fill the json data in appropriate files. Basic structure of json file can be found in basic_json file.

All json keys containing the description within the brackets are dynamic. The rest are static and always have the same name.

Dynamic json keys description:

	- /correct/path/to/yang/object/or/container - as the name describes, this key should contain correct path to yang object or yang container. Correct path implies '/' at the begining of path and 							      without one at the end.
	- value_type - can be 'expected_value' | 'new_value' | 'new_entry' depending on the type of tests. 'expected_value' is used for expected tests, 'new_value' for edit and negative tests, and 		               'new_entry' for new-entries type.
	- order_number - as the name describes this key contains order number of value (json object) which it represents. This order number must be unique, the numbers and sequences themselves are not 	                  important except under key "key". Under "key" order-number represents order of uniqe key value from array that contains "actual_key_values".

All values in json files are dynamic. Below is a description of their meanings depending of test type:

	- value_to_test - represents actual value/s that should be tested.
		          For expected tests this can contain single value or array of values. Single value is used if single object is tested. Array of values is used if list of values for one object 				  is tested (leaf-list in yang modules).
			  For edit/negative tests it can contain also single or array of values. Array of values can represent list of different values for single object or list of values in which 				  every value belongs to one object that is part of a list.
			  For new-entries test this value represents an array of json objects, where every json object represents one entry that should be added under yang container. This entry which 			  is in form of json object is converted in xml chunk of data and appended under defined container. If new entries also contains lists, these list items should be placed in an 			  array.
	- /correct/path/to/unique/key - as the name describes this value should conatain correct path to yang object that is part of unique key. Correct path implies '/' at the begining of path and 						without one at the end.
	- actual_key_values - array containing the values of the unique key where each value belongs to the unique key with the appropriate ordinal number.


Use-case


Special types of tests are use-case tests. These tests are placed under use-case folder under specific target folder. use-case folder contains folders for every yang module that can be tested. Under this yang module folders, subfolders that represents specific use cases, are placed. Every use case foleders contains json files, where each file represents one use case that needs to be executed. Files are enumerated and in addition to ordinal number name of file contains name of operation that need to be executed. These operations can be edit, new-entries and expected.


Implemented, Implemented_filters


For this type of tests, json files from implemented folder are used. To fill in these files is pretty simple. As top level keys correct paths to yang objects must be used. Every top level key has his value in form of json object. This json objects contains two static keys "read" and "write". This keys can contain "true" or "false" values as an indicator if yang object is readable and/or writable.
	-  
