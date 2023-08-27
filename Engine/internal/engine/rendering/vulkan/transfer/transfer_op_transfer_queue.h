#ifndef TRANSFER_OP_TRANSFER_QUEUE_H
#define TRANSFER_OP_TRANSFER_QUEUE_H

#include <mutex>
#include <memory>
#include <vector>
#include <queue>
#include "engine/rendering/vulkan/transfer/itransfer_op.h"
#include <expected>

class IGVulkanQueue;
class GVulkanFence;
class GVulkanCommandBufferManager;
class GVulkanCommandBuffer;
class GVulkanFenceManager;
class GVulkanSemaphoreManager;
class GVulkanSemaphore;

class TransferQueueHandle : public ITransferHandle
{
public:
	TransferQueueHandle(GVulkanCommandBuffer* cmd, uint32_t index);
	std::vector<TransferQueueHandle*>::iterator get_executed_cmd_iterator();
	void set_executed_cmd_iterator(std::vector<TransferQueueHandle*>::iterator iter);
	virtual GVulkanCommandBuffer* get_command_buffer() override;
	uint32_t get_index() const noexcept;
private:
	std::vector<TransferQueueHandle*>::iterator m_executedCmdIterator;
	uint32_t m_index;
	GVulkanCommandBuffer* m_cmdBuffer;

	// Inherited via ITransferHandle
};
class GVulkanLogicalDevice;

class TransferOpTransferQueue : public ITransferOperations
{
public:
	TransferOpTransferQueue(GVulkanLogicalDevice* dev,uint32_t transferCmdCount);
	 

	virtual bool init() override;
	
	virtual std::expected<ITransferHandle*, TRANSFER_QUEUE_GET_ERR> get_wait_and_begin_transfer_cmd() override;

	virtual std::expected<ITransferHandle*, TRANSFER_QUEUE_GET_ERR> get_wait_and_begin_transfer_cmd(uint64_t timeout) override;
	
	virtual void finish_execute_and_wait_transfer_cmd(ITransferHandle* handle) override;

	virtual void destroy() override;

	virtual std::expected<IVulkanImage*, int> init_image_to_the_gpu_from_cpu_sleep(VkImageCreateInfo* inf, VkImageViewCreateInfo* viewInfo,int buffSize,void* pBuff) override;
private:
	void on_finish(ITransferHandle* finished);
private:
	IGVulkanQueue* m_transferQueue;
	uint32_t m_transferCmdCount;

	std::mutex m_transferOwnerShipMutex;
	
	GVulkanLogicalDevice* m_boundedDevice;
	std::mutex m_transferMutex;

	std::unique_ptr<GVulkanCommandBufferManager> m_transferCommandManager;
	std::unique_ptr<GVulkanCommandBufferManager> m_transferOwnershipCommandManager;

	std::unique_ptr<GVulkanFenceManager> m_fenceManager;
	std::unique_ptr<GVulkanSemaphoreManager> m_semaphoreManager;

	std::vector<GVulkanCommandBuffer*> m_transferCommandBuffers;
	std::vector<GVulkanCommandBuffer*> m_transferOwnershipCommandBuffers;

	std::vector<GVulkanFence*> m_transferCommandBuffersFences;

	// CMD , Index in vector

	std::queue<TransferQueueHandle*> m_availableTransferCommandBuffers;

	// CMD , Index in vector

	std::vector<GVulkanSemaphore*> m_transferCommandBufferSemaphores;
	std::vector<TransferQueueHandle*> m_executedTransferCommandBuffersQueue;
};


#endif // TRANSFER_OP_TRANSFER_QUEUE_H