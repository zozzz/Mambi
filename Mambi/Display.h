#pragma once
#include "stdafx.h"

/*

Correct. the method to read EDID is private to the display/monitor driver. it does not expose a public API for you to get at the EDID directly.  why cant you use the data in the device instance (what you refer to as HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Enum\DISPLAY\{Type}\{SWnummer}\Device Parameters) ? is it because you can't construct this path in the registry ahead of time? that is by design.  you need to use SetupDiGetClassDevices(GUID_DEVINTERFACE_MONITOR)/SetupDiEnumDeviceInterfaces/SetupDiOpenDevRegKeyto get an HKEY to this path.

in other words, the path is an abstraction that you should not parse, the system provides APIs for you to get to the Device Parameters key without knowing the path


https://ofekshilon.com/2011/11/13/reading-monitor-physical-dimensions-or-getting-the-edid-the-right-way/
https://en.wikipedia.org/wiki/Extended_Display_Identification_Data

https://github.com/smasherprog/screen_capture_lite/blob/master/src/windows/DXFrameProcessor.cpp
*/

namespace Mambi
{
	typedef float DisplayDim;


	class Display;


	enum SampleOrient {
		Horizontal = 1,
		Vertical = 1
	};


	enum SampleAlign {
		Begin = 1,
		End = 1		
	};


	struct SampleArea {
		DisplayDim top;
		DisplayDim left;
		DisplayDim width;
		DisplayDim height;
	};


	typedef std::vector<SampleArea> Samples;


	template<SampleOrient Orient>
	class DisplaySample {
	public:
		friend Display;

		inline DisplaySample()
			: _width(0), _height(0), _margin(0) { }

		inline DisplaySample(DisplayDim w, DisplayDim h, DisplayDim m) 
			: _width(w), _height(h), _margin(m) { }

		template<SampleAlign Align>
		const void Samples(Mambi::Samples& samples, DisplayDim size, float scale, uint8_t tcount) const;
		
		inline bool operator==(const DisplaySample& other) 
		{
			return other._width == _width && other._height == _height && other._margin == _margin;
		}
		inline bool operator!=(const DisplaySample& other)
		{
			return other._width != _width || other._height != _height || other._margin != _margin;
		}

	private:
		DisplayDim _width;
		DisplayDim _height;
		DisplayDim _margin;
	};


	class Display
	{
	public:
		struct SampleRequest {
			DisplayDim width;
			DisplayDim height;
			uint8_t hcount;
			uint8_t vcount;

			bool operator() (const SampleRequest& a, const SampleRequest& b) const {
				return a.width < b.width 
					&& a.height < b.height 
					&& a.hcount < b.hcount 
					&& a.vcount < b.vcount;
			}
		};


		static constexpr GUID ClassGuid = { 0x4d36e96e, 0xe325, 0x11ce, {0xbf, 0xc1, 0x08, 0x00, 0x2b, 0xe1, 0x03, 0x18} };

		Display();
		~Display();

		inline auto& HardwareId() const { return _hardwareId; };
		bool Update(const json& cfg);
		const Mambi::Samples& Samples(const SampleRequest& request);

		Display(Display const&) = delete;
		void operator=(Display const&) = delete;

	private:
		std::string _hardwareId;
		std::string _ledStrip;
		DisplayDim _nativeW;
		DisplayDim _nativeH;
		DisplaySample<SampleOrient::Horizontal> _sampleH;
		DisplaySample<SampleOrient::Vertical> _sampleV;

		std::map<SampleRequest, Mambi::Samples, SampleRequest> _cache;
	};
}