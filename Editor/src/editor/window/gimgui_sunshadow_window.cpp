#include "internal/window/gimgui_sunshadow_window.h"
#include "editor/editor_application_impl.h"
#include "engine/rendering/vulkan/named/igvulkan_named_viewport.h"
#include "internal/rendering/vulkan/gimgui_descriptor_creator.h"
#include "imgui/imgui.h"
#include <algorithm>
#include "engine/rendering/vulkan/ivulkan_descriptor.h"
#include "engine/gengine.h"

GImGuiSunShadowPortWindow::GImGuiSunShadowPortWindow()
{
	m_name = "SunShadow";
}

bool GImGuiSunShadowPortWindow::init()
{

	return true;
}

void GImGuiSunShadowPortWindow::set_storage(GImGuiWindowStorage* storage)
{
	m_storage = storage;
}

bool GImGuiSunShadowPortWindow::need_render()
{
	return true;
}

void GImGuiSunShadowPortWindow::render()
{
	ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
	int x = std::max(0, int(viewportPanelSize.x));
	int y = std::max(0, int(viewportPanelSize.y));
	if (EditorApplicationImpl::get_instance()->sunShadowPortSet != nullptr)
	{
		ImGui::Image(EditorApplicationImpl::get_instance()->sunShadowPortSet->get_vk_descriptor(), ImVec2{ float(x),float(y) });
	}


}

void GImGuiSunShadowPortWindow::on_resize()
{

}

void GImGuiSunShadowPortWindow::on_data_update()
{
}

void GImGuiSunShadowPortWindow::destroy()
{
}

const char* GImGuiSunShadowPortWindow::get_window_name()
{
	return m_name.c_str();
}
