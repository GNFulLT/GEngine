#ifndef IGLOGGER_MANAGER_H
#define IGLOGGER_MANAGER_H

#include "engine/GEngine_EXPORT.h"

enum LOG_LEVEL
{
	LOG_LEVEL_TRACE=0,
	LOG_LEVEL_DEBUG,
	LOG_LEVEL_INFO,
	LOG_LEVEL_WARNING,
	LOG_LEVEL_ERROR,
	LOG_LEVEL_CRITICAL,
	LOG_LEVEL_OFF,
};

class ENGINE_API IGLoggerManager
{
public:
	virtual ~IGLoggerManager() = default;

	virtual void set_log_level(LOG_LEVEL level) = 0;

	virtual LOG_LEVEL get_log_level() = 0;

	virtual bool init() = 0;

	virtual void destroy() = 0;

	virtual void log_d(const char* tag,const char* msg) = 0;
	virtual void log_i(const char* tag, const char* msg) = 0;
	virtual void log_w(const char* tag, const char* msg) = 0;
	virtual void log_e(const char* tag, const char* msg) = 0;
	virtual void log_c(const char* tag, const char* msg) = 0;

private:
};

#endif // IGLOGGER_MANAGER_H