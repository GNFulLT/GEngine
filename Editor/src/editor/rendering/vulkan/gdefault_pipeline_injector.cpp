#include "internal/rendering/vulkan/gdefault_pipeline_injector.h"
#include "engine/rendering/vulkan/ivulkan_ldevice.h"
#include "public/math/gmat4.h"
#include "internal/rendering/vulkan/gvulkan_pipeline_layout_wrapper.h"
GDefaultPipelineInjector::GDefaultPipelineInjector(IGVulkanLogicalDevice* boundedDevice)
{
	m_boundedDevice = boundedDevice;
}

void GDefaultPipelineInjector::inject_create_info(VkGraphicsPipelineCreateInfo* info)
{
}

std::expected<IGVulkanPipelineLayout*, LAYOUT_CREATOR_ERROR> GDefaultPipelineInjector::create_layout_for(IGVulkanGraphicPipeline* pipeline)
{
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
	inf.setLayoutCount = 0;
	inf.pSetLayouts = nullptr;
	inf.pushConstantRangeCount = 1;
	inf.pPushConstantRanges = &push_constant;

	VkPipelineLayout layout;

	auto res = vkCreatePipelineLayout(m_boundedDevice->get_vk_device(), &inf, nullptr, &layout); 
	
	if (res != VK_SUCCESS)
	{
		return std::unexpected(LAYOUT_CREATOR_ERROR_UNKNOWN);
	}

	return new GVulkanPipelineLayoutWrapper(m_boundedDevice, layout);
}

std::expected<IGVulkanDescriptorPool*, LAYOUT_CREATOR_ERROR> GDefaultPipelineInjector::create_descriptor_pool_and_sets(IGVulkanGraphicPipeline* pipeline, std::vector<VkDescriptorSet_T*>* descriptorSets)
{
	return (IGVulkanDescriptorPool*)nullptr;
}
