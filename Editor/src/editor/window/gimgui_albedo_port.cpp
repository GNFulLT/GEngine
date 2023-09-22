#include "internal/window/gimgui_albedo_port.h"
#include "editor/editor_application_impl.h"
#include "engine/rendering/vulkan/named/igvulkan_named_viewport.h"
#include "internal/rendering/vulkan/gimgui_descriptor_creator.h"
#include "imgui/imgui.h"
#include <algorithm>
#include "engine/rendering/vulkan/ivulkan_descriptor.h"
#include "engine/gengine.h"
GImGuiAlbedoPortWindow::GImGuiAlbedoPortWindow()
{
	m_name = "Albedo";
	
}

bool GImGuiAlbedoPortWindow::init()
{

	return true;
}

void GImGuiAlbedoPortWindow::set_storage(GImGuiWindowStorage* storage)
{
}

bool GImGuiAlbedoPortWindow::need_render()
{
	return true;
}

void GImGuiAlbedoPortWindow::render()
{
	if (EditorApplicationImpl::get_instance()->albedoPortSet == nullptr)
		return;
	ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
	int x = std::max(0, int(viewportPanelSize.x));
	int y = std::max(0, int(viewportPanelSize.y));
	ImGui::Image(EditorApplicationImpl::get_instance()->albedoPortSet->get_vk_descriptor(), ImVec2{ float(x),float(y) });
}

void GImGuiAlbedoPortWindow::on_resize()
{

}

void GImGuiAlbedoPortWindow::on_data_update()
{
}

void GImGuiAlbedoPortWindow::destroy()
{
}

const char* GImGuiAlbedoPortWindow::get_window_name()
{
	return m_name.c_str();
}
