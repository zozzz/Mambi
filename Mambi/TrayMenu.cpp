#include "stdafx.h"
#include "TrayMenu.h"
#include "Resources.h"
#include "Console.h"


namespace Mambi 
{

	TrayMenu::TrayMenu()
	{
		_menu = CreatePopupMenu();
		_profileMenu = CreatePopupMenu();

		InsertMenu(_menu, Position::Profile, MF_BYPOSITION | MF_POPUP, (UINT_PTR)_profileMenu, Resources::TrayProfile().Data());
		InsertMenu(_menu, Position::Enabled, MF_BYPOSITION | MF_STRING, IDM_CHANGE_ENABLE, Resources::TrayEnabled().Data());
		InsertMenu(_menu, Position::Exit, MF_BYPOSITION | MF_STRING, IDM_EXIT, Resources::TrayExit().Data());		
	}


	TrayMenu::~TrayMenu()
	{
		if (_menu != NULL) {
			DestroyMenu(_menu);
			_menu = NULL;
		}

		if (_profileMenu != NULL) {
			DestroyMenu(_profileMenu);
			_profileMenu = NULL;
		}
	}


	UINT TrayMenu::Show()
	{
		POINT mouse;
		GetCursorPos(&mouse);

		UINT res = TrackPopupMenu(_menu, TPM_RETURNCMD | TPM_NONOTIFY | TPM_LEFTBUTTON, mouse.x, mouse.y, 0, Application::WindowHandle(), NULL);
		
		return res;	
	}


	void TrayMenu::Update()
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

	void TrayMenu::UpdateProfile()
	{
		int count = GetMenuItemCount(_profileMenu);
		while (count-- > 0) 
		{
			DeleteMenu(_profileMenu, 0, MF_BYPOSITION);
		}

		bool isAutoDetect = Application::Profile().IsAutoDetect();
		InsertMenu(_profileMenu, 0, MF_BYPOSITION | MF_STRING | (isAutoDetect ? MF_CHECKED : 0), IDM_TPROFILE_MIN, Resources::Auto().Data());

		_profileMenuLabel = "Profile";
		if (isAutoDetect)
		{
			auto& active = Application::Profile().Active();
			_profileMenuLabel += ": Auto (" + active.Title() + ")";
		}

		int pos = 1;
		for (auto& item : Application::Profile().Profiles()) 
		{			
			bool isActive = !isAutoDetect && Application::Profile().Active().Title() == item.second.Title();
			if (isActive)
			{
				_profileMenuLabel += ": " + item.second.Title();
			}			

			InsertMenuA(_profileMenu, pos++, MF_BYPOSITION | MF_STRING | (isActive ? MF_CHECKED : 0), IDM_TPROFILE_MIN + pos, item.second.Title().c_str());
		}

		ModifyMenuA(_menu, Position::Profile, MF_BYPOSITION | MF_POPUP, (UINT_PTR)_profileMenu, _profileMenuLabel.c_str());
	}
}
