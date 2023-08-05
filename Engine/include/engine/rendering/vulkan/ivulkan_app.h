#ifndef IVULKAN_APP_H
#define IVULKAN_APP_H

//X Forward Declarations 
#include "version.h"
#include "engine/GEngine_EXPORT.h"

struct VkLayerProperties;

class ENGINE_API IGVulkanApp
{
public:
	virtual ~IGVulkanApp() = default;

	virtual bool init() = 0;

	virtual VersionInfo* get_version_info() = 0;
	
	virtual void* get_vk_instance() = 0;

	virtual void destroy() = 0;
};


#endif // IVULKAN_APP_H 