#include "stdafx.h"
#include "TrayMenu.h"
#include "Resources.h"
#include "Console.h"


namespace Mambi 
{

	TrayMenu::TrayMenu()
	{
		_menu = CreatePopupMenu();
		_displayProfileMenu = CreatePopupMenu();

		InsertMenu(_menu, Position::Profile, MF_BYPOSITION | MF_POPUP, (UINT_PTR)_displayProfileMenu, Resources::TrayProfile().Data());
		InsertMenu(_menu, Position::Enabled, MF_BYPOSITION | MF_STRING, IDM_CHANGE_ENABLE, Resources::TrayEnabled().Data());
		InsertMenu(_menu, Position::Calibrate, MF_BYPOSITION | MF_STRING, IDM_CALIBRATE_TOGGLE, Resources::TrayCalibrateDisabled().Data());
		InsertMenu(_menu, Position::Exit, MF_BYPOSITION | MF_STRING, IDM_EXIT, Resources::TrayExit().Data());	
	}


	TrayMenu::~TrayMenu()
	{
		if (_menu != NULL) {
			DestroyMenu(_menu);
			_menu = NULL;
		}

		for (auto& item : _profileMenus)
		{
			DestroyMenu(item.second);			
		}
		_profileMenus.clear();

		if (_displayProfileMenu != NULL) {
			DestroyMenu(_displayProfileMenu);
			_displayProfileMenu = NULL;
		}
	}


	UINT TrayMenu::Show()
	{
		POINT mouse;
		GetCursorPos(&mouse);

		UINT res = TrackPopupMenu(_menu, TPM_RETURNCMD | TPM_NONOTIFY | TPM_LEFTBUTTON, mouse.x, mouse.y, 0, Application::WindowHandle(), NULL);
		
		if (IDM_TPROFILE_MIN <= res && res < IDM_TPROFILE_MAX)
		{
			ProfileItem pi = _profileItems[res - IDM_TPROFILE_MIN];
			std::string cfgPath = "/display/" + pi.display + "/profile";

			if (pi.profile.size() == 0)
			{
				Application::Config().SetPath(cfgPath.c_str(), nullptr);
			}
			else
			{
				Application::Config().SetPath(cfgPath.c_str(), pi.profile);				
			}			

			return res;
		}

		return res;	
	}


	void TrayMenu::UpdateEnabled()
	{
		if (Application::UserEnabled()) 
		{
			ModifyMenu(_menu, Position::Enabled, MF_BYPOSITION | MF_STRING, IDM_CHANGE_ENABLE, Resources::TrayEnabled().Data());
		}
		else
		{
			ModifyMenu(_menu, Position::Enabled, MF_BYPOSITION | MF_STRING, IDM_CHANGE_ENABLE, Resources::TrayDisabled().Data());
		}
	}

	void TrayMenu::UpdateCalibrate()
	{
		if (Application::Calibrate().Enabled())
		{
			ModifyMenu(_menu, Position::Calibrate, MF_BYPOSITION | MF_STRING, IDM_CALIBRATE_TOGGLE, Resources::TrayCalibrateEnabled().Data());
		}
		else
		{
			ModifyMenu(_menu, Position::Calibrate, MF_BYPOSITION | MF_STRING, IDM_CALIBRATE_TOGGLE, Resources::TrayCalibrateDisabled().Data());
		}
	}

	void TrayMenu::UpdateProfile()
	{
		_profileItems.clear();
		auto& displays = Application::Display().DisplayMap();
		std::vector<std::string> remove;

		for (auto& p : _profileMenus) 
		{
			if (displays.find(p.first) == displays.end())
			{
				remove.push_back(p.first);
			}
		}

		for (auto& k: remove)
		{	
			DestroyMenu(_profileMenus[k]);
			_profileMenus.erase(k);
		}


		int displayIndex = 0;
		int menuItemCommand = IDM_TPROFILE_MIN;
		for (auto& display: displays)
		{
			if (_profileMenus.find(display.first) == _profileMenus.end())
			{
				_profileMenus[display.first] = CreatePopupMenu();				
			}
			

			HMENU profileMenu = _profileMenus[display.first];
			int menuCount = GetMenuItemCount(profileMenu);
			int menuIndex = 0;
			bool isActive = display.second.Profile() && display.second.Profile()->Type() == ProfileType::Auto;
			std::string activeLabel;


			if (menuIndex >= menuCount) 
			{
				InsertMenu(profileMenu, menuIndex, MF_BYPOSITION | MF_STRING | (isActive ? MF_CHECKED : 0), menuItemCommand, Resources::Auto().Data());
			}
			else
			{
				ModifyMenu(profileMenu, menuIndex, MF_BYPOSITION | MF_STRING | (isActive ? MF_CHECKED : 0), menuItemCommand, Resources::Auto().Data());
			}
			
			menuIndex++;
			menuItemCommand++;
			_profileItems.push_back({ display.first, "" });

			if (isActive)
			{
				activeLabel = display.second.Profile()->Title();
			}

			for (auto& p: Application::Profile().Profiles())
			{
				isActive = display.second.Profile() == p.second;
				
				if (isActive)
				{
					activeLabel = p.second->Title().c_str();
				}

				if (menuIndex >= menuCount)
				{
					InsertMenuA(profileMenu, menuIndex, MF_BYPOSITION | MF_STRING | (isActive ? MF_CHECKED : 0), menuItemCommand, p.second->Title().c_str());
				}
				else
				{
					ModifyMenuA(profileMenu, menuIndex, MF_BYPOSITION | MF_STRING | (isActive ? MF_CHECKED : 0), menuItemCommand, p.second->Title().c_str());
				}

				menuIndex++;
				menuItemCommand++;
				_profileItems.push_back({ display.first, p.second->Title() });
			}
		}


		// remove all display menu item
		int count = GetMenuItemCount(_displayProfileMenu);
		size_t pmenuCount = _profileMenus.size();
		while (count-- > pmenuCount)
		{
			DeleteMenu(_displayProfileMenu, 0, MF_BYPOSITION);
		}

		// todo: _profileMenus.size() == 1, display only profiles

		int i = 0;
		for (auto& m: _profileMenus)
		{
			if (i > count)
			{
				InsertMenuA(_displayProfileMenu, i, MF_BYPOSITION | MF_POPUP, (UINT_PTR)m.second, m.first.c_str());
			}
			else
			{
				ModifyMenuA(_displayProfileMenu, i, MF_BYPOSITION | MF_POPUP, (UINT_PTR)m.second, m.first.c_str());
			}
			
			i++;
		}		
	}
}
