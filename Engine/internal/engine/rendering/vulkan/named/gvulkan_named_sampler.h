#ifndef GVULKAN_NAMED_SAMPLER_H
#define GVULKAN_NAMED_SAMPLER_H

#include "engine/rendering/vulkan/named/igvulkan_named_sampler.h"
#include <string>

class IGVulkanLogicalDevice;

class GVulkanNamedSampler : public IGVulkanNamedSampler
{
public:
	GVulkanNamedSampler(IGVulkanLogicalDevice* boundedDevice, const char* name,VkSampler_T* sampler);

	void destroy();
	
	virtual VkSampler_T* get_vk_sampler() const noexcept override;
private:
	IGVulkanLogicalDevice* m_boundedDevice;
	VkSampler_T* m_sampler;
	std::string m_name;
};

#endif // GVULKAN_NAMED_SAMPLER_H