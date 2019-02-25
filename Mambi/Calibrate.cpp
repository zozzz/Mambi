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
		for (auto& item: Application::Display().DisplayMap())
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
		int winWidth = display.DesktopWidth();
		int winHeight = display.DesktopHeight();
		

		_hWnd = CreateWindowEx(
			0,
			Resources::CalibrateWindowClass().Data(),
			Resources::CalibrateWindowClass().Data(),
			WS_POPUP,
			//0, 0, display.DesktopWidth(), display.DesktopHeight(),
			0, 0, winWidth, winHeight,
			nullptr, nullptr, Application::InstanceHandle(), this);

		if (!_hWnd)
		{
			throw new std::exception("TODO: GetLastError.");
		}

		// SW_HIDE, SW_FORCEMINIMIZE
		ShowWindow(_hWnd, SW_SHOW);
		UpdateWindow(_hWnd);

		/*
		std::string horWidth = "/display/" + display.HardwareId() + "/samples/horizontal/width";
		std::string horHeight = "/display/" + display.HardwareId() + "/samples/horizontal/height";
		std::string horMargin = "/display/" + display.HardwareId() + "/samples/horizontal/margin";

		std::string verWidth = "/display/" + display.HardwareId() + "/samples/vertical/width";
		std::string verHeight = "/display/" + display.HardwareId() + "/samples/vertical/height";
		std::string verMargin = "/display/" + display.HardwareId() + "/samples/vertical/margin";
		
		
		int w = 400;
		int h = 6 *40;
		int x = winWidth / 2 - w / 2;
		int y = winHeight / 2 - h / 2;

		_trackbars.push_back(new Trackbar(_hWnd, x, y, 400, 30, horWidth.c_str(), 0, 200));
		_trackbars.push_back(new Trackbar(_hWnd, x, y + 40, 400, 30, horHeight.c_str(), 0, 500));
		_trackbars.push_back(new Trackbar(_hWnd, x, y + 80, 400, 30, horMargin.c_str(), 0, 500));
		*/
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

			/*
			for (auto& item : _trackbars)
			{
				delete item;
			}
			_trackbars.clear();
			*/
		}
	}

	
	void CalibrateWindow::Update()
	{
		if (_samples != NULL)
		{
			delete _samples;
			_samples = NULL;
		}

		auto hFactory = const_cast<Display*>(&_display)->Samples().HFactory();
		auto vFactory = const_cast<Display*>(&_display)->Samples().VFactory();
		_samples = new DisplaySamples(hFactory, vFactory);
		_samples->Update(
			_display.DesktopWidth(), _display.DesktopHeight(),
			_display.NativeWidth(), _display.NativeHeight(),
			_display.LedStrip()->HCount(),
			_display.LedStrip()->VCount());
		RedrawWindow(_hWnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE);
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
		//for (auto& s : const_cast<Display&>(_display).Samples().Items())
		{
			RECT rect = { s.src.left, s.src.top, s.src.right, s.src.bottom };
			//Console::WriteLine("DRAW(%ld, %ld, %ld, %ld)", rect.left, rect.top, rect.right, rect.bottom);
		
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
			

			//FillRect(hdc, &rect, brushes[i % bc]);
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

		/*
		case WM_HSCROLL:
			HWND hTrack = reinterpret_cast<HWND>(lParam);
			switch (LOWORD(wParam))
			{
			case TB_ENDTRACK:
				DWORD pos = SendMessage(hTrack, TBM_GETPOS, 0, 0);
				Console::WriteLine("HWND = %p", hTrack);
				const char* path = reinterpret_cast<const char*>(GetWindowLongPtr(hTrack, GWLP_USERDATA));
				Console::WriteLine("TB_ENDTRACK %s = %d", path, pos);
				// TODO: wind trackbar by hTrackbar and set valu
				break;
			}
		*/
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
			//SetWindowLong(hWnd, GWL_EXSTYLE, 0);
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





/*
	CalibrateWindow::Trackbar::Trackbar(HWND parent, int x, int y, int w, int h, const char* path, int minV, int maxV): _path(path)
	{
		int value = Application::Config().GetPath(path);
			   
		_hWnd = CreateWindowEx(
			0,                               // no extended styles 
			TRACKBAR_CLASS,                  // class name 
			L"Title",              // title (caption) 
			WS_CHILD | WS_VISIBLE | TBS_NOTICKS | TBS_ENABLESELRANGE | TBS_TOOLTIPS, // style 
			x, y,                          // position 
			w, h,                         // size 
			parent,                           // parent window 
			NULL,                     // control identifier 
			Application::InstanceHandle(),                         // instance 
			NULL                             // no WM_CREATE parameter 
		);
		
		SetWindowLongPtr(_hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(path));

		SendMessage(_hWnd, TBM_SETRANGE,
			(WPARAM)TRUE,                   // redraw flag 
			(LPARAM)MAKELONG(minV, maxV));  // min. & max. positions

		SendMessage(_hWnd, TBM_SETPAGESIZE,
			0, (LPARAM)1);                  // new page size 

		SendMessage(_hWnd, TBM_SETSEL,
			(WPARAM)FALSE,                  // redraw flag 
			(LPARAM)MAKELONG(value, value));

		SendMessage(_hWnd, TBM_SETPOS,
			(WPARAM)TRUE,                   // redraw flag 
			(LPARAM)value);
	}


	CalibrateWindow::Trackbar::~Trackbar()
	{
		DestroyWindow(_hWnd);
	}

*/
	

#if 0
	void Calibrate::StopDraw()
	{
		if (_drawThread != NULL)
		{
			MAMBI_MLOCK_BEGIN(_mutex, INFINITE)
				if (TerminateThread(_drawThread, 0))
				{
					CloseHandle(_drawThread);
					_drawThread = NULL;
				}
			MAMBI_MLCOK_END_VOID(_mutex)

			// force redraw desktop
			RECT rect;
			GetClientRect(GetDesktopWindow(), &rect);
			RedrawWindow(NULL, &rect, NULL, RDW_ERASENOW | RDW_INVALIDATE | RDW_ALLCHILDREN);
		}		
	}


	void Calibrate::DisplayWindows()
	{
		StopDraw();
		MAMBI_MLOCK_BEGIN(_mutex, INFINITE)
			_drawThread = CreateThread(NULL, 0, DrawThread, this, 0, NULL);
		MAMBI_MLCOK_END_VOID(_mutex)
		
		/*
		Console::WriteLine("Calibrate::DisplayWindows");

		HWND hDesktop = GetDesktopWindow();
		HDC hDC_Desktop = GetDC(hDesktop);

		RECT rect = { 20, 20, 200, 200 };
		HBRUSH blueBrush = CreateSolidBrush(RGB(0, 0, 255));
		FillRect(hDC_Desktop, &rect, blueBrush);

		ReleaseDC(hDesktop, hDC_Desktop);
		*/
		
	}


	void Calibrate::DrawWindows()
	{
		HWND hDesktop = GetDesktopWindow();
		HDC hDC_Desktop = GetDC(hDesktop);
		auto releaseDc = Finally([&] {
			ReleaseDC(hDesktop, hDC_Desktop);
		});

		
		for (auto& item: Application::Display().DisplayMap())
		{
			for (auto& s: item.second.Samples())
			{
				RECT rect = { s.left, s.top, s.left + s.width, s.top + s.height };
				//Console::WriteLine("DRAW(%ld, %ld, %ld, %ld)", rect.left, rect.top, rect.right, rect.bottom);
				HBRUSH blueBrush = CreateSolidBrush(RGB(0, 0, 255));
				FillRect(hDC_Desktop, &rect, blueBrush);
			}
		}		
	}


	DWORD WINAPI Calibrate::DrawThread(LPVOID lpParam)
	{
		Calibrate* self = (Calibrate*)lpParam;
		while (true)
		{
			MAMBI_MLOCK_BEGIN(self->_mutex, INFINITE)
				self->DrawWindows();
			MAMBI_MLCOK_END(self->_mutex, FALSE)
			//Sleep(1000);
		}

		return 0;
	}	

#endif
}