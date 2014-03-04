# Clock Rate

###### Support
| Code | Version |
| -----| ------- |
| `serialfc-windows` | `v2.0.0` 


###### Operating Range
| Card Family | Range |
| ----------- | ----- |
| FSCC (16C950) | 200 Hz - 270 MHz |
| Async-335 (17D15X) | 6 Mhz - 180 Mhz |
| Async-PCIe (17V35X) | Not required |


## Set
```c
IOCTL_FASTCOM_SET_CLOCK_RATE
```

###### Examples
```
#include <serialfc.h>
...

DeviceIoControl(h, IOCTL_FASTCOM_SET_CLOCK_RATE, 
				&level, sizeof(level), 
				NULL, 0, 
				&temp, NULL);
```


### Additional Resources
- Complete example: [`examples\clock-rate.c`](https://github.com/commtech/serialfc-windows/blob/master/examples/clock-rate.c)
