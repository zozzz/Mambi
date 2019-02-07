#pragma once
#include "stdafx.h"

// https://github.com/MrBoe/bambilight

namespace Mambi
{
	struct Color {
		static Color FromHex(const std::string& hex) {
			Color ret;
			ReadHexInto(hex, ret);
			return ret;
		};


		static bool ReadHexInto(const std::string& hex, Color& color)
		{
			return sscanf_s(hex.c_str(), "%02x%02x%02x", &color.r, &color.g, &color.b) == 3;
		}


		uint8_t r, g, b;
	};


	class Effect
	{
	public:
		static Effect* New(const json& cfg);

		Effect(const char* type);
		virtual ~Effect();

		virtual void Update(const json& cfg);
		virtual void Tick() = 0;

		inline auto& Type() const { return _type; }

	protected:
		std::string _type;
	};


	class Effect_Static : public Effect {
	public:
		using Effect::Effect;

		virtual void Update(const json& cfg);
		virtual void Tick();

	protected:
		Color _color;
	};


	class Effect_Ambilight : public Effect {
	public:
		using Effect::Effect;

		virtual void Update(const json& cfg);
		virtual void Tick();
	};


	class Effect_Breath : public Effect {
	public:
		using Effect::Effect;

		virtual void Update(const json& cfg);
		virtual void Tick();
	};
}