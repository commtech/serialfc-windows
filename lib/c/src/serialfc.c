/*! \file */ 
#include <stdio.h>

#include "serialfc.h"

#define MAX_NAME_LENGTH 25

/******************************************************************************/
/*!

  \brief Opens a handle to a SerialFC port.

  \param[in] port_num 
    the SerialFC port number
  \param[in] overlapped 
    whether you would like to use the port in overlapped mode
  \param[out] h 
    user variable that the port's HANDLE will be assigned to
      
  \return 0 
    if the operation completed successfully
  \return >= 1 
    if the operation failed (see MSDN 'System Error Codes')

  \note
    Opening a handle using this API will only give you access to the
	asynchronous functionality of the card. You will need to use the FSCC ports
	if you would like to use the synchronous functionality.

*/
/******************************************************************************/
int serialfc_connect(unsigned port_num, BOOL overlapped, HANDLE *h)
{
  char name[MAX_NAME_LENGTH];
  DWORD flags_and_attributes = FILE_ATTRIBUTE_NORMAL;

  sprintf_s(name, MAX_NAME_LENGTH, "\\\\.\\COM%u", port_num);
        
  if (overlapped)
  	flags_and_attributes |= FILE_FLAG_OVERLAPPED;

  *h = CreateFile(name,
  		GENERIC_READ | GENERIC_WRITE,
  		FILE_SHARE_READ | FILE_SHARE_WRITE,
  		NULL,
  		OPEN_EXISTING,
  		flags_and_attributes,
  		NULL
  );

  return (*h != INVALID_HANDLE_VALUE) ? ERROR_SUCCESS : GetLastError();
}

int serialfc_enable_rs485(HANDLE h)
{
  DWORD temp;
  BOOL result;

  result = DeviceIoControl(h, (DWORD)IOCTL_FASTCOM_ENABLE_RS485, 
                           NULL, 0, 
                           NULL, 0, 
                           &temp, (LPOVERLAPPED)NULL);

  return (result == TRUE) ? ERROR_SUCCESS : GetLastError();
}

int serialfc_disable_rs485(HANDLE h)
{
  DWORD temp;
  BOOL result;

  result = DeviceIoControl(h, (DWORD)IOCTL_FASTCOM_DISABLE_RS485, 
                           NULL, 0, 
                           NULL, 0, 
                           &temp, (LPOVERLAPPED)NULL);

  return (result == TRUE) ? ERROR_SUCCESS : GetLastError();
}

int serialfc_get_rs485(HANDLE h, BOOL *status)
{
  DWORD temp;
  BOOL result;

  result = DeviceIoControl(h, (DWORD)IOCTL_FASTCOM_GET_RS485, 
                           NULL, 0, 
                           status, sizeof(*status), 
                           &temp, (LPOVERLAPPED)NULL);

  return (result == TRUE) ? ERROR_SUCCESS : GetLastError();
}

int serialfc_enable_echo_cancel(HANDLE h)
{
  DWORD temp;
  BOOL result;

  result = DeviceIoControl(h, (DWORD)IOCTL_FASTCOM_ENABLE_ECHO_CANCEL, 
                           NULL, 0, 
                           NULL, 0, 
                           &temp, (LPOVERLAPPED)NULL);

  return (result == TRUE) ? ERROR_SUCCESS : GetLastError();
}

int serialfc_disable_echo_cancel(HANDLE h)
{
  DWORD temp;
  BOOL result;

  result = DeviceIoControl(h, (DWORD)IOCTL_FASTCOM_DISABLE_ECHO_CANCEL, 
                           NULL, 0, 
                           NULL, 0, 
                           &temp, (LPOVERLAPPED)NULL);

  return (result == TRUE) ? ERROR_SUCCESS : GetLastError();
}

int serialfc_get_echo_cancel(HANDLE h, BOOL *status)
{
  DWORD temp;
  BOOL result;

  result = DeviceIoControl(h, (DWORD)IOCTL_FASTCOM_GET_ECHO_CANCEL, 
                           NULL, 0, 
                           status, sizeof(*status), 
                           &temp, (LPOVERLAPPED)NULL);

  return (result == TRUE) ? ERROR_SUCCESS : GetLastError();
}

int serialfc_enable_termination(HANDLE h)
{
  DWORD temp;
  BOOL result;

  result = DeviceIoControl(h, (DWORD)IOCTL_FASTCOM_ENABLE_TERMINATION, 
                           NULL, 0, 
                           NULL, 0, 
                           &temp, (LPOVERLAPPED)NULL);

  return (result == TRUE) ? ERROR_SUCCESS : GetLastError();
}

int serialfc_disable_termination(HANDLE h)
{
  DWORD temp;
  BOOL result;

  result = DeviceIoControl(h, (DWORD)IOCTL_FASTCOM_DISABLE_TERMINATION, 
                           NULL, 0, 
                           NULL, 0, 
                           &temp, (LPOVERLAPPED)NULL);

  return (result == TRUE) ? ERROR_SUCCESS : GetLastError();
}

int serialfc_get_termination(HANDLE h, BOOL *status)
{
  DWORD temp;
  BOOL result;

  result = DeviceIoControl(h, (DWORD)IOCTL_FASTCOM_GET_TERMINATION, 
                           NULL, 0, 
                           status, sizeof(*status), 
                           &temp, (LPOVERLAPPED)NULL);

  return (result == TRUE) ? ERROR_SUCCESS : GetLastError();
}

int serialfc_set_sample_rate(HANDLE h, unsigned rate)
{
  DWORD temp;
  BOOL result;

  result = DeviceIoControl(h, (DWORD)IOCTL_FASTCOM_SET_SAMPLE_RATE, 
                           &rate, sizeof(rate), 
                           NULL, 0, 
                           &temp, (LPOVERLAPPED)NULL);

  return (result == TRUE) ? ERROR_SUCCESS : GetLastError();
}

int serialfc_get_sample_rate(HANDLE h, unsigned *rate)
{
  DWORD temp;
  BOOL result;

  result = DeviceIoControl(h, (DWORD)IOCTL_FASTCOM_GET_SAMPLE_RATE, 
                           NULL, 0, 
                           rate, sizeof(*rate), 
                           &temp, (LPOVERLAPPED)NULL);

  return (result == TRUE) ? ERROR_SUCCESS : GetLastError();
}

int serialfc_set_tx_trigger(HANDLE h, unsigned level)
{
  DWORD temp;
  BOOL result;

  result = DeviceIoControl(h, (DWORD)IOCTL_FASTCOM_SET_TX_TRIGGER, 
                           &level, sizeof(level), 
                           NULL, 0, 
                           &temp, (LPOVERLAPPED)NULL);

  return (result == TRUE) ? ERROR_SUCCESS : GetLastError();
}

int serialfc_get_tx_trigger(HANDLE h, unsigned *level)
{
  DWORD temp;
  BOOL result;

  result = DeviceIoControl(h, (DWORD)IOCTL_FASTCOM_GET_TX_TRIGGER, 
                           NULL, 0, 
                           level, sizeof(*level), 
                           &temp, (LPOVERLAPPED)NULL);

  return (result == TRUE) ? ERROR_SUCCESS : GetLastError();
}

int serialfc_set_rx_trigger(HANDLE h, unsigned level)
{
  DWORD temp;
  BOOL result;

  result = DeviceIoControl(h, (DWORD)IOCTL_FASTCOM_SET_RX_TRIGGER, 
                           &level, sizeof(level), 
                           NULL, 0, 
                           &temp, (LPOVERLAPPED)NULL);

  return (result == TRUE) ? ERROR_SUCCESS : GetLastError();
}

int serialfc_get_rx_trigger(HANDLE h, unsigned *level)
{
  DWORD temp;
  BOOL result;

  result = DeviceIoControl(h, (DWORD)IOCTL_FASTCOM_GET_RX_TRIGGER, 
                           NULL, 0, 
                           level, sizeof(*level), 
                           &temp, (LPOVERLAPPED)NULL);

  return (result == TRUE) ? ERROR_SUCCESS : GetLastError();
}

int serialfc_set_clock_rate(HANDLE h, unsigned rate)
{
  DWORD temp;
  BOOL result;

  result = DeviceIoControl(h, (DWORD)IOCTL_FASTCOM_SET_CLOCK_RATE, 
                           &rate, sizeof(rate), 
                           NULL, 0, 
                           &temp, (LPOVERLAPPED)NULL);

  return (result == TRUE) ? ERROR_SUCCESS : GetLastError();
}

int serialfc_enable_isochronous(HANDLE h, unsigned mode)
{
  DWORD temp;
  BOOL result;

  result = DeviceIoControl(h, (DWORD)IOCTL_FASTCOM_ENABLE_ISOCHRONOUS, 
                           &mode, sizeof(mode), 
                           NULL, 0, 
                           &temp, (LPOVERLAPPED)NULL);

  return (result == TRUE) ? ERROR_SUCCESS : GetLastError();
}

int serialfc_disable_isochronous(HANDLE h)
{
  DWORD temp;
  BOOL result;

  result = DeviceIoControl(h, (DWORD)IOCTL_FASTCOM_ENABLE_ISOCHRONOUS, 
                           NULL, 0, 
                           NULL, 0, 
                           &temp, (LPOVERLAPPED)NULL);

  return (result == TRUE) ? ERROR_SUCCESS : GetLastError();
}


 
/******************************************************************************/
/*!

  \brief Transmits data out of a port.

  \param[in] h 
    HANDLE to the port
  \param[in] buf
    the buffer containing the data to transmit
  \param[in] size
    the number of bytes to transmit from 'buf'
  \param[out] bytes_written
    the input variable to store how many bytes were actually written
  \param[in,out] o
    OVERLAPPED structure for asynchronous operation
      
  \return 0 
    if the operation completed successfully
  \return >= 1 
    if the operation failed (see MSDN 'System Error Codes')

*/
/******************************************************************************/
int serialfc_write(HANDLE h, char *buf, unsigned size, unsigned *bytes_written, OVERLAPPED *o)
{
  BOOL result;
        
  result = WriteFile(h, buf, size, (DWORD*)bytes_written, o);

  return (result == TRUE) ? ERROR_SUCCESS : GetLastError();
}

/******************************************************************************/
/*!

  \brief Reads data out of a port.

  \param[in] h 
    HANDLE to the port
  \param[in] buf
    the input buffer used to store the receive data
  \param[in] size
    the maximum number of bytes to read in (typically sizeof(buf))
  \param[out] bytes_read
    the user variable to store how many bytes were actually read
  \param[in,out] o
    OVERLAPPED structure for asynchronous operation
      
  \return 0 
    if the operation completed successfully
  \return >= 1 
    if the operation failed (see MSDN 'System Error Codes')

*/
/******************************************************************************/
int serialfc_read(HANDLE h, char *buf, unsigned size, unsigned *bytes_read, OVERLAPPED *o)
{
  BOOL result;

  result = ReadFile(h, buf, size, (DWORD*)bytes_read, o);

  return (result == TRUE) ? ERROR_SUCCESS : GetLastError();
}

/******************************************************************************/
/*!

  \brief Closes the handle to a SerialFC port.

  \param[in] h 
    HANDLE to the port
      
  \return 0 
    if closing the port completed successfully
  \return >= 1 
    if the operation failed (see MSDN 'System Error Codes')

*/
/******************************************************************************/
int serialfc_disconnect(HANDLE h)
{
	BOOL result;

	result = CloseHandle(h);

	return (result == TRUE) ? ERROR_SUCCESS : GetLastError();
}