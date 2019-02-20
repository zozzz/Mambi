#include "stdafx.h"
#include "Mambi.h"
#include "KeyboardInterceptor.h"
#include "utils.h"


namespace Mambi {

	bool Application::_userEnabled = true;

	Application::Application()
	{
	}


	Application::~Application()
	{
		delete _calibrate;
		delete _kbInterceptor;
		delete _trayIcon;
		delete _config;
		delete _led;
		delete _display;
		delete _profile;
	}


	bool Application::Init(HINSTANCE hInstance)
	{
		Console::WriteLine("Application::Init");
		_hInstance = hInstance;
		RegisterWindowClass();
		CalibrateWindow::RegisterWindowClass();
		CreateMainWindow();

		_config = new Mambi::Config();
		_trayIcon = new Mambi::TrayIcon();
		_kbInterceptor = new Mambi::KeyboardInterceptor();
		_led = new Mambi::LedStripManager();
		_display = new Mambi::DisplayManager();
		_profile = new Mambi::ProfileManager();
		_calibrate = new Mambi::Calibrate();

		_config->Init();
		//_profile->Watch();

		return TRUE;
	}


	int Application::Run()
	{
		HACCEL hAccelTable = LoadAccelerators(_hInstance, Resources::MainWindowClass().IntResource());
		MSG msg;

		// Main message loop:
		while (GetMessage(&msg, nullptr, 0, 0))
		{
			if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
			{
				TranslateMessage(&msg);
				//Console::WriteLine("Application::Run(%d, %d, %d)", msg.message, msg.wParam, msg.lParam);
				DispatchMessage(&msg);
			}
		}

		return (int)msg.wParam;		
	}


	void Application::SetUserEnabled(bool enabled)
	{
		if (_userEnabled != enabled) 
		{
			_userEnabled = enabled;
			_trayIcon->Update();
		}
	}


	LRESULT CALLBACK Application::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		//Console::WriteLine("Application::WndProc(%d, %d, %d)", message, wParam, lParam);
		switch (message)
		{
		case WM_DESTROY:
			PostQuitMessage(0);
			break;

		case WM_MAMBI_TRAY_ICON:
			return TrayIcon().WndProc(hWnd, message, wParam, lParam);

		case WM_MAMBI_CONFIG_CHANGED:
			OnConfigUpdate();			
			return 1;

		case WM_MAMBI_PROFILE_CHANGED:
			Console::WriteLine("WM_MAMBI_PROFILE_CHANGED");
			TrayIcon().Menu().UpdateProfile();
			return 1;

		case WM_DISPLAYCHANGE:
			Console::WriteLine("WM_DISPLAYCHANGE");
			Display().UpdateOutputs();
			Calibrate().Update();
			break;

		case WM_PAINT:
			Console::WriteLine("WM_PAINT %d, %d", wParam, lParam);
			break;

		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}

		return 0;
	}

	void Application::OnConfigUpdate()
	{
		AcquireMutex lock(Config().Mutex());

		if (lock)
		{
			Console::WriteLine("Application::OnConfigUpdate", Config().Path().c_str());
			Led().Update();
			Profile().Update();
			Display().Update();
			Calibrate().Update();
			lock.Release();			
		}		
	}


	LRESULT CALLBACK Application::_WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		return Instance().WndProc(hWnd, message, wParam, lParam);
	}


	ATOM Application::RegisterWindowClass()
	{
		WNDCLASSEXW wcex;
		wcex.cbSize = sizeof(WNDCLASSEX);
		wcex.style = 0;
		wcex.lpfnWndProc = _WndProc;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = 0;
		wcex.hInstance = _hInstance;
		wcex.hIcon = Resources::Icon().Data();
		wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
		wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
		wcex.lpszMenuName = NULL;
		wcex.lpszClassName = Resources::MainWindowClass().Data();
		wcex.hIconSm = NULL;

		return RegisterClassExW(&wcex);
	}


	void Application::CreateMainWindow()
	{
		_hWnd = CreateWindowEx(
			WS_EX_TOOLWINDOW | WS_EX_TOPMOST | WS_EX_TRANSPARENT,
			Resources::MainWindowClass().Data(), 
			NULL, 
			WS_POPUP,
			0, 0, 400, 200, 
			nullptr, nullptr, _hInstance, nullptr);

		if (!_hWnd)
		{
			throw new std::exception("TODO: GetLastError.");
		}

		// SW_HIDE, SW_FORCEMINIMIZE
		ShowWindow(_hWnd, SW_FORCEMINIMIZE);
		UpdateWindow(_hWnd);
	}
}