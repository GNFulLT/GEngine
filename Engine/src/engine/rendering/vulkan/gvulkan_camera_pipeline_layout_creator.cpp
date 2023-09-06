#include "volk.h"

#include "internal/engine/rendering/vulkan/gvulkan_camera_pipeline_layout_creator.h"
#include "engine/rendering/vulkan/ivulkan_ldevice.h"
#include "public/math/gmat4.h"
#include <array>
#include "internal/engine/rendering/vulkan/gvulkan_basic_pipeline_layout.h"

GVulkanCameraLayoutCreator::GVulkanCameraLayoutCreator(IGVulkanLogicalDevice* boundedDevice, std::vector<IGVulkanUniformBuffer*> cameraBuff,uint32_t frameInFlight)
{
	m_camBuff = cameraBuff;
	m_boundedDevice = boundedDevice;
	m_frameInFlight = frameInFlight;
	m_descriptorSetLayout = nullptr;
	m_pipelineLayout = nullptr;
}

void GVulkanCameraLayoutCreator::inject_create_info(VkGraphicsPipelineCreateInfo* info)
{
}

std::expected<IGVulkanPipelineLayout*, LAYOUT_CREATOR_ERROR> GVulkanCameraLayoutCreator::create_layout_for(IGVulkanGraphicPipeline* pipeline)
{
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

	VkPipelineLayout layout;

	auto res = vkCreatePipelineLayout(m_boundedDevice->get_vk_device(), &inf, nullptr, &layout);

	if (res != VK_SUCCESS)
	{
		return std::unexpected(LAYOUT_CREATOR_ERROR_UNKNOWN);
	}

	m_pipelineLayout =  new GVulkanBasicPipelineLayout(m_boundedDevice, layout);
	return m_pipelineLayout;
}

std::expected<IGVulkanDescriptorPool*, LAYOUT_CREATOR_ERROR> GVulkanCameraLayoutCreator::create_descriptor_pool_and_sets(IGVulkanGraphicPipeline* pipeline, std::vector<VkDescriptorSet_T*>* descriptorSets)
{
	//X First create the pool for cam
	std::unordered_map<VkDescriptorType,int> map;
	map.emplace(VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2);
	auto pool = m_boundedDevice->create_and_init_vector_pool(map,m_frameInFlight);
	if (pool == nullptr)
	{
		return std::unexpected(LAYOUT_CREATOR_ERROR_UNKNOWN);
	}

	//X Create the layout
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
	setinfo.pBindings =bindings.data();

	auto res = vkCreateDescriptorSetLayout(m_boundedDevice->get_vk_device(), &setinfo, nullptr, &m_descriptorSetLayout);


	std::vector<VkDescriptorSetLayout> layouts(m_frameInFlight,m_descriptorSetLayout);

	//allocate one descriptor set for each frame
	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.pNext = nullptr;
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = pool->get_vk_descriptor_pool();
	allocInfo.descriptorSetCount = layouts.size();
	allocInfo.pSetLayouts = layouts.data();

	m_descriptorSets.resize(m_frameInFlight);

	vkAllocateDescriptorSets(m_boundedDevice->get_vk_device(), &allocInfo, m_descriptorSets.data());

	
	// Update sets

	for (int i = 0; i < m_frameInFlight; i++)
	{
		VkDescriptorBufferInfo binfo;
		//it will be the camera buffer
		binfo.buffer = m_camBuff[i]->get_vk_buffer();
		//at 0 offset
		binfo.offset = 0;
		//of the size of a camera data struct
		binfo.range = sizeof(gmat4);

		VkWriteDescriptorSet setWrite = {};
		setWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		setWrite.pNext = nullptr;

		setWrite.dstBinding = 0;
		//of the global descriptor
		setWrite.dstSet = m_descriptorSets[i];

		setWrite.descriptorCount = 1;
		setWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		setWrite.pBufferInfo = &binfo;


		vkUpdateDescriptorSets(m_boundedDevice->get_vk_device(), 1, &setWrite, 0, nullptr);
	}
	(*descriptorSets) = m_descriptorSets;
	return pool;
}

void GVulkanCameraLayoutCreator::destroy()
{
	if (m_descriptorSetLayout != nullptr)
	{
		vkDestroyDescriptorSetLayout(m_boundedDevice->get_vk_device(), m_descriptorSetLayout, nullptr);
		m_descriptorSetLayout = nullptr;
	}
}
