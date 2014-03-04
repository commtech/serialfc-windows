# 9-Bit Protocol

Enabling 9-Bit protocol has a couple of effects.

- Transmitting with 9-bit protocol enabled automatically sets the 1st byte's 9th bit to MARK, and all remaining bytes's 9th bits to SPACE.
- Receiving with 9-bit protocol enabled will return two bytes per each 9-bits of data. The second of each byte-duo contains the 9th bit.


###### Support
| Code           | Version
| -------------- | --------
| `serialfc-windows` | `v2.0.0` 


## Get
```c
IOCTL_FASTCOM_GET_9BIT
```

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
- Complete example: [`examples\nine_bit.c`](https://github.com/commtech/serialfc-windows/blob/master/examples/nine_bit.c)
