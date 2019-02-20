#pragma once
#include "stdafx.h"
#include "Display.h"


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

		// static DWORD WINAPI DrawThread(LPVOID lpParam);

		// void StopDraw();
		// void DisplayWindows();
		// void DrawWindows();

		bool _enabled;	
		std::map<std::string, CalibrateWindow*> _windows;
		// HANDLE _mutex;
		// HANDLE _drawThread;
	};



	class CalibrateWindow {
	public:
		/*
		class Trackbar {
		public:
			Trackbar(HWND parent, int x, int y, int w, int h, const char* path, int minV, int maxV);
			~Trackbar();

		private:
			HWND _hWnd;
			const char* _path;
		};
		*/


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
		//std::vector<Trackbar*> _trackbars;
	};
}