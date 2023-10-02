#ifndef GVULKAN_NAMED_GRAPHIC_PIPELINE_H
#define GVULKAN_NAMED_GRAPHIC_PIPELINE_H


#include "volk.h"
#include "engine/rendering/vulkan/ivulkan_ldevice.h"
#include <string>
#include "engine/rendering/vulkan/named/igvulkan_named_renderpass.h"
#include "engine/rendering/vulkan/named/igvulkan_named_pipeline_layout.h"
#include "engine/rendering/vulkan/named/igvulkan_named_graphic_pipeline.h"

class IGVulkanPipelineLayout;
class IGVulkanDescriptorPool;

class GVulkanNamedGraphicPipeline : public IGVulkanNamedGraphicPipeline
{
public:
	GVulkanNamedGraphicPipeline(IGVulkanLogicalDevice* dev, IGVulkanNamedRenderPass* boundedRenderpass,const char* name);

	bool init(IGVulkanNamedPipelineLayout* layout,const std::vector<IVulkanShaderStage*>& shaderStages, const std::vector<IGVulkanGraphicPipelineState*>& states) override;

	void destroy() override;
private:
	std::string m_name;
	VkPipeline m_pipeline;
	IGVulkanLogicalDevice* p_boundedDevice;
	IGVulkanNamedRenderPass* p_boundedRenderpass;

	// Inherited via IGVulkanNamedGraphicPipeline
	virtual VkPipeline_T* get_vk_pipeline() const noexcept override;
};

#endif // GVULKAN_NAMED_GRAPHIC_PIPELINE_H