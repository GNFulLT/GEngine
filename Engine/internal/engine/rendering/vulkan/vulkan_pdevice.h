#ifndef VULKAN_PDEVICE_H
#define VULKAN_PDEVICE_H

#include "engine/rendering/vulkan/ivulkan_pdevice.h"
#include "public/core/templates/shared_ptr.h"
#include <vector>
#include <volk.h>

class IGVulkanApp;

class GVulkanPhysicalDevice : public IGVulkanPhysicalDevice
{
public:
	GVulkanPhysicalDevice(GWeakPtr<IGVulkanApp> vulkanApp);
	~GVulkanPhysicalDevice();
	virtual bool init() override;

	virtual void destroy() override;

	virtual bool is_valid() const override;

	//X This function will be called inside init method. So given function owner should be live until init method
	//X What should i do here
	//X Elector info will be copied
	virtual void register_physical_device_elector(PhysicalDeviceElectorFN, const ElectorInfo* = nullptr) override;

	virtual uint32_t get_default_queue_family_index() const noexcept override;

	virtual void* get_vk_physical_device() const noexcept override;

	virtual const VkPhysicalDeviceFeatures& get_vk_features() const noexcept override;

	virtual IGVulkanApp* get_bounded_app()  noexcept override;

	virtual const std::vector<VkQueueFamilyProperties>& get_all_queues() const noexcept override;

	virtual bool does_support_only_transfer() const noexcept override;

	// If only transfer dowsn't support it returns default queue
	virtual uint32_t get_only_transfer() const noexcept override;

	virtual const VkPhysicalDeviceProperties* get_vk_properties() const noexcept;
private:
	struct Elector
	{
		PhysicalDeviceElectorFN fn;
		ElectorInfo inf;
	};

	GWeakPtr<IGVulkanApp> m_weakVulkanApp;
	std::vector<Elector> m_electors;

	std::vector<VkQueueFamilyProperties> m_allQueues;


	VkPhysicalDevice m_physicalDev;
	uint32_t m_defaultQueueFamilyIndex;
	uint32_t m_onlyTransferFamilyIndex;

	bool m_onlyTransferSupport;
	//X Cache
	std::vector<VkPhysicalDevice> m_availableDevices;
	VkPhysicalDeviceFeatures m_physicalDevFeatures;
	VkPhysicalDeviceProperties m_physicalDevProperties;

	bool destroyed;
};

#endif // VULKAN_PDEVICE_H