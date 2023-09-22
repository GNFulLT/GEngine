#ifndef GSCENE_RENDERER_H
#define GSCENE_RENDERER_H

#include <cstdint>
#include "engine/rendering/vulkan/ivulkan_buffer.h"
#include "engine/rendering/material/gmaterial.h"
#include "engine/rendering/mesh_data.h"
#include "engine/resource/igshader_resource.h"
#include <memory>
#include <vector>
#include "internal/engine/rendering/mesh/gmesh_renderable.h"
#include "engine/rendering/vulkan/ivulkan_shader_stage.h"
#include "engine/rendering/vulkan/ivulkan_graphic_pipeline.h"
#include "engine/rendering/vulkan/ivulkan_viewport.h"
#include "engine/rendering/wireframe_spec.h"
#include "engine/rendering/vulkan/ivulkan_descriptor_pool.h"
#include "engine/rendering/vulkan/named/igvulkan_named_viewport.h"
#include "engine/rendering/vulkan/ivulkan_renderpass.h"
#include "internal/engine/rendering/vulkan/named/gvulkan_named_deferred_viewport.h"
#include "internal/engine/rendering/renderer/gscene_composition_renderer_layout.h"
#include "engine/resource/igtexture_resource.h"
class IGVulkanLogicalDevice;
class IGCameraManager;
class IGResourceManager;
class IGShaderManager;
class Scene;
class GSceneWireframeRendererLayout;
class IGSceneManager;
struct VkDescriptorSetLayout_T;
struct VkDescriptorSet_T;
struct VkPipelineLayout_T;
struct VkPipeline_T;
class GSceneMaterialRendererLayout;
class GSceneDeferredRendererLayout;
struct DrawDataGlobalInfo
{
	uint32_t drawCount;
	uint32_t meshCount;
};

class GSceneRenderer
{
public:
	GSceneRenderer(IGVulkanLogicalDevice* device, IGCameraManager* cam, IGSceneManager* sceneMng,IGResourceManager* res, IGShaderManager* sm, uint32_t frameCount, Scene* scene,
		std::vector<MaterialDescription>& materials, MeshData* meshData);

	bool init(IGVulkanViewport* vp);

	void destroy();

	bool init_deferred(uint32_t width, uint32_t height);
	
	bool resize_deferred(uint32_t width, uint32_t height);

	IGVulkanNamedViewport* get_deferred_vp();

	void update_scene_nodes();

	void fill_command_buffer(GVulkanCommandBuffer* cmd, uint32_t frameIndex, IGVulkanViewport* vp);
	void fill_command_deferred(GVulkanCommandBuffer* cmd, uint32_t frameIndex);
	void fill_command_buffer_dispatch(GVulkanCommandBuffer* cmd, uint32_t frameIndex);

	int add_mesh(MeshData* mesh);

	std::vector<MaterialDescription>* get_global_materials();

	Scene* get_global_scene();

	MeshData* get_global_mesh_data();

	WireFrameSpec* get_wireframe_spec();

	uint32_t add_node_with_mesh(uint32_t meshIndex);

	bool create_deferred_resources();
	
	bool init_bindless();

	uint32_t add_texture(IGTextureResource* res);


	uint32_t add_material(const MaterialDescription& desc);

	void set_material_for_node(uint32_t nodeID,uint32_t materialIndex);
private:
	void resize_vertex_buffer(uint32_t newSize);
	void resize_index_buffer(uint32_t newSize);

private:
	//X Vertex Buffer Full Size
	uint64_t m_vertexBufferFullSize;
	uint64_t m_vertexBufferInUsageSize;
	
	uint32_t m_textureSize = 0;

	VkDescriptorPool_T* m_bindlessPool;

	VkDescriptorSetLayout_T* m_bindlessSetLayout;
	VkDescriptorSet_T* m_bindlessSet;
	//X Index Buffer Sizes
	uint64_t m_indexBufferFullSize;
	uint64_t m_indexBufferInUsageSize;

	uint64_t m_maxIndirectDrawCommand;

	std::unique_ptr<IVulkanBuffer> m_mergedVertexBuffer;
	std::unique_ptr<IVulkanBuffer> m_mergedIndexBuffer;
	std::unique_ptr<IVulkanBuffer> m_meshDataSSBO;
	std::unique_ptr<IVulkanBuffer> m_materialSSBO;
	std::unique_ptr<IVulkanBuffer> m_drawDataSSBO;

	std::unique_ptr<IVulkanBuffer> m_drawDataIDSSBO;
	std::unique_ptr<IVulkanBuffer> m_cullDataUniform;
	void* m_cullDataMappedMem;

	std::unique_ptr<IVulkanBuffer> m_transformDataSSBO;
	void* m_transformDataMappedMem;

	std::vector<std::unique_ptr<IVulkanBuffer>> m_indirectCommandSSBOs;
	std::unique_ptr<IVulkanBuffer> m_indirectCommandStagingBuffer;
	GVulkanNamedDeferredViewport* m_deferredVp;
	uint32_t m_indirectCommandsBeginOffset;
	//X Pointers
	IGVulkanLogicalDevice* p_boundedDevice;
	IGCameraManager* p_cameraManager;
	IGResourceManager* p_resourceManager;
	IGShaderManager* p_shaderManager;
	IGSceneManager* p_sceneManager;

	//X Caches
	uint32_t m_framesInFlight;
	Scene* m_scene;
	MeshData* m_meshData;
	std::vector<MaterialDescription> m_materials;

	IGVulkanGraphicPipeline* m_wireframePipeline;
	IGVulkanGraphicPipeline* m_materialPipeline;
	IGVulkanGraphicPipeline* m_deferredPipeline;
	IGVulkanGraphicPipeline* m_composingPipeline;

	IGShaderResource* m_materialFragShader;
	IGShaderResource* m_normalVertexShader;
	IGShaderResource* m_wireframeFragShader;
	IGShaderResource* m_cullCompShader;
	IGShaderResource* m_deferredVertexShader;
	IGShaderResource* m_deferredPixelShader;
	IGShaderResource* m_composingVertexShader;
	IGShaderResource* m_composingPixelShader;

	IVulkanShaderStage* m_materialFragStage;
	IVulkanShaderStage* m_normalVertexStage;
	IVulkanShaderStage* m_wireframeFragStage;
	IVulkanShaderStage* m_cullCompStage;
	IVulkanShaderStage* m_deferredVertexStage;
	IVulkanShaderStage* m_deferredFragStage;
	IVulkanShaderStage* m_composingVertexStage;
	IVulkanShaderStage* m_composingFragStage;

	DrawDataGlobalInfo m_drawGlobalData;
	//------------------------------------------------

	GSceneWireframeRendererLayout* m_wireframeLayout;
	GSceneMaterialRendererLayout* m_materialLayout;
	GSceneDeferredRendererLayout* m_deferredLayout;
	GSceneCompositionRendererLayout* m_compositionLayout;

	std::vector<GMeshData> meshData;
	std::vector<DrawData> drawDatas;

	WireFrameSpec m_wireframeSpec;

	//X Compute
	IGVulkanDescriptorPool* m_compPool;
	VkDescriptorSetLayout_T* m_compSetLayout;
	std::vector< VkDescriptorSet_T*> m_compSets;
	VkPipelineLayout_T* m_compPipeLayout;
	VkPipeline_T* m_compPipeline;


	bool isWireframeInUse = true;
};

#endif // GSCENE_RENDERER_H