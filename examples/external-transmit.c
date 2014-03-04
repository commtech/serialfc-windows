#include <serialfc.h>

int main(void)
{
    HANDLE h = 0;
    DWORD tmp;
    unsigned num_chars;

    h = CreateFile("\\\\.\\COM3", GENERIC_READ | GENERIC_WRITE, 0, NULL, 
                   OPEN_EXISTING, 0, NULL);
    
    DeviceIoControl(h, IOCTL_FASTCOM_GET_EXTERNAL_TRANSMIT, 
                    NULL, 0, 
                    &num_chars, sizeof(num_chars), 
                    &tmp, (LPOVERLAPPED)NULL);

    num_chars = 4;
    DeviceIoControl(h, IOCTL_FASTCOM_ENABLE_EXTERNAL_TRANSMIT, 
                    &num_chars, sizeof(num_chars), 
                    NULL, 0, 
                    &tmp, (LPOVERLAPPED)NULL);
    
    DeviceIoControl(h, IOCTL_FASTCOM_DISABLE_EXTERNAL_TRANSMIT, 
                    NULL, 0, 
                    NULL, 0, 
                    &tmp, (LPOVERLAPPED)NULL);

    CloseHandle(h);
    
    return 0;
}