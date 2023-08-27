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
GImGuiContentBrowserWindow::GImGuiContentBrowserWindow()
{
	m_name = "ContentBrowser";
}

bool GImGuiContentBrowserWindow::init()
{
	IManagerTable* table = EditorApplicationImpl::get_instance()->m_engine->get_manager_table();
	GSharedPtr<IGResourceManager>* resManager = (GSharedPtr<IGResourceManager>*)table->get_engine_manager_managed(ENGINE_MANAGER_RESOURCE);
	
	auto res = (*resManager)->create_texture_resource("TextFileIcon", "EditorResources", "./text_icon.png", EditorApplicationImpl::get_instance()->get_descriptor_creator());

	if (!res.has_value())
	{
		EditorApplicationImpl::get_instance()->get_editor_logger()->log_c("Couldn't create the texture");
		return false;
	}

	m_textIconResource = GSharedPtr<IGTextureResource,GSHARED_PTR_INTERNAL_MODE_THREAD_SAFE>(res.value());


	RESOURCE_INIT_CODE code = m_textIconResource->load();

	if (code != RESOURCE_INIT_CODE_OK)
	{
		EditorApplicationImpl::get_instance()->get_editor_logger()->log_c("Couldn't load the texture");
		return false;
	}

	return true;
}

void GImGuiContentBrowserWindow::set_storage(GImGuiWindowStorage* storage)
{
}

bool GImGuiContentBrowserWindow::need_render()
{
	return true;
}

void GImGuiContentBrowserWindow::render()
{
	ImGui::Image(m_textIconResource->get_descriptor_set()->get_vk_descriptor(), ImVec2{50,50});
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
	m_textIconResource->destroy();
	m_textIconResource = GSharedPtr<IGTextureResource, GSHARED_PTR_INTERNAL_MODE_THREAD_SAFE>();
}
