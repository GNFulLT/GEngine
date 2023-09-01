#ifndef GVULKAN_GRAPHIC_PIPELINE
#define GVULKAN_GRAPHIC_PIPELINE

#include <volk.h>
#include "engine/rendering/vulkan/ivulkan_graphic_pipeline.h"

struct VkPipeline_T;
class IGVulkanGraphicPipelineState;

class IGVulkanLogicalDevice;
class IGVulkanRenderPass;
class IGVulkanPipelineLayout;
class GVulkanGraphicPipeline : public IGVulkanGraphicPipeline
{
public:
	GVulkanGraphicPipeline(IGVulkanLogicalDevice* dev, IGVulkanRenderPass* boundedRenderpass, IGVulkanPipelineLayout* bindedLayout,
		const std::vector<IVulkanShaderStage*>& shaderStages,const std::vector<IGVulkanGraphicPipelineState*>& states,int flag);

	bool init();

	void destroy();

	// Inherited via IGVulkanGraphicPipeline
	virtual const std::vector<IVulkanShaderStage*>* get_shader_stages() override;
private:
	IGVulkanLogicalDevice* m_boundedDevice;
	IGVulkanPipelineLayout* m_boundedLayout;
	IGVulkanRenderPass* m_boundedRenderpass;
	VkPipeline_T* m_vkPipeline;
	std::vector<IVulkanShaderStage*> m_shaderStages;
	std::vector<IGVulkanGraphicPipelineState*> m_pipelineStates;
	int m_flag;

};


#endif // GVULKAN_GRAPHIC_PIPELINE