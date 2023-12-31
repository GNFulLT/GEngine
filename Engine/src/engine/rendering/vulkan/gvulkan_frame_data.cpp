#include "volk.h"

#include "internal/engine/rendering/gvulkan_frame_data.h"
#include "engine/rendering/vulkan/vulkan_memory.h"
#include "engine/rendering/vulkan/vulkan_command_buffer.h"
#include "engine/rendering/vulkan/ivulkan_queue.h"

GVulkanFrameData::GVulkanFrameData(IGVulkanLogicalDevice* dev, uint32_t imageIndex,IGVulkanQueue* queue)
{
	m_boundedDev = dev;
	m_imageIndex = imageIndex;
	m_queue = queue;

	m_cmdManager = GSharedPtr<GVulkanCommandBufferManager>(new GVulkanCommandBufferManager(dev,queue,true));
	m_fenceManager = GSharedPtr<GVulkanFenceManager>(new GVulkanFenceManager(dev));
	m_semaphoreManager = GSharedPtr<GVulkanSemaphoreManager>(new GVulkanSemaphoreManager(dev));

	m_cmdManager->init();

	m_waitQueueFence = m_fenceManager->create_fence();
	m_imageAcquiredSemaphore = m_semaphoreManager->create_semaphore();
	m_renderSemaphore = m_semaphoreManager->create_semaphore();


	m_waitQueueFence->init(true);
	m_imageAcquiredSemaphore->init();
	m_renderSemaphore->init();

	m_cmd = m_cmdManager->create_buffer(true);
	m_cmd->init();
}
void GVulkanFrameData::wait_fence()
{
	//X Wait
	m_waitQueueFence->wait();
}
GVulkanCommandBuffer* GVulkanFrameData::get_cmd()
{
	return m_cmd;
}

GVulkanCommandBuffer* GVulkanFrameData::get_the_main_cmd()
{
	return m_cmd;
}

void GVulkanFrameData::submit_the_cmd()
{
	VkSemaphore semaphore = m_renderSemaphore->get_semaphore();
	auto cmd = m_cmd->get_handle();
	VkSubmitInfo inf = {
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.pNext = nullptr,
		.waitSemaphoreCount = uint32_t(m_waitSemaphores.size()),
		.pWaitSemaphores = m_waitSemaphores.data(),
		.pWaitDstStageMask = m_waitStages.data(),
		.commandBufferCount = 1,
		.pCommandBuffers = &cmd,
		.signalSemaphoreCount = 1,
		.pSignalSemaphores = &semaphore
	};

	vkQueueSubmit(m_queue->get_queue(), 1, &inf, m_waitQueueFence->get_fence());
}

void GVulkanFrameData::destroy()
{
	m_cmd->destroy();
	delete m_cmd;
	m_waitQueueFence->destroy();
	delete m_waitQueueFence;
	m_imageAcquiredSemaphore->destroy();
	m_renderSemaphore->destroy();
	delete m_imageAcquiredSemaphore;
	delete m_renderSemaphore;
	m_cmdManager->destroy();
}

bool GVulkanFrameData::prepare_for_rendering()
{	
	//X Reset the fence
	m_waitQueueFence->reset();
	m_waitSemaphores.clear();
	m_waitStages.clear();
	m_waitSemaphores.push_back(m_imageAcquiredSemaphore->get_semaphore());
	m_waitStages.push_back(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
	m_cmd->reset();

	

	return true;
}

GVulkanSemaphore* GVulkanFrameData::get_image_acquired_semaphore()
{
	return m_imageAcquiredSemaphore;
}

GVulkanSemaphore* GVulkanFrameData::get_render_finished_semaphore()
{
	return m_renderSemaphore;
}

GVulkanCommandBuffer* GVulkanFrameData::create_command_buffer_for_this_frame()
{
	return m_cmdManager->create_buffer(true);
}

void GVulkanFrameData::add_wait_semaphore_for_this_frame(GVulkanSemaphore* waitSemaphore, int stage)
{
	m_waitSemaphores.push_back(waitSemaphore->get_semaphore());
	m_waitStages.push_back(stage);
}


