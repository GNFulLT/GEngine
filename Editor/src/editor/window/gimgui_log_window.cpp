#include "internal/window/gimgui_log_window.h"
#include <imgui.h>
GImGuiLogWindow::GImGuiLogWindow()
{
	m_name = "Debug Log";
}

bool GImGuiLogWindow::init()
{
	return true;
}

void GImGuiLogWindow::set_storage(GImGuiWindowStorage* storage)
{
	m_storage = storage;
}

bool GImGuiLogWindow::need_render()
{
	return true;
}

void GImGuiLogWindow::render()
{
	ImGui::Selectable("Im Log");
}

void GImGuiLogWindow::on_resize()
{
}

void GImGuiLogWindow::on_data_update()
{
}

void GImGuiLogWindow::destroy()
{
}

const char* GImGuiLogWindow::get_window_name()
{
	return m_name.c_str();
}
