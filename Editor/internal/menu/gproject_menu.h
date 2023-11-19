#ifndef GPROJECT_MENU_H
#define GPROJECT_MENU_H

#include "editor/igimgui_menu_impl.h"
#include "editor/igimgui_theme.h"
#include "public/core/templates/unordered_dense.h"
#include <string>
#include <utility>
#include <vector>

class GProjectMenu : public IGImGuiMenuImpl
{
public:
	// Inherited via IGImGuiMenuImpl
	virtual bool init() override;
	virtual bool need_render() override;
	virtual void render() override;
	virtual void on_resize() override;
	virtual void on_data_update() override;
	virtual const char* get_menu_name() override;
	virtual void destroy() override;

private:
	std::string m_menuName;
};



#endif // GPROJECT_MENU_H