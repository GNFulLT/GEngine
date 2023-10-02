#ifndef IGVULKAN_DEFERRED_RENDERER_H
#define IGVULKAN_DEFERRED_RENDERER_H

#include "engine/rendering/vulkan/vulkan_command_buffer.h"
#include "engine/rendering/vulkan/named/igvulkan_named_renderpass.h"
#include <vector>
#include "engine/rendering/renderer/igvulkan_deferred_renderer.h"
enum VkFormat;

class IGVulkanDeferredRenderer
{
public:
	virtual IGVulkanNamedRenderPass* get_deferred_pass() const noexcept = 0;

	virtual IGVulkanNamedRenderPass* get_composition_pass() const noexcept = 0;

	virtual void fill_deferred_cmd(GVulkanCommandBuffer* cmd,uint32_t frame) = 0;

	virtual void fill_composition_cmd(GVulkanCommandBuffer* cmd, uint32_t frame) = 0;

	virtual std::vector<VkFormat> get_deferred_formats() const noexcept = 0;

	virtual VkFormat get_composition_format() const noexcept = 0;
private:
};

#endif // IGVULKAN_DEFERRED_RENDERER_H