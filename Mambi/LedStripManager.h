#pragma once
#include "LedStrip.h"


namespace Mambi
{

	class LedStripManager
	{
	public:
		LedStripManager() {};
		~LedStripManager() {};

		inline auto& Strips() const { return _strips; }
		inline auto  Get(const std::string& name) { return _strips.at(name); }
		bool Update();
			
		LedStripManager(LedStripManager const&) = delete;
		void operator=(LedStripManager const&) = delete;
	private:
		bool LoadStrips(const json& items);

		std::map<std::string, std::shared_ptr<LedStrip>> _strips;
	};

}