## Installation Instructions
### Downloading Source Code
Most users will install from the binaries and won't require building the source 
code. If this is you, you can skip ahead to the section on loading the driver.

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

## Operating Instructions
The serialfc driver is a slightly modified version of the Windows serial driver
with support for the extra features of the Fastcom asychronous cards. Since
it is based on the standard Windows serial driver you get to leverage to full
suite of options available in the Windows [Serial Communication]
(http://msdn.microsoft.com/en-us/library/ff802693.aspx) API.

### Setting Baud Rate
The Fastcom cards include a variable clock generator that can be used to create
nearly any baud rate you would want. 

To get a non-standard baud rate there are a couple variables you need to setup
before you can use the Windows [DCB structure]
(http://msdn.microsoft.com/en-us/library/windows/desktop/aa363214(v=vs.85).aspx) 
to specify the baud rate.

First in the variable clock generator frequency and second is the variable
sampling rate. The formula for determining the baud rate is as follows.

```
Baud Rate = Clock Rate / Sampling Rate / Integer Divisor.
```

The 'Integer Divisor' value is determined in the driver and as long as the rest
of the formula allows for an integer divisor it can be ignored.

You can change the card's clock rate using a few different methods.

First is using the base IOCTL and the Windows API.
fscc - 200 - 270,000,000
pci -  6,000,000 - 200,000,000

```c
unsigned rate = 18432000;

DeviceIoControl(h, IOCTL_FASTCOM_SET_CLOCK_RATE, 
                &rate, sizeof(rate), 
                NULL, 0, 
                &temp, NULL);
```

There is also a handful of libraries includes that wrap around the Windows
DeviceIoControl call that allow you to ignore the implementation.

```c
serialfc_set_clock_rate(h, 18432000);
```

```c++
port.SetClockRate(18432000);
```

```c#
port.ClockRate = 18432000;
```

```python
port.clock_rate = 18432000;
```


### Sample Rate
pci - 8, 16
pcie - 4, 8, 16
fscc - 4 - 16

```c
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

```c
unsigned rate;

serialfc_set_sample_rate(h, 16);
serialfc_get_sample_rate(h, &rate);
```

```c++
port.SetSampleRate(16);
unsigned rate = port.GetSampleRate();
```

```c#
port.SampleRate = 16;
```

```python
port.sample_rate = 16;
```

### 485
```c
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

```c
BOOL status;

serialfc_enable_rs485(h);
serialfc_disable_sample_rate(h);

serialfc_get_rs485(h, &status);
```

```c++
port.EnableRS485();
port.DisableRS485();

bool status = port.GetRS485();
```

```c#
port.RS485 = True;
```

```python
port.rs485 = True;
```


### Echo Cancel
```c
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

```c
BOOL status;

serialfc_enable_echo_cancel(h);
serialfc_disable_echo_cancel(h);

serialfc_get_echo_cancel(h, &status);
```

```c++
port.EnableEchoCancel();
port.DisableEchoCancel();

bool status = port.GetEchoCancel();
```

```c#
port.EchoCancel = True;
```

```python
port.echo_cancel = True;
```


### Termination
```c
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

```c
BOOL status;

serialfc_enable_termination(h);
serialfc_disable_termination(h);

serialfc_get_termination(h, &status);
```

```c++
port.EnableTermination();
port.DisableTermination();

bool status = port.GetTermination();
```

```c#
port.Termination = True;
```

```python
port.termination = True;
```

### Trigger Levels
```c
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

```c
unsigned level;

serialfc_set_tx_trigger(h, 32);
serialfc_set_rx_trigger(h, 32);

serialfc_get_tx_trigger(h, &level);
serialfc_get_rx_trigger(h, &level);
```

```c++
unsigned level;

port.SetTxTrigger(32);
port.SetRxTrigger(32);

level = port.GetTxTrigger();
level = port.GetRxTrigger();
```

```c#
port.TxTrigger = 32;
```

```python
port.tx_trigger = 32;
port.rx_trigger = 32;
```

### Isochronous

```c
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

```c
unsigned mode;

serialfc_enable_isochronous(h, TODO);
serialfc_disable_isochronous(h);

serialfc_get_isochronousl(h, &mode);
```

```c++
port.EnableIsochronous(TODO);
port.DisableIsochronous();

unsigned mode = port.GetIsochronous();
```

```c#
port.EnableIsochronous(TODO);
port.DisableIsochronous();

int mode = port.GetIsochronous();
```

```python
port.enable_isochronous(TODO)
port.disable_isochrnous()

mode = port.get_isochronous()
```
