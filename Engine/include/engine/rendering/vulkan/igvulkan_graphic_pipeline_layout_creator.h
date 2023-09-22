#ifndef IGVULKAN_GRAPHIC_PIPELINE_LAYOUT_CREATOR_H
#define IGVULKAN_GRAPHIC_PIPELINE_LAYOUT_CREATOR_H

#include "engine/rendering/vulkan/ivulkan_pipeline_layout.h"
#include "engine/rendering/vulkan/ivulkan_graphic_pipeline.h"
#include "engine/rendering/vulkan/ivulkan_descriptor_pool.h"

#include <expected>
#include <vector>

struct VkDescriptorSet_T;

enum LAYOUT_CREATOR_ERROR
{
	LAYOUT_CREATOR_ERROR_UNKNOWN
};

struct VkGraphicsPipelineCreateInfo;

class ENGINE_API IGVulkanGraphicPipelineLayoutCreator
{
public:
	virtual ~IGVulkanGraphicPipelineLayoutCreator() = default;
	
	virtual void inject_create_info(VkGraphicsPipelineCreateInfo* info) = 0;

	virtual std::expected<IGVulkanPipelineLayout*, LAYOUT_CREATOR_ERROR> create_layout_for(IGVulkanGraphicPipeline* pipeline) = 0;

	virtual std::expected<IGVulkanDescriptorPool*, LAYOUT_CREATOR_ERROR> create_descriptor_pool_and_sets(IGVulkanGraphicPipeline* pipeline, std::vector<VkDescriptorSet_T*>* descriptorSets) = 0;
	
	virtual void destroy() {};

	virtual bool own_sets() { return true; }
private:
};

#endif // IGVULKAN_GRAPHIC_PIPELINE_LAYOUT_CREATOR_H