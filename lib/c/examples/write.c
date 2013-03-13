#include <conio.h>
#include <stdio.h>

#include <Windows.h>

#include "../src/serialfc.h"
int main(int argc, char *argv[])
{
	HANDLE h;
	int e = 0;
	unsigned i = 0;
	unsigned bytes_written = 0;
	char odata[20];

	e = serialfc_connect(3, FALSE, &h);
	if (e != 0) {
		fprintf(stderr, "serialfc_connect failed with %d\n", e);
		return EXIT_FAILURE;
	}

	memset(odata, 0, sizeof(odata));
	memset(odata, 'U', sizeof(odata) - 1);

	e = serialfc_write(h, odata, sizeof(odata), &bytes_written, 0);
	if (e != 0) {
		fprintf(stderr, "serialfc_read failed with %d\n", e);
		return EXIT_FAILURE;
	}

	serialfc_disconnect(h);

	return EXIT_SUCCESS;
}