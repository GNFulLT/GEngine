#include "volk.h"

#include "internal/engine/rendering/vulkan/gvulkan_default_sampler_creator.h"
#include "vma/vk_mem_alloc.h"
#include "engine/rendering/vulkan/ivulkan_ldevice.h"
#include "internal/engine/rendering/vulkan/gvulkan_sampler.h"

std::expected<IGVulkanSampler*, std::string> GDefaultSamplerCreator::create_sampler(IGVulkanLogicalDevice* dev)
{
	
	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_NEAREST;
	samplerInfo.minFilter = VK_FILTER_NEAREST;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.anisotropyEnable = VK_FALSE;
	samplerInfo.maxAnisotropy = 0;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = VK_LOD_CLAMP_NONE;
	

	VkSampler sampler = 0;
	auto res = vkCreateSampler((VkDevice)dev->get_vk_device(), &samplerInfo, nullptr, &sampler);
	if (VkResult::VK_SUCCESS != res)
	{
		return std::unexpected("Unknown error while trying to create sampler");
	}

	//X TODO : GDNEWDA
	return new GVulkanSampler(this,sampler,dev);
}

void GDefaultSamplerCreator::destroy_sampler(IGVulkanSampler* sampler)
{
	vkDestroySampler(sampler->get_bounded_device()->get_vk_device(), sampler->get_vk_sampler(), nullptr);
	delete sampler;
}

const char* GDefaultSamplerCreator::get_name()
{
	return nullptr;
}
