#ifndef GVULKAN_OFFSCREEN_VIEWPORT_H
#define GVULKAN_OFFSCREEN_VIEWPORT_H

#include "volk.h"

#include "engine/rendering/vulkan/ivulkan_viewport.h"
#include "internal/engine/rendering/vulkan/vulkan_renderpass.h"
class IGVulkanLogicalDevice;
class IVulkanImage;
class IGVulkanDescriptorCreator;
class IGVulkanDescriptorSet;

class GVulkanOffScreenViewport : public IGVulkanViewport
{
public:
	GVulkanOffScreenViewport(IGVulkanLogicalDevice* dev, IGVulkanDescriptorCreator* descriptorCreator);
	~GVulkanOffScreenViewport();
	bool init(int width,int height,int vkFormat) override;

	void destroy(bool forResize) override;
	// Inherited via IGVulkanViewport
	virtual void* get_vk_current_image_renderpass() override;

	virtual uint32_t get_width() const override;

	virtual uint32_t get_height() const override;

	virtual void begin_draw_cmd(GVulkanCommandBuffer* cmd) override;

	virtual void end_draw_cmd(GVulkanCommandBuffer* cmd) override;


	virtual bool can_be_used_as_texture() override;

	virtual IGVulkanRenderPass* get_render_pass() override;
	// If the viewport couldn't be used as texture it returns null
	virtual IGVulkanDescriptorSet* get_descriptor() override;
private:
	IGVulkanLogicalDevice* m_boundedDevice;
	GVulkanRenderpass m_renderpass;
	VkSampler_T* m_sampler;
	IVulkanImage* m_image;
	IGVulkanDescriptorCreator* m_descriptorCreator;
	IGVulkanDescriptorSet* m_descriptorSet;
	
	VkViewport m_viewport;
	VkRect2D m_scissor;


	// Inherited via IGVulkanViewport
	virtual const VkViewport* get_viewport_area() const noexcept override;

	virtual const VkRect2D* get_scissor_area() const noexcept override;

};

#endif // GVULKAN_OFFSCREEN_VIEWPORT_H