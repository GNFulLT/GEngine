#ifndef GIMGUI_GRID_SETTINGS_WINDOW_H
#define GIMGUI_GRID_SETTINGS_WINDOW_H


#include "editor/igimgui_window_impl.h"

#include <string>
class IGVulkanViewport;
class GImGuiWindowStorage;

class GImGuiGridSettingsWindow : public IGImGuiWindowImpl
{
public:
	GImGuiGridSettingsWindow();
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
	
};

#endif // GIMGUI_GRID_SETTINGS_WINDOW_H