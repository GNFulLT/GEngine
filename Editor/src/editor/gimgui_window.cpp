#include "internal/gimgui_window.h"
#include "editor/igimgui_window_impl.h"
#include <imgui/imgui.h>

GImGuiWindow::GImGuiWindow(IGImGuiWindowImpl* impl)
{
	m_impl = impl;
	m_storage.height = 0;
	m_storage.width = 0;
	m_windowId = 0;
}
bool GImGuiWindow::init()
{
	return m_impl->init();
}

void GImGuiWindow::render()
{
	m_windowId = ImGui::GetID(get_window_name());
	if (ImGui::Begin(get_window_name()))
	{
		m_impl->render();
		ImGui::End();
	}
}

bool GImGuiWindow::need_render()
{
	m_impl->set_storage(&m_storage);
	return m_impl->need_render();
}

const char* GImGuiWindow::get_window_name()
{
	return m_impl->get_window_name();
}

