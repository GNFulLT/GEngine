#include "internal/menu/gproject_menu.h"
#include "imgui/imgui.h"
#include "engine/globals.h"
#include "editor/editor_application_impl.h"
#include "internal/manager/gproject_manager.h"

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
		GProjectManager* projManager = EditorApplicationImpl::get_instance()->get_project_manager();
		if (projManager->get_selected_project())
		{
			projManager->save_project(projManager->get_selected_project());
		}
	}
	if (ImGui::MenuItem("Create Project"))
	{
		
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
