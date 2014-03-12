# Read

###### Support
| Code | Version |
| ---- | ------- |
| serialfc-windows | 2.0.0 |


## Read
The Windows [`ReadFile`](http://msdn.microsoft.com/en-us/library/windows/desktop/aa365467.aspx) is used to read data from the port.

###### Examples
```c
#include <fscc.h>
...

char idata[20] = {0};
unsigned bytes_read;

ReadFile(h, idata, sizeof(idata), (DWORD*)bytes_read, NULL);
```


### Additional Resources
- Complete example: [`examples/tutorial.c`](../examples/tutorial.c)
