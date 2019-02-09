#include "stdafx.h"
#include "Profile.h"
#include "Messages.h"
#include "Console.h"
#include "Effect.h"
#include "Config.h"


namespace Mambi
{
	Profile::Profile(): _effect(NULL)
	{
	}


	Profile::~Profile()
	{		
		if (_effect != NULL) 
		{
			delete _effect;
		}
	}


	bool Profile::Update(const std::string& title,  const json& cfg)
	{
		_title = title;

		if (cfg.count("fgApplication")) 
		{
			if (cfg["fgApplication"] == "game") 
			{
				_detection = FGDetection::Game;
			}
			else if (cfg["fgApplication"] == "any")
			{
				_detection = FGDetection::Any;
			}
			else if (cfg["fgApplication"].is_string())
			{
				_detection = FGDetection::Specified;
				_exe = cfg["fgApplication"].get<std::string>();
			} 
			else 
			{
				ErrorAlert("Error", "Unexpected 'fgApplication' property value");
				return false;
			}
		}
		else
		{
			ErrorAlert("Error", "Missing 'fgApplication' property from profile config");
			return false;
		}

		MAMBI_CFG_VNUM_INT_RANGE(cfg, "priority", "profile", 0, 100)
		_priority = cfg["priority"];

		if (cfg.count("effect")) 
		{
			if (cfg["effect"].is_object())
			{
				if (cfg["effect"].count("type"))
				{
					Mambi::Effect* newEffect = Mambi::Effect::New(cfg["effect"]);
					if (newEffect != NULL)
					{
						if (_effect == NULL)
						{
							_effect = newEffect;
						} 
						else if (*_effect != *newEffect)
						{
							delete _effect;
							_effect = newEffect;
						}
						else 
						{
							delete newEffect;
						}
					}
				}
				else
				{
					ErrorAlert("Error", "Missing 'type' property from effect config");
					return false;
				}
			}
			else
			{
				ErrorAlert("Error", "Incorrect type 'effect' option, must be an object");
				return false;
			}
		}
		else
		{
			ErrorAlert("Error", "Missing 'effect' property from profile config");
			return false;
		}
	}


	bool Profile::Test(HWND fgWindow)
	{
		switch (_detection)
		{
		case FGDetection::Any:
			return true;

		case FGDetection::Game:
			return DetectGame(fgWindow);

		case FGDetection::Specified:
			return DetectSpecified(fgWindow);
		}
		return false;
	}


	bool Profile::DetectGame(HWND fgWindow)
	{
		RECT desktop, wnd;

		GetWindowRect(fgWindow, &wnd);
		GetWindowRect(GetDesktopWindow(), &desktop);

		return desktop.left == wnd.left 
			&& desktop.top == wnd.top 
			&& desktop.right == wnd.right 
			&& desktop.bottom == wnd.bottom;		
	}	


	bool Profile::DetectSpecified(HWND fgWindow)
	{
		bool result = false;

		DWORD pid;
		GetWindowThreadProcessId(fgWindow, &pid);

		HANDLE hProc = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);

		if (hProc)
		{
			CHAR path[MAX_PATH];
			if (GetProcessImageFileNameA(hProc, path, MAX_PATH))
			{
				int l = lstrlenA(path);
				int i = l;
				while (i-- > 0)
				{
					if (path[i] == '\\')
					{
						std::string exe(path + i + 1, path + l);
						if (_exe == exe)
						{
							result = true;
						}						
						break;
					}					
				}				
			}
			CloseHandle(hProc);
		}	

		return result;
	}
}