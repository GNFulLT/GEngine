#include "internal/imgui_window_manager.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"



bool ImGuiWindowManager::create_imgui_window(IGImGuiWindowImpl* impl, GIMGUIWINDOWDIR dir)
{
	std::lock_guard guard(m_windowCreationMutex);
	
	//X TODO : GDNEWDA
	assert(false);
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
	for (int i = 0; m_menuVector.size(); i++)
	{
		m_menuVector[i]->destroy();
	}
}

void ImGuiWindowManager::render_windows()
{
	render_main_dockspace();
	ImGuiViewportP* viewport = (ImGuiViewportP*)(void*)ImGui::GetMainViewport();
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_MenuBar;
	
	draw_main_menu_bar();

	ImGui::ShowDemoWindow();

	ImGui::Begin("Helloooo");
	ImGui::End();

	ImGui::Begin("HellooooB");
	ImGui::End();

	ImGui::Begin("HellooooL");
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
		ImGui::DockBuilderDockWindow("HellooooL", dock_id_middle);

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
