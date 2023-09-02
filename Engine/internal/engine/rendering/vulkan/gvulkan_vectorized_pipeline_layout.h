#ifndef GVULKAN_VECTORIZED_PIPELINE_LAYOUT_H
#define GVULKAN_VECTORIZED_PIPELINE_LAYOUT_H


#include "engine/rendering/vulkan/ivulkan_pipeline_layout.h"
#include <vector>

class IGVulkanLogicalDevice;
struct VkDescriptorSetLayout_T;

class GVulkanVectorizedPipelineLayout : public IGVulkanPipelineLayout
{
public:
	GVulkanVectorizedPipelineLayout(IGVulkanLogicalDevice* dev, const std::vector<VkDescriptorSetLayout_T*>& layouts, int flags = 0);

	bool init();

	// Inherited via IGVulkanPipelineLayout
	virtual VkPipelineLayout_T* get_vk_pipeline_layout() override;
	virtual void destroy() override;
private:
	IGVulkanLogicalDevice* m_boundedDevice;
	std::vector<VkDescriptorSetLayout_T*> m_setLayouts;
	VkPipelineLayout_T* m_layout;
	int m_flags;

};

#endif // GVULKAN_VECTORIZED_PIPELINE_LAYOUT_H