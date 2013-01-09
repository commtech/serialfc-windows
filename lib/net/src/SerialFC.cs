using System;
using System.IO;
using System.Runtime.InteropServices;
using System.Text;
using Microsoft.Win32.SafeHandles;
using System.Threading;
using System.IO.Ports;
using System.Reflection;

namespace SerialFC
{
    public class Port : SerialPort
    {
        const string DLL_PATH = "cserialfc.dll";

        private SafeFileHandle Handle
        {
            get
            {
                object stream = typeof(SerialPort).GetField("internalSerialStream", BindingFlags.NonPublic | BindingFlags.Instance).GetValue(this);
                return (SafeFileHandle)stream.GetType().GetField("_handle", BindingFlags.NonPublic | BindingFlags.Instance).GetValue(stream);
            }
        }
        
        [DllImport(DLL_PATH, CallingConvention = CallingConvention.Cdecl)]
        private static extern int serialfc_enable_rs485(SafeFileHandle h);
        
        [DllImport(DLL_PATH, CallingConvention = CallingConvention.Cdecl)]
        private static extern int serialfc_disable_rs485(SafeFileHandle h);

        [DllImport(DLL_PATH, CallingConvention = CallingConvention.Cdecl)]
        private static extern int serialfc_get_rs485(SafeFileHandle h, out bool status);

        public bool RS485
        {
            set
            {
                int e = 0;

                if (value == true)
                    e = serialfc_enable_rs485(this.Handle);
                else
                    e = serialfc_disable_rs485(this.Handle);

                if (e >= 1)
                    throw new Exception(e.ToString());
            }

            get
            {
                bool status;

                int e = serialfc_get_rs485(this.Handle, out status);

                if (e >= 1)
                    throw new Exception(e.ToString());

                return status;
            }
        }
        
        [DllImport(DLL_PATH, CallingConvention = CallingConvention.Cdecl)]
        private static extern int serialfc_enable_echo_cancel(SafeFileHandle h);
        
        [DllImport(DLL_PATH, CallingConvention = CallingConvention.Cdecl)]
        private static extern int serialfc_disable_echo_cancel(SafeFileHandle h);

        [DllImport(DLL_PATH, CallingConvention = CallingConvention.Cdecl)]
        private static extern int serialfc_get_echo_cancel(SafeFileHandle h, out bool status);

        public bool EchoCancel
        {
            set
            {
                int e = 0;

                if (value == true)
                    e = serialfc_enable_echo_cancel(this.Handle);
                else
                    e = serialfc_disable_echo_cancel(this.Handle);

                if (e >= 1)
                    throw new Exception(e.ToString());
            }

            get
            {
                bool status;

                int e = serialfc_get_echo_cancel(this.Handle, out status);

                if (e >= 1)
                    throw new Exception(e.ToString());

                return status;
            }
        }
        
        [DllImport(DLL_PATH, CallingConvention = CallingConvention.Cdecl)]
        private static extern int serialfc_enable_termination(SafeFileHandle h);
        
        [DllImport(DLL_PATH, CallingConvention = CallingConvention.Cdecl)]
        private static extern int serialfc_disable_termination(SafeFileHandle h);

        [DllImport(DLL_PATH, CallingConvention = CallingConvention.Cdecl)]
        private static extern int serialfc_get_termination(SafeFileHandle h, out bool status);

        public bool Termination
        {
            set
            {
                int e = 0;

                if (value == true)
                    e = serialfc_enable_termination(this.Handle);
                else
                    e = serialfc_disable_termination(this.Handle);

                if (e >= 1)
                    throw new Exception(e.ToString());
            }

            get
            {
                bool status;

                int e = serialfc_get_termination(this.Handle, out status);

                if (e >= 1)
                    throw new Exception(e.ToString());

                return status;
            }
        }

        [DllImport(DLL_PATH, CallingConvention = CallingConvention.Cdecl)]
        private static extern int serialfc_set_sample_rate(SafeFileHandle h, uint rate);

        [DllImport(DLL_PATH, CallingConvention = CallingConvention.Cdecl)]
        private static extern int serialfc_get_sample_rate(SafeFileHandle h, out uint rate);

        public uint SampleRate
        {
            set
            {
                int e = serialfc_set_sample_rate(this.Handle, value);

                if (e >= 1)
                    throw new Exception(e.ToString());
            }

            get
            {
                uint rate;

                int e = serialfc_get_sample_rate(this.Handle, out rate);

                if (e >= 1)
                    throw new Exception(e.ToString());

                return rate;
            }
        }

        [DllImport(DLL_PATH, CallingConvention = CallingConvention.Cdecl)]
        private static extern int serialfc_set_tx_trigger(SafeFileHandle h, uint level);

        [DllImport(DLL_PATH, CallingConvention = CallingConvention.Cdecl)]
        private static extern int serialfc_get_tx_trigger(SafeFileHandle h, out uint level);

        public uint TxTrigger
        {
            set
            {
                int e = serialfc_set_tx_trigger(this.Handle, value);

                if (e >= 1)
                    throw new Exception(e.ToString());
            }

            get
            {
                uint level;

                int e = serialfc_get_tx_trigger(this.Handle, out level);

                if (e >= 1)
                    throw new Exception(e.ToString());

                return level;
            }
        }

        [DllImport(DLL_PATH, CallingConvention = CallingConvention.Cdecl)]
        private static extern int serialfc_set_rx_trigger(SafeFileHandle h, uint level);

        [DllImport(DLL_PATH, CallingConvention = CallingConvention.Cdecl)]
        private static extern int serialfc_get_rx_trigger(SafeFileHandle h, out uint level);

        public uint RxTrigger
        {
            set
            {
                int e = serialfc_set_rx_trigger(this.Handle, value);

                if (e >= 1)
                    throw new Exception(e.ToString());
            }

            get
            {
                uint level;

                int e = serialfc_get_rx_trigger(this.Handle, out level);

                if (e >= 1)
                    throw new Exception(e.ToString());

                return level;
            }
        }
    }
}