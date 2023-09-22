#ifndef GGRID_PIPELINE_LAYOUT_CREATOR_H
#define GGRID_PIPELINE_LAYOUT_CREATOR_H

#include "engine/rendering/vulkan/igvulkan_graphic_pipeline_layout_creator.h"
#include <vector>
#include "public/core/templates/shared_ptr.h"
#include "engine/manager/igpipeline_object_manager.h"
#include "engine/manager/igscene_manager.h"


class IGVulkanLogicalDevice;
class IGVulkanDescriptorPool;
struct VkDescriptorSetLayout_T;
struct VkDescriptorSet_T;
class IGTextureResource;
class IGVulkanPipelineLayout;

class GGridPipelineLayoutCreator : public IGVulkanGraphicPipelineLayoutCreator
{
public:
	GGridPipelineLayoutCreator(IGVulkanLogicalDevice* device, IGSceneManager* sceneManager, IGPipelineObjectManager* objManager
		, uint32_t framesInFlight);

	virtual void inject_create_info(VkGraphicsPipelineCreateInfo* info) override;
	virtual std::expected<IGVulkanPipelineLayout*, LAYOUT_CREATOR_ERROR> create_layout_for(IGVulkanGraphicPipeline* pipeline) override;
	virtual std::expected<IGVulkanDescriptorPool*, LAYOUT_CREATOR_ERROR> create_descriptor_pool_and_sets(IGVulkanGraphicPipeline* pipeline, std::vector<VkDescriptorSet_T*>* descriptorSets) override;

private:
	IGVulkanLogicalDevice* m_boundedDevice;
	IGPipelineObjectManager* m_pipelineObjectManager;
	IGSceneManager* m_sceneManager;
	uint32_t m_framesInFlight;
	IGVulkanDescriptorPool* m_descriptorPool;
	VkDescriptorSetLayout_T* m_descriptorSetLayout;

	std::vector<VkDescriptorSet_T*> m_descriptorSets;
};

#endif // GGRID_PIPELINE_LAYOUT_CREATOR_H