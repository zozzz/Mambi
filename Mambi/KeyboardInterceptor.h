#pragma once

namespace Mambi 
{

	class KeyboardInterceptor
	{
	public:
		KeyboardInterceptor();
		~KeyboardInterceptor();


		KeyboardInterceptor(KeyboardInterceptor const&) = delete;
		void operator=(KeyboardInterceptor const&) = delete;

	private:
		static LRESULT CALLBACK LowLevelKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
		LRESULT CALLBACK HandleKeypress(int nCode, WPARAM wParam, LPARAM lParam);

		HHOOK hook;
	};

}