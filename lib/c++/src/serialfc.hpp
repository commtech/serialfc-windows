#ifndef SERIALFC_HPP
#define SERIALFC_HPP

#pragma warning(disable:4290)

#include <Windows.h>
#include <string>

#include "sys_exception.hpp"

namespace SerialFC {

	class __declspec(dllexport) Port {
	public:
		Port(unsigned port_num) throw(SystemException);
		Port(unsigned port_num, bool overlapped) throw(SystemException);

		Port(const Port &other) throw(SystemException); /* Untested */
		Port& operator=(const Port &other) throw(SystemException); /* Untested */

		virtual ~Port(void) throw(SystemException);

		void EnableRS485(void) throw(SystemException);
		void DisableRS485(void) throw(SystemException);
		bool GetRS485(void) throw(SystemException);
		void EnableEchoCancel(void) throw(SystemException);
		void DisableEchoCancel(void) throw(SystemException);
		bool GetEchoCancel(void) throw(SystemException);
		void EnableTermination(void) throw(SystemException);
		void DisableTermination(void) throw(SystemException);
		bool GetTermination(void) throw(SystemException);
		void SetSampleRate(unsigned rate) throw(SystemException);
		unsigned GetSampleRate(void) throw(SystemException);
		void SetTxTrigger(unsigned level) throw(SystemException);
		unsigned GetTxTrigger(void) throw(SystemException);
		void SetRxTrigger(unsigned level) throw(SystemException);
		unsigned GetRxTrigger(void) throw(SystemException);

		unsigned Write(const char *buf, unsigned size, OVERLAPPED *o) throw(SystemException);
		unsigned Write(const char *buf, unsigned size) throw(SystemException);
		unsigned Write(const std::string &s) throw(SystemException);
		unsigned Read(char *buf, unsigned size, OVERLAPPED *o) throw(SystemException);
		unsigned Read(char *buf, unsigned size) throw(SystemException);

	protected:
		void init(unsigned port_num, bool overlapped) throw(SystemException);
		void init(unsigned port_num, bool overlapped, HANDLE h) throw();
		void cleanup(void) throw(SystemException);

	private:
		HANDLE _h;
		unsigned _port_num;
		bool _overlapped;
	};

} /* namespace SerialFC */

#endif