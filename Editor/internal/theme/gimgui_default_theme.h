#ifndef GIMGUI_DEFAULT_THEME_H
#define GIMGUI_DEFAULT_THEME_H

#include "editor/igimgui_theme.h"
#include <string>
class GImGuiDefaultTheme : public IGImGuiTheme
{
public:
	GImGuiDefaultTheme();
	// Inherited via IGImguiTheme
	virtual bool is_valid() override;
	virtual const char* get_theme_name() override;
	virtual void setImGuiTheme(ImGuiStyle& style) override;
	virtual bool init() override;
	virtual void destroy() override;
private:
	std::string m_name;
	
};

#endif // GIMGUI_DEFAULT_THEME_H