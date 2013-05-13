/*! \file */ 

#ifndef SERIALFC_H
#define SERIALFC_H

#ifdef __cplusplus
extern "C" 
{
#endif

#include <Windows.h>

#define SERIALFC_IOCTL_MAGIC 0x8019

#define IOCTL_FASTCOM_ENABLE_RS485 CTL_CODE(SERIALFC_IOCTL_MAGIC, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_FASTCOM_DISABLE_RS485 CTL_CODE(SERIALFC_IOCTL_MAGIC, 0x801, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_FASTCOM_GET_RS485 CTL_CODE(SERIALFC_IOCTL_MAGIC, 0x802, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_FASTCOM_ENABLE_ECHO_CANCEL CTL_CODE(SERIALFC_IOCTL_MAGIC, 0x803, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_FASTCOM_DISABLE_ECHO_CANCEL CTL_CODE(SERIALFC_IOCTL_MAGIC, 0x804, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_FASTCOM_GET_ECHO_CANCEL CTL_CODE(SERIALFC_IOCTL_MAGIC, 0x805, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_FASTCOM_ENABLE_TERMINATION CTL_CODE(SERIALFC_IOCTL_MAGIC, 0x806, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_FASTCOM_DISABLE_TERMINATION CTL_CODE(SERIALFC_IOCTL_MAGIC, 0x807, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_FASTCOM_GET_TERMINATION CTL_CODE(SERIALFC_IOCTL_MAGIC, 0x808, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_FASTCOM_SET_SAMPLE_RATE CTL_CODE(SERIALFC_IOCTL_MAGIC, 0x809, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_FASTCOM_GET_SAMPLE_RATE CTL_CODE(SERIALFC_IOCTL_MAGIC, 0x80A, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_FASTCOM_SET_TX_TRIGGER CTL_CODE(SERIALFC_IOCTL_MAGIC, 0x80B, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_FASTCOM_GET_TX_TRIGGER CTL_CODE(SERIALFC_IOCTL_MAGIC, 0x80C, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_FASTCOM_SET_RX_TRIGGER CTL_CODE(SERIALFC_IOCTL_MAGIC, 0x80D, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_FASTCOM_GET_RX_TRIGGER CTL_CODE(SERIALFC_IOCTL_MAGIC, 0x80E, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_FASTCOM_SET_CLOCK_RATE CTL_CODE(SERIALFC_IOCTL_MAGIC, 0x80F, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_FASTCOM_ENABLE_ISOCHRONOUS CTL_CODE(SERIALFC_IOCTL_MAGIC, 0x810, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_FASTCOM_DISABLE_ISOCHRONOUS CTL_CODE(SERIALFC_IOCTL_MAGIC, 0x811, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_FASTCOM_GET_ISOCHRONOUS CTL_CODE(SERIALFC_IOCTL_MAGIC, 0x812, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_FASTCOM_ENABLE_EXTERNAL_TRANSMIT CTL_CODE(SERIALFC_IOCTL_MAGIC, 0x813, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_FASTCOM_DISABLE_EXTERNAL_TRANSMIT CTL_CODE(SERIALFC_IOCTL_MAGIC, 0x814, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_FASTCOM_GET_EXTERNAL_TRANSMIT CTL_CODE(SERIALFC_IOCTL_MAGIC, 0x815, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_THALES_ENABLE_MASTER_MODE CTL_CODE(SERIALFC_IOCTL_MAGIC, 0x900, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_THALES_ENABLE_SLAVE_MODE CTL_CODE(SERIALFC_IOCTL_MAGIC, 0x901, METHOD_BUFFERED, FILE_ANY_ACCESS)


__declspec(dllexport) int serialfc_connect(unsigned port_num, BOOL overlapped, HANDLE *h);
__declspec(dllexport) int serialfc_enable_rs485(HANDLE h);
__declspec(dllexport) int serialfc_disable_rs485(HANDLE h);
__declspec(dllexport) int serialfc_get_rs485(HANDLE h, BOOL *status);
__declspec(dllexport) int serialfc_enable_echo_cancel(HANDLE h);
__declspec(dllexport) int serialfc_disable_echo_cancel(HANDLE h);
__declspec(dllexport) int serialfc_get_echo_cancel(HANDLE h, BOOL *status);
__declspec(dllexport) int serialfc_enable_termination(HANDLE h);
__declspec(dllexport) int serialfc_disable_termination(HANDLE h);
__declspec(dllexport) int serialfc_get_termination(HANDLE h, BOOL *status);
__declspec(dllexport) int serialfc_set_sample_rate(HANDLE h, unsigned rate);
__declspec(dllexport) int serialfc_get_sample_rate(HANDLE h, unsigned *rate);
__declspec(dllexport) int serialfc_set_tx_trigger(HANDLE h, unsigned level);
__declspec(dllexport) int serialfc_get_tx_trigger(HANDLE h, unsigned *level);
__declspec(dllexport) int serialfc_set_rx_trigger(HANDLE h, unsigned level);
__declspec(dllexport) int serialfc_get_rx_trigger(HANDLE h, unsigned *level);
__declspec(dllexport) int serialfc_set_clock_rate(HANDLE h, unsigned rate);
__declspec(dllexport) int serialfc_enable_isochronous(HANDLE h, unsigned mode);
__declspec(dllexport) int serialfc_disable_isochronous(HANDLE h);
__declspec(dllexport) int serialfc_get_isochronous(HANDLE h, int *mode);
__declspec(dllexport) int serialfc_write(HANDLE h, char *buf, unsigned size, unsigned *bytes_written, OVERLAPPED *o);
__declspec(dllexport) int serialfc_read(HANDLE h, char *buf, unsigned size, unsigned *bytes_read, OVERLAPPED *o);
__declspec(dllexport) int serialfc_disconnect(HANDLE h);

__declspec(dllexport) int serialfc_thales_enable_master_mode(HANDLE h);
__declspec(dllexport) int serialfc_thales_enable_slave_mode(HANDLE h);

#ifdef __cplusplus
}
#endif

#endif