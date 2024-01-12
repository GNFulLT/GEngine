#include "internal/engine/rendering/vulkan/named/viewports/gvulkan_swapchain_deferred_viewport.h"

const char* GVulkanSwapchainDeferredViewport::get_name() const noexcept
{
	return nullptr;
}

const VkViewport* GVulkanSwapchainDeferredViewport::get_viewport_area() const noexcept
{
	return nullptr;
}

const VkRect2D* GVulkanSwapchainDeferredViewport::get_scissor_area() const noexcept
{
	return nullptr;
}

void GVulkanSwapchainDeferredViewport::begin_draw_cmd(GVulkanCommandBuffer* cmd)
{
}

void GVulkanSwapchainDeferredViewport::end_draw_cmd(GVulkanCommandBuffer* cmd)
{
}

bool GVulkanSwapchainDeferredViewport::init(uint32_t width, uint32_t height)
{
	return false;
}

void GVulkanSwapchainDeferredViewport::destroy()
{
}

bool GVulkanSwapchainDeferredViewport::resize(uint32_t width, uint32_t height)
{
	return false;
}

IVulkanImage* GVulkanSwapchainDeferredViewport::get_albedo_attachment() const noexcept
{
	return nullptr;
}

IVulkanImage* GVulkanSwapchainDeferredViewport::get_emission_attachment() const noexcept
{
	return nullptr;
}

IVulkanImage* GVulkanSwapchainDeferredViewport::get_position_attachment() const noexcept
{
	return nullptr;
}

IVulkanImage* GVulkanSwapchainDeferredViewport::get_pbr_attachment() const noexcept
{
	return nullptr;
}

IVulkanImage* GVulkanSwapchainDeferredViewport::get_composition_attachment() const noexcept
{
	return nullptr;
}

IGVulkanNamedRenderPass* GVulkanSwapchainDeferredViewport::get_composition_renderpass() const noexcept
{
	return nullptr;
}

IGVulkanNamedRenderPass* GVulkanSwapchainDeferredViewport::get_deferred_renderpass() const noexcept
{
	return nullptr;
}

void GVulkanSwapchainDeferredViewport::begin_composition_draw_cmd(GVulkanCommandBuffer* cmd)
{
}
