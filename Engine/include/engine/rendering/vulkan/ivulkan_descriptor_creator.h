#ifndef IVULKAN_DESCRIPTOR_CREATOR_H
#define IVULKAN_DESCRIPTOR_CREATOR_H


#include "engine/GEngine_EXPORT.h"
#include <expected>
#include <string>
class IGVulkanLogicalDevice;
class IGVulkanDescriptorSet;
class IVulkanImage;
struct VkSampler_T;



class ENGINE_API IGVulkanDescriptorCreator
{
public:
	~IGVulkanDescriptorCreator() = default;

	virtual std::expected<IGVulkanDescriptorSet*,std::string> create_descriptor_set_for_texture(IVulkanImage* image,VkSampler_T* sampler) = 0;

	virtual void destroy_descriptor_set_dtor(IGVulkanDescriptorSet* set) = 0;
	virtual IGVulkanLogicalDevice* get_bounded_device() = 0;

private:
};

#endif // IVULKAN_DESCRIPTOR_CREATOR_H