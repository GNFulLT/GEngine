#include "volk.h"
#include "internal/engine/rendering/vulkan/named/pipelayouts/gvulkan_named_pipeline_layout_camera.h"
#include "public/math/gmat4.h"
#include "engine/rendering/vulkan/ivulkan_ldevice.h"
#include <array>

GVulkanNamedPipelineLayoutCamera::GVulkanNamedPipelineLayoutCamera(IGVulkanLogicalDevice* dev,const char* name)
{
	m_layout = nullptr;
	m_name = name;
}
bool GVulkanNamedPipelineLayoutCamera::init()
{
	std::array<VkDescriptorSetLayoutBinding, 1> bindings;
	bindings[0].binding = 0;
	bindings[0].descriptorCount = 1;
	bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	bindings[0].pImmutableSamplers = nullptr;


	VkDescriptorSetLayoutCreateInfo setinfo = {};
	setinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	setinfo.pNext = nullptr;
	//we are going to have 1 binding
	setinfo.bindingCount = bindings.size();
	//no flags
	setinfo.flags = 0;
	//point to the camera buffer binding
	setinfo.pBindings = bindings.data();

	auto res = vkCreateDescriptorSetLayout(m_boundedDevice->get_vk_device(), &setinfo, nullptr, &m_descriptorSetLayout);

	//X First push constant
	VkPushConstantRange push_constant;

	//this push constant range starts at the beginning
	push_constant.offset = 0;
	//this push constant range takes up the size of a MeshPushConstants struct
	push_constant.size = sizeof(gmat4);
	//this push constant range is accessible only in the vertex shader
	push_constant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	VkPipelineLayoutCreateInfo inf = {};
	inf.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	inf.flags = 0;
	inf.setLayoutCount = 1;
	inf.pSetLayouts = &m_descriptorSetLayout;
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
	if (m_descriptorSetLayout != nullptr)
	{
		vkDestroyDescriptorSetLayout(m_boundedDevice->get_vk_device(),m_descriptorSetLayout,nullptr);
		m_descriptorSetLayout = nullptr;
	}
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
