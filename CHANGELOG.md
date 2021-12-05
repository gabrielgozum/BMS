# Battery Master System Changelog

## [2.0.0]
### Added
- Updates for 77s4p pack
- New NTC temperature readings

## [1.4.0]
### Added
 - Cell Level OCV Calculation
 - Cell Level Capacity Calculation (Amp hour integration)
 - Cell Level SOC

### Fixed
 - Scalings on CAN Resistance signals

## [1.3.3]
### Fixed
 - SOC calculation while car is operating

## [1.3.2]
### Added
 - Pack I Offset Debug Message

### Changed
 - Frequency of Cell Voltage Broadcast

## [1.3.1]
###Fixed
 - Current offset calculation
 - The current variable used throughout the program
 - DCL and CCL first order calculation

## [1.3.0]
###Changed
 - Functions that calculate statistics
 - Moved calculating current from ltc1864.c to current.c

###Fixed
 - Transfering of data from the bms_ic struct to the bms struct
 - Ignore Voltage measurements while taking temperature measurements

###Removed
 - Sending cyclic DTC messages
 - Balancing for now

## [1.2.0] - MISSED
Meant to be for competition, but didn't get done.
Major changes coming so skip to 1.3.0

## [1.1.0] - 20180605
### Added
 - override the cell temperature for cell 15 and 26

### Changed
 - number of BSBs to the correct amount

### Fixed
 - Failing Unit Tests

### Removed
 - Reduced the extra time waited for a charger message to enter charging mode
 - correct the voltage/current charge limit to be commanded to the charger to one that works with the charger
 - charger checksum and counter as these features don't work on the charger
 - temperature workaround for the incorrectly installed taps

## [1.0.1] - 20180505
### Changed
 - number of BSBs to 2 because only 2 are installed
 - CAN speed to 500k

## [1.0.0] - 20180425
### Added
 - Everything
