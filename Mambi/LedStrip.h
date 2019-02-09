#pragma once
#include "stdafx.h"


namespace Mambi
{
	class LedStripManager;

	class LedStrip
	{
	public:
		friend LedStripManager;

		struct LedInfo {

		};

		typedef std::vector<LedInfo> LightInfo;

		LedStrip();
		~LedStrip();


		inline uint8_t HCount() const { return _countH; }
		inline uint8_t VCount() const { return _countV; }
		bool Update(const json& cfg);
		void Light(LightInfo& info);


		LedStrip(LedStrip const&) = delete;
		void operator=(LedStrip const&) = delete;

	protected:
		uint8_t _countH;
		uint8_t _countV;
	};	
}

