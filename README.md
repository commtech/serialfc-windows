# serialfc-windows
This README file is best viewed [online](http://github.com/commtech/serialfc-windows/).

## Installation

### Downloading Driver Package
You will more than likely want to download our pre-built driver package from
the [Commtech website](http://www.commtech-fastcom.com/CommtechSoftware.html).


### Downloading Source Code
If you are installing from the pre-built driver packge you can skip ahead
to the section on loading the driver.

The source code for the Fastcom serial driver is hosted on Github code hosting.
To check out the latest code you will need Git and to run the following in a
terminal.

```
git clone git://github.com/commtech/serialfc-windows.git serialfc
```

NOTE: We prefer you use the above method for downloading the driver source code
      (because it is the easiest way to stay up to date) but you can also get 
      the driver source code from the
      [download page](https://github.com/commtech/serialfc-windows/tags/).

Now that you have the latest code checked out you will more than likely want
to switch to a stable version within the code directory. To do this browse
the various tags for one you would like to switch to. Version v1.0.0 is only
listed here as an example.

```
git tag
git checkout v1.0.0
```

### Compiling Driver
Compiling the driver is relatively simple assuming you have all of the
required dependencies. You will need Windows Driver Kit 7.1.0 at a 
minimum. After assembling all of these things you can build the driver by
simply running the BLD command from within the source code directory.

```
cd serialfc/src/
BLD
```


## Using The Serial Port

The serialfc driver is a slightly modified version of the Windows serial driver
with support for extra features of the Fastcom asychronous cards. Since the
driver is based on the standard Windows serial driver you get to leverage the
full suite of options available in the Windows [Serial Communication]
(http://msdn.microsoft.com/en-us/library/ff802693.aspx) API.

Configuring the Fastcom specific features are documented below but we recommend
studying the Windows 
[Serial Communciation API](http://msdn.microsoft.com/en-us/library/ff802693.aspx) 
for as it will contain the information for 99% of your code.

### Setting Baud Rate
##### Max Supported Speeds
- FSCC Family (16c950): 15 MHz
- Async-335 Family (17D15X): 6.25 MHz
- Async-PCIe Family (17V35X): 25 MHz


The Fastcom cards have their baud rate configured using the standard Windows
[DCB structure]
(http://msdn.microsoft.com/en-us/library/windows/desktop/aa363214.aspx) 
but require some tweaks to achieve non-standard baud rates.

To get a non-standard baud rate there are a couple variables you need to setup
before you can use the DCB structure to specify the baud rate.

First is the variable clock generator frequency and second is the variable
sampling rate. The formula for determining a baud rate is as follows.

```
Baud Rate = Clock Rate / Sampling Rate / Integer Divisor.
```

The 'Integer Divisor' value is determined in the driver and as long as the rest
of the formula allows for an integer divisor it can be ignored.

Here is an example of some values that will work. We would like a baud rate of
1 Mhz so we find a combination of a clock rate of 16 Mhz and a sampling rate of
16 that can be divided by an integer to end up with 1 Mhz. Now if we configure
these two values before using the DCB structure to specify the baud rate we will 
be able to achieve any supported rate we want.

```
1,000,000 = 16,000,000 / 16 / 1
```

If you are using a card from our 335 product line things get a little trickier
if you want to use multiple non-standard rates at the same time. This is due
to the card's clock frequency being a board-wide (not channel by channel) setting.

To simplify calculating a clock frequency in these situations you can use a tool
that computes the least common multiple of your desired rates.

Here is an [example link](http://www.wolframalpha.com/input/?i=lcm%282000000%2C+9600%29+*+8)
that computes the least common multiple of 2 MHz and 9600 with a sampling rate
of 8. This shows that you can use a clock frequency of 48 MHz and come up with
integer divisors for both of those baud rates.

```
lcm(2000000, 9600) * 8 = 48000000
```




All of the Fastcom released features can be configured using the basic Windows
API or by using one of the included libraries (C, C++, .NET, Python).


### Clock Rate

##### Operating Ranges
- FSCC Family (16c950): 200 Hz - 270 MHz
- Async-335 Family (17D15X): 6 Mhz - 180 Mhz
- Async-PCIe Family (17V35X): Not required

###### Windows API
```c
#include <serialfc.h>

...

unsigned rate = 18432000;

DeviceIoControl(h, IOCTL_FASTCOM_SET_CLOCK_RATE, 
                &rate, sizeof(rate), 
                NULL, 0, 
                &temp, NULL);
```

###### C Library
```
#include <serialfc.h>
...

serialfc_set_clock_rate(h, 18432000);
```

###### C++ Library
```cpp
#include <serialfc.hpp>
...

port.SetClockRate(18432000);
```

###### .NET Library
```csharp
using SerialFC;
...

port.ClockRate = 18432000;
```

###### Python Library
```python
import serialfc
...

port.clock_rate = 18432000
```


### Sample Rate

##### Operating Ranges
- FSCC Family (16c950): 4 - 16
- Async-335 Family (17D15X): 8, 16
- Async-PCIe Family (17V35X): 4, 8, 16

###### Windows API
```c
#include <serialfc.h>
...

unsigned rate = 16;

DeviceIoControl(h, IOCTL_FASTCOM_SET_SAMPLE_RATE, 
                &rate, sizeof(rate), 
                NULL, 0, 
                &temp, NULL);

DeviceIoControl(h, IOCTL_FASTCOM_GET_SAMPLE_RATE, 
                NULL, 0, 
                &rate, sizeof(rate), 
                &temp, NULL);
```

###### C Library
```c
#include <serialfc.h>
...

unsigned rate;

serialfc_set_sample_rate(h, 16);
serialfc_get_sample_rate(h, &rate);
```

###### C++ Library
```cpp
#include <serialfc.hpp>
...

port.SetSampleRate(16);

unsigned rate = port.GetSampleRate();
```

###### .NET Library
```csharp
using SerialFC;
...

port.SampleRate = 16;
```

###### Python Library
```python
import serialfc
...

port.sample_rate = 16
```


### 485

###### Windows API
```c
#include <serialfc.h>
...

BOOL status;

DeviceIoControl(h, IOCTL_FASTCOM_ENABLE_RS485, 
                NULL, 0, 
                NULL, 0, 
                &temp, NULL);

DeviceIoControl(h, IOCTL_FASTCOM_DISABLE_RS485, 
                NULL, 0, 
                NULL, 0, 
                &temp, NULL);
				
DeviceIoControl(h, IOCTL_FASTCOM_GET_RS485, 
                NULL, 0, 
                &status, sizeof(status), 
                &temp, NULL);
```

###### C Library
```c
#include <serialfc.h>
...

BOOL status;

serialfc_enable_rs485(h);
serialfc_disable_rs485(h);

serialfc_get_rs485(h, &status);
```

###### C++ Library
```cpp
#include <serialfc.hpp>
...

port.EnableRS485();
port.DisableRS485();

bool status = port.GetRS485();
```

###### .NET Library
```csharp
using SerialFC;
...

port.RS485 = true;
```

###### Python Library
```python
import serialfc
...

port.rs485 = True
```


### Echo Cancel

###### Windows API
```c
#include <serialfc.h>
...

BOOL status;

DeviceIoControl(h, IOCTL_FASTCOM_ENABLE_ECHO_CANCEL, 
                NULL, 0, 
                NULL, 0, 
                &temp, NULL);

DeviceIoControl(h, IOCTL_FASTCOM_DISABLE_ECHO_CANCEL, 
                NULL, 0, 
                NULL, 0, 
                &temp, NULL);
				
DeviceIoControl(h, IOCTL_FASTCOM_GET_ECHO_CANCEL, 
                NULL, 0, 
                &status, sizeof(status), 
                &temp, NULL);
```

###### C Library
```c
#include <serialfc.h>
...

BOOL status;

serialfc_enable_echo_cancel(h);
serialfc_disable_echo_cancel(h);

serialfc_get_echo_cancel(h, &status);
```

###### C++ Library
```cpp
#include <serialfc.hpp>
...

port.EnableEchoCancel();
port.DisableEchoCancel();

bool status = port.GetEchoCancel();
```

###### .NET Library
```csharp
using SerialFC;
...

port.EchoCancel = true;
```

###### Python Library
```python
import serialfc
...

port.echo_cancel = True
```


### Termination

###### Windows API
```c
#include <serialfc.h>

...

BOOL status;

DeviceIoControl(h, IOCTL_FASTCOM_ENABLE_TERMINATION, 
                NULL, 0, 
                NULL, 0, 
                &temp, NULL);

DeviceIoControl(h, IOCTL_FASTCOM_DISABLE_TERMINATION, 
                NULL, 0, 
                NULL, 0, 
                &temp, NULL);
				
DeviceIoControl(h, IOCTL_FASTCOM_GET_TERMINATION, 
                NULL, 0, 
                &status, sizeof(status), 
                &temp, NULL);
```

###### C Library
```c
#include <serialfc.h>
...

BOOL status;

serialfc_enable_termination(h);
serialfc_disable_termination(h);

serialfc_get_termination(h, &status);
```

###### C++ Library
```cpp
#include <serialfc.hpp>
...

port.EnableTermination();
port.DisableTermination();

bool status = port.GetTermination();
```

###### .NET Library
```csharp
using SerialFC;
...

port.Termination = true;
```

###### Python Library
```python
import serialfc
...

port.termination = True
```

### Trigger Levels

###### Windows API
```c
#include <serialfc.h>
...

unsigned level = 32;

DeviceIoControl(h, IOCTL_FASTCOM_SET_TX_TRIGGER, 
				&level, sizeof(level), 
                NULL, 0, 
                &temp, NULL);
				
DeviceIoControl(h, IOCTL_FASTCOM_GET_TX_TRIGGER, 
                NULL, 0, 
				&level, sizeof(level), 
                &temp, NULL);
```

###### C Library
```c
#include <serialfc.h>
...

unsigned level;

serialfc_set_tx_trigger(h, 32);
serialfc_set_rx_trigger(h, 32);

serialfc_get_tx_trigger(h, &level);
serialfc_get_rx_trigger(h, &level);
```

###### C++ Library
```cpp
#include <serialfc.hpp>
...

unsigned level;

port.SetTxTrigger(32);
port.SetRxTrigger(32);

level = port.GetTxTrigger();
level = port.GetRxTrigger();
```

###### .NET Library
```csharp
using SerialFC;
...

port.TxTrigger = 32;
```

###### Python Library
```python
import serialfc
...

port.tx_trigger = 32
port.rx_trigger = 32
```

### Isochronous

###### Windows API
```c
#include <serialfc.h>
...

unsigned mode = TODO;

DeviceIoControl(h, IOCTL_FASTCOM_ENABLE_ISOCHRONOUS, 
                &mode, sizeof(mode), 
                NULL, 0, 
                &temp, NULL);

DeviceIoControl(h, IOCTL_FASTCOM_DISABLE_ISOCHRONOUS, 
                NULL, 0, 
                NULL, 0, 
                &temp, NULL);
				
DeviceIoControl(h, IOCTL_FASTCOM_GET_ISOCHRONOUS, 
                NULL, 0, 
                &mode, sizeof(mode), 
                &temp, NULL);
```

###### C Library
```c
#include <serialfc.h>
...

unsigned mode;

serialfc_enable_isochronous(h, TODO);
serialfc_disable_isochronous(h);

serialfc_get_isochronousl(h, &mode);
```

###### C++ Library
```cpp
#include <serialfc.hpp>
...

port.EnableIsochronous(TODO);
port.DisableIsochronous();

unsigned mode = port.GetIsochronous();
```

###### .NET Library
```csharp
using SerialFC;
...

port.EnableIsochronous(TODO);
port.DisableIsochronous();

int mode = port.GetIsochronous();
```

###### Python Library
```python
import serialfc
...

port.enable_isochronous(TODO)
port.disable_isochrnous()

mode = port.get_isochronous()
```


### 9-Bit Protocol
Enabling 9-Bit protocol has a couple of effects.

- Transmitting with 9-bit protocol enabled automatically sets the 1st byte's 9th bit to MARK, 
  and all remaining bytes's 9th bits to SPACE.
- Receiving with 9-bit protocol enabled will return two bytes per each 9-bits of data. 
  The second of each byte-duo contains the 9th bit.

###### Windows API
```c
#include <serialfc.h>
...

BOOL status;

DeviceIoControl(h, IOCTL_FASTCOM_ENABLE_9BIT, 
                NULL, 0, 
                NULL, 0, 
                &temp, NULL);

DeviceIoControl(h, IOCTL_FASTCOM_DISABLE_9BIT, 
                NULL, 0, 
                NULL, 0, 
                &temp, NULL);
				
DeviceIoControl(h, IOCTL_FASTCOM_GET_9BIT, 
                NULL, 0, 
                &status, sizeof(status), 
                &temp, NULL);
```

###### C Library
```c
#include <serialfc.h>
...

BOOL status;

serialfc_enable_9bit(h);
serialfc_disable_9bit(h);

serialfc_get_9bit(h, &status);
```

###### C++ Library
```cpp
#include <serialfc.hpp>
...

port.Enable9Bit();
port.Disable9Bit();

bool status = port.Get9Bit();
```

###### .NET Library
```csharp
using SerialFC;
...

port.NineBit = true;
```

###### Python Library
```python
import serialfc
...

port.nine_bit = True
```


##### How to change the default boot settings?
There are two locations in the registry where settings can be stored. The first location is
where you will assign the default settings computer wide.

`HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\services\Serial\Parameters`

You will see many different options available in this section that can be changed.

If you would like to change settings on a port-by-port basis you can do so by adding one of the
parameters from the section above to the following key.

`HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Enum\MF\<DEVICE>\<PORT>\Device Parameters`

In addition to the parameters above, you can also set the default clock frequency in the device
specific key by adding a `ClockRate` DWORD. For example, if you want a specific port to default to
20 MHz you would set the value to `20000000`.

##### How do I change the COM port numbering?
1. Open the 'Device Manager'
2. Right click & select 'Properties' on each Commtech COM port
3. Switch to the 'Port Settings' tab
4. Click on the 'Advanced' button
5. Then change the value in the 'COM Port Number' field


## Build Dependencies
- Windows Driver Kit (7.1.0 used internally to support XP)


## Run-time Dependencies
- OS: Windows XP+


## API Compatibility
We follow [Semantic Versioning](http://semver.org/) when creating releases.


## License

Copyright (C) 2014 [Commtech, Inc.](http://commtech-fastcom.com)

Licensed under the [GNU General Public License v3](http://www.gnu.org/licenses/gpl.txt).
