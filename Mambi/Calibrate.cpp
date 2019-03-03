#include "stdafx.h"
#include "Application.h"
#include "Calibrate.h"
#include "Config.h"
#include "Console.h"
#include "utils.h"
#include "DisplaySample.h"
#include "Resources.h"


namespace Mambi
{

	Calibrate::Calibrate()
	{
		
	}


	Calibrate::~Calibrate()
	{
		HideWindows();
	}


	bool Calibrate::Update()
	{
		auto& cfg = Application::Config().Data();
		bool enabled = cfg.count("calibrate") == 1 && cfg["calibrate"].is_boolean() && cfg["calibrate"];
		Enabled(enabled && Application::Enabled());	
		return true;
	}


	void Calibrate::Enabled(bool enabled, bool write)
	{
		if (_enabled != enabled)
		{
			_enabled = enabled;

			if (write)
			{
				Application::Config().WriteUser("calibrate", _enabled);
			}

			if (_enabled)
			{
				ShowWindows();
			}
			else
			{
				HideWindows();
			}

			Application::TrayIcon().Menu().UpdateCalibrate();			
		}

		if (_enabled)
		{
			for (auto& item: _windows)
			{
				item.second->Update();
			}
		}
	}


	void Calibrate::ShowWindows()
	{
		for (auto& item: Application::Display().Displays())
		{
			if (_windows.find(item.first) == _windows.end())
			{
				_windows[item.first] = new CalibrateWindow(item.second);
			}						
		}
	}

	void Calibrate::HideWindows()
	{
		for (auto& item: _windows)
		{
			delete item.second;
		}	
		_windows.clear();
	}



	CalibrateWindow::CalibrateWindow(const Display& display): _display(display), _samples(NULL)
	{
		int winWidth, winHeight;

		if (_display.Ambilight()->Available())
		{
			winWidth = _display.Ambilight()->Output()->Width;
			winHeight = _display.Ambilight()->Output()->Height;
		}
		else
		{
			throw new std::exception("Unexpected error");
		}
		

		_hWnd = CreateWindowEx(
			0,
			Resources::CalibrateWindowClass().Data(),
			Resources::CalibrateWindowClass().Data(),
			WS_POPUP,
			0, 0, winWidth, winHeight,
			nullptr, nullptr, Application::InstanceHandle(), this);

		if (!_hWnd)
		{
			throw new std::exception("TODO: GetLastError.");
		}

		// SW_HIDE, SW_FORCEMINIMIZE
		ShowWindow(_hWnd, SW_SHOW);
		UpdateWindow(_hWnd);
	}


	CalibrateWindow::~CalibrateWindow()
	{
		Hide();
		if (_samples != NULL) 
		{
			delete _samples;
			_samples = NULL;
		}
	}

	void CalibrateWindow::Hide()
	{
		if (_hWnd)
		{
			DestroyWindow(_hWnd);
			_hWnd = NULL;
		}
	}

	
	void CalibrateWindow::Update()
	{
		if (_samples != NULL)
		{
			delete _samples;
			_samples = NULL;
		}

		auto ambilight = _display.Ambilight();
		if (ambilight->Available())
		{
			auto samples = _display.Ambilight()->Samples();
			_samples = new DisplaySamples(samples->HFactory(), samples->VFactory(), 0);
			_samples->Update(
				_display.NativeWidth(), _display.NativeHeight(),
				ambilight->Output()->Width, ambilight->Output()->Height);
			RedrawWindow(_hWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
		}		
	}


	template<int MS>
	struct CWSide {
		CWSide(int begin, int end) 
		{
			this->begin = begin;
			this->end = end;

			this->middleBegin = begin + (end - begin) / MS - (MS / 2);
			this->middleEnd = this->middleBegin + (MS / 2);
		}

		int begin;
		int end;
		int middleBegin;
		int middleEnd;
	};


	void CalibrateWindow::Paint()
	{
		if (_samples == NULL)
		{
			return;
		}

		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(_hWnd, &ps);

		// TOP: RED ... PURPLE ... RED
		// RIGHT: BLUE ... PURPLE ... BLUE
		// BOTTOM: YELLOW ... PURPLE ... YELLOW
		// LEFT: GREEN ... PURPLE ... GREEN


		auto red = CreateSolidBrush(RGB(255, 0, 0));
		auto green = CreateSolidBrush(RGB(0, 255, 0));
		auto blue = CreateSolidBrush(RGB(0, 0, 255));
		auto yellow = CreateSolidBrush(RGB(255, 255, 0));
		auto purple = CreateSolidBrush(RGB(255, 0, 255));
		auto black = CreateSolidBrush(RGB(0, 0, 0));


		int hCount = _display.LedStrip()->HCount();
		int vCount = _display.LedStrip()->VCount();

		int i = 0;
		CWSide<2> top(0, hCount - 1),
			right(hCount, hCount + vCount - 1),
			bottom(hCount + vCount, hCount * 2 + vCount - 1),
			left(hCount * 2 + vCount, hCount * 2 + vCount * 2 - 1);

		for (auto& s : _samples->Items())
		{
#pragma warning( push )
#pragma warning( disable : 4838)
			RECT rect = { s.Src.left, s.Src.top, s.Src.right, s.Src.bottom };
#pragma warning( pop ) 
		
			if (top.begin == i || top.end == i)
			{
				FillRect(hdc, &rect, red);
			}
			else if (right.begin == i || right.end == i) 
			{
				FillRect(hdc, &rect, blue);
			}
			else if (bottom.begin == i || bottom.end == i)
			{
				FillRect(hdc, &rect, yellow);
			}
			else if (left.begin == i || left.end == i)
			{
				FillRect(hdc, &rect, green);
			}
			else if (top.middleBegin <= i && top.middleEnd >= i)
			{
				FillRect(hdc, &rect, purple);
			}
			else if (right.middleBegin <= i && right.middleEnd >= i)
			{
				FillRect(hdc, &rect, purple);
			}
			else if (bottom.middleBegin <= i && bottom.middleEnd >= i)
			{
				FillRect(hdc, &rect, purple);
			}
			else if (left.middleBegin <= i && left.middleEnd >= i)
			{
				FillRect(hdc, &rect, purple);
			}
			else 
			{
				FillRect(hdc, &rect, black);
			}			
			i++;
		}
		

		EndPaint(_hWnd, &ps);
	}


	LRESULT CALLBACK CalibrateWindow::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		switch (message)
		{
		case WM_KEYDOWN:
			if (wParam == VK_ESCAPE)
			{
				Application::Config().WriteUser("calibrate", false);
				return 1;
			}			
			break;

		case WM_PAINT:
			Console::WriteLine("CalibrateWindow WM_PAINT");
			Paint();
			return DefWindowProc(hWnd, message, wParam, lParam);

		case WM_LBUTTONDOWN:
			SetFocus(_hWnd);
			return 0;		
		}

		return DefWindowProc(hWnd, message, wParam, lParam);
	}


	ATOM CalibrateWindow::RegisterWindowClass()
	{
		WNDCLASSEXW wcex;
		wcex.cbSize = sizeof(WNDCLASSEX);
		wcex.style = 0;
		wcex.lpfnWndProc = _WndProc;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = 0;
		wcex.hInstance = Application::InstanceHandle();
		wcex.hIcon = Resources::Icon().Data();
		wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
		wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
		wcex.lpszMenuName = NULL;
		wcex.lpszClassName = Resources::CalibrateWindowClass().Data();
		wcex.hIconSm = NULL;

		return RegisterClassExW(&wcex);
	}


	LRESULT CALLBACK CalibrateWindow::_WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		if (message == WM_NCCREATE)
		{
			LPCREATESTRUCT lpcs = reinterpret_cast<LPCREATESTRUCT>(lParam);
			SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(lpcs->lpCreateParams));			
		}
		else
		{
			CalibrateWindow* self = reinterpret_cast<CalibrateWindow*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
			if (self)
			{
				return self->WndProc(hWnd, message, wParam, lParam);
			}
		}

		return DefWindowProc(hWnd, message, wParam, lParam);
	}
}