# Frame Length

The frame length specifies the number of bytes that get transmitted between the start and stop bits. The standard asynchronous serial communication protocol uses a frame length of one.

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

## Get
```c
IOCTL_FASTCOM_GET_FRAME_LENGTH
```

| System Error | Value | Cause |
| ------------ | -----:| ----- |
| `ERROR_NOT_SUPPORTED` | 50 (0x32) | Not supported on this family of cards |

###### Examples
```
#include <serialfc.h>
...

unsigned length;

DeviceIoControl(h, IOCTL_FASTCOM_GET_FRAME_LENGTH,
				NULL, 0,
				&length, sizeof(length),
				&temp, NULL);
```


## Set
```c
IOCTL_FASTCOM_SET_FRAME_LENGTH
```

| System Error | Value | Cause |
| ------------ | -----:| ----- |
| `ERROR_NOT_SUPPORTED` | 50 (0x32) | Not supported on this family of cards |
| `ERROR_INVALID_PARAMETER` | 87 (0x57) | Invalid parameter |

###### Examples
```
#include <serialfc.h>
...

unsigned length = 4;

DeviceIoControl(h, IOCTL_FASTCOM_SET_FRAME_LENGTH,
				&length, sizeof(length),
				NULL, 0,
				&temp, NULL);
```


### Additional Resources
- Complete example: [`examples/frame-length.c`](../examples/frame-length.c)
