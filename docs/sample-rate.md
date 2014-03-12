# Sample Rate

###### Code Support
| Code | Version |
| ---- | ------- |
| serialfc-windows | 2.0.0 |

###### Card Support
| Card Family | Supported |
| ----------- |:-----:|
| FSCC (16C950) | Yes |
| Async-335 (17D15X) | Yes |
| Async-PCIe (17V35X) | Yes |

###### Operating Range
| Card Family | Range |
| ----------- | ----- |
| FSCC (16C950) | 4 - 16 |
| Async-335 (17D15X) | 8, 16 |
| Async-PCIe (17V35X) | 4, 8, 16 |


## Get
```c
IOCTL_FASTCOM_GET_SAMPLE_RATE
```

###### Examples
```
#include <serialfc.h>
...

unsigned rate;

DeviceIoControl(h, IOCTL_FASTCOM_GET_SAMPLE_RATE,
				NULL, 0,
				&rate, sizeof(rate),
				&temp, NULL);
```


## Set
```c
IOCTL_FASTCOM_SET_SAMPLE_RATE
```

| System Error | Value | Cause |
| ------------ | -----:| ----- |
| `ERROR_INVALID_PARAMETER` | 87 (0x57) | Invalid parameter |

###### Examples
```
#include <serialfc.h>
...

unsigned rate = 16;

DeviceIoControl(h, IOCTL_FASTCOM_SET_SAMPLE_RATE,
				&rate, sizeof(rate),
				NULL, 0,
				&temp, NULL);
```


### Additional Resources
- Complete example: [`examples/sample-rate.c`](../examples/sample-rate.c)
