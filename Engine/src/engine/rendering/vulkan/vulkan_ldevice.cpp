#include "internal/engine/rendering/vulkan/vulkan_ldevice.h"
#include "internal/engine/rendering/vulkan/vulkan_pdevice.h"
#include "internal/engine/rendering/vulkan/vulkan_utils.h"
#include "engine/rendering/vulkan/vulkan_command_buffer.h"
#include "internal/engine/manager/glogger_manager.h"

#include <algorithm>
GVulkanLogicalDevice::GVulkanLogicalDevice(GWeakPtr<IGVulkanPhysicalDevice> physicalDev, bool debugEnabled) : m_physicalDev(physicalDev),m_debugEnabled(debugEnabled)
{
	m_destroyed = true;
	m_logicalDevice = 0;
}

GVulkanLogicalDevice::~GVulkanLogicalDevice()
{
	if (!m_destroyed)
	{
		vkDestroyDevice(m_logicalDevice, nullptr);
	}
}

bool GVulkanLogicalDevice::init()
{
	auto physicalDevice = m_physicalDev.as_shared();
	if (!physicalDevice.is_valid())
		return false;

	m_logger = GLoggerManager::get_instance()->create_owning_glogger("GVulkanLogicalDevice");
	
	m_logger->log_d("Creating Queue Info");

	uint32_t queueIndex = physicalDevice->get_default_queue_family_index();

	QueueCreateInf queueCreateInf;

	std::vector<float> mainPriority = { 1.f };

	queueCreateInf.add_create_info(queueIndex, mainPriority);

	//X All Layers
	std::vector<VkLayerProperties> allLayerProps;
	uint32_t propCount = 0;
	
	m_logger->log_d("Enumerating Layer Properties");

	if (VK_SUCCESS != vkEnumerateDeviceLayerProperties((VkPhysicalDevice)physicalDevice->get_vk_physical_device(), &propCount, nullptr))
		return false;
	allLayerProps.resize(propCount);
	if (VK_SUCCESS != vkEnumerateDeviceLayerProperties((VkPhysicalDevice)physicalDevice->get_vk_physical_device(), &propCount, allLayerProps.data()))
		return false;

	if (m_debugEnabled)
	{
		m_logger->log_d("Debug enabled. Trying to find debug layer");
		for (int i = 0; i < allLayerProps.size(); i++)
		{
			if (strcmp(allLayerProps[i].layerName, VK_LAYER_NAME_DEBUG_LAYER_NAME) == 0)
			{
				m_logger->log_d("Debug layer found. Added");
				m_deviceLayers.push_back(allLayerProps[i]);
				break;
			}
		}
	}

	if (m_deviceLayers.size() == 0)
	{
		m_debugEnabled = false;
		//X TODO : LOGGER HERE FORCE CLOSING DEBUG LAYER
		m_logger->log_d("There is no debug layer. Closing debug mode by force");
	}
	m_logger->log_d("Get explicit extensions");
	std::vector<VkExtensionProperties> implicitExs;
	get_device_implicit_exs((VkPhysicalDevice)physicalDevice->get_vk_physical_device(), implicitExs);

	//X Try to find swapchain extension
	int swapchainExsIndex = -1;
	m_logger->log_d("Trying to find swapchain extension");
	for (int i = 0; i < implicitExs.size(); i++)
	{
		if (strcmp(implicitExs[i].extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0)
		{
			m_logger->log_d("Swapchain extension added");
			swapchainExsIndex = i;
			break;
		}
	}

	//X TODO : LOGGER
	if (swapchainExsIndex == -1)
		return false;

	m_logger->log_d("Swapchain extension added to the main extensions");

	//X Push swapchain extension
	m_deviceExtensions[IMPLICIT_EXTENSIONS_NAME].push_back(implicitExs[swapchainExsIndex]);
	
	//X PLUGIN GOES HERE




	m_logger->log_d("Checking duplicates for layers");

	//X Check the array if plugins added same thing more than once
	
	m_deviceLayers.erase(std::unique(m_deviceLayers.begin(), m_deviceLayers.end(),
		[](const VkLayerProperties& lhs, const VkLayerProperties& rhs) {
			return std::strcmp(lhs.layerName, rhs.layerName) == 0;
		}),
		m_deviceLayers.end());


	//X Check the extension array if plugins added same thing more than once

	m_logger->log_d("Checking duplicates for extensions");

	//X Check the array if plugins added same thing more than once
	for (auto& exs : m_deviceExtensions)
	{
		exs.second.erase(std::unique(exs.second.begin(), exs.second.end(),
			[](const VkExtensionProperties& lhs, const VkExtensionProperties& rhs) {
				return std::strcmp(lhs.extensionName, rhs.extensionName) == 0;
			}),
			exs.second.end());
	}


	m_logger->log_d("Creating info layers from selected layers");

	std::vector<const char*> enabledLayers(m_deviceLayers.size());

	int vexSize = 0;
	for (const auto& exs : m_deviceExtensions)
	{
		for (const auto& ex : exs.second)
		{
			vexSize++;
		}
	}
	
	m_logger->log_d("Creating info extensions from selected extensions");

	std::vector<const char*> enabledExtensions(vexSize);

	// Make ready data for vulkan
	for (int i = 0; i <	m_deviceLayers.size(); i++)
	{
		enabledLayers[i] = m_deviceLayers[i].layerName;
	}

	int vexIndex = 0;
	for (const auto& exs :m_deviceExtensions)
	{
		for (const auto& ex : exs.second)
		{
			enabledExtensions[vexIndex] = ex.extensionName;
			vexIndex++;
		}
	}


	// NOW CREATE LOGICAL DEVICE

	m_logger->log_d("Creating device");


	VkDeviceCreateInfo createInfo;
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.pNext = nullptr;
	createInfo.flags = 0;
	createInfo.queueCreateInfoCount = (uint32_t)queueCreateInf.get_queue_create_inf_count();
	createInfo.pQueueCreateInfos = queueCreateInf.data();
	createInfo.enabledLayerCount = (uint32_t)enabledLayers.size();
	createInfo.ppEnabledLayerNames = enabledLayers.data();
	createInfo.enabledExtensionCount = (uint32_t)enabledExtensions.size();
	createInfo.ppEnabledExtensionNames = enabledExtensions.data();
	createInfo.pEnabledFeatures = &(physicalDevice->get_vk_features());

	if (auto res = vkCreateDevice((VkPhysicalDevice)physicalDevice->get_vk_physical_device(), &createInfo, nullptr, &(m_logicalDevice)); res != VK_SUCCESS)
		return false;

	m_logger->log_d("Device created. Getting meta infos");

	m_defaultQueue = GVulkanQueue(this,physicalDevice->get_default_queue_family_index());

	if (!m_defaultQueue.is_valid())
	{
		return false;
	}

	m_inited = true;
	m_destroyed = false;

	return true;
}

bool GVulkanLogicalDevice::is_valid() const
{
	return m_inited && !m_destroyed;
}

void GVulkanLogicalDevice::destroy()
{
	vkDestroyDevice(m_logicalDevice, nullptr);
	m_destroyed = true;
}

void* GVulkanLogicalDevice::get_vk_device()
{
	return m_logicalDevice;
}

IGVulkanPhysicalDevice* GVulkanLogicalDevice::get_bounded_physical_device()
{
	auto physical=  m_physicalDev.as_shared();
	return physical.get();
}

IGVulkanQueue* GVulkanLogicalDevice::get_present_queue()
{
	return &m_defaultQueue;
}

IGVulkanQueue* GVulkanLogicalDevice::get_render_queue()
{
	return &m_defaultQueue;
}

bool GVulkanLogicalDevice::begin_command_buffer_record(GVulkanCommandBuffer* buff)
{
	//X TODO : BUFF RETURN BOOL
	if (buff != nullptr)
		buff->begin();
	else
		return false;
	return true;
}

void GVulkanLogicalDevice::end_command_buffer_record(GVulkanCommandBuffer* buff)
{
	buff->end();
}
