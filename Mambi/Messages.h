#pragma once

#define WM_MAMBI_TRAY_ICON (WM_USER + 1)
#define WM_MAMBI_CONFIG_CHANGED (WM_USER + 2)
#define WM_MAMBI_PROFILE_CHANGED (WM_USER + 3)

#define ErrorAlert(title, msg) MessageBoxA(NULL, msg, title, MB_OK | MB_ICONERROR);