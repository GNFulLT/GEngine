#include "internal/menu/gtheme_menu.h"

bool GThemeMenu::init()
{
	return true;
}

bool GThemeMenu::need_render()
{
	return true;
}

void GThemeMenu::render()
{
}

void GThemeMenu::on_resize()
{
}

void GThemeMenu::on_data_update()
{
}

const char* GThemeMenu::get_menu_name()
{
	return "Themes";
}
