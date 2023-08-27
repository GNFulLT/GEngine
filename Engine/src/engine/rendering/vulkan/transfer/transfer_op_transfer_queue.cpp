#include "volk.h"

#include "internal/engine/rendering/vulkan/transfer/transfer_op_transfer_queue.h"
#include "engine/rendering/vulkan/vulkan_command_buffer.h"
#include "engine/rendering/vulkan/vulkan_memory.h"
#include "internal/engine/rendering/vulkan/vulkan_ldevice.h"
#include "engine/rendering/vulkan/ivulkan_queue.h"


#include <thread>
TransferOpTransferQueue::TransferOpTransferQueue(GVulkanLogicalDevice* dev,uint32_t transferCmdCount)
{
	m_transferQueue = dev->get_resource_queue();
	m_transferCmdCount = transferCmdCount;
	m_transferCommandManager = std::unique_ptr<GVulkanCommandBufferManager>(new GVulkanCommandBufferManager(dev,m_transferQueue, false));
	m_fenceManager = std::unique_ptr<GVulkanFenceManager>(new GVulkanFenceManager(dev));
}

bool TransferOpTransferQueue::init()
{
	bool inited = m_transferCommandManager->init();

	if (!inited)
		return false;

	//X TODO GDNEWDA 
	m_transferCommandBuffers.resize(m_transferCmdCount);
	m_transferCommandBuffersFences.resize(m_transferCmdCount);

	for (int i = 0; i < m_transferCmdCount; i++)
	{
		m_transferCommandBuffers[i] = m_transferCommandManager->create_buffer(true);
		m_transferCommandBuffersFences[i] = m_fenceManager->create_fence();
		
		
		inited = m_transferCommandBuffers[i]->init();
		//X TODO: DESTROY AND DELETE ALL LAST CREATED CMDS
		if (!inited)
		{
			return false;
		}
		inited = m_transferCommandBuffersFences[i]->init(false);
		if (!inited)
		{
			return false;
		}

		m_availableTransferCommandBuffers.push(new TransferQueueHandle(m_transferCommandBuffers[i],i));
	}
	return true;
}

std::expected<ITransferHandle*, TRANSFER_QUEUE_GET_ERR> TransferOpTransferQueue::get_wait_and_begin_transfer_cmd()
{
	return get_wait_and_begin_transfer_cmd(3000);
}

std::expected<ITransferHandle*, TRANSFER_QUEUE_GET_ERR> TransferOpTransferQueue::get_wait_and_begin_transfer_cmd(uint64_t timeout)
{
	{
		std::lock_guard guard(m_transferMutex);

		// There is an available cmd
		if (!m_availableTransferCommandBuffers.empty())
		{
			auto handle = m_availableTransferCommandBuffers.front();
			m_availableTransferCommandBuffers.pop();
			handle->get_command_buffer()->begin();
			m_executedTransferCommandBuffersQueue.push_back(handle);
			return handle;
		}
	}
	int ctimeout = 0;
	int wait = timeout / 5;
	// There is not available cmd wait until timeout occurs
	while (true)
	{
		// TIMEOUT OCCURS
		if (ctimeout >= wait)
		{
			break;
		}

		{
			std::lock_guard guard(m_transferMutex);

			if (!m_availableTransferCommandBuffers.empty())
			{
				auto handle = m_availableTransferCommandBuffers.front();
				m_availableTransferCommandBuffers.pop();
				handle->get_command_buffer()->begin();
				m_executedTransferCommandBuffersQueue.push_back(handle);
				return handle;
			}
		}
		ctimeout++;
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}
	

	return std::unexpected(TRANSFER_QUEUE_GET_ERR_TIMEOUT);
}

void TransferOpTransferQueue::finish_execute_and_wait_transfer_cmd(ITransferHandle* handle)
{
	VkCommandBuffer buff = handle->get_command_buffer()->get_handle();
	VkPipelineStageFlags flag = VK_PIPELINE_STAGE_NONE;
	//X UNSAFE 
	TransferQueueHandle* mhandle = (TransferQueueHandle*)handle;
	// Finish the buffer and wait until the end
	handle->get_command_buffer()->end();
	VkSubmitInfo info = {};
	info.pNext = nullptr;
	info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	info.commandBufferCount = 1;
	info.pCommandBuffers = &buff;
	info.pWaitDstStageMask = &flag;

	vkQueueSubmit(m_transferQueue->get_queue(), 1, &info, m_transferCommandBuffersFences[mhandle->get_index()]->get_fence());
	m_transferCommandBuffersFences[mhandle->get_index()]->wait();

	// Finished call callback 
	on_finish(mhandle);
}

void TransferOpTransferQueue::destroy()
{
	// First wait for queue
	std::lock_guard guard(m_transferMutex);
	
	vkQueueWaitIdle(m_transferQueue->get_queue());

	assert(m_availableTransferCommandBuffers.size() == m_transferCmdCount);

	for (int i = 0; i < m_transferCmdCount; i++)
	{
		//X TODO : DELETE FROM MANAGER AND GDNEWDA
		m_transferCommandBuffers[i]->destroy();
		delete m_transferCommandBuffers[i];

		// DELETE THE FENCE

		m_transferCommandBuffersFences[i]->destroy();
		delete m_transferCommandBuffersFences[i];
	
		auto handle = m_availableTransferCommandBuffers.front();
		m_availableTransferCommandBuffers.pop();
		delete handle;
	}

	// DESTROY MANAGERS

	m_transferCommandManager->destroy();
}

void TransferOpTransferQueue::on_finish(ITransferHandle* finished)
{
	std::lock_guard guard(m_transferMutex);
	//X TODO : UNSAFE
	TransferQueueHandle* handle = (TransferQueueHandle*)finished;
	
	// Give it back to the available
	m_availableTransferCommandBuffers.push(handle);
	m_executedTransferCommandBuffersQueue.erase(handle->get_executed_cmd_iterator());
	// Erase from executed vector
	

	//X TODO : CALLBACK
	// Call the callback function in handle
	
}

TransferQueueHandle::TransferQueueHandle(GVulkanCommandBuffer* cmd, uint32_t index)
{
	m_cmdBuffer = cmd;
	m_index = index;
}

std::vector<TransferQueueHandle*>::iterator TransferQueueHandle::get_executed_cmd_iterator()
{
	return m_executedCmdIterator;
}

void TransferQueueHandle::set_executed_cmd_iterator(std::vector<TransferQueueHandle*>::iterator iter)
{
	m_executedCmdIterator = iter;
}

uint32_t TransferQueueHandle::get_index() const noexcept
{
	return m_index;
}

GVulkanCommandBuffer* TransferQueueHandle::get_command_buffer()
{
	return m_cmdBuffer;
	return nullptr;
}
