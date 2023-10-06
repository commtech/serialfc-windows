#include <serialfc.h>
#include "calculate-clock-bits.h"

int main(void)
{
    HANDLE h = 0;
    DWORD tmp;
	unsigned type = 0;
	int return_value;

    h = CreateFile("\\\\.\\COM3", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);

	DeviceIoControl(h, IOCTL_FASTCOM_GET_CARD_TYPE,
                    NULL, 0,
                    &type, sizeof(type),
                    &tmp, (LPOVERLAPPED)NULL);

	switch(type) {
		case SERIALFC_CARD_TYPE_FSCC: {
			struct clock_data_fscc clock_data;
			
			clock_data.frequency = 18432000;
			return_value = calculate_clock_bits_fscc(&clock_data, 10);
			DeviceIoControl(h, IOCTL_FASTCOM_SET_CLOCK_BITS,
                    &clock_data, sizeof(clock_data),
                    NULL, 0,
                    &tmp, (LPOVERLAPPED)NULL);
			break;
		}
		case SERIALFC_CARD_TYPE_PCI: {
			struct clock_data_335 clock_data;
			
			clock_data.frequency = 18432000;
			return_value = calculate_clock_bits_335(&clock_data);
			DeviceIoControl(h, IOCTL_FASTCOM_SET_CLOCK_BITS,
                    &clock_data, sizeof(clock_data),
                    NULL, 0,
                    &tmp, (LPOVERLAPPED)NULL);
			break;
		}
		default:
			break;
	}

    CloseHandle(h);

    return 0;
}
