#include "internal/engine/io/gowning_glogger.h"
#include "internal/engine/io/logger_helper.h"
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/base_sink.h>

GOwningGLogger::GOwningGLogger(const char* ownerName,bool haveSink)
{
	m_ownerName = ownerName;
	m_haveSink = haveSink;
	if (haveSink)
	{
		auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
		auto logger = new spdlog::logger(ownerName, { console_sink });
		m_logger = std::shared_ptr<spdlog::logger>(logger);
	}
	else
	{
		auto logger = new spdlog::logger(ownerName);
		m_logger = std::shared_ptr<spdlog::logger>(logger);
	}
	

	m_logger->set_level(spdlog::level::warn);
#ifdef _DEBUG
	m_logger->set_level(spdlog::level::trace);
#endif
}

void GOwningGLogger::set_log_level(LOG_LEVEL level)
{
	m_logger->set_level(glog_to_spd(level));
}

LOG_LEVEL GOwningGLogger::get_log_level()
{
	return spd_to_glog(m_logger->level());
}
void GOwningGLogger::log_d(const char* msg)
{
	if (m_haveSink)
	{
		m_logger->log(spdlog::level::debug, fmt::format("[{}] {}", m_ownerName.c_str(), msg));
	}
	else
	{
		m_logger->log(spdlog::level::debug,msg);
	}
}

void GOwningGLogger::log_i(const char* msg)
{
	if (m_haveSink)
	{
		m_logger->log(spdlog::level::info, fmt::format("[{}] {}", m_ownerName.c_str(), msg));
	}
	else
	{
		m_logger->log(spdlog::level::info, msg);
	}
}

void GOwningGLogger::log_w(const char* msg)
{
	if (m_haveSink)
	{
		m_logger->log(spdlog::level::warn, fmt::format("[{}] {}", m_ownerName.c_str(), msg));
	}
	else
	{
		m_logger->log(spdlog::level::warn, msg);
	}
}

void GOwningGLogger::log_e(const char* msg)
{
	if (m_haveSink)
	{
		m_logger->log(spdlog::level::err, fmt::format("[{}] {}", m_ownerName.c_str(), msg));
	}
	else
	{
 		m_logger->log(spdlog::level::err, msg);
	}
}

void GOwningGLogger::log_c(const char* msg)
{
	if (m_haveSink)
	{
		m_logger->log(spdlog::level::critical, fmt::format("[{}] {}", m_ownerName.c_str(), msg));
	}
	else
	{
		m_logger->log(spdlog::level::critical, msg);
	}
}
const char* GOwningGLogger::get_owner_name()
{
	return m_ownerName.c_str();
}

void GOwningGLogger::add_sink(spdlog::sinks::sink* sink)
{
	m_logger->sinks().push_back(std::shared_ptr<spdlog::sinks::sink>(sink));
}
