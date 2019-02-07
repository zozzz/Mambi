#pragma once
#include "LedStrip.h"


namespace Mambi
{

	class LedStripManager
	{
	public:
		LedStripManager();
		~LedStripManager();

		void Update();
		void Light(int id, LedStrip::LightInfo& info);

			
		LedStripManager(LedStripManager const&) = delete;
		void operator=(LedStripManager const&) = delete;
	private:
		void LoadStrips(const json& items);

		std::vector<LedStrip> _strips;
	};

}