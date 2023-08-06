#ifndef GIMGUI_OPEN_DARK_THEME_H
#define GIMGUI_OPEN_DARK_THEME_H

#include "editor/igimgui_theme.h"
#include <string>
class GImGuiBrightDarkTheme : public IGImGuiTheme
{
public:
	GImGuiBrightDarkTheme();
	// Inherited via IGImguiTheme
	virtual bool is_valid() override;
	virtual const char* get_theme_name() override;
	virtual void setImGuiTheme(ImGuiStyle& style) override;
	virtual bool init() override;
	virtual void destroy() override;
private:
	std::string m_name;

};

#endif // GIMGUI_OPEN_DARK_THEME_H