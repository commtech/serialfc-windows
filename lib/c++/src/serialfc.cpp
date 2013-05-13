#include <cstdio>

#include <serialfc.h>

#include "serialfc.hpp"

namespace SerialFC {

/// <param name="port_num">Used to indicate status.</param>
Port::Port(unsigned port_num)
{
	init(port_num, false);
}

Port::Port(unsigned port_num, bool overlapped)
{
	init(port_num, overlapped);
}

Port::~Port(void)
{
	cleanup();
}

void Port::init(unsigned port_num, bool overlapped, HANDLE h)
{
	_port_num = port_num;
	_overlapped = overlapped;
	_h = h;
} 

void Port::init(unsigned port_num, bool overlapped)
{
	_port_num = port_num;
	_overlapped = overlapped;

	int e = serialfc_connect(port_num, overlapped, &_h);

	switch (e) {
	case ERROR_SUCCESS:
		break;

	case ERROR_FILE_NOT_FOUND:
		throw PortNotFoundException(port_num);

	case ERROR_ACCESS_DENIED:
		throw InsufficientPermissionsException();

	default:
		throw SystemException(e);
	}
} 

void Port::cleanup(void)
{
	int e = serialfc_disconnect(_h);

	if (e)
			throw SystemException(e);
} 

Port::Port(const Port &other)
{
	init(other._port_num, other._overlapped);
}

Port& Port::operator=(const Port &other)
{
	if (this != &other) {
		HANDLE h2;
		int e = serialfc_connect(other._port_num, other._overlapped, &h2);

		if (e) {
				throw SystemException(e);
		}
		else {
				cleanup();
				init(other._port_num, other._overlapped, h2);
		}
	}

	return *this;
}

void Port::EnableRS485(void) throw(SystemException)
{
	int e = serialfc_enable_rs485(_h);

	if (e)
		throw SystemException(e);
}

void Port::DisableRS485(void) throw(SystemException)
{
	int e = serialfc_disable_rs485(_h);

	if (e)
		throw SystemException(e);
}

bool Port::GetRS485(void) throw(SystemException)
{
	BOOL status;

	int e = serialfc_get_rs485(_h, &status);

	if (e)
		throw SystemException(e);

	return status != 0;
}

void Port::EnableEchoCancel(void) throw(SystemException)
{
	int e = serialfc_enable_echo_cancel(_h);

	if (e)
		throw SystemException(e);
}

void Port::DisableEchoCancel(void) throw(SystemException)
{
	int e = serialfc_disable_echo_cancel(_h);

	if (e)
		throw SystemException(e);
}

bool Port::GetEchoCancel(void) throw(SystemException)
{
	BOOL status;

	int e = serialfc_get_echo_cancel(_h, &status);

	if (e)
		throw SystemException(e);

	return status != 0;
}

void Port::EnableTermination(void) throw(SystemException)
{
	int e = serialfc_enable_termination(_h);

	if (e)
		throw SystemException(e);
}

void Port::DisableTermination(void) throw(SystemException)
{
	int e = serialfc_disable_termination(_h);

	if (e)
		throw SystemException(e);
}

bool Port::GetTermination(void) throw(SystemException)
{
	BOOL status;

	int e = serialfc_get_termination(_h, &status);

	if (e)
		throw SystemException(e);

	return status != 0;
}

void Port::SetSampleRate(unsigned rate) throw(SystemException)
{
	int e = serialfc_set_sample_rate(_h, rate);

	if (e)
		throw SystemException(e);
}

unsigned Port::GetSampleRate(void) throw(SystemException)
{
	unsigned rate;

	int e = serialfc_get_sample_rate(_h, &rate);

	if (e)
		throw SystemException(e);

	return rate;
}

void Port::SetTxTrigger(unsigned level) throw(SystemException)
{
	int e = serialfc_set_tx_trigger(_h, level);

	if (e)
		throw SystemException(e);
}

unsigned Port::GetTxTrigger(void) throw(SystemException)
{
	unsigned level;

	int e = serialfc_get_tx_trigger(_h, &level);

	if (e)
		throw SystemException(e);

	return level;
}

void Port::SetRxTrigger(unsigned level) throw(SystemException)
{
	int e = serialfc_set_rx_trigger(_h, level);

	if (e)
		throw SystemException(e);
}

unsigned Port::GetRxTrigger(void) throw(SystemException)
{
	unsigned level;

	int e = serialfc_get_rx_trigger(_h, &level);

	if (e)
		throw SystemException(e);

	return level;
}

void Port::SetClockRate(unsigned rate) throw(SystemException)
{
	int e = serialfc_set_clock_rate(_h, rate);

	if (e)
		throw SystemException(e);
}

void Port::EnableIsochronous(unsigned mode) throw(SystemException)
{
	int e = serialfc_enable_isochronous(_h, mode);

	if (e)
		throw SystemException(e);
}

void Port::DisableIsochronous() throw(SystemException)
{
	int e = serialfc_disable_isochronous(_h);

	if (e)
		throw SystemException(e);
}

int Port::GetIsochronous(void) throw(SystemException)
{
	int mode;

	int e = serialfc_get_isochronous(_h, &mode);

	if (e)
		throw SystemException(e);

	return mode;
}

void Port::EnableExternalTransmit(unsigned num_chars) throw(SystemException)
{
	int e = serialfc_enable_external_transmit(_h, num_chars);

	if (e)
		throw SystemException(e);
}

void Port::DisableExternalTransmit() throw(SystemException)
{
	int e = serialfc_disable_external_transmit(_h);

	if (e)
		throw SystemException(e);
}

unsigned Port::GetExternalTransmit(void) throw(SystemException)
{
	unsigned num_chars;

	int e = serialfc_get_external_transmit(_h, &num_chars);

	if (e)
		throw SystemException(e);

	return num_chars;
}

unsigned Port::Write(const char *buf, unsigned size, OVERLAPPED *o)
{
    unsigned bytes_written;

    int e = serialfc_write(_h, (char *)buf, size, &bytes_written, o);

    if (e)
		throw SystemException(e);

    return bytes_written;
}

unsigned Port::Write(const char *buf, unsigned size) 
{
	return Write(buf, size, 0);
}

unsigned Port::Write(const std::string &s)
{
	return Write(s.c_str(), s.length());
}

unsigned Port::Read(char *buf, unsigned size, OVERLAPPED *o)
{
    unsigned bytes_read;

    int e = serialfc_read(_h, buf, size, &bytes_read, o);

    if (e)
		throw SystemException(e);

    return bytes_read;
}

unsigned Port::Read(char *buf, unsigned size)
{
	return Read(buf, size, (OVERLAPPED *)0);
}

Port::operator HANDLE()
{
	return _h;
}

} /* namespace SerialFC */