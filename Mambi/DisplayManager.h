#pragma once
#include "stdafx.h"
#include "Display.h"
#include "DuplicatedOutput.h"


namespace Mambi
{
	typedef std::map<std::string, Display> Displays;

	class DisplayManager
	{
	public:		
		DisplayManager() {};
		~DisplayManager() {};

		inline auto& Displays() { return _displays; }
		inline auto& Get(const std::string& hardwereId) const { return _displays.at(hardwereId); }

		bool Update();
		//bool UpdateOutputs();

		DisplayManager(DisplayManager const&) = delete;
		void operator=(DisplayManager const&) = delete;
	private:
		//bool UpdateDisplays(CComPtr<IDXGIAdapter1> adapter, DisplayManager::Displays& displays);

		Mambi::Displays _displays;
	};
}