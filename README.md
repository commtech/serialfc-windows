# serialfc-windows
This README file is best viewed [online](http://github.com/commtech/serialfc-windows/).

## Installing Driver

##### Downloading Driver Package
You can download a pre-built driver package directly from our
[website](http://www.commtech-fastcom.com/CommtechSoftware.html).


## Quick Start Guide
There is documentation for each specific function listed below, but lets get started
with a quick programming example for fun.f
_This tutorial has already been set up for you at_ 
[`serialfc/examples/tutorial.c`](https://github.com/commtech/serialfc-windows/tree/master/examples/tutorial.c).

Create a new C file (named tutorial.c) with the following code.

```c
#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>

int main(void)
{
	HANDLE h = 0;
	DWORD tmp;
	char odata[] = "Hello world!";
	char idata[20];
	
	/* Open port 0 in a blocking IO mode */
	h = CreateFile("\\\\.\\FSCC0", GENERIC_READ | GENERIC_WRITE, 0, NULL, 
	                  OPEN_EXISTING, 0, NULL);

	if (h == INVALID_HANDLE_VALUE) { 
        fprintf(stderr, "CreateFile failed with %d\n", GetLastError());		   
		return EXIT_FAILURE; 
	}
	
	/* Send "Hello world!" text */
	WriteFile(h, odata, sizeof(odata), &tmp, NULL);

	/* Read the data back in (with our loopback connector) */
	ReadFile(h, idata, sizeof(idata), &tmp, NULL);

	fprintf(stdout, "%s\n", idata);
	
	CloseHandle(h);
	
	return EXIT_SUCCESS;
}

```

For this example I will use the Visual Studio command line compiler, but
you can use your compiler of choice.

```
# cl tutorial.c
```

Now attach the included loopback connector.

```
# tutorial.exe
Hello world!
```

You have now transmitted and received data! 


## Using The Serial Port

The SerialFC driver is a slightly modified version of the Windows serial driver
with support for extra features of the Fastcom asynchronous cards. Since the
driver is based on the standard Windows serial driver you get to leverage the
full suite of options available in the Windows [Serial Communication]
(http://msdn.microsoft.com/en-us/library/ff802693.aspx) API.

Configuring the Fastcom specific features are documented below but we recommend
studying the Windows 
[Serial Communciation API](http://msdn.microsoft.com/en-us/library/ff802693.aspx) 
for as it will contain the information for 99% of your code.

### Setting Baud Rate
##### Max Supported Speeds
| Card Family | Baud Rate |
| ----------- | --------- |
| FSCC (16C950) | 15 MHz |
| Async-335 (17D15X) | 6.25 MHz |
| Async-PCIe (17V35X) | 25 MHz |


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

## API Reference

There are likely other configuration options you will need to set up for your 
own program. All of these options are described on their respective documentation page.

- [Connect](https://github.com/commtech/serialfc-windows/blob/master/docs/connect.md)
- [Card Type](https://github.com/commtech/serialfc-windows/blob/master/docs/card-type.md)
- [Clock Rate](https://github.com/commtech/serialfc-windows/blob/master/docs/clock-rate.md)
- [Echo Cancel](https://github.com/commtech/serialfc-windows/blob/master/docs/echo-cancel.md)
- [External Transmit](https://github.com/commtech/serialfc-windows/blob/master/docs/external-transmit.md)
- [Fixed Baud Rate](https://github.com/commtech/serialfc-windows/blob/master/docs/fixed-baud-rate.md)
- [Frame Length](https://github.com/commtech/serialfc-windows/blob/master/docs/frame-length.md)
- [Isochronous](https://github.com/commtech/serialfc-windows/blob/master/docs/isochronous.md)
- [9-Bit Protocol](https://github.com/commtech/serialfc-windows/blob/master/docs/nine_bit.md)
- [RS485](https://github.com/commtech/serialfc-windows/blob/master/docs/rs485.md)
- [RX Trigger](https://github.com/commtech/serialfc-windows/blob/master/docs/rx-trigger.md)
- [Sample Rate](https://github.com/commtech/serialfc-windows/blob/master/docs/sample-rate.md)
- [Termination](https://github.com/commtech/serialfc-windows/blob/master/docs/termination.md)
- [TX Trigger](https://github.com/commtech/serialfc-windows/blob/master/docs/tx-trigger.md)
- [Disconnect](https://github.com/commtech/serialfc-windows/blob/master/docs/disconnect.md)


### FAQ

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
