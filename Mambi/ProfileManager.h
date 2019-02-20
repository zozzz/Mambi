#pragma once
#include "stdafx.h"
#include "Profile.h"


namespace Mambi
{

	class ProfileManager
	{
	public:
		ProfileManager() {};
		~ProfileManager() {};

		inline auto& Profiles() const { return _profiles; }
		
		bool Update();
		bool Activate(const Display* display, std::shared_ptr<Profile> profile);
		bool Activate(const Display* display, const nullptr_t& profile);
		bool Activate(const std::string& display, const std::string& profile);
		bool Activate(const std::string& display, const nullptr_t& profile);
		
		ProfileManager(ProfileManager const&) = delete;
		void operator=(ProfileManager const&) = delete;
	private:

		void ReplaceProfile(std::shared_ptr<Profile> current, std::shared_ptr<Profile> replcae);

		//static DWORD WINAPI AutoDetectThread(LPVOID lpParam);

		//void DoUpdate();
		//void AutoDetect();
		//bool SetActive(Profile* profile);
		//void StartAutoDetect();
		//void StopAutoDetect();

		std::map<std::string, std::shared_ptr<Profile>> _profiles;
	};

}