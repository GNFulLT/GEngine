#ifndef IGIMGUI_THEME_H
#define IGIMGUI_THEME_H

#include "GEngine_EXPORT.h"

#include <string>

class ImGuiStyle;

class IGImGuiTheme
{
public:
	virtual bool is_valid() = 0;
	virtual const char* get_theme_name() = 0;
	virtual void setImGuiTheme(ImGuiStyle& style) = 0;

	virtual bool init() = 0;
	virtual void destroy() = 0;
private:
};


#endif // IGIMGUI_THEME_H