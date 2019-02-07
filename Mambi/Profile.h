#pragma once


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
		inline int Priority() const { return _priority; };
		void Update(const std::string& title, const json& cfg);
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
	};

}