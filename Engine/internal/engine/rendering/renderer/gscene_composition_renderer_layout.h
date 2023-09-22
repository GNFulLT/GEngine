#ifndef GSCENE_COMPOSITION_RENDERER_LAYOUT_H
#define GSCENE_COMPOSITION_RENDERER_LAYOUT_H


#include "engine/rendering/vulkan/igvulkan_graphic_pipeline_layout_creator.h"
#include "engine/rendering/vulkan/ivulkan_ldevice.h"
#include "engine/rendering/vulkan/ivulkan_buffer.h"
#include "engine/manager/igscene_manager.h"
#include "internal/engine/rendering/vulkan/gvulkan_basic_pipeline_layout.h"
#include "engine/rendering/vulkan/ivulkan_ldevice.h"
#include "engine/rendering/vulkan/ivulkan_image.h"
struct VkDescriptorSetLayout_T;
struct VkDescriptorBufferInfo;
class IGSceneManager;
class IVulkanBuffer;
#include <memory>
#include <cstdint>
#include <vector>

class GSceneCompositionRendererLayout : public IGVulkanGraphicPipelineLayoutCreator
{
public:
	GSceneCompositionRendererLayout(IGVulkanLogicalDevice* dev,uint32_t framesInFlight,IVulkanImage* posImage,IVulkanImage* normalImage,IVulkanImage* albedoImage,VkSampler_T* sampler);
	// Inherited via IGVulkanGraphicPipelineLayoutCreator
	virtual void inject_create_info(VkGraphicsPipelineCreateInfo* info) override;
	virtual std::expected<IGVulkanPipelineLayout*, LAYOUT_CREATOR_ERROR> create_layout_for(IGVulkanGraphicPipeline* pipeline) override;
	virtual std::expected<IGVulkanDescriptorPool*, LAYOUT_CREATOR_ERROR> create_descriptor_pool_and_sets(IGVulkanGraphicPipeline* pipeline, std::vector<VkDescriptorSet_T*>* descriptorSets) override;

	void write_sets(IVulkanImage* posImage, IVulkanImage* normalImage, IVulkanImage* albedoImage);
private:
	IGVulkanLogicalDevice* p_boundedDevice;
	std::vector<VkDescriptorSet_T*>* p_descriptorSets;
	VkDescriptorSetLayout_T* m_descriptorSetLayout;
	IVulkanImage* m_posImage;
	IVulkanImage* m_normalImage;
	IVulkanImage* m_albedoIamge;
	uint32_t m_framesInFlight;
	VkSampler_T* m_sampler;
	IGVulkanPipelineLayout* m_pipelineLayout;
};
#endif //GSCENE_COMPOSITION_RENDERER_LAYOUT_H