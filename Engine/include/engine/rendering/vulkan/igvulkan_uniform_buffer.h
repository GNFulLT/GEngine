#ifndef IGVULKAN_UNIFORM_BUFFER_H
#define IGVULKAN_UNIFORM_BUFFER_H

#include "engine/GEngine_EXPORT.h"
#include "engine/rendering/vulkan/ivulkan_buffer.h"

class ENGINE_API IGVulkanUniformBuffer : public IVulkanBuffer
{
public:
	virtual ~IGVulkanUniformBuffer() = default;

private:
};

#endif // IGVULKAN_UNIFORM_BUFFER_H