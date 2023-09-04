#include "internal/engine/rendering/gvulkan_frame_data.h"

GVulkanFrameData::GVulkanFrameData(IGVulkanLogicalDevice* dev, uint32_t imageIndex,GVulkanFence* waitFence, GVulkanSemaphore* presentSemaphore, GVulkanSemaphore* renderSemaphore)
{
	m_boundedDev = dev;
	m_imageIndex = imageIndex;
	m_waitQueueFence = waitFence;
	m_presentSemaphore = presentSemaphore;
	m_renderSemaphore = renderSemaphore;
}
