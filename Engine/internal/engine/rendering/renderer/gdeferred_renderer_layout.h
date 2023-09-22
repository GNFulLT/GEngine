#ifndef GSCENE_DEFERRED_RENDERER_LAYOUT_H
#define GSCENE_DEFERRED_RENDERER_LAYOUT_H



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
struct VkPipelineLayout_T;

class GSceneDeferredRendererLayout : public IGVulkanGraphicPipelineLayoutCreator
{
public:
	GSceneDeferredRendererLayout(IGVulkanLogicalDevice* dev ,VkDescriptorSetLayout_T* layout, VkDescriptorSetLayout_T* textureLayout,std::vector<VkDescriptorSet_T*>* descriptorSets);

	// Inherited via IGVulkanGraphicPipelineLayoutCreator
	virtual void inject_create_info(VkGraphicsPipelineCreateInfo* info) override;
	virtual std::expected<IGVulkanPipelineLayout*, LAYOUT_CREATOR_ERROR> create_layout_for(IGVulkanGraphicPipeline* pipeline) override;
	virtual std::expected<IGVulkanDescriptorPool*, LAYOUT_CREATOR_ERROR> create_descriptor_pool_and_sets(IGVulkanGraphicPipeline* pipeline, std::vector<VkDescriptorSet_T*>* descriptorSets) override;

	void write_set_layout(uint32_t binding, uint32_t bufferType, VkDescriptorBufferInfo* info);

	virtual bool own_sets() override;

	VkPipelineLayout_T* get_pipeline_layout();

	VkDescriptorSet_T* get_set_by_index(uint32_t frameIndex);

	
private:
	IGVulkanLogicalDevice* p_boundedDevice;


	VkDescriptorSetLayout_T* m_descriptorSetLayout;
	IGVulkanPipelineLayout* m_pipelineLayout;
	VkPipelineLayout_T* m_pipeLayout;
	VkDescriptorSetLayout_T* m_textureLayout;

	std::vector<VkDescriptorSet_T*>* p_descriptorSets;
};


#endif // GSCENE_DEFERRED_RENDERER_LAYOUT_H