#include "stdafx.h"
#include "LedStripManager.h"
#include "Application.h"
#include "Console.h"


namespace Mambi
{
	bool LedStripManager::Update()
	{
		auto& cfg = Application::Config().Data();

		MAMBI_CFG_IS_OBJECT(cfg, "ledStrip", "config");

		if (!LoadStrips(cfg["ledStrip"]))
		{
			_strips.clear();
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
			
			if (_strips.find(key) == _strips.end())
			{
				std::shared_ptr<LedStrip> strip(new LedStrip());
				if (strip->Update(item))
				{
					_strips[key] = strip;
				}
			}
			else
			{
				_strips[key]->Update(item);
			}

			defined.push_back(key);
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

}
