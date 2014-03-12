#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>

int main(void)
{
    HANDLE h = 0;
    DWORD tmp;
    char odata[] = "Hello world!";
    char idata[20];
    DCB mdcb;
    COMMTIMEOUTS cto;

    /* Open port 0 in a blocking IO mode */
    h = CreateFile("\\\\.\\COM3", GENERIC_READ | GENERIC_WRITE, 0, NULL,
                   OPEN_EXISTING, 0, NULL);

    if (h == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "CreateFile failed with %d\n", GetLastError());
        return EXIT_FAILURE;
    }

	memset(&mdcb, 0, sizeof(mdcb));
	memset(&cto, 0, sizeof(cto));

	if (BuildCommDCB("baud=115200 parity=N data=8 stop=1", &mdcb) == 0) {
		fprintf(stdout, "BuildCommDCB failed with %d\n", GetLastError());
		return EXIT_FAILURE;
    }

	cto.ReadIntervalTimeout = 1;

	if (SetCommState(h, &mdcb) == FALSE) {
		fprintf (stderr, "SetCommState failed with %d\n", GetLastError());
		return EXIT_FAILURE;
	}

	if (SetCommTimeouts(h, &cto) == FALSE) {
		fprintf(stdout, "SetCommTimeouts failed with %d\n", GetLastError());
		return EXIT_FAILURE;
	}

	PurgeComm(h, PURGE_TXCLEAR | PURGE_RXCLEAR);

    /* Send "Hello world!" text */
    WriteFile(h, odata, sizeof(odata), &tmp, NULL);

    /* Read the data back in (with our loopback connector) */
    ReadFile(h, idata, sizeof(idata), &tmp, NULL);

    fprintf(stdout, "%s\n", idata);

    CloseHandle(h);

    return EXIT_SUCCESS;
}
