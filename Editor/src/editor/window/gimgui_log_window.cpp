#include "internal/window/gimgui_log_window.h"
#include <imgui.h>

class EditorLogSink : public spdlog::sinks::base_sink <std::mutex>
{
public:
	EditorLogSink(GImGuiLogWindow* parent)
	{
		m_parent = parent;
	}
protected:
	void sink_it_(const spdlog::details::log_msg& msg) override
	{
		LOG_LEVEL level;
		switch (msg.level)
		{
		case spdlog::level::trace:
			level = LOG_LEVEL_TRACE;
				break;
		case spdlog::level::debug:
			level = LOG_LEVEL_DEBUG;
				break;
		case spdlog::level::info:
			level = LOG_LEVEL_INFO;
				break;
		case spdlog::level::warn:
			level = LOG_LEVEL_WARNING;
				break;
		case spdlog::level::critical:
			level = LOG_LEVEL_CRITICAL;
				break;
		case spdlog::level::err:
			level = LOG_LEVEL_ERROR;
				break;
		default:
			level = LOG_LEVEL_TRACE;
		}
		
		m_parent->add_log({ level,msg.payload.data()});
	}

	void flush_() override
	{

	}
private:
	GImGuiLogWindow* m_parent;

};

GImGuiLogWindow::GImGuiLogWindow()
{
	m_name = "Debug Log";
}

bool GImGuiLogWindow::init()
{
	return true;
}

void GImGuiLogWindow::set_storage(GImGuiWindowStorage* storage)
{
	m_storage = storage;
}

bool GImGuiLogWindow::need_render()
{
	return true;
}

void GImGuiLogWindow::render()
{
	for (auto log : m_lines)
	{
		if (log.level == LOG_LEVEL_CRITICAL || log.level == LOG_LEVEL_ERROR)
		{
			ImGui::TextColored({ 1,0,0,1 },log.logString.c_str());
		}
		else
		{
			ImGui::Text(log.logString.c_str());
		}
	}
}

void GImGuiLogWindow::on_resize()
{
}

void GImGuiLogWindow::on_data_update()
{
}

void GImGuiLogWindow::destroy()
{
}

const char* GImGuiLogWindow::get_window_name()
{
	return m_name.c_str();
}

spdlog::sinks::base_sink<std::mutex>* GImGuiLogWindow::create_sink()
{
	return new EditorLogSink(this);
}

void GImGuiLogWindow::add_log(const Log& log)
{
	m_lines.push_back(log);
}

Log::Log(LOG_LEVEL level, const char* logMessage)
{
	switch (level)
	{
	case LOG_LEVEL_TRACE:
		logString = "[Trace] ";
		break;
	case LOG_LEVEL_DEBUG:
		logString = "[Debug] ";
		break;
	case LOG_LEVEL_INFO:
		logString = "[Info] ";
		break;
	case LOG_LEVEL_WARNING:
		logString = "[Warning] ";
		break;
	case LOG_LEVEL_ERROR:
		logString = "[Error] ";
		break;
	case LOG_LEVEL_CRITICAL:
		logString = "[Critical] ";
		break;
	case LOG_LEVEL_OFF:
		break;
	default:
		break;
	}

	if (logString.size() != 0)
	{
		logString += logMessage;
	}

	this->level = level;
}
