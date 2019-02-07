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

	class Display
	{
	public:
		static constexpr GUID ClassGuid = { 0x4d36e96e, 0xe325, 0x11ce, {0xbf, 0xc1, 0x08, 0x00, 0x2b, 0xe1, 0x03, 0x18} };

		Display();
		~Display();

		void HardwareId(const std::string& id);
		inline auto& HardwareId() const { return _hardwareId; };


		Display(Display const&) = delete;
		void operator=(Display const&) = delete;

	private:
		std::string _hardwareId;
	};

}