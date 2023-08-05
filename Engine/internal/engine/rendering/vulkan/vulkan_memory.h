#ifndef GVULKAN_FENCE_MANAGER_H
#define GVULKAN_FENCE_MANAGER_H

class GVulkanFence;
class GVulkanLogicalDevice;
class GVulkanFenceManager;
class GVulkanSemaphore;
class GVulkanSemaphoreManager;
struct VkFence_T;
struct VkSemaphore_T;

class GVulkanSemaphore
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


class GVulkanSemaphoreManager
{
public:
	GVulkanSemaphoreManager(GVulkanLogicalDevice* owner);

	GVulkanLogicalDevice* get_bounded_device();

	GVulkanSemaphore* create_semaphore();
private:
	GVulkanLogicalDevice* m_owner;
};

class GVulkanFence
{
public:
	GVulkanFence(GVulkanFenceManager* owner);

	bool init(bool signaled);

	void destroy();

	bool reset();
	
	void wait();


	VkFence_T* get_fence();
	~GVulkanFence();
private:
	VkFence_T* m_fence;
	GVulkanFenceManager* m_owner;
};

class GVulkanFenceManager
{
public:
	GVulkanFenceManager(GVulkanLogicalDevice* dev);
	~GVulkanFenceManager();
	GVulkanLogicalDevice* get_bounded_device();
	GVulkanFence* create_fence();
private:
	GVulkanLogicalDevice* m_device;
};


#endif // GVULKAN_FENCE_MANAGER_H