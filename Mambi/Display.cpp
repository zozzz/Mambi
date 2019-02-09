#include "stdafx.h"
#include "Display.h"
#include "Console.h"
#include "Config.h"


namespace Mambi
{

	Display::Display()
	{		
	}


	Display::~Display()
	{
	}	


	bool Display::Update(const json& cfg)
	{
		MAMBI_CFG_VNUM_INT_RANGE(cfg, "width", "display.*", 0, 65535);
		MAMBI_CFG_VNUM_INT_RANGE(cfg, "height", "display.*", 0, 65535);
		MAMBI_CFG_IS_STRING(cfg, "ledStrip", "display.*");
		MAMBI_CFG_IS_OBJECT(cfg, "samples", "display.*");
		auto& samples = cfg["samples"];
		MAMBI_CFG_IS_OBJECT(samples, "horizontal", "display.*.samples");
		auto& samplesH = samples["horizontal"];
		MAMBI_CFG_VNUM_INT_RANGE(samplesH, "width", "display.*.samples.horizontal", 0, 65535);
		MAMBI_CFG_VNUM_INT_RANGE(samplesH, "height", "display.*.samples.horizontal", 0, 65535);
		MAMBI_CFG_VNUM_INT_RANGE(samplesH, "margin", "display.*.samples.horizontal", 0, 65535);
		MAMBI_CFG_IS_OBJECT(samples, "vertical", "display.*.samples");
		auto& samplesV = samples["vertical"];
		MAMBI_CFG_VNUM_INT_RANGE(samplesV, "width", "display.*.samples.vertical", 0, 65535);
		MAMBI_CFG_VNUM_INT_RANGE(samplesV, "height", "display.*.samples.vertical", 0, 65535);
		MAMBI_CFG_VNUM_INT_RANGE(samplesV, "margin", "display.*.samples.horizontal", 0, 65535);

		DisplaySample<SampleOrient::Horizontal> sh(samplesH["width"], samplesH["height"], samplesH["margin"]);
		DisplaySample<SampleOrient::Vertical> sv(samplesV["width"], samplesV["height"], samplesV["margin"]);

		if (_nativeW != cfg["width"] 
			|| _nativeH != cfg["height"] 
			|| _ledStrip != cfg["ledStrip"] 
			|| _sampleH != sh 
			|| _sampleV != sv)
		{
			_cache.empty();
			_sampleH = sh;
			_sampleV = sv;
			_nativeW = cfg["width"];
			_nativeH = cfg["height"];
			_ledStrip = cfg["ledStrip"].get<std::string>();
		}

		return true;
	}

	const Mambi::Samples& Display::Samples(const SampleRequest& request)
	{
		auto cached = _cache.find(request);
		if (cached != _cache.end()) 
		{
			return cached->second;
		}
		else
		{
			Mambi::Samples& samples = _cache[request];

			auto& hSamplesTop = _sampleH.Samples<SampleAlign::Begin>(request.width, request.width / _nativeW, request.hcount);
			auto& hSamplesBot = _sampleH.Samples<SampleAlign::End>(request.width, request.width / _nativeW, request.hcount);
			auto& vSamplesLeft = _sampleV.Samples<SampleAlign::Begin>(request.height, request.height / _nativeH, request.vcount);
			auto& vSamplesRight = _sampleV.Samples<SampleAlign::End>(request.height, request.height / _nativeH, request.vcount);

			samples.insert(samples.end(), hSamplesTop.begin(), hSamplesTop.end());
			samples.insert(samples.end(), hSamplesBot.begin(), hSamplesBot.end());
			samples.insert(samples.end(), vSamplesLeft.begin(), vSamplesLeft.end());
			samples.insert(samples.end(), vSamplesRight.begin(), vSamplesRight.end());

			return samples;
		}
	}


	template<SampleOrient Orient>
	template<SampleAlign Align>
	const Mambi::Samples& DisplaySample<Orient>::Samples<Align>(DisplayDim size, float scale, uint8_t tcount)
	{
		float lsize = size * scale;
		float lwidth = _width * scale;
		float lheight = _height * scale;
		lsize -= _margin * scale * 2;

		if (Orient == SampleOrient::Horizontal)
		{
			int mcount = floor(lsize / lwidth);
			int count = min(tcount, mcount);
			int gap = floor((lsize - (lwidth * count)) / (count - 1));
			Mambi::Samples samples(count);

			DisplayDim x = _margin * scale;
			for (int i = 0; i < count; i++) 
			{
				samples[i].top = (Align == SampleAlign::Begin ? 0 : floor(lsize - lheight));
				samples[i].left = x;
				samples[i].width = floor(lwidth);
				samples[i].height = floor(lheight);				
				x += lwidth + gap
			}

			return samples;
		}
		else if (Orient == SampleOrient::Vertical)
		{
			int mcount = floor(lsize / lheight);
			int count = min(tcount, mcount);
			int gap = floor((lsize - (lheight * count)) / (count - 1));
			Mambi::Samples samples(count);

			DisplayDim y = _margin * scale;
			for (int i = 0; i < count; i++)
			{
				samples[i].top = y;
				samples[i].left = (Align == SampleAlign::Begin ? 0 : floor(lsize - lwidth));
				samples[i].width = floor(lwidth);
				samples[i].height = floor(lheight);
				y += lheight + gap
			}

			return samples;
		}
	}
}