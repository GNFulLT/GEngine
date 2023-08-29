#include "internal/window/gimgui_content_browser_window.h"
#include "editor/editor_application_impl.h"
#include "engine/gengine.h"
#include "engine/imanager_table.h"
#include "engine/manager/igresource_manager.h"
#include "engine/resource/igtexture_resource.h"
#include "internal/rendering/vulkan/gimgui_descriptor_creator.h"
#include "engine/io/iowning_glogger.h"
#include "imgui/imgui.h"
#include "engine/rendering/vulkan/ivulkan_descriptor.h"
#include "internal/utils.h"
#include "public/platform/window.h"
#include "public/platform/imouse_manager.h"

GImGuiContentBrowserWindow::GImGuiContentBrowserWindow()
{
	m_name = "ContentBrowser";
	m_currentPath = std::filesystem::current_path();
}

bool GImGuiContentBrowserWindow::init()
{
	IManagerTable* table = EditorApplicationImpl::get_instance()->m_engine->get_manager_table();
	GSharedPtr<IGResourceManager>* resManager = (GSharedPtr<IGResourceManager>*)table->get_engine_manager_managed(ENGINE_MANAGER_RESOURCE);

	auto window = (*(GSharedPtr<Window>*)table->get_engine_manager_managed(ENGINE_MANAGER_WINDOW)).get();
	m_mouse = window->get_mouse_manager();
	auto res = (*resManager)->create_texture_resource("FolderIcon", "EditorResources", "./assets/folder.png", EditorApplicationImpl::get_instance()->get_descriptor_creator());
	auto res2 = (*resManager)->create_texture_resource("TextIcon", "EditorResources", "./assets/txt.png", EditorApplicationImpl::get_instance()->get_descriptor_creator());

	if (!res.has_value())
	{
		EditorApplicationImpl::get_instance()->get_editor_logger()->log_c("Couldn't create the texture");
		return false;
	}
	if (!res2.has_value())
	{
		return false;
	}

	m_folderIcon = GSharedPtr<IGTextureResource>(res.value());
	m_txtIcon = GSharedPtr<IGTextureResource>(res2.value());

	RESOURCE_INIT_CODE code = m_folderIcon->load();

	if (code != RESOURCE_INIT_CODE_OK)
	{
		EditorApplicationImpl::get_instance()->get_editor_logger()->log_c("Couldn't load the texture");
		return false;
	}

	code = m_txtIcon->load();
	if (code != RESOURCE_INIT_CODE_OK)
	{
		EditorApplicationImpl::get_instance()->get_editor_logger()->log_c("Couldn't load the texture");
		return false;
	}

	return true;
}

void GImGuiContentBrowserWindow::set_storage(GImGuiWindowStorage* storage)
{
	m_storage = storage;
}

bool GImGuiContentBrowserWindow::need_render()
{
	return true;
}

void GImGuiContentBrowserWindow::render()
{
	static float padding = 64.f;
	static float btn_size = 32.f;
	float cell_size = padding + btn_size;
	int column_count =  m_storage->width / cell_size;
	if (column_count < 1)
		column_count = 1;

		ImGui::Columns(column_count, 0, false);


	using recursive_directory_iterator = std::filesystem::recursive_directory_iterator;
	std::filesystem::directory_entry entry;
	
	for (const auto& dirEntry : recursive_directory_iterator(m_currentPath))
	{
		VkDescriptorSet_T* desc = nullptr;
		auto str = dirEntry.path().filename().string();
		
		FILE_TYPE fileType = get_file_type_from_name(str.c_str());
		
		switch (fileType)
		{
		case FILE_TYPE_FOLDER:
			desc = m_folderIcon->get_descriptor_set()->get_vk_descriptor();
			break;
		case FILE_TYPE_TXT:
			desc = m_txtIcon->get_descriptor_set()->get_vk_descriptor();
			break;
		default:
			desc = m_txtIcon->get_descriptor_set()->get_vk_descriptor();
			break;
		}

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0.2f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0.3f));
		

		ImGui::ImageButton(desc, ImVec2{ btn_size,btn_size },{0,0},{1,1},btn_size/2);


		ImGui::PopStyleColor(3);

		if (ImGui::IsItemHovered())
		{
			entry = dirEntry;
		}

		auto textWidth = ImGui::CalcTextSize(str.c_str()).x;

		auto textBegin = ImGui::GetCursorPosX() + ((btn_size - textWidth) * 0.5f);
		if (textBegin >= ImGui::GetCursorPosX())
			ImGui::SetCursorPosX(textBegin);

		ImGui::Text(str.c_str());
		ImGui::NextColumn();
		
	}

	if (m_mouse->get_mouse_button_state(MOUSE_BUTTON_RIGHT) && entry.exists())
	{
		m_rightClickedFile = entry.path();
		
		ImGui::OpenPopup("file_popup",0);

	}

	if (ImGui::BeginPopup("file_popup"))
	{
		if (ImGui::Selectable("Open in text editor"))
		{
			EditorApplicationImpl::get_instance()->get_editor_logger()->log_d("Selected");
		}
		ImGui::EndPopup();
	}
	
}

void GImGuiContentBrowserWindow::on_resize()
{
}

void GImGuiContentBrowserWindow::on_data_update()
{
}

const char* GImGuiContentBrowserWindow::get_window_name()
{
	return m_name.c_str();
}

void GImGuiContentBrowserWindow::destroy()
{
	if (m_folderIcon.is_valid())
	{
		m_folderIcon->destroy();
		m_folderIcon = GSharedPtr<IGTextureResource>();
	}
	if (m_txtIcon.is_valid())
	{ 
		m_txtIcon->destroy();
		m_txtIcon = GSharedPtr<IGTextureResource>();
	}

}
