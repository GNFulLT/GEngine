#ifndef GPU_MESH_STREAM_RESOURCES_H
#define GPU_MESH_STREAM_RESOURCES_H
#include "volk.h"
#include "vma/vk_mem_alloc.h"
#include <memory>
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
#include "internal/engine/rendering/gpu_helpers.h"

struct VkDescriptorSet_T;
class GSceneRenderer2;

class GPUMeshStreamResources
{
public:
	
	GPUMeshStreamResources(IGVulkanLogicalDevice* dev,uint32_t floatCountPerVertex,uint32_t framesInFlight,IGPipelineObjectManager* manager);

	bool init(uint32_t beginVertexCount, uint32_t beginIndexCount, uint32_t beginMeshCount, uint32_t beginDrawDataAndIdCount);

	uint32_t add_mesh_data(const MeshData* meshData);
	uint32_t add_mesh_data(const MeshData2* meshData);

	uint32_t add_mesh_data(const GMeshletData* meshlet);

	uint32_t create_draw_data(uint32_t meshIndex,uint32_t materialIndex,uint32_t transformIndex);
	uint32_t get_max_indirect_command_count();
	IGVulkanNamedSetLayout* get_indirect_set_layout();

	void destroy();
	void handle_gpu_datas(GVulkanCommandBuffer* cmd,uint32_t frameIndex);
	void bind_vertex_index_stream(GVulkanCommandBuffer* cmd,uint32_t frameIndex);
	void cmd_draw_indirect_data(GVulkanCommandBuffer* cmd, uint32_t frameIndex);
	void cmd_reset_indirect_buffers(GVulkanCommandBuffer* cmd, uint32_t frameIndex);
	void cmd_indirect_barrier_for_indirect_read(GVulkanCommandBuffer* cmd, uint32_t frameIndex);
	void cmd_copy_cmd(GVulkanCommandBuffer* cmd);
	void cmd_delete_cmd(GVulkanCommandBuffer* cmd);
	uint32_t get_count_of_draw_data();

	IGVulkanNamedSetLayout* get_draw_set_layout();
	IGVulkanNamedSetLayout* get_compute_set_layout();
	IGVulkanNamedSetLayout* get_drawlet_set_layout();

	VkDescriptorSet_T* get_draw_set_by_index(uint32_t currentFrame);
	VkDescriptorSet_T* get_compute_set_by_index(uint32_t currentFrame);

private:
	void update_draw_data_sets();
	void update_compute_sets();
private:
	//X Should be in another class
	CPUGPUData<GMeshMeshletData> m_mergedMeshlet;
	CPUGPUData<GMeshlet> m_mergedGMeshlet;
	CPUGPUData<uint32_t> m_mergedMeshletVertex;
	CPUGPUData<uint8_t> m_mergedMeshletTriangles;
	CPUGPUData<GMeshletExtra> m_meshletExtraData;
 	IGVulkanNamedSetLayout* m_drawletStreamSetLayout;

	//--------------
	std::queue<vkcmd_func> m_copyQueue;
	std::queue<vkcmd_delete_func> m_deleteQueue;

	uint32_t m_framesInFlight;
	uint32_t m_maxIndirectDrawCommand = 0;

	IGPipelineObjectManager* p_pipelineObjectMng;

	CPURGPUData<float> m_mergedVertex;
	CPURGPUData<uint32_t> m_mergedIndex;
	CPUGPUData<GMeshData> m_mergedMesh;

	CPUGPUData<DrawData> m_globalDrawData;
	friend class GSceneRenderer2;

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

#endif // GPU_MESH_RESOURCES_H

