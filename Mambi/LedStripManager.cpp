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


	bool LedStripManager::Update()
	{
		auto& cfg = Application::Config().Data();
		
		MAMBI_CFG_IS_OBJECT(cfg, "ledStrip", "config");

		if (!LoadStrips(cfg["ledStrip"]))
		{
			_strips.empty();
			return false;
		}

		return true;
	}


	bool LedStripManager::LoadStrips(const json& items)
	{
		std::vector<std::string> defined;

		for (auto& el : items.items()) 
		{
			auto& key = el.key();
			auto& item = el.value();
			
			if (!_strips[key].Update(item))
			{
				_strips.empty();
				return false;
			}
		}

		// remove unpresented configs
		std::vector<std::string> remove;
		for (auto& el : _strips)
		{
			if (std::find(defined.begin(), defined.end(), el.first) == defined.end())
			{
				remove.push_back(el.first);
			}
		}

		for (auto& el : remove)
		{
			_strips.erase(el);
		}

		return true;
	}


	void LedStripManager::Light(int id, LedStrip::LightInfo& info)
	{
	}

}