#include "internal/window/gimgui_pbr_window.h"
#include "editor/editor_application_impl.h"
#include "engine/rendering/vulkan/named/igvulkan_named_viewport.h"
#include "internal/rendering/vulkan/gimgui_descriptor_creator.h"
#include "imgui/imgui.h"
#include <algorithm>
#include "engine/rendering/vulkan/ivulkan_descriptor.h"
#include "engine/gengine.h"

GImGuiPBRPortWindow::GImGuiPBRPortWindow()
{
	m_name = "PBR(AO/R/M)";	
}

bool GImGuiPBRPortWindow::init()
{

	return true;
}

void GImGuiPBRPortWindow::set_storage(GImGuiWindowStorage* storage)
{
	m_storage = storage;
}

bool GImGuiPBRPortWindow::need_render()
{
	return true;
}

void GImGuiPBRPortWindow::render()
{
	ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
	int x = std::max(0, int(viewportPanelSize.x));
	int y = std::max(0, int(viewportPanelSize.y));
	if (EditorApplicationImpl::get_instance()->pbrPortSet != nullptr)
	{
		ImGui::Image(EditorApplicationImpl::get_instance()->pbrPortSet->get_vk_descriptor(), ImVec2{ float(x),float(y) });
	}


}

void GImGuiPBRPortWindow::on_resize()
{

}

void GImGuiPBRPortWindow::on_data_update()
{
}

void GImGuiPBRPortWindow::destroy()
{
}

const char* GImGuiPBRPortWindow::get_window_name()
{
	return m_name.c_str();
}
