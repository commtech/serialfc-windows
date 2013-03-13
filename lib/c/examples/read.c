#include <conio.h>
#include <stdio.h>

#include <Windows.h>

#include "../src/serialfc.h"

int main(int argc, char *argv[])
{
	HANDLE h;
	int e = 0;
	unsigned i = 0;
	unsigned bytes_read = 0;
	char idata[100];

	e = serialfc_connect(3, FALSE, &h);
	if (e != 0) {
		fprintf(stderr, "serialfc_connect failed with %d\n", e);
		return EXIT_FAILURE;
	}

	e = serialfc_read(h, idata, sizeof(idata), &bytes_read, 0);
	if (e != 0) {
		fprintf(stderr, "serialfc_read failed with %d\n", e);
		return EXIT_FAILURE;
	}

	serialfc_disconnect(h);

	return EXIT_SUCCESS;
}