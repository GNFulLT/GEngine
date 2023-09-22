#ifndef GSCENE_RENDERER_LAYOUT_H
#define GSCENE_RENDERER_LAYOUT_H


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

class GSceneWireframeRendererLayout : public IGVulkanGraphicPipelineLayoutCreator
{
public:
	GSceneWireframeRendererLayout(IGVulkanLogicalDevice* dev, uint32_t framesInFlight,IGSceneManager* sceneManager,IVulkanBuffer* meshBuffer, IVulkanBuffer* transformBuffer,IVulkanBuffer* drawDataBuffer,IVulkanBuffer* drawDataIDBuffer,IVulkanBuffer* materialBuffer);

	// Inherited via IGVulkanGraphicPipelineLayoutCreator
	virtual void inject_create_info(VkGraphicsPipelineCreateInfo* info) override;
	virtual std::expected<IGVulkanPipelineLayout*, LAYOUT_CREATOR_ERROR> create_layout_for(IGVulkanGraphicPipeline* pipeline) override;
	virtual std::expected<IGVulkanDescriptorPool*, LAYOUT_CREATOR_ERROR> create_descriptor_pool_and_sets(IGVulkanGraphicPipeline* pipeline, std::vector<VkDescriptorSet_T*>* descriptorSets) override;

	VkDescriptorSetLayout_T* get_set_layout();
	std::vector<VkDescriptorSet_T*>* get_sets();


	void write_set_layout(uint32_t binding,uint32_t bufferType,VkDescriptorBufferInfo* info);
private:
	IGVulkanLogicalDevice* p_boundedDevice;
	IGSceneManager* p_sceneManager;

	IVulkanBuffer* p_meshBuffer;
	IVulkanBuffer* p_drawDataBuffer;
	IVulkanBuffer* p_drawDataIDuffer;
	IVulkanBuffer* p_materialBuffer;
	IVulkanBuffer* p_transformBuffer;

	uint32_t m_framesInFlight;

	VkDescriptorSetLayout_T* m_descriptorSetLayout;
	IGVulkanPipelineLayout* m_pipelineLayout;

	std::vector<VkDescriptorSet_T*>* p_descriptorSets;
};

#endif // GSCENE_RENDERER_LAYOUT_H