#include "internal/window/gimgui_material_window.h"
#include "engine/gengine.h"
#include "editor/editor_application_impl.h"
#include "imgui/imgui.h"
#include "spdlog/fmt/fmt.h"

GImGuiMaterialsWindow::GImGuiMaterialsWindow()
{
	m_name = "Materials";
}

bool GImGuiMaterialsWindow::init()
{
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
	/*MaterialDescription d;
	auto materials = EditorApplicationImpl::get_instance()->m_engine->get_global_materials();
	int i = 1;
	for (auto& material : *materials)
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

			}
			if (ImGui::CollapsingHeader("Normal Map"))
			{

			}
			if (ImGui::CollapsingHeader("Ambient Occlusion Map"))
			{

			}
			if (ImGui::CollapsingHeader("Emmisive Map"))
			{

			}
			if (ImGui::CollapsingHeader("Metallic Map"))
			{

			}
			
		}
		i++;
	}
	*/
	
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
