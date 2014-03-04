# Isochronous


###### Support
| Code           | Version
| -------------- | --------
| `serialfc-windows` | `v2.0.0` 


## Get
```c
IOCTL_FASTCOM_GET_ISOCHRONOUS
```

###### Examples
```c
#include <serialfc.h>
...

unsigned mode;

DeviceIoControl(h, IOCTL_FASTCOM_GET_ISOCHRONOUS, 
                NULL, 0, 
                &mode, sizeof(mode), 
                &temp, NULL);
```


## Enable
```c
IOCTL_FASTCOM_ENABLE_ISOCHRONOUS
```

###### Examples
```c
#include <serialfc.h>
...

DeviceIoControl(h, IOCTL_FASTCOM_ENABLE_ISOCHRONOUS, 
                &mode, sizeof(mode),
                NULL, 0, 
                &temp, NULL);
```


## Disable
```c
IOCTL_FASTCOM_DISABLE_ISOCHRONOUS
```

###### Examples
```c
#include <serialfc.h>
...

DeviceIoControl(h, IOCTL_FASTCOM_DISABLE_ISOCHRONOUS, 
                NULL, 0, 
                NULL, 0, 
                &temp, NULL);
```


### Additional Resources
- Complete example: [`examples\isochronous.c`](https://github.com/commtech/serialfc-windows/blob/master/examples/isochronous.c)
