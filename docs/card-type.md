# Card Type

###### Code Support
| Code | Version |
| ---- | ------- |
| serialfc-windows | 2.1.6 |

###### Card Support
| Card Family | Supported |
| ----------- |:-----:|
| FSCC (16C950) | Yes |
| Async-335 (17D15X) | Yes |
| Async-PCIe (17V35X) | Yes |

## Get
```c
IOCTL_FASTCOM_GET_CARD_TYPE
```

###### Examples
```
#include <serialfc.h>
...

unsigned type;

DeviceIoControl(h, IOCTL_FASTCOM_GET_CARD_TYPE,
				NULL, 0,
				&type, sizeof(type),
				&temp, NULL);
```


### Additional Resources
- Complete example: [`examples/card-type.c`](../examples/card-type.c)
