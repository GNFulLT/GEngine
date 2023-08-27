#ifndef GIMGUI_CONTENT_BROWSER_WINDOW_H
#define GIMGUI_CONTENT_BROWSER_WINDOW_H

#include "editor/igimgui_window_impl.h"

#include <string>
class IGVulkanViewport;
class GImGuiWindowStorage;
class IGTextureResource;

#include "public/core/templates/shared_ptr.h"

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
	std::string m_name;
	GSharedPtr<IGTextureResource, GSHARED_PTR_INTERNAL_MODE_THREAD_SAFE> m_textIconResource;
};

#endif // GIMGUI_CONTENT_BROWSER_WINDOW_H