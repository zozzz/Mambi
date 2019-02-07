#include "stdafx.h"
#include "Effect.h"
#include "Messages.h"


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

		return res;
	}


	Effect::Effect(const char* type)
	{
		_type = type;
	}


	Effect::~Effect()
	{
	}


	void Effect::Update(const json& cfg)
	{
	}


	void Effect_Ambilight::Update(const json& cfg)
	{
	}


	void Effect_Ambilight::Tick()
	{
	}


	void Effect_Breath::Update(const json& cfg)
	{
	}


	void Effect_Breath::Tick()
	{
	}


	void Effect_Static::Update(const json& cfg)
	{
		if (cfg.count("color")) 
		{
			if (!Color::ReadHexInto(cfg["color"], _color))
			{
				ErrorAlert("Error", "Invalid value of 'color' option in static effect");
			}
		}
		else
		{
			ErrorAlert("Error", "Missing 'color' option from static effect");
		}
	}


	void Effect_Static::Tick()
	{
	}

}