#include "volk.h"

#include "internal/engine/rendering/vulkan/gvulkan_pipeline_layout.h"
#include "engine/rendering/vulkan/ivulkan_ldevice.h"

GVulkanPipelineLayout::GVulkanPipelineLayout(IGVulkanLogicalDevice* dev, VkDescriptorSetLayout_T* setLayout,int flags)
{
	m_boundedDevice = dev;
	m_layout = nullptr;
	m_flags = flags;
	m_setLayout = setLayout;

}

bool GVulkanPipelineLayout::init()
{
	VkPipelineLayoutCreateInfo inf = {};
	inf.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	inf.flags = m_flags;
	inf.setLayoutCount = 1;
	inf.pSetLayouts = &m_setLayout;
	inf.pushConstantRangeCount = 0;
	inf.pPushConstantRanges = 0;


	auto res = vkCreatePipelineLayout(m_boundedDevice->get_vk_device(), &inf, nullptr, &m_layout);
	return res == VK_SUCCESS;
}

VkPipelineLayout_T* GVulkanPipelineLayout::get_vk_pipeline_layout()
{
	return m_layout;
}

void GVulkanPipelineLayout::destroy()
{
	if (m_layout != nullptr)
	{
		vkDestroyPipelineLayout(m_boundedDevice->get_vk_device(), m_layout, nullptr);
		m_layout = nullptr;
	}
}
