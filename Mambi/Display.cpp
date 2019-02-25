#include "stdafx.h"
#include "Display.h"
#include "Console.h"
#include "Config.h"
#include "Application.h"
#include "utils.h"


#define MAMBI_AUTODETECT_INTERVAL 500


namespace Mambi
{

	Display::Display(): _output(NULL), _profile(NULL), _hEffectThread(NULL), _frame(NULL)
	{		
	}


	Display::~Display()
	{
		StopEffectThread();
		DupedOutput(NULL);		
	}	


	void Display::DupedOutput(DuplicatedOutput* output)
	{
		if (_output != NULL)
		{
			delete _output;
		}
		_output = output;
		UpdateSamples();
	}


	bool Display::Update(const std::string& id, const json& cfg)
	{
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
		MAMBI_CFG_VNUM_INT_RANGE(samplesH, "width", "display.*.samples.horizontal", 0, 65535);
		MAMBI_CFG_VNUM_INT_RANGE(samplesH, "height", "display.*.samples.horizontal", 0, 65535);
		MAMBI_CFG_VNUM_INT_RANGE(samplesH, "margin", "display.*.samples.horizontal", 0, 65535);
		MAMBI_CFG_IS_OBJECT(samples, "vertical", "display.*.samples");
		auto& samplesV = samples["vertical"];
		MAMBI_CFG_VNUM_INT_RANGE(samplesV, "width", "display.*.samples.vertical", 0, 65535);
		MAMBI_CFG_VNUM_INT_RANGE(samplesV, "height", "display.*.samples.vertical", 0, 65535);
		MAMBI_CFG_VNUM_INT_RANGE(samplesV, "margin", "display.*.samples.horizontal", 0, 65535);

		_nativeW = cfg["width"];
		_nativeH = cfg["height"];

		DisplaySampleFactory<SampleOrient::Horizontal> sh(samplesH["width"], samplesH["height"], samplesH["margin"]);
		DisplaySampleFactory<SampleOrient::Vertical> sv(samplesV["width"], samplesV["height"], samplesV["margin"]);
		_samples = DisplaySamples(sh, sv);

		_ledStrip = Application::Led().Get(ledStrip["name"]);
		_stripOffset = ledStrip["offset"];

		if (!_colors.EnsureSize(_ledStrip->Count()))
		{
			ErrorAlert("Error", "Out of memory");
			return false;
		}

		if (Application::Enabled()) 
		{
			if (UpdateSamples())
			{
				StartEffectThread();
				return true;
			}
		}
		else 
		{
			StopEffectThread();
		}
		
		return false;
	}


	bool Display::UpdateSamples()
	{
		if (_output == NULL)
		{
			return true;
		}

		_samples.Update(DesktopWidth(), DesktopHeight(), _nativeW, _nativeH, _ledStrip->HCount(), _ledStrip->VCount());
		_samples.Move(_stripOffset);

		return true;
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
		Console::WriteLine("Display::EffectThread");

		Display* display = reinterpret_cast<Display*>(param);
		Mambi::Profile* profile;
		Mambi::Effect* effect = NULL;
		Mambi::Effect* newEffect = NULL;
		auto detectTime = std::chrono::high_resolution_clock::now();
		auto effectTime = std::chrono::high_resolution_clock::now();
		int elapsed;
		INT16 interval;

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
				}

				interval = effect->interval;
				elapsed = (int) std::chrono::duration_cast<std::chrono::milliseconds>(now - effectTime).count();				

				if (elapsed >= interval)
				{
					effect->Tick(display);

					now = std::chrono::high_resolution_clock::now();
					elapsed = (int)std::chrono::duration_cast<std::chrono::milliseconds>(now - effectTime).count();
					effectTime = std::chrono::high_resolution_clock::now();					

					lock.Release();
					
					interval = (interval - (elapsed - interval));
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