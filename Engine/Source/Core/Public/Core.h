#pragma once

#include "Log.h"

#if PLATFORM_WINDOWS
#define LE_DEBUG_BREAK() __debugbreak()
#endif

#define LE_ASSERT(expr, ...)                            \
	if(!(expr))											\
	{													\
		LE_ERROR("Assertion failed: {}", __VA_ARGS__);	\
		LE_DEBUG_BREAK();								\
	}													