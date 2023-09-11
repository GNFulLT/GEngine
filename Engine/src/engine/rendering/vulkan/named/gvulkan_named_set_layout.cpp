#include "volk.h"
#include "internal/engine/rendering/vulkan/named/gvulkan_named_set_layout.h"
#include "engine/rendering/vulkan/ivulkan_ldevice.h"

GVulkanNamedSetLayout::GVulkanNamedSetLayout(IGVulkanLogicalDevice* dev,VkDescriptorSetLayout_T* layout, const char* name)
{
	m_layout = layout;
	m_boundedDevice = dev;
	m_name = name;
}

VkDescriptorSetLayout_T* GVulkanNamedSetLayout::get_layout() const noexcept
{
	return m_layout;
}

void GVulkanNamedSetLayout::destroy()
{
	if (m_layout == nullptr)
		return;
	vkDestroyDescriptorSetLayout(m_boundedDevice->get_vk_device(), m_layout, nullptr);
	m_layout = nullptr;
}
