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
		
		if (fgApplication == "fullscreen")
		{
			return std::shared_ptr<Profile>(new FullScreenProfile(title));
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
		//Console::WriteLine("AutoProfile::Detect %s", display->HardwareId().c_str());		

		std::vector<Profile*> possible;
		HWND fgWindow = GetForegroundWindow();

		if (_fgWindow != fgWindow)
		{
			_fgWindow = fgWindow;
			UpdateExe();			
		}

		for (auto p: Application::Profile().Profiles())
		{
			if (p.second->Test(display, fgWindow, _fgExe))
			{
				possible.push_back(p.second.get());
			}
		}

		std::shared_ptr<Profile> selected;

		switch (possible.size())
		{
		case 0:
			return Status::Failed;

		case 1:
			selected = Application::Profile().Profiles().at(possible[0]->Title());
			break;

		default:
			std::sort(possible.begin(), possible.end(), SortByPriority);
			selected = Application::Profile().Profiles().at(possible[0]->Title());
		}

		if (_selected != selected)
		{
			_selected = selected;
			_title = "Auto: " + selected->Title();
			Console::WriteLine("Auto detected: %s(%d)", Effect()->Type().c_str(), Effect()->interval);
			SendMessage(Application::WindowHandle(), WM_MAMBI_PROFILE_CHANGED, 0, 0);
			return Status::Changed;
		}
		else
		{
			return Status::Success;
		}		
	}

	void AutoProfile::UpdateExe()
	{
		DWORD pid;
		GetWindowThreadProcessId(_fgWindow, &pid);
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
						_fgExe = std::string(path + i + 1, path + l);
						break;
					}
				}
			}
			CloseHandle(hProc);
		}
	}


	bool FullScreenProfile::Test(const Display* display, HWND fgWindow, const std::string& exe)
	{
		if (!display->Ambilight()->Available())
		{
			return false;
		}

		RECT wndRect;
		RECT desktop = display->Ambilight()->Output()->Desc.DesktopCoordinates;
		GetWindowRect(fgWindow, &wndRect);

		/*
		Console::WriteLine("%s // %d >= %d && %d >= %d && %d <= %d && %d <= %d",
			exe.c_str(),
			desktop.left , wndRect.left,
			desktop.top , wndRect.top,
			desktop.right , wndRect.right,
			desktop.bottom , wndRect.bottom);
		*/

		return desktop.left >= wndRect.left
			&& desktop.top >= wndRect.top
			&& desktop.right <= wndRect.right
			&& desktop.bottom <= wndRect.bottom;
	}


	bool SpecifiedProfile::Test(const Display* display, HWND fgWindow, const std::string& exe)
	{
		return _exe == exe;
	}	
}