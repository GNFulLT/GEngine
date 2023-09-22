#ifndef GVULKAN_CAMERA_LAYOUT_CREATOR_H
#define GVULKAN_CAMERA_LAYOUT_CREATOR_H

#include "engine/rendering/vulkan/igvulkan_graphic_pipeline_layout_creator.h"
#include "engine/rendering/vulkan/igvulkan_uniform_buffer.h"

class IGVulkanLogicalDevice;
struct VkDescriptorSetLayout_T;
class IGSceneManager;

class GVulkanCameraLayoutCreator : public IGVulkanGraphicPipelineLayoutCreator
{
public:
	GVulkanCameraLayoutCreator(IGVulkanLogicalDevice* boundedDevice, IGSceneManager* sceneMng,uint32_t frameInFlight);
	// Inherited via IGVulkanGraphicPipelineLayoutCreator
	virtual void inject_create_info(VkGraphicsPipelineCreateInfo* info) override;
	virtual std::expected<IGVulkanPipelineLayout*, LAYOUT_CREATOR_ERROR> create_layout_for(IGVulkanGraphicPipeline* pipeline) override;
	virtual std::expected<IGVulkanDescriptorPool*, LAYOUT_CREATOR_ERROR> create_descriptor_pool_and_sets(IGVulkanGraphicPipeline* pipeline, std::vector<VkDescriptorSet_T*>* descriptorSets) override;

	virtual void destroy() override;

private:
	IGVulkanLogicalDevice* m_boundedDevice;
	IGSceneManager* m_sceneMng;
	uint32_t m_frameInFlight;

	std::vector<VkDescriptorSet_T*> m_descriptorSets;
	VkDescriptorSetLayout_T* m_descriptorSetLayout;
	IGVulkanPipelineLayout* m_pipelineLayout;

};
#endif // GVULKAN_CAMERA_LAYOUT_CREATOR_H