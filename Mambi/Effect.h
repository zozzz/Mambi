#pragma once
#include "stdafx.h"
#include "Color.h"

// https://github.com/MrBoe/bambilight

namespace Mambi
{
	
	class Effect
	{
	public:
		static Effect* New(const json& cfg);
				
		Effect(const char* type);
		virtual ~Effect();

		virtual bool Update(const json& cfg) = 0;
		virtual void Tick() = 0;

		virtual bool Equals(const Effect& other) const = 0;
		inline bool operator==(const Effect& other) const 
		{
			return other._type == _type && Equals(other);
		}
		inline bool operator!=(const Effect& other) const
		{
			return other._type != _type || !Equals(other);
		}
		

		inline auto& Type() const { return _type; }

	protected:
		std::string _type;
	};


	class Effect_Static : public Effect {
	public:		
		using Effect::Effect;

		bool Update(const json& cfg);
		void Tick();
		inline bool Equals(const Effect& other) const
		{
			return static_cast<const Effect_Static&>(other)._color == _color;
		}

	protected:
		
		color_t _color;
	};



	class Effect_Ambilight : public Effect {
	public:
		struct Spot {
			enum Orient {
				Horizontal = 1,
				Vertical = 2,
			};

			Orient orient;
			uint8_t count;
			uint16_t margin;
			uint16_t width;
			uint16_t height;
		};

		using Effect::Effect;

		bool Update(const json& cfg);
		void Tick();
		inline bool Equals(const Effect& other) const
		{
			return false;
		}

	protected:
		Spot hspot;
		Spot vspot;		
	};


	class Effect_Breath : public Effect {
	public:
		using Effect::Effect;

		bool Update(const json& cfg);
		void Tick();
		inline bool Equals(const Effect& other) const
		{
			return false;
		}

	protected:
		unsigned int duration;
		uint8_t max;
		uint8_t min;
		
	};



	/*
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
	*/
}