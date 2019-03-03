#pragma once
#include "stdafx.h"
#include "Messages.h"
#include "Console.h"


#define MAMBI_SERIAL_READ_BUFFER 500
#define MAMBI_SERIAL_READ_TIMEOUT 10
#if _DEBUG
#  define MAMBI_SERIAL_READ 0
#else
#  define MAMBI_SERIAL_READ 0
#endif


namespace Mambi
{

	template<int BR>
	class Serial
	{
	public:
		Serial(const std::string& name): _name(name)
		{
			_handle = CreateFileA(
				_name.c_str(),
				GENERIC_READ | GENERIC_WRITE,
				FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
				NULL,
				OPEN_EXISTING,
				0,
				NULL);

			Console::WriteLine("PORT %s", _name.c_str());

			if (_handle == INVALID_HANDLE_VALUE)
			{
				DWORD errCode = GetLastError();
				throw new std::exception("Failed to connect to port");
			}

			DCB dcb;
			if (GetCommState(_handle, &dcb))
			{
				dcb.fBinary = true;
				dcb.BaudRate = BR;
				dcb.ByteSize = sizeof(BYTE);
				dcb.Parity = NOPARITY;
				dcb.StopBits = ONESTOPBIT;
				dcb.EofChar = 0;

				if (!SetCommState(_handle, &dcb))
				{
					throw new std::exception("Failed to init serial");
				}

#if MAMBI_SERIAL_READ
				COMMTIMEOUTS timeouts = { 
					0,  //interval timeout. 0 = not used
					0,  // read multiplier
					MAMBI_SERIAL_READ_TIMEOUT, // read constant (milliseconds)
					0,  // Write multiplier
					0   // Write Constant
				};
				if (!SetCommTimeouts(_handle, &timeouts))
				{
					throw new std::exception("Failed to init serial");
				}
#endif
			}
			else
			{
				DWORD errCode = GetLastError();
				//throw new std::exception("Failed to connect to port");
				ErrorAlert("Error", "Cannot connect to ledstrip, maybe port value is invalid, or the device is not connected");
				PostQuitMessage(1);
			}

#if MAMBI_SERIAL_READ
			WatchInput();
#endif

		};

		~Serial() 
		{
			if (_handle != NULL)
			{
				CloseHandle(_handle);
			}

#if MAMBI_SERIAL_READ
			if (_inputThread != NULL)
			{
				TerminateThread(_inputThread, 0);
				CloseHandle(_inputThread);
				_inputThread = NULL;
			}
#endif
		};

		inline auto& Name() const { return _name; }

		DWORD Write(const BYTE* data, DWORD size)
		{
			DWORD written;
			bool res = WriteFile(_handle, data, size, &written, NULL);
			return res ? written : 0;
		}

	private:
		HANDLE _handle;
		std::string _name;

#if MAMBI_SERIAL_READ
		HANDLE _inputThread = NULL;

		void WatchInput() 
		{
			if (_inputThread == NULL)
			{
				_inputThread = CreateThread(NULL, 0, WatcherThread, this, 0, NULL);
			}
		}

		static DWORD WINAPI WatcherThread(LPVOID param)
		{
			Serial<BR>* self = reinterpret_cast<Serial<BR>*>(param);
			char buff[MAMBI_SERIAL_READ_BUFFER + 1];
			DWORD readed;

			while (true)
			{
				ReadFile(self->_handle, buff, MAMBI_SERIAL_READ_BUFFER, &readed, NULL);
				if (readed > 0)
				{
					size_t tmpSize = readed + self->_name.size() + 9;
					char* tmp = new char[tmpSize];
					buff[readed] = 0;
					sprintf_s(tmp, tmpSize, "[%s] << %s", self->_name.c_str(), buff);
					tmp[tmpSize - 1] = 0;
					OutputDebugStringA(tmp);
					delete[] tmp;
				}

				Sleep(MAMBI_SERIAL_READ_TIMEOUT);
			}

			return 0;
		}
#endif
	};
}