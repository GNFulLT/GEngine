#include "internal/window/gimgui_grid_settings_window.h"
#include "internal/rendering/vulkan/ggrid_renderer.h"
#include <imgui/imgui.h>

GImGuiGridSettingsWindow::GImGuiGridSettingsWindow(GridRenderer* renderer)
{
	m_windowName = "Grid Settings";
	m_renderer = renderer;
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
	ImGui::Checkbox("Enabled",&m_renderer->m_wantsRender);
	ImGui::SliderFloat("Grid Cell Size",&m_renderer->m_spec.gridCellSize,0.01f,1.0f);
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
