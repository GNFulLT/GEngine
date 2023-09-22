#ifndef GSCENE_MATERIAL_RENDERER_LAYOUT_H
#define GSCENE_MATERIAL_RENDERER_LAYOUT_H



#include "engine/rendering/vulkan/igvulkan_graphic_pipeline_layout_creator.h"
#include "engine/rendering/vulkan/ivulkan_ldevice.h"
#include "engine/rendering/vulkan/ivulkan_buffer.h"
#include "engine/manager/igscene_manager.h"
#include "internal/engine/rendering/vulkan/gvulkan_basic_pipeline_layout.h"

struct VkDescriptorSetLayout_T;
struct VkDescriptorBufferInfo;

class IGVulkanLogicalDevice;
class IGSceneManager;
class IVulkanBuffer;
#include <memory>
#include <cstdint>
#include <vector>

class GSceneMaterialRendererLayout : public IGVulkanGraphicPipelineLayoutCreator
{
public:
	GSceneMaterialRendererLayout(IGVulkanLogicalDevice* dev, VkDescriptorSetLayout_T* materialLayout,VkDescriptorSetLayout_T* layout,std::vector<VkDescriptorSet_T*>* descriptorSets);

	// Inherited via IGVulkanGraphicPipelineLayoutCreator
	virtual void inject_create_info(VkGraphicsPipelineCreateInfo* info) override;
	virtual std::expected<IGVulkanPipelineLayout*, LAYOUT_CREATOR_ERROR> create_layout_for(IGVulkanGraphicPipeline* pipeline) override;
	virtual std::expected<IGVulkanDescriptorPool*, LAYOUT_CREATOR_ERROR> create_descriptor_pool_and_sets(IGVulkanGraphicPipeline* pipeline, std::vector<VkDescriptorSet_T*>* descriptorSets) override;

	void write_set_layout(uint32_t binding, uint32_t bufferType, VkDescriptorBufferInfo* info);

	virtual bool own_sets() override;

	IGVulkanPipelineLayout* get_pipeline_layout();
private:
	IGVulkanLogicalDevice* p_boundedDevice;


	VkDescriptorSetLayout_T* m_descriptorSetLayout;
	VkDescriptorSetLayout_T* m_materialLayout;
	IGVulkanPipelineLayout* m_pipelineLayout;

	std::vector<VkDescriptorSet_T*>* p_descriptorSets;
};


#endif // GSCENE_MATERIAL_RENDERER_LAYOUT_H