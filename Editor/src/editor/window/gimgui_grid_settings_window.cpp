#include "internal/window/gimgui_grid_settings_window.h"

GImGuiGridSettingsWindow::GImGuiGridSettingsWindow()
{
	m_windowName = "Grid Settings";
}

bool GImGuiGridSettingsWindow::init()
{
	return true;
}

void GImGuiGridSettingsWindow::set_storage(GImGuiWindowStorage* storage)
{
}

bool GImGuiGridSettingsWindow::need_render()
{
	return true;
}

void GImGuiGridSettingsWindow::render()
{
}

void GImGuiGridSettingsWindow::on_resize()
{
}

void GImGuiGridSettingsWindow::on_data_update()
{
}

void GImGuiGridSettingsWindow::destroy()
{
}

const char* GImGuiGridSettingsWindow::get_window_name()
{
	return m_windowName.c_str();
}
