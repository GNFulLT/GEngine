#ifndef GVULKAN_OFFSCREEN_DEPTH_VIEWPORT_H
#define GVULKAN_OFFSCREEN_DEPTH_VIEWPORT_H


#include "volk.h"

#include "engine/rendering/vulkan/ivulkan_viewport.h"
#include "internal/engine/rendering/vulkan/vulkan_renderpass.h"
#include "engine/manager/igpipeline_object_manager.h"
#include "internal/engine/rendering/vulkan/gvulkan_render_target.h"

class IGVulkanLogicalDevice;
class IVulkanImage;
class IGVulkanDescriptorCreator;
class IGVulkanDescriptorSet;

class GVulkanOffScreenDepthViewport : public IGVulkanViewport
{
public:
	GVulkanOffScreenDepthViewport(IGPipelineObjectManager* mng,IGVulkanLogicalDevice* dev, IGVulkanDescriptorCreator* descriptorCreator);


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
	GVulkanRenderTarget m_renderTarget;
	VkSampler_T* m_sampler;

	IGPipelineObjectManager* m_pipelineObjectManager;

	//X Render Image
	IVulkanImage* m_image;
	//X Depth Image
	IVulkanImage* m_depthImage;
	//X Bounded pool
	IGVulkanDescriptorCreator* m_descriptorCreator;
	//X Texture set
	IGVulkanDescriptorSet* m_descriptorSet;

	VkViewport m_viewport;
	VkRect2D m_scissor;

	VkFormat m_format;
	VkFormat m_depthFormat;
	
	GSharedPtr<IGVulkanNamedRenderPass> m_depthRenderpass;

	// Inherited via IGVulkanViewport
	virtual const VkViewport* get_viewport_area() const noexcept override;
	virtual const VkRect2D* get_scissor_area() const noexcept override;
};

#endif // GVULKAN_OFFSCREEN_DEPTH_VIEWPORT_H