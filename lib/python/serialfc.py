import serial
import win32file
import struct

def CTL_CODE(DeviceType, Function, Method, Access):
    return (DeviceType<<16) | (Access << 14) | (Function << 2) | Method

SERIALFC_IOCTL_MAGIC = 0x8019
METHOD_BUFFERED = 0
FILE_ANY_ACCESS = 0

IOCTL_FASTCOM_ENABLE_RS485 = CTL_CODE(SERIALFC_IOCTL_MAGIC, 0x800, METHOD_BUFFERED, FILE_ANY_ACCESS)
IOCTL_FASTCOM_DISABLE_RS485 = CTL_CODE(SERIALFC_IOCTL_MAGIC, 0x801, METHOD_BUFFERED, FILE_ANY_ACCESS)
IOCTL_FASTCOM_GET_RS485 = CTL_CODE(SERIALFC_IOCTL_MAGIC, 0x802, METHOD_BUFFERED, FILE_ANY_ACCESS)

IOCTL_FASTCOM_ENABLE_ECHO_CANCEL = CTL_CODE(SERIALFC_IOCTL_MAGIC, 0x803, METHOD_BUFFERED, FILE_ANY_ACCESS)
IOCTL_FASTCOM_DISABLE_ECHO_CANCEL = CTL_CODE(SERIALFC_IOCTL_MAGIC, 0x804, METHOD_BUFFERED, FILE_ANY_ACCESS)
IOCTL_FASTCOM_GET_ECHO_CANCEL = CTL_CODE(SERIALFC_IOCTL_MAGIC, 0x805, METHOD_BUFFERED, FILE_ANY_ACCESS)

IOCTL_FASTCOM_ENABLE_TERMINATION = CTL_CODE(SERIALFC_IOCTL_MAGIC, 0x806, METHOD_BUFFERED, FILE_ANY_ACCESS)
IOCTL_FASTCOM_DISABLE_TERMINATION = CTL_CODE(SERIALFC_IOCTL_MAGIC, 0x807, METHOD_BUFFERED, FILE_ANY_ACCESS)
IOCTL_FASTCOM_GET_TERMINATION = CTL_CODE(SERIALFC_IOCTL_MAGIC, 0x808, METHOD_BUFFERED, FILE_ANY_ACCESS)

IOCTL_FASTCOM_SET_SAMPLE_RATE = CTL_CODE(SERIALFC_IOCTL_MAGIC, 0x809, METHOD_BUFFERED, FILE_ANY_ACCESS)
IOCTL_FASTCOM_GET_SAMPLE_RATE = CTL_CODE(SERIALFC_IOCTL_MAGIC, 0x80A, METHOD_BUFFERED, FILE_ANY_ACCESS)

IOCTL_FASTCOM_SET_TX_TRIGGER = CTL_CODE(SERIALFC_IOCTL_MAGIC, 0x80B, METHOD_BUFFERED, FILE_ANY_ACCESS)
IOCTL_FASTCOM_GET_TX_TRIGGER = CTL_CODE(SERIALFC_IOCTL_MAGIC, 0x80C, METHOD_BUFFERED, FILE_ANY_ACCESS)

IOCTL_FASTCOM_SET_RX_TRIGGER = CTL_CODE(SERIALFC_IOCTL_MAGIC, 0x80D, METHOD_BUFFERED, FILE_ANY_ACCESS)
IOCTL_FASTCOM_GET_RX_TRIGGER = CTL_CODE(SERIALFC_IOCTL_MAGIC, 0x80E, METHOD_BUFFERED, FILE_ANY_ACCESS)

IOCTL_FASTCOM_SET_CLOCK_RATE = CTL_CODE(SERIALFC_IOCTL_MAGIC, 0x80F, METHOD_BUFFERED, FILE_ANY_ACCESS)

IOCTL_FASTCOM_ENABLE_ISOCHRONOUS = CTL_CODE(SERIALFC_IOCTL_MAGIC, 0x810, METHOD_BUFFERED, FILE_ANY_ACCESS)
IOCTL_FASTCOM_DISABLE_ISOCHRONOUS = CTL_CODE(SERIALFC_IOCTL_MAGIC, 0x811, METHOD_BUFFERED, FILE_ANY_ACCESS)
IOCTL_FASTCOM_GET_ISOCHRONOUS = CTL_CODE(SERIALFC_IOCTL_MAGIC, 0x812, METHOD_BUFFERED, FILE_ANY_ACCESS)

IOCTL_FASTCOM_ENABLE_EXTERNAL_TRANSMIT = CTL_CODE(SERIALFC_IOCTL_MAGIC, 0x813, METHOD_BUFFERED, FILE_ANY_ACCESS)
IOCTL_FASTCOM_DISABLE_EXTERNAL_TRANSMIT = CTL_CODE(SERIALFC_IOCTL_MAGIC, 0x814, METHOD_BUFFERED, FILE_ANY_ACCESS)
IOCTL_FASTCOM_GET_EXTERNAL_TRANSMIT = CTL_CODE(SERIALFC_IOCTL_MAGIC, 0x815, METHOD_BUFFERED, FILE_ANY_ACCESS)

IOCTL_FASTCOM_SET_FRAME_LENGTH = CTL_CODE(SERIALFC_IOCTL_MAGIC, 0x816, METHOD_BUFFERED, FILE_ANY_ACCESS)
IOCTL_FASTCOM_GET_FRAME_LENGTH = CTL_CODE(SERIALFC_IOCTL_MAGIC, 0x817, METHOD_BUFFERED, FILE_ANY_ACCESS)

IOCTL_FASTCOM_GET_CARD_TYPE = CTL_CODE(SERIALFC_IOCTL_MAGIC, 0x818, METHOD_BUFFERED, FILE_ANY_ACCESS)


CARD_TYPE_PCI, CARD_TYPE_PCIe, CARD_TYPE_FSCC, CARD_TYPE_UNKNOWN = range(4)


class Port(serial.Serial):

    def _set_rs485(self, status):
        """Sets the value of the rs485 setting."""
        if status:
        	win32file.DeviceIoControl(self.hComPort, IOCTL_FASTCOM_ENABLE_RS485, None, 0, None)
        else:
        	win32file.DeviceIoControl(self.hComPort, IOCTL_FASTCOM_DISABLE_RS485, None, 0, None)

    def _get_rs485(self):
        """Gets the value of the rs485 setting."""
        buf_size = struct.calcsize("?")
        buf = win32file.DeviceIoControl(self.hComPort, IOCTL_FASTCOM_GET_RS485, None, buf_size, None)
        value = struct.unpack("?", buf)

        if (value[0]):
            return True
        else:
            return False

    rs485 = property(fset=_set_rs485, fget=_get_rs485)

    def _set_echo_cancel(self, status):
        """Sets the value of the echo_cancel setting."""
        if status:
        	win32file.DeviceIoControl(self.hComPort, IOCTL_FASTCOM_ENABLE_ECHO_CANCEL, None, 0, None)
        else:
        	win32file.DeviceIoControl(self.hComPort, IOCTL_FASTCOM_DISABLE_ECHO_CANCEL, None, 0, None)

    def _get_echo_cancel(self):
        """Gets the value of the echo_cancel setting."""
        buf_size = struct.calcsize("?")
        buf = win32file.DeviceIoControl(self.hComPort, IOCTL_FASTCOM_GET_ECHO_CANCEL, None, buf_size, None)
        value = struct.unpack("?", buf)

        if (value[0]):
            return True
        else:
            return False

    echo_cancel = property(fset=_set_echo_cancel, fget=_get_echo_cancel)

    def _set_termination(self, status):
        """Sets the value of the termination setting."""
        if status:
        	win32file.DeviceIoControl(self.hComPort, IOCTL_FASTCOM_ENABLE_TERMINATION, None, 0, None)
        else:
        	win32file.DeviceIoControl(self.hComPort, IOCTL_FASTCOM_DISABLE_TERMINATION, None, 0, None)

    def _get_termination(self):
        """Gets the value of the termination setting."""
        buf_size = struct.calcsize("?")
        buf = win32file.DeviceIoControl(self.hComPort, IOCTL_FASTCOM_GET_TERMINATION, None, buf_size, None)
        value = struct.unpack("?", buf)

        if (value[0]):
            return True
        else:
            return False

    termination = property(fset=_set_termination, fget=_get_termination)

    def _set_sample_rate(self, rate):
        """Sets the value of the sample_rate setting."""
        value = struct.pack("I", rate)
        win32file.DeviceIoControl(self.hComPort, IOCTL_FASTCOM_SET_SAMPLE_RATE, value, 0, None)

    def _get_sample_rate(self):
        """Gets the value of the sample_rate setting."""
        buf_size = struct.calcsize("I")
        buf = win32file.DeviceIoControl(self.hComPort, IOCTL_FASTCOM_GET_SAMPLE_RATE, None, buf_size, None)
        value = struct.unpack("I", buf)

        return value[0]

    sample_rate = property(fset=_set_sample_rate, fget=_get_sample_rate)

    def _set_tx_trigger(self, level):
        """Sets the value of the tx_trigger setting."""
        value = struct.pack("I", level)
        win32file.DeviceIoControl(self.hComPort, IOCTL_FASTCOM_SET_TX_TRIGGER, value, 0, None)

    def _get_tx_trigger(self):
        """Gets the value of the tx_trigger setting."""
        buf_size = struct.calcsize("I")
        buf = win32file.DeviceIoControl(self.hComPort, IOCTL_FASTCOM_GET_TX_TRIGGER, None, buf_size, None)
        value = struct.unpack("I", buf)

        return value[0]

    tx_trigger = property(fset=_set_tx_trigger, fget=_get_tx_trigger)

    def _set_rx_trigger(self, level):
        """Sets the value of the rx_trigger setting."""
        value = struct.pack("I", level)
        win32file.DeviceIoControl(self.hComPort, IOCTL_FASTCOM_SET_RX_TRIGGER, value, 0, None)

    def _get_rx_trigger(self):
        """Gets the value of the rx_trigger setting."""
        buf_size = struct.calcsize("I")
        buf = win32file.DeviceIoControl(self.hComPort, IOCTL_FASTCOM_GET_RX_TRIGGER, None, buf_size, None)
        value = struct.unpack("I", buf)

        return value[0]

    rx_trigger = property(fset=_set_rx_trigger, fget=_get_rx_trigger)

    def _set_clock_rate(self, rate):
        """Sets the value of the clock_rate setting."""
        value = struct.pack("I", rate)
        win32file.DeviceIoControl(self.hComPort, IOCTL_FASTCOM_SET_CLOCK_RATE, value, 0, None)

    clock_rate = property(fset=_set_clock_rate, fget=None)

    def enable_isochronous(self, mode):
        """Enables isochronous mode."""
        value = struct.pack("I", mode)
        win32file.DeviceIoControl(self.hComPort, IOCTL_FASTCOM_ENABLE_ISOCHRONOUS, value, 0, None)

    def disable_isochronous(self):
        """Disables isochronous mode."""
        win32file.DeviceIoControl(self.hComPort, IOCTL_FASTCOM_DISABLE_ISOCHRONOUS, None, 0, None)

    def get_isochronous(self):
        """Gets the value of the isochronous setting."""
        buf_size = struct.calcsize("i")
        buf = win32file.DeviceIoControl(self.hComPort, IOCTL_FASTCOM_GET_ISOCHRONOUS, None, buf_size, None)
        value = struct.unpack("i", buf)

        return value[0]

    def enable_external_transmit(self, num_frames):
        """Enables external transmit mode."""
        value = struct.pack("I", num_frames)
        win32file.DeviceIoControl(self.hComPort, IOCTL_FASTCOM_ENABLE_EXTERNAL_TRANSMIT, value, 0, None)

    def disable_external_transmit(self):
        """Disables external transmit mode."""
        win32file.DeviceIoControl(self.hComPort, IOCTL_FASTCOM_DISABLE_EXTERNAL_TRANSMIT, None, 0, None)

    def get_external_transmit(self):
        """Gets the value of the external transmit setting."""
        buf_size = struct.calcsize("i")
        buf = win32file.DeviceIoControl(self.hComPort, IOCTL_FASTCOM_GET_EXTERNAL_TRANSMIT, None, buf_size, None)
        value = struct.unpack("i", buf)

        return value[0]

    def _set_frame_length(self, num_chars):
        """Sets the value of the frame length setting."""
        value = struct.pack("I", num_chars)
        win32file.DeviceIoControl(self.hComPort, IOCTL_FASTCOM_SET_FRAME_LENGTH, value, 0, None)

    def _get_frame_length(self):
        """Gets the value of the frame length setting."""
        buf_size = struct.calcsize("I")
        buf = win32file.DeviceIoControl(self.hComPort, IOCTL_FASTCOM_GET_FRAME_LENGTH, None, buf_size, None)
        value = struct.unpack("I", buf)

        return value[0]

    frame_length = property(fset=_set_frame_length, fget=_get_frame_length)

    def _get_card_type(self):
        buf_size = struct.calcsize("I")
        buf = win32file.DeviceIoControl(self.hComPort, IOCTL_FASTCOM_GET_CARD_TYPE, None, buf_size, None)
        value = struct.unpack("I", buf)

        return value[0]

    _card_type = property(fget=_get_card_type)

if __name__ == '__main__':
    p = Port('COM3')

    try:
    	print("RS485", p.rs485)
    except:
    	pass

    try:
    	print("Termination", p.termination)
    except:
    	pass

    try:

        print("Isochronous", p.get_isochronous())
    except:
        pass

    print("Echo Cancel", p.echo_cancel)
    print("Sample Rate", p.sample_rate)
    print("Tx Trigger", p.tx_trigger)
    print("Rx Trigger", p.rx_trigger)

    p.rs485 = False
    p.echo_cancel = False
    p.sample_rate = 16
    p.baudrate = 115200

    p.write("UUUUU".encode())