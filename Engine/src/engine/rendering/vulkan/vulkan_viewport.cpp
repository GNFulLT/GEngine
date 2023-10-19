#include "internal/engine/rendering/vulkan/vulkan_main_viewport.h"
#include "internal/engine/rendering/vulkan/vulkan_swapchain.h"
#include "engine/rendering/vulkan/vulkan_command_buffer.h"
#include "public/core/templates/memnewd.h"
#include "internal/engine/rendering/vulkan/vulkan_ldevice.h"

static uint32_t SwapchainImageCount = 3;

GVulkanMainViewport::GVulkanMainViewport(GVulkanLogicalDevice* inDevice, uint32_t sizeX, uint32_t sizeY)
{
	m_device = inDevice;
	m_sizeX = sizeX;
	m_sizeY = sizeY;
	m_currentImage = 0;
	m_imageViews = nullptr;
	m_viewport.height = 0;
	m_viewport.width = 0;
	m_viewport.minDepth = 0;
	m_viewport.maxDepth = 1;
	m_viewport.x = 0;
	m_viewport.y = 0;
	m_scissor.offset = { 0,0 };
	m_scissor.extent = { 0,0 };
}

GVulkanMainViewport::~GVulkanMainViewport()
{
}

bool GVulkanMainViewport::init(int width, int height,int format)
{
	assert(m_imageViews != nullptr);
	m_viewport.width = width;
	m_viewport.height = height;
	m_scissor.extent.height = height;
	m_scissor.extent.width = width;
	//X TODO GDNEWDA
	std::vector<C_GVec2> sizes(m_imageViews->size());
	for (int i = 0; i < sizes.size(); i++)
	{
		sizes[i].x = width;
		sizes[i].y = height;
	}

	std::vector<VkClearValue> clearValues;
	clearValues.push_back({ {0.f,0.f,0.f,0.f} });

	m_renderpass.create((VkDevice)m_device->get_vk_device(), *m_imageViews, sizes, clearValues, (VkFormat)format,
		VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

	return true;
}

void GVulkanMainViewport::set_image_views_ref(const std::vector<VkImageView_T*>* imageViews)
{
	m_imageViews = imageViews;
}

void GVulkanMainViewport::set_current_image(uint32_t index)
{
	m_currentImage = index;
}


void GVulkanMainViewport::destroy(bool forResize)
{
	m_renderpass.destroy((VkDevice)m_device->get_vk_device(), forResize);
}

void* GVulkanMainViewport::get_vk_current_image_renderpass()
{
	return m_renderpass.get_handle(m_currentImage);
}

uint32_t GVulkanMainViewport::get_width() const
{
	return m_sizeX;
}

uint32_t GVulkanMainViewport::get_height() const
{
	return m_sizeY;
}

void GVulkanMainViewport::begin_draw_cmd(GVulkanCommandBuffer* cmd)
{
	m_renderpass.begin(cmd->get_handle(), m_currentImage);
}

void GVulkanMainViewport::end_draw_cmd(GVulkanCommandBuffer* cmd)
{
	m_renderpass.end(cmd->get_handle());
}

bool GVulkanMainViewport::can_be_used_as_texture()
{
	return false;
}

IGVulkanDescriptorSet* GVulkanMainViewport::get_descriptor()
{
	return nullptr;
}

IGVulkanRenderPass* GVulkanMainViewport::get_render_pass()
{
	return &m_renderpass;
}

const VkViewport* GVulkanMainViewport::get_viewport_area() const noexcept
{
	return &m_viewport;
}
const VkRect2D* GVulkanMainViewport::get_scissor_area() const noexcept
{
	return &m_scissor;
}
