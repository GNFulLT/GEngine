#include "internal/window/gimgui_scene_window.h"
#include "engine/rendering/scene/scene.h"
#include "engine/gengine.h"
#include "editor/editor_application_impl.h"
#include "imgui/imgui.h"
#include <spdlog/fmt/fmt.h>

GImGuiSceneWindow::GImGuiSceneWindow()
{
	m_name = "Scene";
}

bool GImGuiSceneWindow::init()
{
	//m_scene = EditorApplicationImpl::get_instance()->m_engine->get_global_scene();
	return true;
}

void GImGuiSceneWindow::set_storage(GImGuiWindowStorage* storage)
{
}

bool GImGuiSceneWindow::need_render()
{
	return true;
}

void GImGuiSceneWindow::render()
{
	//if (m_scene->hierarchy.size() == 0)
	//	return;
	//int iter = m_scene->hierarchy[0].firstChild;
	//
	//while (iter > -1 && iter < m_scene->hierarchy.size())
	//{

	//	bool isOpen = ImGui::TreeNodeEx(fmt::format("Node {}", iter).c_str(), ImGuiTreeNodeFlags_OpenOnDoubleClick);
	//	if (ImGui::IsItemClicked(ImGuiMouseButton_::ImGuiMouseButton_Left))
	//	{
	//		m_selectedEntity = iter;
	//	}
	//	//X Iterate here left child tree
	//	if (isOpen)
	//	{
	//		ImGui::TreePop();
	//	}

	//	//X Iterate parents
	//	iter = m_scene->hierarchy[iter].nextSibling;
	//}
}

void GImGuiSceneWindow::on_resize()
{
}

void GImGuiSceneWindow::on_data_update()
{
}

void GImGuiSceneWindow::destroy()
{
}

const char* GImGuiSceneWindow::get_window_name()
{
	return m_name.c_str();
}

uint32_t GImGuiSceneWindow::get_selected_entity() const noexcept
{
	return m_selectedEntity;
}
