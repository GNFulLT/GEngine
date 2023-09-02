#include "volk.h"
#include "internal/engine/rendering/vulkan/gvulkan_vectorized_pipeline_layout.h"
#include "engine/rendering/vulkan/ivulkan_ldevice.h"

GVulkanVectorizedPipelineLayout::GVulkanVectorizedPipelineLayout(IGVulkanLogicalDevice* dev, const std::vector<VkDescriptorSetLayout_T*>& layouts, int flags)
{
	m_boundedDevice = dev;
	m_setLayouts = layouts;
	m_flags = flags;
}

bool GVulkanVectorizedPipelineLayout::init()
{
	VkPipelineLayoutCreateInfo inf = {};
	inf.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	inf.flags = m_flags;
	inf.setLayoutCount = m_setLayouts.size();
	inf.pSetLayouts = m_setLayouts.size() == 0 ? nullptr : m_setLayouts.data();
	inf.pushConstantRangeCount = 0;
	inf.pPushConstantRanges = 0;


	auto res = vkCreatePipelineLayout(m_boundedDevice->get_vk_device(), &inf, nullptr, &m_layout);
	return res == VK_SUCCESS;
}

VkPipelineLayout_T* GVulkanVectorizedPipelineLayout::get_vk_pipeline_layout()
{
	return m_layout;
}

void GVulkanVectorizedPipelineLayout::destroy()
{
	if (m_layout != nullptr)
	{
		vkDestroyPipelineLayout(m_boundedDevice->get_vk_device(), m_layout, nullptr);
		m_layout = nullptr;
	}
}
