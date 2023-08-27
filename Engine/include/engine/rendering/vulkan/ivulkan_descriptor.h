#ifndef IVULKAN_DESCRIPTOR_H
#define IVULKAN_DESCRIPTOR_H

#include "engine/GEngine_EXPORT.h"
class IGVulkanDescriptorCreator;

struct VkDescriptorSet_T;

class ENGINE_API IGVulkanDescriptorSet
{
public:
	virtual ~IGVulkanDescriptorSet() = default;

	virtual IGVulkanDescriptorCreator* get_creator() = 0;
	
	virtual void destroy_dtor() = 0;
	
	virtual VkDescriptorSet_T* get_vk_descriptor() = 0;
private:
};

#endif // IVULKAN_DESCRIPTOR_H