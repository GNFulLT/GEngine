#ifndef ISCENE_MANAGER_H
#define ISCENE_MANAGER_H


#include "engine/GEngine_EXPORT.h"
#include <cstdint>

class IGVulkanUniformBuffer;

class ENGINE_API IGSceneManager
{
public:
	virtual ~IGSceneManager() = default;

	virtual IGVulkanUniformBuffer* get_global_buffer_for_frame(uint32_t frame) const noexcept = 0;

	virtual bool init(uint32_t framesInFlight) = 0;

	virtual void reconstruct_global_buffer_for_frame(uint32_t frame) = 0;

	virtual void destroy() = 0;

};
#endif // ISCENE_MANAGER_H