#ifndef GVULKAN_NAMED_PIPELINE_LAYOUT_H
#define GVULKAN_NAMED_PIPELINE_LAYOUT_H


struct VkDescriptorSetLayout_T;
class IGVulkanLogicalDevice;
#include <string>
#include "engine/rendering/vulkan/named/igvulkan_named_pipeline_layout.h"

class GVulkanNamedPipelineLayoutCamera : public IGVulkanNamedPipelineLayout
{
public:
	GVulkanNamedPipelineLayoutCamera(IGVulkanLogicalDevice* dev, IGVulkanNamedSetLayout* namedLayout,const char* name);
	bool init();
	virtual void destroy() override;
	virtual VkPipelineLayout_T* get_vk_pipeline_layout() const noexcept override;
	virtual IGVulkanNamedSetLayout* get_pipeline_set_layout() const noexcept override;
private:
	IGVulkanLogicalDevice* m_boundedDevice;
	VkPipelineLayout_T* m_layout;
	IGVulkanNamedSetLayout* m_descriptorSetLayout;
	std::string m_name;



};

#endif // GVULKAN_NAMED_PIPELINE_LAYOUT_H