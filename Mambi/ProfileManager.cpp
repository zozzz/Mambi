#include "stdafx.h"
#include "ProfileManager.h"
#include "Application.h"
#include "Console.h"
#include "Synchronisation.h"


namespace Mambi
{
	ProfileManager::ProfileManager(): _active(NULL), _hAutoDetect(NULL)
	{
		_mutex = CreateMutex(NULL, FALSE, NULL);
	}


	ProfileManager::~ProfileManager()
	{
		CloseHandle(_mutex);
		StopAutoDetect();
	}


	void ProfileManager::Update()
	{
		MAMBI_MLOCK_BEGIN(_mutex, INFINITE)
			DoUpdate();
		MAMBI_MLCOK_END_VOID(_mutex)	
	}


	void ProfileManager::DoUpdate()
	{
		auto& cfg = Application::Config().Data();
		std::vector<std::string> defined;

		if (cfg.count("profile"))
		{
			if (cfg["profile"].is_object())
			{
				for (auto& el : cfg["profile"].items())
				{
					auto& key = el.key();
					auto& value = el.value();

					defined.push_back(key);

					auto& profile = _profiles[key];
					profile.Update(key, value);

					Console::WriteLine("Profile = %s", key.c_str());
				}
			}
			else
			{
				ErrorAlert("Error", "The 'display' option must be an object.");
			}
		}
		else
		{
			ErrorAlert("Error", "Missing 'profile' option in config.");
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
			Console::WriteLine("Erase %s", el.c_str());
			_profiles.erase(el);
		}

		bool isAutoDetect = true;
		Profile* active = NULL;

		if (cfg.count("selectedProfile") && cfg["selectedProfile"].is_string())
		{
			if (_profiles.find(cfg["selectedProfile"]) != _profiles.end())
			{
				active = &_profiles[cfg["selectedProfile"]];
				isAutoDetect = false;
			}
			else
			{
				ErrorAlert("Error", "Can't select the given profile, because is missing from profiles definition.");
			}
		}

		bool eventRequired = false;
		if (_isAutoDetect != isAutoDetect)
		{
			eventRequired = true;
			_isAutoDetect = isAutoDetect;
			if (isAutoDetect)
			{
				StartAutoDetect();
			}
			else
			{
				StopAutoDetect();
			}
		}

		if (active != NULL)
		{
			eventRequired = !SetActive(active);
		}

		if (eventRequired && _active != NULL)
		{
			SendMessage(Application::WindowHandle(), WM_MAMBI_PROFILE_CHANGED, 0, 0);
		}
	}

	
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
			MAMBI_MLOCK_BEGIN(self->_mutex, INFINITE)
				self->AutoDetect();
			MAMBI_MLCOK_END(self->_mutex, FALSE)
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
}