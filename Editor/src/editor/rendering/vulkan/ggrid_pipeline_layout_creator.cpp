#include "volk.h"
#include "internal/rendering/vulkan/ggrid_pipeline_layout_creator.h"
#include "internal/rendering/vulkan/gvulkan_pipeline_layout_ref.h"
#include "engine/rendering/vulkan/ivulkan_ldevice.h"
#include "engine/manager/igcamera_manager.h"
#include "engine/rendering/vulkan/igvulkan_uniform_buffer.h"
#include "internal/rendering/grid_spec.h"
#include <unordered_map>
#include <array>
#include "public/math/gmat4.h"
#include "internal/rendering/vulkan/gvulkan_pipeline_layout_wrapper.h"

GGridPipelineLayoutCreator::GGridPipelineLayoutCreator(IGVulkanLogicalDevice* device, IGCameraManager* cameraManager, IGPipelineObjectManager* objManager
	, uint32_t framesInFlight)
{
	m_boundedDevice = device;
	m_cameraManager = cameraManager;
	m_framesInFlight = framesInFlight;
	m_pipelineObjectManager = objManager;
}

void GGridPipelineLayoutCreator::inject_create_info(VkGraphicsPipelineCreateInfo* info)
{
}

std::expected<IGVulkanPipelineLayout*, LAYOUT_CREATOR_ERROR> GGridPipelineLayoutCreator::create_layout_for(IGVulkanGraphicPipeline* pipeline)
{
	//X First push constant

	std::array<VkPushConstantRange,2> pushConstants;
	pushConstants[0].offset = 0;
	//this push constant range takes up the size of a MeshPushConstants struct
	pushConstants[0].size = sizeof(gmat4);
	//this push constant range is accessible only in the vertex shader
	pushConstants[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;


	pushConstants[1].offset = sizeof(gmat4);
	pushConstants[1].size = sizeof(GridSpec);
	pushConstants[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	VkPipelineLayoutCreateInfo inf = {};
	inf.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	inf.flags = 0;
	inf.setLayoutCount = 1;
	inf.pSetLayouts = &m_descriptorSetLayout;
	inf.pushConstantRangeCount = pushConstants.size();
	inf.pPushConstantRanges = pushConstants.data();

	VkPipelineLayout layout;
	auto res = vkCreatePipelineLayout(m_boundedDevice->get_vk_device(), &inf, nullptr, &layout);

	if (res != VK_SUCCESS)
	{
		return nullptr;
	}

	return  new GVulkanPipelineLayoutWrapper(m_boundedDevice, layout);
}

std::expected<IGVulkanDescriptorPool*, LAYOUT_CREATOR_ERROR> GGridPipelineLayoutCreator::create_descriptor_pool_and_sets(IGVulkanGraphicPipeline* pipeline, std::vector<VkDescriptorSet_T*>* descriptorSets)
{
	//X Create the pool
	std::unordered_map<VkDescriptorType, int> map;
	map.emplace(VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2);

	m_descriptorPool = m_boundedDevice->create_and_init_vector_pool(map, m_framesInFlight);

	if (m_descriptorPool == nullptr)
		return std::unexpected(LAYOUT_CREATOR_ERROR_UNKNOWN);

	auto camLayout = m_pipelineObjectManager->get_named_pipeline_layout(m_pipelineObjectManager->CAMERA_PIPE_LAYOUT.data());

	//X We wants same pipeline set layout
	m_descriptorSetLayout = camLayout->get_vk_pipeline_set_layout();

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
		binfo.buffer = m_cameraManager->get_camera_buffer_for_frame(i)->get_vk_buffer();
		//at 0 offset
		binfo.offset = 0;
		//of the size of a camera data struct
		binfo.range = m_cameraManager->get_camera_buffer_for_frame(i)->get_size();
	
		std::array<VkWriteDescriptorSet, 1> descriptorWrites;
		descriptorWrites[0] = {};
		descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[0].pNext = nullptr;
		descriptorWrites[0].dstBinding = 0;
		descriptorWrites[0].dstSet = m_descriptorSets[i];
		descriptorWrites[0].descriptorCount = 1;
		descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[0].pBufferInfo = &binfo;

		vkUpdateDescriptorSets(m_boundedDevice->get_vk_device(), descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);
	}
	(*descriptorSets) = m_descriptorSets;
	return m_descriptorPool;
}
