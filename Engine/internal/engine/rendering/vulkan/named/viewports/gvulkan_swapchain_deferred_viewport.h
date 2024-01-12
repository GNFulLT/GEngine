#ifndef GVULKAN_SWAPCHAIN_DEFERRED_VIEWPORT_H
#define GVULKAN_SWAPCHAIN_DEFERRED_VIEWPORT_H

#include "volk.h"
#include "engine/rendering/vulkan/named/viewports/igvulkan_named_deferred_viewport.h"
#include "engine/rendering/vulkan/named/igvulkan_named_renderpass.h"
#include "engine/rendering/vulkan/ivulkan_ldevice.h"
#include <array>

class GVulkanSwapchainDeferredViewport : public IGVulkanNamedDeferredViewport
{
public:
	GVulkanSwapchainDeferredViewport(IGVulkanLogicalDevice* dev, IGVulkanNamedRenderPass* deferredPass, IGVulkanNamedRenderPass* compositionPass
		, VkFormat positionFormat, VkFormat albedoFormat, VkFormat emissionFormat, VkFormat pbrFormmat, VkFormat compositionFormat);

	// Inherited via IGVulkanNamedDeferredViewport
	virtual const char* get_name() const noexcept override;
	virtual const VkViewport* get_viewport_area() const noexcept override;
	virtual const VkRect2D* get_scissor_area() const noexcept override;
	virtual void begin_draw_cmd(GVulkanCommandBuffer* cmd) override;
	virtual void end_draw_cmd(GVulkanCommandBuffer* cmd) override;
	virtual bool init(uint32_t width, uint32_t height) override;
	virtual void destroy() override;
	virtual bool resize(uint32_t width, uint32_t height) override;
	virtual IVulkanImage* get_albedo_attachment() const noexcept override;
	virtual IVulkanImage* get_emission_attachment() const noexcept override;
	virtual IVulkanImage* get_position_attachment() const noexcept override;
	virtual IVulkanImage* get_pbr_attachment() const noexcept override;
	virtual IVulkanImage* get_composition_attachment() const noexcept override;
	virtual IGVulkanNamedRenderPass* get_composition_renderpass() const noexcept override;
	virtual IGVulkanNamedRenderPass* get_deferred_renderpass() const noexcept override;
	virtual void begin_composition_draw_cmd(GVulkanCommandBuffer* cmd) override;
private:
	std::string m_name;
};

#endif // GVULKAN_SWAPCHAIN_DEFERRED_VIEWPORT_H