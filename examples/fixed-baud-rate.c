#include <serialfc.h>

int main(void)
{
    HANDLE h = 0;
    DWORD tmp;
    unsigned rate;

    h = CreateFile("\\\\.\\COM3", GENERIC_READ | GENERIC_WRITE, 0, NULL, 
                   OPEN_EXISTING, 0, NULL);
    
    DeviceIoControl(h, IOCTL_FASTCOM_GET_FIXED_BAUD_RATE, 
                    NULL, 0, 
                    &rate, sizeof(rate), 
                    &tmp, (LPOVERLAPPED)NULL);

    rate = 1000000;
    DeviceIoControl(h, IOCTL_FASTCOM_ENABLE_FIXED_BAUD_RATE, 
                    &rate, sizeof(rate), 
                    NULL, 0, 
                    &tmp, (LPOVERLAPPED)NULL);
    
    DeviceIoControl(h, IOCTL_FASTCOM_DISABLE_FIXED_BAUD_RATE, 
                    NULL, 0, 
                    NULL, 0, 
                    &tmp, (LPOVERLAPPED)NULL);

    CloseHandle(h);
    
    return 0;
}