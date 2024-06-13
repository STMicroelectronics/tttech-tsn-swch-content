'''
Created on Jan 29, 2019

@author: tijanak
'''
# ---------------------------------------------------
 
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