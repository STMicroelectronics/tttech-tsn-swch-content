import pytest
import json
import ctypes
from ctypes import *

libbsd = ctypes.CDLL("/usr/lib/libbsd.so", mode=ctypes.RTLD_GLOBAL)
libtsn = cdll.LoadLibrary("libtsn.so.1.4")

@pytest.fixture(scope="session", autouse=True)
def out_value():
    yield

def pytest_generate_tests(metafunc):

    negative_tests={}
    # Give a null pointer as port handler
    # E_TSN_LIB_PARAMS will be returned -> 17<<8
    function_null = [[libtsn.tsn_fqtss_get_tx_selection_table,17<<8],
                     [libtsn.tsn_fqtss_set_tx_selection_table,17<<8],
                     [libtsn.tsn_fqtss_get_admin_slope_table,17<<8],
                     [libtsn.tsn_fqtss_set_admin_slope_table,17<<8],
                     [libtsn.tsn_fqtss_get_oper_slope_table,17<<8],
                     [libtsn.tsn_fqtss_get_prio_regen_table,17<<8],
                     [libtsn.tsn_fqtss_get_tc_table,17<<8],
                     [libtsn.tsn_fqtss_set_prio_regen_table,17<<8],
                     [libtsn.tsn_fqtss_set_tc_table,17<<8],
                     [libtsn.tsn_bridge_port_attr,17<<8],
                     [libtsn.tsn_bridge_get_num_traffic_classes,17<<8],
                     [libtsn.tsn_bridge_set_default_prio,17<<8],
                     [libtsn.tsn_bridge_get_default_prio,17<<8],
                     [libtsn.tsn_bridge_port_id,17<<8],
                     [libtsn.tsn_bridge_port_attr,17<<8],
                     [libtsn.tsn_bridge_attr,17<<8],
                     [libtsn.tsn_mstp_flush_tree_port,17<<8],
                     [libtsn.tsn_mstp_get_max_msti,17<<8],
                     [libtsn.tsn_mstp_create_msti,17<<8],
                     [libtsn.tsn_mstp_delete_msti,17<<8],
                     [libtsn.tsn_mstp_get_fid_to_msti,17<<8],
                     [libtsn.tsn_mstp_set_fid_to_msti,17<<8],
                     [libtsn.tsn_mstp_get_msti_list,17<<8],
                     [libtsn.tsn_mstp_get_port_state,17<<8],
                     [libtsn.tsn_mstp_set_port_state,17<<8],
                     [libtsn.tsn_preemp_get_status_table,17<<8],
                     [libtsn.tsn_preemp_set_status_table,17<<8],
                     [libtsn.tsn_preemp_get_hold_advance,17<<8],
                     [libtsn.tsn_preemp_get_hold_request,17<<8],
                     [libtsn.tsn_preemp_get_preemption_active,17<<8],
                     [libtsn.tsn_preemp_get_release_advance,17<<8],
                     [libtsn.tsn_preemp_get_support,17<<8],
                     [libtsn.tsn_preemp_get_verify_status,17<<8],
                     [libtsn.tsn_preemp_get_tx_enabled,17<<8],
                     [libtsn.tsn_preemp_set_tx_enable,17<<8],
                     [libtsn.tsn_preemp_get_verify_tx_disabled,17<<8],
                     [libtsn.tsn_preemp_set_verify_tx_disable,17<<8],
                     [libtsn.tsn_preemp_get_status_tx,17<<8],
                     [libtsn.tsn_preemp_get_verify_time,17<<8],
                     [libtsn.tsn_preemp_set_verify_time,17<<8],
                     [libtsn.tsn_preemp_get_frag_size,17<<8],
                     [libtsn.tsn_preemp_set_frag_size,17<<8],
                     [libtsn.tsn_preemp_get_asserr_cnt,17<<8],
                     [libtsn.tsn_preemp_get_smderr_cnt,17<<8],
                     [libtsn.tsn_preemp_get_assok_cnt,17<<8],
                     [libtsn.tsn_preemp_get_fragrx_cnt,17<<8],
                     [libtsn.tsn_preemp_get_fragtx_cnt,17<<8],
                     [libtsn.tsn_preemp_get_hold_cnt,17<<8],
                     [libtsn.tsn_qbridge_get_max_vlan_id, 17<<8],
                     [libtsn.tsn_qbridge_get_max_supported_vlans, 17<<8],
                     [libtsn.tsn_qbridge_get_num_vlans,17<<8],
                     [libtsn.tsn_qbridge_set_vid2fid,17<<8],
                     [libtsn.tsn_qbridge_get_vid2fid,17<<8],
                     [libtsn.tsn_qbridge_get_fid2vid,17<<8],
                     [libtsn.tsn_st_get_max_sdu,17<<8],
                     [libtsn.tsn_st_get_overrun,17<<8],
                     [libtsn.tsn_st_set_max_sdu, 17<<8],
                     [libtsn.tsn_st_get_gate_enabled, 17<<8],
                     [libtsn.tsn_st_set_gate_enabled,17<<8],
                     [libtsn.tsn_st_get_admin_gate_states,17<<8],
                     [libtsn.tsn_st_set_admin_gate_states,17<<8],
                     [libtsn.tsn_st_get_oper_gate_states,17<<8],
                     [libtsn.tsn_st_get_admin_control_list_length,17<<8],
                     [libtsn.tsn_st_set_admin_control_list_length,17<<8],
                     [libtsn.tsn_st_get_oper_control_list_length,17<<8],
                     [libtsn.tsn_st_get_admin_control_list,17<<8],
                     [libtsn.tsn_st_set_admin_control_list,17<<8],
                     [libtsn.tsn_st_get_oper_control_list,17<<8],
                     [libtsn.tsn_st_get_admin_cycle_time,17<<8],
                     [libtsn.tsn_st_set_admin_cycle_time,17<<8],
                     [libtsn.tsn_st_get_oper_cycle_time,17<<8],
                     [libtsn.tsn_st_get_admin_cycle_time_extension,17<<8],
                     [libtsn.tsn_st_set_admin_cycle_time_extension,17<<8],
                     [libtsn.tsn_st_get_oper_cycle_time_extension,17<<8],
                     [libtsn.tsn_st_get_admin_base_time,17<<8],
                     [libtsn.tsn_st_set_admin_base_time,17<<8],
                     [libtsn.tsn_st_get_oper_base_time,17<<8],
                     [libtsn.tsn_st_get_config_change,17<<8],
                     [libtsn.tsn_st_set_config_change,17<<8],
                     [libtsn.tsn_st_get_config_change_time,17<<8],
                     [libtsn.tsn_st_get_tick_granularity,17<<8],
                     [libtsn.tsn_st_get_current_time,17<<8],
                     [libtsn.tsn_st_get_config_pending,17<<8],
                     [libtsn.tsn_st_get_config_change_error,17<<8],
                     [libtsn.tsn_st_get_supported_list_max,17<<8],
                     [libtsn.tsn_edgex_get_cut_through, 17<<8],
                     [libtsn.tsn_edgex_set_cut_through, 17<<8],
                    # [libtsn.tsn_edgex_get_tx_overrun, 17<<8],
                    # [libtsn.tsn_edgex_get_mgmt_tc, 17<<8],
                    # [libtsn.tsn_edgex_set_mgmt_tc, 17<<8],
                     [libtsn.tsn_edgex_set_mirror_portid, 17<<8],
                     [libtsn.tsn_edgex_get_mirror_portid, 17<<8],
                     ]

    # Check the default value returned by a getter function using only port handler as parameter
    functions_single = [[libtsn.tsn_qbridge_get_max_vlan_id, 'br0', 4094, c_uint32()],
                        [libtsn.tsn_qbridge_get_max_supported_vlans, 'br0', 64, c_uint32()],
                        [libtsn.tsn_qbridge_get_num_vlans, 'br0', 1, c_uint32()],
                        [libtsn.tsn_st_get_admin_gate_states, 'sw0p2', 255, c_uint8()],
                        [libtsn.tsn_st_get_admin_control_list_length, 'sw0p2', 0, c_uint32()],
                        [libtsn.tsn_st_get_config_change, 'sw0p2', False, c_bool()],
                        [libtsn.tsn_st_get_config_pending, 'sw0p2', False, c_bool()],
                        [libtsn.tsn_st_get_config_change_error, 'sw0p2', 0, c_int()],
                        [libtsn.tsn_st_get_supported_list_max, 'sw0p2', 255, c_uint32()],
                        [libtsn.tsn_st_set_admin_cycle_time_extension, 'sw0p2',0,c_uint32()],
                        [libtsn.tsn_st_get_tick_granularity, 'sw0p3',100000, c_uint32()],
                        [libtsn.tsn_edgex_get_cut_through, 'sw0p2', False, c_bool()],
                        [libtsn.tsn_edgex_get_mirror_portid, 'sw0p2', -1, c_int()],
                        [libtsn.tsn_preemp_get_verify_time, 'sw0p2', 10, c_uint32()],
                        [libtsn.tsn_preemp_get_frag_size, 'sw0p2', 0, c_uint8()],
                        [libtsn.tsn_preemp_get_asserr_cnt, 'sw0p2', 0, c_uint32()],
                        [libtsn.tsn_preemp_get_smderr_cnt, 'sw0p2', 0, c_uint32()],
                        [libtsn.tsn_preemp_get_assok_cnt, 'sw0p2', 0, c_uint32()],
                        [libtsn.tsn_preemp_get_fragrx_cnt, 'sw0p2', 0, c_uint32()],
                        [libtsn.tsn_preemp_get_fragtx_cnt, 'sw0p2', 0, c_uint32()],
                        [libtsn.tsn_preemp_get_hold_cnt, 'sw0p2', 0, c_uint32()],
                        [libtsn.tsn_mstp_get_max_msti, 'br0', 64, c_int()],
                        ]
    # Check the return code of a getter function when the value is out of range
    functions_set = [[libtsn.tsn_bridge_set_default_prio, 'sw0p2', 65794, c_uint32(8)], #(0<<24|PORT_DFLT_USER_PRIORITY<<16|E_TSN_LIB_NOSUCHFILE<<8|ENOENT)
                     [libtsn.tsn_st_set_config_change, 'sw0p2', 8<<24|15<<16|1<<8|2, c_bool(True)], #TSN_ST<<24|CONFIG_CHANGE<<16|E_TSN_LIB_NOSUCHFILE<<8|ENOENT
                     [libtsn.tsn_mstp_flush_tree_port, 'sw0p2', 4<<24|0<<16|10<<8|22, c_int(67)], #(TSN_MSTP<<24|FLUSH_TREE<<16|E_TSN_LIB_FPRINT<<8|EINVAL)
                     [libtsn.tsn_mstp_create_msti, 'sw0p2', 4<<24|2<<16|1<<8|2, c_int(67)], #(TSN_MSTP<<24|CREATE_MSTI<<16|E_TSN_LIB_NOSUCHFILE<<8|ENOENT )
                     [libtsn.tsn_mstp_delete_msti, 'sw0p2', 4<<24|3<<16|1<<8|2, c_int(67)],
                     [libtsn.tsn_mstp_delete_msti, 'sw0p2', 4<<24|3<<16|1<<8|2, c_int(60)],
                     #[libtsn.tsn_st_get_gate_enabled, 'sw0p2', 2, c_bool(True) ]
                     #[libtsn.tsn_st_set_admin_control_list_length, 'sw0p2',  -1, c_uint32(300)]
                     ]
    # Check the return code of a getter function when the index is out of range
    # 'function_param1_out', 'port', 'index', 'type', 'index_error'
    functions_param1 = [[libtsn.tsn_qbridge_get_vid2fid, 'br0', c_uint32(4097), c_uint32(), 23<<8], #E_TSN_LIB_MAX_VID_ERANGE
                        [libtsn.tsn_mstp_get_fid_to_msti, 'br0', c_uint32(67), c_uint32(), 21<<8], #E_TSN_LIB_MAX_FID_ERANGE
                        [libtsn.tsn_fqtss_get_prio_regen_table, 'sw0p2', c_uint8(8), c_uint8(), 14<<8|90], #(E_TSN_LIB_FRDENTRY<<8|EMSGSIZE) Failed reading entry from list, Additionaly we could have a more specific error code
                        [libtsn.tsn_fqtss_get_tc_table,'sw0p2', c_uint8(8), c_uint8(), 14<<8|90], #(E_TSN_LIB_FRDENTRY<<8|EMSGSIZE) Failed reading entry from list, Additionaly we could have a more specific error code
                        [libtsn.tsn_fqtss_set_tc_table,'sw0p2', c_uint8(8), c_uint8(), 200475], #(0<<24|TRAFFIC_CLASS_TBL<<18|E_TSN_LIB_FWRENTRY<<8|EFBIG) Failed writing entry to list, Additionaly we could have a more specific error code
                        [libtsn.tsn_fqtss_get_tx_selection_table, 'sw0p2', c_uint8(8), c_uint8(), 14<<8|90],
                        [libtsn.tsn_fqtss_get_admin_slope_table, 'sw0p2', c_uint8(8), c_uint32(), 14<<8|90],
                        [libtsn.tsn_fqtss_get_oper_slope_table, 'sw0p2', c_uint8(8), c_uint32(), 14<<8|90],
                        [libtsn.tsn_preemp_get_status_table,'sw0p2', c_uint8(8), c_uint8(), 14<<8|90],
                        [libtsn.tsn_st_get_max_sdu, 'sw0p2', c_uint8(8), c_uint32(), 14<<8|90],
                        [libtsn.tsn_st_get_overrun, 'sw0p2', c_uint8(8), c_uint32(), 14<<8|90]]

    # Check the return code of a setter function when a parameter is out of range
    # 'function_index_set_out', 'port', 'index', 'out_index_value', 'param_error'
    functions_index_first_out =  [[libtsn.tsn_qbridge_set_vid2fid, 'br0', c_uint32(4097), c_uint32(10), 23<<8], #E_TSN_LIB_MAX_VID_ERANGE
                            [libtsn.tsn_mstp_set_fid_to_msti, 'br0', c_uint32(67), c_uint32(10), 21<<8], #E_TSN_LIB_MAX_FID_ERANGE
                            [libtsn.tsn_fqtss_set_prio_regen_table,'sw0p2', c_uint8(8), c_uint8(7), 134939], #TSN_BRIDGE<<24|PRIORITY_REGEN_TBL<<16|E_TSN_LIB_FWRENTRY<<8|EFBIG
                            [libtsn.tsn_fqtss_set_tc_table, 'sw0p2', c_uint8(7), c_uint8(8),200470], #TSN_BRIDGE<<24|TRAFFIC_CLASS_TBL<<16|E_TSN_LIB_FWRENTRY<<8|EMSGSIZE
                            [libtsn.tsn_fqtss_set_tx_selection_table,'sw0p2', c_uint8(8), c_uint8(1), 2<<24|2<<16|15<<8|27], #TSN_FQTSS<<24...|EFBIG
                            [libtsn.tsn_fqtss_set_admin_slope_table, 'sw0p2', c_uint8(8), c_uint32(1000000),2<<24|0<<16|15<<8|27], #TSN_FQTSS<<24...|EFBIG
                            [libtsn.tsn_preemp_set_status_table,'sw0p2', c_uint8(8), c_uint8(0),5<<24|4<<16|15<<8|27], #TSN_PREEMP<<24...|EFBIG
                            [libtsn.tsn_st_set_max_sdu, 'sw0p2', c_uint8(8), c_uint32(1504),8<<24|0<<16|15<<8|27]]  #TSN_ST<<24...|EFBIG

    functions_index_second_out =  [[libtsn.tsn_qbridge_set_vid2fid, 'br0', c_uint32(1), c_uint32(65),  21<<8], #E_TSN_LIB_MAX_FID_ERANGE
                            [libtsn.tsn_mstp_set_fid_to_msti, 'br0', c_uint32(1), c_uint32(67), 4<<24|5<<16|15<<8|22], #TSN_MSTP|...|E_TSN_LIB_FWRENTRY|EINVAL
                            [libtsn.tsn_fqtss_set_prio_regen_table,'sw0p2', c_uint8(7), c_uint8(8), 134934], #PRIORITY_REGEN_TBL...|EINVAL
                            [libtsn.tsn_fqtss_set_tx_selection_table,'sw0p2', c_uint8(7), c_uint8(2), 2<<24|2<<16|15<<8|22], # TSN_FQTSS<<24|TX_SEL_ALGORITHM_TABLE<<16|E_TSN_LIB_FWRENTRY<<8|EINVAL
                            [libtsn.tsn_fqtss_set_admin_slope_table, 'sw0p2', c_uint8(7), c_uint64(1500000000), 2<<24|0<<16|15<<8|22], #TSN_FQTSS<<24|..|EFBIG
                            [libtsn.tsn_preemp_set_status_table, 'sw0p2', c_uint8(7), c_uint8(3), 5<<24|4<<16|15<<8|22],  #TSN_PREEMP<<24...|EFBIG
                            [libtsn.tsn_st_set_max_sdu, 'sw0p2', c_uint8(7), c_uint32(1510), 8<<24|0<<16|15<<8|34]] #TSN_ST|...|ERANGE

       # Try to set a value out of range for TC 7 and check the error code
                         
    
    # For getter function failling on a link status
    functions_nok = [[libtsn.tsn_st_get_tick_granularity, 'sw0p2', 8<<24|17<<16|1<<8|2], #(TSN_ST<<24|TICK_GRANULARITY<<16|E_TSN_LIB_NOSUCHFILE<<8|ENOENT) Maybe we could introduce an specific error here? Like not valid without link
                     [libtsn.tsn_st_get_oper_gate_states, 'sw0p2', 8<<24|4<<16|1<<8|2], #TSN_ST<<24|...|ENOENT
                     [libtsn.tsn_st_get_oper_control_list_length, 'sw0p2', 8<<24|6<<16|9<<8|2], #TSN_ST<<24|...|ENOENT
                     ]
    if 'function_null' in metafunc.fixturenames:
        metafunc.parametrize(('function_null', 'null_pointer_error'), 
                             [(function[0], function[1]) for function in function_null],
                             ids=[(function[0].__name__) for function in function_null],
                             scope='session')


    if 'function_set_out' in metafunc.fixturenames:
        metafunc.parametrize(('function_set_out', 'port', 'set_error', 'out_value'), 
                             [(function[0], function[1], function[2], function[3]) for function in functions_set],
                             ids=[(function[0].__name__) for function in functions_set ],
                             scope='session')

    if 'function_nok' in metafunc.fixturenames:
        metafunc.parametrize(('function_nok', 'port', 'nok_error'), 
                             [(function[0], function[1], function[2]) for function in functions_nok],
                             ids=[(function[0].__name__) for function in functions_nok ],
                             scope='session')

    if 'function_single' in metafunc.fixturenames:
        metafunc.parametrize(('function_single', 'port_single', 'expected_value', 'value_type'), 
                             [(function[0], function[1], function[2], function[3]) for function in functions_single],
                             ids=[(function[0].__name__) for function in functions_single ],
                             scope='session')

    if 'function_param1_out' in metafunc.fixturenames:
        metafunc.parametrize(('function_param1_out', 'port', 'index', 'type', 'index_error'), 
                             [(function[0], function[1], function[2], function[3], function[4]) for function in functions_param1],
                             ids=[(function[0].__name__) for function in functions_param1 ],
                             scope='session')

    if 'function_first_index_set_out' in metafunc.fixturenames:
        metafunc.parametrize(('function_first_index_set_out', 'port1', 'out_index_value1', 'index1', 'param_error1'), 
                             [(function[0], function[1], function[2], function[3], function[4]) for function in functions_index_first_out],
                             ids=[(function[0].__name__) for function in functions_index_first_out ],
                             scope='session')

    if 'function_second_index_set_out' in metafunc.fixturenames:
	metafunc.parametrize(('function_second_index_set_out', 'port2', 'index2', 'out_index_value2', 'param_error2'),
                             [(function[0], function[1], function[2], function[3], function[4]) for function in functions_index_second_out],
                             ids=[(function[0].__name__) for function in functions_index_second_out ],
                             scope='session')
