#ifndef IGVULKAN_INDIRECT_BUFFER_H
#define IGVULKAN_INDIRECT_BUFFER_H

#include "engine/GEngine_EXPORT.h"
#include "engine/rendering/vulkan/ivulkan_buffer.h"

class ENGINE_API IGVulkanIndirectBuffer : public IVulkanBuffer
{
public:
	virtual ~IGVulkanIndirectBuffer() = default;

private:
};

#endif // IGVULKAN_INDIRECT_BUFFER_H