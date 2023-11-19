#include "internal/window/content_helper/descriptors/gimgui_project_descriptor.h"
#include <imgui/imgui.h>
#include <filesystem>
#include <spdlog/fmt/fmt.h>
#include <fstream>
#include "editor/editor_application_impl.h"
#include "internal/manager/gproject_manager.h"
#include "internal/gproject.h"

GImGuiProjectDescriptor::GImGuiProjectDescriptor()
{
	m_supportedFiles.push_back(".");
}

const std::vector<std::string>* GImGuiProjectDescriptor::get_file_types()
{
	return &m_supportedFiles;
}

void GImGuiProjectDescriptor::draw_menu_for_file(std::filesystem::path path)
{
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
	auto proj = EditorApplicationImpl::get_instance()->get_project_manager()->get_selected_project();
	if (proj != nullptr)
	{
		auto ppath = proj->get_project_path();
		if (strcmp(ppath, path.string().c_str()) == 0)
		{
			if (ImGui::Selectable("Create Script"))
			{

			}
		}
	}
}
