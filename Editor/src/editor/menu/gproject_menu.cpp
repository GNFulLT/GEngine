#include "internal/menu/gproject_menu.h"
#include "imgui/imgui.h"
#include "engine/globals.h"

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
