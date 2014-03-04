#include <serialfc.h>

int main(void)
{
    HANDLE h = 0;
    DWORD tmp;
    unsigned mode;

    h = CreateFile("\\\\.\\COM3", GENERIC_READ | GENERIC_WRITE, 0, NULL, 
                   OPEN_EXISTING, 0, NULL);
    
    DeviceIoControl(h, IOCTL_FASTCOM_GET_ISOCHRONOUS, 
                    NULL, 0, 
                    &mode, sizeof(mode), 
                    &tmp, (LPOVERLAPPED)NULL);

    mode = 7;
    DeviceIoControl(h, IOCTL_FASTCOM_ENABLE_ISOCHRONOUS, 
                    &mode, sizeof(mode), 
                    NULL, 0, 
                    &tmp, (LPOVERLAPPED)NULL);
    
    DeviceIoControl(h, IOCTL_FASTCOM_DISABLE_ISOCHRONOUS, 
                    NULL, 0, 
                    NULL, 0, 
                    &tmp, (LPOVERLAPPED)NULL);

    CloseHandle(h);
    
    return 0;
}