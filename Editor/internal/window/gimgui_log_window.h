#ifndef GIMGUI_LOG_WINDOW_H
#define GIMGUI_LOG_WINDOW_H

#include "editor/igimgui_window_impl.h"
#include <string>

#include "engine/io/iowning_glogger.h"
#include <vector>
#include <spdlog/sinks/base_sink.h>

struct Log
{
	LOG_LEVEL level;
	std::string logString;

	Log(LOG_LEVEL level,const char* logMessage);
};

typedef std::vector<Log> Lines;

class GImGuiLogWindow : public IGImGuiWindowImpl
{
public:
	GImGuiLogWindow();



	// Inherited via IGImGuiWindowImpl
	virtual bool init() override;
	virtual void set_storage(GImGuiWindowStorage* storage) override;
	virtual bool need_render() override;
	virtual void render() override;
	virtual void on_resize() override;
	virtual void on_data_update() override;
	virtual void destroy() override;
	virtual const char* get_window_name() override;


	spdlog::sinks::base_sink<std::mutex>* create_sink();

	void add_log(const Log& log);
private:
	std::string m_name;
	GImGuiWindowStorage* m_storage;
	Lines m_lines;
};

#endif // GIMGUI_LOG_WINDOW_H