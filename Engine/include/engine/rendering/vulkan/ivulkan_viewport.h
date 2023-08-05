#ifndef IGVULKAN_VIEWPORT_H
#define IGVULKAN_VIEWPORT_H

#include "engine/GEngine_EXPORT.h"
#include <cstdint>

class GVulkanCommandBuffer;
class GVulkanSemaphore;
class ENGINE_API IGVulkanViewport
{
public:
	virtual ~IGVulkanViewport() = default;

	virtual int get_current_image_index() const = 0;

	virtual void* get_vk_current_image_renderpass() = 0;

	virtual uint32_t get_width() const = 0;

	virtual uint32_t get_height() const = 0;

	virtual uint32_t get_total_image() const = 0;

	//X TODO : CHANGE THIS
	virtual void begin_draw_cmd(GVulkanCommandBuffer* cmd) = 0;
	virtual void end_draw_cmd(GVulkanCommandBuffer* cmd) = 0;

	virtual bool acquire_draw_image(GVulkanSemaphore* waitSemaphore) = 0;

	virtual bool present_image(uint32_t waitSemaphoreCount,GVulkanSemaphore* waitSemaphores) = 0;

	// Engine will call handle method by this return value
	virtual bool need_handle() = 0;

	virtual bool handle() = 0;
private:
};

#endif // IGVULKAN_VIEWPORT_H