
#ifdef _WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#elif __linux__
#define VK_USE_PLATFORM_XLIB_KHR
#elif __APPLE__
#define VK_USE_PLATFORM_METAL_EXT
#endif



#include "internal/engine/rendering/vulkan/vulkan_app.h"
#include "internal/engine/rendering/vulkan/vulkan_utils.h"


#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

#include <algorithm>


_IMP_RETURN_ bool layer_selection(std::vector<VkLayerProperties>& props, std::unordered_map<std::string, std::vector<VkExtensionProperties>>& extensions,bool debugEnabled = false);
bool get_layer_extension(const char* layerName,std::vector<VkExtensionProperties>& exs);

static VkBool32 VKAPI_CALL vk_debug_messenger_callback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
	(void)pUserData; // Unused argument

	std::string error = std::string("[VULKAN] [MESSENGER] Debug report from ObjectType: ") + std::to_string(uint32_t(pCallbackData->pObjects->objectType)) + "\n\nMessage: " + pCallbackData->pMessage + "\n\n";

 	return VK_FALSE;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL VulkanDebugReportCallback
(
	VkDebugReportFlagsEXT      flags,
	VkDebugReportObjectTypeEXT objectType,
	uint64_t                   object,
	size_t                     location,
	int32_t                    messageCode,
	const char* pLayerPrefix,
	const char* pMessage,
	void* UserData
)
{
	// https://github.com/zeux/niagara/blob/master/src/device.cpp   [ignoring performance warnings]
	// This silences warnings like "For optimal performance image layout should be VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL instead of GENERAL."
	if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)
		return VK_FALSE;

	/*LoggerManager::get_singleton()->log_cout(RenderDevice::get_singleton(), boost::str(boost::format("[VULKAN] [REPORTER] [%1%] %2%") % pLayerPrefix % pMessage).c_str(),
		Logger::DEBUG);*/

	return VK_FALSE;
}


GVulkanApp::GVulkanApp()
{
	m_vinfo.major = GNF_VERSION_MAJOR;
	m_vinfo.minor = GNF_VERSION_MINOR;
	m_vinfo.hex = GNF_VERSION_AS_HEX;
	m_vinfo.vulkanVersion = VK_API_VERSION_1_2;
	m_appName = std::string(_STR_XDEF(GNF_APP_NAME_FULL));
}

GVulkanApp::~GVulkanApp()
{
	if (!m_destroyed)
	{
		if (messenger != nullptr)
		{
			vkDestroyDebugUtilsMessengerEXT(m_instance, messenger, nullptr);
		}
		if (reportCallback != nullptr)
		{
			vkDestroyDebugReportCallbackEXT(m_instance, reportCallback, nullptr);
		}
		vkDestroyInstance(m_instance, nullptr);
	}
}

bool GVulkanApp::init()
{
	bool isDebugEnabled = true;
	if (VkResult::VK_SUCCESS != volkInitialize())
	{
		return false;
	}

	VkApplicationInfo applicationInfo;
	applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	applicationInfo.pNext = nullptr;
	applicationInfo.pApplicationName = m_appName.c_str();
	applicationInfo.pEngineName = "GEngine";
	applicationInfo.engineVersion = m_vinfo.hex;
	applicationInfo.apiVersion = m_vinfo.vulkanVersion;

	//X This will set the instance layer vector
	bool succeeded = layer_selection(m_instanceLayers,m_instanceExtensions, isDebugEnabled);
	if (!succeeded)
		return succeeded;

	//X Make Ready Layers for vulkan
	std::vector<const char*> layers(m_instanceLayers.size());
	for (int i = 0; i < m_instanceLayers.size(); i++)
	{
		layers[i] = m_instanceLayers[i].layerName;
	}

	//X MAKE READY EXTENSIONS FOR VULKAN
	uint32_t vsize = 0;
	for (auto& exs : m_instanceExtensions)
	{
		vsize += (uint32_t)exs.second.size();
	}

	std::vector<const char*> enabledExs(vsize);

	int vindex = 0;
	// Copy data to vector
	for (auto& exs : m_instanceExtensions)
	{
		for (int i = 0; i < exs.second.size(); i++)
		{
			enabledExs[vindex] = exs.second[i].extensionName;
			vindex++;
		}
	}

	//X NOW WE ARE READY TO CREATE INSTANCE
	VkInstanceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pNext = nullptr;

	createInfo.pApplicationInfo = &applicationInfo;
	createInfo.enabledLayerCount = (uint32_t)layers.size();
	createInfo.ppEnabledLayerNames = layers.data();
	createInfo.enabledExtensionCount = (uint32_t)enabledExs.size();
	createInfo.ppEnabledExtensionNames = enabledExs.data();
#if defined VK_USE_PLATFORM_METAL_EXT 
	createInfo.flags = 1;
#else
	createInfo.flags = 0;
#endif
	auto res = vkCreateInstance(&createInfo, nullptr, &m_instance);
	if (res != VK_SUCCESS)
		return false;

	volkLoadInstance(m_instance);


	if (isDebugEnabled)
	{
		create_debug_messenger(m_instance, &messenger, &reportCallback,
			vk_debug_messenger_callback, VulkanDebugReportCallback);
	}

	return true;
}

VersionInfo* GVulkanApp::get_version_info()
{
	return &m_vinfo;
}

void* GVulkanApp::get_vk_instance()
{
	return m_instance;
}

void GVulkanApp::destroy()
{
	if (messenger != nullptr)
	{
		vkDestroyDebugUtilsMessengerEXT(m_instance, messenger, nullptr);
	}
	if (reportCallback != nullptr)
	{
		vkDestroyDebugReportCallbackEXT(m_instance, reportCallback, nullptr);
	}
	vkDestroyInstance(m_instance,nullptr);

	m_destroyed = true;
}

bool layer_selection(std::vector<VkLayerProperties>& layerProps,std::unordered_map<std::string,std::vector<VkExtensionProperties>>& extensions, bool debugEnabled)
{
	std::vector<VkLayerProperties> allLayers = std::vector<VkLayerProperties>();
	//X GET ALL LAYERS
	uint32_t layer_count = 0;
	if (vkEnumerateInstanceLayerProperties(&layer_count, nullptr) != VK_SUCCESS)
	{
		return false;
	}
	allLayers.resize(layer_count);
	if (vkEnumerateInstanceLayerProperties(&layer_count, allLayers.data()) != VK_SUCCESS)
	{
		return false;
	}

	bool isDebugLayerAdded = false;

	if (debugEnabled)
	{
		int index = find_layer_by_name(allLayers,VK_LAYER_NAME_DEBUG_LAYER_NAME);
		if (index == -1)
		{
			//X Couldn't find debug layer name log it
		}
		else
		{
			layerProps.push_back(allLayers[index]);
		}
	}

	//X TODO : Plugins go here

	//X Check the array if plugins added same thing more than once
	layerProps.erase(std::unique(layerProps.begin(), layerProps.end(),
		[](const VkLayerProperties& lhs, const VkLayerProperties& rhs) {
			return std::strcmp(lhs.layerName, rhs.layerName) == 0;
		}),
		layerProps.end());

	std::vector<const char*> enabledLayers(layerProps.size());

	for (int i = 0; i < layerProps.size(); i++)
	{
		enabledLayers[i] = layerProps[i].layerName;
	}
	
	std::unordered_map<std::string, std::vector<VkExtensionProperties>> allExtensions;
	for (int i = 0; i < layerProps.size();i++)
	{
		std::vector<VkExtensionProperties> exs;
		if (!get_layer_extension(layerProps[i].layerName, exs))
			return false;
		allExtensions.emplace(layerProps[i].layerName, exs);
	}

	//X Implicit extensions


	allExtensions.emplace(IMPLICIT_EXTENSIONS_NAME, std::vector<VkExtensionProperties>());

	get_layer_extension(nullptr, allExtensions[IMPLICIT_EXTENSIONS_NAME]);

	//X Plugin Goes here


#ifdef VK_USE_PLATFORM_WIN32_KHR 
	std::vector<const char*> names = {
	 VK_KHR_SURFACE_EXTENSION_NAME ,VK_KHR_WIN32_SURFACE_EXTENSION_NAME
	};
	std::vector<int> indexes;
	bool allFounded = find_instance_exs_by_names(&allExtensions[IMPLICIT_EXTENSIONS_NAME], &names, indexes);

	if (!allFounded)
		return false;
	extensions[IMPLICIT_EXTENSIONS_NAME].push_back(allExtensions[IMPLICIT_EXTENSIONS_NAME][indexes[0]]);
	extensions[IMPLICIT_EXTENSIONS_NAME].push_back(allExtensions[IMPLICIT_EXTENSIONS_NAME][indexes[1]]);

#elif defined VK_USE_PLATFORM_XLIB_KHR 
	std::vector<const char*> names = {
 VK_KHR_SURFACE_EXTENSION_NAME ,VK_KHR_XLIB_SURFACE_EXTENSION_NAME
	};
	std::vector<int> indexes;
	bool allFounded = find_instance_exs_by_names(&all_exs[IMPLICIT_EXTENSIONS_NAME], &names, indexes);

	if (!allFounded)
		return false;
	extensions[IMPLICIT_EXTENSIONS_NAME].push_back(all_exs[IMPLICIT_EXTENSIONS_NAME][indexes[0]]);
	extensions[IMPLICIT_EXTENSIONS_NAME].push_back(all_exs[IMPLICIT_EXTENSIONS_NAME][indexes[1]]);

#elif defined VK_USE_PLATFORM_METAL_EXT 
	std::vector<const char*> names = {
 VK_KHR_SURFACE_EXTENSION_NAME ,"VK_EXT_metal_surface","VK_KHR_portability_enumeration"
	};
	std::vector<int> indexes;
	bool allFounded = find_instance_exs_by_names(&all_exs[IMPLICIT_EXTENSIONS_NAME], &names, indexes);

	if (!allFounded)
		return false;
	extensions[IMPLICIT_EXTENSIONS_NAME].push_back(all_exs[IMPLICIT_EXTENSIONS_NAME][indexes[0]]);
	extensions[IMPLICIT_EXTENSIONS_NAME].push_back(all_exs[IMPLICIT_EXTENSIONS_NAME][indexes[1]]);
	extensions[IMPLICIT_EXTENSIONS_NAME].push_back(all_exs[IMPLICIT_EXTENSIONS_NAME][indexes[2]]);
#else
#error UNSUPPORTED PLATFORM
#endif 


	//X TODO : PLUGIN GOES HERE ...
	
	//X Check the array if plugins added same thing more than once
	for (auto& exs : extensions)
	{
		exs.second.erase(std::unique(exs.second.begin(), exs.second.end(),
			[](const VkExtensionProperties& lhs, const VkExtensionProperties& rhs) {
				return std::strcmp(lhs.extensionName, rhs.extensionName) == 0;
			}),
			exs.second.end());
	}

	
	return true;
}

bool get_layer_extension(const char* layerName, std::vector<VkExtensionProperties>& exs)
{
	uint32_t ex_count = 0;
	if (vkEnumerateInstanceExtensionProperties(layerName, &ex_count, nullptr) != VK_SUCCESS)
	{
		return false;
	}
	exs = std::vector<VkExtensionProperties>(ex_count);
	if (vkEnumerateInstanceExtensionProperties(layerName, &ex_count, exs.data()) != VK_SUCCESS)
	{
		return false;
	}
	return true;
}