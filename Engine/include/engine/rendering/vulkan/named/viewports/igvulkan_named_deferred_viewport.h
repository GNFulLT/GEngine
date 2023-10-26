#ifndef IGVULKAN_NAMED_DEFERRED_VIEWPORT_H
#define IGVULKAN_NAMED_DEFERRED_VIEWPORT_H

#include "engine/rendering/vulkan/named/igvulkan_named_viewport.h"

enum VkFormat;

class IGVulkanNamedDeferredViewport : public IGVulkanNamedViewport
{
public:
	virtual ~IGVulkanNamedDeferredViewport() = default;

	virtual IVulkanImage* get_albedo_attachment() const noexcept = 0;

	virtual IVulkanImage* get_emission_attachment() const noexcept = 0;

	virtual IVulkanImage* get_position_attachment() const noexcept = 0;

	virtual IVulkanImage* get_pbr_attachment() const noexcept = 0;

	virtual IVulkanImage* get_composition_attachment() const noexcept = 0;

	virtual IGVulkanNamedRenderPass* get_composition_renderpass() const noexcept = 0;

	virtual IGVulkanNamedRenderPass* get_deferred_renderpass() const noexcept = 0;


	virtual void begin_composition_draw_cmd(GVulkanCommandBuffer* cmd) = 0;
private:
};

#endif // IGVULKAN_NAMED_DEFERRED_VIEWPORT_H