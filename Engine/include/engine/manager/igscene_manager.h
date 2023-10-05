#ifndef ISCENE_MANAGER_H
#define ISCENE_MANAGER_H


#include "engine/GEngine_EXPORT.h"
#include "engine/rendering/vulkan/named/viewports/igvulkan_named_deferred_viewport.h"
#include "engine/rendering/vulkan/named/viewports/igvulkan_named_composition_viewport.h"
#include "engine/rendering/renderer/igvulkan_deferred_renderer.h"
#include "engine/rendering/material/gmaterial.h"

#include <cstdint>
#include <vector>
#include <span>

struct VkDescriptorSet_T;
enum VkFormat;
class IGVulkanUniformBuffer;
struct MeshData;
class Scene;

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

	virtual IGVulkanDeferredRenderer* get_deferred_renderer() const noexcept = 0;
	
	virtual VkDescriptorSet_T* get_global_set_for_frame(uint32_t frame) const noexcept = 0;

	virtual IGVulkanNamedDeferredViewport* create_default_deferred_viewport(IGVulkanNamedRenderPass* deferredPass, IGVulkanNamedRenderPass* compositionPass,VkFormat compositionFormat) = 0;
	
	virtual uint32_t add_mesh_to_scene(const MeshData* mesh) = 0;
	virtual uint32_t add_node_with_mesh_and_defaults(uint32_t meshIndex) = 0;

	virtual Scene* get_current_scene() const noexcept = 0;

	virtual std::span<MaterialDescription> get_current_scene_materials() = 0;

	virtual void set_material_by_index(const MaterialDescription* material,uint32_t index) = 0;
};
#endif // ISCENE_MANAGER_H