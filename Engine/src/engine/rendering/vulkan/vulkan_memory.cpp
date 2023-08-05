#include "internal/engine/rendering/vulkan/vulkan_memory.h"
#include "internal/engine/rendering/vulkan/vulkan_ldevice.h"
#include <cassert>
#include "volk.h"
GVulkanFenceManager::GVulkanFenceManager(GVulkanLogicalDevice* dev)
{
	m_device = dev;
}

GVulkanFenceManager::~GVulkanFenceManager()
{
}

GVulkanLogicalDevice* GVulkanFenceManager::get_bounded_device()
{
	return m_device;
}

GVulkanFence* GVulkanFenceManager::create_fence()
{
	return new GVulkanFence(this);
}

GVulkanFence::GVulkanFence(GVulkanFenceManager* owner)
{
	m_owner = owner;
	m_fence = nullptr;
}

bool GVulkanFence::init(bool signaled)
{
	VkFenceCreateInfo inf = {};
	inf.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	inf.pNext = nullptr;
	inf.flags = signaled;

	return VK_SUCCESS == vkCreateFence((VkDevice)m_owner->get_bounded_device()->get_vk_device(), &inf, nullptr, &m_fence);
}

void GVulkanFence::destroy()
{
	if (m_fence != nullptr)
		vkDestroyFence((VkDevice)m_owner->get_bounded_device()->get_vk_device(), m_fence, nullptr);
	m_fence = nullptr;
}

bool GVulkanFence::reset()
{
	return VK_SUCCESS == vkResetFences((VkDevice)m_owner->get_bounded_device()->get_vk_device(), 1, &m_fence);
}

void GVulkanFence::wait()
{
	vkWaitForFences((VkDevice)m_owner->get_bounded_device()->get_vk_device(), 1, &m_fence, VK_TRUE, UINT64_MAX);
}

VkFence_T* GVulkanFence::get_fence()
{
	return m_fence;
}

GVulkanFence::~GVulkanFence()
{
	assert(m_fence == nullptr);
}

GVulkanSemaphoreManager::GVulkanSemaphoreManager(GVulkanLogicalDevice* owner)
{
	m_owner = owner;
}

GVulkanLogicalDevice* GVulkanSemaphoreManager::get_bounded_device()
{
	return m_owner;
}

GVulkanSemaphore* GVulkanSemaphoreManager::create_semaphore()
{
	return new GVulkanSemaphore(this);
}

GVulkanSemaphore::GVulkanSemaphore(GVulkanSemaphoreManager* owner)
{
	m_owner = owner;
	m_semaphore = nullptr;
}

GVulkanSemaphore::~GVulkanSemaphore()
{
	assert(m_semaphore == nullptr);
}

bool GVulkanSemaphore::init()
{
	VkSemaphoreCreateInfo inf = {};
	inf.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	inf.pNext = nullptr;
	inf.flags = 0;

	return VK_SUCCESS == vkCreateSemaphore((VkDevice)m_owner->get_bounded_device()->get_vk_device(), &inf, nullptr, &m_semaphore);
}

void GVulkanSemaphore::destroy()
{
	if(m_semaphore != nullptr)
		vkDestroySemaphore((VkDevice)m_owner->get_bounded_device()->get_vk_device(), m_semaphore, nullptr);
	m_semaphore = nullptr;
}

VkSemaphore_T* GVulkanSemaphore::get_semaphore()
{
	return m_semaphore;
}
