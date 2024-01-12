#include "internal/menu/gproject_menu.h"
#include "imgui/imgui.h"
#include "engine/globals.h"
#include "editor/editor_application_impl.h"
#include "internal/imgui_layer.h"
#include "internal/modal/gimgui_function_modal.h"
#include "internal/imgui_window_manager.h"

bool GProjectMenu::init()
{
	m_menuName = "Project";
	return true;
}

bool GProjectMenu::need_render()
{
	return true;
}

void GProjectMenu::render()
{
	if (ImGui::MenuItem("Open Project"))
	{

	}
	if (ImGui::MenuItem("Save Project"))
	{
		IGProjectManager* projManager = EditorApplicationImpl::get_instance()->get_project_manager();
		if (projManager->get_selected_project())
		{
			projManager->save_project(projManager->get_selected_project());
		}
	}
	if (ImGui::MenuItem("Create Project"))
	{
		IGProjectManager* projManager = EditorApplicationImpl::get_instance()->get_project_manager();

		EditorApplicationImpl::get_instance()->get_editor_layer()->get_window_manager()->add_modal_to_queue(new GImGuiFunctionModal("Create Project", [&, projMng = projManager,buf = std::vector<char>(50)]() {
			auto chr = (char*)&buf[0];
			ImGui::InputText("Class Name",chr , 50);
				if (ImGui::Button("Create"))
				{
					std::string className = buf.data();
					auto size = className.size();
					auto currPath = std::filesystem::current_path() / className;

					projMng->create_project(currPath.string(), className);
					return false;
					
				}
				if (ImGui::Button("Close"))
				{
					return false;
				}
				return true;
			}));
	}
	if (ImGui::MenuItem("Build Project(Debug)"))
	{
		IGProjectManager* projManager = EditorApplicationImpl::get_instance()->get_project_manager();
		if (projManager->get_selected_project())
		{
			auto res = projManager->build_script(true);
		}
	}
	if (ImGui::MenuItem("Run"))
	{
		enable_update();
	}
	if (ImGui::MenuItem("Stop"))
	{
		disable_update();
	}
}

void GProjectMenu::on_resize()
{
}

void GProjectMenu::on_data_update()
{
}

const char* GProjectMenu::get_menu_name()
{
	return m_menuName.c_str();
}

void GProjectMenu::destroy()
{
}
