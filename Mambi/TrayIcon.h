#pragma once
#include "stdafx.h"

#include "Messages.h"
#include "TrayMenu.h"


namespace Mambi
{

	class TrayIcon
	{
	public:
		TrayIcon();
		~TrayIcon();

		LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
		
		void Update();
		
		inline TrayMenu& Menu() const { return *_menu; }

			
		TrayIcon(TrayIcon const&) = delete;
		void operator=(TrayIcon const&) = delete;
	private:
		NOTIFYICONDATA data;
		TrayMenu* _menu;
	};

}