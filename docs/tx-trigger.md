# TX Trigger Level

The TX FIFO trigger level generates an interrupt whenever the data level in the transmit FIFO falls below this preset trigger level.

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
| FSCC (16C950) | 0 - 127 |
| Async-335 (17D15X) | 0 - 64 |
| Async-PCIe (17V35X) | 0 - 255 |

## Get
```c
IOCTL_FASTCOM_GET_TX_TRIGGER
```

###### Examples
```
#include <serialfc.h>
...

unsigned level;

DeviceIoControl(h, IOCTL_FASTCOM_GET_TX_TRIGGER,
				NULL, 0,
				&level, sizeof(level),
				&temp, NULL);
```


## Set
```c
IOCTL_FASTCOM_SET_TX_TRIGGER
```

| System Error | Value | Cause |
| ------------ | -----:| ----- |
| `ERROR_INVALID_PARAMETER` | 87 (0x57) | Invalid parameter |

###### Examples
```
#include <serialfc.h>
...

unsigned level = 32;

DeviceIoControl(h, IOCTL_FASTCOM_SET_TX_TRIGGER,
				&level, sizeof(level),
				NULL, 0,
				&temp, NULL);
```


### Additional Resources
- Complete example: [`examples/tx-trigger.c`](../examples/tx-trigger.c)
