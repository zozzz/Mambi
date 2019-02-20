#pragma once
#include "stdafx.h"
#include "Display.h"
#include "DuplicatedOutput.h"


namespace Mambi
{
	class DisplayManager
	{
	public:
		typedef std::map<std::string, Display> Displays;
		typedef std::map<UINT, std::shared_ptr<DuplicatedOutput::Device>> AdpaterDevices;

		DisplayManager() {};
		~DisplayManager() {};

		inline auto& DisplayMap() { return _displays; }
		inline auto& Get(const std::string& hardwereId) const { return _displays.at(hardwereId); }

		bool Update();
		bool UpdateOutputs();

		DisplayManager(DisplayManager const&) = delete;
		void operator=(DisplayManager const&) = delete;
	private:
		std::shared_ptr<DuplicatedOutput::Device> GetDevice(CComPtr<IDXGIAdapter1> adapter);
		bool UpdateDisplays(CComPtr<IDXGIAdapter1> adapter, DisplayManager::Displays& displays);

		Displays _displays;
		AdpaterDevices _devices;
	};
}