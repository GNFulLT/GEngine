#ifndef GWINDOW_MENU_H
#define GWINDOW_MENU_H

#include "editor/igimgui_window_provider.h"
#include <unordered_map>
#include <string>
#include <vector>
#include "editor/igimgui_menu_impl.h"
#include "internal/imgui_window_manager.h"

class GWindowMenu : public IGImGuiMenuImpl
{
public:
	GWindowMenu(ImGuiWindowManager* windowMng);
	bool register_provider(IGImGuiWindowProvider* provider);

	// Inherited via IGImGuiMenuImpl
	virtual bool init() override;
	virtual bool need_render() override;
	virtual void render() override;
	virtual void on_resize() override;
	virtual void on_data_update() override;
	virtual const char* get_menu_name() override;
	virtual void destroy() override;
private:
	std::unordered_map<std::string, IGImGuiWindowProvider*> m_providerMap;
	std::vector<IGImGuiWindowProvider*> m_providers;
	ImGuiWindowManager* m_windowManager;
	std::string m_name;
};


#endif // GWINDOW_MENU_H