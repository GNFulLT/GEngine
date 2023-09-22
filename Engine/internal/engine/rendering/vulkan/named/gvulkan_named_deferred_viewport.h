#ifndef GVULKAN_NAMED_DEFERRED_VIEWPORT_H
#define GVULKAN_NAMED_DEFERRED_VIEWPORT_H

#include "volk.h"
#include "engine/rendering/vulkan/named/igvulkan_named_viewport.h"
#include "engine/rendering/vulkan/ivulkan_ldevice.h"
#include <unordered_map>
#include <string_view>
#include "engine/rendering/vulkan/ivulkan_image.h"
#include <array>
#include "engine/rendering/vulkan/ivulkan_renderpass.h"
#include "engine/rendering/vulkan/named/igvulkan_named_renderpass.h"


inline static constexpr const std::string_view POSITION_ATTACHMENT = "POSITION_ATTACHMENT";
inline static constexpr const std::string_view NORMAL_ATTACHMENT = "NORMAL_ATTACHMENT";
inline static constexpr const std::string_view ALBEDO_ATTACHMENT = "ALBEDO_ATTACHMENT";
inline static constexpr const std::string_view FIRST_RENDER_PASS = "deferred_first_render_pass";
inline static constexpr const std::string_view SECOND_RENDER_PASS = "deferred_second_render_pass";


class GVulkanNamedDeferredViewport :public IGVulkanNamedViewport
{
public:
	GVulkanNamedDeferredViewport(IGVulkanLogicalDevice* dev,std::string_view name);
	~GVulkanNamedDeferredViewport();
	// Inherited via IGVulkanNamedViewport
	virtual const char* get_name() const noexcept override;

	virtual const VkViewport* get_viewport_area() const noexcept override;

	virtual const VkRect2D* get_scissor_area() const noexcept override;

	virtual void begin_draw_cmd(GVulkanCommandBuffer* cmd) override;

	virtual void end_draw_cmd(GVulkanCommandBuffer* cmd) override;

	virtual bool is_sampled() const noexcept override;

	virtual IVulkanImage* get_named_attachment(const std::string& name) const noexcept override;

	// Inherited via IGVulkanNamedViewport
	virtual bool init(uint32_t width, uint32_t height) override;

	virtual void destroy() override;

	virtual bool resize(uint32_t width, uint32_t height) override;

	virtual VkSampler_T* get_sampler_for_named_attachment(const std::string& name) const noexcept override;

	virtual IGVulkanRenderPass* get_dedicated_renderpass() const noexcept;

	virtual IGVulkanNamedRenderPass* get_named_renderpass(const char* name) const noexcept;

	virtual bool begin_draw_cmd_to_named_pass(GVulkanCommandBuffer* cmd,const char* name);
private:
	IGVulkanLogicalDevice* m_boundedDevice;
	VkViewport m_viewport;
	VkRect2D m_scissor;
	std::string_view m_name;
	VkRenderPassBeginInfo m_renderPassBeginInfo;
	VkRenderPassBeginInfo m_secondPassBeginInfo;

	std::array<VkClearValue, 4> clearValues;
	std::array<VkClearValue, 2> secondClearValues;

	IVulkanImage* m_composedAttachment;

	IVulkanImage* m_positionAttachment;
	IVulkanImage* m_normalAttachment;
	IVulkanImage* m_albedoAttachment;
	IVulkanImage* m_depthAttachment;

	std::unordered_map<std::string, IVulkanImage*> m_attachments;
	std::unordered_map<std::string, IGVulkanNamedRenderPass*> m_namedRenderpasses;

	IGVulkanNamedRenderPass* m_firstPass;
	IGVulkanNamedRenderPass* m_secondPass;

	VkRenderPass m_renderPass;
	

	VkFramebuffer m_frameBuffer;
	VkFramebuffer m_secondFrameBuffer;

	VkSampler m_sampler;

};

#endif // GVULKAN_NAMED_DEFERRED_VIEWPORT_H