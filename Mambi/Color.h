#pragma once
#include "stdafx.h"
#include "Console.h"


namespace Mambi 
{
	struct abgr {
		uint8_t A;
		uint8_t B;
		uint8_t G;
		uint8_t R;
	};

	union color_t
	{
		uint32_t Value;
		abgr Parts;
	};

	inline bool operator==(const color_t& a, const color_t& b) { return a.Value == b.Value; }
	inline bool operator!=(const color_t& a, const color_t& b) { return a.Value != b.Value; }


	namespace Color 
	{
		inline bool RedFromHex(color_t& into, const std::string& hex)
		{
			if (hex.size() != 6)
			{
				return false;
			}

			into.Parts.A = 255;
#pragma warning( push )
#pragma warning( disable : 4477)
			unsigned int r, g, b;
			unsigned int c = sscanf_s(hex.c_str(), "%02x%02x%02x", &r, &g, &b);
			if (c == 3) 
			{
				into.Parts.R = r;
				into.Parts.G = g;
				into.Parts.B = b;
				return true;
			}
			return false;			
#pragma warning( pop )
		}
	}
}
