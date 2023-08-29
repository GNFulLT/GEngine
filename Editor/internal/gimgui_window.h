#ifndef IMGUI_WINDOW_H
#define IMGUI_WINDOW_H

#include "editor/igimgui_window_impl.h"
#include <string>

enum GIMGUIWINDOWDIR
{
	GIMGUIWINDOWDIR_NONE,
	GIMGUIWINDOWDIR_LEFT,
	GIMGUIWINDOWDIR_RIGHT,
	GIMGUIWINDOWDIR_MIDDLE,
	GIMGUIWINDOWDIR_BOTTOM
};


class GImGuiWindow
{
public:
	GImGuiWindow(IGImGuiWindowImpl* impl);

	bool init();
	
	void render();

	bool need_render();

	const char* get_window_name();

	void destroy();
	
	void set_dock_dir(GIMGUIWINDOWDIR dir);

	bool wants_docking();

	bool wants_destroy();

	GIMGUIWINDOWDIR get_dock_dir();
private:
	IGImGuiWindowImpl* m_impl;
	GImGuiWindowStorage m_storage;
	GIMGUIWINDOWDIR m_dockDir = GIMGUIWINDOWDIR_NONE;
	uint32_t m_windowId;
	bool m_isShown = true;
	std::string m_id;
};

#endif // IMGUI_WINDOW_H