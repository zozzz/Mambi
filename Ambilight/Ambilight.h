#pragma once

#ifdef AMBILIGHT_EXPORTS
#  define AMBILIGHT_API __declspec(dllexport)
#else
#  define AMBILIGHT_API __declspec(dllimport)
#endif


#include "Output.h"
#include "DisplaySample.h"
#include "Frame.h"
#include "SampleProcessor.h"


namespace Mambi 
{	

	class Ambilight
	{
	public:
		AMBILIGHT_API Ambilight(const char* hardwareId);
		AMBILIGHT_API ~Ambilight();

		inline auto& Output() const { return _output; }
		inline auto& Frame() const { return _frame; }
		inline auto Samples() { return _samples; }

		AMBILIGHT_API bool Available();
		AMBILIGHT_API bool UpdateSamplesDesc(const SampleDesc& horizontal, const SampleDesc& vertical, int8_t rotate);
		AMBILIGHT_API bool ProcessSamples();		

		const char* HardwareId;

	private:
		bool CheckOutput();
		void ResetOutput();
		bool InitOutput();
		void InitFrame();
		void UpdateSamples();

		SampleProcessor _processor;
		std::shared_ptr<DisplaySamples> _samples;
		Mambi::Output* _output;
		Mambi::Frame* _frame;		
	};
};