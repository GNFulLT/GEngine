#ifndef GVULKAN_OFFSCREEN_DEPTH_VIEWPORT_H
#define GVULKAN_OFFSCREEN_DEPTH_VIEWPORT_H


#include "volk.h"

#include "engine/rendering/vulkan/ivulkan_viewport.h"
#include "internal/engine/rendering/vulkan/vulkan_renderpass.h"
class IGVulkanLogicalDevice;
class IVulkanImage;
class IGVulkanDescriptorCreator;
class IGVulkanDescriptorSet;

class GVulkanOffScreenDepthViewport : public IGVulkanViewport
{
public:
	GVulkanOffScreenDepthViewport(IGVulkanLogicalDevice* dev, IGVulkanDescriptorCreator* descriptorCreator);


	// Inherited via IGVulkanViewport
	virtual void* get_vk_current_image_renderpass() override;
	virtual uint32_t get_width() const override;
	virtual uint32_t get_height() const override;
	virtual bool init(int width, int height, int vkFormat) override;
	virtual void destroy(bool forResize) override;
	virtual void begin_draw_cmd(GVulkanCommandBuffer* cmd) override;
	virtual void end_draw_cmd(GVulkanCommandBuffer* cmd) override;
	virtual IGVulkanRenderPass* get_render_pass() override;
	virtual bool can_be_used_as_texture() override;
	virtual IGVulkanDescriptorSet* get_descriptor() override;
private:
	IGVulkanLogicalDevice* m_boundedDevice;
	GVulkanRenderpass m_renderpass;
	VkSampler_T* m_sampler;

	//X Render Image
	IVulkanImage* m_image;
	//X Depth Image
	IVulkanImage* m_depthImage;
	//X Bounded pool
	IGVulkanDescriptorCreator* m_descriptorCreator;
	//X Texture set
	IGVulkanDescriptorSet* m_descriptorSet;

	VkViewport m_viewport;

	VkFormat m_format;
	VkFormat m_depthFormat;
};

#endif // GVULKAN_OFFSCREEN_DEPTH_VIEWPORT_H