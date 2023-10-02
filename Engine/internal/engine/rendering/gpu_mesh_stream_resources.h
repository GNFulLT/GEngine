#ifndef GPU_MESH_STREAM_RESOURCES_H
#define GPU_MESH_STREAM_RESOURCES_H

#include <memory>
#include "engine/rendering/vulkan/ivulkan_buffer.h"
#include "engine/rendering/vulkan/ivulkan_ldevice.h"
#include "engine/rendering/mesh_data.h"
#include "internal/engine/rendering/mesh/gmesh_renderable.h"
#include <vector>
#include "engine/rendering/vulkan/named/igvulkan_named_set_layout.h"
#include "engine/manager/igpipeline_object_manager.h"

struct VkDescriptorSet_T;

class GPUMeshStreamResources
{
public:
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

		void unload_gpu_buffer();

		void destroy();
	};

	GPUMeshStreamResources(IGVulkanLogicalDevice* dev,uint32_t floatCountPerVertex,uint32_t framesInFlight,IGPipelineObjectManager* manager);

	bool init(uint32_t beginVertexCount, uint32_t beginIndexCount, uint32_t beginMeshCount, uint32_t beginDrawDataAndIdCount);

	void add_mesh_data(MeshData* meshData);

	void destroy();
	
	uint32_t get_count_of_draw_data();

	IGVulkanNamedSetLayout* get_draw_set_layout();
	IGVulkanNamedSetLayout* get_compute_set_layout();

	VkDescriptorSet_T* get_draw_set_by_index(uint32_t currentFrame);
	VkDescriptorSet_T* get_compute_set_by_index(uint32_t currentFrame);

private:
	void update_draw_data_sets();
	void update_compute_sets();
private:
	uint32_t m_framesInFlight;


	IGPipelineObjectManager* p_pipelineObjectMng;

	CPUGPUData<float> m_mergedVertex;
	CPUGPUData<uint32_t> m_mergedIndex;
	CPUGPUData<GMeshData> m_mergedMesh;
	CPUGPUData<DrawData> m_globalDrawData;

	std::unique_ptr<IVulkanBuffer> m_globalDrawIdBuffer;
	uint32_t m_inUsageSizeGlobalDrawDataBuffer;

	std::vector<std::unique_ptr<IVulkanBuffer>> m_globalIndirectCommandBuffers;
	uint32_t m_globalIndirectCommandsBeginOffset; // Also indicates the size of the count buffer

	IGVulkanLogicalDevice* p_boundedDevice;
	uint32_t m_floatPerVertex;


	IGVulkanNamedSetLayout* m_drawStreamSetLayout;
	IGVulkanNamedSetLayout* m_computeSetLayout;

	IGVulkanDescriptorPool* m_generalPool;
	std::vector<VkDescriptorSet_T*> m_drawStreamSets;
	std::vector<VkDescriptorSet_T*> m_computeSets;

};

template<typename T>
inline void GPUMeshStreamResources::CPUGPUData<T>::create_internals()
{
	assert(gpuBuffer->get_size() % sizeof(T) == 0);
	this->gpuBegin = (T*)this->gpuBuffer->map_memory();
	this->gpuCurrentPos = gpuBegin;
	this->cpuVector.resize(this->gpuBuffer->get_size()/sizeof(T));
}

template<typename T>
inline uint32_t GPUMeshStreamResources::CPUGPUData<T>::get_current_pos_as_index()
{
	return this->gpuCurrentPos - this->gpuBegin;
}

template<typename T>
inline uint32_t GPUMeshStreamResources::CPUGPUData<T>::add_to_buffer(const std::vector<T>& buff)
{
	//X First check there is space
	uint32_t currentPosIndex = get_current_pos_as_index();
	if ((cpuVector.size() - currentPosIndex) < buff.size())
	{
		//X No space resize
		assert(false);
	}
	memcpy(&cpuVector[currentPosIndex], buff.data(),sizeof(T)*buff.size());
	//X Now copy to gpu
	memcpy(gpuCurrentPos, buff.data(), buff.size() * sizeof(T));

	//X Move the cursor
	gpuCurrentPos += buff.size();

	return currentPosIndex;
}

template<typename T>
inline void GPUMeshStreamResources::CPUGPUData<T>::unload_gpu_buffer()
{
	gpuBuffer->unmap_memory();
	gpuBuffer->unload();
	gpuBuffer.reset();
	gpuBegin = gpuCurrentPos = nullptr;
}

template<typename T>
inline void GPUMeshStreamResources::CPUGPUData<T>::destroy()
{
	unload_gpu_buffer();
	cpuVector.clear();
	inUsage = 0;
}

#endif // GPU_MESH_RESOURCES_H


