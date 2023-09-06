#include "volk.h"
#include "internal/engine/rendering/vulkan/gvulkan_basic_pipeline_layout.h"
#include "engine/rendering/vulkan/ivulkan_ldevice.h"

GVulkanBasicPipelineLayout::GVulkanBasicPipelineLayout(IGVulkanLogicalDevice* dev, VkPipelineLayout_T* layout)
{
	m_layout = layout;
	m_boundedDevice = dev;
}

VkPipelineLayout_T* GVulkanBasicPipelineLayout::get_vk_pipeline_layout()
{
	return m_layout;
	
}

void GVulkanBasicPipelineLayout::destroy()
{
	if (m_layout != nullptr)
	{
		vkDestroyPipelineLayout(m_boundedDevice->get_vk_device(), m_layout, nullptr);
		m_layout = nullptr;
	}
}
