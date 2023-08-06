#include "internal/gimgui_menu.h"

#include "imgui/imgui.h"

GImGuiMenu::GImGuiMenu(IGImGuiMenuImpl* impl)
{
	m_impl = impl;
	m_menuId = 0;
}

bool GImGuiMenu::init()
{
	return m_impl->init();
}

void GImGuiMenu::render()
{
	m_menuId = ImGui::GetID(get_menu_name());
	auto name = get_menu_name();
	if (ImGui::BeginMenu(name))
	{
		m_impl->render();
		ImGui::EndMenu();
	}
}

bool GImGuiMenu::need_render()
{
	return m_impl->need_render();
}

const char* GImGuiMenu::get_menu_name()
{
	return m_impl->get_menu_name();
}
