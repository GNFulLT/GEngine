#ifndef GVULKAN_SAMPLER_H
#define GVULKAN_SAMPLER_H

#include "engine/rendering/vulkan/ivulkan_sampler.h"
class GVulkanSampler : public IGVulkanSampler
{
public:
	GVulkanSampler(IGVulkanSamplerCreator* creator,VkSampler_T* sampler,IGVulkanLogicalDevice* dev);
	// Inherited via IGVulkanSampler
	virtual IGVulkanSamplerCreator* get_creator() const noexcept override;
	virtual VkSampler_T* get_vk_sampler() const noexcept override;
	virtual IGVulkanLogicalDevice* get_bounded_device() const noexcept override;
private:
	IGVulkanSamplerCreator* m_creator;
	VkSampler_T* m_sampler;
	IGVulkanLogicalDevice* m_device;
	
};

#endif // GVULKAN_SAMPLER_H