#ifndef VULKAN_COMMAND_BUFFER_H
#define VULKAN_COMMAND_BUFFER_H


#include "engine/GEngine_EXPORT.h"

class IGVulkanLogicalDevice;
class IGVulkanQueue;

struct VkCommandPool_T;
struct VkCommandBuffer_T;

//X TODO : INLINE 
//X GVULKANLOGICALDEVICE WEAK OR SHARED PTR CONSIDERED 
class ENGINE_API GVulkanCommandPool
{
public:
	GVulkanCommandPool(IGVulkanLogicalDevice* inDevice, IGVulkanQueue* queue,bool onlyPoolCanReset = true);
	~GVulkanCommandPool();

	bool is_valid();

	bool init();

	void destroy();

	bool is_self_reset_allowed() const noexcept;

	VkCommandPool_T* get_command_pool() noexcept;

	bool reset_pool();

private:
	VkCommandPool_T* m_commandPool;
	// GSHARED PTR MAYBE
	IGVulkanLogicalDevice* m_device;
	IGVulkanQueue* m_boundedQueue;
	bool m_isSelfResetAllowed;
#ifdef _DEBUG
	bool isDestroyed = false;
#endif
};


class ENGINE_API GVulkanCommandBuffer
{
public:
	GVulkanCommandBuffer(IGVulkanLogicalDevice* inDevice,GVulkanCommandPool* pool,bool isPrimary);
	~GVulkanCommandBuffer();
	bool init();

	bool is_valid();

	void destroy();
	
	void reset();

	VkCommandBuffer_T* get_handle();
	VkCommandBuffer_T** get_handle_p();

	void begin();

	void end();

private:
	GVulkanCommandPool* m_ownerPool;
	IGVulkanLogicalDevice* m_device;
	VkCommandBuffer_T* m_cmd;
	bool m_isPrimary;

#ifdef _DEBUG
	bool isDestroyed = false;
#endif
};


class ENGINE_API GVulkanCommandBufferManager
{
public:
	GVulkanCommandBufferManager(IGVulkanLogicalDevice* inDevice, IGVulkanQueue* queue, bool onlyPoolCanReset = true);

	bool init();

	bool is_valid();

	void destroy();

	GVulkanCommandBuffer* get_active_command_buffer();
	
	GVulkanCommandPool* get_pool();

	GVulkanCommandBuffer* create_buffer(bool isPrimary);

	void set_active_command_buffer(GVulkanCommandBuffer* buffer);

	bool reset_pool();

private:
	IGVulkanLogicalDevice* m_device;
	IGVulkanQueue* m_queue;
	GVulkanCommandPool* m_pool;
	bool m_isSelfResetAllowed;

};




#endif // VULKAN_COMMAND_BUFFER_H