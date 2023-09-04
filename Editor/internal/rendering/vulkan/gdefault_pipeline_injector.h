#ifndef GDEFAULT_PIPELINE_INJECTOR_H
#define GDEFAULT_PIPELINE_INJECTOR_H

#include "volk.h"
class IGVulkanLogicalDevice;
#include "engine/rendering/vulkan/igvulkan_graphic_pipeline_layout_creator.h"
class GDefaultPipelineInjector : public IGVulkanGraphicPipelineLayoutCreator
{
public:
	GDefaultPipelineInjector(IGVulkanLogicalDevice* boundedDevice);
	// Inherited via IGVulkanGraphicPipelineLayoutCreator
	virtual void inject_create_info(VkGraphicsPipelineCreateInfo* info) override;
	virtual std::expected<IGVulkanPipelineLayout*, LAYOUT_CREATOR_ERROR> create_layout_for(IGVulkanGraphicPipeline* pipeline) override;
	virtual std::expected<IGVulkanDescriptorPool*, LAYOUT_CREATOR_ERROR> create_descriptor_pool_and_sets(IGVulkanGraphicPipeline* pipeline, std::vector<VkDescriptorSet_T*>* descriptorSets) override;

private:

	IGVulkanLogicalDevice* m_boundedDevice;
};

#endif // GDEFAULT_PIPELINE_INJECTOR_H