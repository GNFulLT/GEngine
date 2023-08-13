#include "internal/engine/manager/glogger_manager.h"

#include <spdlog/spdlog.h>
#include <spdlog/fmt/fmt.h>
#include <spdlog/sinks/basic_file_sink.h>
#include "internal/engine/io/logger_helper.h"
#include "internal/engine/io/gowning_glogger.h"
#include "gobject/gobject_utils.h"

GOBJECT_ENABLE(GLoggerManager)
GOBJECT_DEFINE_MEMBER_METHOD("log_d", &GLoggerManager::log_d);
GOBJECT_DEFINE_MEMBER_METHOD("log_c", &GLoggerManager::log_c);
GOBJECT_DEFINE_MEMBER_METHOD("log_i", &GLoggerManager::log_i);
GOBJECT_DEFINE_MEMBER_METHOD("log_e", &GLoggerManager::log_e);
GOBJECT_DEFINE_MEMBER_METHOD("log_w", &GLoggerManager::log_w);

}


GLoggerManager* GLoggerManager::get_instance()
{
	return s_instance;
}

void GLoggerManager::set_instance(GLoggerManager* manager)
{
	s_instance = manager;
}

GLoggerManager::GLoggerManager()
{
	spdlog::set_level(spdlog::level::warn);
#ifdef _DEBUG
	spdlog::set_level(spdlog::level::trace);
#endif
}

void GLoggerManager::enable_file_logging(const char* fileName, LOG_LEVEL level)
{
	auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(fileName, true);
	file_sink->set_level(glog_to_spd(level));

	spdlog::default_logger()->sinks().push_back(file_sink);
}

GSharedPtr<IOwningGLogger> GLoggerManager::create_owning_glogger(const char* ownerName)
{
	//X TODO DONT USE NEW GDNEWDA+
	return GSharedPtr<IOwningGLogger>(new GOwningGLogger(ownerName));
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
