#ifndef GIMGUI_LOG_WINDOW_H
#define GIMGUI_LOG_WINDOW_H

#include "editor/igimgui_window_impl.h"
#include <string>
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
private:
	std::string m_name;
	GImGuiWindowStorage* m_storage;
};

#endif // GIMGUI_LOG_WINDOW_H