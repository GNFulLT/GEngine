#include "volk.h"

#include "internal/engine/rendering/gvulkan_frame_data.h"
#include "engine/rendering/vulkan/vulkan_memory.h"
#include "engine/rendering/vulkan/vulkan_command_buffer.h"
#include "engine/rendering/vulkan/ivulkan_queue.h"

GVulkanFrameData::GVulkanFrameData(IGVulkanLogicalDevice* dev, uint32_t imageIndex,GVulkanFence* waitFence, GVulkanSemaphore* presentSemaphore, 
	GVulkanSemaphore* renderSemaphore, GVulkanCommandBuffer* cmd)
{
	m_boundedDev = dev;
	m_imageIndex = imageIndex;
	m_waitQueueFence = waitFence;
	m_imageAcquiredSemaphore = presentSemaphore;
	m_renderSemaphore = renderSemaphore;
	m_cmd = cmd;
}

GVulkanCommandBuffer* GVulkanFrameData::get_cmd()
{
	return m_cmd;
}

void GVulkanFrameData::submit_the_cmd(IGVulkanQueue* queue)
{
	VkSubmitInfo inf = {
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.pNext = nullptr,
		.waitSemaphoreCount = m_waitSemaphores.size(),
		.pWaitSemaphores = m_waitSemaphores.data(),
		.pWaitDstStageMask = m_waitStages.data(),
		.commandBufferCount = 1,
		.pCommandBuffers = m_cmd->get_handle_p(),
		.signalSemaphoreCount = m_signalSemaphores.size(),
		.pSignalSemaphores = m_signalSemaphores.data()
	};

	vkQueueSubmit(queue->get_queue(), 1, &inf, m_waitQueueFence->get_fence());
}


