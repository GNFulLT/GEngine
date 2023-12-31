#include "internal/window/gimgui_composition_window.h"
#include "editor/editor_application_impl.h"
#include "engine/rendering/vulkan/named/igvulkan_named_viewport.h"
#include "internal/rendering/vulkan/gimgui_descriptor_creator.h"
#include "imgui/imgui.h"
#include <algorithm>
#include "engine/rendering/vulkan/ivulkan_descriptor.h"
#include "engine/gengine.h"
#include <cassert>
#include "engine/imanager_table.h"
#include "engine/manager/igscene_manager.h"
#include "internal/imgui_layer.h"
#include "engine/manager/igcamera_manager.h"
#include "internal/window/gimgui_scene_window.h"
#include "internal/imguizmo.h"
#include "engine/manager/igscene_manager.h"
#include "engine/imanager_table.h"
#include "public/core/templates/shared_ptr.h"
#include <glm/ext.hpp>
#include "engine/rendering/scene/scene.h"
#include <glm/glm.hpp>
#include "engine/rendering/renderer/igvulkan_deferred_renderer.h"

GImGuiCompositionPortWindow::GImGuiCompositionPortWindow()
{
	m_name = "Composition";
}

bool GImGuiCompositionPortWindow::init()
{
	m_sceneWindow = EditorApplicationImpl::get_instance()->get_editor_layer()->get_scene_window();
	m_cameraManager = ((GSharedPtr<IGCameraManager>*)EditorApplicationImpl::get_instance()->m_engine->get_manager_table()->get_engine_manager_managed(ENGINE_MANAGER_CAMERA))->get();
	m_sceneManager = ((GSharedPtr<IGSceneManager>*)EditorApplicationImpl::get_instance()->m_engine->get_manager_table()->get_engine_manager_managed(ENGINE_MANAGER_SCENE))->get();

	return true;
}

void GImGuiCompositionPortWindow::set_storage(GImGuiWindowStorage* storage)
{
	m_storage = storage;
}

bool GImGuiCompositionPortWindow::need_render()
{
	return true;
}

void GImGuiCompositionPortWindow::render()
{
	auto winSize = ImGui::GetWindowSize();
	
	auto oldX = ImGui::GetCursorPosX();
	auto size = ImGui::CalcTextSize("Transform");
	ImGui::SetCursorPosX((winSize.x / 2) - size.x);
	
	if (ImGui::Button("Transform"))
	{
		tType = TRANSFORM_TYPE_TRANSLATE;
	}
	ImGui::SameLine();
	if (ImGui::Button("Scale"))
	{
		tType = TRANSFORM_TYPE_SCALE;

	}
	ImGui::SameLine();
	if (ImGui::Button("Rotate"))
	{
		tType = TRANSFORM_TYPE_ROTATE;
	}


	if (EditorApplicationImpl::get_instance()->compositionPortSet == nullptr)
		return;
	ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
	int x = std::max(0, int(viewportPanelSize.x));
	int y = std::max(0, int(viewportPanelSize.y));
	ImGui::Image(EditorApplicationImpl::get_instance()->compositionPortSet->get_vk_descriptor(), ImVec2{ float(x),float(y) });

	if (auto selectedNode = m_sceneWindow->get_selected_entity(); selectedNode != -1)
	{
		auto scene = m_sceneManager->get_current_scene();
		auto mtrx = scene->get_matrix_of(selectedNode);
		if (scene->has_matrix(selectedNode))
		{
			if (m_selectedNode != selectedNode)
			{
				m_selectedNode = selectedNode;
			}
			ImGuizmo::SetOrthographic(false);
			ImGuizmo::SetDrawlist();
			float width = ImGui::GetWindowWidth();
			float height = ImGui::GetWindowHeight();
			ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, width,height);
			auto camProj = glm::make_mat4(m_cameraManager->get_camera_proj_matrix());
			bool isChanged = ImGuizmo::Manipulate(m_cameraManager->get_camera_view_matrix(), glm::value_ptr(camProj), ImGuizmo::OPERATION(transformTypeToImgui(tType)),
				ImGuizmo::LOCAL, glm::value_ptr(mtrx));

			if (isChanged)
			{
				scene->set_transform_of(m_selectedNode, mtrx);
				//scene->mark_as_changed(m_selectedNode);
			}
		}
	}
	else
	{
		m_selectedNode = -1;
	}
}

void GImGuiCompositionPortWindow::on_resize()
{
	EditorApplicationImpl::get_instance()->m_engine->add_recreation([&, posPort = EditorApplicationImpl::get_instance()->positionPortSet,
		albedoPort = EditorApplicationImpl::get_instance()->albedoPortSet, normalPort = EditorApplicationImpl::get_instance()->normalPortSet, composSet = EditorApplicationImpl::get_instance()->compositionPortSet,
		pbrSet = EditorApplicationImpl::get_instance()->pbrPortSet, sunShadowSet = EditorApplicationImpl::get_instance()->sunShadowPortSet]() {
			assert(m_storage->width > 0 && m_storage->height > 0);
			////X Destroy Sets
			EditorApplicationImpl::get_instance()->get_descriptor_creator()->destroy_descriptor_set_dtor(posPort);
			EditorApplicationImpl::get_instance()->get_descriptor_creator()->destroy_descriptor_set_dtor(normalPort);
			EditorApplicationImpl::get_instance()->get_descriptor_creator()->destroy_descriptor_set_dtor(albedoPort);
			EditorApplicationImpl::get_instance()->get_descriptor_creator()->destroy_descriptor_set_dtor(composSet);
			EditorApplicationImpl::get_instance()->get_descriptor_creator()->destroy_descriptor_set_dtor(pbrSet);
			EditorApplicationImpl::get_instance()->get_descriptor_creator()->destroy_descriptor_set_dtor(sunShadowSet);

			auto deferredVp = EditorApplicationImpl::get_instance()->m_renderViewport;
			deferredVp->resize(m_storage->width, m_storage->height);


			EditorApplicationImpl::get_instance()->positionPortSet = EditorApplicationImpl::get_instance()->get_descriptor_creator()->create_descriptor_set_for_texture(deferredVp->get_position_attachment(),
				deferredVp->get_sampler_for_named_attachment("position_attachment")).value();
			EditorApplicationImpl::get_instance()->normalPortSet = EditorApplicationImpl::get_instance()->get_descriptor_creator()->create_descriptor_set_for_texture(deferredVp->get_emission_attachment(),
				deferredVp->get_sampler_for_named_attachment("normal_attachment")).value();
			EditorApplicationImpl::get_instance()->albedoPortSet = EditorApplicationImpl::get_instance()->get_descriptor_creator()->create_descriptor_set_for_texture(deferredVp->get_albedo_attachment(),
				deferredVp->get_sampler_for_named_attachment("albedo_attachment")).value();
			EditorApplicationImpl::get_instance()->compositionPortSet = EditorApplicationImpl::get_instance()->get_descriptor_creator()->create_descriptor_set_for_texture(deferredVp->get_composition_attachment(),
				deferredVp->get_sampler_for_named_attachment("composition_attachment")).value();

			EditorApplicationImpl::get_instance()->pbrPortSet = EditorApplicationImpl::get_instance()->get_descriptor_creator()->create_descriptor_set_for_texture(deferredVp->get_pbr_attachment(),
				deferredVp->get_sampler_for_named_attachment("pbr_attachment")).value();
			

			auto sceneManager = ((GSharedPtr<IGSceneManager>*)EditorApplicationImpl::get_instance()->m_engine->get_manager_table()->get_engine_manager_managed(ENGINE_MANAGER_SCENE))->get();
			sceneManager->init_deferred_renderer(deferredVp);
			auto sunShadowAttachment = sceneManager->get_deferred_renderer()->get_sun_shadow_attachment();

			EditorApplicationImpl::get_instance()->sunShadowPortSet = EditorApplicationImpl::get_instance()->get_descriptor_creator()->create_descriptor_set_for_depth_texture(sunShadowAttachment,
				deferredVp->get_sampler_for_named_attachment("pbr_attachment")).value();
		});

	EditorApplicationImpl::get_instance()->albedoPortSet = nullptr;
	EditorApplicationImpl::get_instance()->normalPortSet = nullptr;
	EditorApplicationImpl::get_instance()->positionPortSet = nullptr;
	EditorApplicationImpl::get_instance()->compositionPortSet = nullptr;
	EditorApplicationImpl::get_instance()->pbrPortSet = nullptr;
	EditorApplicationImpl::get_instance()->sunShadowPortSet = nullptr;

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

uint32_t GImGuiCompositionPortWindow::transformTypeToImgui(TRANSFORM_TYPE type)
{
	switch (type)
	{
	case GImGuiCompositionPortWindow::TRANSFORM_TYPE_TRANSLATE:
		return ImGuizmo::TRANSLATE;
	case GImGuiCompositionPortWindow::TRANSFORM_TYPE_SCALE:
		return ImGuizmo::SCALE;
	case GImGuiCompositionPortWindow::TRANSFORM_TYPE_ROTATE:
		return ImGuizmo::ROTATE;
	default:
		return ImGuizmo::TRANSLATE;
	}
}
