#include "stdafx.h"
#include "LedStrip.h"
#include "Config.h"
#include "Console.h"


namespace Mambi
{

	LedStrip::LedStrip(): _countH(0), _countV(0), _port(NULL)
	{
	}


	LedStrip::~LedStrip()
	{
		if (_port != NULL)
		{
			delete _port;
			_port = NULL;
		}
	}


	bool LedStrip::Update(const json& cfg)
	{
		MAMBI_CFG_IS_STRING(cfg, "port", "ledStrip.*");
		MAMBI_CFG_IS_OBJECT(cfg, "leds", "ledStrip.*");
		auto& leds = cfg["leds"];
		MAMBI_CFG_VNUM_INT_RANGE(leds, "horizontal", "ledStrip.*", 0, 255);
		MAMBI_CFG_VNUM_INT_RANGE(leds, "vertical", "ledStrip.*", 0, 255);

		_countH = leds["horizontal"];
		_countV = leds["vertical"];

		if (_port == NULL || _port->Name() != cfg["port"].get<std::string>())
		{
			ComPort* port = new ComPort(cfg["port"].get<std::string>());
			ComPort* tmp = _port;
			_port = port;
			if (tmp != NULL)
			{
				delete tmp;
			}
		}

		_count = _countH * 2 + _countV * 2;

		Console::WriteLine("leds (%d x %d)", _countH, _countV);

		return true;
	}


	void LedStrip::Light(const rgb_t* info, DWORD size) const
	{
		DWORD msgSize = const_cast<LedStrip*>(this)->CreateMessage(Command::CMDUpdate, (BYTE*)info, sizeof(rgb_t) * size);
		Write(msgSize);
	}


	void LedStrip::Off() const
	{
		DWORD msgSize = const_cast<LedStrip*>(this)->CreateMessage(Command::CMDOff, NULL, 0);
		Write(msgSize);
	}


	void LedStrip::SetBrightness(uint8_t brightness) const
	{
		DWORD msgSize = const_cast<LedStrip*>(this)->CreateMessage(Command::CMDBrightness, &brightness, 1);
		Write(msgSize);
	}


	DWORD LedStrip::CreateMessage(Command cmd, BYTE* payload, uint8_t payloadSize)
	{
		DWORD size = ARRAYSIZE(Header) + 2 + payloadSize;
		if (!_message.EnsureSize(size))
		{
			return 0;
		}

		if (!_message.Append(0, (BYTE*)Header, ARRAYSIZE(Header)))
		{
			return 0;
		}
		
		_message[ARRAYSIZE(Header)] = payloadSize;
		_message[ARRAYSIZE(Header) + 1] = cmd;

		if (payloadSize > 0)
		{
			if (!_message.Append(ARRAYSIZE(Header) + 2, payload, payloadSize))
			{
				return 0;
			}
		}

		return size;
	}

	bool LedStrip::Write(DWORD size) const
	{
		if (size != 0)
		{
			return _port->Write(_message, size) == size;
		}
		return false;
	}
}
