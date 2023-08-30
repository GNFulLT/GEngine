#ifndef GIMGUI_CONTENT_BROWSER_WINDOW_H
#define GIMGUI_CONTENT_BROWSER_WINDOW_H

#include "editor/igimgui_window_impl.h"

#include <string>
#include <filesystem>

class IGVulkanViewport;
class GImGuiWindowStorage;
class IGTextureResource;

#include "public/core/templates/shared_ptr.h"
#include "content_helper/gimgui_content_helper.h"
enum FILE_TYPE;

class IMouseManager;
class GImGuiContentBrowserWindow : public IGImGuiWindowImpl
{
public:
	GImGuiContentBrowserWindow();
	// Inherited via IGImGuiWindowImpl
	virtual bool init() override;
	virtual void set_storage(GImGuiWindowStorage* storage) override;
	virtual bool need_render() override;
	virtual void render() override;
	virtual void on_resize() override;
	virtual void on_data_update() override;
	virtual const char* get_window_name() override;

	virtual void destroy() override;

private:
	GImGuiWindowStorage* m_storage;
	std::filesystem::path m_currentPath;
	std::filesystem::path m_rightClickedFile;
	FILE_TYPE m_rightClickedFileType;

	std::string m_name;
	GSharedPtr<IGTextureResource> m_folderIcon;
	GSharedPtr<IGTextureResource> m_txtIcon;
	GSharedPtr<IGTextureResource> m_hlslIcon;
	GSharedPtr<IGTextureResource> m_glslIcon;

	IMouseManager* m_mouse;
	GImGuiContentHelper m_contentHelper;
};

#endif // GIMGUI_CONTENT_BROWSER_WINDOW_H