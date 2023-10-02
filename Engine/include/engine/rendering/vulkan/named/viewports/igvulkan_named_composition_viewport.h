#ifndef IGVULKAN_NAMED_COMPOSITION_VIEWPORT_H
#define IGVULKAN_NAMED_COMPOSITION_VIEWPORT_H


#include "engine/rendering/vulkan/named/igvulkan_named_viewport.h"

class IGVulkanNamedCompositionViewport : public IGVulkanNamedViewport
{
public:
	virtual ~IGVulkanNamedCompositionViewport() = default;

	virtual IVulkanImage* get_composition_attachment() const noexcept = 0;

	virtual IGVulkanNamedRenderPass* get_composition_renderpass() const noexcept = 0;
private:
};

#endif // IGVULKAN_NAMED_COMPOSITION_VIEWPORT_H