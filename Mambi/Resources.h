#pragma once
#include "Resource.h"
#include "Application.h"
#include "ResourceLoaders.h"


namespace Mambi {
	class Resources
	{
	public:
		static inline auto& AppTitle() { static StringRes<IDS_APP_TITLE> res; return res; };
		static inline auto& MainWindowClass() { static StringRes<IDC_MAMBI> res; return res; };
		static inline auto& CalibrateWindowClass() { static StringRes<IDC_CALIBRATE> res; return res; };
		static inline auto& TrayExit() { static StringRes<IDS_TRAY_EXIT> res; return res; };
		static inline auto& TrayEnabled() { static StringRes<IDS_TRAY_ENABLED> res; return res; };
		static inline auto& TrayDisabled() { static StringRes<IDS_TRAY_DISABLED> res; return res; };
		static inline auto& TrayCalibrateEnabled() { static StringRes<IDS_CALIBRATE_ENABLED> res; return res; };
		static inline auto& TrayCalibrateDisabled() { static StringRes<IDS_CALIBRATE_DISABLED> res; return res; };
		static inline auto& TrayProfile() { static StringRes<IDS_TRAY_PROFILE> res; return res; };
		static inline auto& Auto() { static StringRes<IDS_AUTO> res; return res; };

		static inline auto& Icon() { static IconRes<IDI_MAMBI> res; return res; };		
		static inline auto& IconGs() { static IconRes<IDI_MAMBI_GS> res; return res; };
	};	
}
