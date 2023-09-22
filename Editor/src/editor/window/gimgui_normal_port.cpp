#include "internal/window/gimgui_normal_port.h"
#include "editor/editor_application_impl.h"
#include "engine/rendering/vulkan/named/igvulkan_named_viewport.h"
#include "internal/rendering/vulkan/gimgui_descriptor_creator.h"
#include "imgui/imgui.h"
#include <algorithm>
#include "engine/rendering/vulkan/ivulkan_descriptor.h"
#include "engine/gengine.h"

GImGuiNormalPortWindow::GImGuiNormalPortWindow()
{	
	m_name = "Normal";
}

bool GImGuiNormalPortWindow::init()
{	
	return true;
}

void GImGuiNormalPortWindow::set_storage(GImGuiWindowStorage* storage)
{
}

bool GImGuiNormalPortWindow::need_render()
{
	return true;
}

void GImGuiNormalPortWindow::render()
{
	
	if (EditorApplicationImpl::get_instance()->normalPortSet == nullptr)
		return;
	ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
	int x = std::max(0, int(viewportPanelSize.x));
	int y = std::max(0, int(viewportPanelSize.y));
	ImGui::Image(EditorApplicationImpl::get_instance()->normalPortSet->get_vk_descriptor(), ImVec2{ float(x),float(y) });
}

void GImGuiNormalPortWindow::on_resize()
{
}

void GImGuiNormalPortWindow::on_data_update()
{
}

void GImGuiNormalPortWindow::destroy()
{
}

const char* GImGuiNormalPortWindow::get_window_name()
{
	return m_name.c_str();
}
