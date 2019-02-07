#pragma once

namespace Mambi
{

	class TrayMenu
	{
	public:
		const enum Position {
			Profile,
			Enabled,
			Exit
		};

		TrayMenu();
		~TrayMenu();

		UINT Show();
		void Update();
		void UpdateProfile();


		TrayMenu(TrayMenu const&) = delete;
		void operator=(TrayMenu const&) = delete;
	private:
		HMENU _menu;
		HMENU _profileMenu;
		std::string _profileMenuLabel;
	};

}