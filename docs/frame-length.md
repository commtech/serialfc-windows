# Frame Length

###### Support
| Code           | Version
| -------------- | --------
| `serialfc-windows` | `v2.0.0` 

## Get
```c
IOCTL_FASTCOM_GET_FRAME_LENGTH
```

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

###### Examples
```
#include <serialfc.h>
...

DeviceIoControl(h, IOCTL_FASTCOM_SET_FRAME_LENGTH, 
				&length, sizeof(length), 
				NULL, 0, 
				&temp, NULL);
```


### Additional Resources
- Complete example: [`examples\frame-length.c`](https://github.com/commtech/serialfc-windows/blob/master/examples/frame-length.c)
