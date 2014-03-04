# TX Trigger Level

###### Support
| Code           | Version
| -------------- | --------
| `serialfc-windows` | `v2.0.0` 

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

###### Examples
```
#include <serialfc.h>
...

DeviceIoControl(h, IOCTL_FASTCOM_SET_TX_TRIGGER, 
				&level, sizeof(level), 
				NULL, 0, 
				&temp, NULL);
```


### Additional Resources
- Complete example: [`examples\tx-trigger.c`](https://github.com/commtech/serialfc-windows/blob/master/examples/tx-trigger.c)
