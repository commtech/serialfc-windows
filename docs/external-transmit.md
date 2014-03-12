# External Transmit

###### Code Support
| Code | Version |
| ---- | ------- |
| serialfc-windows | 2.1.5 |

###### Card Support
| Card Family | Supported |
| ----------- |:-----:|
| FSCC (16C950) | Yes |
| Async-335 (17D15X) | No |
| Async-PCIe (17V35X) | No |

###### Operating Range
| Card Family | Range |
| ----------- | ----- |
| FSCC (16C950) | 0 - 8191 |


## Get
```c
IOCTL_FASTCOM_GET_EXTERNAL_TRANSMIT
```

| System Error | Value | Cause |
| ------------ | -----:| ----- |
| `ERROR_NOT_SUPPORTED` | 50 (0x32) | Not supported on this family of cards |

###### Examples
```c
#include <serialfc.h>
...

unsigned num_chars;

DeviceIoControl(h, IOCTL_FASTCOM_GET_EXTERNAL_TRANSMIT,
                NULL, 0,
                &num_chars, sizeof(num_chars),
                &temp, NULL);
```


## Enable
```c
IOCTL_FASTCOM_ENABLE_EXTERNAL_TRANSMIT
```

| System Error | Value | Cause |
| ------------ | -----:| ----- |
| `ERROR_NOT_SUPPORTED` | 50 (0x32) | Not supported on this family of cards |
| `ERROR_INVALID_PARAMETER` | 87 (0x57) | Invalid parameter |

###### Examples
```c
#include <serialfc.h>
...

unsigned num_chars = 4;

DeviceIoControl(h, IOCTL_FASTCOM_ENABLE_EXTERNAL_TRANSMIT,
                &num_chars, sizeof(num_chars),
                NULL, 0,
                &temp, NULL);
```


## Disable
```c
IOCTL_FASTCOM_DISABLE_EXTERNAL_TRANSMIT
```

| System Error | Value | Cause |
| ------------ | -----:| ----- |
| `ERROR_NOT_SUPPORTED` | 50 (0x32) | Not supported on this family of cards |

###### Examples
```c
#include <serialfc.h>
...

DeviceIoControl(h, IOCTL_FASTCOM_DISABLE_EXTERNAL_TRANSMIT,
                NULL, 0,
                NULL, 0,
                &temp, NULL);
```


### Additional Resources
- Complete example: [`examples/external-transmit.c`](../examples/external-transmit.c)
