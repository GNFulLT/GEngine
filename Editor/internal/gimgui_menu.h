#ifndef GIMGUI_MENU_H
#define GIMGUI_MENU_H

#include "editor/igimgui_menu_impl.h"
#include <cstdint>
class GImGuiMenu
{
public:
	GImGuiMenu(IGImGuiMenuImpl* impl);

	bool init();

	void render();

	bool need_render();

	const char* get_menu_name();

	void destroy();
private:
	IGImGuiMenuImpl* m_impl;

	uint32_t m_menuId;
};

#endif // GIMGUI_MENU_H