#ifndef IGVULKAN_NAMED_GRAPHIC_PIPELINE_H
#define IGVULKAN_NAMED_GRAPHIC_PIPELINE_H

#include <vector>

struct VkPipeline_T;
class IGVulkanNamedPipelineLayout;
class IVulkanShaderStage;
class IGVulkanGraphicPipelineState;

class IGVulkanNamedGraphicPipeline
{
public:
	virtual ~IGVulkanNamedGraphicPipeline() = default;
	
	virtual VkPipeline_T* get_vk_pipeline() const noexcept = 0;

	virtual bool init(IGVulkanNamedPipelineLayout* layout, const std::vector<IVulkanShaderStage*>& shaderStages, const std::vector<IGVulkanGraphicPipelineState*>& states) = 0;

	virtual void destroy() = 0;
private:
};

#endif // IGVULKAN_NAMED_GRAPHIC_PIPELINE_H