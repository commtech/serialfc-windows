# Card Type

###### Support
| Code           | Version
| -------------- | --------
| `serialfc-windows` | `v2.0.0` 

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
- Complete example: [`examples\card-type.c`](https://github.com/commtech/serialfc-windows/blob/master/examples/card-type.c)
