#pragma once
#include "stdafx.h"
#include "Serial.h"
#include "Color.h"
#include "utils.h"


namespace Mambi
{
	class LedStripManager;

	class LedStrip
	{
	public:
		friend LedStripManager;

		typedef Serial<115200> ComPort;
		typedef UINT16 IndexType;

		static constexpr BYTE Header[] = {'M', 'A', 'M', 'B', 'I'};

		enum Command 
		{
			CMDOff = 0x01,
			CMDUpdate = 0x02,
			CMDBrightness = 0x03
		};

		LedStrip();
		~LedStrip();


		inline IndexType HCount() const { return _countH; }
		inline IndexType VCount() const { return _countV; }
		inline IndexType Count() const { return _count; };
		bool Update(const json& cfg);
		void Light(const rgb_t* info, DWORD size) const;
		void Off() const;
		void SetBrightness(uint8_t brightness) const;

		LedStrip(LedStrip const&) = delete;
		void operator=(LedStrip const&) = delete;

	private:
		DWORD CreateMessage(Command cmd, BYTE* payload, uint8_t payloadSize);
		bool Write(DWORD size) const;

		ComPort* _port;
		IndexType _countH;
		IndexType _countV;
		IndexType _count;
		Buffer<BYTE> _message;
	};	
}

