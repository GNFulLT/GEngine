#ifndef GPU_HELPERS_H
#define GPU_HELPERS_H

#include "volk.h"
#include "vma/vk_mem_alloc.h"

#include "engine/rendering/vulkan/ivulkan_buffer.h"
#include "engine/rendering/vulkan/ivulkan_ldevice.h"
#include "engine/rendering/mesh_data.h"
#include "internal/engine/rendering/mesh/gmesh_renderable.h"
#include <vector>
#include "engine/rendering/vulkan/named/igvulkan_named_set_layout.h"
#include "engine/manager/igpipeline_object_manager.h"
#include "engine/rendering/vulkan/vulkan_command_buffer.h"
#include <queue>
#include <functional>
#include "engine/manager/igscene_manager.h"
#include "engine/rendering/mesh/gmesh.h"
#include "engine/rendering/mesh/gmeshlet.h"

typedef std::function<void(GVulkanCommandBuffer*)> vkcmd_func;
typedef std::function<void()> vkcmd_delete_func;

template<typename T>
struct CPURGPUData
{
	IGVulkanLogicalDevice* dev;
	std::queue<vkcmd_func>* copyQueue;
	std::queue<vkcmd_delete_func>* deleteQueue;

	std::unique_ptr<IVulkanBuffer> gpuBuffer;
	std::vector<T> cpuVector;
	uint32_t inUsage = 0;
	uint32_t gpuCurrentStride = 0;
	void create_internals(std::queue<vkcmd_func>* copyQueue, std::queue<vkcmd_delete_func>* deleteQueue, IGVulkanLogicalDevice* dev, VkAccessFlagBits srcAccess, VkPipelineStageFlagBits srcStage);
	void unload_gpu_buffer();
	void destroy();
	uint32_t add_to_buffer(const std::vector<T>& buff);

	VkAccessFlagBits expectedFlag;
	VkPipelineStageFlagBits expectedPipelineStage;
};
template<typename T>
struct RCPUGPUData
{
	std::unique_ptr<IVulkanBuffer> gpuBuffer;
	T* gpuBegin = nullptr;
	T* gpuCurrentPos = nullptr;
	uint32_t inUsage = 0;
	void create_internals();
	uint32_t get_current_pos_as_index();
	void unload_gpu_buffer();
	uint32_t add_to_buffer(const std::vector<T>& buff);
	void set_by_index(const T* data, uint32_t index);
	void destroy();
};

template<typename T>
struct CPUGPUData
{
	std::unique_ptr<IVulkanBuffer> gpuBuffer;
	std::vector<T> cpuVector;
	T* gpuBegin = nullptr;
	T* gpuCurrentPos = nullptr;
	uint32_t inUsage = 0;

	void create_internals();

	uint32_t get_current_pos_as_index();

	//X Begin Index
	uint32_t add_to_buffer(const std::vector<T>& buff);
	uint32_t add_to_buffer(const T& buff);

	void unload_gpu_buffer();

	void destroy();

	void set_by_index(const T* data, uint32_t index);
};


template<typename T>
inline void CPUGPUData<T>::create_internals()
{
	assert(gpuBuffer->get_size() % sizeof(T) == 0);
	this->gpuBegin = (T*)this->gpuBuffer->map_memory();
	this->gpuCurrentPos = gpuBegin;
	this->cpuVector.resize(this->gpuBuffer->get_size() / sizeof(T));
}

template<typename T>
inline uint32_t CPUGPUData<T>::get_current_pos_as_index()
{
	return this->gpuCurrentPos - this->gpuBegin;
}

template<typename T>
inline uint32_t CPUGPUData<T>::add_to_buffer(const std::vector<T>& buff)
{
	//X First check there is space
	uint32_t currentPosIndex = get_current_pos_as_index();
	if ((cpuVector.size() - currentPosIndex) < buff.size())
	{
		//X No space resize
		assert(false);
	}
	memcpy(&cpuVector[currentPosIndex], buff.data(), sizeof(T) * buff.size());
	//X Now copy to gpu
	memcpy(gpuCurrentPos, buff.data(), buff.size() * sizeof(T));

	//X Move the cursor
	gpuCurrentPos += buff.size();
	inUsage += buff.size();
	return currentPosIndex;
}

template<typename T>
inline uint32_t CPUGPUData<T>::add_to_buffer(const T& buff)
{
	//X First check there is space
	uint32_t currentPosIndex = get_current_pos_as_index();
	if ((cpuVector.size() - currentPosIndex) < 1)
	{
		//X No space resize
		assert(false);
	}
	memcpy(&cpuVector[currentPosIndex], &buff, sizeof(T));
	//X Now copy to gpu
	memcpy(gpuCurrentPos, &buff, sizeof(T));

	//X Move the cursor
	gpuCurrentPos += 1;
	inUsage += 1;
	return currentPosIndex;
}

template<typename T>
inline void CPUGPUData<T>::unload_gpu_buffer()
{
	gpuBuffer->unmap_memory();
	gpuBuffer->unload();
	gpuBuffer.reset();
	gpuBegin = gpuCurrentPos = nullptr;
}

template<typename T>
inline void CPUGPUData<T>::destroy()
{
	unload_gpu_buffer();
	cpuVector.clear();
	inUsage = 0;
}

template<typename T>
inline void CPUGPUData<T>::set_by_index(const T* data, uint32_t index)
{
	assert(index < cpuVector.size());
	memcpy(&cpuVector[index], data, sizeof(T));
	memcpy(&gpuBegin[index], data, sizeof(T));
}

template<typename T>
inline void RCPUGPUData<T>::create_internals()
{
	assert(gpuBuffer->get_size() % sizeof(T) == 0);
	auto begin = gpuBuffer->map_memory();
	this->gpuBegin = (T*)begin;
	this->gpuCurrentPos = gpuBegin;
}

template<typename T>
inline uint32_t RCPUGPUData<T>::get_current_pos_as_index()
{
	return this->gpuCurrentPos - this->gpuBegin;
}

template<typename T>
inline void RCPUGPUData<T>::unload_gpu_buffer()
{

	gpuBuffer->unmap_memory();
	gpuBuffer->unload();
	gpuBuffer.reset();
	gpuBegin = gpuCurrentPos = nullptr;
}

template<typename T>
inline uint32_t RCPUGPUData<T>::add_to_buffer(const std::vector<T>& buff)
{
	//X First check there is space
	uint32_t currentPosIndex = get_current_pos_as_index();
	if ((buff.size() + currentPosIndex) > gpuBuffer->get_size())
	{
		//X No space resize
		assert(false);
	}
	//X Copy to gpu
	auto size = buff.size() * sizeof(T);
	memcpy(gpuCurrentPos, buff.data(), size);

	//X Move the cursor
	gpuCurrentPos += buff.size();
	inUsage += buff.size();
	return currentPosIndex;
}

template<typename T>
inline void RCPUGPUData<T>::set_by_index(const T* data, uint32_t index)
{
	memcpy(&gpuBegin[index], data, sizeof(T));
}

template<typename T>
inline void RCPUGPUData<T>::destroy()
{
	unload_gpu_buffer();
	inUsage = 0;
}

template<typename T>
inline void CPURGPUData<T>::create_internals(std::queue<vkcmd_func>* copyQueue, std::queue<vkcmd_delete_func>* deleteQueue, IGVulkanLogicalDevice* dev, VkAccessFlagBits srcAccess, VkPipelineStageFlagBits srcStage)
{
	this->dev = dev;
	this->cpuVector.resize(this->gpuBuffer->get_size() / sizeof(T));
	this->copyQueue = copyQueue;
	this->deleteQueue = deleteQueue;
	this->expectedFlag = srcAccess;
	this->expectedPipelineStage = srcStage;
}

template<typename T>
inline void CPURGPUData<T>::unload_gpu_buffer()
{
	gpuBuffer->unload();
	gpuBuffer.reset();
	inUsage = 0;
	this->gpuCurrentStride = 0;
}

template<typename T>
inline void CPURGPUData<T>::destroy()
{
	unload_gpu_buffer();
}

template<typename T>
inline uint32_t CPURGPUData<T>::add_to_buffer(const std::vector<T>& buff)
{
	uint32_t currentPosIndex = inUsage;
	if ((buff.size()) * sizeof(T) > gpuBuffer->get_size())
	{
		//X No space resize
		assert(false);
	}

	//X TODO : OPTIMIZE IT
	//X Copy to cpu
	auto size = buff.size() * sizeof(T);
	memcpy(&cpuVector[currentPosIndex], buff.data(), size);
	//X Create a staging buffer that stores newly data
	auto stagingBuffer = dev->create_buffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY).value();
	auto stagingMapped = stagingBuffer->map_memory();
	memcpy(stagingMapped, buff.data(), size);
	stagingBuffer->unmap_memory();
	this->copyQueue->push([&, stagingBuff = stagingBuffer, dstOffset = currentPosIndex * sizeof(T)](GVulkanCommandBuffer* cmd) {
		//X Add Barrier For Copy OP
		VkBufferMemoryBarrier barr = {};
		barr.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
		barr.pNext = nullptr;
		barr.srcAccessMask = expectedFlag;
		barr.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barr.buffer = gpuBuffer->get_vk_buffer();
		barr.size = gpuBuffer->get_size();

		vkCmdPipelineBarrier(cmd->get_handle(), expectedPipelineStage, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, 0, 1, &barr, 0, 0);
		VkBufferCopy region = {};
		region.size = stagingBuff->get_size();
		region.srcOffset = 0;
		region.dstOffset = dstOffset;

		vkCmdCopyBuffer(cmd->get_handle(), stagingBuff->get_vk_buffer(), gpuBuffer->get_vk_buffer(), 1, &region);

		barr.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barr.dstAccessMask = expectedFlag;

		vkCmdPipelineBarrier(cmd->get_handle(), VK_PIPELINE_STAGE_TRANSFER_BIT, expectedPipelineStage, 0, 0, 0, 1, &barr, 0, 0);

		deleteQueue->push([stagingBufft = stagingBuff]() {
			stagingBufft->unload();
			delete stagingBufft;
			});
		});
	//X Move the cursor
	inUsage += buff.size();
	return currentPosIndex;

}

#endif // GPU_HELPERS_H