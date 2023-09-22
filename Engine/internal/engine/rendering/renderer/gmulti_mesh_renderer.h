#ifndef GMULTI_MESH_RENDERER_H
#define GMULTI_MESH_RENDERER_H

#include <vector>
#include "internal/engine/rendering/mesh/gmesh_renderable.h"
#include "engine/rendering/mesh/gmesh.h"
#include <memory>
#include "engine/rendering/vulkan/igvulkan_storage_buffer.h"
#include "engine/rendering/vulkan/igvulkan_indirect_buffer.h"
#include "engine/rendering/vulkan/ivulkan_ldevice.h"
#include "engine/rendering/vulkan/vulkan_command_buffer.h"
#include "engine/rendering/vulkan/ivulkan_descriptor_pool.h"
#include "engine/manager/igcamera_manager.h"
#include "engine/manager/igresource_manager.h"
#include "engine/manager/igshader_manager.h"
#include "engine/resource/igshader_resource.h"
#include "engine/rendering/vulkan/ivulkan_shader_stage.h"
#include "engine/rendering/vulkan/ivulkan_graphic_pipeline_state.h"
#include "internal/engine/rendering/renderer/gmulti_mesh_renderer_layout.h"
#include "engine/rendering/vulkan/ivulkan_viewport.h"

#include "engine/rendering/mesh_data.h"
#include "engine/rendering/scene/scene.h"
#include "engine/rendering/material/gmaterial.h"
#include "engine/rendering/vulkan/ivulkan_descriptor_pool.h"
#include "engine/rendering/vulkan/vulkan_memory.h"
#include "engine/rendering/wireframe_spec.h"

struct VkDescriptorSet_T;
struct VkDescriptorSetLayout_T;
struct VkPipelineLayout_T;
struct VkPipeline_T;



class GMultiMeshRenderer
{
public:
	GMultiMeshRenderer(IGVulkanLogicalDevice* dev,IGCameraManager* cam,IGResourceManager* res,IGShaderManager* sm,uint32_t frameCount,Scene* scene,std::vector<MaterialDescription>& materials,MeshData* meshData);

	bool init(IGVulkanViewport* port);

	void update_indirect_commands(uint32_t frameIndex);
	
	void fill_command_buffer(GVulkanCommandBuffer* cmd,uint32_t frameIndex,IGVulkanViewport* vp);

	void fill_command_buffer_dispatch(GVulkanCommandBuffer* cmd, uint32_t frameIndex);

	void destroy();

	std::vector<MaterialDescription>* get_global_materials();

	WireFrameSpec* get_wireframe_spec();
private:
	IGVulkanLogicalDevice* m_boundedDevice;
	uint32_t m_frameCount;
	IGCameraManager* m_cameraManager;
	std::vector<InstanceData> m_instanceDatas;

	GMultiMeshRendererLayout* m_layout;
	GMultiMeshRendererLayout* m_rasterizedLayout;

	uint32_t maxVertexBufferSize_, maxIndexBufferSize_;
	uint32_t maxInstances_;
	uint32_t maxInstanceSize_, maxMaterialSize_;


	std::unique_ptr<IGVulkanStorageBuffer> m_storageBuffer;
	std::unique_ptr<IGVulkanStorageBuffer> m_materialBuffer;
	std::unique_ptr<IGVulkanStorageBuffer> m_instancedBuffers;
	std::unique_ptr<IGVulkanStorageBuffer> m_transformBuffer;
	std::unique_ptr<IGVulkanStorageBuffer> m_boundingBoxBuffer;
	std::unique_ptr<IGVulkanStorageBuffer> m_meshDataBuffer;

	std::unique_ptr<IGVulkanUniformBuffer> m_cullDataBuffer;
	void* m_cullDataBufferMappedMem = nullptr;


	std::vector<IGVulkanIndirectBuffer*> m_indirectBuffers;

	IGResourceManager* m_res;
	IGShaderManager* m_sm;

	IGShaderResource* m_vert;
	IGShaderResource* m_frag;
	IGShaderResource* m_comp;

	IVulkanShaderStage* m_vertStage;
	IVulkanShaderStage* m_compStage;

	IGVulkanGraphicPipeline* m_pipeline;
	IGVulkanGraphicPipeline* m_rasterizedPipeline;

	IVulkanShaderStage* m_fragStage;
	VkPipelineLayout_T* m_compPipeLayout;
	Scene* m_scene;
	std::vector<MaterialDescription> m_materialDatas;
	MeshData* m_meshDatas;
	
	VkPipeline_T* m_compPipeline;

	VkDescriptorSetLayout_T* m_compSetLayout;
	std::vector<VkDescriptorSet_T*> m_compSets;
	IGVulkanDescriptorPool* m_compPool;

	WireFrameSpec m_wireframeSpec;
};


#endif // GMULTI_MESH_RENDERER_H