#ifndef GSCENE_RENDERER_2_H
#define GSCENE_RENDERER_2_H

#include "internal/engine/rendering/gpu_mesh_stream_resources.h"
#include <memory>
#include <glm/glm.hpp>
#include "engine/rendering/material/gmaterial.h"
#include "engine/rendering/vulkan/ivulkan_descriptor_pool.h"
#include "internal/engine/rendering/vulkan/named/gvulkan_named_graphic_pipeline.h"
#include "engine/rendering/vulkan/ivulkan_shader_stage.h"
#include "engine/rendering/vulkan/ivulkan_graphic_pipeline_state.h"
#include "engine/manager/igshader_manager.h"
#include "engine/manager/igresource_manager.h"
#include "engine/rendering/renderer/igvulkan_deferred_renderer.h"
#include "engine/rendering/vulkan/named/igvulkan_named_viewport.h"
#include "engine/manager/igcamera_manager.h"
#include "engine/manager/igscene_manager.h"
#include "internal/engine/rendering/gpu_meshlet_stream_resources.h"

#include <span>
struct VkDescriptorPool_T;
struct VkPipelineLayout_T;
enum VkFormat;
struct VkSampler_T;

class GSceneRenderer2 : public IGVulkanDeferredRenderer
{
public:
	inline constexpr static const uint32_t DRAWSTREAM_SET = 0;
	inline constexpr static const uint32_t DRAWDATA_SET = 1;
	inline constexpr static const uint32_t COMPUTE_SET = 2;
	inline constexpr static const uint32_t BINDLESS_TEXTURE_SET = 3;

	GSceneRenderer2(IGVulkanLogicalDevice* dev, IGPipelineObjectManager* pipelineManager,IGResourceManager* res,IGShaderManager* shaderMng,IGSceneManager* sceneMng,
		uint32_t framesInFlight,VkFormat compositionFormat);

	bool init(VkDescriptorSetLayout_T* globalUniformSet,IGVulkanNamedSetLayout* drawDataSetLayout,IGVulkanNamedSetLayout* lightDataSetLayout, IGVulkanNamedSetLayout* cullSetLayout);
	void set_drawdata_set(VkDescriptorSet_T* drawDataSet);
	void set_lightdata_set(VkDescriptorSet_T* lightDataSet);
	void set_culldata_set(VkDescriptorSet_T* cullDataSet);
	uint32_t get_max_indirect_draw_count();
	virtual IGVulkanNamedRenderPass* get_deferred_pass() const noexcept;
	virtual IGVulkanNamedRenderPass* get_composition_pass() const noexcept;
	virtual std::vector<VkFormat> get_deferred_formats() const noexcept;
	virtual void fill_deferred_cmd(GVulkanCommandBuffer* cmd, uint32_t frame) override;
	virtual void fill_composition_cmd(GVulkanCommandBuffer* cmd, uint32_t frame) override;
	virtual void fill_compute_cmd(GVulkanCommandBuffer* cmd, uint32_t frame) override;
	virtual void fill_aabb_cmd_for(GVulkanCommandBuffer* cmd, uint32_t frame,uint32_t drawId) override;
	virtual void begin_and_end_fill_cmd_for_shadow(GVulkanCommandBuffer* cmd, uint32_t frame);
	virtual const DrawData* get_draw_data_by_id(uint32_t drawId) const noexcept;

	virtual VkFormat get_composition_format() const noexcept override;

	virtual void set_composition_views(IVulkanImage* position, IVulkanImage* albedo, IVulkanImage* emission, IVulkanImage* pbr,VkSampler_T* sampler,IGVulkanNamedViewport* deferredVp, IGVulkanNamedViewport* compositionVp);

	uint32_t add_mesh_to_scene(const MeshData* meshData, uint32_t shapeID = 0);
	uint32_t add_mesh_to_scene(const MeshData2* meshData);

	uint32_t add_meshlet_to_scene(const GMeshletData* meshlet);
	uint32_t add_meshlet_to_scene(const GMeshletDataExtra* meshlet);

	uint32_t create_draw_data(uint32_t meshIndex, uint32_t materialIndex, uint32_t transformIndex);

	VkDescriptorSet_T* get_bindless_set();
	void destroy();

	uint32_t get_max_count_of_draw_data();
	template<typename T>
	static uint32_t calculate_nearest_10mb()
	{
		constexpr static const uint32_t SIZE = 1024 * 1024 * 10;
		uint32_t perSize = sizeof(T);
		uint32_t mod = SIZE % perSize;
		uint32_t enoughSize = SIZE - mod;
		return enoughSize / perSize;
	}
	template<typename T>
	static uint32_t calculate_nearest_1mb()
	{
		constexpr static const uint32_t SIZE = 1024 * 1024;
		uint32_t perSize = sizeof(T);
		uint32_t mod = SIZE % perSize;
		uint32_t enoughSize = SIZE - mod;
		return enoughSize / perSize;
	}
private:
	IVulkanImage* m_jitterImage;
	bool load_shadow_resources();
	void generate_offsets_for_shadow_texture(int windowSize,int filterSize,std::vector<float>& data);
	IVulkanImage* m_shadowAttachment;
	IGVulkanNamedRenderPass* m_shadowPass;
	VkFramebuffer_T* m_shadowFrameBuffer;
private:
	VkSampler_T* m_depthSampler;
	VkDescriptorSet_T* m_drawDataSet;
	VkDescriptorSet_T* m_lightDataSet;
	VkDescriptorSet_T* m_cullDataSet;
	VkDescriptorSet_T* m_sunShadowSet;
private:
	bool m_useMeshlet = false;
	IGVulkanNamedPipelineLayout* m_deferredletLayout;
	IGShaderResource* m_deferredletVertexShaderRes;
	IGShaderResource* m_deferredTaskShaderRes;
	IGShaderResource* m_deferredMeshShaderRes;
	IGShaderResource* m_deferredMeshFragmentShaderRes;

	VkPipeline_T* m_compMeshletPipeline;
	IGVulkanNamedPipelineLayout* m_computeMeshletPipelineLayout;


	IGShaderResource* m_sunShadowMeshShaderRes;
	IGShaderResource* m_sunShadowTaskShaderRes;

	GVulkanNamedGraphicPipeline* m_sunShadowMeshPipeline;

	//--------
	IGVulkanNamedSetLayout* m_sunShadowSetLayout;

	uint32_t MAX_BINDLESS_TEXTURE = 16536;
	IGVulkanLogicalDevice* p_boundedDevice;
	IGPipelineObjectManager* p_pipelineManager;
	IGResourceManager* p_resourceManager;
	IGShaderManager* p_shaderManager;

	GPUMeshletStreamResources* m_meshletStreamResources;
	GPUMeshStreamResources* m_meshStreamResources;
	MATERIAL_MODE m_materialMode = MATERIAL_MODE_BLINN_PHONG;
	
	IGVulkanNamedPipelineLayout* m_computePipelineLayout;

	VkPipeline_T* m_compPipeline;

	VkDescriptorPool_T* m_bindlessPool;
	VkDescriptorSetLayout_T* m_bindlessSetLayout;
	VkDescriptorSet_T* m_bindlessSet;

	IGVulkanNamedPipelineLayout* m_deferredLayout;
	IGVulkanNamedPipelineLayout* m_sunShadowLayout;

	IGVulkanNamedPipelineLayout* m_aabbDrawLayout;

	IGVulkanDescriptorPool* m_compositionPool;
	VkDescriptorSetLayout_T* m_compositionSetLayout;
	VkDescriptorSet_T* m_compositionSet;

	IGVulkanNamedPipelineLayout* m_compositionLayout;

	IGVulkanNamedRenderPass* m_deferredPass;
	IGVulkanNamedRenderPass* m_compositionPass;

	IGShaderResource* m_deferredVertexShaderRes;

	IGShaderResource* m_deferredFragmentShaderRes;
	IGShaderResource* m_deferredFragmentPBRShaderRes;


	IGShaderResource* m_compositionVertexShaderRes;
	IGShaderResource* m_compositionFragmentShaderRes;

	IGShaderResource* m_cullComputeShaderRes;
	IGShaderResource* m_cullComputeMeshletShaderRes;

	IGShaderResource* m_boundingBoxVertexShaderRes;
	IGShaderResource* m_boundingBoxFragmentShaderRes;
	IGShaderResource* m_sunShadowVertexShaderRes;
	IGShaderResource* m_sunShadowFragmentShaderRes;


	VkFormat m_compositionFormat;

	GVulkanNamedGraphicPipeline* m_selectedCompositionPipeline;
	GVulkanNamedGraphicPipeline* m_deferredPipeline;
	GVulkanNamedGraphicPipeline* m_deferredMeshletPipeline;

	GVulkanNamedGraphicPipeline* m_compositionPBRPipeline;

	GVulkanNamedGraphicPipeline* m_sunShadowPipeline;

	GVulkanNamedGraphicPipeline* m_compositionPipeline;
	GVulkanNamedGraphicPipeline* m_aabbPipeline;

	IGVulkanNamedViewport* m_deferredVp;
	IGVulkanNamedViewport* m_compositionVp;


	IGSceneManager* p_sceneManager;
	
	uint32_t m_framesInFlight;

	// Inherited via IGVulkanDeferredRenderer
	virtual MATERIAL_MODE get_current_material_mode() const noexcept override;
	virtual void set_material_mode(MATERIAL_MODE mode) noexcept override;

	// Inherited via IGVulkanDeferredRenderer
	virtual IVulkanImage* get_sun_shadow_attachment() override;
};

#endif // GSCENE_RENDERER_2_H