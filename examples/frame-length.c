#include <serialfc.h>

int main(void)
{
    HANDLE h = 0;
    DWORD tmp;
    unsigned length;

    h = CreateFile("\\\\.\\COM3", GENERIC_READ | GENERIC_WRITE, 0, NULL, 
                   OPEN_EXISTING, 0, NULL);
    
    DeviceIoControl(h, IOCTL_FASTCOM_GET_FRAME_LENGTH, 
                    NULL, 0, 
                    &length, sizeof(length), 
                    &tmp, (LPOVERLAPPED)NULL);

    length = 1;
    DeviceIoControl(h, IOCTL_FASTCOM_SET_FRAME_LENGTH, 
                    &length, sizeof(length), 
                    NULL, 0, 
                    &tmp, (LPOVERLAPPED)NULL);

    CloseHandle(h);
    
    return 0;
}