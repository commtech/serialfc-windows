# ChangeLog

## [3.0.0.0](https://github.com/commtech/serialfc-windows/releases/tag/v3.0.0.0) ()
- Removed ClockRate and floating point math from the drivers.
- Added ClockBits for FSCC and PCI family.
- Removed ClockRate from registry, and set default values for ClockBits.
- Updated code to support Windows Universal.
- No longer supports anything before Windows 10.

## [2.4.7](https://github.com/commtech/serialfc-windows/releases/tag/v2.4.6) (12/11/2018)
- Fixed an issue with certain systems booting up and failing to find some serial ports.
- Updated the .inx file to adhere to InfVerifier standards.

## [2.4.6](https://github.com/commtech/serialfc-windows/releases/tag/v2.4.6) (03/14/2014)
- Ignore DTR setting on 422/X-PCIe cards since they aren't allowed.

## [2.4.5](https://github.com/commtech/serialfc-windows/releases/tag/v2.4.5) (02/13/2014)
- Fixed test utility failing on 422/X-PCIe cards
- Fixed 950 trigger level issue causing missing interrupts
- Added better FSCC clock frequency range checks

## [2.4.4](https://github.com/commtech/serialfc-windows/releases/tag/v2.4.4) (01/09/2014)
- Fixed clock frequency IOCTL return code on failure

## [2.4.3](https://github.com/commtech/serialfc-windows/releases/tag/v2.4.3) (12/17/2013)
- Fixed enabling RS485 on the 2nd port of FSCC cards

## [2.4.2](https://github.com/commtech/serialfc-windows/releases/tag/v2.4.2) (11/26/2013)
- Fixed incorrectly setting the clock on FSCC-232 and FSCC green cards
- Improved initialization of trigger levels

## [2.4.1](https://github.com/commtech/serialfc-windows/releases/tag/v2.4.1) (10/25/2013)
- Fixed incorrectly detecting isochronous mode 10

## [2.4.0](https://github.com/commtech/serialfc-windows/releases/tag/v2.4.0) (10/25/2013)
- Added library support for fixed baud rates
- Added isochronous modes 9 and 10
- Prevent getting/setting frame length of FSCC's with older firmware
- Fixed 422/X-PCIe RS485 not being used correctly

## [2.3.0](https://github.com/commtech/serialfc-windows/releases/tag/v2.3.0) (10/11/2013)
- Added an option for setting a fixed baud rate which allows programs that only allow standard baud rates to use a custom value

## [2.2.3](https://github.com/commtech/serialfc-windows/releases/tag/v2.2.3) (10/7/2013)
- Fixed echo cancel and termination being set incorrectly on 422/X-PCIe cards

## [2.2.2](https://github.com/commtech/serialfc-windows/releases/tag/v2.2.2) (9/20/2013)
- Updated qserialfc to
[v1.2.2](https://github.com/commtech/qserialfc/releases/tag/v1.2.2)
- Reinitialize the card correctly after going to sleep

## [2.2.1](https://github.com/commtech/serialfc-windows/releases/tag/v2.2.1) (9/6/2013)
- Improved baud rate precision on 422/X-PCIe cards

## [2.2.0](https://github.com/commtech/serialfc-windows/releases/tag/v2.2.0) (8/12/2013)
- Add 9-bit support

## [2.1.6](https://github.com/commtech/serialfc-windows/releases/tag/v2.1.6) (7/23/2013)
- Fixed a bug that prevented original FSCC devices (0x0f) from being detected correctly.
- Fixed a bug where RS485 wasn't being fully setup on FSCC devices.
- Added remaining get/set functions for the trigger levels.
- Added support for getting the RS485 value.
- Fixed Python bugs where disable_external_transmit and
- disable_isocchronous were failing.

## [2.1.5](https://github.com/commtech/serialfc-windows/releases/tag/v2.1.5) (6/21/2013)
- Added support for configuring frame length (variable data length between start and stop bits)
- Added support for external transmit mode on the FSCC UARTs
- Fixed a bug that occasionally prevented rs485 from being setup correctly on 335 cards

## [2.1.4](https://github.com/commtech/serialfc-windows/releases/tag/v2.1.4) (5/8/2013)
- Added echo cancellation support to the FSCC line
- Fixed a bug that prevented disabling isochronous mode while using the libraries
- Fixed a bug that prevented the use of built in COM ports
- Automatically enable FSTDTR when opening the COM port

## [2.1.3](https://github.com/commtech/serialfc-windows/releases/tag/v2.1.3) (4/10/2013)
- Fixed a bug that allowed users read trigger level values in the PCI and PCIe async cards where the register is write only
- Added initial support for memory-mapped FSCC firmware

## [2.1.2](https://github.com/commtech/serialfc-windows/releases/tag/v2.1.2) (3/13/2013)
- Added a Port to HANDLE conversion operator in the C++ operator
- Added standard ports GUI to allow for changing COM port numbers
- Added VISA example code
- Added C example code

## [2.1.1](https://github.com/commtech/serialfc-windows/releases/tag/v2.1.1) (2/25/2013)
- Added FSCC isochronous mode
- Fixed a bug where the baud rate wasn't being set correctly on the 422/X-PCIe cards
- Added library support for setting the clock rate
- Fixed a bug where some settings were stored incorrectly in the registry

## [2.1.0](https://github.com/commtech/serialfc-windows/releases/tag/v2.1.0) (2/1/2013)
- Manually change clock frequency
- Fixed changeing the sample rate and trigger levels from the python library
- Fixed setting the clock rate on the 335 and FSCC cards
- Fixed detecting of which card is present
- Added more intuitive IOCTL error messages

## [2.0.0](https://github.com/commtech/serialfc-windows/releases/tag/v2.0.0) (1/11/2013)
This is the initial release of the 2.0.X driver series.
