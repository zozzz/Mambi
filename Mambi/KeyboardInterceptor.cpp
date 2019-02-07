#include "stdafx.h"
#include "Mambi.h"
#include "KeyboardInterceptor.h"
#include "Application.h"


namespace Mambi
{
	KeyboardInterceptor::KeyboardInterceptor()
	{
		hook = SetWindowsHookEx(WH_KEYBOARD_LL, LowLevelKeyboardProc, Application::InstanceHandle(), 0);
	}


	KeyboardInterceptor::~KeyboardInterceptor()
	{
		if (hook != NULL) 
		{
			UnhookWindowsHookEx(hook);
		}
	}

	LRESULT CALLBACK KeyboardInterceptor::LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
	{
		return Application::Keyboard().HandleKeypress(nCode, wParam, lParam);
	}

	LRESULT CALLBACK KeyboardInterceptor::HandleKeypress(int nCode, WPARAM wParam, LPARAM lParam)
	{
		if (nCode >= 0 && wParam == WM_KEYDOWN)
		{
			KBDLLHOOKSTRUCT* key = (KBDLLHOOKSTRUCT*)lParam;
			switch (key->vkCode)
			{
			case VK_PAUSE:
				Console::WriteLine("PAUSE PRESSED");
				Application::UserEnabled(!Application::UserEnabled());
				return 1; // prevent the system from passing the message to the rest of the hook chain				
			}			
		}		
		return CallNextHookEx(hook, nCode, wParam, lParam);
	}
}
