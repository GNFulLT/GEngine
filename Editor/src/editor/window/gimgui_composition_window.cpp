#include "internal/window/gimgui_composition_window.h"
#include "editor/editor_application_impl.h"
#include "engine/rendering/vulkan/named/igvulkan_named_viewport.h"
#include "internal/rendering/vulkan/gimgui_descriptor_creator.h"
#include "imgui/imgui.h"
#include <algorithm>
#include "engine/rendering/vulkan/ivulkan_descriptor.h"
#include "engine/gengine.h"

GImGuiCompositionPortWindow::GImGuiCompositionPortWindow()
{
	m_name = "Composition";
}

bool GImGuiCompositionPortWindow::init()
{
	return true;
}

void GImGuiCompositionPortWindow::set_storage(GImGuiWindowStorage* storage)
{
}

bool GImGuiCompositionPortWindow::need_render()
{
	return true;
}

void GImGuiCompositionPortWindow::render()
{
	if (EditorApplicationImpl::get_instance()->compositionPortSet == nullptr)
		return;
	ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
	int x = std::max(0, int(viewportPanelSize.x));
	int y = std::max(0, int(viewportPanelSize.y));
	ImGui::Image(EditorApplicationImpl::get_instance()->compositionPortSet->get_vk_descriptor(), ImVec2{ float(x),float(y) });
}

void GImGuiCompositionPortWindow::on_resize()
{
}

void GImGuiCompositionPortWindow::on_data_update()
{
}

void GImGuiCompositionPortWindow::destroy()
{
}

const char* GImGuiCompositionPortWindow::get_window_name()
{
	return m_name.c_str();
}
