#include "Log/Log.h"

#include "spdlog/sinks/msvc_sink.h"

std::shared_ptr<spdlog::logger> Log::Logger;

void Log::Initialize()
{
	auto sink = std::make_shared<spdlog::sinks::msvc_sink_mt>();
	Logger = std::make_shared<spdlog::logger>("LE", sink);
	Logger->set_pattern("%^[%T] [%n] [%l]: %v%$");
}