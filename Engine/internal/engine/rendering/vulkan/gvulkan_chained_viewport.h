#ifndef GVULKAN_CHAINED_VIEWPORT_H
#define GVULKAN_CHAINED_VIEWPORT_H


#include "volk.h"

#include "engine/rendering/vulkan/igvulkan_chained_viewport.h"
#include "internal/engine/rendering/vulkan/vulkan_renderpass.h"
#include <vector>
#include <cstdint>

class IGVulkanLogicalDevice;
class IVulkanImage;
class IGVulkanDescriptorCreator;
class IGVulkanDescriptorSet;

class GVulkanChainedViewport : public IGVulkanChainedViewport
{
public:
	GVulkanChainedViewport(IGVulkanLogicalDevice* dev, IGVulkanDescriptorCreator* descriptorCreator,uint32_t imageCount);



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



	// Inherited via IGVulkanChainedViewport
	virtual void set_image_index(uint32_t index) override;
	virtual uint32_t get_image_count() override;
private:
	IGVulkanLogicalDevice* m_boundedDevice;
	IGVulkanDescriptorCreator* m_creator;

	GVulkanRenderpass m_renderpass;

	std::vector<IVulkanImage*> m_renderImages;
	std::vector<IVulkanImage*> m_depthImages;
	std::vector<IGVulkanDescriptorSet*> m_descriptorSets;

	VkViewport m_viewport;
	VkRect2D m_scissor;

	VkSampler_T* m_sampler;

	VkFormat m_format;
	VkFormat m_depthFormat;
	uint32_t m_imageCount;

	uint32_t m_currentImage;

	// Inherited via IGVulkanChainedViewport
	virtual const VkViewport* get_viewport_area() const noexcept override;
	virtual const VkRect2D* get_scissor_area() const noexcept override;
};
#endif // GVULKAN_CHAINED_VIEWPORT_H