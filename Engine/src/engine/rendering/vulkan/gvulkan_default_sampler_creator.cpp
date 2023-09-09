#include "volk.h"

#include "internal/engine/rendering/vulkan/gvulkan_default_sampler_creator.h"
#include "vma/vk_mem_alloc.h"
#include "engine/rendering/vulkan/ivulkan_ldevice.h"
#include "internal/engine/rendering/vulkan/gvulkan_sampler.h"

GDefaultSamplerCreator::GDefaultSamplerCreator(IGPipelineObjectManager* obj)
{
	m_maxPerformantSampler = obj->get_named_sampler(obj->MAX_PERFORMANT_SAMPLER.data());
}

std::expected<IGVulkanSampler*, std::string> GDefaultSamplerCreator::create_sampler(IGVulkanLogicalDevice* dev)
{
	//X TODO : GDNEWDA
	return new GVulkanSampler(this, m_maxPerformantSampler->get_vk_sampler(), dev);
}

void GDefaultSamplerCreator::destroy_sampler(IGVulkanSampler* sampler)
{
	delete sampler;
}

const char* GDefaultSamplerCreator::get_name()
{
	return nullptr;
}
