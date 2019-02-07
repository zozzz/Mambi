#pragma once
#include <stdio.h>
#include "stdafx.h"


namespace Mambi {
	namespace Console {
		inline int __cdecl WriteLine(const char *format, ...) {
			char str[1024];

			va_list argptr;
			va_start(argptr, format);
			int ret = vsnprintf(str, sizeof(str), format, argptr);
			va_end(argptr);

			OutputDebugStringA(str);
			OutputDebugStringA("\n");

			return ret;
		}
	}
}