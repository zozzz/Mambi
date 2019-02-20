#include "stdafx.h"
#include "TrayIcon.h"
#include "Application.h"
#include "Resources.h"
#include "Console.h"


namespace Mambi
{

	TrayIcon::TrayIcon()
	{
		memset(&data, 0, sizeof(NOTIFYICONDATA));
		data.cbSize = sizeof(NOTIFYICONDATA);
		data.hWnd = Application::WindowHandle();
		data.uID = IDD_MAMBI_TRAY;
		data.uCallbackMessage = WM_MAMBI_TRAY_ICON;
		data.uFlags = NIF_ICON | NIF_MESSAGE;
		data.hIcon = Resources::Icon().Data();
		
		StringCchCopy(data.szTip, ARRAYSIZE(data.szTip), Resources::AppTitle().Data());

		Shell_NotifyIcon(NIM_ADD, &data);

		_menu = new TrayMenu();
	}


	TrayIcon::~TrayIcon()
	{
		Shell_NotifyIcon(NIM_DELETE, &data);
		delete _menu;
	}


	LRESULT CALLBACK TrayIcon::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		switch (lParam) 
		{
		case WM_LBUTTONDOWN:
			Console::WriteLine("TrayIcon WM_LBUTTONDOWN");
			return 1;

		case WM_RBUTTONDOWN:
			Console::WriteLine("TrayIcon WM_RBUTTONDOWN");
			int cmd = _menu->Show();
			switch (cmd)
			{
			case IDM_EXIT:
				Application::Exit();
				break;

			case IDM_CHANGE_ENABLE:
				Application::UserEnabled(!Application::UserEnabled());
				break;

			case IDM_CALIBRATE_TOGGLE:
				Application::Calibrate().Enabled(!Application::Calibrate().Enabled(), true);
				break;
			}
		}
		//Console::WriteLine("TrayIcon (%d, %d, %d)", message, wParam, lParam);
		return DefWindowProc(hWnd, message, wParam, lParam);
	}


	void TrayIcon::Update()
	{
		if (Application::Enabled())
		{
			data.hIcon = Resources::Icon().Data();
		}
		else
		{
			data.hIcon = Resources::IconGs().Data();
		}

		Shell_NotifyIcon(NIM_MODIFY, &data);
		Menu().UpdateEnabled();
	}

}