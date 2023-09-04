#ifndef IGVULKAN_FRAME_DATA_H
#define IGVULKAN_FRAME_DATA_H


#include "engine/GEngine_EXPORT.h"

class GVulkanCommandBuffer;
class GVulkanSemaphore;

class IGVulkanFrameData
{
public:
	virtual ~IGVulkanFrameData() = default;

	virtual GVulkanCommandBuffer* create_command_buffer_for_this_frame() = 0;

	virtual void add_wait_semaphore_for_this_frame(GVulkanSemaphore* waitSemaphore,int stage) = 0;

	virtual GVulkanCommandBuffer* get_the_main_cmd() =0;
private:
};
#endif // IGVULKAN_FRAME_DATA_H