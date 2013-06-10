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
    If using an FSCC device this handle will only give you access to the 
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

/******************************************************************************/
/*!

  \brief Puts the port into RS485 mode

  \param[in] h 
    HANDLE to the port
      
  \return 0 
    if the operation completed successfully
  \return >= 1 
    if the operation failed (see MSDN 'System Error Codes')

*/
/******************************************************************************/
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

/******************************************************************************/
/*!

  \brief Takes the port out of RS485 mode

  \param[in] h 
    HANDLE to the port
      
  \return 0 
    if the operation completed successfully
  \return >= 1 
    if the operation failed (see MSDN 'System Error Codes')

*/
/******************************************************************************/
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

/******************************************************************************/
/*!

  \brief Gets whether the port is in RS485 mode

  \param[in] h 
    HANDLE to the port
  \param[out] status 
    whether the port is in rs485 mode
      
  \return 0 
    if the operation completed successfully
  \return >= 1 
    if the operation failed (see MSDN 'System Error Codes')

  \todo
    This isn't currently supported in any of the cards. It will be added in a
	future release.

*/
/******************************************************************************/
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

/******************************************************************************/
/*!

  \brief Turns on echo cancellation for the port

  \param[in] h 
    HANDLE to the port
      
  \return 0 
    if the operation completed successfully
  \return >= 1 
    if the operation failed (see MSDN 'System Error Codes')

*/
/******************************************************************************/
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

/******************************************************************************/
/*!

  \brief Turns off echo cancellation for the port

  \param[in] h 
    HANDLE to the port
      
  \return 0 
    if the operation completed successfully
  \return >= 1 
    if the operation failed (see MSDN 'System Error Codes')

*/
/******************************************************************************/
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

/******************************************************************************/
/*!

  \brief Gets whether the port has echo cancel enabled

  \param[in] h 
    HANDLE to the port
  \param[out] status 
    whether the port has echo cancel enabled
      
  \return 0 
    if the operation completed successfully
  \return >= 1 
    if the operation failed (see MSDN 'System Error Codes')

*/
/******************************************************************************/
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

/******************************************************************************/
/*!

  \brief Turns on software termination for the port

  \param[in] h 
    HANDLE to the port
      
  \return 0 
    if the operation completed successfully
  \return >= 1 
    if the operation failed (see MSDN 'System Error Codes')
	
  \note
    Only supported in the Async-PCIe cards

*/
/******************************************************************************/
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

/******************************************************************************/
/*!

  \brief Turns off software termination for the port

  \param[in] h 
    HANDLE to the port
      
  \return 0 
    if the operation completed successfully
  \return >= 1 
    if the operation failed (see MSDN 'System Error Codes')
	
  \note
    Only supported in the Async-PCIe cards

*/
/******************************************************************************/
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

/******************************************************************************/
/*!

  \brief Gets whether the port is software terminated

  \param[in] h 
    HANDLE to the port
  \param[out] status 
    whether the port is software terminated
      
  \return 0 
    if the operation completed successfully
  \return >= 1 
    if the operation failed (see MSDN 'System Error Codes')
	
  \note
    Only supported in the Async-PCIe cards

*/
/******************************************************************************/
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

/******************************************************************************/
/*!

  \brief Sets the port's sample rate

  \param[in] h 
    HANDLE to the port
  \param[in] rate 
    The sample rate
      
  \return 0 
    if the operation completed successfully
  \return >= 1 
    if the operation failed (see MSDN 'System Error Codes')
	
  \note
    Each card family has it's own supported values.
	
    - FSCC Family (16c950): 4 - 16
    - Async-335 Family (17D15X): 8, 16
    - Async-PCIe Family (17V35X): 4, 8, 16

*/
/******************************************************************************/
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

/******************************************************************************/
/*!

  \brief Gets the port's sample rate

  \param[in] h 
    HANDLE to the port
  \param[out] rate 
    the port's sample rate
      
  \return 0 
    if the operation completed successfully
  \return >= 1 
    if the operation failed (see MSDN 'System Error Codes')

*/
/******************************************************************************/
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

/******************************************************************************/
/*!

  \brief Sets the port's transmit trigger level

  \param[in] h 
    HANDLE to the port
  \param[in] level 
    The trigger level
      
  \return 0 
    if the operation completed successfully
  \return >= 1 
    if the operation failed (see MSDN 'System Error Codes')
	
  \note
    This is only supported on the Async-335 and Async-PCIe cards. Each also
	has it's own supported range.
	
    - Async-335 Family (17D15X): 0 - 64
    - Async-PCIe Family (17V35X): 0 - 255

*/
/******************************************************************************/
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

/******************************************************************************/
/*!

  \brief Gets the ports transmit trigger level

  \param[in] h 
    HANDLE to the port
  \param[out] level 
    the port's transmit trigger level
      
  \return 0 
    if the operation completed successfully
  \return >= 1 
    if the operation failed (see MSDN 'System Error Codes')
	
  \note
    This is only supported on the FSCC cards.

*/
/******************************************************************************/
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

/******************************************************************************/
/*!

  \brief Sets the port's receive trigger level

  \param[in] h 
    HANDLE to the port
  \param[in] level 
    The receive level
      
  \return 0 
    if the operation completed successfully
  \return >= 1 
    if the operation failed (see MSDN 'System Error Codes')
	
  \note
    Each card has it's own supported range.
	
    - FSCC Family (16c950): 1 - 127
    - Async-335 Family (17D15X): 0 - 64
    - Async-PCIe Family (17V35X): 0 - 255

*/
/******************************************************************************/
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

/******************************************************************************/
/*!

  \brief Gets the ports receive trigger level

  \param[in] h 
    HANDLE to the port
  \param[out] level 
    the port's receive trigger level
      
  \return 0 
    if the operation completed successfully
  \return >= 1 
    if the operation failed (see MSDN 'System Error Codes')

  \todo
    This is only supported in the FSCC cards but isn't currently implemented.

*/
/******************************************************************************/
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

/******************************************************************************/
/*!

  \brief Sets the ports clock rate

  \param[in] h 
    HANDLE to the port
  \param[out] rate 
    the port's clock rate
      
  \return 0 
    if the operation completed successfully
  \return >= 1 
    if the operation failed (see MSDN 'System Error Codes')
	
  \note
    This is only supported on the Async-335 and FSCC cards. Each also has it's 
	own supported range.
	
    - FSCC Family (16c950): 200 Hz - 270 MHz
    - Async-335 Family (17D15X): 6 MHz - 200 MHz
	
	The Async-PCIe family doesn't use a variable clock generator to achieve
	baud rates so this is not required.

*/
/******************************************************************************/
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

/******************************************************************************/
/*!

  \brief Turns on isochronous mode for the port

  \param[in] h 
    HANDLE to the port
  \param[in] mode 
    which isochronous mode
      
  \return 0 
    if the operation completed successfully
  \return >= 1 
    if the operation failed (see MSDN 'System Error Codes')
	
  \note
    This is only supported on the FSCC cards.

  \note
    If you apply an external clock to the card before turning on isochronous
    mode your system will freeze due to too many serial interrupts. Make sure 
    and apply the clock after you are the isochronous mode (so the interrupts 
    are disabled).
	
  \note
	There are 8 different isochronous combinations you can use.
	
	0: Transmit using external RI#
	1: Transmit using internal BRG
	2: Receive using external DSR#
	3: Transmit using external RI#, receive using external DSR#
	4: Transmit using internal BRG, receive using external DSR#
	5: Receive using internal BRG
	6: Transmit using external RI#, receive using internal BRG
	7: Transmit using internal BRG, receive using internal BRG
	8: Transmit and receive using external RI#

*/
/******************************************************************************/
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

/******************************************************************************/
/*!

  \brief Turns off isochronous mode for the port

  \param[in] h 
    HANDLE to the port
      
  \return 0 
    if the operation completed successfully
  \return >= 1 
    if the operation failed (see MSDN 'System Error Codes')
	
  \note
    This is only supported on the FSCC cards.

  \note
    If you have an external clock applied to the card while turning off 
    isochronous mode your system will freeze due to too many serial interrupts. 
    Make sure and remove the clock before you disable isochronous mode (while 
    the interrupts are disabled).

*/
/******************************************************************************/
int serialfc_disable_isochronous(HANDLE h)
{
  DWORD temp;
  BOOL result;

  result = DeviceIoControl(h, (DWORD)IOCTL_FASTCOM_DISABLE_ISOCHRONOUS, 
                           NULL, 0, 
                           NULL, 0, 
                           &temp, (LPOVERLAPPED)NULL);

  return (result == TRUE) ? ERROR_SUCCESS : GetLastError();
}

/******************************************************************************/
/*!

  \brief Gets the port's isochronous mode

  \param[in] h 
    HANDLE to the port
  \param[out] mode 
    the port's isochronous mode
      
  \return 0 
    if the operation completed successfully
  \return >= 1 
    if the operation failed (see MSDN 'System Error Codes')
	
  \note
    This is only supported on the FSCC cards.

*/
/******************************************************************************/
int serialfc_get_isochronous(HANDLE h, int *mode)
{
  DWORD temp;
  BOOL result;

  result = DeviceIoControl(h, (DWORD)IOCTL_FASTCOM_GET_ISOCHRONOUS, 
                           NULL, 0, 
                           mode, sizeof(*mode), 
                           &temp, (LPOVERLAPPED)NULL);

  return (result == TRUE) ? ERROR_SUCCESS : GetLastError();
}

/******************************************************************************/
/*!

  \brief Turns on external transmit mode for the port

  \param[in] h 
    HANDLE to the port
  \param[in] num_chars 
    the number of characters to send on external signal
      
  \return 0 
    if the operation completed successfully
  \return >= 1 
    if the operation failed (see MSDN 'System Error Codes')
  
  \note
    This is only supported on the FSCC cards.

*/
/******************************************************************************/
int serialfc_enable_external_transmit(HANDLE h, unsigned num_chars)
{
  DWORD temp;
  BOOL result;

  result = DeviceIoControl(h, (DWORD)IOCTL_FASTCOM_ENABLE_EXTERNAL_TRANSMIT, 
                           &num_chars, sizeof(num_chars), 
                           NULL, 0, 
                           &temp, (LPOVERLAPPED)NULL);

  return (result == TRUE) ? ERROR_SUCCESS : GetLastError();
}

/******************************************************************************/
/*!

  \brief Turns off external transmit mode for the port

  \param[in] h 
    HANDLE to the port
      
  \return 0 
    if the operation completed successfully
  \return >= 1 
    if the operation failed (see MSDN 'System Error Codes')
  
  \note
    This is only supported on the FSCC cards.

*/
/******************************************************************************/
int serialfc_disable_external_transmit(HANDLE h)
{
  DWORD temp;
  BOOL result;

  result = DeviceIoControl(h, (DWORD)IOCTL_FASTCOM_DISABLE_EXTERNAL_TRANSMIT, 
                           NULL, 0, 
                           NULL, 0, 
                           &temp, (LPOVERLAPPED)NULL);

  return (result == TRUE) ? ERROR_SUCCESS : GetLastError();
}

/******************************************************************************/
/*!

  \brief Gets the port's external transmit mode

  \param[in] h 
    HANDLE to the port
  \param[out] num_chars 
    the number of characters to send on external signal
      
  \return 0 
    if the operation completed successfully
  \return >= 1 
    if the operation failed (see MSDN 'System Error Codes')
  
  \note
    This is only supported on the FSCC cards.

*/
/******************************************************************************/
int serialfc_get_external_transmit(HANDLE h, unsigned *num_chars)
{
  DWORD temp;
  BOOL result;

  result = DeviceIoControl(h, (DWORD)IOCTL_FASTCOM_GET_EXTERNAL_TRANSMIT, 
                           NULL, 0, 
                           num_chars, sizeof(*num_chars), 
                           &temp, (LPOVERLAPPED)NULL);

  return (result == TRUE) ? ERROR_SUCCESS : GetLastError();
}

/******************************************************************************/
/*!

  \brief Sets how many characters are transmitted per frame.
  

  \param[in] h 
    HANDLE to the port
  \param[in] num_chars 
    the number of characters per frame
      
  \return 0 
    if the operation completed successfully
  \return >= 1 
    if the operation failed (see MSDN 'System Error Codes')
  
  \note
    This is only supported on the FSCC cards.

*/
/******************************************************************************/
int serialfc_set_frame_length(HANDLE h, unsigned num_chars)
{
  DWORD temp;
  BOOL result;

  result = DeviceIoControl(h, (DWORD)IOCTL_FASTCOM_SET_FRAME_LENGTH, 
                           &num_chars, sizeof(num_chars), 
                           NULL, 0, 
                           &temp, (LPOVERLAPPED)NULL);

  return (result == TRUE) ? ERROR_SUCCESS : GetLastError();
}

/******************************************************************************/
/*!

  \brief Gets the frame length.

  \param[in] h 
    HANDLE to the port
  \param[out] num_chars 
    the number of characters sent per frame
      
  \return 0 
    if the operation completed successfully
  \return >= 1 
    if the operation failed (see MSDN 'System Error Codes')
  
  \note
    This is only supported on the FSCC cards.

*/
/******************************************************************************/
int serialfc_get_frame_length(HANDLE h, unsigned *num_chars)
{
  DWORD temp;
  BOOL result;

  result = DeviceIoControl(h, (DWORD)IOCTL_FASTCOM_GET_FRAME_LENGTH, 
                           NULL, 0, 
                           num_chars, sizeof(*num_chars), 
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
int serialfc_write(HANDLE h, char *buf, unsigned size, unsigned *bytes_written, 
                   OVERLAPPED *o)
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
int serialfc_read(HANDLE h, char *buf, unsigned size, unsigned *bytes_read, 
                  OVERLAPPED *o)
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

/******************************************************************************/
/*!

  \brief Turns on Thales master mode

  \param[in] h 
    HANDLE to the port
      
  \return 0 
    if the operation completed successfully
  \return >= 1 
    if the operation failed (see MSDN 'System Error Codes')
  
  \note
    This is only supported with the Thales custom firmware/driver.

*/
/******************************************************************************/
int serialfc_thales_enable_master_mode(HANDLE h)
{
  DWORD temp;
  BOOL result;

  result = DeviceIoControl(h, (DWORD)IOCTL_THALES_ENABLE_MASTER_MODE, 
                           NULL, 0, 
                           NULL, 0, 
                           &temp, (LPOVERLAPPED)NULL);

  return (result == TRUE) ? ERROR_SUCCESS : GetLastError();
}

/******************************************************************************/
/*!

  \brief Turns on Thales slave mode

  \param[in] h 
    HANDLE to the port
      
  \return 0 
    if the operation completed successfully
  \return >= 1 
    if the operation failed (see MSDN 'System Error Codes')
  
  \note
    This is only supported with the Thales custom firmware/driver.

*/
/******************************************************************************/
int serialfc_thales_enable_slave_mode(HANDLE h)
{
  DWORD temp;
  BOOL result;

  result = DeviceIoControl(h, (DWORD)IOCTL_THALES_ENABLE_SLAVE_MODE, 
                           NULL, 0, 
                           NULL, 0, 
                           &temp, (LPOVERLAPPED)NULL);

  return (result == TRUE) ? ERROR_SUCCESS : GetLastError();
}