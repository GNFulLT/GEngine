#include "volk.h"
#include "internal/engine/rendering/renderer/gdeferred_renderer_layout.h"
#include <unordered_map>
#include <array>
#include <cassert>
#include "engine/rendering/vulkan/igvulkan_uniform_buffer.h"
#include "engine/rendering/wireframe_spec.h"
#include "internal/engine/rendering/mesh/gmesh_renderable.h"
#include "internal/engine/rendering/vulkan/gvulkan_basic_pipeline_layout.h"
GSceneDeferredRendererLayout::GSceneDeferredRendererLayout(IGVulkanLogicalDevice* dev, VkDescriptorSetLayout_T* layout, VkDescriptorSetLayout_T* textureLayout, std::vector<VkDescriptorSet_T*>* descriptorSets)
{
	m_textureLayout = textureLayout;
	p_descriptorSets = descriptorSets;
	m_descriptorSetLayout = layout;
	p_boundedDevice = dev;
	m_pipelineLayout = nullptr;
}

void GSceneDeferredRendererLayout::inject_create_info(VkGraphicsPipelineCreateInfo* info)
{
}

std::expected<IGVulkanPipelineLayout*, LAYOUT_CREATOR_ERROR> GSceneDeferredRendererLayout::create_layout_for(IGVulkanGraphicPipeline* pipeline)
{
	std::array<VkDescriptorSetLayout, 2> setLayouts;
	setLayouts[0] = m_descriptorSetLayout;
	setLayouts[1] = m_textureLayout;

	VkPipelineLayoutCreateInfo inf = {};
	inf.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	inf.flags = 0;
	inf.setLayoutCount = setLayouts.size();
	inf.pSetLayouts = setLayouts.data();
	inf.pushConstantRangeCount = 0;
	inf.pPushConstantRanges = nullptr;

	VkPipelineLayout layout;
	auto res = vkCreatePipelineLayout(p_boundedDevice->get_vk_device(), &inf, nullptr, &layout);
	assert(res == VK_SUCCESS);
	m_pipelineLayout = new GVulkanBasicPipelineLayout(p_boundedDevice, layout);
	m_pipeLayout = layout;
	return m_pipelineLayout;
}

std::expected<IGVulkanDescriptorPool*, LAYOUT_CREATOR_ERROR> GSceneDeferredRendererLayout::create_descriptor_pool_and_sets(IGVulkanGraphicPipeline* pipeline, std::vector<VkDescriptorSet_T*>* descriptorSets)
{
	*descriptorSets = *p_descriptorSets;
	return nullptr;
}

void GSceneDeferredRendererLayout::write_set_layout(uint32_t binding, uint32_t bufferType, VkDescriptorBufferInfo* info)
{

}

bool GSceneDeferredRendererLayout::own_sets()
{
	return false;
}

VkPipelineLayout_T* GSceneDeferredRendererLayout::get_pipeline_layout()
{
	return m_pipelineLayout->get_vk_pipeline_layout();
}

VkDescriptorSet_T* GSceneDeferredRendererLayout::get_set_by_index(uint32_t frameIndex)
{
	return (*p_descriptorSets)[frameIndex];
}
