#ifndef IGIMGUI_WINDOW_PROVIDER_H
#define IGIMGUI_WINDOW_PROVIDER_H

#include "editor/igimgui_window_impl.h"


class IGImGuiWindowProvider
{
public:
	virtual ~IGImGuiWindowProvider() = default;

	virtual const char* get_provider_name() = 0;

	virtual IGImGuiWindowImpl* create_window_impl() = 0;

	
private:
};

#endif // IGIMGUI_WINDOW_PROVIDER_H