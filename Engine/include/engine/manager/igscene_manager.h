#ifndef ISCENE_MANAGER_H
#define ISCENE_MANAGER_H


#include "engine/GEngine_EXPORT.h"
#include "engine/rendering/vulkan/named/viewports/igvulkan_named_deferred_viewport.h"
#include "engine/rendering/vulkan/named/viewports/igvulkan_named_composition_viewport.h"
#include "engine/rendering/renderer/igvulkan_deferred_renderer.h"
#include "engine/rendering/material/gmaterial.h"
#include "engine/scene/gentity.h"

#include <cstdint>
#include <vector>
#include <span>
#include <filesystem>
#include <expected>

struct VkDescriptorSet_T;
enum VkFormat;
class IGVulkanUniformBuffer;
struct MeshData;
struct MeshData2;
class Scene;
#include "engine/resource/igtexture_resource.h"

struct SunProperties
{
	float sunLightDirection[3];
	float sunLightColor[3];
	float sunIntensity = 1.f;
};

struct DrawData
{
	uint32_t mesh;
	uint32_t material;
	uint32_t transformIndex;
};

struct GlobalUniformBuffer
{
	float viewProj[16];
	float pos[3];
	float view[16];
	float resolution[2];
	uint32_t pointLightCount = 0;
	uint32_t activePointLightCount = 0;
	float zNear;
	float zFar;
	SunProperties sunProperties;
	float sunLP[16];

};
struct GPointLight;
struct GMeshletData;
struct GMeshletDataExtra;

struct SceneRes
{
	Scene* scene = nullptr;
	std::unordered_map<uint32_t, glm::mat4> transformMap;
	std::unordered_map<uint32_t, DrawData> drawDataMap;
	std::vector<uint32_t> lightMap;
};

class ENGINE_API IGSceneManager
{
public:
	virtual ~IGSceneManager() = default;

	virtual IGVulkanUniformBuffer* get_global_buffer_for_frame(uint32_t frame) const noexcept = 0;

	virtual bool init(uint32_t framesInFlight) = 0;

	virtual void reconstruct_global_buffer_for_frame(uint32_t frame) = 0;

	virtual void destroy() = 0;

	virtual bool init_deferred_renderer(IGVulkanNamedDeferredViewport* deferred) = 0;

	virtual bool is_renderer_active() = 0;
	
	virtual GEntity* get_entity_by_id(uint32_t id) const noexcept = 0;

	virtual IGVulkanDeferredRenderer* get_deferred_renderer() const noexcept = 0;
	
	virtual VkDescriptorSet_T* get_global_set_for_frame(uint32_t frame) const noexcept = 0;

	virtual IGVulkanNamedDeferredViewport* create_default_deferred_viewport(IGVulkanNamedRenderPass* deferredPass, IGVulkanNamedRenderPass* compositionPass,VkFormat compositionFormat) = 0;
	
	virtual uint32_t add_mesh_to_scene(const MeshData* mesh) = 0;
	virtual uint32_t add_mesh_to_scene(const MeshData2* mesh) = 0;

	virtual uint32_t add_node_with_mesh_and_defaults(uint32_t meshIndex) = 0;
	virtual uint32_t add_node_with_mesh_and_material(uint32_t meshIndex,uint32_t materialIndex) = 0;
	virtual uint32_t add_node_with_mesh_and_material_and_transform(uint32_t meshIndex, uint32_t materialIndex,const glm::mat4* transform) = 0;
	virtual uint32_t add_child_node_with_mesh_and_material_and_transform(uint32_t parentNode,uint32_t meshIndex, uint32_t materialIndex, const glm::mat4* transform) = 0;
	virtual uint32_t add_meshlet_to_scene(const GMeshletData* meshlet) = 0;
	//X Unsafe to use for private usage. Use load scene instead
	virtual uint32_t add_meshlet_to_scene(const GMeshletDataExtra* meshlet) = 0;
	virtual std::string get_material_name_by_id(uint32_t materialId) = 0;

	virtual uint32_t add_material_to_scene(const MaterialDescription* material) = 0;
	virtual Scene* get_current_scene() const noexcept = 0;

	virtual std::span<MaterialDescription> get_current_scene_materials() = 0;

	virtual void set_material_by_index(const MaterialDescription* material,uint32_t index) = 0;

	virtual uint32_t register_texture_to_scene(IGTextureResource* textureRes) = 0;
	
	virtual uint32_t add_point_light_node() = 0;

	virtual bool is_cull_enabled() = 0;

	virtual void set_cull_enabled(bool cullEnabled) = 0;
	
	virtual uint32_t get_draw_id_of_node(uint32_t nodeId) const noexcept = 0;

	virtual uint32_t get_gpu_transform_index(uint32_t nodeId) const noexcept = 0;
	virtual const DrawData* get_draw_data_by_id(uint32_t drawId) const noexcept = 0;

	virtual IGTextureResource* get_saved_texture_by_id(uint32_t textureId) const noexcept = 0;

	virtual bool is_node_light(uint32_t nodeId) const noexcept = 0;

	virtual const GPointLight* get_point_light(uint32_t nodeId) const noexcept = 0;

	virtual void set_point_light(const GPointLight* data, uint32_t nodeId) noexcept = 0;

	virtual const SunProperties* get_sun_properties() const noexcept = 0;
	virtual void update_sun_properties(const SunProperties* sunProps) = 0;

	virtual const GlobalUniformBuffer* get_global_data() const noexcept = 0;

	virtual void update_entities(float dt) = 0;

	virtual bool load_scene(std::filesystem::path path) = 0;

	virtual std::string get_mesh_name(uint32_t meshIndex) const noexcept = 0;
	virtual void set_mesh_name(uint32_t meshIndex, const char* name) = 0;
	virtual uint32_t get_mesh_count() const noexcept = 0;

	//X TODO : MOVE THIS FUNCTION  TO ANOTHER CLASS. Given path should be directory and fileName shouldn have extension
	virtual bool save_loaded_mesh(std::filesystem::path path,const char* fileName,uint32_t meshIndex) = 0;

	virtual uint32_t get_saved_texture_count() const noexcept = 0;

	virtual bool serialize_scene(std::filesystem::path path,Scene* scene) const noexcept = 0;

	virtual std::expected<SceneRes, uint32_t> deserialize_scene(std::filesystem::path path) noexcept = 0;

	virtual uint32_t load_gmesh_file(std::filesystem::path path) = 0;

	virtual uint32_t load_gmaterial_file(std::filesystem::path path) = 0;

	virtual IGVulkanNamedDeferredViewport* get_current_viewport() = 0;
};
#endif // ISCENE_MANAGER_H