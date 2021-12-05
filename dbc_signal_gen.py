

for cell_i in range(0, 60):
    print(' SG_ BMS_Cell{}SOE m{} : 48|8@1+ (0.5,0) [0|0] "" Vector__XXX'.format(cell_i, cell_i))
    print(' SG_ BMS_Cell{}SOC m{} : 40|8@1+ (0.5,0) [0|0] "" Vector__XXX'.format(cell_i, cell_i))
    print(' SG_ BMS_Cell{}Energy m{} : 24|16@1+ (0.1,0) [0|0] "Watt*hr" Vector__XXX'.format(cell_i, cell_i))
    print(' SG_ BMS_Cell{}Capacity m{} : 8|16@1+ (0.1,0) [0|0] "Amp*hr" Vector__XXX'.format(cell_i, cell_i))
    print(' SG_ BMS_Cell{}BalancingActive m{} : 63|1@1+ (1,0) [0|0] "" Vector__XXX'.format(cell_i, cell_i))
