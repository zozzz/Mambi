#pragma once
#include "stdafx.h"
#include "Profile.h"


namespace Mambi
{

	class ProfileManager
	{
	public:
		ProfileManager();
		~ProfileManager();

		inline auto& Active() const { return *_active; }
		inline auto& Profiles() const { return _profiles; }
		inline bool IsAutoDetect() const { return _isAutoDetect; }
		void Update();
		void Activate(int idx);
		// void Watch();


		ProfileManager(ProfileManager const&) = delete;
		void operator=(ProfileManager const&) = delete;
	private:
		static DWORD WINAPI AutoDetectThread(LPVOID lpParam);

		void DoUpdate();
		void AutoDetect();
		bool SetActive(Profile* profile);
		void StartAutoDetect();
		void StopAutoDetect();

		std::map<std::string, Profile> _profiles;
		Profile* _active;
		bool _isAutoDetect;		
		HANDLE _mutex;
		HANDLE _hAutoDetect;		
	};

}