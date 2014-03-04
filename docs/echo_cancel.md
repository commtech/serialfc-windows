# Echo Cancel


###### Support
| Code           | Version
| -------------- | --------
| `serialfc-windows` | `v2.0.0` 


## Get
```c
IOCTL_FASTCOM_GET_ECHO_CANCEL
```

###### Examples
```c
#include <serialfc.h>
...

unsigned status;

DeviceIoControl(h, IOCTL_FASTCOM_GET_ECHO_CANCEL, 
                NULL, 0, 
                &status, sizeof(status), 
                &temp, NULL);
```


## Enable
```c
IOCTL_FASTCOM_ENABLE_ECHO_CANCEL
```

###### Examples
```c
#include <serialfc.h>
...

DeviceIoControl(h, IOCTL_FASTCOM_ENABLE_ECHO_CANCEL, 
                NULL, 0, 
                NULL, 0, 
                &temp, NULL);
```


## Disable
```c
IOCTL_FASTCOM_DISABLE_ECHO_CANCEL
```

###### Examples
```c
#include <serialfc.h>
...

DeviceIoControl(h, IOCTL_FASTCOM_DISABLE_ECHO_CANCEL, 
                NULL, 0, 
                NULL, 0, 
                &temp, NULL);
```


### Additional Resources
- Complete example: [`examples\echo-cancel.c`](https://github.com/commtech/serialfc-windows/blob/master/examples/echo-cancel.c)
