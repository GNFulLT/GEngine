#ifndef GIMGUI_SCRIPT_WINDOW_PROVIDER_H
#define GIMGUI_SCRIPT_WINDOW_PROVIDER_H

#include "editor/igimgui_window_provider.h"
#include <string>

class GImGuiScriptWindowProvider : public IGImGuiWindowProvider
{
public:
	GImGuiScriptWindowProvider();

	// Inherited via IGImGuiWindowProvider
	virtual const char* get_provider_name() override;
	virtual IGImGuiWindowImpl* create_window_impl() override;

private:
	std::string m_providerName;

};
#endif // GIMGUI_SCRIPT_WINDOW_PROVIDER_H