# External Transmit


###### Support
| Code           | Version
| -------------- | --------
| `serialfc-windows` | `v2.0.0` 


## Get
```c
IOCTL_FASTCOM_GET_EXTERNAL_TRANSMIT
```

###### Examples
```c
#include <serialfc.h>
...

unsigned num_chars;

DeviceIoControl(h, IOCTL_FASTCOM_GET_EXTERNAL_TRANSMIT, 
                NULL, 0, 
                &num_chars, sizeof(num_chars), 
                &temp, NULL);
```


## Enable
```c
IOCTL_FASTCOM_ENABLE_EXTERNAL_TRANSMIT
```

###### Examples
```c
#include <serialfc.h>
...

DeviceIoControl(h, IOCTL_FASTCOM_ENABLE_EXTERNAL_TRANSMIT, 
                &num_chars, sizeof(num_chars),
                NULL, 0, 
                &temp, NULL);
```


## Disable
```c
IOCTL_FASTCOM_DISABLE_EXTERNAL_TRANSMIT
```

###### Examples
```c
#include <serialfc.h>
...

DeviceIoControl(h, IOCTL_FASTCOM_DISABLE_EXTERNAL_TRANSMIT, 
                NULL, 0, 
                NULL, 0, 
                &temp, NULL);
```


### Additional Resources
- Complete example: [`examples\external-transmit.c`](https://github.com/commtech/serialfc-windows/blob/master/examples/external-transmit.c)
