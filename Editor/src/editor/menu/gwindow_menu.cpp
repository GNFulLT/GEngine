#include "internal/menu/gwindow_menu.h"
#include <imgui/imgui.h>
GWindowMenu::GWindowMenu(ImGuiWindowManager* windowMng)
{
	m_name = "View";
	m_windowManager = windowMng;
}

bool GWindowMenu::register_provider(IGImGuiWindowProvider* provider)
{
	if (auto provIter = m_providerMap.find(provider->get_provider_name()); provIter != m_providerMap.end())
		return false;

	m_providerMap.emplace(provider->get_provider_name(), provider);
	m_providers.push_back(provider);
	return true;
}

bool GWindowMenu::init()
{
	return true;
}

bool GWindowMenu::need_render()
{
	return true;
}

void GWindowMenu::render()
{
	for (int i = 0; i < m_providers.size(); i++)
	{
		auto name = m_providers[i]->get_provider_name();;
		bool windowIsOpen = m_windowManager->get_window_if_exist(name);
		if (!windowIsOpen)
		{
			if (ImGui::Selectable(name))
			{
				auto impl = m_providers[i]->create_window_impl();
				if(!m_windowManager->create_imgui_window(impl,GIMGUIWINDOWDIR_LEFT))
					delete impl;
			}
		}
	}
}

void GWindowMenu::on_resize()
{
}

void GWindowMenu::on_data_update()
{
}

const char* GWindowMenu::get_menu_name()
{
	return m_name.c_str();
}

void GWindowMenu::destroy()
{
	for (int i = 0; i < m_providers.size(); i++)
	{
		delete m_providers[i];
	}
}
