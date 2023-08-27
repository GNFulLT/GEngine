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
	return m_impl->init();
}

void GImGuiWindow::render()
{
	if (ImGui::Begin(get_window_name()))
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
	}
	ImGui::End();

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

void GImGuiWindow::destroy()
{
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

GIMGUIWINDOWDIR GImGuiWindow::get_dock_dir()
{
	return m_dockDir;
}

