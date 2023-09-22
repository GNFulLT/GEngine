#include "volk.h"
#include "internal/engine/rendering/renderer/gscene_composition_renderer_layout.h"
#include <array>
#include <cassert>

GSceneCompositionRendererLayout::GSceneCompositionRendererLayout(IGVulkanLogicalDevice* dev,uint32_t framesInFlight ,IVulkanImage* posImage, IVulkanImage* normalImage, IVulkanImage* albedoImage,VkSampler_T* sampler)
{
	m_sampler = sampler;
	m_framesInFlight = framesInFlight;
	p_boundedDevice = dev;
	m_posImage = posImage;
	m_normalImage = normalImage;
	m_albedoIamge = albedoImage;
}

void GSceneCompositionRendererLayout::inject_create_info(VkGraphicsPipelineCreateInfo* info)
{
}

std::expected<IGVulkanPipelineLayout*, LAYOUT_CREATOR_ERROR> GSceneCompositionRendererLayout::create_layout_for(IGVulkanGraphicPipeline* pipeline)
{
	VkPipelineLayoutCreateInfo inf = {};
	inf.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	inf.flags = 0;
	inf.setLayoutCount = 1;
	inf.pSetLayouts = &m_descriptorSetLayout;
	inf.pushConstantRangeCount = 0;
	inf.pPushConstantRanges = nullptr;

	VkPipelineLayout layout;
	auto res = vkCreatePipelineLayout(p_boundedDevice->get_vk_device(), &inf, nullptr, &layout);
	assert(res == VK_SUCCESS);

	m_pipelineLayout = new GVulkanBasicPipelineLayout(p_boundedDevice, layout);
	return m_pipelineLayout;
}

std::expected<IGVulkanDescriptorPool*, LAYOUT_CREATOR_ERROR> GSceneCompositionRendererLayout::create_descriptor_pool_and_sets(IGVulkanGraphicPipeline* pipeline, std::vector<VkDescriptorSet_T*>* descriptorSets)
{

	p_descriptorSets = descriptorSets;

	//X First create necessary pool
	std::unordered_map<VkDescriptorType, int> map;
	map.emplace(VkDescriptorType::VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 3);

	auto pool = p_boundedDevice->create_and_init_vector_pool(map, m_framesInFlight);

	std::array<VkDescriptorSetLayoutBinding, 3> bindings;
	//X Pos Buff
	bindings[0].binding = 0;
	bindings[0].descriptorCount = 1;
	bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	bindings[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	bindings[0].pImmutableSamplers = nullptr;

	//X Normal Buff
	bindings[1].binding = 1;
	bindings[1].descriptorCount = 1;
	bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	bindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	bindings[1].pImmutableSamplers = nullptr;

	//X	ALBEDO BUFF
	bindings[2].binding = 2;
	bindings[2].descriptorCount = 1;
	bindings[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	bindings[2].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	bindings[2].pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutCreateInfo setinfo = {};
	setinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	setinfo.pNext = nullptr;
	//we are going to have 1 binding
	setinfo.bindingCount = bindings.size();
	//no flags
	setinfo.flags = 0;
	//point to the camera buffer binding
	setinfo.pBindings = bindings.data();

	auto res = vkCreateDescriptorSetLayout(p_boundedDevice->get_vk_device(), &setinfo, nullptr, &m_descriptorSetLayout);
	assert(res == VK_SUCCESS);

	std::vector<VkDescriptorSetLayout> layouts(m_framesInFlight, m_descriptorSetLayout);

	//allocate one descriptor set for each frame
	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.pNext = nullptr;
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = pool->get_vk_descriptor_pool();
	allocInfo.descriptorSetCount = layouts.size();
	allocInfo.pSetLayouts = layouts.data();

	std::vector<VkDescriptorSet> vkdescriptorSets;
	vkdescriptorSets.resize(m_framesInFlight);

	assert(VK_SUCCESS == vkAllocateDescriptorSets(p_boundedDevice->get_vk_device(), &allocInfo, vkdescriptorSets.data()));
	for (int i = 0; i < vkdescriptorSets.size(); i++)
	{
		std::array<VkDescriptorImageInfo, 3> imageInfos;
		//it will be the camera buffer
		imageInfos[0].imageView = m_posImage->get_vk_image_view();
		imageInfos[0].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfos[0].sampler = m_sampler;

		imageInfos[1].imageView = m_normalImage->get_vk_image_view();
		imageInfos[1].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfos[1].sampler = m_sampler;

		imageInfos[2].imageView = m_albedoIamge->get_vk_image_view();
		imageInfos[2].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfos[2].sampler = m_sampler;

		std::array< VkWriteDescriptorSet, 3> setWrites;
		setWrites[0] = {};
		setWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		setWrites[0].pNext = nullptr;
		setWrites[0].dstBinding = 0;
		setWrites[0].dstSet = vkdescriptorSets[i];
		setWrites[0].descriptorCount = 1;
		setWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		setWrites[0].pImageInfo = &imageInfos[0];

		setWrites[1] = {};
		setWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		setWrites[1].pNext = nullptr;
		setWrites[1].dstBinding = 1;
		setWrites[1].dstSet = vkdescriptorSets[i];
		setWrites[1].descriptorCount = 1;
		setWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		setWrites[1].pImageInfo = &imageInfos[1];

		setWrites[2] = {};
		setWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		setWrites[2].pNext = nullptr;
		setWrites[2].dstBinding = 2;
		setWrites[2].dstSet = vkdescriptorSets[i];
		setWrites[2].descriptorCount = 1;
		setWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		setWrites[2].pImageInfo = &imageInfos[2];
		//-------------------
		vkUpdateDescriptorSets(p_boundedDevice->get_vk_device(), setWrites.size(), setWrites.data(), 0, 0);
	}

	(*descriptorSets) = vkdescriptorSets;
	return pool;
}

void GSceneCompositionRendererLayout::write_sets(IVulkanImage* posImage, IVulkanImage* normalImage, IVulkanImage* albedoImage)
{
	m_posImage = posImage;
	m_normalImage = normalImage;
	m_albedoIamge = albedoImage;

	for (int i = 0; i < p_descriptorSets->size(); i++)
	{
		std::array<VkDescriptorImageInfo, 3> imageInfos;
		//it will be the camera buffer
		imageInfos[0].imageView = m_posImage->get_vk_image_view();
		imageInfos[0].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfos[0].sampler = m_sampler;

		imageInfos[1].imageView = m_normalImage->get_vk_image_view();
		imageInfos[1].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfos[1].sampler = m_sampler;

		imageInfos[2].imageView = m_albedoIamge->get_vk_image_view();
		imageInfos[2].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfos[2].sampler = m_sampler;

		std::array< VkWriteDescriptorSet, 3> setWrites;
		setWrites[0] = {};
		setWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		setWrites[0].pNext = nullptr;
		setWrites[0].dstBinding = 0;
		setWrites[0].dstSet = (*p_descriptorSets)[i];
		setWrites[0].descriptorCount = 1;
		setWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		setWrites[0].pImageInfo = &imageInfos[0];

		setWrites[1] = {};
		setWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		setWrites[1].pNext = nullptr;
		setWrites[1].dstBinding = 1;
		setWrites[1].dstSet = (*p_descriptorSets)[i];
		setWrites[1].descriptorCount = 1;
		setWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		setWrites[1].pImageInfo = &imageInfos[1];

		setWrites[2] = {};
		setWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		setWrites[2].pNext = nullptr;
		setWrites[2].dstBinding = 2;
		setWrites[2].dstSet = (*p_descriptorSets)[i];
		setWrites[2].descriptorCount = 1;
		setWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		setWrites[2].pImageInfo = &imageInfos[2];
		//-------------------
		vkUpdateDescriptorSets(p_boundedDevice->get_vk_device(), setWrites.size(), setWrites.data(), 0, 0);
	}

}
