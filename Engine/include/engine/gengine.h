#ifndef GAPPLICATION_H
#define GAPPLICATION_H

#include "gapplication_impl.h"
#include "GEngine_EXPORT.h"
#include "public/core/templates/shared_ptr.h"

class Window;
class IGVulkanDevice;
class IGVulkanApp;
class IManagerTable;
class IGVulkanViewport;
class GVulkanSwapchain;
class IGVulkanDescriptorCreator;
struct VkSurfaceKHR_T;
class IGVulkanSwapchain;

class ENGINE_API GEngine
{
public:
	virtual ~GEngine() = default;

	GEngine();

	void run();

	void init(GApplicationImpl* impl);


	Window* get_main_window();

	IManagerTable* get_manager_table();

	IGVulkanViewport* get_viewport();

	IGVulkanApp* get_app();

	IGVulkanViewport* create_offscreen_viewport(IGVulkanDescriptorCreator* descriptor);

	void destroy_offscreen_viewport(IGVulkanViewport* port);
	static GEngine* get_instance();

	IGVulkanSwapchain* get_swapchain();
private:
	void exit();

	void tick();
	bool before_render();
	void after_render();
	Window* m_window;
	GApplicationImpl* m_impl;
	IManagerTable* m_managerTable;
	GSharedPtr<IGVulkanApp> m_vulkanApp;
	GVulkanSwapchain* m_vulkanSwapchain;
	VkSurfaceKHR_T* m_mainSurface;
#ifdef _DEBUG
	bool m_inited = false;
#endif
};

extern ENGINE_API GEngine* create_the_engine();

#endif // GApplication