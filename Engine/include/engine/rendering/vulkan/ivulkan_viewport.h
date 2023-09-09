#ifndef IGVULKAN_VIEWPORT_H
#define IGVULKAN_VIEWPORT_H

#include "engine/GEngine_EXPORT.h"
#include <cstdint>

class GVulkanCommandBuffer;
class GVulkanSemaphore;
class IGVulkanDescriptorSet;
class IGVulkanRenderPass;

struct VkRect2D;
struct VkViewport;

class ENGINE_API IGVulkanViewport
{
public:
	virtual ~IGVulkanViewport() = default;

	virtual void* get_vk_current_image_renderpass() = 0;

	virtual uint32_t get_width() const = 0;

	virtual uint32_t get_height() const = 0;

	virtual bool init(int width, int height, int vkFormat) = 0;

	virtual void destroy(bool forResize) = 0;

	//X TODO : CHANGE THIS
	virtual void begin_draw_cmd(GVulkanCommandBuffer* cmd) = 0;
	virtual void end_draw_cmd(GVulkanCommandBuffer* cmd) = 0;

	virtual IGVulkanRenderPass* get_render_pass() = 0;

	virtual bool can_be_used_as_texture() = 0;
	
	virtual const VkViewport* get_viewport_area() const noexcept = 0;
	virtual const VkRect2D* get_scissor_area() const noexcept = 0;

	// If the viewport couldn't be used as texture it returns null
	virtual IGVulkanDescriptorSet* get_descriptor() = 0;
private:
};

#endif // IGVULKAN_VIEWPORT_H