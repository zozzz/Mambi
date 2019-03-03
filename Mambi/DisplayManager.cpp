#include "stdafx.h"
#include "DisplayManager.h"
#include "Application.h"
#include "Console.h"


namespace Mambi
{

	bool DisplayManager::Update()
	{
		auto& cfg = Application::Config().Data();
		std::vector<std::string> defined;

		MAMBI_CFG_IS_OBJECT(cfg, "display", "config");
		
		for (auto& el : cfg["display"].items())
		{
			auto& key = el.key();
			auto& value = el.value();

			defined.push_back(key);

			if (!_displays[key].Update(key, value))
			{
				_displays.clear();
				return false;
			}

			if (value.count("profile") == 0 || value["profile"].is_null())
			{
				Application::Profile().Activate(key, nullptr);
			}
			else if (value["profile"].is_string())
			{
				Application::Profile().Activate(key, value["profile"].get<std::string>());
			}
			else
			{
				Application::Profile().Activate(key, nullptr);
				ErrorAlert("Error", "Invalid 'profile' type in display config, must be string");
			}
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
			_displays.erase(el);
		}

		return true;
	}
}
