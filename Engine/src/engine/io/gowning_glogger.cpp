#include "internal/engine/io/gowning_glogger.h"
#include "internal/engine/io/logger_helper.h"
#include <spdlog/sinks/stdout_color_sinks.h>
GOwningGLogger::GOwningGLogger(const char* ownerName)
{
	m_ownerName = ownerName;
	auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();

	auto logger = new spdlog::logger(ownerName, { console_sink });
	m_logger = std::shared_ptr<spdlog::logger>(logger);

}

void GOwningGLogger::set_log_level(LOG_LEVEL level)
{
	m_logger->set_level(glog_to_spd(level));
}

LOG_LEVEL GOwningGLogger::get_log_level()
{
	return spd_to_glog(m_logger->level());
}
void GOwningGLogger::log_d(const char* tag, const char* msg)
{
	m_logger->log(spdlog::level::debug, fmt::format("[{}] {}", tag, msg));
}

void GOwningGLogger::log_i(const char* tag, const char* msg)
{
	m_logger->log(spdlog::level::info, fmt::format("[{}] {}", tag, msg));
}

void GOwningGLogger::log_w(const char* tag, const char* msg)
{
	m_logger->log(spdlog::level::warn, fmt::format("[{}] {}", tag, msg));
}

void GOwningGLogger::log_e(const char* tag, const char* msg)
{
	m_logger->log(spdlog::level::err, fmt::format("[{}] {}", tag, msg));
}

void GOwningGLogger::log_c(const char* tag, const char* msg)
{
	m_logger->log(spdlog::level::critical, fmt::format("[{}] {}", tag, msg));
}
const char* GOwningGLogger::get_owner_name()
{
	return m_ownerName.c_str();
}
