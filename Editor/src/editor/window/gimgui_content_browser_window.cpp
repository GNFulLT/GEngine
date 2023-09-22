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
#include "internal/imgui_layer.h"
#include "internal/imgui_window_manager.h"
#include <stack>
#include "internal/window/content_helper/descriptors/gimgui_texteditor_descriptor.h"
#include "internal/window/content_helper/descriptors/gimgui_shader_descriptor.h"
#include "internal/window/content_helper/descriptors/gimgui_spirv_descriptor.h"
#include "internal/window/content_helper/descriptors/gimgui_model_asset_descriptor.h"
#include "internal/window/content_helper/descriptors/gimgui_gmesh_descriptor.h"

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
	auto res3 = (*resManager)->create_texture_resource("HLSLIcon", "EditorResources", "./assets/hlsl.png", EditorApplicationImpl::get_instance()->get_descriptor_creator());
	auto res4 = (*resManager)->create_texture_resource("GLSLIcon", "EditorResources", "./assets/glsl.png", EditorApplicationImpl::get_instance()->get_descriptor_creator());
	auto res5 = (*resManager)->create_texture_resource("SPVIcon", "EditorResources", "./assets/spv.png", EditorApplicationImpl::get_instance()->get_descriptor_creator());

	if (!res.has_value())
	{
		EditorApplicationImpl::get_instance()->get_editor_logger()->log_c("Couldn't create the texture");
		return false;
	}
	if (!res2.has_value())
	{
		return false;
	}
	if (!res3.has_value() || !res4.has_value() || !res5.has_value())
	{
		return false;
	}
	
	m_folderIcon = GSharedPtr<IGTextureResource>(res.value());
	m_txtIcon = GSharedPtr<IGTextureResource>(res2.value());
	m_hlslIcon= GSharedPtr<IGTextureResource>(res3.value());
	m_glslIcon = GSharedPtr<IGTextureResource>(res4.value());
	m_spvIcon = GSharedPtr<IGTextureResource>(res5.value());

	//X TODO : Make parallel
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
	code = m_hlslIcon->load();
	if (code != RESOURCE_INIT_CODE_OK)
	{
		EditorApplicationImpl::get_instance()->get_editor_logger()->log_c("Couldn't load the texture");
		return false;
	}

	code = m_glslIcon->load();
	if (code != RESOURCE_INIT_CODE_OK)
	{
		EditorApplicationImpl::get_instance()->get_editor_logger()->log_c("Couldn't load the texture");
		return false;
	}
	code = m_spvIcon->load();
	if (code != RESOURCE_INIT_CODE_OK)
	{
		EditorApplicationImpl::get_instance()->get_editor_logger()->log_c("Couldn't load the texture");
		return false;
	}

	m_contentHelper.register_descriptor(new GImGuiTextEditorDescriptor());
	m_contentHelper.register_descriptor(new GImGuiShaderDescriptor());
	m_contentHelper.register_descriptor(new GImGuiSpirvDescriptor());
	m_contentHelper.register_descriptor(new GImGuiModelAssetDescriptor());
	m_contentHelper.register_descriptor(new GImGuiGMeshDescriptor());
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
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));

	auto iterPath = m_currentPath;
	std::stack<std::string> strStack;
	while (iterPath.has_parent_path() && iterPath.has_relative_path())
	{
		auto str = iterPath.filename().string();
		strStack.push(str+="/");
		iterPath = iterPath.parent_path();
	}

	if (m_currentPath.has_parent_path() && m_currentPath.has_relative_path())
	{
		ImGui::SameLine();
		if (ImGui::Button("<<"))
		{
			m_currentPath = m_currentPath.parent_path();
		}
	}
	

	while (!strStack.empty())
	{
		ImGui::SameLine();
		ImGui::Button(strStack.top().c_str());
		strStack.pop();
	}



	ImGui::PopStyleVar();

	static float padding = 64.f;
	static float btn_size = 40.f;
	static int frame = btn_size / 2;
	float cell_size = padding + btn_size;
	int column_count =  m_storage->width / cell_size;
	if (column_count < 1)
		column_count = 1;

	ImGui::Columns(column_count, 0, false);


	using directory_iterator = std::filesystem::directory_iterator;
	
	std::filesystem::directory_entry entry;
	std::filesystem::directory_entry pathEntry;
	FILE_TYPE selectedFile = FILE_TYPE_UNKNOWN;
	auto iter = m_currentPath;
	bool enteredFolder = false;
	for (const auto& dirEntry : directory_iterator(iter))
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
		case FILE_TYPE_HLSL:
			desc = m_hlslIcon->get_descriptor_set()->get_vk_descriptor();
			break;
		case FILE_TYPE_GLSL:
			desc = m_glslIcon->get_descriptor_set()->get_vk_descriptor();
			break;
		case FILE_TYPE_SPIRV:
			desc = m_spvIcon->get_descriptor_set()->get_vk_descriptor();
			break;
		default:
			desc = m_txtIcon->get_descriptor_set()->get_vk_descriptor();
			break;
		}

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0, 0, 0, 0.2f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0, 0, 0, 0.3f));
		

		if (ImGui::ImageButton(desc, ImVec2{ btn_size,btn_size }, { 0,0 }, { 1,1 }, frame));
		if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && fileType == FILE_TYPE_FOLDER && !enteredFolder)
		{
			enteredFolder = true;
		}
		


		ImGui::PopStyleColor(3);

		if (ImGui::IsItemHovered())
		{
			entry = dirEntry;
			selectedFile = fileType;
		}

		auto textWidth = ImGui::CalcTextSize(str.c_str()).x;

		auto textBegin = ImGui::GetCursorPosX() + ((btn_size+frame*2 - textWidth) * 0.5f);
		if (textBegin >= ImGui::GetCursorPosX())
			ImGui::SetCursorPosX(textBegin);

		ImGui::Text(str.c_str());
		ImGui::NextColumn();
		
	}
	

	if (m_mouse->get_mouse_button_state(MOUSE_BUTTON_RIGHT) && entry.exists())
	{
		auto descriptors = m_contentHelper.get_descriptor_of_type_if_any(selectedFile);
		if (descriptors != nullptr && descriptors->size() > 0)
		{
			m_rightClickedFile = entry.path();
			m_rightClickedFileType = selectedFile;
			ImGui::OpenPopup("file_popup", 0);
		}
	}

	if (enteredFolder)
	{
		auto pth = entry.path();
		if (std::filesystem::is_directory(pth))
		{
			m_currentPath = pth;
		}
	}

	if (ImGui::BeginPopup("file_popup"))
	{
		auto descriptors = m_contentHelper.get_descriptor_of_type_if_any(m_rightClickedFileType);
		if (descriptors != nullptr && descriptors->size() > 0)
		{
			for (auto descriptor : *descriptors)
			{
				descriptor->draw_menu_for_file(m_rightClickedFile);
			}
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
	m_contentHelper.destroy();
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
	if (m_hlslIcon.is_valid())
	{
		m_hlslIcon->destroy();
		m_hlslIcon = GSharedPtr<IGTextureResource>();
	}
	if (m_glslIcon.is_valid())
	{
		m_glslIcon->destroy();
		m_glslIcon = GSharedPtr<IGTextureResource>();
	}
	if (m_spvIcon.is_valid())
	{
		m_spvIcon->destroy();
		m_spvIcon = GSharedPtr<IGTextureResource>();
	}
}
