#ifndef GPU_MESHLET_STREAM_RESOURCES_H
#define GPU_MESHLET_STREAM_RESOURCES_H

#include "internal/engine/rendering/gpu_helpers.h"


class GPUMeshletStreamResources
{
public:
	GPUMeshletStreamResources(IGVulkanLogicalDevice* boundedDevice, IGPipelineObjectManager* pipelineManager,uint32_t frameInFlight);

	bool init(uint32_t beginMeshCount,uint32_t beginGMeshletCount,uint32_t beginMeshletIndicesCount,uint32_t beginPrimitiveIndicesCount);

	IGVulkanNamedSetLayout* get_meshlet_set_layout() const noexcept;

	VkDescriptorSet get_set_by_index(uint32_t index) const;

	void cmd_draw_indirect_data(GVulkanCommandBuffer* cmd, uint32_t frame, uint32_t maxIndirectDrawCommand, IVulkanBuffer* indirectBuffer, uint32_t countOffset);

	uint32_t add_meshlet_data(const GMeshletData* gmeshlet);
	uint32_t add_meshlet_data(const GMeshletDataExtra* gmeshlet);
private:
	IGVulkanLogicalDevice* p_boundedDevice;
	IGPipelineObjectManager* p_pipelineManager;
	uint32_t m_framesInFlight;
	IGVulkanDescriptorPool* m_generalPool;

private:
	CPUGPUData<GMeshlet> m_mergedGMeshlet;
	CPUGPUData<uint32_t> m_mergedMeshletVertex;
	CPUGPUData<uint8_t> m_mergedMeshletTriangles;
	CPUGPUData<GMeshletExtra> m_meshletExtraData;
	IGVulkanNamedSetLayout* m_meshletInfoSetLayout = nullptr;

	PFN_vkCmdDrawMeshTasksIndirectCountEXT vkCmdDrawMeshTasksIndirectCountEXTMethod = nullptr;

	std::vector<VkDescriptorSet> m_meshletInfoSets;
};

#endif // GPU_MESHLET_STREAM_RESOURCES_H