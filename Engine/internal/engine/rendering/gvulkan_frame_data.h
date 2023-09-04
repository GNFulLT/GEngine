#ifndef GVULKAN_FRAME_DATA_H
#define GVULKAN_FRAME_DATA_H

#include <vector>
#include <cstdint>

class IGVulkanLogicalDevice;
class GVulkanFence;
class GVulkanSemaphore;

class GVulkanFrameData
{
public:
	GVulkanFrameData(IGVulkanLogicalDevice* dev,uint32_t imageIndex,GVulkanFence* waitFence,GVulkanSemaphore* presentSemaphore,GVulkanSemaphore* renderSemaphore);
private:
	GVulkanFence* m_waitQueueFence;
	GVulkanSemaphore* m_presentSemaphore;
	GVulkanSemaphore* m_renderSemaphore;

	IGVulkanLogicalDevice* m_boundedDev;
	uint32_t m_imageIndex;
};

#endif // GVULKAN_FRAME_DATA_H