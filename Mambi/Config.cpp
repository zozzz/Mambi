#include "stdafx.h"
#include "Application.h"
#include "Resources.h"
#include "Config.h"
#include "Console.h"


namespace Mambi
{
	bool FileExists(std::wstring &path);
	DWORD WINAPI FileWatcher(LPVOID param);

	const wchar_t Config::FileName[] = L"config.json";
	const char Config::DefaultConfig[] = \
R"###({
	"selectedProfile": "Breath",
	"profile": {
		"Game": {
			"fgApplication": "game",
			"effect": {
				"type": "ambilight"				
			},
			"priority": 100
		},
		"Breath": {
            "fgApplication": "any",
            "effect": {
				"type": "breath",
				"min": 50,
				"max": 100,
				"duration": 2000
			},
			"priority": 1
        },
		"Static": {
            "fgApplication": "any",
            "effect": {
				"type": "static",
				"color": "#CC3300"
			},
			"priority": 0
        }
	},
	"display": {
		"HARDWARE_ID": {
			"width": 3440,
			"height": 1440,
			"ledStrip": "main",
			"samples": {
				"horizontal": {
					"width": 100,
					"height": 100,
                    "margin": 20
				},
				"vertical": {
					"width": 100,
					"height": 100,
                    "margin": 20
				}
			}
		}
	},
	"ledStrip": {
		"main": {
			"port": "",
			"leds": {
				"horizontal": 20,
				"vertical": 10
			}
		}
	}
})###";


	Config::Config()
	{
		PWSTR appData = NULL;
		SHGetKnownFolderPath(FOLDERID_LocalAppData, KF_FLAG_NO_ALIAS, NULL, &appData);
		_appDataPath = appData;
		_appDataPath += '\\';
		_appDataPath += Resources::AppTitle().Data();		
		CoTaskMemFree(appData);

		SHCreateDirectory(Application::WindowHandle(), _appDataPath.c_str());

		_configPath = _appDataPath + L"\\" + FileName;

		DWORD fileAttrs = GetFileAttributes(_configPath.c_str());

		if (!FileExists(_configPath))
		{
			HANDLE hFile = CreateFile(_configPath.c_str(), GENERIC_WRITE, 0, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
			DWORD written = 0;
			WriteFile(hFile, DefaultConfig, sizeof(DefaultConfig) - sizeof(char), &written, NULL);
			CloseHandle(hFile);		
		}
	}


	Config::~Config()
	{
		if (_hFileWatcher != NULL) 
		{
			CloseHandle(_hFileWatcher);
		}		
	}


	void Config::Init()
	{
		Load();
		Watch();
	}


	void Config::Load()
	{
		std::ifstream input(Path(), std::ios::binary | std::ios::in);

		try 
		{
			input >> _data;
		}
		catch (nlohmann::detail::exception e)
		{
			ErrorAlert("Error", e.what());
			return;
		}
		
		SendMessage(Application::WindowHandle(), WM_MAMBI_CONFIG_CHANGED, 0, 0);
	}


	void Config::WriteUser(const char* prop, json value)
	{
		_data[prop] = value;

		std::ofstream out(Path());

		try
		{
			out << std::setw(4) << _data;
		}
		catch (nlohmann::detail::exception e)
		{
			ErrorAlert("Error", e.what());
			return;
		}
	}


	ULARGE_INTEGER Config::LastWriteTime()
	{
		ULARGE_INTEGER res = {0, 0};

		HANDLE hFile = CreateFile(Path().c_str(), GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, NULL);
		if (hFile != INVALID_HANDLE_VALUE)
		{
			FILETIME writeTime;
			if (GetFileTime(hFile, NULL, NULL, &writeTime))
			{				
				res.LowPart = writeTime.dwLowDateTime;
				res.HighPart = writeTime.dwHighDateTime;		
			}
		}

		CloseHandle(hFile);
		return res;
	}


	// SendMessage(Application::WindowHandle(), WM_MAMBI_CONFIG_FILE_CHANGED, 0, 0);
	void Config::Watch()
	{		
		_hFileWatcher = CreateThread(NULL, 0, FileWatcher, this, 0, NULL);
	}


	bool FileExists(std::wstring &path)
	{
		DWORD attr = GetFileAttributes(path.c_str());
		if (attr == INVALID_FILE_ATTRIBUTES) 
		{
			DWORD err = GetLastError();
			switch (err)
			{
			case ERROR_FILE_NOT_FOUND:
			case ERROR_PATH_NOT_FOUND:
				return false;
			case ERROR_ACCESS_DENIED:
				return true;
			default:
				return false;
			}			
		}

		return !(attr & FILE_ATTRIBUTE_DIRECTORY);
	}


	DWORD WINAPI FileWatcher(LPVOID param)
	{
		Config* config = (Config*)param;
		auto lastWriteTime = config->LastWriteTime();

		HANDLE hWatch = FindFirstChangeNotification(config->Directory().c_str(), false, FILE_NOTIFY_CHANGE_LAST_WRITE);
		if (hWatch == INVALID_HANDLE_VALUE)
		{
			throw std::exception("TODO GetLastError");
		}
		else 
		{
			DWORD waitStatus;

			while (true)
			{
				waitStatus = WaitForMultipleObjects(1, &hWatch, FALSE, INFINITE);

				switch (waitStatus)
				{
				// A file was created, renamed, or deleted in the directory.
				// Refresh this directory and restart the notification.
				case WAIT_OBJECT_0:

					auto writeTime = config->LastWriteTime();
					if (writeTime.QuadPart > lastWriteTime.QuadPart) 
					{
						lastWriteTime = writeTime;
						config->Load();
					}
					
					if (FindNextChangeNotification(hWatch) == FALSE)
					{
						return 1;
					}
					break;
				
				// A directory was created, renamed, or deleted.
				// Refresh the tree and restart the notification.
				case WAIT_OBJECT_0 + 1:
					if (FindNextChangeNotification(hWatch) == FALSE)
					{
						return 1;
					}
					break;

				case WAIT_TIMEOUT:
					continue;

				default:
					throw new std::exception("TODO GetLastError");
					return 1;
				}
			}
		}

		return 0;
	}
}