#pragma once
#include "stdafx.h"


namespace Mambi
{
	class LedStrip
	{
	public:
		struct LedInfo {

		};

		typedef std::vector<LedInfo> LightInfo;

		LedStrip();
		~LedStrip();

		void Light(LightInfo& info);


		LedStrip(LedStrip const&) = delete;
		void operator=(LedStrip const&) = delete;
	};	
}

