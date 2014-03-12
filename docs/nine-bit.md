# 9-Bit Protocol

Enabling 9-Bit protocol has a couple of effects.

- Transmitting with 9-bit protocol enabled automatically sets the 1st byte's 9th bit to MARK, and all remaining bytes's 9th bits to SPACE.
- Receiving with 9-bit protocol enabled will return two bytes per each 9-bits of data. The second of each byte-duo contains the 9th bit.

###### Code Support
| Code | Version |
| ---- | ------- |
| serialfc-windows | 2.2.0 |

###### Card Support
| Card Family | Supported |
| ----------- |:-----:|
| FSCC (16C950) | Yes |
| Async-335 (17D15X) | No |
| Async-PCIe (17V35X) | No |


## Get
```c
IOCTL_FASTCOM_GET_9BIT
```

| System Error | Value | Cause |
| ------------ | -----:| ----- |
| `ERROR_NOT_SUPPORTED` | 50 (0x32) | Not supported on this family of cards |

###### Examples
```c
#include <serialfc.h>
...

unsigned status;

DeviceIoControl(h, IOCTL_FASTCOM_GET_9BIT,
                NULL, 0,
                &status, sizeof(status),
                &temp, NULL);
```


## Enable
```c
IOCTL_FASTCOM_ENABLE_9BIT
```

| System Error | Value | Cause |
| ------------ | -----:| ----- |
| `ERROR_NOT_SUPPORTED` | 50 (0x32) | Not supported on this family of cards |

###### Examples
```c
#include <serialfc.h>
...

DeviceIoControl(h, IOCTL_FASTCOM_ENABLE_9BIT,
                NULL, 0,
                NULL, 0,
                &temp, NULL);
```


## Disable
```c
IOCTL_FASTCOM_DISABLE_9BIT
```

| System Error | Value | Cause |
| ------------ | -----:| ----- |
| `ERROR_NOT_SUPPORTED` | 50 (0x32) | Not supported on this family of cards |

###### Examples
```c
#include <serialfc.h>
...

DeviceIoControl(h, IOCTL_FASTCOM_DISABLE_9BIT,
                NULL, 0,
                NULL, 0,
                &temp, NULL);
```


### Additional Resources
- Complete example: [`examples/nine-bit.c`](../examples/nine-bit.c)
