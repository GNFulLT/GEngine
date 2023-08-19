#ifndef IVULKAN_PDEVICE_H
#define IVULKAN_PDEVICE_H

#include "engine/GEngine_EXPORT.h"

#include <string>
#include <vector>

struct ElectorInfo
{
	std::string who;
	std::string why;
};

struct VkPhysicalDeviceFeatures;
struct VkQueueFamilyProperties;

class IGVulkanApp;

class ENGINE_API IGVulkanPhysicalDevice
{
public:
	typedef bool(*PhysicalDeviceElectorFN)(void*);

	virtual ~IGVulkanPhysicalDevice() = default;

	virtual bool init() = 0;

	virtual bool is_valid() const = 0;

	//X This function will be called inside init method. So given function owner should be live until init method
	//X What should i do here
	//X Elector info will be copied so u should delete if u created in heap memory
	virtual void register_physical_device_elector(PhysicalDeviceElectorFN,const ElectorInfo*) = 0;

	virtual const VkPhysicalDeviceFeatures& get_vk_features() const noexcept = 0;

	virtual void* get_vk_physical_device() const noexcept = 0;

	virtual uint32_t get_default_queue_family_index() const noexcept = 0;

	virtual void destroy() = 0;

	virtual IGVulkanApp* get_bounded_app() noexcept = 0;

	virtual bool does_support_only_transfer() const noexcept = 0;
	
	// If only transfer dowsn't support it returns default queue
	virtual uint32_t get_only_transfer() const noexcept = 0;

	virtual const std::vector<VkQueueFamilyProperties>& get_all_queues() const noexcept = 0;
private:
};

#endif // IVULKAN_PDEVICE_H