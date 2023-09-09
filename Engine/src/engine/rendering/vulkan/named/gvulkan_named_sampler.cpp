#include "volk.h"
#include "internal/engine/rendering/vulkan/named/gvulkan_named_sampler.h"
#include "engine/rendering/vulkan/ivulkan_ldevice.h"
GVulkanNamedSampler::GVulkanNamedSampler(IGVulkanLogicalDevice* boundedDevice, const char* name,VkSampler_T* sampler)
{
	m_boundedDevice = boundedDevice;
	m_sampler = sampler;
	m_name = name;
}

void GVulkanNamedSampler::destroy()
{
	if (m_sampler == nullptr)
		return;
	vkDestroySampler(m_boundedDevice->get_vk_device(), m_sampler, nullptr);
	m_sampler = nullptr;
}

VkSampler_T* GVulkanNamedSampler::get_vk_sampler() const noexcept
{
	return m_sampler;
}
