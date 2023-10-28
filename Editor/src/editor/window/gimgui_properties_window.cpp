#include "internal/window/gimgui_properties_window.h"
#include "editor/editor_application_impl.h"
#include "engine/gengine.h"
#include "engine/manager/igcamera_manager.h"
#include "public/core/templates/shared_ptr.h"
#include <glm/ext.hpp>
#include "imgui.h"
#include "engine/imanager_table.h"
#include "engine/rendering/wireframe_spec.h"
#include "internal/imgui_layer.h"
#include "internal/window/gimgui_scene_window.h"
#include "engine/rendering/scene/scene.h"
#include "glm/gtx/matrix_decompose.hpp"
#include "spdlog/fmt/fmt.h"
#include "engine/rendering/point_light.h"

GImGuiPropertiesWindow::GImGuiPropertiesWindow()
{
	m_name = "Properties";
	m_frustrumMatrix = glm::perspective(70.f, 16.f / 9.f, m_nearPlane, m_farPlane);

}

bool GImGuiPropertiesWindow::init()
{
	m_cam = ((GSharedPtr<IGCameraManager>*)EditorApplicationImpl::get_instance()->m_engine->get_manager_table()->get_engine_manager_managed(ENGINE_MANAGER_CAMERA))->get();
	m_lodBase = m_cam->get_cull_data()->lodBase;
	m_lodStep = m_cam->get_cull_data()->lodStep;
	m_sceneWindow =  EditorApplicationImpl::get_instance()->get_editor_layer()->get_scene_window();
	m_sceneManager = ((GSharedPtr<IGSceneManager>*)EditorApplicationImpl::get_instance()->m_engine->get_manager_table()->get_engine_manager_managed(ENGINE_MANAGER_SCENE))->get();
	return true;
}

void GImGuiPropertiesWindow::set_storage(GImGuiWindowStorage* storage)
{
}

bool GImGuiPropertiesWindow::need_render()
{
	return true;
}

void GImGuiPropertiesWindow::render()
{
	auto cullData = m_cam->get_cull_data();
	auto renderer = m_sceneManager->get_deferred_renderer();
	auto sunProps = *m_sceneManager->get_sun_properties();
	//auto wireframeSpec = EditorApplicationImpl::get_instance()->m_engine->get_wireframe_spec();
	if (auto selectedNode = m_sceneWindow->get_selected_entity();selectedNode != -1)
	{
		auto scene=  m_sceneManager->get_current_scene();
		auto mtrx = scene->get_matrix_of(selectedNode);
		if (mtrx != nullptr)
		{
			if (selectedNode != m_currentSelectedNode)
			{
				m_currentSelectedNode = selectedNode;
				//X Reset caches
			
			}
			glm::vec3 skew;
			glm::vec4 perspective;
			glm::decompose(*mtrx, m_currentNodeScale, m_currentNodeRotatition, m_currentNodePosition, skew, perspective);
			m_currentNodeRotatition = glm::conjugate(m_currentNodeRotatition);
			bool isChanged = false;

			if (ImGui::InputFloat3("position : ", glm::value_ptr(m_currentNodePosition)))
			{
				isChanged = true;
			}
			if (ImGui::InputFloat3("scale : ", glm::value_ptr(m_currentNodeScale)))
			{
				isChanged = true;
			}


			if (isChanged)
			{
				scene->localTransform_[m_currentSelectedNode] = glm::translate(glm::mat4(1.f), m_currentNodePosition) * glm::mat4_cast(m_currentNodeRotatition) * glm::scale(glm::mat4(1.f), m_currentNodeScale);
				scene->mark_as_changed(m_currentSelectedNode);
			}
		}
		auto drawId = m_sceneManager->get_draw_id_of_node(selectedNode);
		if (drawId != -1)
		{
			auto drawInfo = m_sceneManager->get_draw_data_by_id(drawId);
			ImGui::Text(fmt::format("Mesh Id : {}",drawInfo->mesh).c_str());
			ImGui::Text(fmt::format("Material Id : {}", drawInfo->material).c_str());
		}

		auto isLight = m_sceneManager->is_node_light(selectedNode);
		if (isLight)
		{
			auto currProps = *m_sceneManager->get_point_light(selectedNode);
			bool isChanged = false;
			if (ImGui::InputFloat3("color : ", glm::value_ptr(currProps.color)))
			{
				isChanged = true;
			}
			if (ImGui::InputFloat("intensity : ",&currProps.intensity))
			{
				isChanged = true;
			}
			if (ImGui::InputFloat("radius : ", &currProps.radius))
			{
				isChanged = true;
			}
			if (ImGui::InputFloat("linearFalloff : ", &currProps.linearFalloff))
			{
				isChanged = true;
			}
			if (ImGui::InputFloat("quadraticFalloff : ", &currProps.quadraticFalloff))
			{
				isChanged = true;
			}

			if (isChanged)
			{
				m_sceneManager->set_point_light(&currProps, selectedNode);
			}
			
		}
	}
	else
	{
		m_currentSelectedNode = -1;
	}
	bool sunLightChanged = false;

	if (ImGui::CollapsingHeader("Scene Settings"))
	{
		ImGui::SliderFloat("Frustum Near Plane", &m_nearPlane, 0.0001f, 0.1f,"%.4f");
		ImGui::SliderFloat("Frustum Far Plane", &m_farPlane, 20.f, 400.f, "%.2f");

		ImGui::SliderFloat("LOD Base", &m_lodBase, 10.f, 100.f, "%.2f");
		ImGui::SliderFloat("LOD Step", &m_lodStep, 1.01f, 2.f, "%.3f");

		bool f = true;
		bool ft = false;
		
		ImGui::Checkbox("Convert Textures With KTX Optimizer", &f);
		ImGui::Checkbox("Scale Down Material Textures", &f);
		ImGui::Checkbox("Automized LODs", &f);
		ImGui::Checkbox("Sloppy LODs Enabled", &f);


		if (ImGui::InputFloat3("Sun Direction", sunProps.sunLightDirection))
		{
			sunLightChanged = true;
		}
		if (ImGui::InputFloat3("Sun Color", sunProps.sunLightColor))
		{
			sunLightChanged = true;
		}
		if (ImGui::InputFloat("Sun Intensity", &sunProps.sunIntensity))
		{
			sunLightChanged = true;
		}

		
		if (ImGui::Button("Add Default Light"))
		{
			m_sceneManager->add_point_light_node();
		}
		auto mode = renderer->get_current_material_mode();
		static uint32_t selectedMode = 0;
		uint32_t prev = selectedMode;
		switch (mode)
		{
		case MATERIAL_MODE_BLINN_PHONG:
			ImGui::Text("Current material mode is BLINN PHONG");
			selectedMode = 0;
			break;
		case MATERIAL_MODE_PBR:
			ImGui::Text("Current material mode is PBR");
			selectedMode = 1;
			break;
		default:
			ImGui::Text("UNKNOWN MATERIAL MODE ASSERT");
			break;
		}
		const char* items[] = {"BLINN PHONG","PBR"};
		if (ImGui::BeginCombo("##MATERIAL_MODE_SELECTION", items[selectedMode]))
		{
			for (int n = 0; n < IM_ARRAYSIZE(items); n++)
			{
				bool isSelected = (selectedMode == n); // You can store your selection however you want, outside or inside your objects
				if (ImGui::Selectable(items[n], isSelected))
					selectedMode = n;
				if (isSelected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}

		if (prev != selectedMode)
		{
			renderer->set_material_mode((MATERIAL_MODE)selectedMode);
		}
		/*ImGui::Checkbox("Wireframe Pipeline", &f);
		ImGui::SliderFloat("Wireframe Thickness", &wireframeSpec->thickness, 0.01f, 1.f, "%.2f");
		ImGui::SliderFloat("Wireframe Step", &wireframeSpec->step, 0.1f, 10.f, "%.3f");
		ImGui::Spacing();
		ImGui::Spacing();

		ImGui::Checkbox("Occlusion culling (Not stabilized)", &ft);*/

	}

	m_frustrumMatrix = glm::perspective(70.f, 16.f / 9.f, m_nearPlane, m_farPlane);
	m_cam->update_frustrum_proj_matrix(m_frustrumMatrix);
	cullData->lodBase = m_lodBase;
	cullData->lodStep = m_lodStep;
	auto cullEnabled = m_sceneManager->is_cull_enabled();
	if(ImGui::Checkbox("Cull Enabled", &cullEnabled))
	{
		m_sceneManager->set_cull_enabled(cullEnabled);
	}

	if (sunLightChanged)
	{
		m_sceneManager->update_sun_properties(&sunProps);
	}
}

void GImGuiPropertiesWindow::on_resize()
{
}

void GImGuiPropertiesWindow::on_data_update()
{
}

void GImGuiPropertiesWindow::destroy()
{
}

const char* GImGuiPropertiesWindow::get_window_name()
{
	return m_name.c_str();
}
