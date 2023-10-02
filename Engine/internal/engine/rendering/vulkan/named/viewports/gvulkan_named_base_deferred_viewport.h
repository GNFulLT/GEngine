#ifndef GVULKAN_BASED_NAMED_DEFERRED_VIEWPORT_H
#define GVULKAN_BASED_NAMED_DEFERRED_VIEWPORT_H

#include "volk.h"
#include "engine/rendering/vulkan/named/viewports/igvulkan_named_deferred_viewport.h"
#include "engine/rendering/vulkan/named/igvulkan_named_renderpass.h"
#include "engine/rendering/vulkan/ivulkan_ldevice.h"
#include <array>

class GVulkanNamedBaseDeferredViewport : public IGVulkanNamedDeferredViewport
{
public:
	GVulkanNamedBaseDeferredViewport(IGVulkanLogicalDevice* dev,IGVulkanNamedRenderPass* deferredPass, IGVulkanNamedRenderPass* compositionPass
		,VkFormat positionFormat,VkFormat albedoFormat,VkFormat emissionFormat,VkFormat pbrFormmat,VkFormat compositionFormat);
	// Inherited via IGVulkanNamedDeferredViewport
	virtual const char* get_name() const noexcept override;
	virtual const VkViewport* get_viewport_area() const noexcept override;
	virtual const VkRect2D* get_scissor_area() const noexcept override;
	virtual void begin_draw_cmd(GVulkanCommandBuffer* cmd) override;
	virtual void end_draw_cmd(GVulkanCommandBuffer* cmd) override;
	virtual bool init(uint32_t width, uint32_t height) override;
	virtual void destroy() override;
	virtual bool resize(uint32_t width, uint32_t height) override;
	virtual IVulkanImage* get_albedo_attachment() const noexcept override;
	virtual IVulkanImage* get_emission_attachment() const noexcept override;
	virtual IVulkanImage* get_position_attachment() const noexcept override;
	virtual IVulkanImage* get_pbr_attachment() const noexcept override;
	virtual IGVulkanNamedRenderPass* get_deferred_renderpass() const noexcept override;
	virtual bool is_sampled() const noexcept override;
	virtual VkSampler_T* get_sampler_for_named_attachment(const std::string& name) const noexcept override;
private:
	IGVulkanLogicalDevice* m_boundedDevice;
	IGVulkanNamedRenderPass* m_deferredPass;
	IGVulkanNamedRenderPass* m_compositionPass;
	VkFormat m_positionFormat;
	VkFormat m_albedoFormat;
	VkFormat m_emissionFormat;
	VkFormat m_pbrFormat;
	VkFormat m_compositionFormat;
	VkFormat m_depthFormat;

	std::string m_name;
	VkViewport m_viewport;
	VkRect2D m_scissor;

	VkFramebuffer m_deferredFrameBuffer;
	VkFramebuffer m_compositionFramebuffer;

	VkSampler m_sampler;

	VkRenderPassBeginInfo m_deferredBeginInfo;
	VkRenderPassBeginInfo m_compositionBeginInfo;

	IVulkanImage* m_depthAttachment;

	IVulkanImage* m_positionAttachment;
	IVulkanImage* m_albedoAttachment;
	IVulkanImage* m_emissionAttachment;
	IVulkanImage* m_pbrAttachment;
	IVulkanImage* m_compositionAttachment;


	std::array<VkClearValue, 5> clearValues;
	std::array<VkClearValue, 2> secondClearValues;

	// Inherited via IGVulkanNamedDeferredViewport
	virtual IVulkanImage* get_composition_attachment() const noexcept override;
	virtual IGVulkanNamedRenderPass* get_composition_renderpass() const noexcept override;

	// Inherited via IGVulkanNamedDeferredViewport
	virtual void begin_composition_draw_cmd(GVulkanCommandBuffer* cmd) override;
};

#endif // GVULKAN_BASED_NAMED_DEFERRED_VIEWPORT_H