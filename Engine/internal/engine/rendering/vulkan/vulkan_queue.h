#ifndef VULKAN_QUEUE_H
#define VULKAN_QUEUE_H

#include "engine/rendering/vulkan/ivulkan_queue.h"
#include <vulkan/vulkan.h>
#include <string>
class GVulkanLogicalDevice;
class GVulkanPhysicalDevice;


class GVulkanQueue : public IGVulkanQueue
{
public:
	GVulkanQueue();
	GVulkanQueue(GVulkanLogicalDevice* inDevice,uint32_t familyIndex,uint32_t queueIndex = 0);

	virtual VkQueue_T* get_queue() const noexcept override;

	bool is_valid() const;

	virtual uint32_t get_queue_index() const noexcept override;
	virtual const char* get_all_supported_operations_as_string() const noexcept override;

private:
	GVulkanLogicalDevice* m_device;
	uint32_t m_familyIndex;
	VkQueue_T* m_queue;
	VkQueueFamilyProperties m_properties;
	std::string m_allQueues;

};

#endif // VULKAN_QUEUE_H