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
		size_t msgSize = const_cast<LedStrip*>(this)->CreateMessage(Command::CMDUpdate, (BYTE*)info, sizeof(rgb_t) * size);
		Write(msgSize);
	}

	void LedStrip::Transition(uint16_t duration, const rgb_t* info, DWORD size) const
	{
		size_t msgSize = const_cast<LedStrip*>(this)->CreateMessage(
			Command::CMDTransition, 
			(BYTE*)&duration, sizeof(uint16_t),
			(BYTE*)info, sizeof(rgb_t) * size);
		Write(msgSize);
	}


	void LedStrip::Off() const
	{
		size_t msgSize = const_cast<LedStrip*>(this)->CreateMessage(Command::CMDOff, NULL, 0);
		Write(msgSize);
	}


	void LedStrip::SetBrightness(uint8_t brightness) const
	{
		size_t msgSize = const_cast<LedStrip*>(this)->CreateMessage(Command::CMDBrightness, &brightness, 1);
		Write(msgSize);
	}


	size_t LedStrip::CreateMessage(Command cmd, BYTE* payload, size_t payloadSize)
	{
		return CreateMessage(cmd, NULL, 0, payload, payloadSize);
	}


	size_t LedStrip::CreateMessage(Command cmd, BYTE* params, size_t paramsSize, BYTE* payload, size_t payloadSize)
	{
		assert(payloadSize + paramsSize <= 255);

		size_t size = ARRAYSIZE(Header) + 2 + payloadSize + paramsSize;		
		if (!_message.EnsureSize(size))
		{
			return 0;
		}

		if (!_message.Append(0, (BYTE*)Header, ARRAYSIZE(Header)))
		{
			return 0;
		}
		
#pragma warning( push )
#pragma warning( disable : 4267)
		_message[ARRAYSIZE(Header)] = payloadSize + paramsSize;
#pragma warning( pop ) 
		_message[ARRAYSIZE(Header) + 1] = cmd;

		if (paramsSize > 0)
		{
			if (!_message.Append(ARRAYSIZE(Header) + 2, params, paramsSize))
			{
				return 0;
			}
		}

		if (payloadSize > 0)
		{
			if (!_message.Append(ARRAYSIZE(Header) + 2 + paramsSize, payload, payloadSize))
			{
				return 0;
			}
		}

		return size;
	}

	bool LedStrip::Write(size_t size) const
	{
		if (size != 0)
		{
#pragma warning( push )
#pragma warning( disable : 4267)
			return _port->Write(_message, size) == size;
#pragma warning( pop ) 
		}
		return false;
	}
}
