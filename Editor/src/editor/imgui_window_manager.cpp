#include "internal/imgui_window_manager.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "engine/globals.h"
#include "engine/imanager_table.h"
#include "editor/editor_application_impl.h"
#include "engine/gengine.h"
#include "engine/manager/igresource_manager.h"
#include "internal/rendering/vulkan/gimgui_descriptor_creator.h"
#include "engine/resource/igtexture_resource.h"
#include "engine/rendering/vulkan/ivulkan_descriptor.h"
#include "public/platform/window.h"
#include "public/platform/imouse_manager.h"
#include "editor/editor_application_impl.h"
#include "engine/io/iowning_glogger.h"
#include <spdlog/fmt/fmt.h>
#include "internal/window/gimgui_texteditor_window.h"
#include "internal/utils.h"
#include "imgui/imgui_internal.h"
ImGuiWindowManager::~ImGuiWindowManager()
{
	int a = 5;
}

bool ImGuiWindowManager::create_imgui_window(IGImGuiWindowImpl* impl, GIMGUIWINDOWDIR dir)
{
	std::lock_guard guard(m_windowCreationMutex);
	if (auto menu = m_windowMap.find(GImGuiWindow::generate_id_for_impl(impl)); menu != m_windowMap.end())
	{
		ImGui::SetWindowFocus(menu->second->get_window_name());
		return false;
	}

	GImGuiWindow* window = new GImGuiWindow(impl);
	bool inited = window->init();
	if (!inited)
	{
		//X TODO : CUSTOM DELETER
		delete window;
		return false;
	}
	window->set_dock_dir(dir);
	safe_add_window(window);
	if (!m_isDockDirty)
	{
		dock_the_window_if_needs(window);
	}
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
	auto table = EditorApplicationImpl::get_instance()->m_engine->get_manager_table();
	auto resourceManager = (GSharedPtr<IGResourceManager>*)table->get_engine_manager_managed(ENGINE_MANAGER_RESOURCE);
	m_window = (*(GSharedPtr<Window>*)table->get_engine_manager_managed(ENGINE_MANAGER_WINDOW)).get();
	auto res  = (*resourceManager)->create_texture_resource("MaximizeMinimize","EditorResources","assets/squares.png",EditorApplicationImpl::get_instance()->get_descriptor_creator());
	auto res2 = (*resourceManager)->create_texture_resource("Minus", "EditorResources", "assets/minus.png", EditorApplicationImpl::get_instance()->get_descriptor_creator());
	auto res3 = (*resourceManager)->create_texture_resource("XIcon", "EditorResources", "assets/x.png", EditorApplicationImpl::get_instance()->get_descriptor_creator());
	auto res4 = (*resourceManager)->create_texture_resource("EditorIcon", "EditorResources", "assets/GEngine.png", EditorApplicationImpl::get_instance()->get_descriptor_creator(),43);
	if (res.has_value())
	{
		m_smallBiggerTexture = GSharedPtr<IGTextureResource>(res.value());
		m_smallBiggerTexture->load();
	}
	if (res2.has_value())
	{
		m_minusTexture = GSharedPtr<IGTextureResource>(res2.value());
		m_minusTexture->load();
	}
	if (res3.has_value())
	{
		m_xTexture = GSharedPtr<IGTextureResource>(res3.value());
		m_xTexture->load();
	}
	if (res4.has_value())
	{
		m_editorIcon = GSharedPtr<IGTextureResource>(res4.value());
		m_editorIcon->load();
	}
	return true;
}

void ImGuiWindowManager::destroy()
{
	if (m_smallBiggerTexture.is_valid())
	{
		m_smallBiggerTexture->destroy();
	}
	if (m_xTexture.is_valid())
	{
		m_xTexture->destroy();
	}
	if (m_minusTexture.is_valid())
	{
		m_minusTexture->destroy();
	}
	if (m_editorIcon.is_valid())
	{
		m_editorIcon->destroy();
	}

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
	before_render();

	m_isInRender.store(true);
	render_main_dockspace();
	ImGuiViewportP* viewport = (ImGuiViewportP*)(void*)ImGui::GetMainViewport();
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_MenuBar;
	
	draw_main_menu_bar();

	ImGui::ShowDemoWindow();

	auto iterator = m_windowVector.begin();

	while (iterator != m_windowVector.end())
	{
		if ((*iterator)->wants_destroy())
		{
			if (auto win2 = m_windowMap.find((*iterator)->get_window_name()); win2 != m_windowMap.end())
			{
				m_windowMap.erase(win2);
			}
			(*iterator)->destroy();
			delete (*iterator);
			iterator = m_windowVector.erase(iterator);
			continue;
		}
		if (!(*iterator)->need_render())
		{
			iterator++;
			continue;
		}

		(*iterator)->render();
		iterator++;
		
	}

	ImGui::Begin("Helloooo");
	ImGui::End();

	ImGui::Begin("HellooooB");
	ImGui::End();

	m_isInRender.store(false);
}

void ImGuiWindowManager::render_main_menu()
{
	if (ImGui::BeginMainMenuBar())
	{
		
		ImGui::End();
	}
}

void ImGuiWindowManager::try_to_open_file_in_new_editor(std::filesystem::path path)
{
	//X TODO CHECK IS THERE ANY OPENED WINDOW ALREADY FOR THIS WINDOW
	FILE_TYPE type = get_file_type_from_name(path.filename().string().c_str());
	if (GImGuiTextEditorWindow::can_open(type))
	{
		auto win = new GImGuiTextEditorWindow(path, type,false);
		bool created = create_imgui_window(win,GIMGUIWINDOWDIR_RIGHT);
		if (!created)
			delete win;
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
	ImGui::DockSpace(m_dock_id, ImVec2(0.0f, 0.0f),
			ImGuiDockNodeFlags_PassthruCentralNode);

	ImGui::End();
}

void ImGuiWindowManager::before_render()
{
	while (!m_waitingWindows.empty())
	{
		auto windowAndOp = m_waitingWindows.front();
		auto window = windowAndOp.first;
		m_waitingWindows.pop();
		if (windowAndOp.second == WINDOW_OP_ADD)
		{
			unsafe_add_window(window);
		}
		else
		{
			extract_window(window);
		}
	}
}

void ImGuiWindowManager::build_nodes()
{
	if (!m_isDockDirty)
		return;
	ImGui::DockBuilderRemoveNode(m_dock_id);
	
		ImGui::DockBuilderAddNode(m_dock_id, ImGuiDockNodeFlags_PassthruCentralNode | ImGuiDockNodeFlags_DockSpace);
		ImGui::DockBuilderSetNodeSize(m_dock_id, ImGui::GetMainViewport()->Size);
	
	
		dock_id_right = ImGui::DockBuilderSplitNode(m_dock_id, ImGuiDir_Right,
			0.3f, nullptr, &dock_id_left_top);
	
	
		dock_id_left_top = ImGui::DockBuilderSplitNode(dock_id_left_top, ImGuiDir_Left,
			0.3f, nullptr, &dock_id_middle);
		
		dock_id_left_top = ImGui::DockBuilderSplitNode(dock_id_left_top, ImGuiDir_Up,0.6,nullptr,&dock_id_left_bottom);

		
		dock_id_bottom = ImGui::DockBuilderSplitNode(dock_id_middle, ImGuiDir_Down,
			0.2f, nullptr, &dock_id_middle);

		
		ImGui::DockBuilderDockWindow("Dear ImGui Demo", dock_id_right);
		ImGui::DockBuilderDockWindow("Helloooo", dock_id_left_top);
		ImGui::DockBuilderDockWindow("HellooooB", dock_id_bottom);

		for (const auto& win : m_windowVector)
		{
			dock_the_window_if_needs(win);
		}

		ImGui::DockBuilderFinish(m_dock_id);

		m_isDockDirty = false;
}

void ImGuiWindowManager::draw_main_menu_bar()
{
	IMouseManager* mouse = m_window->get_mouse_manager();
	bool isClicking = mouse->get_mouse_button_state(MOUSE_BUTTON_LEFT);
	bool isInsideMainBar = false;
	auto pos = mouse->get_mouse_pos();
	ImGui::PushStyleVar(ImGuiStyleVar_::ImGuiStyleVar_FramePadding, ImVec2(ImGui::GetStyle().FramePadding.x,16.f));
	if (ImGui::BeginMainMenuBar())
	{
		auto windowSize = ImGui::GetWindowSize();
		if (m_isDragging)
		{
			offset_cpx = pos.first - cp_x;
			offset_cpy = pos.second - cp_y;
		}
		
		// First draw the icon

		int buttonsBegin = ImGui::GetWindowSize().x - (19 * m_buttonWidth) / 6;

		ImGui::SameLine(buttonsBegin);
		ImGui::SetCursorPosY(0.f);
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));

		ImGui::PushStyleColor(ImGuiCol_::ImGuiCol_Button, {0,0,0,0});
		// Right Icons
		// Draw minimize button
		if (m_minusTexture.is_valid() && m_minusTexture->get_resource_state() == RESOURCE_LOADING_STATE_LOADED)
		{
			ImGui::PushStyleVar(ImGuiStyleVar_::ImGuiStyleVar_FramePadding, { (m_buttonWidth - 16.f) / 2.f,8.f });
			if (ImGui::ImageButton(m_minusTexture->get_descriptor_set()->get_vk_descriptor(), { 16.f,16.f }))
			{
				isInsideMainBar = false;
			}
			ImGui::PopStyleVar();

		}
		else
		{
			if (ImGui::Button("-", { m_buttonWidth,0 }))
			{
				isInsideMainBar = false;
			}
		}
	
		ImGui::SetCursorPosY(0.f);
		if (m_smallBiggerTexture.is_valid() && m_smallBiggerTexture->get_resource_state() == RESOURCE_LOADING_STATE_LOADED)
		{
			ImGui::PushStyleVar(ImGuiStyleVar_::ImGuiStyleVar_FramePadding, { (m_buttonWidth - 16.f) / 2.f,8.f });
			if (ImGui::ImageButton(m_smallBiggerTexture->get_descriptor_set()->get_vk_descriptor(), { 16.f,16.f }))
			{
				isInsideMainBar = false;

				if (m_window->get_window_mode() == WINDOW_MODE_WINDOWED_FULLSCREEN)
				{
					m_window->restore();
				}
				else if(m_window->get_window_mode() == WINDOW_MODE_WINDOWED)
				{
					m_window->maximize();
				}
				else
				{
					int a = 5;
				}
			}
			ImGui::PopStyleVar();
		}
		else
		{
			if (ImGui::Button("A", { m_buttonWidth,0 }))
			{
				isInsideMainBar = false;

				if (m_window->get_window_mode() == WINDOW_MODE_WINDOWED_FULLSCREEN)
				{
					m_window->restore();
				}
				else if (m_window->get_window_mode() == WINDOW_MODE_WINDOWED)
				{
					m_window->maximize();
				}
				else
				{
					int a = 5;
				}
			}
		}
		ImGui::SetCursorPosY(0.f);
		if (m_xTexture.is_valid() && m_xTexture->get_resource_state() == RESOURCE_LOADING_STATE_LOADED)
		{
			ImGui::PushStyleVar(ImGuiStyleVar_::ImGuiStyleVar_FramePadding, { (m_buttonWidth - 16.f) / 2.f,8.f });
			if (ImGui::ImageButton(m_xTexture->get_descriptor_set()->get_vk_descriptor(), { 16.f,16.f }))
			{
				isInsideMainBar = false;

				request_exit();
			}
			ImGui::PopStyleVar();
		}
		else
		{
			if (ImGui::Button("X", { m_buttonWidth,0 }))
			{
				isInsideMainBar = false;

				request_exit();
			}
		}

		
		
		ImGui::PopStyleColor();
		ImGui::PopStyleVar();
		int gap = 0;
		if (m_editorIcon.is_valid() && m_editorIcon->get_resource_state() == RESOURCE_LOADING_STATE_LOADED)
		{
			ImGui::SameLine(-32.f);
			ImGui::Spacing();
			ImGui::SetCursorPosY(-32.f);
			
			ImGui::Image(m_editorIcon->get_descriptor_set()->get_vk_descriptor(), {162.f,129.f});

		}
		


		ImGui::Spacing();
		ImGui::SetCursorPosY(24.f);
		int iconBegin = ImGui::GetCursorPosX();

		for (int i = 0; i < m_menuVector.size(); i++)
		{
			if (m_menuVector[i]->need_render())
			{
				m_menuVector[i]->render();
			}
		}

		int yPos = ImGui::GetCursorPosY();


		int lastMenu = ImGui::GetCursorPosX();

		if (pos.first > iconBegin && pos.first < buttonsBegin)
		{
			if (pos.first > lastMenu && pos.first < windowSize.x)
			{
				isInsideMainBar = pos.second > 0.f && pos.second < windowSize.y;
			}
			else
				isInsideMainBar = pos.second > 0.f && pos.second < 24.f;
		}


		ImGui::EndMainMenuBar();
	}
	ImGui::PopStyleVar();

	// Movement

	if (isClicking)
	{
		if (isInsideMainBar)
		{

			if (!m_isDragging)
			{
				if (!m_isClickedOutside)
				{
					cp_x = pos.first;
					cp_y = pos.second;
					m_isDragging = true;
				}
				// It is first clickig state just get the prev
			
			}
			else
			{
				if (m_isDragging)
				{
					m_window->move_by(offset_cpx, offset_cpy);
					offset_cpx = 0;
					offset_cpy = 0;
					cp_x += offset_cpx;
					cp_y += offset_cpy;
				}
			}
		}
		else
		{
			m_isClickedOutside = true;
		}

	}
	else
	{
		m_isClickedOutside = false;
		m_isDragging = false;
		offset_cpx = 0;
		offset_cpy = 0;
		cp_x = 0;
		cp_y = 0;
	}
}

void ImGuiWindowManager::dock_the_window_if_needs(GImGuiWindow* win)
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
			dirId = dock_id_left_top;
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

void ImGuiWindowManager::extract_window(GImGuiWindow* win)
{
	if (auto win2 = m_windowMap.find(win->get_window_name()); win2 != m_windowMap.end())
	{
		m_windowMap.erase(win2);
		auto iter = m_windowVector.begin();
		while (iter != m_windowVector.end())
		{
			if (0 == strcmp((*iter)->get_window_name(), win->get_window_name()))
			{
				m_windowVector.erase(iter);
				break;
			}
			iter++;
		}
		
	}
}

void ImGuiWindowManager::safe_add_window(GImGuiWindow* win)
{
	if (m_isInRender.load())
	{
		m_waitingWindows.push({win,WINDOW_OP_ADD});
	}
	else
	{
		unsafe_add_window(win);
	}
}

void ImGuiWindowManager::unsafe_add_window(GImGuiWindow* window)
{
	m_windowMap.emplace(window->get_window_name(), window);
	m_windowVector.push_back(window);
}

void ImGuiWindowManager::safe_extract_window(GImGuiWindow* win)
{
	if (m_isInRender.load())
	{
		m_waitingWindows.push({ win,WINDOW_OP_EXTRACT });
	}
	else
	{
		extract_window(win);
	}
}
