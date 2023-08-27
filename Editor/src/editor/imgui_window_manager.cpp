#include "internal/imgui_window_manager.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"

ImGuiWindowManager::~ImGuiWindowManager()
{
	int a = 5;
}

bool ImGuiWindowManager::create_imgui_window(IGImGuiWindowImpl* impl, GIMGUIWINDOWDIR dir)
{
	std::lock_guard guard(m_windowCreationMutex);
	if (auto menu = m_windowMap.find(impl->get_window_name()); menu != m_windowMap.end())
		return false;

	GImGuiWindow* window = new GImGuiWindow(impl);
	bool inited = window->init();
	if (!inited)
	{
		//X TODO : CUSTOM DELETER
		delete window;
		return false;
	}
	window->set_dock_dir(dir);
	m_windowMap.emplace(impl->get_window_name(), window);
	m_windowVector.push_back(window);
	return true;

}

bool ImGuiWindowManager::create_imgui_menu(IGImGuiMenuImpl* impl)
{
	if (auto menu = m_menuMap.find(impl->get_menu_name()); menu != m_menuMap.end())
		return false;

	//X GDNEWDA
	GImGuiMenu* newMenu = new GImGuiMenu(impl);

	bool inited = newMenu->init();
	
	if (!inited)
	{
		//X TODO : CUSTOM DELETER
		delete newMenu;
		return false;
	}
	m_menuMap.emplace(impl->get_menu_name(),newMenu);
	m_menuVector.push_back(newMenu);

	return true;
}

bool ImGuiWindowManager::init()
{
	
	return true;
}

void ImGuiWindowManager::destroy()
{
	for (int i = 0; i<m_menuVector.size(); i++)
	{
		m_menuVector[i]->destroy();
		//X TODO : CUSTOM
		delete m_menuVector[i];
	}
	for (int i = 0; i < m_windowVector.size(); i++)
	{
		m_windowVector[i]->destroy();
		delete m_windowVector[i];
	}
}

void ImGuiWindowManager::render_windows()
{
	render_main_dockspace();
	ImGuiViewportP* viewport = (ImGuiViewportP*)(void*)ImGui::GetMainViewport();
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_MenuBar;
	
	draw_main_menu_bar();

	ImGui::ShowDemoWindow();

	for (int i = 0; i < m_windowVector.size(); i++)
	{
		if (!m_windowVector[i]->need_render())
			continue;

		m_windowVector[i]->render();
		

	}
	ImGui::Begin("Helloooo");
	ImGui::End();

	ImGui::Begin("HellooooB");
	ImGui::End();
}

void ImGuiWindowManager::render_main_menu()
{
	if (ImGui::BeginMainMenuBar())
	{
		
		ImGui::End();
	}
}

void ImGuiWindowManager::render_main_dockspace()
{
	m_dock_id = ImGui::GetID("##MainDockSpace");

	build_nodes();

	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking;
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
	window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
	ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->WorkPos);
	ImGui::SetNextWindowSize(viewport->WorkSize);
	ImGui::SetNextWindowViewport(viewport->ID);
	ImGui::Begin("DockSpace", nullptr, window_flags);
	ImGui::PopStyleVar(2);
	ImGui::DockSpace(m_dock_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_::ImGuiDockNodeFlags_NoResize
			| ImGuiDockNodeFlags_PassthruCentralNode);

	ImGui::End();
}

void ImGuiWindowManager::build_nodes()
{
	if (!m_isDockDirty)
		return;
	ImGui::DockBuilderRemoveNode(m_dock_id);
	
		ImGui::DockBuilderAddNode(m_dock_id, ImGuiDockNodeFlags_PassthruCentralNode | ImGuiDockNodeFlags_DockSpace);
		ImGui::DockBuilderSetNodeSize(m_dock_id, ImGui::GetMainViewport()->Size);
	
	
		dock_id_right = ImGui::DockBuilderSplitNode(m_dock_id, ImGuiDir_Right,
			0.3f, nullptr, &dock_id_left);
	
	
		dock_id_left = ImGui::DockBuilderSplitNode(dock_id_left, ImGuiDir_Left,
			0.2f, nullptr, &dock_id_middle);
		
		
		dock_id_bottom = ImGui::DockBuilderSplitNode(dock_id_middle, ImGuiDir_Down,
			0.2f, nullptr, &dock_id_middle);

		
		ImGui::DockBuilderDockWindow("Dear ImGui Demo", dock_id_right);
		ImGui::DockBuilderDockWindow("Helloooo", dock_id_left);
		ImGui::DockBuilderDockWindow("HellooooB", dock_id_bottom);

		for (const auto& win : m_windowVector)
		{
			if (win->wants_docking())
			{
				auto dir = win->get_dock_dir();
				int dirId = -1;
				switch (dir)
				{
				case GIMGUIWINDOWDIR_NONE:
					break;
				case GIMGUIWINDOWDIR_LEFT:
					dirId = dock_id_left;
					break;
				case GIMGUIWINDOWDIR_RIGHT:
					dirId = dock_id_right;
					break;
				case GIMGUIWINDOWDIR_MIDDLE:
					dirId = dock_id_middle;
					break;
				case GIMGUIWINDOWDIR_BOTTOM:
					dirId = dock_id_bottom;
					break;
				default:
					break;
				}
				if (dirId != -1)
				{
					ImGui::DockBuilderDockWindow(win->get_window_name(), dirId);

				}
			}
		}

		ImGui::DockBuilderFinish(m_dock_id);

		m_isDockDirty = false;
}

void ImGuiWindowManager::draw_main_menu_bar()
{
	if (ImGui::BeginMainMenuBar())
	{
		for (int i = 0; i < m_menuVector.size(); i++)
		{
			if (m_menuVector[i]->need_render())
			{
				m_menuVector[i]->render();
			}
		}
		ImGui::EndMainMenuBar();
	}
}
