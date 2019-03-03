#pragma once
#include "stdafx.h"
#include "Display.h"


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

		DisplayManager(DisplayManager const&) = delete;
		void operator=(DisplayManager const&) = delete;
	private:
		Mambi::Displays _displays;
	};
}