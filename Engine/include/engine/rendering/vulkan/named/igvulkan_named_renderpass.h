#ifndef IGVULKAN_NAMED_RENDERPASS_H
#define IGVULKAN_NAMED_RENDERPASS_H

#include "engine/rendering/vulkan/ivulkan_renderpass.h"

class IGVulkanNamedRenderPass : public IGVulkanRenderPass
{
public:
	virtual ~IGVulkanNamedRenderPass() = default;

	virtual int get_supported_render_format() = 0;
private:
};

#endif // IGVULKAN_NAMED_RENDERPASS_H