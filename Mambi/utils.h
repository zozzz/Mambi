#pragma once
#include "Console.h"

#define __MAMBI_STRINGIFY(x) #x
#define MAMBI_STRINGIFY(x) __MAMBI_STRINGIFY(x)


namespace Mambi 
{
	template <typename Fn>
	struct _Finally : public Fn
	{
		_Finally(Fn&& Func) : Fn(std::forward<Fn>(Func)) {}
		_Finally(const _Finally&);
		~_Finally() { this->operator()(); }
	};

	template <typename Fn>
	inline _Finally<Fn> Finally(Fn&& Func)
	{
		return { std::forward<Fn>(Func) };
	}


	class AcquireMutex {
	public:
		AcquireMutex(HANDLE handle, DWORD timeout=INFINITE): _handle(handle) 
		{
			//Console::WriteLine("AcquireMutex::AcquireMutex");
			_state = WaitForSingleObject(handle, timeout);
		}

		~AcquireMutex() 
		{
			//Console::WriteLine("AcquireMutex::~AcquireMutex %p", _handle);
			if (!Release())
			{	
				// throw std::exception("Release Mutex Failed");				
			}			
		}

		inline bool Release() 
		{ 
			if (_handle != NULL && _state == WAIT_OBJECT_0)
			{
				//Console::WriteLine("AcquireMutex::Release %p", _handle);
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


	template<typename T>
	class Buffer 
	{
	public:
		Buffer(): _data(NULL), _size(0) {};
		~Buffer() 
		{
			if (_data != NULL)
			{
				T* tmp = _data;
				_data = NULL;
				free(tmp);
			}
						
		}

		inline T* Data() const { return _data; }
		inline auto Size() const { return _size; }
		inline void Zero() { ZeroMemory(_data, sizeof(T) * _size); }

		bool EnsureSize(size_t required) 
		{
			if (required > _size)
			{
				if (_data == NULL)
				{
					_data = (T*)malloc(sizeof(T) * required);
					if (_data == NULL)
					{
						return false;
					}					
				}
				else
				{
					_data = (T*)realloc(_data, sizeof(T) * required);
					if (_data == NULL)
					{
						return false;
					}					
				}				
				_size = required;
			}
			return true;
		}

		inline bool Append(size_t pos, T* data, size_t dataSize)
		{
			return memcpy_s(_data + pos, _size, data, dataSize) == 0;
		}

		T& operator[](size_t idx) 
		{ 
			assert(idx >= 0 && idx < _size);
			return _data[idx]; 
		}

		const T& operator[](size_t idx) const 
		{ 
			assert(idx >= 0 && idx < _size);
			return _data[idx]; 
		}

		operator const T*() const { return _data; }

		Buffer(Buffer const&) = delete;
		void operator=(Buffer const&) = delete;

	private:
		T* _data;
		size_t _size;		
	};
}