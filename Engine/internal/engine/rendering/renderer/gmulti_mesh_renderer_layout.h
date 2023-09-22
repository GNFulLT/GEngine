#ifndef GMULTI_MESH_RENDERER_LAYOUT_H
#define GMULTI_MESH_RENDERER_LAYOUT_H

#include "engine/rendering/vulkan/igvulkan_graphic_pipeline_layout_creator.h"
#include "engine/rendering/vulkan/ivulkan_ldevice.h"
#include "engine/rendering/vulkan/ivulkan_buffer.h"
#include "engine/manager/igscene_manager.h"
#include "internal/engine/rendering/vulkan/gvulkan_basic_pipeline_layout.h"

struct VkDescriptorSetLayout_T;
class GMultiMeshRendererLayout : public IGVulkanGraphicPipelineLayoutCreator
{
public:
	GMultiMeshRendererLayout(IGVulkanLogicalDevice* dev, IGSceneManager* scene,IVulkanBuffer* storageBuff, IVulkanBuffer* meshBuff,IVulkanBuffer* drawDataBuff, IVulkanBuffer* transformData, IVulkanBuffer* materialData,uint32_t vertSize,uint32_t indexSize,uint32_t frameCount);
	// Inherited via IGVulkanGraphicPipelineLayoutCreator
	virtual void inject_create_info(VkGraphicsPipelineCreateInfo* info) override;
	virtual std::expected<IGVulkanPipelineLayout*, LAYOUT_CREATOR_ERROR> create_layout_for(IGVulkanGraphicPipeline* pipeline) override;
	virtual std::expected<IGVulkanDescriptorPool*, LAYOUT_CREATOR_ERROR> create_descriptor_pool_and_sets(IGVulkanGraphicPipeline* pipeline, std::vector<VkDescriptorSet_T*>* descriptorSets) override;

private:
	IGVulkanLogicalDevice* m_boundedDevice;
	uint32_t m_frameCount;
	VkDescriptorSetLayout_T* m_descriptorSetLayout;
	IGVulkanPipelineLayout* m_pipelineLayout;

	IVulkanBuffer* m_storageBuffer;
	IVulkanBuffer* m_drawDataBuffer;
	
	IVulkanBuffer* m_transformBuffer;
	IVulkanBuffer* m_materialBuffer;

	IVulkanBuffer* m_meshBuffer;

	IGSceneManager* m_sceneManager;


	std::uint32_t vertexSize;
	std::uint32_t indexSize;
};

#endif // GMULTI_MESH_RENDERER_LAYOUT_H