#pragma once
#include "stdafx.h"
#include "Messages.h"


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
		void Init();
		void Load();
		void WriteUser(const char* prop, json value);
		ULARGE_INTEGER LastWriteTime();


		Config(Config const&) = delete;
		void operator=(Config const&) = delete;

	private:		
		void Watch();

		std::wstring _appDataPath;
		std::wstring _configPath;
		HANDLE _hFileWatcher;
		json _data;
	};

}