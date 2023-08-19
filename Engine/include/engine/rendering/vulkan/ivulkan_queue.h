#ifndef IGVULKAN_QUEUE_H
#define IGVULKAN_QUEUE_H

struct VkQueue_T;
#include <cstdint>

class IGVulkanQueue
{
public:
	virtual ~IGVulkanQueue() = default;

	virtual VkQueue_T* get_queue() const noexcept = 0;

	virtual uint32_t get_queue_index() const noexcept = 0;

	virtual const char* get_all_supported_operations_as_string() const noexcept = 0;
private:
};


#endif // IGVULKAN_QUEUE_H