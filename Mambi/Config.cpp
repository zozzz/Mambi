#include "stdafx.h"
#include "Application.h"
#include "Resources.h"
#include "Config.h"
#include "Console.h"
#include "utils.h"


namespace Mambi
{
	bool FileExists(std::wstring &path);
	DWORD WINAPI FileWatcher(LPVOID param);

	const wchar_t Config::FileName[] = L"config.json";
	const char Config::DefaultConfig[] = \
R"###({
	"calibrate": false,
	"profile": {
		"Ambilight": {
			"fgApplication": "fullscreen",
			"effect": {
				"type": "ambilight"				
			},
			"priority": 100
		},
		"Breath": {
            "fgApplication": "any",
            "effect": {
				"color": "CC3300",
                "duration": 5000,
                "fps": 30,
                "maxBrightness": 255,
                "minBrightness": 100,
                "type": "breath"
			},
			"priority": 1
        },
		"Static": {
            "fgApplication": "any",
            "effect": {
				"type": "static",
				"color": "CC3300",
				"brightness": 100
			},
			"priority": 0
        },
		"Rainbow": {
			"fgApplication": "any",
			"effect": {
				"type": "rainbow",
				"duration": 10000,
				"brightness": 200,
				"colors": [
					[0.0, "FF0000"],
					[0.3, "0000FF"],
					[0.3, "00FF00"],
					[1.0, "FF0000"]
				]
			},
			"priority": 100
		},
		"Timber": {
            "effect": {
                "brightness": 255,
                "colors": [
                    [0.0, "fc00ff"],
                    [0.5, "00dbde"],
					[1.0, "fc00ff"]
                ],
                "duration": 5000,
                "type": "rainbow"
            },
            "fgApplication": "any",
            "priority": 100
        }
	},
	"display": {
		"HARDWARE_ID": {
			"profile": null,
			"width": 3440,
			"height": 1440,
			"ledStrip": {
				"name": "main",
				"offset": 0
			},
			"samples": {
				"horizontal": {
                    "height": 150,
                    "hpadding": 50,
                    "vpadding": 0,
                    "width": 100
                },
                "vertical": {
                    "height": 100,
                    "hpadding": 0,
                    "vpadding": 50,
                    "width": 150
                }
			}
		}
	},
	"ledStrip": {
		"main": {
			"port": "\\\\.\\COM10",
			"leds": {
				"horizontal": 24,
				"vertical": 10
			}
		}
	}
})###";


	Config::Config()
	{
		_mutex = CreateMutex(NULL, FALSE, L"mambi.config");

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
			_hFileWatcher = NULL;
		}		

		if (_mutex != NULL)
		{
			CloseHandle(_mutex);
			_mutex = NULL;
		}
	}


	void Config::Init()
	{
		Load();
		Watch();
	}


	void Config::Load()
	{
		AcquireMutex lock(Mutex());

		if (lock) 
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
			
			lock.Release();			

			SendMessage(Application::WindowHandle(), WM_MAMBI_CONFIG_CHANGED, 0, 0);
		}		
	}


	void Config::WriteUser(const char* prop, json value)
	{
		_data[prop] = value;
		WriteOut();	
	}

	void Config::WriteOut()
	{
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