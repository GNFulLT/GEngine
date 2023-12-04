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
#include "engine/gengine.h"
#include "engine/imanager_table.h"
#include "engine/manager/igscript_manager.h"
#include "engine/plugin/igscript_space.h"

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
						memset(buf, 0, 50);
					}
					if (ImGui::Button("Close"))
					{
						m_scriptCreation = false;
					}					
					return m_scriptCreation;

				}));

				m_scriptCreation = true; 
			}
			if (ImGui::Selectable("Load Script"))
			{
				auto scriptPath = projManager->get_script_path(proj);
				auto outDllPath = scriptPath / "out" / "build" / "x64-Debug" / fmt::format("{}.dll",proj->get_project_name());
				if (std::filesystem::exists(outDllPath))
				{
					auto scriptManager = ((GSharedPtr<IGScriptManager>*)GEngine::get_instance()->get_manager_table()->get_engine_manager_managed(ENGINE_MANAGER_SCRIPT))->get();
					auto scriptSpace = scriptManager->load_script_space(outDllPath);
					if (!scriptSpace.has_value())
					{
						auto err = scriptSpace.error();
						if (err == GSCRIPT_SPACE_LOAD_ERROR_ALREADY_LOADED)
						{
							EditorApplicationImpl::get_instance()->get_editor_layer()->get_window_manager()->add_modal_to_queue(new GImGuiFunctionModal("Script Space is already loaded", [&, projMng = projManager]() {
								ImGui::Text("Do you want to reload script space ?");
								if (ImGui::Button("Yes"))
								{
									return false;
								}
								ImGui::SameLine();
								if (ImGui::Button("No"))
								{
									return false;
								}
								return true;
							}));
						}
					}
				}
			}
		}
	}

	
}
