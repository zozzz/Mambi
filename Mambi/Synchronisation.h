#pragma once
#include "Messages.h"


#define MAMBI_MLOCK_BEGIN(__m, __t) \
	switch (WaitForSingleObject(__m, __t)) { \
		case WAIT_OBJECT_0: \
			__try { 

#define __MAMBI_MLCOK_END(__m) \
			} \
			__finally { \
				if (!ReleaseMutex(__m)) { ErrorAlert("Error", "Mutex release failed"); } \
			} \
		break; \
		case WAIT_ABANDONED:

#define MAMBI_MLCOK_END(__m, __failed) \
	__MAMBI_MLCOK_END(__m) return __failed; }

#define MAMBI_MLCOK_END_VOID(__m) \
	__MAMBI_MLCOK_END(__m) return; }
	