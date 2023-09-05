#include "volk.h"

#include "internal/engine/rendering/vulkan/transfer/transfer_op_transfer_queue.h"
#include "engine/rendering/vulkan/vulkan_command_buffer.h"
#include "engine/rendering/vulkan/vulkan_memory.h"
#include "internal/engine/rendering/vulkan/vulkan_ldevice.h"
#include "engine/rendering/vulkan/ivulkan_device.h"

#include "engine/rendering/vulkan/ivulkan_queue.h"
#include "engine/rendering/vulkan/ivulkan_image.h"
#include "engine/rendering/vulkan/ivulkan_buffer.h"

#include <thread>

TransferOpTransferQueue::TransferOpTransferQueue(GVulkanLogicalDevice* dev,uint32_t transferCmdCount)
{
	m_transferQueue = dev->get_resource_queue();
	m_transferCmdCount = transferCmdCount;
	m_transferCommandManager = std::unique_ptr<GVulkanCommandBufferManager>(new GVulkanCommandBufferManager(dev,m_transferQueue, true));
	m_transferOwnershipCommandManager = std::unique_ptr<GVulkanCommandBufferManager>(new GVulkanCommandBufferManager(dev, dev->get_render_queue(), true));
	m_fenceManager = std::unique_ptr<GVulkanFenceManager>(new GVulkanFenceManager(dev));
	m_boundedDevice = dev;
	m_semaphoreManager = std::unique_ptr<GVulkanSemaphoreManager>(new GVulkanSemaphoreManager(dev));
}

bool TransferOpTransferQueue::init()
{
	bool inited = m_transferCommandManager->init();

	if (!inited)
		return false;
	inited = m_transferOwnershipCommandManager->init();

	if (!inited)
		return false;

	//X TODO GDNEWDA 
	m_transferCommandBuffers.resize(m_transferCmdCount);
	m_transferCommandBuffersFences.resize(m_transferCmdCount);
	m_transferCommandBufferSemaphores.resize(m_transferCmdCount);
	m_transferOwnershipCommandBuffers.resize(m_transferCmdCount);

	for (int i = 0; i < m_transferCmdCount; i++)
	{
		m_transferCommandBuffers[i] = m_transferCommandManager->create_buffer(true);
		m_transferCommandBuffersFences[i] = m_fenceManager->create_fence();
		m_transferCommandBufferSemaphores[i] = m_semaphoreManager->create_semaphore();
		m_transferOwnershipCommandBuffers[i] = m_transferOwnershipCommandManager->create_buffer(true);

		inited = m_transferCommandBuffers[i]->init();
		//X TODO: DESTROY AND DELETE ALL LAST CREATED CMDS
		if (!inited)
		{
			return false;
		}

		inited = m_transferOwnershipCommandBuffers[i]->init();
		if (!inited)
		{
			return false;
		}
		inited = m_transferCommandBuffersFences[i]->init(false);
		if (!inited)
		{
			return false;
		}

		inited = m_transferCommandBufferSemaphores[i]->init();
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

		m_transferOwnershipCommandBuffers[i]->destroy();
		delete m_transferOwnershipCommandBuffers[i];

		m_transferCommandBufferSemaphores[i]->destroy();
		delete m_transferCommandBufferSemaphores[i];
	}

	// DESTROY MANAGERS

	m_transferCommandManager->destroy();
	m_transferOwnershipCommandManager->destroy();
}

std::expected<IVulkanImage*, int> TransferOpTransferQueue::init_image_to_the_gpu_from_cpu_sleep(VkImageCreateInfo* inf, VkImageViewCreateInfo* viewInfo, int buffSize, void* pBuff)
{
	// First create the image for transfer queue
	uint32_t index = m_transferQueue->get_queue_index();

	inf->queueFamilyIndexCount = 1;
	inf->pQueueFamilyIndices = &index;


	auto res = m_boundedDevice->create_image(inf, VMA_MEMORY_USAGE_CPU_COPY);

	if (!res.has_value())
	{
		auto err = res.error();
		switch (err) {
		case VULKAN_IMAGE_CREATION_ERROR_UNKNOWN:
			return std::unexpected(1);
		}
	}

	auto image = res.value();


	bool viewCreated = image->create_image_view(viewInfo);

	if (!viewCreated)
		return std::unexpected(1);


	// We are using cached memory
	// 	VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	VkBufferUsageFlags flags = VkBufferUsageFlags(VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
	auto bufferRes = m_boundedDevice->create_buffer(buffSize, flags, VMA_MEMORY_USAGE_CPU_ONLY);

	if (!bufferRes.has_value())
	{
		auto err = bufferRes.error();
		switch (err) {
		case VULKAN_BUFFER_CREATION_ERROR_UNKNOWN:
			return std::unexpected(1);
		}
	}

	auto imageBuffer = bufferRes.value();
	// Now we are creating buffer that will be copied to the image
	imageBuffer->copy_data_to_device_memory(pBuff, buffSize);

	// Ask a transfer queue from the device
	auto cmdRes = m_boundedDevice->get_wait_and_begin_transfer_cmd();

	if (!cmdRes.has_value())
	{
		auto err = cmdRes.error();
		{
			imageBuffer->unload();
			delete imageBuffer;
		}
		// Timeout
		return std::unexpected(1);
	}

	auto transferHandle = cmdRes.value();
	auto cmd = transferHandle->get_command_buffer();


	VkBufferImageCopy region{};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;

	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;

	region.imageOffset = { 0, 0, 0 };
	region.imageExtent = inf->extent;

	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image->get_vk_image();
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.srcAccessMask = 0;
	barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

	auto  sourceStage = VK_PIPELINE_STAGE_HOST_BIT;
	auto destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;

	vkCmdPipelineBarrier(
		cmd->get_handle(),
		sourceStage, destinationStage,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier
	);


	vkCmdCopyBufferToImage(
		cmd->get_handle(),
		imageBuffer->get_vk_buffer(),
		image->get_vk_image(),
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1,
		&region
	);

	sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.srcQueueFamilyIndex = m_transferQueue->get_queue_index();
	barrier.dstQueueFamilyIndex = m_boundedDevice->get_render_queue()->get_queue_index();

	vkCmdPipelineBarrier(
		cmd->get_handle(),
		sourceStage,
		destinationStage,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier
	);

	// Start to transfer op
	TransferQueueHandle* thandle = (TransferQueueHandle*)transferHandle;
	VkSemaphore signalSemaphore = m_transferCommandBufferSemaphores[thandle->get_index()]->get_semaphore();

	VkCommandBuffer buff = transferHandle->get_command_buffer()->get_handle(); 
	transferHandle->get_command_buffer()->end();
	VkSubmitInfo info = {};
	info.pNext = nullptr;
	info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	info.commandBufferCount = 1;
	info.pCommandBuffers = &buff;
	info.signalSemaphoreCount = 0;
	info.pSignalSemaphores = nullptr;

	vkQueueSubmit(m_transferQueue->get_queue(), 1, &info, m_transferCommandBuffersFences[thandle->get_index()]->get_fence());

	// Now give the owner ship to the transer op queue
	//X TODO : Thread Count
	{
		std::lock_guard guard(m_transferOwnerShipMutex);
		
		m_transferOwnershipCommandBuffers[thandle->get_index()]->begin();
		
		cmd = m_transferOwnershipCommandBuffers[thandle->get_index()];

		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		vkCmdPipelineBarrier(
			cmd->get_handle(),
			sourceStage,
			destinationStage,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);

		m_transferOwnershipCommandBuffers[thandle->get_index()]->end();

		VkCommandBuffer bfbf = cmd->get_handle();
		VkPipelineStageFlags flag = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

		VkSubmitInfo info2 = {};
		info2.pNext = nullptr;
		info2.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		info2.commandBufferCount = 1;
		info2.pCommandBuffers = &bfbf;
		info2.pWaitDstStageMask = &flag;
		info2.waitSemaphoreCount = 0;
		info2.pWaitSemaphores = nullptr;


		m_transferCommandBuffersFences[thandle->get_index()]->wait();
		m_transferCommandBuffersFences[thandle->get_index()]->reset();

		vkQueueSubmit(m_boundedDevice->get_render_queue()->get_queue(),1,&info2,m_transferCommandBuffersFences[thandle->get_index()]->get_fence());

		m_transferCommandBuffersFences[thandle->get_index()]->wait();
		m_transferCommandBuffersFences[thandle->get_index()]->reset();

		vkResetCommandBuffer(m_transferCommandBuffers[thandle->get_index()]->get_handle(), 0);
		vkResetCommandBuffer(m_transferOwnershipCommandBuffers[thandle->get_index()]->get_handle(), 0);
	}

	on_finish(thandle);

	imageBuffer->unload();

	//X TODO : GDNEWDA
	delete imageBuffer;

	return image;


}

void TransferOpTransferQueue::on_finish(ITransferHandle* finished)
{
	std::lock_guard guard(m_transferMutex);
	//X TODO : UNSAFE
	TransferQueueHandle* handle = (TransferQueueHandle*)finished;
	
	// Give it back to the available
	m_availableTransferCommandBuffers.push(handle);
	for (int i = 0; i < m_executedTransferCommandBuffersQueue.size(); i++)
	{
		if (m_executedTransferCommandBuffersQueue[i]->get_index() == handle->get_index())
		{
			m_executedTransferCommandBuffersQueue.erase(m_executedTransferCommandBuffersQueue.begin() + i);
			break;
		}
	}
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
