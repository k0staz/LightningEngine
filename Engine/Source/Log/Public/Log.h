#pragma once

#include <memory>
#include <format>
#include <string_view>

namespace spdlog
{
	class logger;
}

class Log
{
public:
	static void Initialize();

	template<typename... Args>
	static void Error(std::string_view String, Args&&... Arguments)
	{
		InternalError(String, std::make_format_args(Arguments...));
	}

	template<typename... Args>
	static void Warn(std::string_view String, Args&&... Arguments)
	{
		InternalWarn(String, std::make_format_args(Arguments...));
	}

	template<typename... Args>
	static void Info(std::string_view String, Args&&... Arguments)
	{
		InternalInfo(String, std::make_format_args(Arguments...));
	}

	template<typename... Args>
	static void Trace(std::string_view String, Args&&... Arguments)
	{
		InternalTrace(String, std::make_format_args(Arguments...));
	}

private:
	static void InternalError(std::string_view String, std::format_args Args);
	static void InternalWarn(std::string_view String, std::format_args Args);
	static void InternalInfo(std::string_view String, std::format_args Args);
	static void InternalTrace(std::string_view String, std::format_args Args);

private:
	static std::shared_ptr<spdlog::logger> Logger;
};

#define LE_ERROR(...) Log::Error(__VA_ARGS__)
#define LE_WARN(...)  Log::Warn(__VA_ARGS__)
#define LE_INFO(...)  Log::Info(__VA_ARGS__)
#define LE_TRACE(...) Log::Trace(__VA_ARGS__)
