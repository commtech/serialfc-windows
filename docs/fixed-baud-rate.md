# Fixed Baud Rate


###### Support
| Code           | Version
| -------------- | --------
| `serialfc-windows` | `v2.0.0` 


## Get
```c
IOCTL_FASTCOM_GET_FIXED_BAUD_RATE
```

###### Examples
```c
#include <serialfc.h>
...

int rate;

DeviceIoControl(h, IOCTL_FASTCOM_GET_FIXED_BAUD_RATE, 
                NULL, 0, 
                &rate, sizeof(rate), 
                &temp, NULL);
```


## Enable
```c
IOCTL_FASTCOM_ENABLE_FIXED_BAUD_RATE
```

###### Examples
```c
#include <serialfc.h>
...

DeviceIoControl(h, IOCTL_FASTCOM_ENABLE_FIXED_BAUD_RATE, 
                &rate, sizeof(rate),
                NULL, 0, 
                &temp, NULL);
```


## Disable
```c
IOCTL_FASTCOM_DISABLE_FIXED_BAUD_RATE
```

###### Examples
```c
#include <serialfc.h>
...

DeviceIoControl(h, IOCTL_FASTCOM_DISABLE_FIXED_BAUD_RATE, 
                NULL, 0, 
                NULL, 0, 
                &temp, NULL);
```


### Additional Resources
- Complete example: [`examples\fixed-baud-rate.c`](https://github.com/commtech/serialfc-windows/blob/master/examples/fixed-baud-rate.c)
