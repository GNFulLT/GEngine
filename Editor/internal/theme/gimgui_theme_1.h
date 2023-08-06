#ifndef GIMGUI_THEME_1_H
#define GIMGUI_THEME_1_H

#include "editor/igimgui_theme.h"
#include <string>
class GImGuiTheme1 : public IGImGuiTheme
{
public:
	GImGuiTheme1();
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