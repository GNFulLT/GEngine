#include "internal/window/gimgui_material_window.h"
#include "engine/gengine.h"
#include "editor/editor_application_impl.h"
#include "imgui/imgui.h"
#include "spdlog/fmt/fmt.h"
#include "engine/imanager_table.h"
#include "engine/manager/igscene_manager.h"
#include <algorithm>
#include "internal/manager/geditor_texture_debug_manager.h"
GImGuiMaterialsWindow::GImGuiMaterialsWindow()
{
	m_name = "Materials";
}

bool GImGuiMaterialsWindow::init()
{
	m_sceneManager = ((GSharedPtr<IGSceneManager>*)EditorApplicationImpl::get_instance()->m_engine->get_manager_table()->get_engine_manager_managed(ENGINE_MANAGER_SCENE))->get();
	return true;
}

void GImGuiMaterialsWindow::set_storage(GImGuiWindowStorage* storage)
{
}

bool GImGuiMaterialsWindow::need_render()
{
	return true;
}

void GImGuiMaterialsWindow::render()
{
	MaterialDescription d;
	auto materials = m_sceneManager->get_current_scene_materials();
	auto textureDebugManager = EditorApplicationImpl::get_instance()->get_texture_debug_manager();

	int i = 0;
	for (auto& material : materials)
	{
		auto winName = fmt::format("Material #{} (A Debug name is not assigned)",i);
		if (ImGui::CollapsingHeader(winName.c_str()))
		{
			ImGui::InputFloat4("Emissive Color",&material.emissiveColor_.x);
			ImGui::InputFloat4("Albedo Color", &material.albedoColor_.x);
			ImGui::InputFloat("Occlusion", &material.occlusionFactor_);
			ImGui::InputFloat("Roughness", &material.roughnessFactor_);
			ImGui::InputFloat("Metallic Factor", &material.metallicFactor_);
			material.metallicFactor_ = std::clamp(material.metallicFactor_, 0.f, 1.f);
			ImGui::InputFloat("Transparency Factor", &material.transparencyFactor_);
			material.metallicFactor_ = std::clamp(material.metallicFactor_, 0.f, 1.f);
			ImGui::InputFloat("Alpha Test", &material.alphaTest_);
			material.alphaTest_ = std::clamp(material.alphaTest_, 0.f, 1.f);
			
			if (ImGui::CollapsingHeader("Albedo Map"))
			{
				if (material.albedoMap_ != UINT32_MAX)
				{
					auto texture = m_sceneManager->get_saved_texture_by_id(material.albedoMap_);
					if (texture != nullptr)
					{
						auto set = textureDebugManager->get_or_save_texture(texture);
						ImGui::Image(set, { 128,128 });
					}
				}
			}
			if (ImGui::CollapsingHeader("Normal Map"))
			{

			}
			if (ImGui::CollapsingHeader("Ambient Occlusion Map"))
			{
				auto texture = m_sceneManager->get_saved_texture_by_id(material.ambientOcclusionMap_);
				if (texture != nullptr)
				{
					auto set = textureDebugManager->get_or_save_texture(texture);
					ImGui::Image(set, { 128,128 });
				}
			}
			if (ImGui::CollapsingHeader("Emmisive Map"))
			{
				auto texture = m_sceneManager->get_saved_texture_by_id(material.emissiveMap_);
				if (texture != nullptr)
				{
					auto set = textureDebugManager->get_or_save_texture(texture);
					ImGui::Image(set, { 128,128 });
				}
			}
			if (ImGui::CollapsingHeader("MetallicRoughness Map"))
			{

			}
			if (ImGui::CollapsingHeader("Metallic Map"))
			{
				if (material.metallicyMap_ != UINT32_MAX)
				{
					auto texture = m_sceneManager->get_saved_texture_by_id(material.metallicyMap_);
					if (texture != nullptr)
					{
						auto set = textureDebugManager->get_or_save_texture(texture);
						ImGui::Image(set, { 128,128 });
					}
				}
			}
			if (ImGui::CollapsingHeader("Roughness Map"))
			{
				if (material.roughnessMap_ != UINT32_MAX)
				{
					auto texture = m_sceneManager->get_saved_texture_by_id(material.roughnessMap_);
					if (texture != nullptr)
					{
						auto set = textureDebugManager->get_or_save_texture(texture);
						ImGui::Image(set, { 128,128 });
					}
				}
			}
			if (ImGui::Button("Update Material"))
			{
				auto updatedMtr = material;
				m_sceneManager->set_material_by_index(&updatedMtr, i);
			}
		}
		i++;
	}
	
	
}

void GImGuiMaterialsWindow::on_resize()
{
}

void GImGuiMaterialsWindow::on_data_update()
{
}

void GImGuiMaterialsWindow::destroy()
{
}

const char* GImGuiMaterialsWindow::get_window_name()
{
	return m_name.c_str();
}
