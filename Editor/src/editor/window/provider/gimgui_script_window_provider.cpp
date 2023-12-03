#include "internal/window/provider/gimgui_script_window_provider.h"
#include "internal/window/gimgui_script_window.h"
GImGuiScriptWindowProvider::GImGuiScriptWindowProvider()
{
	m_providerName = "Script";
}

const char* GImGuiScriptWindowProvider::get_provider_name()
{
	return m_providerName.c_str();
}

IGImGuiWindowImpl* GImGuiScriptWindowProvider::create_window_impl()
{
	return new GImGuiScriptWindow();
}
