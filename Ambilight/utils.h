#pragma once
#include "stdafx.h"


namespace Mambi 
{
	class AcquireMutex {
	public:
		AcquireMutex(HANDLE handle, DWORD timeout = INFINITE) : _handle(handle)
		{
			_state = WaitForSingleObject(handle, timeout);
		}

		~AcquireMutex()
		{
			Release();
		}

		inline bool Release()
		{
			if (_handle != NULL && _state == WAIT_OBJECT_0)
			{
				bool res = ReleaseMutex(_handle);
				_handle = NULL;
				return res;
			}
			return true;
		}

		inline operator bool() const { return _state == WAIT_OBJECT_0; }
		inline bool AcquireSuccess() const { return _state == WAIT_OBJECT_0; }
		inline bool Timeout() const { return _state == WAIT_TIMEOUT; }
		inline bool Abandoned() const { return _state == WAIT_ABANDONED; }

	private:
		HANDLE _handle;
		DWORD _state;
	};
}