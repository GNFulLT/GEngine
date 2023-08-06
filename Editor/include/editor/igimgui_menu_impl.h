#ifndef IGIMGUI_MENU_IMPL_H
#define IGIMGUI_MENU_IMPL_H

#include "GEngine_EXPORT.h"

class EDITOR_API IGImGuiMenuImpl
{
public:
	virtual ~IGImGuiMenuImpl() = default;

	//X If it returns false. The window will be discarded
	virtual bool init() = 0;

	virtual bool need_render() = 0;
	virtual void render() = 0;
	virtual void on_resize() = 0;

	//X TODO : This will be called in update method maybe.
	virtual void on_data_update() = 0;

	virtual const char* get_menu_name() = 0;

	virtual void destroy() = 0;
private:
};


#endif // IGIMGUI_MENU_IMPL_H