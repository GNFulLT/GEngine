#ifndef GIMGUI_SCRIPT_WINDOW_H
#define GIMGUI_SCRIPT_WINDOW_H

#include "editor/igimgui_window_impl.h"
#include <string>
#include "engine/manager/igscript_manager.h"

class GImGuiScriptWindow : public IGImGuiWindowImpl
{
public:
	GImGuiScriptWindow();

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
	std::string m_windowName;
	IGScriptManager* m_scriptManager;

};

#endif // GIMGUI_SCRIPT_WINDOW_H