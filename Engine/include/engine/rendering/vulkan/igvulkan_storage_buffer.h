#ifndef IGVULKAN_STORAGE_BUFFER_H
#define IGVULKAN_STORAGE_BUFFER_H

#include "engine/GEngine_EXPORT.h"
#include "engine/rendering/vulkan/ivulkan_buffer.h"

class ENGINE_API IGVulkanStorageBuffer : public IVulkanBuffer
{
public:
	virtual ~IGVulkanStorageBuffer() = default;

private:
};

#endif // IGVULKAN_STORAGE_BUFFER_H