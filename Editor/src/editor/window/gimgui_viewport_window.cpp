#include "internal/window/gimgui_viewport_window.h"
#include "imgui.h"
#include "engine/rendering/vulkan/ivulkan_viewport.h"
#include "volk.h"
#include "engine/rendering/vulkan/ivulkan_descriptor.h"
#include <algorithm>
#include "editor/editor_application_impl.h"
#include "engine/gengine.h"
#include "internal/imgui_layer.h"
#include "internal/window/gimgui_scene_window.h"
#include "engine/rendering/scene/scene.h"
#include "internal/imguizmo.h"
#include "engine/manager/igcamera_manager.h"
#include "engine/imanager_table.h"
#include "public/core/templates/shared_ptr.h"
#include <glm/ext.hpp>
GImGuiViewportWindow::GImGuiViewportWindow()
{
	m_name = "Viewport";
	m_initedTheViewportFirstTime = false;
}

void GImGuiViewportWindow::set_the_viewport(IGVulkanViewport* viewport)
{
	m_viewport = viewport;

	m_viewport->init(640, 320, VkFormat::VK_FORMAT_B8G8R8A8_UNORM);
	m_initedTheViewportFirstTime = true;
	
	return;
}

bool GImGuiViewportWindow::init()
{
	m_sceneWindow = EditorApplicationImpl::get_instance()->get_editor_layer()->get_scene_window();
	m_cameraManager = ((GSharedPtr<IGCameraManager>*)EditorApplicationImpl::get_instance()->m_engine->get_manager_table()->get_engine_manager_managed(ENGINE_MANAGER_CAMERA))->get();
	return true;
}

void GImGuiViewportWindow::set_storage(GImGuiWindowStorage* storage)
{
	m_storage = storage;
}

bool GImGuiViewportWindow::need_render()
{
	return true;
}

void GImGuiViewportWindow::render()
{
	ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
	int x = std::max(0, int(viewportPanelSize.x));
	int y = std::max(0, int(viewportPanelSize.y));
	auto t = m_viewport->get_descriptor();
	if (t != nullptr)
	{
		ImGui::Image(t->get_vk_descriptor(), ImVec2{ float(x),float(y) });
	}

	if (auto selectedNode = m_sceneWindow->get_selected_entity(); selectedNode != -1)
	{
		auto scene = EditorApplicationImpl::get_instance()->m_engine->get_global_scene();
		auto mtrx = scene->get_matrix_of(selectedNode);
		if (mtrx != nullptr)
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
			camProj[1][1] *= -1.0f;
			bool isChanged =  ImGuizmo::Manipulate(m_cameraManager->get_camera_view_matrix(),glm::value_ptr(camProj) , ImGuizmo::TRANSLATE,
				ImGuizmo::LOCAL, glm::value_ptr(*mtrx));

			if (isChanged)
			{
				scene->mark_as_changed(m_selectedNode);
			}
		}
	}
	else
	{
		m_selectedNode = -1;
	}
}

void GImGuiViewportWindow::on_resize()
{
	EditorApplicationImpl::get_instance()->m_engine->add_recreation([&]() {
		assert(m_storage->width > 0 && m_storage->height > 0);
		m_viewport->destroy(true);
		m_viewport->init(m_storage->width, m_storage->height, VkFormat::VK_FORMAT_B8G8R8A8_UNORM);
	});

	
}

void GImGuiViewportWindow::on_data_update()
{
}

const char* GImGuiViewportWindow::get_window_name()
{
	return m_name.c_str();
}

void GImGuiViewportWindow::destroy()
{
}
