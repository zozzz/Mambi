#include "stdafx.h"
#include "Display.h"
#include "Console.h"
#include "Config.h"
#include "Application.h"
#include "utils.h"


#define MAMBI_AUTODETECT_INTERVAL 500


namespace Mambi
{

	Display::Display(): _ambilight(NULL), _profile(NULL), _hEffectThread(NULL)
	{		
	}


	Display::~Display()
	{
		StopEffectThread();		
	}	


	bool Display::Update(const std::string& id, const json& cfg)
	{
		StopEffectThread();

		_hardwareId = id;
		MAMBI_CFG_VNUM_INT_RANGE(cfg, "width", "display.*", 0, 65535);
		MAMBI_CFG_VNUM_INT_RANGE(cfg, "height", "display.*", 0, 65535);
		MAMBI_CFG_IS_OBJECT(cfg, "ledStrip", "display.*");
		auto& ledStrip = cfg["ledStrip"];
		MAMBI_CFG_IS_STRING(ledStrip, "name", "display.*.ledStrip");
		MAMBI_CFG_VNUM_INT_RANGE(ledStrip, "offset", "display.*.ledStrip", -1000, 1000);		
		auto& samples = cfg["samples"];
		MAMBI_CFG_IS_OBJECT(samples, "horizontal", "display.*.samples");
		auto& samplesH = samples["horizontal"];
		MAMBI_CFG_VNUM_INT_RANGE(samplesH, "width", "display.*.samples.horizontal", MAMBI_SAMPLE_MIN_WIDTH, 65535);
		MAMBI_CFG_VNUM_INT_RANGE(samplesH, "height", "display.*.samples.horizontal", MAMBI_SAMPLE_MIN_HEIGHT, 65535);
		MAMBI_CFG_VNUM_INT_RANGE(samplesH, "hpadding", "display.*.samples.horizontal", 0, 65535);
		MAMBI_CFG_VNUM_INT_RANGE(samplesH, "vpadding", "display.*.samples.horizontal", 0, 65535);
		MAMBI_CFG_IS_OBJECT(samples, "vertical", "display.*.samples");
		auto& samplesV = samples["vertical"];
		MAMBI_CFG_VNUM_INT_RANGE(samplesV, "width", "display.*.samples.vertical", MAMBI_SAMPLE_MIN_WIDTH, 65535);
		MAMBI_CFG_VNUM_INT_RANGE(samplesV, "height", "display.*.samples.vertical", MAMBI_SAMPLE_MIN_HEIGHT, 65535);
		MAMBI_CFG_VNUM_INT_RANGE(samplesV, "hpadding", "display.*.samples.horizontal", 0, 65535);
		MAMBI_CFG_VNUM_INT_RANGE(samplesV, "vpadding", "display.*.samples.horizontal", 0, 65535);

		_nativeW = cfg["width"];
		_nativeH = cfg["height"];

		// TODO: error check
		_ledStrip = Application::Led().Get(ledStrip["name"]);
		if (!_colors.EnsureSize(_ledStrip->Count()))
		{
			ErrorAlert("Error", "Out of memory");
			return false;
		}

		SampleDesc hDesc = {
			/* HPadding */ samplesH["hpadding"],
			/* VPadding */ samplesH["vpadding"],
			/* Width */    samplesH["width"],
			/* Height */   samplesH["height"],
			/* Count */    _ledStrip->HCount()
		};
		SampleDesc VDesc = {
			/* HPadding */ samplesV["hpadding"],
			/* VPadding */ samplesV["vpadding"],
			/* Width */    samplesV["width"],
			/* Height */   samplesV["height"],
			/* Count */    _ledStrip->VCount()
		};

#if MAMBI_SP_SSE
		if (hDesc.Width % MAMBI_SP_SSE_PIXEL_PER_CYCLE != 0)
		{
			ErrorAlert("Mambi", "display.*.samples.horizontal.width mist be deviadable with " MAMBI_STRINGIFY(MAMBI_SP_SSE_PIXEL_PER_CYCLE));
			return false;
		}
		if (VDesc.Width % MAMBI_SP_SSE_PIXEL_PER_CYCLE != 0)
		{
			ErrorAlert("Mambi", "display.*.samples.horizontal.width mist be deviadable with " MAMBI_STRINGIFY(MAMBI_SP_SSE_PIXEL_PER_CYCLE));
			return false;
		}
#endif

		_ambilight.reset();
		_ambilight = std::make_shared<Mambi::Ambilight>(id.c_str());
		_ambilight->UpdateSamplesDesc(hDesc, VDesc, ledStrip["offset"]);
		
		if (Application::Enabled()) 
		{			
			StartEffectThread();
			return true;
		}
		else 
		{
			StopEffectThread();
		}
		
		return false;
	}


	void Display::StartEffectThread()
	{
		if (_hEffectThread == NULL)
		{
			_hEffectThread = CreateThread(NULL, 0, EffectThread, this, 0, NULL);
		}
	}


	void Display::StopEffectThread()
	{
		if (_hEffectThread != NULL)
		{
			TerminateThread(_hEffectThread, 0);
			CloseHandle(_hEffectThread);
			_hEffectThread = NULL;
		}
	}


	DWORD WINAPI Display::EffectThread(LPVOID param)
	{
		Display* display = reinterpret_cast<Display*>(param);
		Mambi::Profile* profile;
		Mambi::Effect* effect = NULL;
		Mambi::Effect* newEffect = NULL;
		auto detectTime = std::chrono::high_resolution_clock::now();
		auto effectTime = std::chrono::high_resolution_clock::now();
		int elapsed;
		int interval;

		while (true)
		{
			AcquireMutex lock(Application::Config().Mutex(), 100);
			if (lock)
			{
				auto now = std::chrono::high_resolution_clock::now();
				profile = display->_profile.get();
				
				if (profile->Type() == ProfileType::Auto)
				{
					elapsed = (int) std::chrono::duration_cast<std::chrono::milliseconds>(now - detectTime).count();
					if (((AutoProfile*)profile)->Selected() == nullptr || elapsed >= MAMBI_AUTODETECT_INTERVAL)
					{
						//Console::WriteLine("AutoDetect... %d", elapsed);
						detectTime = std::chrono::high_resolution_clock::now();
						auto status = ((AutoProfile*)profile)->Detect(display);
						
						switch (status)
						{
						case AutoProfile::Status::Failed:
							//Console::WriteLine("AutoProfile::Status::Failed");
							// if autodetect failed, just wait for next tick to detect the correct profile
							Sleep(50);
							continue;

						case AutoProfile::Status::Success:
							//Console::WriteLine("AutoProfile::Status::Success");
							// do nothing, just continue processing the effect
							break;

						case AutoProfile::Status::Changed:
							//Console::WriteLine("AutoProfile::Status::Changed");
							break;
						}						
					}
				}	
				
				if (effect == NULL)
				{
					effect = profile->Effect();
					effect->Init(display);
					elapsed = effect->interval;
				}
				else
				{
					newEffect = profile->Effect();
					if (newEffect != effect)
					{
						effect = newEffect;
						effect->Init(display);						
					}
					else if (*newEffect != *effect)
					{
						effect->Init(display);						
					}

					elapsed = (int)std::chrono::duration_cast<std::chrono::milliseconds>(now - effectTime).count();
				}

				interval = effect->interval;
				if (elapsed >= interval)
				{
					effect->Tick(display);

					now = std::chrono::high_resolution_clock::now();
					elapsed = (int)std::chrono::duration_cast<std::chrono::milliseconds>(now - effectTime).count();
					effectTime = std::chrono::high_resolution_clock::now();					

					lock.Release();
					
					// Rainbow effect replace interval value after Tick
					interval = effect->interval;
					interval -= (elapsed - interval);
					interval = max(0, interval);					
				}
				else
				{
					lock.Release();					
					interval -= elapsed;					
				}			

				if (profile->Type() == ProfileType::Auto)
				{
					interval = min(MAMBI_AUTODETECT_INTERVAL, interval);					
				}
				
				if (interval > 10)
				{
					//Console::WriteLine("Sleep %d // %d", interval, effect->interval);
					Sleep(interval);
				}
			}
		}
	}

}