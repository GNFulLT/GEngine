#ifndef GSCENE_MANAGER_H
#define GSCENE_MANAGER_H

#include "engine/manager/igscene_manager.h"
#include <vector>
#include "internal/engine/rendering/renderer/gscene_renderer2.h"
#include "engine/rendering/scene/scene.h"
#include "engine/rendering/material/gmaterial.h"
#include "engine/rendering/point_light.h"
#include "public/core/templates/unordered_dense.h"
#include "engine/rendering/vulkan/ivulkan_image.h"
#include "internal/engine/scene/gentity.h"

struct VkDescriptorSet_T;
struct VkFramebuffer_T;
struct VkRenderPass_T;
class IGCameraManager;
class IGVulkanLogicalDevice;
class IGVulkanUniformBuffer;

class GSceneManager : public IGSceneManager
{
public:
	GSceneManager();
	struct GlobalCullBuffer
	{
		DrawCullData cullData;
		bool cullEnabled = true;
	};
	
	
	struct GPUCPUData
	{
		uint32_t selectedDraw = -1;
	};
	// Inherited via IGSceneManager
	virtual IGVulkanUniformBuffer* get_global_buffer_for_frame(uint32_t frame) const noexcept override;
	virtual void reconstruct_global_buffer_for_frame(uint32_t frame) override;
	virtual bool init(uint32_t framesInFlight) override;
	virtual void destroy() override;
	virtual bool init_deferred_renderer(IGVulkanNamedDeferredViewport* deferred) override;
	virtual bool is_renderer_active() override;
	virtual IGVulkanDeferredRenderer* get_deferred_renderer() const noexcept override;
	virtual VkDescriptorSet_T* get_global_set_for_frame(uint32_t frame) const noexcept override;
	virtual IGVulkanNamedDeferredViewport* create_default_deferred_viewport(IGVulkanNamedRenderPass* deferredPass, IGVulkanNamedRenderPass* compositionPass, VkFormat compositionFormat) override;
	
	virtual uint32_t add_node_to_root();
	virtual uint32_t add_mesh_to_scene(const MeshData* mesh);
	virtual uint32_t add_meshlet_to_scene(const GMeshletData* meshlet) override;

	virtual uint32_t add_node_with_mesh_and_defaults(uint32_t meshIndex);


	virtual Scene* get_current_scene() const noexcept override;
	virtual std::span<MaterialDescription> get_current_scene_materials() override;
	virtual void set_material_by_index(const MaterialDescription* material, uint32_t index) override;

	virtual uint32_t register_texture_to_scene(IGTextureResource* textureRes) override;

	virtual uint32_t add_material_to_scene(const MaterialDescription* desc) override;
	virtual uint32_t add_materials_to_scene(const std::vector<MaterialDescription>* desc);
	virtual uint32_t add_point_light_node() override;	
private:
	uint32_t add_default_transform();
	void set_transform_by_index(const glm::mat4* transform, uint32_t gpuIndex);
private:

private:
	//X Global Draw Data
	RCPUGPUData<glm::mat4> m_globalTransformData;
	CPUGPUData<MaterialDescription> m_globalMaterialData;
	CPUGPUData<GPointLight> m_globalPointLights;

	std::vector<CPUGPUData<uint32_t>> m_globalPointLightIndices;
	std::vector<CPUGPUData<uint32_t>> m_globalPointLightBins;
	std::vector<CPUGPUData<uint32_t>> m_globalPointLightTiles;
	
	std::vector<std::unique_ptr<IVulkanBuffer>> m_gpuCpuDataBuffers;
	std::vector<void*> m_gpuCpuDataBufferMappedMems;

	GlobalCullBuffer m_globalCullData;
	IGVulkanNamedSetLayout* m_cullDataLayout;
	GlobalUniformBuffer m_globalData;
	IGCameraManager* m_cameraManager;
	IGVulkanLogicalDevice* m_logicalDevice;
	uint32_t m_framesInFlight;
	std::vector<IGVulkanUniformBuffer*> m_globalBuffers;
	std::vector<void*> m_globalBufferMappedMem;

	std::vector<IVulkanBuffer*> m_globalCullBuffers;
	std::vector<void*> m_globalCullBufferMappedMems;

	ankerl::unordered_dense::segmented_map<uint32_t, IGTextureResource*> m_registeredTextureMap;
	IGVulkanNamedDeferredViewport* m_deferredTargetedViewport = nullptr;
	
	IGVulkanNamedSetLayout* m_globalDataSetLayout;
	IGVulkanNamedSetLayout* m_drawDataSetLayout;
	IGVulkanNamedSetLayout* m_globalLightSetLayout;
	IGVulkanDescriptorPool* m_globalPool;

	IGVulkanNamedSetLayout* m_gpuCpuDataSetLayout;

	uint32_t m_inUsageTextures = 0;

	GSceneRenderer2* m_deferredRenderer = nullptr;

	std::vector<VkDescriptorSet_T*> m_globalSets;
	VkDescriptorSet_T* m_drawDataSet;
	std::vector<VkDescriptorSet_T*> m_cullDataSets;
	std::vector<VkDescriptorSet_T*> m_globalLightSets;
	std::unordered_map<uint32_t, uint32_t> m_cpu_to_gpu_map;
	std::unordered_map<uint32_t, uint32_t> m_nodeToLight;
	std::unordered_map<uint32_t, uint32_t> m_nodeToDrawID;
	std::vector<IGTextureResource*> m_registeredTextures;

	Scene* m_currentScene;
	Scene* m_editorScene;


	// Inherited via IGSceneManager
	virtual uint32_t add_node_with_mesh_and_material(uint32_t meshIndex, uint32_t materialIndex) override;


	// Inherited via IGSceneManager
	virtual uint32_t add_node_with_mesh_and_material_and_transform(uint32_t meshIndex, uint32_t materialIndex, const glm::mat4* transform) override;


	// Inherited via IGSceneManager
	virtual uint32_t add_child_node_with_mesh_and_material_and_transform(uint32_t parentNode, uint32_t meshIndex, uint32_t materialIndex, const glm::mat4* transform) override;


	// Inherited via IGSceneManager
	virtual bool is_cull_enabled() override;

	virtual void set_cull_enabled(bool cullEnabled) override;


	// Inherited via IGSceneManager
	virtual uint32_t get_draw_id_of_node(uint32_t nodeId) override;


	// Inherited via IGSceneManager
	virtual uint32_t get_gpu_transform_index(uint32_t nodeId) const noexcept override;


	// Inherited via IGSceneManager
	virtual const DrawData* get_draw_data_by_id(uint32_t drawId) const noexcept override;


	// Inherited via IGSceneManager
	virtual IGTextureResource* get_saved_texture_by_id(uint32_t textureId) const noexcept override;


	// Inherited via IGSceneManager
	virtual bool is_node_light(uint32_t nodeId) const noexcept override;

	virtual const GPointLight* get_point_light(uint32_t nodeId) const noexcept override;

	virtual void set_point_light(const GPointLight* data,uint32_t nodeId) noexcept override;


	// Inherited via IGSceneManager
	virtual const SunProperties* get_sun_properties() const noexcept override;

	virtual void update_sun_properties(const SunProperties* sunProps) override;


	// Inherited via IGSceneManager
	virtual const GlobalUniformBuffer* get_global_data() const noexcept override;

};

#endif // GSCENE_MANAGER_H