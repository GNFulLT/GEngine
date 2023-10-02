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

struct VkDescriptorPool_T;
struct VkPipelineLayout_T;
enum VkFormat;

class GSceneRenderer2 : public IGVulkanDeferredRenderer
{
public:
	inline constexpr static const uint32_t MAX_BINDLESS_TEXTURE = 16536;

	inline constexpr static const uint32_t DRAWSTREAM_SET = 0;
	inline constexpr static const uint32_t DRAWDATA_SET = 1;
	inline constexpr static const uint32_t COMPUTE_SET = 2;
	inline constexpr static const uint32_t BINDLESS_TEXTURE_SET = 3;

	GSceneRenderer2(IGVulkanLogicalDevice* dev, IGPipelineObjectManager* pipelineManager,IGResourceManager* res,IGShaderManager* shaderMng,
		uint32_t framesInFlight,VkFormat compositionFormat);

	bool init(VkDescriptorSetLayout_T* globalUniformSet);

	void destroy();
private:
	template<typename T>
	uint32_t calculate_nearest_10mb()
	{
		constexpr static const uint32_t SIZE = 1024 * 1024 * 10;
		uint32_t perSize = sizeof(T);
		uint32_t mod = SIZE % perSize;
		uint32_t enoughSize = SIZE - mod;
		return enoughSize / perSize;
	}
	template<typename T>
	uint32_t calculate_nearest_1mb()
	{
		constexpr static const uint32_t SIZE = 1024 * 1024;
		uint32_t perSize = sizeof(T);
		uint32_t mod = SIZE % perSize;
		uint32_t enoughSize = SIZE - mod;
		return enoughSize / perSize;
	}

	virtual IGVulkanNamedRenderPass* get_deferred_pass() const noexcept;
	virtual IGVulkanNamedRenderPass* get_composition_pass() const noexcept;
	virtual std::vector<VkFormat> get_deferred_formats() const noexcept;

private:
	IGVulkanLogicalDevice* p_boundedDevice;
	IGPipelineObjectManager* p_pipelineManager;
	IGResourceManager* p_resourceManager;
	IGShaderManager* p_shaderManager;

	GPUMeshStreamResources* m_meshStreamResources;

	GPUMeshStreamResources::CPUGPUData<glm::mat4> m_globalTransformData;
	GPUMeshStreamResources::CPUGPUData<MaterialDescription> m_globalMaterialData;

	IGVulkanNamedSetLayout* m_drawDataSetLayout;
	VkDescriptorPool_T* m_bindlessPool;
	VkDescriptorSetLayout_T* m_bindlessSetLayout;
	VkDescriptorSet_T* m_bindlessSet;

	IGVulkanNamedPipelineLayout* m_deferredLayout;

	IGVulkanDescriptorPool* m_compositionPool;
	VkDescriptorSetLayout_T* m_compositionSetLayout;
	VkDescriptorSet_T* m_compositionSet;

	IGVulkanNamedPipelineLayout* m_compositionLayout;

	IGVulkanNamedRenderPass* m_deferredPass;
	IGVulkanNamedRenderPass* m_compositionPass;

	IGShaderResource* m_deferredVertexShaderRes;
	IGShaderResource* m_deferredFragmentShaderRes;


	IGShaderResource* m_compositionVertexShaderRes;
	IGShaderResource* m_compositionFragmentShaderRes;

	VkFormat m_compositionFormat;


	GVulkanNamedGraphicPipeline* m_deferredPipeline;
	GVulkanNamedGraphicPipeline* m_compositionPipeline;


	uint32_t m_framesInFlight;

	// Inherited via IGVulkanDeferredRenderer
	virtual void fill_deferred_cmd(GVulkanCommandBuffer* cmd,uint32_t frame) override;
	virtual void fill_composition_cmd(GVulkanCommandBuffer* cmd, uint32_t frame) override;
	virtual VkFormat get_composition_format() const noexcept override;
};

#endif // GSCENE_RENDERER_2_H