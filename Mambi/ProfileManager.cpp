#include "stdafx.h"
#include "ProfileManager.h"
#include "Application.h"
#include "Console.h"
#include "utils.h"


namespace Mambi
{
	bool ProfileManager::Update()
	{
		auto& cfg = Application::Config().Data();
		std::vector<std::string> defined;

		MAMBI_CFG_IS_OBJECT(cfg, "profile", "config");

		for (auto& el : cfg["profile"].items())
		{
			auto& key = el.key();
			auto& value = el.value();

			defined.push_back(key);

			auto profile = Profile::New(key, value);
			if (_profiles.find(key) == _profiles.end())
			{
				_profiles[key] = profile;
				profile->Update(value);
			}
			else if (profile->Type() != _profiles[key]->Type())
			{
				auto current = _profiles.at(key);
				_profiles[key] = profile;
				profile->Update(value);
				ReplaceProfile(current, profile);				
			}
			else
			{
				_profiles[key]->Update(value);
			}
		}
			

		// remove unpresented configs
		std::vector<std::string> remove;
		for (auto& el : _profiles)
		{
			if (std::find(defined.begin(), defined.end(), el.first) == defined.end())
			{
				remove.push_back(el.first);
			}
		}

		for (auto& el : remove)
		{
			ReplaceProfile(_profiles.at(el), std::shared_ptr<Profile>(new AutoProfile()));
			_profiles.erase(el);
		}	

		return true;
	}


	bool ProfileManager::Activate(const Display* display, std::shared_ptr<Profile> profile)
	{
		if (!display->Profile() || display->Profile()->Title() != profile->Title())
		{
			const_cast<Display*>(display)->Profile(profile);
			SendMessage(Application::WindowHandle(), WM_MAMBI_PROFILE_CHANGED, 0, 0);
			return true;
		}
		return false;
	}

	bool ProfileManager::Activate(const Display* display, const nullptr_t& profile)
	{
		if (!display->Profile() || display->Profile()->Type() != ProfileType::Auto)
		{
			const_cast<Display*>(display)->Profile(std::shared_ptr<Mambi::Profile>(new AutoProfile()));
			SendMessage(Application::WindowHandle(), WM_MAMBI_PROFILE_CHANGED, 0, 0);
			return true;
		}
		return false;
	}

	
	bool ProfileManager::Activate(const std::string& display, const std::string& profile)
	{
		auto& disp = Application::Display().Get(display);
		return Activate(&disp, _profiles.at(profile));
	}

	bool ProfileManager::Activate(const std::string& display, const nullptr_t& profile)
	{
		auto& disp = Application::Display().Get(display);
		return Activate(&disp, nullptr);
	}


	void ProfileManager::ReplaceProfile(std::shared_ptr<Profile> current, std::shared_ptr<Profile> replcae)
	{
		bool changed = false;
		for (auto& d : Application::Display().Displays())
		{
			if (d.second.Profile() == current)
			{
				const_cast<Display*>(&d.second)->Profile(replcae);
				changed = true;
			}
		}

		if (changed)
		{
			SendMessage(Application::WindowHandle(), WM_MAMBI_PROFILE_CHANGED, 0, 0);
		}
	}

#if 0
	void ProfileManager::StartAutoDetect()
	{
		if (_hAutoDetect == NULL)
		{
			_hAutoDetect = CreateThread(NULL, 0, AutoDetectThread, this, 0, NULL);
		}
	}


	void ProfileManager::StopAutoDetect()
	{
		if (_hAutoDetect != NULL)
		{
			if (TerminateThread(_hAutoDetect, 0))
			{	
				CloseHandle(_hAutoDetect);
				_hAutoDetect = NULL;
			}
		}
	}


	void ProfileManager::Activate(int idx)
	{
		if (idx <= 0)
		{
			Application::Config().WriteUser("selectedProfile", nullptr);
			return;
		}

		idx--;

		int i = 0;
		for (auto& item: _profiles) 
		{
			if (i == idx)
			{
				Application::Config().WriteUser("selectedProfile", item.second.Title());
				break;
			}
			i++;
		}
	}


	bool ProfileManager::SetActive(Profile* profile)
	{
		if (_active == NULL || _active->Title() != profile->Title())
		{
			_active = profile;	
			SendMessage(Application::WindowHandle(), WM_MAMBI_PROFILE_CHANGED, 0, 0);
			return true;
		}
		return false;
	}


	DWORD WINAPI ProfileManager::AutoDetectThread(LPVOID lpParam)
	{
		ProfileManager* self = (ProfileManager*)lpParam;
		while (true)
		{
			AcquireMutex lock(Application::Config().Mutex());
			if (lock)
			{
				self->AutoDetect();
			}			
			lock.Release();
			Sleep(500);
		}
	}


	bool SortByPriority(Profile* a, Profile* b) 
	{
		return a->Priority() > b->Priority();
	}


	void ProfileManager::AutoDetect()
	{
		HWND fg = GetForegroundWindow();

		/*if (_fgWindow == fg)
		{
			return;
		}
		
		_fgWindow = fg;
		*/

		/*
		WCHAR wnd_title[256];
		GetWindowText(fg, wnd_title, sizeof(wnd_title));
		Console::WriteLine("FgChanged %S", wnd_title);
		*/

		for (auto& disp: Application::Display().DisplayMap())
		{

		}

		std::vector<Profile*> possible;

		for (auto& item : _profiles)
		{
			if (item.second.Test(fg))
			{
				possible.push_back(&item.second);
			}
		}

		if (possible.size() > 0) 
		{
			std::sort(possible.begin(), possible.end(), SortByPriority);
			SetActive(possible[0]);
		}
	}

#endif
}