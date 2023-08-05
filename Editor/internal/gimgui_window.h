#ifndef IMGUI_WINDOW_H
#define IMGUI_WINDOW_H

#include "editor/igimgui_window_impl.h"

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
private:
	IGImGuiWindowImpl* m_impl;
	GImGuiWindowStorage m_storage;

	uint32_t m_windowId;
};

#endif // IMGUI_WINDOW_H