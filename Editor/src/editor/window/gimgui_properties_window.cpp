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
	auto wireframeSpec = EditorApplicationImpl::get_instance()->m_engine->get_wireframe_spec();
	if (auto selectedNode = m_sceneWindow->get_selected_entity();selectedNode != -1)
	{
		auto scene=  EditorApplicationImpl::get_instance()->m_engine->get_global_scene();
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
	}
	else
	{
		m_currentSelectedNode = -1;
	}

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
		
		ImGui::Checkbox("Wireframe Pipeline", &f);
		ImGui::SliderFloat("Wireframe Thickness", &wireframeSpec->thickness, 0.01f, 1.f, "%.2f");
		ImGui::SliderFloat("Wireframe Step", &wireframeSpec->step, 0.1f, 10.f, "%.3f");
		ImGui::Spacing();
		ImGui::Spacing();

		ImGui::Checkbox("Occlusion culling (Not stabilized)", &ft);

	}

	m_frustrumMatrix = glm::perspective(70.f, 16.f / 9.f, m_nearPlane, m_farPlane);
	m_cam->update_frustrum_proj_matrix(m_frustrumMatrix);
	cullData->lodBase = m_lodBase;
	cullData->lodStep = m_lodStep;
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
