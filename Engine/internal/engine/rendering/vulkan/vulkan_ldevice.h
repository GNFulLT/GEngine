#ifndef GVULKAN_LDEVICE_H
#define GVULKAN_LDEVICE_H

#include "engine/rendering/vulkan/ivulkan_ldevice.h"
#include "internal/engine/rendering/vulkan/vulkan_queue.h"

#include "public/core/templates/shared_ptr.h"
#include <unordered_map>
#include "volk.h"
#include <string>

class IGVulkanPhysicalDevice;
class GVulkanCommandBufferManager;

class GVulkanLogicalDevice : public IGVulkanLogicalDevice
{
public:
	GVulkanLogicalDevice(GWeakPtr<IGVulkanPhysicalDevice> physicalDev, bool debugEnabled = true);

	~GVulkanLogicalDevice();

	virtual bool init() override;

	virtual bool is_valid() const override;

	virtual void destroy() override;

	virtual void* get_vk_device() override;

	virtual IGVulkanPhysicalDevice* get_bounded_physical_device() override;

	//X TODO : MAYBE THIS NEEDS TO BE EXPORTED 
	inline GVulkanQueue* get_queue()
	{
		return &m_defaultQueue;
	}

	virtual IGVulkanQueue* get_present_queue() override;

	virtual IGVulkanQueue* get_render_queue() override;


	virtual bool begin_command_buffer_record(GVulkanCommandBuffer* buff) override;

	virtual void end_command_buffer_record(GVulkanCommandBuffer* buff) override;

private:
	bool m_destroyed;
	bool m_inited = false;
	bool m_debugEnabled;
	GWeakPtr<IGVulkanPhysicalDevice> m_physicalDev;
	GSharedPtr<GVulkanCommandBufferManager> m_defaultCommandManager;

	GVulkanQueue m_defaultQueue;

	std::vector<VkLayerProperties> m_deviceLayers;
	std::unordered_map<std::string, std::vector<VkExtensionProperties>> m_deviceExtensions;

	VkDevice m_logicalDevice;
};
#endif // GVULKAN_LDEVICE_H