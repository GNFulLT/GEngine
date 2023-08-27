#include "internal/engine/rendering/vulkan/gvulkan_sampler.h"
#include "engine/rendering/vulkan/ivulkan_sampler.h"
#include "engine/rendering/vulkan/ivulkan_sampler_creator.h"

void IGVulkanSampler::destroy()
{
	get_creator()->destroy_sampler(this);
}

GVulkanSampler::GVulkanSampler(IGVulkanSamplerCreator* creator,VkSampler_T* sampler, IGVulkanLogicalDevice* dev)
{
	m_creator = creator;
	m_sampler = sampler;
	m_device = dev;
}

IGVulkanSamplerCreator* GVulkanSampler::get_creator() const noexcept
{
	return m_creator;
}

VkSampler_T* GVulkanSampler::get_vk_sampler() const noexcept
{
	return m_sampler;
}

IGVulkanLogicalDevice* GVulkanSampler::get_bounded_device() const noexcept
{
	return m_device;
}
