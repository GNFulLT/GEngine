#ifndef GVULKAN_FRAME_DATA_H
#define GVULKAN_FRAME_DATA_H

#include <vector>
#include <cstdint>
#include <utility>

class IGVulkanLogicalDevice;
class GVulkanFence;
class GVulkanSemaphore;
class IGVulkanQueue;
class GVulkanCommandBuffer;
struct VkSemaphore_T;

class GVulkanFrameData
{
public:
	GVulkanFrameData(IGVulkanLogicalDevice* dev,uint32_t imageIndex,GVulkanFence* waitFence,GVulkanSemaphore* imageAcquired,GVulkanSemaphore* renderSemaphore,
		GVulkanCommandBuffer* cmd);

	GVulkanCommandBuffer* get_cmd();

	void submit_the_cmd(IGVulkanQueue* queue);
private:
	GVulkanFence* m_waitQueueFence;
	GVulkanSemaphore* m_imageAcquiredSemaphore;
	GVulkanSemaphore* m_renderSemaphore;

	IGVulkanLogicalDevice* m_boundedDev;
	GVulkanCommandBuffer* m_cmd;
	uint32_t m_imageIndex;


	// These will be cleared
	std::vector<VkSemaphore_T*> m_waitSemaphores;
	std::vector< uint32_t> m_waitStages;
	std::vector<VkSemaphore_T*> m_signalSemaphores;
};

#endif // GVULKAN_FRAME_DATA_H