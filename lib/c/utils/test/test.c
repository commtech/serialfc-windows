#include <conio.h>
#include <stdio.h>

#include <Windows.h>

#include <serialfc.h>

#define DATA_LENGTH 20
#define NUM_ITERATIONS 20

int init(HANDLE h);
int loop(HANDLE h);

int main(int argc, char *argv[])
{
	HANDLE h;
	int e = 0;
	unsigned port_num;
	unsigned i = 0;

	if (argc != 2) {
		fprintf(stdout, "%s PORT_NUM", argv[0]);
		return EXIT_FAILURE;
	}

	port_num = atoi(argv[1]);

	e = serialfc_connect(port_num, FALSE, &h);
	if (e != 0) {
		fprintf(stderr, "serialfc_connect failed with %d\n", e);
		return EXIT_FAILURE;
	}

	fprintf(stdout, "This is a very simple test to verify your card is\n");
	fprintf(stdout, "communicating correctly.\n\n");

	fprintf(stdout, "NOTE: This will change your registers to defaults.\n\n");

	fprintf(stdout, "1) Connect your loopback plug.\n");
	fprintf(stdout, "2) Press any key to start the test.\n\n");


	_getch();

	e = init(h);
	if (e != 0) {
		serialfc_disconnect(h);
		return EXIT_FAILURE;
	}

	for (i = 0; i < NUM_ITERATIONS; i++) {
		e = loop(h);
		if (e != 0) {
			if (e == ERROR_INVALID_DATA) {
				break;
			}
			else {
				serialfc_disconnect(h);
				return EXIT_FAILURE;
			}
		}
	}

	if (e != ERROR_INVALID_DATA)
		fprintf(stdout, "Passed, you can begin development.");
	else
		fprintf(stderr, "Failed, contact technical support.");

	serialfc_disconnect(h);

	return EXIT_SUCCESS;
}

int init(HANDLE h)
{
	DCB mdcb;
	COMMTIMEOUTS cto;
    int e = 0;
 
	e = serialfc_set_clock_rate(h, 18432000);
	if (e != 0 && e != ERROR_NOT_SUPPORTED) {
		fprintf(stderr, "serialfc_set_clock_rate failed with %d\n", e);
		return EXIT_FAILURE;
	}

	e = serialfc_set_sample_rate(h, 16);
	if (e != 0) {
		fprintf(stderr, "serialfc_set_sample_rate failed with %d\n", e);
		return EXIT_FAILURE;
	}

	e = serialfc_disable_rs485(h);
	if (e != 0) {
		fprintf(stderr, "serialfc_connect failed with %d\n", e);
		return EXIT_FAILURE;
	}

	e = serialfc_disable_echo_cancel(h);
	if (e != 0) {
		fprintf(stderr, "serialfc_disable_echo_cancel failed with %d\n", e);
		return EXIT_FAILURE;
	}

	e = serialfc_enable_termination(h);
	if (e != 0 && e != ERROR_NOT_SUPPORTED) {
		fprintf(stderr, "serialfc_enable_termination failed with %d\n", e);
		return EXIT_FAILURE;
	}

	e = serialfc_set_tx_trigger(h, 32);
	if (e != 0) {
		fprintf(stderr, "serialfc_set_tx_trigger failed with %d\n", e);
		return EXIT_FAILURE;
	}

	e = serialfc_set_rx_trigger(h, 32);
	if (e != 0) {
		fprintf(stderr, "serialfc_set_rx_trigger failed with %d\n", e);
		return EXIT_FAILURE;
	}

	e = serialfc_disable_isochronous(h);
	if (e != 0 && e != ERROR_NOT_SUPPORTED) {
		fprintf(stderr, "serialfc_disable_isochronous failed with %d\n", e);
		return EXIT_FAILURE;
	}

	e = serialfc_disable_external_transmit(h);
	if (e != 0 && e != ERROR_NOT_SUPPORTED) {
		fprintf(stderr, "serialfc_disable_external_transmit failed with %d\n", e);
		return EXIT_FAILURE;
	}

	e = serialfc_set_frame_length(h, 1);
	if (e != 0 && e != ERROR_NOT_SUPPORTED) {
		fprintf(stderr, "serialfc_set_frame_length failed with %d\n", e);
		return EXIT_FAILURE;
	}

	e = serialfc_disable_9bit(h);
	if (e != 0 && e != ERROR_NOT_SUPPORTED) {
		fprintf(stderr, "serialfc_disable_9bit failed with %d\n", e);
		return EXIT_FAILURE;
	}

	e = serialfc_disable_fixed_baud_rate(h);
	if (e != 0) {
		fprintf(stderr, "serialfc_disable_fixed_baud_rate failed with %d\n", e);
		return EXIT_FAILURE;
	}

	GetCommState(h, &mdcb);
	
	mdcb.BaudRate = 115200;
	
	if (SetCommState(h, &mdcb) == FALSE) {
        fprintf (stderr, "SetCommState failed with %d\n", GetLastError());
        return EXIT_FAILURE;
    }
	
	PurgeComm(h, PURGE_TXCLEAR | PURGE_RXCLEAR);
	
	cto.ReadIntervalTimeout = 25;
	cto.ReadTotalTimeoutMultiplier = 2;
	cto.ReadTotalTimeoutConstant = 10;
	cto.WriteTotalTimeoutMultiplier = 2;
	cto.WriteTotalTimeoutConstant = 0;

	SetCommTimeouts(h, &cto);
	
	return ERROR_SUCCESS;
}

int loop(HANDLE h)
{
	unsigned bytes_written = 0, bytes_read = 0;
	char odata[DATA_LENGTH];
	char idata[100];
	int e = 0;
	OVERLAPPED o;

	memset(&o, 0, sizeof(o));
	memset(odata, 0x01, sizeof(odata));
	memset(&idata, 0, sizeof(idata));

	e = serialfc_write(h, odata, sizeof(odata), &bytes_written, NULL);
	if (e != 0) {
		fprintf(stderr, "serialfc_write failed with %d\n", e);
		return EXIT_FAILURE;
	}

	e = serialfc_read(h, idata, sizeof(idata), &bytes_read, NULL);
	if (e != 0) {
		fprintf(stderr, "serialfc_read with %d\n", e);
		return EXIT_FAILURE;
	}
	
	if (bytes_read == 0 || memcmp(odata, idata, sizeof(odata)) != 0)
		return ERROR_INVALID_DATA;

	return ERROR_SUCCESS;
}