#pragma once

#include <memory>

#include "spdlog/spdlog.h"

class Log
{
public:
	static void Initialize();

	inline static std::shared_ptr<spdlog::logger>& GetLogger() { return Logger; }
private:
	static std::shared_ptr<spdlog::logger> Logger;
};

#define LE_ERROR(...) Log::GetLogger()->error(__VA_ARGS__)
#define LE_WARN(...)  Log::GetLogger()->warn(__VA_ARGS__)
#define LE_INFO(...)  Log::GetLogger()->info(__VA_ARGS__)
#define LE_TRACE(...) Log::GetLogger()->trace(__VA_ARGS__)
