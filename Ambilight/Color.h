#pragma once
#include "stdafx.h"


namespace Mambi
{

#pragma pack(push, 1)
	struct rgba_t;
	struct bgra_t;
	struct rgb_t;

	struct rgba_t {
		uint8_t r;
		uint8_t g;
		uint8_t b;
		uint8_t a;
	};


	struct bgra_t {
		uint8_t b;
		uint8_t g;
		uint8_t r;
		uint8_t a;
	};


	struct rgb_t {
		uint8_t r;
		uint8_t g;
		uint8_t b;
	};
#pragma pack(pop)

	inline bool operator==(const rgb_t& a, const rgb_t& b) { return a.r == b.r && a.g == b.g && a.b == b.b; }
	inline bool operator!=(const rgb_t& a, const rgb_t& b) { return a.r != b.r || a.g != b.g || a.b != b.b; }
	inline bool operator==(const rgba_t& a, const rgba_t& b) { return a.r == b.r && a.g == b.g && a.b == b.b && a.a == b.a; }
	inline bool operator!=(const rgba_t& a, const rgba_t& b) { return a.r != b.r || a.g != b.g || a.b != b.b || a.a != b.a; }
	inline bool operator==(const bgra_t& a, const bgra_t& b) { return a.r == b.r && a.g == b.g && a.b == b.b && a.a == b.a; }
	inline bool operator!=(const bgra_t& a, const bgra_t& b) { return a.r != b.r || a.g != b.g || a.b != b.b || a.a != b.a; }


	namespace Color
	{
		inline bool RedFromHex(rgb_t& into, const std::string& hex)
		{
			if (hex.size() != 6)
			{
				return false;
			}

#pragma warning( push )
#pragma warning( disable : 4477)
			unsigned int r, g, b;
			unsigned int c = sscanf_s(hex.c_str(), "%02x%02x%02x", &r, &g, &b);
			if (c == 3)
			{
				into.r = r;
				into.g = g;
				into.b = b;
				return true;
			}
			return false;
#pragma warning( pop )
		}


		inline bool RedFromHex(rgba_t& into, const std::string& hex)
		{
			if (hex.size() != 6)
			{
				return false;
			}

			into.a = 255;
#pragma warning( push )
#pragma warning( disable : 4477)
			unsigned int r, g, b;
			unsigned int c = sscanf_s(hex.c_str(), "%02x%02x%02x", &r, &g, &b);
			if (c == 3)
			{
				into.r = r;
				into.g = g;
				into.b = b;
				return true;
			}
			return false;
#pragma warning( pop )
		}

		inline rgb_t* RemoveAlpha(const rgba_t* data, size_t size)
		{
			rgb_t* result = (rgb_t*)malloc(sizeof(rgb_t) * size);
			if (result != NULL)
			{
				for (size_t i = 0; i < size; i++)
				{
					result[i].r = data[i].r;
					result[i].g = data[i].g;
					result[i].b = data[i].b;
				}
			}
			return result;
		}
	}
}
