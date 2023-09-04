#include "engine/rendering/vulkan/vulkan_command_buffer.h"
#include "volk.h"
#include "internal/engine/rendering/vulkan/vulkan_ldevice.h"
#include "internal/engine/rendering/vulkan/vulkan_queue.h"
#include "public/core/templates/memnewd.h"

GVulkanCommandPool::GVulkanCommandPool(IGVulkanLogicalDevice* inDevice, IGVulkanQueue* queue,bool onlyPoolCanReset) : m_device(inDevice)
{
	m_boundedQueue = queue;
	m_commandPool = nullptr;
	m_isSelfResetAllowed = !onlyPoolCanReset;
}

GVulkanCommandPool::~GVulkanCommandPool()
{
#ifdef _DEBUG
	if (!isDestroyed)
		assert(false);
#endif
}

bool GVulkanCommandPool::is_valid()
{
	return m_commandPool != nullptr;
}

bool GVulkanCommandPool::init()
{
	VkCommandPoolCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	createInfo.pNext = nullptr;
	createInfo.flags = 0;
	createInfo.queueFamilyIndex = m_boundedQueue->get_queue_index();

	if (m_isSelfResetAllowed)
	{
		createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	}

	return VkResult::VK_SUCCESS == vkCreateCommandPool((VkDevice)m_device->get_vk_device(),&createInfo,nullptr,&m_commandPool);
}

void GVulkanCommandPool::destroy()
{
	vkDestroyCommandPool((VkDevice)m_device->get_vk_device(), m_commandPool, nullptr);
	m_commandPool = nullptr;
#ifdef _DEBUG
	isDestroyed = true;
#endif
}

bool GVulkanCommandPool::is_self_reset_allowed() const noexcept
{
	return m_isSelfResetAllowed;
}

VkCommandPool_T* GVulkanCommandPool::get_command_pool() noexcept
{
	return m_commandPool;
}

bool GVulkanCommandPool::reset_pool()
{
	return VK_SUCCESS == vkResetCommandPool((VkDevice)m_device->get_vk_device(), m_commandPool, 0);
}

GVulkanCommandBuffer::GVulkanCommandBuffer(IGVulkanLogicalDevice* inDevice, GVulkanCommandPool* pool,bool isPrimary)
{
	m_ownerPool = pool;
	m_device = inDevice;
	m_isPrimary = isPrimary;
	m_cmd = nullptr;
}

GVulkanCommandBuffer::~GVulkanCommandBuffer()
{

}

bool GVulkanCommandBuffer::init()
{
	VkCommandBufferAllocateInfo allocateInf = {};
	allocateInf.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocateInf.pNext = nullptr;
	allocateInf.commandPool = m_ownerPool->get_command_pool();
	allocateInf.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocateInf.commandBufferCount = 1;

	if(!m_isPrimary) 
		allocateInf.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;

	return VK_SUCCESS == vkAllocateCommandBuffers((VkDevice)m_device->get_vk_device(),&allocateInf,&m_cmd);
}

bool GVulkanCommandBuffer::is_valid()
{
	return m_cmd != nullptr;
}

void GVulkanCommandBuffer::destroy()
{
#ifdef _DEBUG
	bool isDestroyed = true;
#endif
}

VkCommandBuffer_T* GVulkanCommandBuffer::get_handle()
{
	return m_cmd;
}

VkCommandBuffer_T** GVulkanCommandBuffer::get_handle_p()
{
	return &m_cmd;
}

void GVulkanCommandBuffer::begin()
{
	VkCommandBufferBeginInfo inf = {};
	inf.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	inf.pNext = nullptr;
	inf.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	inf.pInheritanceInfo = nullptr;
	vkBeginCommandBuffer(m_cmd, &inf);
}

void GVulkanCommandBuffer::end()
{
	vkEndCommandBuffer(m_cmd);
}

GVulkanCommandBufferManager::GVulkanCommandBufferManager(IGVulkanLogicalDevice* inDevice, IGVulkanQueue* queue, bool onlyPoolCanReset)
{
	m_queue = queue;
	m_device = inDevice;
	m_isSelfResetAllowed = onlyPoolCanReset;
	m_pool = nullptr;
}

bool GVulkanCommandBufferManager::init()
{
	m_pool = gdnewa(GVulkanCommandPool,m_device,m_queue,m_isSelfResetAllowed);
	
	bool inited = m_pool->init();
	if (!inited)
	{
		gddel(m_pool);
		return false;
	}
	return true;
}

bool GVulkanCommandBufferManager::is_valid()
{
	return m_pool != nullptr && m_pool->is_valid();
}

void GVulkanCommandBufferManager::destroy()
{
	m_pool->destroy();
}

GVulkanCommandBuffer* GVulkanCommandBufferManager::get_active_command_buffer()
{
	return nullptr;
}

GVulkanCommandPool* GVulkanCommandBufferManager::get_pool()
{
	return m_pool;
}

GVulkanCommandBuffer* GVulkanCommandBufferManager::create_buffer(bool isPrimary)
{
	//X TODO USE GDNEWDA 
	return new GVulkanCommandBuffer(m_device, m_pool, isPrimary);
}

void GVulkanCommandBufferManager::set_active_command_buffer(GVulkanCommandBuffer* buffer)
{
	static_assert("This class doesn't support active command buffer");
}

bool GVulkanCommandBufferManager::reset_pool()
{
	return m_pool->reset_pool();
}
