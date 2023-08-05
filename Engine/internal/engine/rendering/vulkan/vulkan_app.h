#ifndef VULKAN_APP_H
#define VULKAN_APP_H

//X Forward Declarations 
#include "version.h"
#include "engine/GEngine_EXPORT.h"
#include <vector>
#include "engine/rendering/vulkan/ivulkan_app.h"
#include <string>
#include "volk.h"
#include <unordered_map>
struct VkLayerProperties;

class GVulkanApp : public IGVulkanApp 
{
public:
	GVulkanApp();
	~GVulkanApp();
	bool init();

	VersionInfo* get_version_info();

	virtual void* get_vk_instance() override;

	virtual void destroy() override;
private:
	VersionInfo m_vinfo;
	std::string m_appName;
	std::vector<VkLayerProperties> m_instanceLayers;
	std::unordered_map<std::string, std::vector<VkExtensionProperties>> m_instanceExtensions;

	VkInstance m_instance;
	VkDebugUtilsMessengerEXT messenger = nullptr;
	VkDebugReportCallbackEXT reportCallback = nullptr;

	bool m_destroyed = false;
};


#endif // VULKAN_APP_H 