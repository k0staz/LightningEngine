#include "Log.h"

#include <spdlog/spdlog.h>
#include "spdlog/sinks/msvc_sink.h"

std::shared_ptr<spdlog::logger> Log::Logger;

void Log::Initialize()
{
	auto sink = std::make_shared<spdlog::sinks::msvc_sink_mt>();
	Logger = std::make_shared<spdlog::logger>("LE", sink);
	Logger->set_pattern("%^[%T] [%n] [%l]: %v%$");
}

void Log::InternalError(std::string_view String, std::format_args Args)
{
	Logger->error(std::vformat(String, Args));
}

void Log::InternalWarn(std::string_view String, std::format_args Args)
{
	Logger->warn(std::vformat(String, Args));
}

void Log::InternalInfo(std::string_view String, std::format_args Args)
{
	Logger->info(std::vformat(String, Args));
}

void Log::InternalTrace(std::string_view String, std::format_args Args)
{
	Logger->trace(std::vformat(String, Args));
}