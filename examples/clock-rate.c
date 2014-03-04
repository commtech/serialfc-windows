#include <serialfc.h>

int main(void)
{
    HANDLE h = 0;
    DWORD tmp;
    unsigned rate = 10000000;
    
    h = CreateFile("\\\\.\\COM3", GENERIC_READ | GENERIC_WRITE, 0, NULL, 
                   OPEN_EXISTING, 0, NULL);

    DeviceIoControl(h, IOCTL_FASTCOM_SET_CLOCK_RATE, 
                    &rate, sizeof(rate), 
                    NULL, 0, 
                    &tmp, (LPOVERLAPPED)NULL);

    CloseHandle(h);
    
    return 0;
}
