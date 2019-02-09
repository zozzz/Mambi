#pragma once
#include "stdafx.h"
#include "Effect.h"


namespace Mambi
{

	class Profile
	{
	public:
		enum FGDetection {
			Any,
			Game,
			Specified
		};

		Profile();
		~Profile();

		inline auto& Title() const { return _title; }
		inline auto& Effect() const { return _effect; }
		inline int Priority() const { return _priority; };
		bool Update(const std::string& title, const json& cfg);
		bool Test(HWND fgWindow);


		Profile(Profile const&) = delete;
		void operator=(Profile const&) = delete;
	private:
		bool DetectGame(HWND fgWindow);
		bool DetectSpecified(HWND fgWindow);

		std::string _title;
		std::string _exe;
		int _priority;
		FGDetection _detection;
		Mambi::Effect* _effect;
	};

}