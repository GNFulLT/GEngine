#include "internal/window/gimgui_scene_window.h"
#include "engine/rendering/scene/scene.h"
#include "engine/gengine.h"
#include "editor/editor_application_impl.h"
#include "imgui/imgui.h"
#include <spdlog/fmt/fmt.h>
#include "engine/manager/igscene_manager.h"
#include "engine/imanager_table.h"
#include "engine/plugin/igscript_object.h"
#include "engine/scene/component/script_group_component.h"

GImGuiSceneWindow::GImGuiSceneWindow()
{
	m_name = "Scene";
}

bool GImGuiSceneWindow::init()
{
	m_sceneManager = ((GSharedPtr<IGSceneManager>*)EditorApplicationImpl::get_instance()->m_engine->get_manager_table()->get_engine_manager_managed(ENGINE_MANAGER_SCENE))->get();
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
	auto m_scene = m_sceneManager->get_current_scene();
	if (m_scene == nullptr)
		return;
	if (m_scene->hierarchy.size() == 0)
		return;
	int iter = m_scene->hierarchy[0].firstChild;
	
	while (iter > -1 && iter < m_scene->hierarchy.size())
	{

		bool isOpen = ImGui::TreeNodeEx(fmt::format("Node {}", iter).c_str(), ImGuiTreeNodeFlags_OpenOnDoubleClick);

		if (ImGui::IsItemClicked(ImGuiMouseButton_::ImGuiMouseButton_Left))
		{
			m_selectedEntity = iter;
		}
	

		if (ImGui::BeginDragDropTarget())
		{
			if (ImGui::AcceptDragDropPayload("SCRIPT_TYPE"))
			{
				auto currentNodeGEntity = m_sceneManager->get_entity_by_id(iter);
				auto ramLoc = (std::size_t*)ImGui::GetDragDropPayload()->Data;
				if (!currentNodeGEntity->has_component<ScriptGroupComponent>())
					currentNodeGEntity->emplace_component<ScriptGroupComponent>(currentNodeGEntity);
				auto& scriptComp = currentNodeGEntity->get_component<ScriptGroupComponent>();
				auto obj = (IGScriptObject*)(*ramLoc);
				scriptComp.try_to_register_script(obj);
			}
			ImGui::EndDragDropTarget();
		}
		
		
		//X Iterate here left child tree
		if (isOpen)
		{
			ImGui::TreePop();
		}

		//X Iterate parents
		iter = m_scene->hierarchy[iter].nextSibling;
	}
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
