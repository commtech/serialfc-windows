# Echo Cancel

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


## Get
```c
IOCTL_FASTCOM_GET_ECHO_CANCEL
```

###### Examples
```c
#include <serialfc.h>
...

unsigned status;

DeviceIoControl(h, IOCTL_FASTCOM_GET_ECHO_CANCEL,
                NULL, 0,
                &status, sizeof(status),
                &temp, NULL);
```


## Enable
```c
IOCTL_FASTCOM_ENABLE_ECHO_CANCEL
```

###### Examples
```c
#include <serialfc.h>
...

DeviceIoControl(h, IOCTL_FASTCOM_ENABLE_ECHO_CANCEL,
                NULL, 0,
                NULL, 0,
                &temp, NULL);
```


## Disable
```c
IOCTL_FASTCOM_DISABLE_ECHO_CANCEL
```

###### Examples
```c
#include <serialfc.h>
...

DeviceIoControl(h, IOCTL_FASTCOM_DISABLE_ECHO_CANCEL,
                NULL, 0,
                NULL, 0,
                &temp, NULL);
```


### Additional Resources
- Complete example: [`examples/echo-cancel.c`](../examples/echo-cancel.c)
