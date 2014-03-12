#include <serialfc.h>

int main(void)
{
    HANDLE h = 0;
    DWORD tmp;
    unsigned level;

    h = CreateFile("\\\\.\\COM3", GENERIC_READ | GENERIC_WRITE, 0, NULL,
                   OPEN_EXISTING, 0, NULL);

    DeviceIoControl(h, IOCTL_FASTCOM_GET_RX_TRIGGER,
                    NULL, 0,
                    &level, sizeof(level),
                    &tmp, (LPOVERLAPPED)NULL);

    level = 32;
    DeviceIoControl(h, IOCTL_FASTCOM_SET_RX_TRIGGER,
                    &level, sizeof(level),
                    NULL, 0,
                    &tmp, (LPOVERLAPPED)NULL);

    CloseHandle(h);

    return 0;
}