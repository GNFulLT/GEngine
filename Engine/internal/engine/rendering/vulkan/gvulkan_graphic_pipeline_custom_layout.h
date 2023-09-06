#ifndef GVULKAN_GRAPHIC_PIPELINE_CUSTOM_LAYOUT_H
#define GVULKAN_GRAPHIC_PIPELINE_CUSTOM_LAYOUT_H

#include <volk.h>
#include "engine/rendering/vulkan/ivulkan_graphic_pipeline.h"
#include "public/core/templates/unordered_dense.h"
#include "engine/rendering/vulkan/igvulkan_graphic_pipeline_layout_creator.h"

class IGVulkanGraphicPipelineState;

class IGVulkanLogicalDevice;
class IGVulkanRenderPass;
class IGVulkanPipelineLayout;
class IGVulkanDescriptorPool;
struct VkDescriptorSet_T;
class IGVulkanUniformBuffer;

class GVulkanGraphicPipelineCustomLayout : public IGVulkanGraphicPipeline
{
public:
	GVulkanGraphicPipelineCustomLayout(IGVulkanLogicalDevice* dev, IGVulkanRenderPass* boundedRenderpass,
		const std::vector<IVulkanShaderStage*>& shaderStages, const std::vector<IGVulkanGraphicPipelineState*>& states,IGVulkanGraphicPipelineLayoutCreator* creator ,int flag);


	// Inherited via IGVulkanGraphicPipeline
	virtual const std::vector<IVulkanShaderStage*>* get_shader_stages() override;
	virtual VkPipeline_T* get_pipeline() override;
	virtual void destroy() override;
	
	virtual void bind_sets(GVulkanCommandBuffer* cmd,uint32_t frameIndex) override;

	bool init();
private:
	IGVulkanLogicalDevice* m_boundedDevice;
	IGVulkanPipelineLayout* m_pipelineLayout;
	IGVulkanRenderPass* m_boundedRenderpass;
	VkPipeline_T* m_vkPipeline;
	std::vector <VkDescriptorSet_T*> m_descriptorSets;
	IGVulkanDescriptorPool* m_graphicPool;
	std::vector<IVulkanShaderStage*> m_shaderStages;
	std::vector<IGVulkanGraphicPipelineState*> m_pipelineStates;
	int m_flag;


	IGVulkanGraphicPipelineLayoutCreator* m_layoutCreator;


	// Inherited via IGVulkanGraphicPipeline
	virtual IGVulkanPipelineLayout* get_pipeline_layout() override;

};

#endif // GVULKAN_GRAPHIC_PIPELINE_CUSTOM_LAYOUT_H