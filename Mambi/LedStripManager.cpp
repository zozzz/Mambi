#include "stdafx.h"
#include "LedStripManager.h"
#include "Application.h"
#include "Console.h"


namespace Mambi
{

	LedStripManager::LedStripManager()
	{
		Console::WriteLine("LedStripManager::LedStripManager");
	}


	LedStripManager::~LedStripManager()
	{
	}


	void LedStripManager::Update()
	{
		auto& cfg = Application::Config().Data();
		
		if (cfg.count("ledStrip")) 
		{
			if (cfg["ledStrip"].is_array())
			{
				LoadStrips(cfg["ledStrip"]);
			}
			else
			{
				ErrorAlert("Error", "The 'ledStrip' option is must be an array.");
			}
		}
		else
		{
			ErrorAlert("Error", "Missing 'ledStrip' option in config file.");			
		}
	}


	void LedStripManager::LoadStrips(const json& items)
	{
		int count = 0;
		for (auto& el : items.items()) 
		{
			auto& item = el.value();
			
			if (item.count("port"))
			{
			}
			else
			{
				ErrorAlert("Error", "Missing 'port' from ledstrip");
			}

			count++;
		}

		int diff = _strips.size() - count;
		while (diff-- >= 0) 
		{
			_strips.pop_back();
		}

		//_strips.resize(count);
	}


	void LedStripManager::Light(int id, LedStrip::LightInfo& info)
	{
	}

}