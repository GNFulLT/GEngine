#ifndef GTHEME_MENU_H
#define GTHEME_MENU_H

#include "editor/igimgui_menu_impl.h"
#include "editor/igimgui_theme.h"
class GThemeMenu : public IGImGuiMenuImpl
{
public:
	// Inherited via IGImGuiMenuImpl
	virtual bool init() override;
	virtual bool need_render() override;
	virtual void render() override;
	virtual void on_resize() override;
	virtual void on_data_update() override;
	virtual const char* get_menu_name() override;
private:

	
};



#endif // GTHEME_MENU_H