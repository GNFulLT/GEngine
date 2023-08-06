#ifndef IMGUI_WINDOW_MANAGER_H
#define IMGUI_WINDOW_MANAGER_H

#include "public/core/templates/unordered_dense.h"
#include "internal/gimgui_window.h"
#include "internal/gimgui_menu.h"

#include <mutex>
#include <string>
#include <vector>

class ImGuiWindowManager
{
public:
	// Multi-thread safe
	bool create_imgui_window(IGImGuiWindowImpl* impl, GIMGUIWINDOWDIR dir = GIMGUIWINDOWDIR_LEFT);

	bool create_imgui_menu(IGImGuiMenuImpl* impl);

	bool init();

	void destroy();

	void render_windows();

	void render_main_menu();
private:
	void render_main_dockspace();

	void build_nodes();

	void draw_main_menu_bar();
private:
	//X TODO : USE SHARED OR UNIQUE PTR
	ankerl::unordered_dense::segmented_map<std::string, GImGuiWindow*> m_windowMap;

	ankerl::unordered_dense::segmented_map<std::string,GImGuiMenu*> m_menuMap;
	std::vector<GImGuiMenu*> m_menuVector;

	uint32_t m_dock_id;

	// Nodes
	uint32_t dock_id_left;
	uint32_t dock_id_right;
	uint32_t dock_id_bottom;
	uint32_t dock_id_middle;

	bool m_isDockDirty = true;

	uint64_t m_windowId = 0;

	std::mutex m_windowCreationMutex;
};

#endif // IMGUI_WINDOW_MANAGER_H