#ifndef IOWNING_GLOGGER_H
#define IOWNING_GLOGGER_H

#include "engine/GEngine_EXPORT.h"
#include <spdlog/sinks/base_sink-inl.h>


enum LOG_LEVEL
{
	LOG_LEVEL_TRACE = 0,
	LOG_LEVEL_DEBUG,
	LOG_LEVEL_INFO,
	LOG_LEVEL_WARNING,
	LOG_LEVEL_ERROR,
	LOG_LEVEL_CRITICAL,
	LOG_LEVEL_OFF,
};

#include <mutex>


class ENGINE_API IOwningGLogger
{
public:
	virtual ~IOwningGLogger() = default;
	virtual void set_log_level(LOG_LEVEL level) = 0;

	virtual LOG_LEVEL get_log_level() = 0;

	virtual void log_d(const char* msg) = 0;
	virtual void log_i(const char* msg) = 0;
	virtual void log_w(const char* msg) = 0;
	virtual void log_e(const char* msg) = 0;
	virtual void log_c(const char* msg) = 0;

	virtual const char* get_owner_name() = 0;

	virtual void add_sink(spdlog::sinks::sink* sink) = 0;

private:
};

#endif // OWNING_IGLOGGER_H