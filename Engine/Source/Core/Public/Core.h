#pragma once

#include "Log.h"
#include "cstring"

#if PLATFORM_WINDOWS
#define LE_DEBUG_BREAK() __debugbreak()
#endif

#define LE_ASSERT_DESC(expr, ...)                       \
	if(!(expr))											\
	{													\
		LE_ERROR("Assertion failed: {}", __VA_ARGS__);	\
		LE_DEBUG_BREAK();								\
	}

#define LE_ASSERT(expr)	\
if (!(expr))			\
{						\
LE_DEBUG_BREAK();		\
}

#define MemsetZero(Dest, Size) std::memset(Dest, 0, Size);
