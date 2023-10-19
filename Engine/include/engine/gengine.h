#ifndef GAPPLICATION_H
#define GAPPLICATION_H

#include "gapplication_impl.h"
#include "GEngine_EXPORT.h"
#include "engine/rendering/mesh/gmesh.h"
#include "engine/rendering/vulkan/named/igvulkan_named_viewport.h"
#include "public/core/templates/shared_ptr.h"
#include <vector>
#include <queue>
#include "engine/rendering/vulkan/vulkan_command_buffer.h"
#include <functional>

class IGVulkanRenderPass;
class Window;
class IGVulkanDevice;
class IGVulkanApp;
class IManagerTable;
class IGVulkanViewport;
class GVulkanSwapchain;
class IGVulkanDescriptorCreator;
struct VkSurfaceKHR_T;
class IGVulkanSwapchain;
class GVulkanFrameData;
class IGVulkanFrameData;
class IGVulkanChainedViewport;
class Scene;
struct MaterialDescription;
struct WireFrameSpec;
struct MeshData;
class IGTextureResource;

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

	IGVulkanViewport* create_offscreen_viewport_depth(IGVulkanDescriptorCreator* descriptor);


	void destroy_offscreen_viewport(IGVulkanViewport* port);
	static GEngine* get_instance();

	IGVulkanSwapchain* get_swapchain();

	uint32_t get_current_frame();

	uint32_t get_frame_count();

	IGVulkanFrameData* get_frame_data_by_index(uint32_t index);

	void bind_copy_stage(std::function<void(GVulkanCommandBuffer*)> fnc);

	void bind_staging_delete_stage(std::function<void()> fnc);

	void add_recreation(std::function<void()> recreationFun);

private:
	void wait_all_frame_data();
	void exit();

	void tick(double deltaTime);
	bool before_render();
	void after_render();
	Window* m_window;
	GApplicationImpl* m_impl;
	IManagerTable* m_managerTable;
	GSharedPtr<IGVulkanApp> m_vulkanApp;
	GVulkanSwapchain* m_vulkanSwapchain;
	VkSurfaceKHR_T* m_mainSurface;
	std::vector<GVulkanFrameData*> m_frames;
	uint32_t m_currentFrame;

//X Need to be changed
private:
	IGVulkanChainedViewport* create_offscreen_viewport_depth_chained(IGVulkanDescriptorCreator* descriptor, uint32_t imageCount);
	IGVulkanViewport* create_offscreen_viewport(IGVulkanDescriptorCreator* descriptor);

	
	std::queue<std::function<void()>> m_recreationQueues;
	std::queue<std::function<void(GVulkanCommandBuffer*)>> m_copyStageQueue;
	std::queue<std::function<void()>> m_stagingDeletionQueue;
#ifdef _DEBUG
	bool m_inited = false;
#endif
};

extern ENGINE_API GEngine* create_the_engine();

#endif // GApplication