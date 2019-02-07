#include "stdafx.h"
#include "Display.h"
#include "Console.h"


namespace Mambi
{

	Display::Display()
	{
	}


	Display::~Display()
	{
	}


	void Display::HardwareId(const std::string& id)
	{
		_hardwareId = id;
		Console::WriteLine("HardwareId = %s", id.c_str());
	}



}