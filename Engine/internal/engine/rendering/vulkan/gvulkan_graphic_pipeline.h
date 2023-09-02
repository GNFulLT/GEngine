#ifndef GVULKAN_GRAPHIC_PIPELINE
#define GVULKAN_GRAPHIC_PIPELINE

#include <volk.h>
#include "engine/rendering/vulkan/ivulkan_graphic_pipeline.h"
#include "public/core/templates/unordered_dense.h"


class IGVulkanGraphicPipelineState;

class IGVulkanLogicalDevice;
class IGVulkanRenderPass;
class IGVulkanPipelineLayout;
class IGVulkanDescriptorPool;
struct VkDescriptorSet_T;
class IGVulkanUniformBuffer;

class GVulkanGraphicPipeline : public IGVulkanGraphicPipeline
{
public:
	GVulkanGraphicPipeline(IGVulkanLogicalDevice* dev, IGVulkanRenderPass* boundedRenderpass,
		const std::vector<IVulkanShaderStage*>& shaderStages,const std::vector<IGVulkanGraphicPipelineState*>& states,int flag);

	bool init();

	virtual void destroy() override;

	// Inherited via IGVulkanGraphicPipeline
	virtual const std::vector<IVulkanShaderStage*>* get_shader_stages() override;
private:
	IGVulkanLogicalDevice* m_boundedDevice;
	IGVulkanPipelineLayout* m_pipelineLayout;
	IGVulkanRenderPass* m_boundedRenderpass;
	VkPipeline_T* m_vkPipeline;
	std::vector <VkDescriptorSet_T*> m_descriptorSets;

	std::vector<IVulkanShaderStage*> m_shaderStages;
	std::vector<IGVulkanGraphicPipelineState*> m_pipelineStates;
	int m_flag;


	ankerl::unordered_dense::segmented_map<uint32_t, std::vector<VkWriteDescriptorSet>> m_writeSets;
	
	IGVulkanDescriptorPool* m_graphicPool;

	// Inherited via IGVulkanGraphicPipeline
	virtual VkPipeline_T* get_pipeline() override;
};


#endif // GVULKAN_GRAPHIC_PIPELINE