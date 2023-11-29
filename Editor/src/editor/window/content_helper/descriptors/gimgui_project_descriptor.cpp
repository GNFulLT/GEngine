#include "internal/window/content_helper/descriptors/gimgui_project_descriptor.h"
#include <imgui/imgui.h>
#include <filesystem>
#include <spdlog/fmt/fmt.h>
#include <fstream>
#include "editor/editor_application_impl.h"
#include "internal/manager/gproject_manager.h"
#include "engine/gproject.h"
#include <imgui/imgui_internal.h>
#include "internal/imgui_layer.h"
#include "internal/imgui_window_manager.h"
#include "internal/modal/gimgui_function_modal.h"

GImGuiProjectDescriptor::GImGuiProjectDescriptor()
{
	m_supportedFiles.push_back(".");
	m_supportedFiles.push_back(".gproject");

	memset(buf, 0, 50);
}

const std::vector<std::string>* GImGuiProjectDescriptor::get_file_types()
{
	return &m_supportedFiles;
}

void GImGuiProjectDescriptor::draw_menu_for_file(std::filesystem::path path)
{
	auto projManager = EditorApplicationImpl::get_instance()->get_project_manager();
	if (m_scriptCreation)
	{
	}

	if (std::filesystem::is_regular_file(path))
	{
		if (ImGui::Selectable("Open GProject"))
		{
			projManager->open_project(path);
		}
		return;
	}
	if (ImGui::Selectable("Create Project"))
	{
		std::string projectName = "ExampleProject";
		
		auto exp = EditorApplicationImpl::get_instance()->get_project_manager()->create_project(path.string(), projectName);
		if (!exp.has_value())
		{
			//X TODO : ERR
			return;
		}
		auto proj = exp.value();
		EditorApplicationImpl::get_instance()->get_project_manager()->swap_selected_project(proj);
	}

	auto proj = projManager->get_selected_project();
	bool same = false;
	if (proj != nullptr)
	{
		auto ppath = proj->get_project_path();
		std::error_code err;
		auto relPath = std::filesystem::relative(path,ppath,err);
		
		if (relPath.string().rfind("..",0) != 0)
		{
			same = true;
			if (ImGui::Selectable("Create Script"))
			{
				EditorApplicationImpl::get_instance()->get_editor_layer()->get_window_manager()->add_modal_to_queue(new GImGuiFunctionModal("Create GScript", [&, projMng = projManager]() {

					ImGui::InputText("Class Name", buf, 50);
					if (ImGui::Button("Create"))
					{
						std::string className = buf;
						auto size = className.size();
						projMng->create_script(className);
						m_scriptCreation = false;
						ImGui::CloseCurrentPopup();
						memset(buf, 0, 50);
					}
					if (ImGui::Button("Close"))
					{
						m_scriptCreation = false;
						ImGui::CloseCurrentPopup();
					}					
					return m_scriptCreation;

				}));
				/*EditorApplicationImpl::get_instance()->get_editor_layer()->get_window_manager()->set_modal_setter([&,projMng = projManager]() {
					if (m_scriptCreation)
					{
						ImGui::OpenPopup("Create GScript");
						ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

					}
					if (ImGui::BeginPopupModal("Create GScript", nullptr,  ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
					{

						ImGui::InputText("Class Name", buf, 50);
						if (ImGui::Button("Create"))
						{
							std::string className = buf;
							auto size = className.size();
							projMng->create_script(className);
							m_scriptCreation = false;
							ImGui::CloseCurrentPopup();
							memset(buf, 0, 50);
						}
						if (ImGui::Button("Close"))
						{
							m_scriptCreation = false;
							ImGui::CloseCurrentPopup();
						}
						ImGui::EndPopup();
						return true;
					}
					return false;
				});*/
				m_scriptCreation = true; 
			}
		}
	}

	
}