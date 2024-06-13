'''
Created on Mar 13, 2019

@author: tijanak
'''
import sys
import os
import pytest
import json
from optparse import OptionParser
from utils.constants import INPUT_TARGET_CONFIG

possible_targets = ['DE-eval', 'DE-eval-jenkins', 'DE-eval-jenkins2','netleap-jenkins']

test_names_map = {
    "implemented": "test_all_data",
    "implemented_filter": "test_all_data_with_filters",
    "edit": "test_edit_config_data",
    "expected": "test_data_with_expected_values",
    "new-entries": "test_add_new_entries",
    "use-case": "test_use_cases"
    }

def create_opt_parser():
    parser = OptionParser(usage="%prog -t TARGET -r PATH/TO/REPORT", version="%prog 0.1")
    parser.add_option("-t",
                      "--target",
                      dest="target",
                      help=("Name of the target to test. "
                            "This is mandatory."
                            "Possible targets: "+str(possible_targets)))
    parser.add_option("-r",
                      "--report",
                      dest="report",
                      help=("Path to JUnit report."))
    return parser

if __name__ == "__main__":
    args = []
    parser = create_opt_parser()
    opts = parser.parse_args()[0]
    if opts.target is None:
        sys.stderr.write("Target is not specified!\n")
    elif opts.target not in possible_targets:
        sys.stderr.write("{} target does not exists!\n".format(opts.target))
    else:
        try:
            args.append("tests")
            if args:
                args.append("--target="+opts.target)
                args.append("-sv")
                if opts.report is not None:
                    args.append("--junitxml="+opts.report)
                pytest.main(args)
            else:
                sys.stderr.write("Please select tests to run for {} target!\n".format(format(opts.target)))
        except IOError:
            sys.stderr.write("{} target does not exists!\n".format(opts.target))
    
    
    
