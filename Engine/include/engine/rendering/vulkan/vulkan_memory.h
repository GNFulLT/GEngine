#ifndef GVULKAN_FENCE_MANAGER_H
#define GVULKAN_FENCE_MANAGER_H

class GVulkanFence;
class IGVulkanLogicalDevice;
class GVulkanFenceManager;
class GVulkanSemaphore;
class GVulkanSemaphoreManager;
struct VkFence_T;
struct VkSemaphore_T;

#include "engine/GEngine_EXPORT.h"

class ENGINE_API GVulkanSemaphore
{
public:
	GVulkanSemaphore(GVulkanSemaphoreManager* owner);
	~GVulkanSemaphore();
	bool init();

	void destroy();

	VkSemaphore_T* get_semaphore();
private:
	VkSemaphore_T* m_semaphore;
	GVulkanSemaphoreManager* m_owner;
};


class ENGINE_API GVulkanSemaphoreManager
{
public:
	GVulkanSemaphoreManager(IGVulkanLogicalDevice* owner);

	IGVulkanLogicalDevice* get_bounded_device();

	GVulkanSemaphore* create_semaphore();
private:
	IGVulkanLogicalDevice* m_owner;
};

enum FENCE_WAIT
{
	FENCE_WAIT_SUCCESS,
	FENCE_WAIT_TIMEOUT,
	FENCE_WAIT_OUT_OF_MEMORY
};

class ENGINE_API GVulkanFence
{
public:
	GVulkanFence(GVulkanFenceManager* owner);

	bool init(bool signaled);

	void destroy();

	bool reset();
	
	void wait();

	FENCE_WAIT wait_for(uint64_t time);

	VkFence_T* get_fence();
	~GVulkanFence();
private:
	VkFence_T* m_fence;
	GVulkanFenceManager* m_owner;
};

class ENGINE_API GVulkanFenceManager
{
public:
	GVulkanFenceManager(IGVulkanLogicalDevice* dev);
	~GVulkanFenceManager();
	IGVulkanLogicalDevice* get_bounded_device();
	GVulkanFence* create_fence();
private:
	IGVulkanLogicalDevice* m_device;
};


#endif // GVULKAN_FENCE_MANAGER_H