#include "stdafx.h"
#include "Profile.h"
#include "Messages.h"
#include "Console.h"
#include "Effect.h"
#include "Config.h"
#include "Application.h"


namespace Mambi
{
	std::shared_ptr<Profile> Profile::New(const std::string& title, const json& cfg)
	{
		MAMBI_CFG_IS_STRING(cfg, "fgApplication", "profile.*");
		std::string fgApplication = cfg["fgApplication"].get<std::string>();
		
		if (fgApplication == "game")
		{
			return std::shared_ptr<Profile>(new GameProfile(title));
		}
		else if (fgApplication == "any")
		{
			return std::shared_ptr<Profile>(new AnyProfile(title));
		}
		else 
		{
			return std::shared_ptr<Profile>(new SpecifiedProfile(title, fgApplication));
		}
	}

	Profile::~Profile()
	{		
		if (_effect != NULL) 
		{
			delete _effect;
		}
	}


	bool Profile::Update(const json& cfg)
	{
		Console::WriteLine("Profile::Update");

		MAMBI_CFG_VNUM_INT_RANGE(cfg, "priority", "profile.*", 0, 100);
		_priority = cfg["priority"];
		MAMBI_CFG_IS_OBJECT(cfg, "effect", "profile.*");
		auto& effect = cfg["effect"];
		MAMBI_CFG_IS_STRING(effect, "type", "profile.*.effect");
		
		Mambi::Effect* newEffect = Mambi::Effect::New(effect);
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

		return true;
	}


	bool SortByPriority(Profile* a, Profile* b)
	{
		return a->Priority() > b->Priority();
	}


	AutoProfile::Status AutoProfile::Detect(const Display* display)
	{
		Console::WriteLine("AutoProfile::Detect %s", display->HardwareId().c_str());
		Status status = Status::Failed;

		std::vector<Profile*> possible;

		HWND fgWindow = GetForegroundWindow();
		const char* exe = "";

		for (auto p: Application::Profile().Profiles())
		{
			if (p.second->Test(display, fgWindow, exe))
			{
				possible.push_back(p.second.get());
			}
		}

		// TODO: _selected = WeakRef

		if (possible.size() > 0)
		{
			std::sort(possible.begin(), possible.end(), SortByPriority);
			auto selected = Application::Profile().Profiles().at(possible[0]->Title());;

			if (_selected != selected)
			{
				_selected = selected;
				_title = "Auto: " + selected->Title();
				//Console::WriteLine("Auto detected: %s(%d)", _effect->Type().c_str(), _effect->interval);
				status = Status::Changed;
				SendMessage(Application::WindowHandle(), WM_MAMBI_PROFILE_CHANGED, 0, 0);
			}
			else
			{
				status = Status::Success;
			}			
		}

		//Console::WriteLine("AutoProfile::Detect return with %d", status);

		return status;
	}


	bool GameProfile::Test(const Display* display, HWND fgWindow, const char* exe)
	{
		return false;
	}


	bool SpecifiedProfile::Test(const Display* display, HWND fgWindow, const char* exe)
	{
		return false;
	}

	/*
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
	*/
}