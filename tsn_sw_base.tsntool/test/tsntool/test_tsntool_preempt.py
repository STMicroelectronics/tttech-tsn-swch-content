
from utils import nok_command, ok_command, ok_command_stdout

def test_tsntool_preempt_show_not_connected():
    ''' Check the result of preempt show without link'''
    expected_stdout = {}
    expected_stdout = ["Preemption active:    NO",
					  "hold/release advance: 0/0 ns",
					  "hold request status:  HOLD",
					  "Preemption Status Table:",
					  "Traffic class   Status",
					  "0              EXPRESS",
					  "1              EXPRESS",
					  "2              EXPRESS",
					  "3              EXPRESS",
					  "4              EXPRESS",
					  "5              EXPRESS",
					  "6              EXPRESS",
					  "7              EXPRESS",
					  "MAC Merge Entity Status:",
					  "aMACMergeSupport:            YES",
					  "aMACMergeStatusVerify:       initial",
					  "aMACMergeEnableTx:           disabled",
					  "aMACMergeVerifyDisableTx:    enabled",
					  "aMACMergeStatusTx:           unknown",
					  "aMACMergeVerifyTime:         10 ms",
					  "aMACMergeAddFragSize:        64B",
					  "MAC Merge Entity Counters",
					  "aMACMergeFrameAssErrorCount: 0",
					  "aMACMergeFrameSmdErrorCount: 0",
					  "aMACMergeFrameAssOkCount:    0",
					  "aMACMergeFragCountRx:        0",
					  "aMACMergeFragCountTx:        0",
					  "aMACMergeHoldCount:          0"]
    value = ok_command_stdout(['tsntool', 'preempt', 'show', 'sw0p2'], expected_stdout)
 
def test_tsntool_preempt_show_connected():
    ''' Check the result of preempt show with link'''
    expected_stdout = {}
    expected_stdout = ["Preemption active:    NO",
					  "hold/release advance: 2568/804 ns",
					  "hold request status:  HOLD",
					  "Preemption Status Table:",
					  "Traffic class   Status",
					  "0              EXPRESS",
					  "1              EXPRESS",
					  "2              EXPRESS",
					  "3              EXPRESS",
					  "4              EXPRESS",
					  "5              EXPRESS",
					  "6              EXPRESS",
					  "7              EXPRESS",
					  "MAC Merge Entity Status:",
					  "aMACMergeSupport:            YES",
					  "aMACMergeStatusVerify:       initial",
					  "aMACMergeEnableTx:           disabled",
					  "aMACMergeVerifyDisableTx:    enabled",
					  "aMACMergeStatusTx:           unknown",
					  "aMACMergeVerifyTime:         10 ms",
					  "aMACMergeAddFragSize:        64B",
					  "MAC Merge Entity Counters",
					  "aMACMergeFrameAssErrorCount: 0",
					  "aMACMergeFrameSmdErrorCount: 0",
					  "aMACMergeFrameAssOkCount:    0",
					  "aMACMergeFragCountRx:        0",
					  "aMACMergeFragCountTx:        0",
					  "aMACMergeHoldCount:          0"]
    value = ok_command_stdout(['tsntool', 'preempt', 'show', 'sw0p3'], expected_stdout)
