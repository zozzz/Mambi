#include "stdafx.h"
#include "LedStrip.h"
#include "Config.h"
#include "Console.h"


namespace Mambi
{

	LedStrip::LedStrip(): _countH(0), _countV(0)
	{
	}


	LedStrip::~LedStrip()
	{
	}


	bool LedStrip::Update(const json& cfg)
	{
		MAMBI_CFG_EXISTS(cfg, "port", "ledStrip.*");
		MAMBI_CFG_IS_OBJECT(cfg, "leds", "ledStrip.*");
		auto& leds = cfg["leds"];
		MAMBI_CFG_VNUM_INT_RANGE(leds, "horizontal", "ledStrip.*", 0, 200);
		MAMBI_CFG_VNUM_INT_RANGE(leds, "vertical", "ledStrip.*", 0, 200);

		_countH = leds["horizontal"];
		_countV = leds["vertical"];

		Console::WriteLine("leds (%d x %d)", _countH, _countV);

		return true;
	}


	void LedStrip::Light(LedStrip::LightInfo& info)
	{
	}

}
