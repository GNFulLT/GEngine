#include "internal/engine/rendering/vulkan/named/gvulkan_named_pipeline_layout.h"

GVulkanNamedPipelineLayout::GVulkanNamedPipelineLayout(IGVulkanLogicalDevice* dev,const char* name, VkPipelineLayout_T* layout)
{
	m_name  = name;
	m_layout = layout;
	p_device = dev;
}

VkPipelineLayout_T* GVulkanNamedPipelineLayout::get_vk_pipeline_layout() const noexcept
{
	return m_layout;
}

void GVulkanNamedPipelineLayout::destroy()
{
	if (m_layout != nullptr)
	{
		vkDestroyPipelineLayout(p_device->get_vk_device(), m_layout, nullptr);
		m_layout = nullptr;
	}
}
