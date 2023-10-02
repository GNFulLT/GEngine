#ifndef IGVULKAN_NAMED_VIEWPORT_H
#define IGVULKAN_NAMED_VIEWPORT_H

#include "engine/rendering/vulkan/ivulkan_viewport.h"
#include "engine/rendering/vulkan/vulkan_command_buffer.h"
#include <string>
#include "engine/rendering/vulkan/ivulkan_image.h"
#include "engine/rendering/vulkan//ivulkan_renderpass.h"
#include "engine/rendering/vulkan/named/igvulkan_named_renderpass.h"
struct VkSampler_T;

class IGVulkanNamedViewport
{
public:
	virtual ~IGVulkanNamedViewport() = default;

	virtual const char* get_name() const noexcept = 0;
	virtual const VkViewport* get_viewport_area() const noexcept =0;
	virtual const VkRect2D* get_scissor_area() const noexcept = 0;

	virtual void begin_draw_cmd(GVulkanCommandBuffer* cmd) = 0;
	virtual void end_draw_cmd(GVulkanCommandBuffer* cmd) = 0;

	virtual bool is_sampled() const noexcept { return false; }

	virtual bool init(uint32_t width,uint32_t height) = 0;
	virtual void destroy() = 0;
	virtual bool resize(uint32_t width,uint32_t height) = 0;

	//X Extension methods
	//X If the vp is not sampled this method will return null pointer no matter what
	virtual IVulkanImage* get_named_attachment(const std::string& name) const noexcept { return nullptr; }

	//X If there is default sampler give it.
	virtual VkSampler_T* get_sampler_for_named_attachment(const std::string& name) const noexcept { return nullptr; }

	//X If there is default renderpass. Give it
	virtual IGVulkanRenderPass* get_dedicated_renderpass() const noexcept { return nullptr; }

	//X If there is named renderpass give it.
	virtual IGVulkanNamedRenderPass* get_named_renderpass(const char* name) const noexcept { return nullptr; }

	//X Begin to specific renderpass
	virtual bool begin_draw_cmd_to_named_pass(GVulkanCommandBuffer* cmd, const char* name) { return false; };

private:
};

#endif // IGVULKAN_NAMED_VIEWPORT_H