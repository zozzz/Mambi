#pragma once
#include "stdafx.h"
#include "Effect.h"
#include "Display.h"


namespace Mambi
{
	class Display;
	class Effect;


	enum ProfileType {
		Auto = 1,
		Any,
		Game,
		Specified
	};


	class Profile
	{
	public:		
		static std::shared_ptr<Profile> New(const std::string& title, const json& cfg);

		virtual ~Profile();

		inline auto& Type() const { return _type; }
		inline auto& Title() const { return _title; }
		virtual Mambi::Effect* Effect() const { return _effect; }
		inline auto& Priority() const { return _priority; };
		
		virtual bool Update(const json& cfg);		
		virtual bool Test(const Display* display, HWND fgWindow, const char* exe) = 0;

		Profile(Profile const&) = delete;
		void operator=(Profile const&) = delete;

	protected:
		Profile(const std::string& title, ProfileType type) : _title(title), _type(type), _effect(NULL) {};
	
		std::string _title;
		ProfileType _type;
		int _priority;		
		Mambi::Effect* _effect;
	};


	class AutoProfile: public Profile
	{
	public: 
		enum Status {
			Failed,
			Success,
			Changed
		};

		AutoProfile() : Profile("Auto", ProfileType::Auto), _selected(NULL) { Console::WriteLine("AutoProfile::AutoProfile"); }

		inline auto Selected() const { return _selected; }
		Mambi::Effect* Effect() const { return _selected->Effect(); }

		Status Detect(const Display* display);

		bool Test(const Display* display, HWND fgWindow, const char* exe) { return false; };

	private:
		std::shared_ptr<Profile> _selected;
	};


	class AnyProfile : public Profile 
	{
	public:
		AnyProfile(const std::string& title) : Profile(title, ProfileType::Any) {}	
		bool Test(const Display* display, HWND fgWindow, const char* exe) { return true; };
	};


	class GameProfile : public Profile
	{
	public:
		GameProfile(const std::string& title) : Profile(title, ProfileType::Any) {}	
		bool Test(const Display* display, HWND fgWindow, const char* exe);
	};


	class SpecifiedProfile : public Profile
	{
	public:
		SpecifiedProfile(const std::string& title, const std::string& exe) : Profile(title, ProfileType::Any), _exe(exe) {}	
		bool Test(const Display* display, HWND fgWindow, const char* exe);

	private:
		std::string _exe;
	};

}