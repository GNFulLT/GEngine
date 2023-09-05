#ifndef GVULKAN_FRAME_DATA_H
#define GVULKAN_FRAME_DATA_H

#include <vector>
#include <cstdint>
#include <utility>
#include "engine/rendering/vulkan/vulkan_memory.h"
#include "engine/rendering/vulkan/vulkan_command_buffer.h"
#include "public/core/templates/shared_ptr.h"
#include "engine/rendering/igvulkan_frame_data.h"

class IGVulkanLogicalDevice;
class IGVulkanQueue;
struct VkSemaphore_T;

class GVulkanFrameData : public IGVulkanFrameData
{
public:
	GVulkanFrameData(IGVulkanLogicalDevice* dev,uint32_t imageIndex,IGVulkanQueue* queue);

	GVulkanCommandBuffer* get_cmd();

	virtual GVulkanCommandBuffer* get_the_main_cmd() override;

	void submit_the_cmd();

	void destroy();

	bool prepare_for_rendering();

	void wait_fence();

	GVulkanSemaphore* get_image_acquired_semaphore();

	GVulkanSemaphore* get_render_finished_semaphore();
private:
	GVulkanFence* m_waitQueueFence;
	GVulkanSemaphore* m_imageAcquiredSemaphore;
	GVulkanSemaphore* m_renderSemaphore;
	IGVulkanLogicalDevice* m_boundedDev;
	GVulkanCommandBuffer* m_cmd;
	uint32_t m_imageIndex;

	GSharedPtr<GVulkanCommandBufferManager> m_cmdManager;
	GSharedPtr<GVulkanFenceManager> m_fenceManager;
	GSharedPtr<GVulkanSemaphoreManager> m_semaphoreManager;

	IGVulkanQueue* m_queue;


	// These will be cleared
	std::vector<VkSemaphore_T*> m_waitSemaphores;
	std::vector< uint32_t> m_waitStages;
	std::vector<VkSemaphore_T*> m_signalSemaphores;

	// Inherited via IGVulkanFrameData
	virtual GVulkanCommandBuffer* create_command_buffer_for_this_frame() override;

	// Inherited via IGVulkanFrameData
	virtual void add_wait_semaphore_for_this_frame(GVulkanSemaphore* waitSemaphore, int stage) override;
};

#endif // GVULKAN_FRAME_DATA_H