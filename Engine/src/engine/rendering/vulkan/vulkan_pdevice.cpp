#include "internal/engine/rendering/vulkan/vulkan_pdevice.h"
#include "internal/engine/rendering/vulkan/vulkan_app.h"
#include "internal/engine/rendering/vulkan/vulkan_utils.h"
#include "internal/engine/manager/glogger_manager.h"

constexpr static const char* TAG = "GVulkanPhysicalDevice";

//X This checks surface capability 
bool surface_check(void* vDev)
{
	auto dev = (VkPhysicalDevice)vDev;
	std::vector<std::string> requiredExtensionsForPhysicalDevice = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME
	};

	//X If all of it's aren't supported. This returns all supported physical device extensions. So You can check which extension is not supported
	std::vector<VkExtensionProperties> requiredExtensionPropsForPhysicalDevice;

	if (!check_device_extension_support(dev, &requiredExtensionsForPhysicalDevice, requiredExtensionPropsForPhysicalDevice))
	{
		return false;
	}
	return true;
}

//X General Queue Check

bool general_queue_check(void* vDev)
{
	auto dev = (VkPhysicalDevice)vDev;

	auto queueRequirements = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT;

	int index = -1;
	if (!check_queue_support(dev, queueRequirements, index))
		return false;

	return true;
	
}

bool discrete_gpu_check(void* vDev)
{
	auto dev = (VkPhysicalDevice)vDev;
	
	VkPhysicalDeviceProperties props;
	vkGetPhysicalDeviceProperties(dev, &props);
	if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
	{
		return true;
	}

	return false;
}


GVulkanPhysicalDevice::GVulkanPhysicalDevice(GWeakPtr<IGVulkanApp> vulkanApp) : m_weakVulkanApp(vulkanApp)
{
	destroyed = true;
}

GVulkanPhysicalDevice::~GVulkanPhysicalDevice()
{
}

bool GVulkanPhysicalDevice::init()
{
	bool restrictedToDiscreteGpu = false;


	GLoggerManager::get_instance()->log_d(TAG,"Getting vulkan app from global");
	auto vulkanApp = m_weakVulkanApp.as_shared();
	
	//X TODO : LOGGER
	if (!vulkanApp.is_valid())
		return false;

	//X Get All VkPhysicalDevice

	GLoggerManager::get_instance()->log_d(TAG, "Enumerating physical devices");

	std::vector<VkPhysicalDevice> physicalDevices;
	uint32_t physicalDeviceSize;
	vkEnumeratePhysicalDevices((VkInstance)vulkanApp->get_vk_instance(), &physicalDeviceSize, nullptr);
	physicalDevices.resize(physicalDeviceSize);
	vkEnumeratePhysicalDevices((VkInstance)vulkanApp->get_vk_instance(), &physicalDeviceSize, physicalDevices.data());

	//X Election

	GLoggerManager::get_instance()->log_d(TAG, "Initializing physical device electors");

	//X Surface check elector
	GLoggerManager::get_instance()->log_d(TAG, "Surface check elector");

	ElectorInfo inf;
	inf.who = "GEngine";
	inf.why = "surface_check";
	auto t = &surface_check;
	register_physical_device_elector(&surface_check,&inf);
	
	GLoggerManager::get_instance()->log_d(TAG, "Queue elector");

	inf.why = "general_queue_check";
	register_physical_device_elector(&general_queue_check, &inf);

	if (restrictedToDiscreteGpu)
	{
		GLoggerManager::get_instance()->log_d(TAG, "Discrete gpu elector");

		inf.why = "discrete_gpu_check";
		register_physical_device_elector(&discrete_gpu_check, &inf);
	}

	

	//X TODO : PLUGIN GOES HERE


	//X Select device with electors

	GLoggerManager::get_instance()->log_d(TAG, "Selection started.");

	for (int i = 0; i < physicalDevices.size(); i++)
	{
		bool isDeviceOk = true;
		for (int j = 0; j < m_electors.size(); j++)
		{
			bool isOk= m_electors[j].fn((void*)physicalDevices[i]);
			if (!isOk)
			{
				isDeviceOk = false;
				//X TODO : LOG HERE
				continue;
			}
		}
		if (isDeviceOk)
		{
			m_availableDevices.push_back(physicalDevices[i]);
		}
	}

	if (m_availableDevices.size() == 0)
	{
		GLoggerManager::get_instance()->log_e(TAG, "There is no physical device that meets the requirements.");

		return false;
	}

	//X Select first physical Device 
	GLoggerManager::get_instance()->log_d(TAG, "Selecting first device that meets the requirements");

	m_physicalDev = m_availableDevices[0];


	//X Save Default Queue index

	GLoggerManager::get_instance()->log_d(TAG, "Getting queues");


	auto queueRequirements = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT;

	bool supported = check_queue_support(m_physicalDev, queueRequirements, (int&)m_defaultQueueFamilyIndex);

	//X Get Props of physical device
	vkGetPhysicalDeviceFeatures(m_physicalDev, &(m_physicalDevFeatures));

	vkGetPhysicalDeviceProperties(m_physicalDev, &(m_physicalDevProperties));

	destroyed = false;
	return true;
}

void GVulkanPhysicalDevice::destroy()
{
	//X Nothing to destroy
	destroyed = true;
}

bool GVulkanPhysicalDevice::is_valid() const
{
	return !destroyed;
}

void GVulkanPhysicalDevice::register_physical_device_elector(PhysicalDeviceElectorFN pFn, const ElectorInfo* pInf)
{
	Elector elect;
	elect.fn = pFn;
	elect.inf.who = pInf->who;
	elect.inf.why = pInf->why;
	m_electors.push_back(elect);
}

uint32_t GVulkanPhysicalDevice::get_default_queue_family_index() const noexcept
{
	return m_defaultQueueFamilyIndex;
}

void* GVulkanPhysicalDevice::get_vk_physical_device() const noexcept
{
	return m_physicalDev;
}

const VkPhysicalDeviceFeatures& GVulkanPhysicalDevice::get_vk_features() const noexcept
{
	return m_physicalDevFeatures;
}


