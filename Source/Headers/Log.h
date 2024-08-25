#pragma once
#include <spdlog/spdlog.h>
#include <memory>
#include <spdlog/fmt/ostr.h>
namespace Renderer
{
	class Log
	{
	public:
		static void Init();
		inline static std::shared_ptr<spdlog::logger>& GetCoreLogger() {
			return s_coreLogger;
		}
		inline static std::shared_ptr<spdlog::logger>& GetClientLogger()
		{
			return s_clientLogger;
		}

	private:
		static std::shared_ptr<spdlog::logger> s_coreLogger;
		static std::shared_ptr<spdlog::logger> s_clientLogger;
	};
}
//Core logs
#define LOG_CORE_FATAL(...)   ::Renderer::Log::GetCoreLogger()->fatal(__VA_ARGS__)
#define LOG_CORE_ERROR(...)  ::Renderer::Log::GetCoreLogger()->error(__VA_ARGS__)
#define LOG_CORE_WARN(...)  ::Renderer::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define LOG_CORE_INFO(...)     ::Renderer::Log::GetCoreLogger()->info(__VA_ARGS__)
#define LOG_CORE_TRACE(...)  ::Renderer::Log::GetCoreLogger()->trace(__VA_ARGS__)

#define LOG_CLIENT_FATAL(...)   ::Renderer::Log::GetClientLogger()->fatal(__VA_ARGS__)
#define LOG_CLIENT_ERROR(...)  ::Renderer::Log::GetClientLogger()->error(__VA_ARGS__)
#define LOG_CLIENT_WARN(...)  ::Renderer::Log::GetClientLogger()->warn(__VA_ARGS__)
#define LOG_CLIENT_INFO(...)     ::Renderer::Log::GetClientLogger()->info(__VA_ARGS__)
#define LOG_CLIENT_TRACE(...)  ::Renderer::Log::GetClientLogger()->trace(__VA_ARGS__)