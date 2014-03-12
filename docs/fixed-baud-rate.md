# Fixed Baud Rate

###### Code Support
| Code | Version |
| ---- | ------- |
| serialfc-windows | 2.3.0 |

###### Card Support
| Card Family | Supported |
| ----------- |:-----:|
| FSCC (16C950) | Yes |
| Async-335 (17D15X) | Yes |
| Async-PCIe (17V35X) | Yes |


## Get
```c
IOCTL_FASTCOM_GET_FIXED_BAUD_RATE
```

###### Examples
```c
#include <serialfc.h>
...

int rate;

DeviceIoControl(h, IOCTL_FASTCOM_GET_FIXED_BAUD_RATE,
                NULL, 0,
                &rate, sizeof(rate),
                &temp, NULL);
```


## Enable
```c
IOCTL_FASTCOM_ENABLE_FIXED_BAUD_RATE
```

###### Examples
```c
#include <serialfc.h>
...

unsigned rate = 9600;

DeviceIoControl(h, IOCTL_FASTCOM_ENABLE_FIXED_BAUD_RATE,
                &rate, sizeof(rate),
                NULL, 0,
                &temp, NULL);
```


## Disable
```c
IOCTL_FASTCOM_DISABLE_FIXED_BAUD_RATE
```

###### Examples
```c
#include <serialfc.h>
...

DeviceIoControl(h, IOCTL_FASTCOM_DISABLE_FIXED_BAUD_RATE,
                NULL, 0,
                NULL, 0,
                &temp, NULL);
```


### Additional Resources
- Complete example: [`examples/fixed-baud-rate.c`](../examples/fixed-baud-rate.c)
