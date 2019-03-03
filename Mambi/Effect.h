#pragma once
#include "stdafx.h"
#include "Color.h"
#include "Display.h"

// https://github.com/MrBoe/bambilight

namespace Mambi
{
	class Display;
	

	class Effect
	{
	public:
		static Effect* New(const json& cfg);
				
		Effect(const char* type);
		virtual ~Effect();

		virtual bool Update(const json& cfg) = 0;
		virtual void Init(Display* display) = 0;
		virtual void Tick(Display* display) = 0;
		
		virtual bool Equals(const Effect& other) const = 0;
		inline bool operator==(const Effect& other) const 
		{
			return other._type == _type && interval == other.interval && Equals(other);
		}
		inline bool operator!=(const Effect& other) const
		{
			return other._type != _type || interval != other.interval || !Equals(other);
		}
		

		inline auto& Type() const { return _type; }

		UINT16 interval;

	protected:
		std::string _type;
	};


	class Effect_Static : public Effect {
	public:		
		using Effect::Effect;

		bool Update(const json& cfg);
		void Init(Display* display);
		void Tick(Display* display);
		inline bool Equals(const Effect& other) const
		{
			return static_cast<const Effect_Static&>(other).color == color 
				&& static_cast<const Effect_Static&>(other).brightness == brightness;
		}

	protected:		
		rgb_t color;
		uint8_t brightness;
	};



	class Effect_Ambilight : public Effect {
	public:
		using Effect::Effect;

		bool Update(const json& cfg);
		void Init(Display* display);
		void Tick(Display* display);
		inline bool Equals(const Effect& other) const
		{
			return true;
		}

	protected:
		uint8_t brightness;		
	};


	class Effect_Breath : public Effect {
	public:
		using Effect::Effect;

		bool Update(const json& cfg);
		void Init(Display* display);
		void Tick(Display* display);
		inline bool Equals(const Effect& other) const
		{
			return duration == static_cast<const Effect_Breath&>(other).duration 
				&& maxBrightness == static_cast<const Effect_Breath&>(other).maxBrightness 
				&& minBrightness == static_cast<const Effect_Breath&>(other).minBrightness;
		}

	protected:
		unsigned int duration;
		uint8_t maxBrightness;
		uint8_t minBrightness;
		rgb_t color;
	
	private:
		UINT16 _currentFrame;
		UINT16 _frameCount;
		float _stepBrightness;
	};


	class Effect_Rainbow : public Effect {
	public:
		using Effect::Effect;

		bool Update(const json& cfg);
		void Init(Display* display);
		void Tick(Display* display);
		inline bool Equals(const Effect& other) const
		{
			// TODO: ...
			return duration == static_cast<const Effect_Rainbow&>(other).duration
				&& brightness == static_cast<const Effect_Rainbow&>(other).brightness;
		}

	protected:
		unsigned int duration;
		uint8_t brightness;

	private:
		struct Keyframe 
		{
			float position;
			rgb_t color;
		};
		typedef std::vector<Keyframe> Keyframes;

		Keyframes _frames;
		uint8_t _currentFrame;
	};
}