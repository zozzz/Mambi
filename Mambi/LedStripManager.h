#pragma once
#include "LedStrip.h"


namespace Mambi
{

	class LedStripManager
	{
	public:
		LedStripManager();
		~LedStripManager();

		bool Update();
		void Light(int id, LedStrip::LightInfo& info);

			
		LedStripManager(LedStripManager const&) = delete;
		void operator=(LedStripManager const&) = delete;
	private:
		bool LoadStrips(const json& items);

		std::map<std::string, LedStrip> _strips;
	};

}