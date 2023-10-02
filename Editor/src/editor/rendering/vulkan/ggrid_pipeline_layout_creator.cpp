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

GGridPipelineLayoutCreator::GGridPipelineLayoutCreator(IGVulkanLogicalDevice* device, IGSceneManager* sceneManager, IGPipelineObjectManager* objManager
	, uint32_t framesInFlight)
{
	m_boundedDevice = device;
	m_sceneManager = sceneManager;
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
	
	(*descriptorSets) = m_descriptorSets;
	return m_descriptorPool;
}
