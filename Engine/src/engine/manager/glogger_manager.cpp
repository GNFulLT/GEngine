#include "internal/engine/manager/glogger_manager.h"

#include <spdlog/spdlog.h>
#include <spdlog/fmt/fmt.h>

LOG_LEVEL spd_to_glog(spdlog::level::level_enum level)
{
	return (LOG_LEVEL)level;
}

spdlog::level::level_enum glog_to_spd(LOG_LEVEL level)
{
	return (spdlog::level::level_enum)level;
}

GLoggerManager::GLoggerManager()
{
	spdlog::set_level(spdlog::level::warn);
#ifdef _DEBUG
	spdlog::set_level(spdlog::level::trace);
#endif
}

GLoggerManager::~GLoggerManager()
{
}

void GLoggerManager::set_log_level(LOG_LEVEL level)
{
	spdlog::set_level(glog_to_spd(level));
}

LOG_LEVEL GLoggerManager::get_log_level()
{
	return spd_to_glog(spdlog::get_level());
}

bool GLoggerManager::init()
{
	return true;
}

void GLoggerManager::destroy()
{
}

void GLoggerManager::log_d(const char* tag, const char* msg)
{
	spdlog::log(spdlog::level::debug, fmt::format("[{}] {}",tag,msg));
}

void GLoggerManager::log_i(const char* tag, const char* msg)
{
	spdlog::log(spdlog::level::info, fmt::format("[{}] {}", tag, msg));
}

void GLoggerManager::log_w(const char* tag, const char* msg)
{
	spdlog::log(spdlog::level::warn, fmt::format("[{}] {}", tag, msg));
}

void GLoggerManager::log_e(const char* tag, const char* msg)
{
	spdlog::log(spdlog::level::err, fmt::format("[{}] {}", tag, msg));
}

void GLoggerManager::log_c(const char* tag, const char* msg)
{
	spdlog::log(spdlog::level::critical, fmt::format("[{}] {}", tag, msg));
}
