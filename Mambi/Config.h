#pragma once
#include "stdafx.h"
#include "Messages.h"


#define MAMBI_CFG_EXISTS(__cfg, __name, __path) \
	if (!(__cfg).count(__name)) { \
		ErrorAlert("Error", "Missing '" ## __name ## "' property from '" ## __path ## "'"); \
		return false; \
	}

#define MAMBI_CFG_VNUM_INT(__cfg, __name, __path) \
	MAMBI_CFG_EXISTS(__cfg, __name, __path) \
	else if (!(__cfg)[__name].is_number_integer()) { \
		ErrorAlert("Error", "The '" ## __name ## "' value must be integer  in '" ## __path ## "'"); \
		return false; \
	}


#define MAMBI_CFG_VNUM_INT_RANGE(__cfg, __name, __path, _min, _max) \
	MAMBI_CFG_VNUM_INT(__cfg, __name, __path) \
	else if ((__cfg)[__name] < _min) { \
		ErrorAlert("Error", "The '" ## __name ## "' value must be greater or equal then " # _min " in '" ## __path ## "'"); \
		return false; \
	} else if ((__cfg)[__name] > _max) { \
		ErrorAlert("Error", "The '" ## __name ## "' value must be lower or equal then " # _max " in '" ## __path ## "'"); \
		return false; \
	}


#define MAMBI_CFG_IS_OBJECT(__cfg, __name, __path) \
	MAMBI_CFG_EXISTS(__cfg, __name, __path) \
	else if (!(__cfg)[__name].is_object()) { \
		ErrorAlert("Error", "The '" ## __name ## "' value must be object  in '" ## __path ## "'"); \
		return false; \
	}


#define MAMBI_CFG_IS_STRING(__cfg, __name, __path) \
	MAMBI_CFG_EXISTS(__cfg, __name, __path) \
	else if (!(__cfg)[__name].is_string()) { \
		ErrorAlert("Error", "The '" ## __name ## "' value must be string  in '" ## __path ## "'"); \
		return false; \
	}


namespace Mambi
{

	class Config
	{
	public:
		static const wchar_t FileName[];
		static const char DefaultConfig[];

		Config();
		~Config();


		inline auto& Directory() const { return _appDataPath; };
		inline auto& Path() const { return _configPath; };
		inline auto& Data() const { return _data; }
		inline auto& Mutex() const { return _mutex; }
		inline auto& GetPath(const char* path) const { return _data[json::json_pointer(path)]; }
		inline void SetPath(const char* path, json value) { _data[json::json_pointer(path)] = value; WriteOut(); }

		void Init();
		void Load();
		void WriteUser(const char* prop, json value);
		ULARGE_INTEGER LastWriteTime();


		Config(Config const&) = delete;
		void operator=(Config const&) = delete;

	private:		
		void Watch();
		void WriteOut();

		std::wstring _appDataPath;
		std::wstring _configPath;
		HANDLE _hFileWatcher;
		HANDLE _mutex;
		json _data;
	};

}