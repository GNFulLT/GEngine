#ifndef GVULKAN_LDEVICE_H
#define GVULKAN_LDEVICE_H

#include "engine/rendering/vulkan/ivulkan_ldevice.h"
#include "internal/engine/rendering/vulkan/vulkan_queue.h"

#include "public/core/templates/shared_ptr.h"
#include "engine/io/iowning_glogger.h"

#include <unordered_map>
#include <string>
#include <vma/vk_mem_alloc.h>
#include <memory>


class ITransferOperations;
class IGVulkanPhysicalDevice;
class GVulkanCommandBufferManager;

class GVulkanLogicalDevice : public IGVulkanLogicalDevice
{
public:
	GVulkanLogicalDevice(IGVulkanDevice* owner,GWeakPtr<IGVulkanPhysicalDevice> physicalDev, bool debugEnabled = true);

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

	virtual IGVulkanQueue* get_present_queue() noexcept override;

	virtual IGVulkanQueue* get_render_queue() noexcept override;

	virtual IGVulkanQueue* get_resource_queue() noexcept override;

	virtual bool begin_command_buffer_record(GVulkanCommandBuffer* buff) override;

	virtual void end_command_buffer_record(GVulkanCommandBuffer* buff) override;

	virtual std::expected<IVulkanBuffer*, VULKAN_BUFFER_CREATION_ERROR> create_buffer(uint64_t size, uint64_t bufferUsageFlag, VmaMemoryUsage memoryUsageFlag ) override;

	virtual std::expected< IVulkanImage*, VULKAN_IMAGE_CREATION_ERROR> create_image(const VkImageCreateInfo* imageCreateInfo, VmaMemoryUsage memoryUsageFlag) override;

	virtual IGVulkanDevice* get_owner() noexcept override;



	virtual std::expected<ITransferHandle*, TRANSFER_QUEUE_GET_ERR> get_wait_and_begin_transfer_cmd() override;

	virtual std::expected<ITransferHandle*, TRANSFER_QUEUE_GET_ERR> get_wait_and_begin_transfer_cmd(uint64_t timeout) override;

	virtual void finish_execute_and_wait_transfer_cmd(ITransferHandle* handle) override;
private:
	bool create_vma_allocator();

private:
	std::unique_ptr< ITransferOperations> m_transferOps;

	bool m_destroyed;
	bool m_inited = false;
	bool m_debugEnabled;
	GWeakPtr<IGVulkanPhysicalDevice> m_physicalDev;
	GSharedPtr<GVulkanCommandBufferManager> m_defaultCommandManager;
	GSharedPtr<IOwningGLogger> m_logger;

	GVulkanQueue m_defaultQueue;
	GVulkanQueue m_transferQueue;

	IGVulkanDevice* m_owner;

	std::vector<VkLayerProperties> m_deviceLayers;
	std::unordered_map<std::string, std::vector<VkExtensionProperties>> m_deviceExtensions;

	VkDevice m_logicalDevice;
	VmaAllocator allocator;

};
#endif // GVULKAN_LDEVICE_H