#include "internal/imgui_window_manager.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"



void ImGuiWindowManager::create_imgui_window(IGImGuiWindowImpl* impl, GIMGUIWINDOWDIR dir)
{
	std::lock_guard guard(m_windowCreationMutex);
	
	//X TODO : GDNEWDA
	assert(false);

}

bool ImGuiWindowManager::init()
{
	
	return true;
}

void ImGuiWindowManager::destroy()
{
}

void ImGuiWindowManager::render_windows()
{
	render_main_dockspace();
	ImGuiViewportP* viewport = (ImGuiViewportP*)(void*)ImGui::GetMainViewport();
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_MenuBar;

	if (ImGui::BeginViewportSideBar("Leftim", viewport, ImGuiDir_Left, 10, window_flags))
	{
		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("Logging"))
			{

				ImGui::EndMenu();
			}
			ImGui::EndMenuBar();
		}

		ImGui::End();
	}


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
