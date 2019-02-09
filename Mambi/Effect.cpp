#include "stdafx.h"
#include "Effect.h"
#include "Messages.h"
#include "Console.h"
#include "Config.h"


namespace Mambi
{
	Effect* Effect::New(const json& cfg)
	{
		if (cfg.count("type") == 0) 
		{
			ErrorAlert("Error", "Missing filter type config");
			return NULL;
		}

		Effect* res;
		if (cfg["type"] == "ambilight")
		{
			res = new Effect_Ambilight("ambilight");
		}
		else if (cfg["type"] == "breath")
		{
			res = new Effect_Breath("breath");
		}
		else if (cfg["type"] == "static")
		{
			res = new Effect_Static("static");
		}
		else
		{
			ErrorAlert("Error", "Unexpected effect type");
			return NULL;
		}

		if (res->Update(cfg))
		{
			return res;
		}
		else
		{
			return NULL;
		}
	}

	Effect::Effect(const char* type): _type(type)
	{		
	}


	Effect::~Effect()
	{
	}


	bool Effect_Ambilight::Update(const json& cfg)
	{
		return true;
	}


	void Effect_Ambilight::Tick()
	{
	}


	bool Effect_Breath::Update(const json& cfg)
	{
		MAMBI_CFG_VNUM_INT_RANGE(cfg, "duration", "breath effect", 0, 100000);
		MAMBI_CFG_VNUM_INT_RANGE(cfg, "min", "breath effect", 0, 100);
		MAMBI_CFG_VNUM_INT_RANGE(cfg, "max", "breath effect", 0, 100);		
		
		duration = cfg["duration"];
		min = cfg["min"];
		max = cfg["max"];

		return true;
	}


	void Effect_Breath::Tick()
	{
	}


	bool Effect_Static::Update(const json& cfg)
	{
		if (cfg.count("color")) 
		{			
			if (!Color::RedFromHex(_color, cfg["color"]))
			{
				ErrorAlert("Error", "Invalid value of 'color' option in static effect");
				return false;
			}					
		}
		else
		{
			ErrorAlert("Error", "Missing 'color' option from static effect");
			return false;
		}
		return true;
	}


	void Effect_Static::Tick()
	{
	}

}