#include "volk.h"
#include "internal/rendering/vulkan/gvulkan_pipeline_layout_wrapper.h"
#include "engine/rendering/vulkan/ivulkan_ldevice.h"

GVulkanPipelineLayoutWrapper::GVulkanPipelineLayoutWrapper(IGVulkanLogicalDevice* dev,VkPipelineLayout_T* layout)
{
	m_layout = layout;
	m_boundedDevice = dev;
}

VkPipelineLayout_T* GVulkanPipelineLayoutWrapper::get_vk_pipeline_layout()
{
	return m_layout;
}

void GVulkanPipelineLayoutWrapper::destroy()
{
	if (m_layout != nullptr)
	{
		vkDestroyPipelineLayout(m_boundedDevice->get_vk_device(), m_layout, nullptr);
		m_layout = nullptr;
	}
}
