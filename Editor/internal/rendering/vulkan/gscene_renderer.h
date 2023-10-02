#ifndef GSCENE_RENDERER_H
#define GSCENE_RENDERER_H
#include "volk.h"

class IGVulkanViewport;
class IGVulkanDevice;
class GVulkanCommandBuffer;
class GVulkanSemaphore;
class IGShaderResource;
class IVulkanShaderStage;
class IGVulkanGraphicPipeline;
class GCubeRenderer;
class GridRenderer;

#include "internal/rendering/renderable.h"

#include "public/core/templates/shared_ptr.h"
#include "engine/rendering/vulkan/named/viewports/igvulkan_named_deferred_viewport.h"

class GSceneRenderer
{
public:
	GSceneRenderer(IGVulkanNamedDeferredViewport* viewport, IGVulkanDevice* device);

	void render_the_scene();
	
	bool init();
	void destroy();


	void set_the_viewport(IGVulkanNamedDeferredViewport* viewport);
private:
	IGVulkanDevice* m_device;
	IGVulkanNamedDeferredViewport* m_viewport;
	std::vector< std::vector<GVulkanCommandBuffer*>> m_frameCmds;
	std::vector< GVulkanSemaphore*> m_frameSemaphores;
	std::vector<uint32_t> m_currentCmdIndex;

	VkViewport m_vkViewport;
	VkRect2D m_vkScissor;

	GSharedPtr< IGShaderResource> m_basicFragShader;
	GSharedPtr< IGShaderResource> m_basicVertexShader;
	IVulkanShaderStage* m_fragShaderStage = nullptr;
	IVulkanShaderStage* m_vertexShaderStage = nullptr;

	GSharedPtr<GCubeRenderer> m_cubemapRenderer;
	GSharedPtr< GridRenderer> m_gridRenderer;
	Renderable* triangle;
};

#endif // GSCENE_RENDERER_H