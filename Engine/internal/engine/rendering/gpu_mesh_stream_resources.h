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

class GPUMeshStreamResources
{
public:
	GPUMeshStreamResources(IGVulkanLogicalDevice* dev,uint32_t floatCountPerVertex,uint32_t framesInFlight,IGPipelineObjectManager* manager);

	bool init(uint32_t beginVertexCount, uint32_t beginIndexCount, uint32_t beginMeshCount, uint32_t beginDrawDataAndIdCount);
private:
	void update_draw_data_sets();
	void update_compute_sets();
private:
	uint32_t m_framesInFlight;
	template<typename T>
	struct CPUGPUData
	{
		std::unique_ptr<IVulkanBuffer> gpuBuffer;
		std::vector<T> cpuVector;
		T* gpuBegin = nullptr;
		T* gpuCurrentPos = nullptr;
		uint32_t inUsage = 0;
		
		void create_internals();
	};

	IGPipelineObjectManager* p_pipelineObjectMng;

	CPUGPUData<float> m_mergedVertex;
	CPUGPUData<uint32_t> m_mergedIndex;
	CPUGPUData<GMeshData> m_mergedMesh;
	CPUGPUData<GMeshData> m_globalDrawData;

	std::unique_ptr<IVulkanBuffer> m_globalDrawIdBuffer;
	uint32_t m_inUsageSizeGlobalDrawDataBuffer;

	std::vector<std::unique_ptr<IVulkanBuffer>> m_globalIndirectCommandBuffers;
	uint32_t m_globalIndirectCommandsBeginOffset; // Also indicates the size of the count buffer

	IGVulkanLogicalDevice* p_boundedDevice;
	uint32_t m_floatPerVertex;


	IGVulkanNamedSetLayout* m_drawStreamSetLayout;
	IGVulkanNamedSetLayout* m_computeSetLayout;

	IGVulkanDescriptorPool* m_generalPool;
	std::vector<VkDescriptorSet> m_drawStreamSets;
	std::vector<VkDescriptorSet> m_computeSets;

};

template<typename T>
inline void GPUMeshStreamResources::CPUGPUData<T>::create_internals()
{
	assert(gpuBuffer->get_size() % sizeof(T) == 0);
	this->gpuBegin = (T*)this->gpuBuffer->map_memory();
	this->gpuCurrentPos = gpuBegin;
	this->cpuVector.resize(this->gpuBuffer->get_size()/sizeof(T));
}

#endif // GPU_MESH_RESOURCES_H


