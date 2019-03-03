#pragma once
#include "stdafx.h"
#include "Display.h"
#include "DisplaySample.h"


namespace Mambi
{
	class CalibrateWindow;


	class Calibrate
	{
	public:
		Calibrate();
		~Calibrate();

		inline auto Enabled() const { return _enabled; }
		void Enabled(bool enabled, bool write=false);
		
		bool Update();

		Calibrate(Calibrate const&) = delete;
		void operator=(Calibrate const&) = delete;
	private:
		void ShowWindows();
		void HideWindows();

		bool _enabled;	
		std::map<std::string, CalibrateWindow*> _windows;		
	};



	class CalibrateWindow {
	public:
		static ATOM RegisterWindowClass();

		CalibrateWindow(const Display& display);
		~CalibrateWindow();

		void Update();

		CalibrateWindow(CalibrateWindow const&) = delete;
		void operator=(CalibrateWindow const&) = delete;
	private:
		void Hide();
		void Paint();

		LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
		static LRESULT CALLBACK _WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

		HWND _hWnd;
		const Display& _display;
		DisplaySamples* _samples;		
	};
}