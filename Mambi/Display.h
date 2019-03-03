#pragma once
#include "stdafx.h"
#include "Profile.h"
#include "LedStrip.h"
#include "utils.h"


namespace Mambi
{
	class Profile;

	class Display
	{
	public:
		//static constexpr GUID ClassGuid = { 0x4d36e96e, 0xe325, 0x11ce, {0xbf, 0xc1, 0x08, 0x00, 0x2b, 0xe1, 0x03, 0x18} };

		Display();
		~Display();

		inline auto& HardwareId() const { return _hardwareId; };
		inline auto  Profile() const { return _profile; };
		inline void  Profile(std::shared_ptr<Mambi::Profile> profile) { _profile = profile; };
		inline auto  Ambilight() const { return _ambilight; };
		inline auto& NativeWidth() const { return _nativeW; }
		inline auto& NativeHeight() const { return _nativeH; }
		inline auto& LedStrip() const { return _ledStrip; };
		inline auto& Colors() { return _colors; }
		
		bool Update(const std::string& id, const json& cfg);
		
		Display(Display const&) = delete;
		void operator=(Display const&) = delete;

	private:
		static DWORD WINAPI EffectThread(LPVOID param);

		void StartEffectThread();
		void StopEffectThread();

		std::string _hardwareId;
		std::shared_ptr<Mambi::Profile> _profile;
		std::shared_ptr<Mambi::LedStrip> _ledStrip;
		DisplayDim _nativeW;
		DisplayDim _nativeH;		
		std::shared_ptr<Mambi::Ambilight> _ambilight;

		HANDLE _hEffectThread;
		Mambi::Buffer<rgb_t> _colors;
	};
}