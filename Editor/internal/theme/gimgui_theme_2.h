#ifndef GIMGUI_THEME_2_H
#define GIMGUI_THEME_2_H

#include "editor/igimgui_theme.h"
#include <string>
class GImGuiTheme2 : public IGImGuiTheme
{
public:
	GImGuiTheme2();
	// Inherited via IGImguiTheme
	virtual bool is_valid() override;
	virtual const char* get_theme_name() override;
	virtual void setImGuiTheme(ImGuiStyle& style) override;
	virtual bool init() override;
	virtual void destroy() override;
private:
	std::string m_name;

};

#endif // GIMGUI_THEME_2_H