# Sample Rate

###### Support
| Code           | Version
| -------------- | --------
| `serialfc-windows` | `v2.0.0` 

###### Operating Range
| Card Family | Range |
| ----------- | ----- |
| FSCC (16C950) | 4 - 16 |
| Async-335 (17D15X) | 8, 16 |
| Async-PCIe (17V35X) | 4, 8, 16 |


## Get
```c
IOCTL_FASTCOM_GET_SAMPLE_RATE
```

###### Examples
```
#include <serialfc.h>
...

unsigned rate;

DeviceIoControl(h, IOCTL_FASTCOM_GET_SAMPLE_RATE, 
				NULL, 0, 
				&rate, sizeof(rate), 
				&temp, NULL);	
```


## Set
```c
IOCTL_FASTCOM_SET_SAMPLE_RATE
```

###### Examples
```
#include <serialfc.h>
...

DeviceIoControl(h, IOCTL_FASTCOM_SET_SAMPLE_RATE, 
				&rate, sizeof(rate), 
				NULL, 0, 
				&temp, NULL);
```


### Additional Resources
- Complete example: [`examples\sample-rate.c`](https://github.com/commtech/serialfc-windows/blob/master/examples/sample-rate.c)
