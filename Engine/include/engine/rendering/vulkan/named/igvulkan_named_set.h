#ifndef IGVULKAN_NAMED_SET_H
#define IGVULKAN_NAMED_SET_H


#include "engine/GEngine_EXPORT.h"
#include "engine/rendering/vulkan/named/igvulkan_named_set_layout.h"
struct VkDescriptorSet_T;

class ENGINE_API IGVulkanNamedSet
{
public:
	virtual ~IGVulkanNamedSet() = default;

	virtual VkDescriptorSet_T* get_set() const noexcept = 0;

	virtual void destroy() = 0;

	virtual IGVulkanNamedSetLayout* get_created_layout() const noexcept = 0;

	virtual const char* get_name() const noexcept = 0;

private:
};

#endif // IGVULKAN_NAMED_SET_LAYOUT_H