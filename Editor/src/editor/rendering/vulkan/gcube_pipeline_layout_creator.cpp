#include <volk.h>
#include "internal/rendering/vulkan/gcube_pipeline_layout_creator.h"
#include "engine/rendering/vulkan/ivulkan_ldevice.h"
#include <unordered_map>
#include "engine/rendering/vulkan/ivulkan_descriptor_pool.h"
#include <array>
#include "engine/manager/igcamera_manager.h"
#include "engine/rendering/vulkan/igvulkan_uniform_buffer.h"
#include "engine/resource/igtexture_resource.h"
#include "engine/rendering/vulkan/ivulkan_image.h"
#include "engine/rendering/vulkan/ivulkan_sampler.h"
#include "public/math/gmat4.h"
#include "internal/rendering/vulkan/gvulkan_pipeline_layout_wrapper.h"

GCubePipelinelayoutCreator::GCubePipelinelayoutCreator(IGVulkanLogicalDevice* device, IGSceneManager* sceneManager, IGPipelineObjectManager* objManager,
	GSharedPtr<IGTextureResource> cubeTexture,uint32_t framesInFlight)
{
	m_boundedDevice = device;
	m_framesInFlight = framesInFlight;
	m_descriptorPool = nullptr;
	m_descriptorSetLayout = nullptr;
	m_sceneManager = sceneManager;
	m_cubeTexture = cubeTexture;
	m_objManager = objManager;
}

void GCubePipelinelayoutCreator::inject_create_info(VkGraphicsPipelineCreateInfo* info)
{

}

std::expected<IGVulkanPipelineLayout*, LAYOUT_CREATOR_ERROR> GCubePipelinelayoutCreator::create_layout_for(IGVulkanGraphicPipeline* pipeline)
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

	m_pipelineLayout = new GVulkanPipelineLayoutWrapper(m_boundedDevice, layout);
	return m_pipelineLayout;
}

std::expected<IGVulkanDescriptorPool*, LAYOUT_CREATOR_ERROR> GCubePipelinelayoutCreator::create_descriptor_pool_and_sets(IGVulkanGraphicPipeline* pipeline, std::vector<VkDescriptorSet_T*>* descriptorSets)
{
	//X Create the pool
	std::unordered_map<VkDescriptorType, int> map;
	map.emplace(VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2);
	map.emplace(VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1);

	m_descriptorPool = m_boundedDevice->create_and_init_vector_pool(map, m_framesInFlight);

	if (m_descriptorPool == nullptr)
		return std::unexpected(LAYOUT_CREATOR_ERROR_UNKNOWN);

	std::array<VkDescriptorSetLayoutBinding, 2> bindings;
	bindings[0].binding = 0;
	bindings[0].descriptorCount = 1;
	bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	bindings[0].pImmutableSamplers = nullptr;

	bindings[1].binding = 1;
	bindings[1].descriptorCount = 1;
	bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	bindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	bindings[1].pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutCreateInfo setinfo = {};
	setinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	setinfo.pNext = nullptr;
	setinfo.bindingCount = bindings.size();
	setinfo.flags = 0;
	setinfo.pBindings = bindings.data();

	auto res = vkCreateDescriptorSetLayout(m_boundedDevice->get_vk_device(), &setinfo, nullptr, &m_descriptorSetLayout);
	if (res != VK_SUCCESS)
		return std::unexpected(LAYOUT_CREATOR_ERROR_UNKNOWN);
	

	std::vector<VkDescriptorSetLayout> layouts(m_framesInFlight, m_descriptorSetLayout);

	//allocate one descriptor set for each frame
	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.pNext = nullptr;
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = m_descriptorPool->get_vk_descriptor_pool();
	allocInfo.descriptorSetCount = layouts.size();
	allocInfo.pSetLayouts = layouts.data();

	m_descriptorSets.resize(m_framesInFlight);

	vkAllocateDescriptorSets(m_boundedDevice->get_vk_device(), &allocInfo, m_descriptorSets.data());

	for (int i = 0; i < m_framesInFlight; i++)
	{
		VkDescriptorBufferInfo binfo;
		//it will be the camera buffer
		binfo.buffer = m_sceneManager->get_global_buffer_for_frame(i)->get_vk_buffer();
		//at 0 offset
		binfo.offset = 0;
		//of the size of a camera data struct
		binfo.range = m_sceneManager->get_global_buffer_for_frame(i)->get_size();

		VkDescriptorImageInfo iinfo = {};
		//X TODO CACHE AND USE SAMPLER BOUNDERS
		iinfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		iinfo.imageView = m_cubeTexture->get_vulkan_image()->get_vk_image_view();
		iinfo.sampler = m_objManager->get_named_sampler(IGPipelineObjectManager::MAX_PERFORMANT_SAMPLER.data())->get_vk_sampler();

		std::array<VkWriteDescriptorSet, 2> descriptorWrites;
		descriptorWrites[0] = {};
		descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[0].pNext = nullptr;
		descriptorWrites[0].dstBinding = 0;
		descriptorWrites[0].dstSet = m_descriptorSets[i];
		descriptorWrites[0].descriptorCount = 1;
		descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[0].pBufferInfo = &binfo;
		descriptorWrites[1] = {};
		descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[1].pNext = nullptr;
		descriptorWrites[1].dstBinding = 1;
		descriptorWrites[1].dstSet = m_descriptorSets[i];
		descriptorWrites[1].descriptorCount = 1;
		descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrites[1].pImageInfo = &iinfo;

		vkUpdateDescriptorSets(m_boundedDevice->get_vk_device(), descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);
	}
	(*descriptorSets) = m_descriptorSets;
	return m_descriptorPool;
}
