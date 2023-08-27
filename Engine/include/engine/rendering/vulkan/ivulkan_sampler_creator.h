#ifndef ISAMPLER_CREATOR_H
#define ISAMPLER_CREATOR_H

#include "engine/GEngine_EXPORT.h"

class IGVulkanSampler;

#include <expected>
#include <string>

class IGVulkanLogicalDevice;


class ENGINE_API IGVulkanSamplerCreator
{
public:
	~IGVulkanSamplerCreator() = default;


	virtual std::expected<IGVulkanSampler* ,std::string> create_sampler(IGVulkanLogicalDevice* dev) = 0;

	virtual void destroy_sampler(IGVulkanSampler* sampler) = 0;

	virtual const char* get_name() = 0;
private:
};

#endif // ISAMPLER_CREATOR_H