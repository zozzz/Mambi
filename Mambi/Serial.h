#pragma once
#include "stdafx.h"
#include "Messages.h"
#include "Console.h"


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
				GENERIC_WRITE,
				0,
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
			}
			else
			{
				DWORD errCode = GetLastError();
				//throw new std::exception("Failed to connect to port");
				ErrorAlert("Error", "Cannot connect to ledstrip, maybe port value is invalid, or the device is not connected");
				PostQuitMessage(1);
			}

		};

		~Serial() 
		{
			if (_handle != NULL)
			{
				CloseHandle(_handle);
			}
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
	};
}