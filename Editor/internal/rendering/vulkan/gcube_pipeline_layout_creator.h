#ifndef GCUBE_PIPELINE_LAYOUT_CREATOR_H
#define GCUBE_PIPELINE_LAYOUT_CREATOR_H

#include "engine/rendering/vulkan/igvulkan_graphic_pipeline_layout_creator.h"
#include <vector>
#include "public/core/templates/shared_ptr.h"

class IGVulkanLogicalDevice;
class IGVulkanDescriptorPool;
struct VkDescriptorSetLayout_T;
struct VkDescriptorSet_T;
class IGCameraManager;
class IGTextureResource;
class IGVulkanPipelineLayout;

class GCubePipelinelayoutCreator : public IGVulkanGraphicPipelineLayoutCreator
{
public:
	GCubePipelinelayoutCreator(IGVulkanLogicalDevice* device, IGCameraManager* cameraManager,GSharedPtr<IGTextureResource> cubeTextureResource,uint32_t framesInFlight);

	// Inherited via IGVulkanGraphicPipelineLayoutCreator
	virtual void inject_create_info(VkGraphicsPipelineCreateInfo* info) override;
	virtual std::expected<IGVulkanPipelineLayout*, LAYOUT_CREATOR_ERROR> create_layout_for(IGVulkanGraphicPipeline* pipeline) override;
	virtual std::expected<IGVulkanDescriptorPool*, LAYOUT_CREATOR_ERROR> create_descriptor_pool_and_sets(IGVulkanGraphicPipeline* pipeline, std::vector<VkDescriptorSet_T*>* descriptorSets) override;
private:
	IGVulkanLogicalDevice* m_boundedDevice;
	uint32_t m_framesInFlight;
	IGVulkanDescriptorPool* m_descriptorPool;
	VkDescriptorSetLayout_T* m_descriptorSetLayout;
	std::vector<VkDescriptorSet_T*> m_descriptorSets;
	IGCameraManager* m_cameraManager;
	GSharedPtr<IGTextureResource> m_cubeTexture;
	IGVulkanPipelineLayout* m_pipelineLayout;
};

#endif // GCUBE_PIPELINE_LAYOUT_CREATOR_H