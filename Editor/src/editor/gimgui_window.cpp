#include "internal/gimgui_window.h"
#include "editor/igimgui_window_impl.h"
#include <imgui/imgui.h>
#include "editor/editor_application_impl.h"
#include "engine/io/iowning_glogger.h"
GImGuiWindow::GImGuiWindow(IGImGuiWindowImpl* impl)
{
	m_impl = impl;
	m_storage.height = 0;
	m_storage.width = 0;
	m_windowId = 0;
}
bool GImGuiWindow::init()
{
	bool init =  m_impl->init();
	if (!init)
		return false;
	m_id = m_impl->get_window_name();
	if (m_impl->can_open_multiple())
	{
		m_id += "##";
		if (m_impl->get_window_id() == nullptr)
		{
			m_impl->destroy();
			return false;
		}
		m_id += m_impl->get_window_id();
	}
	return true;
}

void GImGuiWindow::render()
{
	
	if (ImGui::Begin(get_window_name(),&m_isShown,m_impl->get_flag()))
	{
		ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
		if (viewportPanelSize.x != m_storage.width || viewportPanelSize.y != m_storage.height)
		{
			
			if (m_storage.width == 0 && m_storage.height == 0)
			{
				m_storage.width = viewportPanelSize.x;
				m_storage.height = viewportPanelSize.y;
			}
			else
			{
				EditorApplicationImpl::get_instance()->get_editor_logger()->log_d("On Resize");
				m_storage.width = viewportPanelSize.x;
				m_storage.height = viewportPanelSize.y;
				m_impl->on_resize();
			}
		}
		m_impl->render();

		m_focused = ImGui::IsWindowFocused();
	}
	ImGui::End();

}

bool GImGuiWindow::need_render()
{
	m_impl->set_storage(&m_storage);
	return m_impl->need_render();
}

std::string GImGuiWindow::generate_id_for_impl(IGImGuiWindowImpl* impl)
{
	std::string m_id = impl->get_window_name();
	if (impl->can_open_multiple())
	{
		if (impl->get_window_id() == nullptr)
		{
			return m_id;
		}
		m_id += "##";
		m_id += impl->get_window_id();
	}
	return m_id;
}

const char* GImGuiWindow::get_window_name()
{
	return m_id.c_str();
}


void GImGuiWindow::destroy()
{
	m_impl->destroy();
	delete m_impl;
}

void GImGuiWindow::set_dock_dir(GIMGUIWINDOWDIR dir)
{
	m_dockDir = dir;
}

bool GImGuiWindow::wants_docking()
{
	return m_dockDir != GIMGUIWINDOWDIR_NONE;
}

bool GImGuiWindow::wants_destroy()
{
	return !m_isShown;
}

bool GImGuiWindow::is_focused() const noexcept
{
	return m_focused;
}

GIMGUIWINDOWDIR GImGuiWindow::get_dock_dir()
{
	return m_dockDir;
}

