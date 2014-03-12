# Connect

###### Code Support
| Code | Version |
| ---- | ------- |
| serialfc-windows | 2.0.0 |


## Connect
The Windows [`CreateFile`](http://msdn.microsoft.com/en-us/library/windows/desktop/aa363858.aspx) is used to connect to the port.

###### Examples
```c
#include <Windows.h>
...

HANDLE h;

h = CreateFile("\\\\.\\COM3", GENERIC_READ | GENERIC_WRITE, 0, NULL,
               OPEN_EXISTING, 0, NULL);
```


### Additional Resources
- Complete example: [`examples/tutorial.c`](../examples/tutorial.c)
