#ifndef IMGUI_WINDOW_MANAGER_H
#define IMGUI_WINDOW_MANAGER_H

#include "public/core/templates/unordered_dense.h"
#include "internal/gimgui_window.h"

#include <mutex>

class IGImGuiWindowImpl;

class ImGuiWindowManager
{
public:
	// Multi-thread safe
	void create_imgui_window(IGImGuiWindowImpl* impl, GIMGUIWINDOWDIR dir = GIMGUIWINDOWDIR_LEFT);

	bool init();

	void destroy();

	void render_windows();

	void render_main_menu();
private:
	void render_main_dockspace();

	void build_nodes();
private:
	ankerl::unordered_dense::segmented_map<uint32_t, GImGuiWindow*> m_windowMap;
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