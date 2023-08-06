#ifndef GTHEME_MENU_H
#define GTHEME_MENU_H

#include "editor/igimgui_menu_impl.h"
#include "editor/igimgui_theme.h"
#include "public/core/templates/unordered_dense.h"
#include <string>
#include <utility>
#include <vector>

class GThemeMenu : public IGImGuiMenuImpl
{
public:
	//X TODO : Pointer should be unique or shared

	//X Theme name should be unique
	bool add_theme(IGImGuiTheme* theme);

	// Inherited via IGImGuiMenuImpl
	virtual bool init() override;
	virtual bool need_render() override;
	virtual void render() override;
	virtual void on_resize() override;
	virtual void on_data_update() override;
	virtual const char* get_menu_name() override;
	

private:
	void select_theme(int index);

	void init_built_in_themes();
private:
	//X TODO : Pointer should be unique or shared
	ankerl::unordered_dense::segmented_map<std::string, IGImGuiTheme*> m_themes;
	std::vector<std::pair<bool, IGImGuiTheme*>> m_themesVector;
	std::pair<bool, IGImGuiTheme*>* m_selectedTheme = nullptr;

	// Inherited via IGImGuiMenuImpl
	virtual void destroy() override;
};



#endif // GTHEME_MENU_H