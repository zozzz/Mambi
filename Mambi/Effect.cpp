#include "stdafx.h"
#include "Effect.h"
#include "Messages.h"
#include "Console.h"
#include "Config.h"


namespace Mambi
{

#pragma region Effect

	Effect* Effect::New(const json& cfg)
	{
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
		else if (cfg["type"] == "rainbow")
		{
			res = new Effect_Rainbow("rainbow");
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

	Effect::Effect(const char* type): _type(type), interval(0)
	{		
	}


	Effect::~Effect()
	{
	}

#pragma endregion


#pragma region Effect_Ambilight

	bool Effect_Ambilight::Update(const json& cfg)
	{
		MAMBI_CFG_VNUM_INT_RANGE(cfg, "fps", "profile / ambilight effect", 0, 255);
		MAMBI_CFG_VNUM_INT_RANGE(cfg, "brightness", "profile / static effect", 0, 255);

		uint8_t fps = cfg["fps"].get<uint8_t>();
		interval = (UINT16)round(1000 / fps);
		brightness = cfg["brightness"];
		return true;
	}


	void Effect_Ambilight::Init(Display* display)
	{
		if (!display->Colors().EnsureSize(display->Ambilight()->Samples()->Items().size()))
		{
			ErrorAlert("Error", "Out of memory");
			PostQuitMessage(1);
		}
		display->Colors().Zero();
		display->LedStrip()->SetBrightness(brightness);		
	}


	void Effect_Ambilight::Tick(Display* display)
	{
		auto ambilight = display->Ambilight();
		if (ambilight->ProcessSamples())
		{
			auto& data = display->Colors();
			size_t i = 0;
			for (auto& sample : ambilight->Samples()->Items())
			{
				data[i++] = sample.Avg;
			}
		}
		//display->LedStrip()->Light(display->Colors(), display->LedStrip()->Count());
		display->LedStrip()->Transition(interval, display->Colors(), display->LedStrip()->Count());
	}

#pragma endregion


#pragma region Effect_Breath

	bool Effect_Breath::Update(const json& cfg)
	{
		MAMBI_CFG_VNUM_INT_RANGE(cfg, "fps", "profile / breath effect", 0, 255);
		MAMBI_CFG_VNUM_INT_RANGE(cfg, "duration", "profile / breath effect", 0, 100000);
		MAMBI_CFG_VNUM_INT_RANGE(cfg, "minBrightness", "profile / breath effect", 0, 255);
		MAMBI_CFG_VNUM_INT_RANGE(cfg, "maxBrightness", "profile / breath effect", 0, 255);
		MAMBI_CFG_EXISTS(cfg, "color", "profile / breath effect");
		
		duration = cfg["duration"];
		minBrightness = cfg["minBrightness"];
		maxBrightness = cfg["maxBrightness"];

		uint8_t fps = cfg["fps"].get<uint8_t>();
		interval = (UINT16)round(1000 / fps);
		_frameCount = (UINT16)round(duration / interval);
#pragma warning( push )
#pragma warning( disable : 4244)
		_stepBrightness = (float)(maxBrightness - minBrightness) / ((float)_frameCount / 2.0);
#pragma warning( pop ) 

		if (!Color::RedFromHex(color, cfg["color"]))
		{
			ErrorAlert("Error", "Invalid value of 'color' option in 'profile / breath effect'");
			return false;
		}

		return true;
	}

	void Effect_Breath::Init(Display* display)
	{
		_currentFrame = 0;
		display->LedStrip()->SetBrightness(minBrightness);

		LedStrip::IndexType lc = display->LedStrip()->Count();
		auto& data = display->Colors();

		for (LedStrip::IndexType i = 0; i < lc; ++i)
		{
			data[i] = color;
		}

		display->LedStrip()->Light(display->Colors(), display->LedStrip()->Count());
	}

	void Effect_Breath::Tick(Display* display)
	{
		uint8_t b;

		if (_currentFrame >= _frameCount)
		{
			_currentFrame = 0;
			b = minBrightness;
		}
		else if (_currentFrame >= _frameCount / 2)
		{
			b = maxBrightness - (uint8_t)round((_currentFrame - _frameCount / 2) * _stepBrightness);
		}
		else
		{
			b = minBrightness + (uint8_t)round(_currentFrame * _stepBrightness);
		}

		display->LedStrip()->SetBrightness(b);
		++_currentFrame;
	}

#pragma endregion


#pragma region Effect_Static

	bool Effect_Static::Update(const json& cfg)
	{
		MAMBI_CFG_EXISTS(cfg, "color", "profile / static effect");
		MAMBI_CFG_VNUM_INT_RANGE(cfg, "brightness", "profile / static effect", 0, 255);

		interval = 2000;
		brightness = cfg["brightness"];

		if (!Color::RedFromHex(color, cfg["color"]))
		{
			ErrorAlert("Error", "Invalid value of 'color' option in 'profile / static effect'");
			return false;
		}
		
		return true;
	}

	void Effect_Static::Init(Display* display)
	{
		display->LedStrip()->SetBrightness(brightness);
		LedStrip::IndexType lc = display->LedStrip()->Count();
		auto& data = display->Colors();

		for (LedStrip::IndexType i = 0; i < lc; ++i)
		{
			data[i] = color;
		}
	}

	void Effect_Static::Tick(Display* display)
	{
		display->LedStrip()->Light(display->Colors(), display->LedStrip()->Count());
	}

#pragma endregion


#pragma region Effect_Rainbow

	bool Effect_Rainbow::Update(const json& cfg)
	{
		Console::WriteLine("Effect_Rainbow::Update");
		MAMBI_CFG_VNUM_INT_RANGE(cfg, "duration", "profile / rainbow effect", 0, 100000);
		MAMBI_CFG_VNUM_INT_RANGE(cfg, "brightness", "profile / rainbow effect", 0, 255);
		MAMBI_CFG_IS_ARRAY(cfg, "colors", "profile / rainbow effect");

		duration = cfg["duration"];
		brightness = cfg["brightness"];

		_frames.clear();

		for (auto& item: cfg["colors"]) 
		{
			if (item.is_array() && item.size() == 2)
			{
				Keyframe kf;
				kf.position = item[0];
				if (!Color::RedFromHex(kf.color, item[1].get<std::string>())) 
				{
					_frames.clear();
					ErrorAlert("Mambi", "Colors entry must be array with size of 2 in 'profile / rainbow effect'");
					return false;
				}
				_frames.push_back(kf);
			}
			else
			{
				_frames.clear();
				ErrorAlert("Mambi", "Colors entry must be array with size of 2 in 'profile / rainbow effect'");
				return false;
			}
		}

		if (_frames.size() < 1)
		{
			ErrorAlert("Mambi", "Colors must have at least two entry in 'profile / rainbow effect'");
			return false;
		}

		interval = 0;

		return true;
	}


	void Effect_Rainbow::Init(Display* display)
	{
		display->LedStrip()->SetBrightness(brightness);		
	}

	void Effect_Rainbow::Tick(Display* display)
	{
		LedStrip::IndexType lc = display->LedStrip()->Count();
		auto& data = display->Colors();

		if (_currentFrame == 0) 
		{
			auto& frame = _frames[0];
			for (LedStrip::IndexType i = 0; i < lc; ++i)
			{
				data[i] = frame.color;
			}
			display->LedStrip()->Light(display->Colors(), display->LedStrip()->Count());
			interval = 0;
			Console::WriteLine("FRAME %d RGB(%d, %d, %d) ~ %d", _currentFrame, frame.color.r, frame.color.g, frame.color.b, interval);
			++_currentFrame;
		}
		else if (_currentFrame < _frames.size())
		{
			auto& frame = _frames[_currentFrame];
			for (LedStrip::IndexType i = 0; i < lc; ++i)
			{
				data[i] = frame.color;
			}

			interval = static_cast<UINT16>(round((_frames[_currentFrame].position - _frames[_currentFrame - 1].position) * duration));
			Console::WriteLine("FRAME %d RGB(%d, %d, %d) ~ %d", _currentFrame, frame.color.r, frame.color.g, frame.color.b, interval);
			display->LedStrip()->Transition(interval, display->Colors(), display->LedStrip()->Count());
			++_currentFrame;
		}
		else
		{
			_currentFrame = 0;
			interval = 0;
		}		
	}

#pragma endregion
}