#include "stdafx.h"
#include "DisplayManager.h"
#include "Application.h"
#include "Console.h"


namespace Mambi
{
	DisplayManager::DisplayManager()
	{
	}


	DisplayManager::~DisplayManager()
	{
	}


	void DisplayManager::Update()
	{
		auto& cfg = Application::Config().Data();
		std::vector<std::string> defined;

		if (cfg.count("display")) 
		{
			if (cfg["display"].is_object()) 
			{
				for (auto& el : cfg["display"].items())
				{
					auto& key = el.key();
					auto& value = el.value();

					defined.push_back(key);

					auto& display = _displays[key];
					display.HardwareId(key);

					Console::WriteLine("Key = %s", key.c_str());
				}
			}
			else
			{
				ErrorAlert("Error", "The 'display' option must be an object.");
			}			
		}
		else
		{
			ErrorAlert("Error", "Missing 'display' option in config.");
		}

		// remove unpresented configs
		std::vector<std::string> remove;
		for (auto& el : _displays)
		{
			if (std::find(defined.begin(), defined.end(), el.first) == defined.end())
			{
				remove.push_back(el.first);
			}
		}

		for (auto& el : remove)
		{
			Console::WriteLine("Erase %s", el.c_str());
			_displays.erase(el);
		}
	}
}