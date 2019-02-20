#pragma once

namespace Mambi
{
	class TrayMenu
	{
	public:
		const enum Position {
			Profile,
			Enabled,
			Calibrate,
			Exit
		};

		struct ProfileItem {
			std::string display;
			std::string profile;
		};

		TrayMenu();
		~TrayMenu();

		UINT Show();
		void UpdateEnabled();
		void UpdateCalibrate();
		void UpdateProfile();


		TrayMenu(TrayMenu const&) = delete;
		void operator=(TrayMenu const&) = delete;
	private:
		HMENU _menu;
		HMENU _displayProfileMenu;
		std::map<std::string, HMENU> _profileMenus;
		std::vector<ProfileItem> _profileItems;
	};


	

}