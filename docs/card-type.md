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

| Card Type | Value | Description |
| --------- | -----:| ----------- |
| `SERIALFC_CARD_TYPE_PCI` | 0 | Async-335 (17D15X) |
| `SERIALFC_CARD_TYPE_PCIE` | 1 | Async-PCIe (17V35X) |
| `SERIALFC_CARD_TYPE_FSCC` | 2 | FSCC (16C950) |
| `SERIALFC_CARD_TYPE_UNKNOWN` | 3 | Unknown |

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
