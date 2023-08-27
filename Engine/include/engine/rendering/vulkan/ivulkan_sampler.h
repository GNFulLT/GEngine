#ifndef ISAMPLER_H
#define ISAMPLER_H


#include "engine/GEngine_EXPORT.h"

class IGVulkanSamplerCreator;
class IGVulkanLogicalDevice;

struct VkSampler_T;

class ENGINE_API IGVulkanSampler
{
public:
	virtual ~IGVulkanSampler() = default;

	virtual IGVulkanSamplerCreator* get_creator() const noexcept = 0;


	virtual VkSampler_T* get_vk_sampler() const noexcept = 0;

	void destroy();

	virtual IGVulkanLogicalDevice* get_bounded_device() const noexcept = 0;
private:
};

#endif // ISAMPLER