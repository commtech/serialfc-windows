# Clock Rate

###### Code Support
| Code | Version |
| ---- | ------- |
| serialfc-windows | 2.1.0 |

###### Card Support
| Card Family | Supported |
| ----------- |:-----:|
| FSCC (16C950) | Yes |
| Async-335 (17D15X) | Yes |
| Async-PCIe (17V35X) | Not required |


###### Operating Range
| Card Family | Range |
| ----------- | ----- |
| FSCC (16C950) | 200 Hz - 270 MHz |
| Async-335 (17D15X) | 6 Mhz - 180 Mhz |
| Async-PCIe (17V35X) | Not required |


## Set
```c
IOCTL_FASTCOM_SET_CLOCK_RATE
```

| System Error | Value | Cause |
| ------------ | -----:| ----- |
| `ERROR_NOT_SUPPORTED` | 50 (0x32) | Not supported on this family of cards |
| `ERROR_INVALID_PARAMETER` | 87 (0x57) | Invalid parameter |

###### Examples
```
#include <serialfc.h>
...

unsigned rate = 18432000; /* 18.432 MHz */

DeviceIoControl(h, IOCTL_FASTCOM_SET_CLOCK_RATE,
				&rate, sizeof(rate),
				NULL, 0,
				&temp, NULL);
```


### Additional Resources
- Complete example: [`examples/clock-rate.c`](../examples/clock-rate.c)
