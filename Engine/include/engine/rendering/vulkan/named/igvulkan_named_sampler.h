#ifndef IGVULKAN_NAMED_SAMPLER_H
#define IGVULKAN_NAMED_SAMPLER_H


#include "engine/GEngine_EXPORT.h"


struct VkSampler_T;

class ENGINE_API IGVulkanNamedSampler
{
public:
	virtual ~IGVulkanNamedSampler() = default;

	virtual VkSampler_T* get_vk_sampler() const noexcept = 0;
private:
};

#endif // IGVULKAN_SAMPLER_H