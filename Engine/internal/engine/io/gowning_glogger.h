#ifndef GOWNING_GLOGGER_H
#define GOWNING_GLOGGER_H

#include "engine/io/iowning_glogger.h"
#include <string>
#include <spdlog/spdlog.h>
#include <memory>
class GOwningGLogger : public IOwningGLogger
{
public:
	GOwningGLogger(const char* ownerName);

	// Inherited via IOwningGlogger
	virtual void set_log_level(LOG_LEVEL level) override;
	virtual LOG_LEVEL get_log_level() override;
	virtual void log_d(const char* msg) override;
	virtual void log_i(const char* msg) override;
	virtual void log_w(const char* msg) override;
	virtual void log_e(const char* msg) override;
	virtual void log_c(const char* msg) override;
	virtual const char* get_owner_name() override;
private:
	std::string m_ownerName;
	std::shared_ptr<spdlog::logger> m_logger;

};

#endif // GOWNING_GLOGGER_H