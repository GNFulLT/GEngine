#include "internal/engine/rendering/vulkan/vulkan_viewport.h"
#include "internal/engine/rendering/vulkan/vulkan_swapchain.h"
#include "engine/rendering/vulkan/vulkan_command_buffer.h"

#include "public/core/templates/memnewd.h"

static uint32_t SwapchainImageCount = 3;

GVulkanViewport::GVulkanViewport(GVulkanLogicalDevice* inDevice, uint32_t sizeX, uint32_t sizeY)
{
	m_device = inDevice;
	m_sizeX = sizeX;
	m_sizeY = sizeY;
	m_swapchain = nullptr;
}

GVulkanViewport::~GVulkanViewport()
{
}

bool GVulkanViewport::init(VkSurfaceKHR surface, VkSurfaceFormatKHR format, IGVulkanQueue* presentQueue)
{
	//X TODO GDNEWDA
	m_swapchain = new GVulkanSwapchain(m_device, surface, SwapchainImageCount, m_sizeX, m_sizeY, format, presentQueue);

	bool inited = m_swapchain->init();
	if (!inited)
	{
		delete m_swapchain;
		return false;
	}


	return true;
}

void GVulkanViewport::destroy()
{
	if (m_swapchain != nullptr)
	{
		m_swapchain->destroy();
	}
}

int GVulkanViewport::get_current_image_index() const
{
	return m_swapchain->get_current_image_index();
}

void* GVulkanViewport::get_vk_current_image_renderpass()
{
	return m_swapchain->get_current_image_renderpass();
}

uint32_t GVulkanViewport::get_width() const
{
	return m_sizeX;
}

uint32_t GVulkanViewport::get_height() const
{
	return m_sizeY;
}

uint32_t GVulkanViewport::get_total_image() const
{
	return m_swapchain->get_total_image();
}

void GVulkanViewport::begin_draw_cmd(GVulkanCommandBuffer* cmd)
{
	m_swapchain->begin_cmd(cmd->get_handle());
}

void GVulkanViewport::end_draw_cmd(GVulkanCommandBuffer* cmd)
{
	m_swapchain->end_cmd(cmd->get_handle());
}

bool GVulkanViewport::acquire_draw_image(GVulkanSemaphore* waitSemaphore)
{
	return m_swapchain->acquire_draw_image(waitSemaphore);
}

bool GVulkanViewport::present_image(uint32_t waitSemaphoreCount, GVulkanSemaphore* waitSemaphores)
{
	return m_swapchain->present_image(waitSemaphoreCount,waitSemaphores);
}

bool GVulkanViewport::need_handle()
{
	return m_swapchain->need_handle();
}

bool GVulkanViewport::handle()
{
	return m_swapchain->handle();
}
