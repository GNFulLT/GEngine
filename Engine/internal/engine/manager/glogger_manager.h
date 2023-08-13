#ifndef GLOGGER_MANAGER_H
#define GLOGGER_MANAGER_H

#include "engine/manager/iglogger_manager.h"

class GLoggerManager : public IGLoggerManager
{
public:
	static GLoggerManager* get_instance();

	static void set_instance(GLoggerManager* manager);

	GLoggerManager();
	~GLoggerManager();
	// Inherited via IGLoggerManager
	virtual void set_log_level(LOG_LEVEL level) override;
	virtual LOG_LEVEL get_log_level() override;
	virtual bool init() override;
	virtual void destroy() override;
	virtual void log_d(const char* tag, const char* msg) override;
	virtual void log_i(const char* tag, const char* msg) override;
	virtual void log_w(const char* tag, const char* msg) override;
	virtual void log_e(const char* tag, const char* msg) override;
	virtual void log_c(const char* tag, const char* msg) override;


	void enable_file_logging(const char* fileName,LOG_LEVEL level);

	virtual GSharedPtr<IOwningGLogger> create_owning_glogger(const char* ownerName) override;
private:
	inline static GLoggerManager* s_instance = nullptr;;
};

#endif // GLOGGER_MANAGER_H