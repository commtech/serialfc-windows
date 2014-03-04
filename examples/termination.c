#include <serialfc.h>

int main(void)
{
    HANDLE h = 0;
    DWORD tmp;
    unsigned status = 0;

    h = CreateFile("\\\\.\\COM3", GENERIC_READ | GENERIC_WRITE, 0, NULL, 
                   OPEN_EXISTING, 0, NULL);
    
    DeviceIoControl(h, IOCTL_FASTCOM_GET_TERMINATION, 
                    NULL, 0, 
                    &status, sizeof(status), 
                    &tmp, (LPOVERLAPPED)NULL);

    DeviceIoControl(h, IOCTL_FASTCOM_ENABLE_TERMINATION, 
                    NULL, 0, 
                    NULL, 0, 
                    &tmp, (LPOVERLAPPED)NULL);
    
    DeviceIoControl(h, IOCTL_FASTCOM_DISABLE_TERMINATION, 
                    NULL, 0, 
                    NULL, 0, 
                    &tmp, (LPOVERLAPPED)NULL);

    CloseHandle(h);
    
    return 0;
}