env = Environment()
env.Command('bms_faults.dbc', 'bms_faults.json', 'python3 fault_json_to_dbc.py ../../../BMS/bms_faults.json ../../../BMS/bms_faults.dbc BMS', chdir='../Libraries/can_tools/fault_management')
env.Command(['inc/can_pack.h', 'src/can_pack.c'], 'bms_faults.dbc', 'python3 dbc_to_c.py ../../../BMS/bms_faults.dbc BMS ../../../BMS/inc/can_pack.h ../../../BMS/src/can_pack.c', chdir='../Libraries/can_tools/message_packing')
env.Command(['inc/fault_manager.h', 'src/fault_manager.c'], 'bms_faults.json', 'python3 fault_json_to_c.py ../../../BMS/bms_faults.json ../../../BMS/inc/fault_manager.h ../../../BMS/src/fault_manager.c BMS', chdir='../Libraries/can_tools/fault_management')
