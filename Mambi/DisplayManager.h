#pragma once
#include "stdafx.h"
#include "Display.h"


namespace Mambi
{
	class DisplayManager
	{
	public:
		DisplayManager();
		~DisplayManager();

		void Update();


		DisplayManager(DisplayManager const&) = delete;
		void operator=(DisplayManager const&) = delete;
	private:
		std::map<std::string, Display> _displays;
	};
}