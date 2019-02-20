#pragma once
#include "stdafx.h"


namespace Mambi
{

	/*
	Timeline<2> tl({20, 500});
	if (tl.Is(1))
	{
	}

	if (tl.Is(2)) 
	{
	}
	tl.Tick();
	Sleep(tl.Wait());
	
	*/

	template<int K>
	class Timeline
	{
	public:
		Timeline(int[K] keyframe) : _keyframe(keyframe) {}
		~Timeline() {}

		inline void Tick()
		{
			_current
		}

		inline int Wait()
		{
		}

	private:
		int _keyframe[K];
		std::chrono::time_point _begin;
		std::chrono::time_point _current;
	};

}
