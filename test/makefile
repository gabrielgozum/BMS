#=============================================================================
# makefile for unit testing 218e_BMS
#=============================================================================

UNITY_ROOT=../../Libraries/Unity-master
CMOCK_ROOT=../../Libraries/cmock

CLEANUP = rm -f
MKDIR = mkdir -p
TARGET_EXTENSION = .out

CC = gcc
CFLAGS = -std=c99
CFLAGS += -Wall
CFLAGS += -Wextra
CFLAGS += -Wpointer-arith
CFLAGS += -Wcast-align
CFLAGS += -Wwrite-strings
CFLAGS += -Wswitch-default
CFLAGS += -Wunreachable-code
CFLAGS += -Winit-self
CFLAGS += -Wmissing-field-initializers
CFLAGS += -Wno-unknown-pragmas
CFLAGS += -Wstrict-prototypes
CFLAGS += -Wundef
CFLAGS += -Wold-style-definition
CFLAGS += -g3

TARGET_BASE1 = test_dtc
TARGET_BASE2 = test_sdc_control
TARGET_BASE3 = test_bms
TARGET_BASE4 = test_temperature
TARGET_BASE5 = test_cell_asic_mutators
TARGET_BASE7 = test_charger
TARGET_BASE8 = test_current
TARGET_BASE9 = test_lookup
TARGET_BASE10 = test_scheduler
TARGET_BASE11 = test_voltage
TARGET_BASE12 = test_ltc1864
TARGET_BASE13 = test_faults

TARGET1 = $(TARGET_BASE1)$(TARGET_EXTENSION)
TARGET2 = $(TARGET_BASE2)$(TARGET_EXTENSION)
TARGET3 = $(TARGET_BASE3)$(TARGET_EXTENSION)
TARGET4 = $(TARGET_BASE4)$(TARGET_EXTENSION)
TARGET5 = $(TARGET_BASE5)$(TARGET_EXTENSION)
TARGET7 = $(TARGET_BASE7)$(TARGET_EXTENSION)
TARGET8 = $(TARGET_BASE8)$(TARGET_EXTENSION)
TARGET9 = $(TARGET_BASE9)$(TARGET_EXTENSION)
TARGET10 = $(TARGET_BASE10)$(TARGET_EXTENSION)
TARGET11 = $(TARGET_BASE11)$(TARGET_EXTENSION)
TARGET12 = $(TARGET_BASE12)$(TARGET_EXTENSION)
TARGET13 = $(TARGET_BASE13)$(TARGET_EXTENSION)

SRC_FILES1 = $(UNITY_ROOT)/src/unity.c test_dtc.c test_runners/test_dtc_runner.c
SRC_FILES2 = $(UNITY_ROOT)/src/unity.c $(CMOCK_ROOT)/src/cmock.c ../src/sdc_control.c ./mock/Mockdtc.c ../src/bms.c ../src/charger.c ../src/params.c ../src/lookup.c mock/mock_gpio.c test_sdc_control.c test_runners/test_sdc_control_runner.c
SRC_FILES3 = $(UNITY_ROOT)/src/unity.c ../src/bms.c test_bms.c ../src/lookup.c ../src/params.c mock/mock_ltc1864.c mock/mock_eeprom.c test_runners/test_bms_runner.c
SRC_FILES4 = $(UNITY_ROOT)/src/unity.c $(CMOCK_ROOT)/src/cmock.c cell_asic_mutators.c ../src/temperature.c ../src/bms.c ../src/lookup.c ../src/params.c mock/Mockdtc.c mock/mock_ltc1864.c mock/mock_eeprom.c mock/mock_can.c test_temperature.c test_runners/test_temperature_runner.c
SRC_FILES5 = $(UNITY_ROOT)/src/unity.c cell_asic_mutators.c test_cell_asic_mutators.c test_runners/test_cell_asic_mutators_runner.c
SRC_FILES7 = $(UNITY_ROOT)/src/unity.c $(CMOCK_ROOT)/src/cmock.c ../src/bms.c ../src/charger.c ../src/lookup.c ../src/params.c mock/Mockdtc.c mock/Mocksdc_control.c mock/mock_gpio.c test_charger.c test_runners/test_charger_runner.c
SRC_FILES8 = $(UNITY_ROOT)/src/unity.c $(CMOCK_ROOT)/src/cmock.c ../src/bms.c ../src/current.c ../src/lookup.c ../src/params.c test_current.c mock/Mockdtc.c mock/Mocksdc_control.c test_runners/test_current_runner.c
SRC_FILES9 = $(UNITY_ROOT)/src/unity.c ../src/lookup.c test_lookup.c test_runners/test_lookup_runner.c
SRC_FILES10 = $(UNITY_ROOT)/src/unity.c ../src/scheduler.c test_scheduler.c mock/mock_sys.c test_runners/test_scheduler_runner.c
SRC_FILES11 = $(UNITY_ROOT)/src/unity.c $(CMOCK_ROOT)/src/cmock.c ../src/voltage.c test_voltage.c cell_asic_mutators.c ../src/lookup.c ../src/bms.c mock/mock_can.c mock/Mockdtc.c test_runners/test_voltage_runner.c
SRC_FILES11 += mock/mock_eeprom.c mock/mock_ltc1864.c ../src/params.c
SRC_FILES12 = $(UNITY_ROOT)/src/unity.c $(CMOCK_ROOT)/src/cmock.c ../src/ltc1864.c test_ltc1864.c ../src/bms.c ../src/lookup.c ../src/params.c mock/Mockdtc.c mock/Mockspi.c mock/mock_sys.c
SRC_FILES12 += mock/mock_eeprom.c mock/mock_gpio.c test_runners/test_ltc1864_runner.c
SRC_FILES13 = $(UNITY_ROOT)/src/unity.c ../src/faults.c test_faults.c test_runners/test_faults_runner.c

INC_DIRS = -Isrc -I$(UNITY_ROOT)/src -I../inc -I.
INC_DIRS += -I'../../Libraries/TivaWare_C_Series-2.1.4.178/'
INC_DIRS += -I$(CMOCK_ROOT)/src

SYMBOLS= -D PART_TM4C123GH6PM -D UNIT_TESTING

#=============================================================================
# Default Commands
#	Add in each test to the default
#=============================================================================
all: clean default

default: test_dtc test_sdc_control test_bms test_temperature test_cell_asic_mutators test_charger test_current test_lookup test_scheduler test_voltage test_ltc1864 test_faults

#=============================================================================
# Test Builds
#	Add for each test module
#=============================================================================
test_dtc: $(SRC_FILES1)
	$(CC) $(CFLAGS) $(INC_DIRS) $(SYMBOLS) $(SRC_FILES1) -o $(TARGET1)
	- ./$(TARGET1)

test_sdc_control: $(SRC_FILES2)
	$(CC) $(CFLAGS) $(INC_DIRS) $(SYMBOLS) $(SRC_FILES2) -o $(TARGET2)
	- ./$(TARGET2)

test_bms: $(SRC_FILES3)
	$(CC) $(CFLAGS) $(INC_DIRS) $(SYMBOLS) $(SRC_FILES3) -o $(TARGET3)
	- ./$(TARGET3)

test_temperature: $(SRC_FILES4)
	$(CC) $(CFLAGS) $(INC_DIRS) $(SYMBOLS) $(SRC_FILES4) -o $(TARGET4)
	- ./$(TARGET4)

test_cell_asic_mutators: $(SRC_FILES5)
	$(CC) $(CFLAGS) $(INC_DIRS) $(SYMBOLS) $(SRC_FILES5) -o $(TARGET5)
	- ./$(TARGET5)

test_charger: $(SRC_FILES7)
	$(CC) $(CFLAGS) $(INC_DIRS) $(SYMBOLS) $(SRC_FILES7) -o $(TARGET7)
	- ./$(TARGET7)

test_current: $(SRC_FILES8)
	$(CC) $(CFLAGS) $(INC_DIRS) $(SYMBOLS) $(SRC_FILES8) -o $(TARGET8)
	- ./$(TARGET8)

test_lookup: $(SRC_FILES9)
	$(CC) $(CFLAGS) $(INC_DIRS) $(SYMBOLS) $(SRC_FILES9) -o $(TARGET9)
	- ./$(TARGET9)

test_scheduler: $(SRC_FILES10)
	$(CC) $(CFLAGS) $(INC_DIRS) $(SYMBOLS) $(SRC_FILES10) -o $(TARGET10)
	- ./$(TARGET10)

test_voltage: $(SRC_FILES11)
	$(CC) $(CFLAGS) $(INC_DIRS) $(SYMBOLS) $(SRC_FILES11) -o $(TARGET11)
	- ./$(TARGET11)

test_ltc1864: $(SRC_FILES12)
	$(CC) $(CFLAGS) $(INC_DIRS) $(SYMBOLS) $(SRC_FILES12) -o $(TARGET12)
	- ./$(TARGET12)

test_faults: $(SRC_FILES13)
	$(CC) $(CFLAGS) $(INC_DIRS) $(SYMBOLS) $(SRC_FILES13) -o $(TARGET13)
	- ./$(TARGET13)

#=============================================================================
# Test Runners - generates test executable
#	Add for each test module
#=============================================================================
test_runners/test_dtc_runner.c: ../src/dtc.c
	ruby $(UNITY_ROOT)/auto/generate_test_runner.rb test_dtc.c test_runners/test_dtc_runner.c

test_runners/test_sdc_control_runner.c: ../src/sdc_control.c
	ruby $(UNITY_ROOT)/auto/generate_test_runner.rb test_sdc_control.c test_runners/test_sdc_control_runner.c

test_runners/test_bms_runner.c: ../src/bms.c
	ruby $(UNITY_ROOT)/auto/generate_test_runner.rb test_bms.c test_runners/test_bms_runner.c

test_runners/test_temperature_runner.c: ../src/temperature.c
	ruby $(UNITY_ROOT)/auto/generate_test_runner.rb test_temperature.c test_runners/test_temperature_runner.c

test_runners/test_cell_asic_mutators_runner.c: cell_asic_mutators.c
	ruby $(UNITY_ROOT)/auto/generate_test_runner.rb test_cell_asic_mutators.c test_runners/test_cell_asic_mutators_runner.c

test_runners/test_charger_runner.c: ../src/charger.c
	ruby $(UNITY_ROOT)/auto/generate_test_runner.rb test_charger.c test_runners/test_charger_runner.c

test_runners/test_current_runner.c: ../src/current.c
	ruby $(UNITY_ROOT)/auto/generate_test_runner.rb test_current.c test_runners/test_current_runner.c

test_runners/test_lookup_runner.c: ../src/lookup.c
	ruby $(UNITY_ROOT)/auto/generate_test_runner.rb test_lookup.c test_runners/test_lookup_runner.c

test_runners/test_scheduler_runner.c: ../src/scheduler.c
	ruby $(UNITY_ROOT)/auto/generate_test_runner.rb test_scheduler.c test_runners/test_scheduler_runner.c

test_runners/test_voltage_runner.c: ../src/voltage.c
	ruby $(UNITY_ROOT)/auto/generate_test_runner.rb test_voltage.c test_runners/test_voltage_runner.c

test_runners/test_ltc1864_runner.c: ../src/ltc1864.c
	ruby $(UNITY_ROOT)/auto/generate_test_runner.rb test_ltc1864.c test_runners/test_ltc1864_runner.c

test_runners/test_faults_runner.c: ../src/faults.c
	ruby $(UNITY_ROOT)/auto/generate_test_runner.rb test_faults.c test_runners/test_faults_runner.c

#=============================================================================
# Cleanup - deletes executables
#	Add for each test module
#=============================================================================
clean:
	$(CLEANUP) $(TARGET1) $(TARGET2) $(TARGET3) $(TARGET4) $(TARGET5) $(TARGET6) $(TARGET7) $(TARGET8) $(TARGET9) $(TARGET10) $(TARGET11) $(TARGET12) $(TARGET13) test_runners/*
