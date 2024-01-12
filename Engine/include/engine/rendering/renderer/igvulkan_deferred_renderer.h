#ifndef IGVULKAN_DEFERRED_RENDERER_H
#define IGVULKAN_DEFERRED_RENDERER_H

#include "engine/rendering/vulkan/vulkan_command_buffer.h"
#include "engine/rendering/vulkan/named/igvulkan_named_renderpass.h"
#include <vector>
#include "engine/rendering/renderer/igvulkan_deferred_renderer.h"

class IVulkanImage;

enum MATERIAL_MODE
{
	MATERIAL_MODE_BLINN_PHONG,
	MATERIAL_MODE_PBR
};

enum VkFormat;

class IGVulkanDeferredRenderer
{
public:
	virtual IGVulkanNamedRenderPass* get_deferred_pass() const noexcept = 0;

	virtual IGVulkanNamedRenderPass* get_composition_pass() const noexcept = 0;

	virtual void fill_deferred_cmd(GVulkanCommandBuffer* cmd,uint32_t frame) = 0;

	virtual void fill_composition_cmd(GVulkanCommandBuffer* cmd, uint32_t frame) = 0;

	virtual void fill_compute_cmd(GVulkanCommandBuffer* cmd, uint32_t frame) = 0;

	virtual std::vector<VkFormat> get_deferred_formats() const noexcept = 0;

	virtual VkFormat get_composition_format() const noexcept = 0;

	virtual void fill_aabb_cmd_for(GVulkanCommandBuffer* cmd, uint32_t frame, uint32_t drawId) = 0;

	virtual MATERIAL_MODE get_current_material_mode() const noexcept = 0;

	virtual void set_material_mode(MATERIAL_MODE mode) noexcept = 0;

	virtual void begin_and_end_fill_cmd_for_shadow(GVulkanCommandBuffer* cmd, uint32_t frame) = 0;

	virtual IVulkanImage* get_sun_shadow_attachment() = 0;

	virtual void set_debug_mode(bool debugMode) = 0;

	virtual bool is_debug_mode_enabled() = 0;
private:
};

#endif // IGVULKAN_DEFERRED_RENDERER_H