#include "volk.h"

#include "internal/engine/rendering/vulkan/vulkan_device.h"
#include "internal/engine/rendering/vulkan/vulkan_ldevice.h"
#include "internal/engine/rendering/vulkan/vulkan_pdevice.h"
#include "engine/rendering/vulkan/vulkan_command_buffer.h"
#include "internal/engine/rendering/vulkan/vulkan_memory.h"
#include "internal/engine/manager/glogger_manager.h"

static GVulkanLogicalDevice* s_logicalDevice;

constexpr static const char* TAG = "GVulkanDevice";

GVulkanDevice::GVulkanDevice(GWeakPtr<IGVulkanApp> app) : m_vulkanApp(app)
{
	is_inited = false;
	m_vulkanPhysicalDevice = GSharedPtr<IGVulkanPhysicalDevice>(new GVulkanPhysicalDevice(app));
	auto vulkanLogicalDevice = new GVulkanLogicalDevice(m_vulkanPhysicalDevice);
	m_vulkanLogicalDevice = GSharedPtr<IGVulkanLogicalDevice>(vulkanLogicalDevice);
	s_logicalDevice = vulkanLogicalDevice;
	auto commandManager = new GVulkanCommandBufferManager(vulkanLogicalDevice, vulkanLogicalDevice->get_queue());
	auto fenceManager = new GVulkanFenceManager(vulkanLogicalDevice);
	auto semaphoreManager = new GVulkanSemaphoreManager(vulkanLogicalDevice);



	m_defaultCommandManager = GSharedPtr<GVulkanCommandBufferManager>(commandManager);
	m_fenceManager = GSharedPtr<GVulkanFenceManager>(fenceManager);
	m_semaphoreManager = GSharedPtr<GVulkanSemaphoreManager>(semaphoreManager);

	m_destroyed = true;
}

GVulkanDevice::~GVulkanDevice()
{
	if (!m_destroyed)
	{

		m_defaultCommandManager->destroy();

		m_vulkanLogicalDevice->destroy();

		m_vulkanPhysicalDevice->destroy();
		
	}
}

GSharedPtr<IGVulkanPhysicalDevice> GVulkanDevice::as_physical_device()
{
	return m_vulkanPhysicalDevice;
}

GSharedPtr<IGVulkanLogicalDevice> GVulkanDevice::as_logical_device()
{
	return m_vulkanLogicalDevice;
}

bool GVulkanDevice::init()
{
	//X Needs a initalizer graph and reverse destroy inited objects
	//X Also needs a logger

	GLoggerManager::get_instance()->log_d(TAG,"Initializing vulkan physical device");
	bool inited =  m_vulkanPhysicalDevice->init();
	if (!inited)
		return inited;

	GLoggerManager::get_instance()->log_d(TAG, "Initializing vulkan logical device");
	inited = m_vulkanLogicalDevice->init();
	if (!inited)
		return inited;

	inited = m_defaultCommandManager->init();
	if (!inited)
		return inited;

	m_renderingFence = m_fenceManager->create_fence();
	m_presentationSemaphore =  m_semaphoreManager->create_semaphore();
	m_imageAcquiredSemaphore = m_semaphoreManager->create_semaphore();

	m_imageAcquiredSemaphore->init();
	m_renderingFence->init(true);
	m_presentationSemaphore->init();

	m_mainCommandBuffer = m_defaultCommandManager->create_buffer(true);
	m_singleTimeCommandBuffer = m_defaultCommandManager->create_buffer(true);
	m_mainCommandBuffer->init();
	m_singleTimeCommandBuffer->init();
	

	m_destroyed = true;
	is_inited = true;
	return true;
}

bool GVulkanDevice::is_valid() const
{
	return is_inited;
}

void GVulkanDevice::destroy()
{
	m_imageAcquiredSemaphore->destroy();

	m_renderingFence->destroy();

	m_presentationSemaphore->destroy();
	
	m_defaultCommandManager->destroy();

	//X First logical device should be destroyed
	m_vulkanLogicalDevice->destroy();
	
	m_vulkanPhysicalDevice->destroy();


	delete m_mainCommandBuffer;
	delete m_presentationSemaphore;
	delete m_renderingFence;
	delete m_imageAcquiredSemaphore;
	delete m_singleTimeCommandBuffer;
	m_destroyed = true;
}

GVulkanCommandBuffer* GVulkanDevice::get_main_command_buffer()
{
	return m_mainCommandBuffer;
}

GVulkanCommandBuffer* GVulkanDevice::get_single_time_command_buffer()
{
	m_singleTimeCommandBuffer->begin();
	return m_singleTimeCommandBuffer;
}

void GVulkanDevice::execute_single_time_command_buffer_and_wait()
{
	VkCommandBuffer buff = m_singleTimeCommandBuffer->get_handle();
	m_singleTimeCommandBuffer->end();
	VkSubmitInfo info = {};
	info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	info.pNext = nullptr;
	info.commandBufferCount = 1;
	info.pCommandBuffers = &buff;
	info.signalSemaphoreCount = 0;
	info.waitSemaphoreCount = 0;
	vkQueueSubmit(m_vulkanLogicalDevice->get_render_queue()->get_queue(), 1, &info, nullptr);
	vkQueueWaitIdle(m_vulkanLogicalDevice->get_render_queue()->get_queue());
}

bool GVulkanDevice::prepare_for_rendering()
{	
	return reset_things();
}

GVulkanSemaphore* GVulkanDevice::get_image_acquired_semaphore()
{
	return m_imageAcquiredSemaphore;
}

GVulkanSemaphore* GVulkanDevice::get_render_complete_semaphore()
{
	return m_presentationSemaphore;
}

GVulkanFence* GVulkanDevice::get_queue_fence()
{
	return m_renderingFence;
}

bool GVulkanDevice::reset_things()
{
	m_defaultCommandManager->reset_pool();
	m_renderingFence->reset();

	return true;
}
