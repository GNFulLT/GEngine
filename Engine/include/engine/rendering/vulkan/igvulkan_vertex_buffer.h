#ifndef IGVULKAN_VERTEX_BUFFER_H
#define IGVULKAN_VERTEX_BUFFER_H


#include "engine/GEngine_EXPORT.h"
#include "engine/rendering/vulkan/ivulkan_buffer.h"

class ENGINE_API IGVulkanVertexBuffer : public IVulkanBuffer
{
public:
	virtual ~IGVulkanVertexBuffer() = default;
private:
};

#endif // IGVULKAN_VERTEX_BUFFER_H