#include "internal/window/gimgui_position_port.h"
#include "editor/editor_application_impl.h"
#include "engine/rendering/vulkan/named/igvulkan_named_viewport.h"
#include "internal/rendering/vulkan/gimgui_descriptor_creator.h"
#include "imgui/imgui.h"
#include <algorithm>
#include "engine/rendering/vulkan/ivulkan_descriptor.h"
#include "engine/gengine.h"
GImGuiPositionPortWindow::GImGuiPositionPortWindow()
{
	m_name = "Position";
}

bool GImGuiPositionPortWindow::init()
{
	
	return true;
}

void GImGuiPositionPortWindow::set_storage(GImGuiWindowStorage* storage)
{
	m_storage = storage;
}

bool GImGuiPositionPortWindow::need_render()
{
	return true;
}

void GImGuiPositionPortWindow::render()
{
	ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
	int x = std::max(0, int(viewportPanelSize.x));
	int y = std::max(0, int(viewportPanelSize.y));
	if (EditorApplicationImpl::get_instance()->positionPortSet != nullptr)
	{
		ImGui::Image(EditorApplicationImpl::get_instance()->positionPortSet->get_vk_descriptor(), ImVec2{ float(x),float(y) });
	}
	

}

void GImGuiPositionPortWindow::on_resize()
{

}

void GImGuiPositionPortWindow::on_data_update()
{
}

void GImGuiPositionPortWindow::destroy()
{
}

const char* GImGuiPositionPortWindow::get_window_name()
{
	return m_name.c_str();
}
