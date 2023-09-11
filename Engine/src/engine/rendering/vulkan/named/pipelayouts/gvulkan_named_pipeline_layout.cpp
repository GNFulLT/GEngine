#include "volk.h"
#include "internal/engine/rendering/vulkan/named/pipelayouts/gvulkan_named_pipeline_layout_camera.h"
#include "public/math/gmat4.h"
#include "engine/rendering/vulkan/ivulkan_ldevice.h"
#include <array>

GVulkanNamedPipelineLayoutCamera::GVulkanNamedPipelineLayoutCamera(IGVulkanLogicalDevice* dev, IGVulkanNamedSetLayout* namedLayout,const char* name)
{
	m_layout = nullptr;
	m_boundedDevice = dev;
	m_name = name;
	m_descriptorSetLayout = namedLayout;
}
bool GVulkanNamedPipelineLayoutCamera::init()
{
	//X First push constant
	VkPushConstantRange push_constant;

	//this push constant range starts at the beginning
	push_constant.offset = 0;
	//this push constant range takes up the size of a MeshPushConstants struct
	push_constant.size = sizeof(gmat4);
	//this push constant range is accessible only in the vertex shader
	push_constant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	VkDescriptorSetLayout layout = m_descriptorSetLayout->get_layout();

	VkPipelineLayoutCreateInfo inf = {};
	inf.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	inf.flags = 0;
	inf.setLayoutCount = 1;
	inf.pSetLayouts = &layout;
	inf.pushConstantRangeCount = 1;
	inf.pPushConstantRanges = &push_constant;


	auto res = vkCreatePipelineLayout(m_boundedDevice->get_vk_device(), &inf, nullptr, &m_layout);

	if (res != VK_SUCCESS)
	{
		return false;
	}

	return true;
}

void GVulkanNamedPipelineLayoutCamera::destroy()
{
	if (m_layout != nullptr)
	{
		vkDestroyPipelineLayout(m_boundedDevice->get_vk_device(), m_layout, nullptr);
		m_layout = nullptr;
	}
}

VkPipelineLayout_T* GVulkanNamedPipelineLayoutCamera::get_vk_pipeline_layout() const noexcept
{
	return m_layout;
}

IGVulkanNamedSetLayout* GVulkanNamedPipelineLayoutCamera::get_pipeline_set_layout() const noexcept
{
	return m_descriptorSetLayout;
}


