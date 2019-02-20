#include "stdafx.h"
#include "Effect.h"
#include "Messages.h"
#include "Console.h"
#include "Config.h"


namespace Mambi
{
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
		if (!display->Colors().EnsureSize(display->Samples().Items().size()))
		{
			ErrorAlert("Error", "Out of memory");
			PostQuitMessage(1);
		}
		display->LedStrip()->SetBrightness(brightness);
	}


	void Effect_Ambilight::Tick(Display* display)
	{
		auto frame = display->Frame();
		auto& samples = display->Samples();
		auto& colors = display->Colors();

		if (frame->Acquire())
		{
			frame->UpdateSamples(samples);
			frame->Release();
		}

		int i = 0;
		for (auto& sample : samples.Items())
		{
			colors[i++] = sample.avg;
			//Console::WriteLine("AVG(%d, %d, %d)", sample.avg.r, sample.avg.g, sample.avg.b);
		}

		display->LedStrip()->Light(colors, i);

		/*
		auto frame = display->DupedOutput()->AcquireNextFrame();
		auto& samples = display->Samples();
		auto& colors = display->Colors();
		frame->UpdateSamples(samples);

		int i = 0;
		for (auto& s: samples.Items())
		{
			colors[i++] = s.Avg();
		}

		display->LedStrip()->Light(colors, display->Samples().Items().size());
		*/
	}


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
		_stepBrightness = (float)(maxBrightness - minBrightness) / ((float)_frameCount / 2.0);

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

}