#pragma once
#include "stdafx.h"
#include "KeyboardInterceptor.h"
#include "TrayIcon.h"
#include "Config.h"
#include "LedStripManager.h"
#include "DisplayManager.h"
#include "ProfileManager.h"
#include "Calibrate.h"


namespace Mambi {

	class Application
	{
	public:
		static inline HWND WindowHandle() { return Instance()._hWnd; }
		static inline HINSTANCE InstanceHandle() { return Instance()._hInstance; }
		static inline void Exit(int code=0) { PostQuitMessage(code); }
		static inline void UserEnabled(bool enabled) { Instance().SetUserEnabled(enabled); }
		static inline bool UserEnabled() { return Instance()._userEnabled; }
		static inline bool Enabled() { return UserEnabled(); }
		static inline Mambi::KeyboardInterceptor& Keyboard() { return *Instance()._kbInterceptor; }
		static inline Mambi::TrayIcon& TrayIcon() { return *Instance()._trayIcon; }
		static inline Mambi::Config& Config() { return *Instance()._config; }
		static inline Mambi::LedStripManager& Led() { return *Instance()._led; }
		static inline Mambi::DisplayManager& Display() { return *Instance()._display; }
		static inline Mambi::ProfileManager& Profile() { return *Instance()._profile; }
		static inline Mambi::Calibrate& Calibrate() { return *Instance()._calibrate; }
		
	private:
		bool _userEnabled;

	public:
		static Application& Instance() {
			static Application instance;
			return instance;
		}

		~Application();

		bool Init(HINSTANCE hInstance);
		int Run();
		void SetUserEnabled(bool enabled);

		Application(Application const&) = delete;
		void operator=(Application const&) = delete;

	private:
		Application();

		LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
		static LRESULT CALLBACK _WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
		ATOM RegisterWindowClass();
		void CreateMainWindow();
		void OnConfigUpdate();
		void OnDisplayChange();

		HINSTANCE _hInstance;
		HWND _hWnd;

		Mambi::KeyboardInterceptor* _kbInterceptor;
		Mambi::TrayIcon* _trayIcon;
		Mambi::Config* _config;
		Mambi::LedStripManager* _led;
		Mambi::DisplayManager* _display;
		Mambi::ProfileManager* _profile;
		Mambi::Calibrate* _calibrate;
	};

}