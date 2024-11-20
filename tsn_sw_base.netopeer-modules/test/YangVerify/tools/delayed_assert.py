# TTTech YangVerify
# Copyright(c) 2019 TTTech Computertechnik AG.
#
# ALL RIGHTS RESERVED.
# Usage of this software, including source code, netlists, documentation,
# is subject to restrictions and conditions of the applicable license
# agreement with TTTech Computertechnik AG or its affiliates.
#
# All trademarks used are the property of their respective owners.
#
# TTTech Computertechnik AG and its affiliates do not assume any liability
# arising out of the application or use of any product described or shown
# herein. TTTech Computertechnik AG and its affiliates reserve the right to
# make changes, at any time, in order to improve reliability, function or
# design.
#
# Contact Information:
# support@4me.tttech-dependablenetworks.com
# TTTech Computertechnik AG, Schoenbrunnerstrasse 7, 1040 Vienna, Austria

 
def expect(expr, msg=None):
    'keeps track of failed expectations'
    if not expr:
        _log_failure(msg)
 
def assert_expectations():
    'raise an assert if there are any failed expectations'
    if _failed_expectations:
        assert False, _report_failures()
 
# ---------------------------------------------------
 
import inspect
import os.path
 
_failed_expectations = []
 
def _log_failure(msg=None):
    (filename, line, funcname, contextlist) =  inspect.stack()[2][1:5]
    filename = os.path.basename(filename)
    context = contextlist[0]
    _failed_expectations.append('file "%s", line %s, in %s()%s\n%s' % 
        (filename, line, funcname, (('\n%s' % msg) if msg else ''), context))
 
def _report_failures():
    global _failed_expectations
    if _failed_expectations:
        (filename, line, funcname) =  inspect.stack()[2][1:4]
        report = [
            '\n\nassert_expectations() called from',
            '"%s" line %s, in %s()\n' % (os.path.basename(filename), line, funcname),
            'Failed Expectations:%s\n' % len(_failed_expectations)]
        for i,failure in enumerate(_failed_expectations, start=1):
            report.append('%d: %s' % (i, failure))
        _failed_expectations = []
    return ('\n'.join(report))