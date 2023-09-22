#include "volk.h"
#include "internal/engine/rendering/renderer/gscene_material_renderer_layout.h"
#include <unordered_map>
#include <array>
#include <cassert>
#include "engine/rendering/vulkan/igvulkan_uniform_buffer.h"
#include "engine/rendering/wireframe_spec.h"
#include "internal/engine/rendering/mesh/gmesh_renderable.h"
#include "internal/engine/rendering/vulkan/gvulkan_basic_pipeline_layout.h"


GSceneMaterialRendererLayout::GSceneMaterialRendererLayout(IGVulkanLogicalDevice* dev, VkDescriptorSetLayout_T* materialLayout,VkDescriptorSetLayout_T* layout, std::vector<VkDescriptorSet_T*>* descriptorSets)
{
	p_descriptorSets = descriptorSets;
	m_descriptorSetLayout = layout;
	p_boundedDevice = dev;
	m_materialLayout = materialLayout;
}

void GSceneMaterialRendererLayout::inject_create_info(VkGraphicsPipelineCreateInfo* info)
{
}

std::expected<IGVulkanPipelineLayout*, LAYOUT_CREATOR_ERROR> GSceneMaterialRendererLayout::create_layout_for(IGVulkanGraphicPipeline* pipeline)
{

	std::array<VkDescriptorSetLayout, 2> layouts;
	layouts[0] = m_descriptorSetLayout;
	layouts[1] = m_materialLayout;

	VkPipelineLayoutCreateInfo inf = {};
	inf.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	inf.flags = 0;
	inf.setLayoutCount = layouts.size();
	inf.pSetLayouts = layouts.data();
	inf.pushConstantRangeCount = 0;
	inf.pPushConstantRanges = nullptr;

	VkPipelineLayout layout;
	auto res = vkCreatePipelineLayout(p_boundedDevice->get_vk_device(), &inf, nullptr, &layout);
	assert(res == VK_SUCCESS);

	m_pipelineLayout = new GVulkanBasicPipelineLayout(p_boundedDevice, layout);
	return m_pipelineLayout;
}

std::expected<IGVulkanDescriptorPool*, LAYOUT_CREATOR_ERROR> GSceneMaterialRendererLayout::create_descriptor_pool_and_sets(IGVulkanGraphicPipeline* pipeline, std::vector<VkDescriptorSet_T*>* descriptorSets)
{
	*descriptorSets = *p_descriptorSets;
	return nullptr;
}

void GSceneMaterialRendererLayout::write_set_layout(uint32_t binding, uint32_t bufferType, VkDescriptorBufferInfo* info)
{

}

bool GSceneMaterialRendererLayout::own_sets()
{
	return false;
}

IGVulkanPipelineLayout* GSceneMaterialRendererLayout::get_pipeline_layout()
{
	return m_pipelineLayout;
}
